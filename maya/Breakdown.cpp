//*********************************************************
// Breakdown.cpp
//
// Copyright (C) 2007-2021 Skeletal Studios
// All rights reserved.
//
//*********************************************************

//*********************************************************
#include "Breakdown.h"
#include "ErrorReporting.h"
//*********************************************************

//*********************************************************
// Name: Constructor
// Desc: 
//*********************************************************
Breakdown::Breakdown( MFnAnimCurve &animCurve,
                      double weight,
                      Breakdown::BreakdownMode mode,
                      bool tickDrawSpecial,
                      MTime time,
                      bool isBoolean,
                      unsigned int id,
                      MStatus *status )
{
    breakdownStatus = MS::kSuccess;

    originalKeyValue = 0.0;
    originalKeyIndex = -1;
    previousKeyIndex = -1;
    nextKeyIndex     = -1;

    if( !fnAnimCurve.setObject( animCurve.object())) {
        pluginError( "Breakdown", "Breakdown", "Failed to set animCurve object" );
        breakdownStatus = MS::kFailure;
    }
    else if( (numKeys = fnAnimCurve.numKeys()) == 0 ) {
        pluginError( "Breakdown", "Breakdown", "No keys are set on fnAnimCurve" );
        breakdownStatus = MS::kFailure;       
    }
    else {
        breakdownWeight = weight;
        breakdownMode = mode;
        breakdownTime = time;
        
        keyTickDrawSpecial = tickDrawSpecial;
        undoKeyTickDrawSpecial = false;

        isBooleanAttr = isBoolean;

        objID = id;

        breakdownIndex = -1;
        breakdownValue = 0.0;

        originalPlayheadTime = MAnimControl::currentTime();

        closestIndex = fnAnimCurve.findClosest( breakdownTime );
        closestTime = fnAnimCurve.time( closestIndex );

        // Evaluate original key first as the next two methods depend
        // on its results.
        evaluateOriginalKey();
        evaluateNextKey();
        evaluatePreviousKey();

        // -> A key AFTER the breakdown time is always required
        // -> A key BEFORE the breakdown time is always required in overwrite mode
        // -> A key BEFORE the breakdown time in ripple mode is only required when there
        //    is no original key set.  If an original key is set, it is used as the
        //    previous key
        if( !hasNextKey()) {
            errorMsg.set( "No key set after the current time" );
            breakdownStatus = MS::kFailure;
        }
        else if ((!hasPreviousKey() && breakdownMode == kOverwrite ) ||
                 (!hasPreviousKey() && (breakdownMode == kRipple) && !hasOriginalKey() ))
        {
            errorMsg.set( "No key set before the current time" );
            breakdownStatus = MS::kFailure;
        }
        else {
            calcNewBreakdownValue();
        }
    }

    initialized = false;

    *status = breakdownStatus;
}

//*********************************************************
// Name: Desturctor
// Desc: 
//*********************************************************
Breakdown::~Breakdown()
{

}

//*********************************************************
// Name: redoIt
// Desc: 
//*********************************************************
MStatus Breakdown::redoIt()
{
    if( breakdownMode == Breakdown::kOverwrite )
        redoOverwrite();
    else if( breakdownMode == Breakdown::kRipple )
        redoRipple();

    // Set the tick color to be displayed on the timeline
    setTickDrawSpecial();

    return breakdownStatus;
}

//*********************************************************
// Name: undoIt
// Desc: 
//*********************************************************
MStatus Breakdown::undoIt()
{
    // Return the tickDrawSpecial to its original state
    setTickDrawSpecial( true );

    if( breakdownMode == Breakdown::kOverwrite )
        undoOverwrite();
    else if( breakdownMode == Breakdown::kRipple )
        undoRipple();

    return breakdownStatus;
}

//*********************************************************
// Name: redoOverwrite
// Desc: 
//*********************************************************
void Breakdown::redoOverwrite()
{
    if( hasOriginalKey() && !initialized ) {
        breakdownStatus = fnAnimCurve.setValue( originalKeyIndex, breakdownValue, &animCache );
        if( !breakdownStatus )
            pluginError( "Breakdown", "redoOverwrite", "Failed to set breakdown key value" );

        breakdownIndex = originalKeyIndex;

        initialized = true;
    }
    else if( !hasOriginalKey() && !initialized ) {

        // Use stepped out tangents for breakdowns on boolean attributes
        MFnAnimCurve::TangentType outTangent;

        if( isBooleanAttr )
            outTangent = MFnAnimCurve::kTangentStep;
        else
            outTangent = MFnAnimCurve::kTangentGlobal;
        
        breakdownIndex = fnAnimCurve.addKey( breakdownTime,
                                             breakdownValue,
                                             MFnAnimCurve::kTangentGlobal,
                                             outTangent,
                                             &animCache,
                                             &breakdownStatus );

        if( !breakdownStatus )
            pluginError( "Breakdown", "redoOverwrite", "Failed to add key" );

        initialized = true;
    }
    else {
        breakdownStatus = animCache.redoIt();
        if( !breakdownStatus )
            pluginError( "Breakdown", "redoOverwrite", "Failed to redo" );
    }
}

//*********************************************************
// Name: undoOverwrite
// Desc: 
//*********************************************************
void Breakdown::undoOverwrite()
{
    breakdownStatus = animCache.undoIt();
    if( !breakdownStatus )
        pluginError( "Breakdown", "redoOverwrite", "Failed to Undo" );
}

//*********************************************************
// Name: redoRipple
// Desc: 
//*********************************************************
void Breakdown::redoRipple()
{
    if( !hasOriginalKey() )
        redoOverwrite();
    else if( !initialized ) {
        MTime keyTime;
        // Move all keys after the original key one frame forward
        for( unsigned int index = (numKeys - 1); index > (unsigned int)originalKeyIndex; index-- )
        {
            keyTime = fnAnimCurve.time( index, &breakdownStatus );
            keyTime.setUnit( MTime::uiUnit() );
            keyTime++;

            fnAnimCurve.setTime( index, keyTime, &animCache );
        }

        // Use stepped out tangents for breakdowns on boolean attributes
        MFnAnimCurve::TangentType outTangent;

        if( isBooleanAttr )
            outTangent = MFnAnimCurve::kTangentStep;
        else
            outTangent = MFnAnimCurve::kTangentGlobal;

        // Add the new breakdown one frame after the original
        breakdownTime = fnAnimCurve.time( originalKeyIndex, &breakdownStatus );
        breakdownTime.setUnit( MTime::uiUnit() );
        breakdownTime++;

        breakdownIndex = fnAnimCurve.addKey( breakdownTime,
                                             breakdownValue,
                                             MFnAnimCurve::kTangentGlobal,
                                             outTangent,
                                             &animCache,
                                             &breakdownStatus );

        // Move the playhead forward to the new frame
        MAnimControl::setCurrentTime( breakdownTime );

        initialized = true;
    }
    else {
        breakdownStatus = animCache.redoIt();
        if( !breakdownStatus )
            pluginError( "Breakdown", "redoOverwrite", "Failed to redo" );

        // Move the playhead forward to the new frame
        MAnimControl::setCurrentTime( breakdownTime );
    }



}

//*********************************************************
// Name: undoRipple
// Desc: 
//*********************************************************
void Breakdown::undoRipple()
{
    if( !hasOriginalKey() )
        undoOverwrite();
    else {
        breakdownStatus = animCache.undoIt();
        if( !breakdownStatus )
            pluginError( "Breakdown", "redoOverwrite", "Failed to Undo" );

        // Move the playhead back to its original position
        MAnimControl::setCurrentTime( originalPlayheadTime );
    }
}

//*********************************************************
// Name: evaluateOriginalKey
// Desc: 
//*********************************************************
bool Breakdown::evaluateOriginalKey()
{
    if( closestTime == breakdownTime ) {
        originalKeyIndex = closestIndex;
        originalKeyValue = fnAnimCurve.value( originalKeyIndex );
    }

    return hasOriginalKey();
}

//*********************************************************
// Name: evaluatePreviousKey
// Desc: 
//*********************************************************
bool Breakdown::evaluatePreviousKey()
{
    if( hasOriginalKey() ) {
        if( originalKeyIndex > 0 )
            previousKeyIndex = originalKeyIndex - 1;
    }
    else {
        if( closestTime < breakdownTime )
            previousKeyIndex = closestIndex;
        else if( closestIndex > 0 )
            previousKeyIndex = closestIndex - 1;
        else
            previousKeyIndex = -1;
    }

    return hasPreviousKey();
}

//*********************************************************
// Name: evaluateNextKey
// Desc: 
//*********************************************************
bool Breakdown::evaluateNextKey()
{
    if( hasOriginalKey() ) {
        if( originalKeyIndex < ((int)numKeys - 1))
            nextKeyIndex = originalKeyIndex + 1;
    }
    else {
        if( closestTime > breakdownTime )
            nextKeyIndex = closestIndex;
        else if( closestIndex < numKeys - 1 )
            nextKeyIndex = closestIndex + 1;
        else
            nextKeyIndex = -1;
    }

    return hasNextKey();
}

//*********************************************************
// Name: calcNewBreakdownValue
// Desc: 
//*********************************************************
void Breakdown::calcNewBreakdownValue()
{
    double previousValue;
    double nextValue;

    if( (breakdownMode == kRipple) && hasOriginalKey() )
        previousValue = fnAnimCurve.value( originalKeyIndex );
    else
        previousValue = fnAnimCurve.value( previousKeyIndex );

    nextValue = fnAnimCurve.value( nextKeyIndex );
    
    if( isBooleanAttr )
        breakdownValue = previousValue;
    else
        breakdownValue = ((nextValue - previousValue) * breakdownWeight) + previousValue;
}

//*********************************************************
// Name: setTickDrawSpecial
// Desc: 
//*********************************************************
MStatus Breakdown::setTickDrawSpecial( bool isUndo )
{
    MPlug drawSpecPlugArray = fnAnimCurve.findPlug( "keyTickDrawSpecial", &breakdownStatus );
    if( !breakdownStatus ) {
        pluginError( "Breakdown", "setTickDrawSpecial", "No MPlug with name keyTickDrawSpecial" );
    }
    else {
        MPlug drawSpecialPlug = drawSpecPlugArray.elementByLogicalIndex( breakdownIndex, &breakdownStatus );
        if( !breakdownStatus ) {
            pluginError( "Breakdown", "setTickDrawSpecial", "Failed to get logical index" );
        }
        else {
            // Are we redoing or undoing the command
            if( !isUndo ) {
                // Redo: Store the previous value for undoing and set the new value
                drawSpecialPlug.getValue( undoKeyTickDrawSpecial );
                drawSpecialPlug.setValue( keyTickDrawSpecial );
            }
            else
                // Undo: Restore the previous value
                drawSpecialPlug.setValue( undoKeyTickDrawSpecial );
        }
    }

    return breakdownStatus;
}