//*********************************************************
// CurveCleanerCommand.cpp
//
// Copyright (C) 2007 Circus Ink Entertainment
// All rights reserved.
//
//*********************************************************

//*********************************************************
#include "CurveCleanerCommand.h"
#include "ErrorReporting.h"
//*********************************************************

//*********************************************************
// Constants
//*********************************************************
const char *CurveCleanerCommand::tangentsFlag = "-t";
const char *CurveCleanerCommand::tangentsLongFlag = "-tangents";
const char *CurveCleanerCommand::removeRedundantKeysFlag = "-rrk";
const char *CurveCleanerCommand::removeRedundantKeysLongFlag = "-removeRedundantKeys";
const char *CurveCleanerCommand::splineStartEndFlag = "-sse";
const char *CurveCleanerCommand::splineStartEndLongFlag = "-splineStartEnd";
const char *CurveCleanerCommand::smoothnessFlag = "-s";
const char *CurveCleanerCommand::smoothnessLongFlag = "-smoothness";
const char *CurveCleanerCommand::weightFactorFlag = "-wf";
const char *CurveCleanerCommand::weightFactorLongFlag = "-weightFactor";
const char *CurveCleanerCommand::smoothAllSplinesFlag = "-sas";
const char *CurveCleanerCommand::smoothAllSplinesLongFlag = "-smoothAllSplines";

//*********************************************************
// Name: CurveCleanerCommand
// Desc: Constructor
//*********************************************************
CurveCleanerCommand::CurveCleanerCommand()
{
    pluginTrace( "CurveCleanerCommand", "CurveCleanerCommand", "******* CurveCleanerCommand *******" );

    initialized = false;

    numKeysRemoved = 0;
    numCurvesCleaned = 0;

    cleanTangents = false;
    removeRedundantKeys = false;

    startEndTangentType = MFnAnimCurve::kTangentSmooth;
    smoothingValue = 0.0;
    smoothAllSplines = false;
    weightFactor = 0.333;
}

//*********************************************************
// Name: CurveCleanerCommand
// Desc: Destructor
//*********************************************************
CurveCleanerCommand::~CurveCleanerCommand()
{
    // Cleanup all memory allocated for this command
    if( !animCurveFnList.empty()) {
        animCurveListIter = animCurveFnList.begin();

        while( animCurveListIter != animCurveFnList.end() ) {

            delete (*animCurveListIter).pAnimCurveFn;
            delete (*animCurveListIter).pAnimCache;
            animCurveListIter++;
        }
    }
}

//*********************************************************
// Name: doIt
// Desc: All of the one-time setup and initialization
//       code for the set key command.  doIt is called
//       by Maya when any command is executed in MEL.
//       Any code that changes the state of Maya is
//       handled by the redoIt method.
//*********************************************************
MStatus CurveCleanerCommand::doIt( const MArgList &args )
{
    MStatus status = MS::kFailure;

    // Set the command flag values appropriately
    if( !parseCommandFlags( args )) {
        pluginError( "CurveCleanerCommand", "doIt", "Failed to parse command flags" );
    }
    // Get a list of the currently selected objects
    else if( !getSelectedObjects() ) {
        pluginError( "CurveCleanerCommand", "doIt", "Failed to get selected objects" );
    }
    // Get a list of all of the anim curve function sets
    else if( !getAnimCurveFnList() ) {
        pluginError( "CurveCleanerCommand", "doIt", "Failed to create AnimCurveFnList" );
        MGlobal::displayError( "No Keys Set" );
    }

    else {

        // Execute all operations that change the state of Maya
        if( !(status = redoIt() )) {
            pluginError( "CurveCleanerCommand", "doIt", "Failed to redoIt" );
        }
    }

    if( status ) {
        MString result( "Result: " );
        if( removeRedundantKeys )
            MGlobal::displayInfo( result + numKeysRemoved );
        if( cleanTangents )
            MGlobal::displayInfo( result + numCurvesCleaned );
    }

    return status;
}

//*********************************************************
// Name: redoIt
// Desc: Contains the code that changes the internal state
//       of Maya.  It is called by Maya to redo.
//*********************************************************
MStatus CurveCleanerCommand::redoIt()
{
    MStatus status = MS::kSuccess;

    if( !initialized ) {
        initialized = true;

        if( status && removeRedundantKeys ) {
            if( !(status = removeRedundantKeysFromSelected())) {
                // Cleanup any keys that were affected
                undoIt();

                pluginError( "CurveCleanerCommand", "redoIt", "Failed to remove redundant keys" );
            }
        }
        if( status && cleanTangents ) {
            if( !(status = cleanTangentsOnSelected())) {
                // Cleanup any keys that were affected
                undoIt();

                pluginError( "CurveCleanerCommand", "redoIt", "Failed to clean tangents" );
            }
        }
    }

    else {
        // Just use the anim curve cache to redo
        if( !animCurveFnList.empty()) {
            animCurveListIter = animCurveFnList.begin();

            while( animCurveListIter != animCurveFnList.end() ) {
                // Be sure to skip any NULL pointers
                if( (*animCurveListIter).pAnimCache != NULL )
                    ((*animCurveListIter).pAnimCache)->redoIt();

                animCurveListIter++;
            }
        }
    }

    return status;
}

//*********************************************************
// Name: undoIt
// Desc: Contains the code to undo the internal state 
//       changes made by the breakdown command (redoIt).
//       It is called by Maya to undo.
//*********************************************************
MStatus CurveCleanerCommand::undoIt()
{
    MStatus status = MS::kSuccess;

    // Use the anim curve cache to undo
    if( !animCurveFnList.empty()) {
        animCurveListIter = animCurveFnList.begin();

        while( animCurveListIter != animCurveFnList.end() ) {
            // Be sure to skip any NULL pointers
            if( (*animCurveListIter).pAnimCache != NULL )
                    ((*animCurveListIter).pAnimCache)->undoIt();

            animCurveListIter++;
        }
    }

    return status;
}

//*********************************************************
// Name: newSyntax
// Desc: Method for registering the command flags
//       with Maya
//*********************************************************
MSyntax CurveCleanerCommand::newSyntax()
{
    MSyntax syntax;
    syntax.addFlag( tangentsFlag, tangentsLongFlag, MSyntax::kNoArg );
    syntax.addFlag( removeRedundantKeysFlag, removeRedundantKeysLongFlag, MSyntax::kNoArg );
    syntax.addFlag( splineStartEndFlag, splineStartEndLongFlag, MSyntax::kBoolean );
    syntax.addFlag( smoothnessFlag, smoothnessLongFlag, MSyntax::kDouble );
    syntax.addFlag( weightFactorFlag, weightFactorLongFlag, MSyntax::kDouble );
    syntax.addFlag( smoothAllSplinesFlag, smoothAllSplinesLongFlag, MSyntax::kBoolean );

    return syntax;
}

//*********************************************************
// Name: parseCommandFlags
// Desc: Parse the command flags and stores the values
//       in the appropriate variables
//*********************************************************
MStatus CurveCleanerCommand::parseCommandFlags(const MArgList &args)
{
    MStatus status = MS::kSuccess;

    // Create an object to parse the arguments
    // and then parse them
    MArgDatabase argData( syntax(), args, &status );
    if( !status ) {
        pluginError( "CurveCleanerCommand", "parseCommandFlags",
                     "Failed to create MArgDatabase for the set key command" );
    }
    else {
        if( argData.isFlagSet( tangentsFlag ))
            cleanTangents = true;
        if( argData.isFlagSet( removeRedundantKeysFlag ))
            removeRedundantKeys = true;

        
        if( argData.isFlagSet( splineStartEndFlag )) {
            bool startEndKeysToSpline;
            argData.getFlagArgument( splineStartEndFlag, 0, startEndKeysToSpline );
            if( startEndKeysToSpline )
                startEndTangentType = MFnAnimCurve::kTangentSmooth;
            else
                startEndTangentType = MFnAnimCurve::kTangentFlat;
        }

        if( argData.isFlagSet( smoothnessFlag ))
            argData.getFlagArgument( smoothnessFlag, 0, smoothingValue );
        if( argData.isFlagSet( smoothAllSplinesFlag ))
            argData.getFlagArgument( smoothAllSplinesFlag, 0, smoothAllSplines );
        if( argData.isFlagSet( weightFactorFlag ))
            argData.getFlagArgument( weightFactorFlag, 0, weightFactor );

        // Default to clean tangents if no cleaning flags provided
        if( cleanTangents == false && removeRedundantKeys == false )
            cleanTangents = true;

    }

    return status;
}

//*********************************************************
// Name: getSelectedObjects
// Desc: Generates a list of all the selected
//       objects
//*********************************************************
MStatus CurveCleanerCommand::getSelectedObjects()
{
    MStatus status = MS::kFailure;
    MSelectionList characterSetList;

    selectionList.clear();

    // Selected Objects Include:
    //    1) The Active Character Set (if in use) and its subset
    //    2) Character Sets selected by the user (and their subsets)
    //    3) Objects selected by the user

    // Get the character set if active (in Maya) and the associated character set subsets
    MStringArray characterSets;
    MGlobal::executeCommand( MString("cie_atbGetActiveCharacterSets"), characterSets, false, false );

    // Create a selection list with all the character set/subsets
    for( unsigned int i = 0; i < characterSets.length(); i++ ) {
        MGlobal::getSelectionListByName( characterSets[i], characterSetList );
    }

    // Get the character sets selected (by user) and the associated character set subsets 
    MGlobal::executeCommand( MString("cie_atbGetSelectedCharacterSets"), characterSets, false, false);

    // Append to the selection list with all the selected character set/subsets
    for( unsigned int i = 0; i < characterSets.length(); i++ ) {
        MGlobal::getSelectionListByName( characterSets[i], characterSetList );
    }

    // Retrive all of the currently selected objects
    if( !MGlobal::getActiveSelectionList( selectionList )) {
        pluginError( "CurveCleanerCommand", "getSelectedObjects", "Failed to get active selection list" );
    }
    // At least one object must be selected for this command
    else if( characterSetList.length() == 0 && selectionList.length() == 0 ) {
        pluginError( "CurveCleanerCommand", "getSelectedObjects", "No Objects Selected" );
        MGlobal::displayError( "No Objects Selected" );
    }
    else
        status = MS::kSuccess;

    selectionList.merge( characterSetList );

    return status;
}

//*********************************************************
// Name: getAnimCurveFnList
// Desc: Generates a list of anim curve function sets to
//       be operated on
//*********************************************************
MStatus CurveCleanerCommand::getAnimCurveFnList()
{
    MStatus status = MS::kSuccess;

    MObject dependNode;
    MPlugArray plugArray;

    // Create an iterator to traverse the selection list
    MItSelectionList sIter( selectionList, MFn::kInvalid, &status );
    if( !status ) {
        pluginError( "CurveCleanerCommand", "getAnimCurveFnList", "Failed to creation SL iterator" );
    }
    else {

        // Traverse all of the dependency nodes for the selected objects
        for( ; !sIter.isDone(); sIter.next() ) {
            // Get the current dependency node
            if( !sIter.getDependNode( dependNode )) {
                pluginError( "CurveCleanerCommand", "getAnimCurveFnList", "Couldn't get dependency node" );
                status = MS::kFailure;
                break;
            }

            // The call to get connections doesn't clear the array
            // so me must do it manually
            plugArray.clear();

            // Get all of the connections to this dependency node
            MFnDependencyNode dependFn( dependNode );
            if( !dependFn.getConnections( plugArray )) {
                // This object has no animation curves... no keys to worry about
                pluginTrace( "CurveCleanerCommand", "getAnimCurveFnList", "No keys on object" );
                continue;
            }

            // Generate the list from the plugs
            if( !getAnimCurveFnListFromPlugs( plugArray )) {
                pluginError( "CurveCleanerCommand", "getAnimCurveFnList", "Failed to create list from Plugs" );
                status = MS::kFailure;
                break;
            } 
        }
    }

    return status;
}

//*********************************************************
// Name: getAnimCurveFnListFromPlugs
// Desc: Generates a list of anim curve function sets from
//       an array of plugs
//*********************************************************
MStatus CurveCleanerCommand::getAnimCurveFnListFromPlugs( MPlugArray plugArray )
{
    MStatus status = MS::kSuccess;
    AnimCurveFnACC animCurveFnACC;

    // Check each connection for an upstream connections that is a kAnimCurve
    for( unsigned int index = 0; index < plugArray.length(); index++ )
    {
        if( plugArray[index].isKeyable() && !plugArray[index].isLocked() )
        {
            // Create an iterator that will exclusively traverse AnimCurve nodes
            MPlug currentPlug = plugArray[index];
            MItDependencyGraph dgIter( currentPlug,
                                       MFn::kAnimCurve,
                                       MItDependencyGraph::kUpstream,
                                       MItDependencyGraph::kBreadthFirst,
                                       MItDependencyGraph::kNodeLevel,
                                       &status );
            if( !status ) {
                pluginError( "CurveCleanerCommand", "getAnimCurveFnListFromPlugs", "DG Iterator error" );
            }
            else {
                // Retrieve the anim curve function sets
                // and store them in the list
                for( ; !dgIter.isDone(); dgIter.next() )
                {
                    MObjectArray nodePath;
                    dgIter.getNodePath( nodePath );
/*                  
                    // *** Original solution that didn't support Blend/Charater nodes ***                    
                    // Only use the anim curves directly connected to the attributes
                    // >1 on a breadth first search is level 2.  Root will always be
                    // last in path
                    if( nodePath.length() > 2 )
                        break;
*/

                    // At a depth of 1 in the DAG, the animation nodes are directly 
                    // connected to animated object.  However, if the depth is greater
                    // than one then we must accomodate both PairBlend nodes and 
                    // Character Set nodes (which sit between the transform node and
                    // the anim nodes.
                    int nodeParentIndex = 1;
                    if( nodePath.length() <= 2 || 
                        (nodePath.length() == 3 && 
                            (nodePath[nodeParentIndex].apiType() == MFn::kPairBlend ||
                            nodePath[nodeParentIndex].apiType() == MFn::kCharacter ))
                        )
                    {
                        MObject anim = dgIter.thisNode( &status );
                        MFnAnimCurve *animCurveFn = new MFnAnimCurve( anim, &status );

                        // Avoid adding duplicate anim curves to the list
                        // Important when dealing with blend nodes
                        MString curveName = animCurveFn->name();
                        bool nameExists = false;

                        animCurveListIter = animCurveFnList.begin();
                        while( animCurveListIter != animCurveFnList.end() ) {
                            if( ((AnimCurveFnACC)*animCurveListIter).pAnimCurveFn->name() == curveName ) {
                                nameExists = true;
                                break;
                            }
                            animCurveListIter++;
                        }

                        if( !nameExists ) {
                            if( !status ) {
                                pluginError( "CurveCleanerCommand", "getAnimCurveFnListFromPlugs", "Can't get AnimCurve function set" );
                                delete animCurveFn;
                            }
                            // If there are no problems, add the function set to the list
                            else {
                                animCurveFnACC.pAnimCurveFn = animCurveFn;
                                animCurveFnACC.pAnimCache = new MAnimCurveChange();
                                animCurveFnList.push_back( animCurveFnACC );
                            }
                        }
                    }
                }
            }
        }
    }

    return status;
}

//*********************************************************
// Name: removeRedundantKeysFromSelected
// Desc: Removes the keys from the selected objects'
//       animation curves that don't affect the curve shape
//*********************************************************
MStatus CurveCleanerCommand::getKeyTimes( MFnAnimCurve *pAnimCurveFn, 
                                         std::vector<MTime> *keyTimes )
{
    MStatus status = MS::kSuccess;

    // Remove all elements from the vector
    keyTimes->clear();

    MObject animCurveObj = pAnimCurveFn->object();
    MItKeyframe kfIter( animCurveObj, &status );
    if( !status ) {
        pluginError( "CurveCleanerCommand", "getKeyTimes", "Failed to get keyframe iterator" );  
    }

    for( kfIter.reset(); !kfIter.isDone(); kfIter.next() ) {
        keyTimes->push_back( kfIter.time() );
    }

    return status;
}

//*********************************************************
// Name: removeRedundantKeysFromSelected
// Desc: Removes the keys from the selected objects'
//       animation curves that don't affect the curve shape
//*********************************************************
MStatus CurveCleanerCommand::removeRedundantKeysFromSelected()
{
    MStatus status = MS::kSuccess;

    // Traverse each curve and remove redundant keys
    if( !animCurveFnList.empty()) {
        animCurveListIter = animCurveFnList.begin();

        while( animCurveListIter != animCurveFnList.end() ) {
            // Remove redundant keys from each curve
            status = removeRedundantKeysFromAnimCurve( ( *animCurveListIter ) );

            if( !status ) {
                pluginError( "CurveCleanerCommand", 
                             "removeRedundantKeysFromSelected", "Failed to remove keys from anim curve" );
                break;
            }

            animCurveListIter++;
        }
    }

    return status;
}

//*********************************************************
// Name: removeRedundantKeysFromAnimCurve
// Desc: Removes the keys from the anim curve that
//       don't affect the shape
//*********************************************************
MStatus CurveCleanerCommand::removeRedundantKeysFromAnimCurve( AnimCurveFnACC animCurveFnACC )
{
    MStatus status = MS::kSuccess;

    MFnAnimCurve *pAnimCurveFn = animCurveFnACC.pAnimCurveFn;

    // Get the time for each key on the anim curve
    std::vector<MTime> keyTimes( pAnimCurveFn->numKeys() );
    if( !(status = getKeyTimes( pAnimCurveFn, &keyTimes ) )) {
        pluginError( "CurveCleanerCommand", 
                     "removeRedundantKeysFromAnimCurve", "Failed to get key times" );
    }
    else {
       // Get rid of the redundant keys   
        double currentValue, prevValue, nextValue;
        
        // First and last keys will never be redundant
        // Minimun of 3 keys for a possible key removal
        if( keyTimes.size() > 2 ) {
            prevValue = getKeyValue( pAnimCurveFn, keyTimes[0] );
            
            // Start on the second key and finish on the 
            // second last key
            for( unsigned int i = 1; i < (keyTimes.size() - 1); i++ ) {
                currentValue = getKeyValue( pAnimCurveFn, keyTimes[i] );
                nextValue = getKeyValue( pAnimCurveFn, keyTimes[i+1] );

                if( (currentValue == prevValue) && (currentValue == nextValue) ) {
                    // Remove the current key when it is the same
                    // as the previous and next keys
                    if( !removeKey( animCurveFnACC, keyTimes[i] )) {
                        pluginError( "CurveCleanerCommand", 
                                     "removeRedundantKeysFromAnimCurve", "Failed to remove key" );
                    }
                }
                else {
                    // Only update the previous value if the
                    // current value was *not* removed
                    prevValue = currentValue;
                }
            }
        }
    }

    return status;
}

//*********************************************************
// Name: getKeyValue
// Desc: Returns the value of a key at a given time
//*********************************************************
double CurveCleanerCommand::getKeyValue( MFnAnimCurve *pAnimCurveFn, MTime time )
{
    unsigned int index = 0;

    if( !(pAnimCurveFn->find( time, index )))
        pluginError( "CurveCleanerCommand", 
                     "getKeyValue", "Failed to find the key index" );

    double value = pAnimCurveFn->value( index );

    if( pAnimCurveFn->animCurveType() == MFnAnimCurve::kAnimCurveTA ) {
        // Convert the value to degrees from radians
        MAngle angle( value, MAngle::kRadians );
        value = angle.asDegrees();
    }

    return value;
}

//*********************************************************
// Name: removeKey
// Desc: Removes a key at the given time
//*********************************************************
MStatus CurveCleanerCommand::removeKey( CurveCleanerCommand::AnimCurveFnACC animCurveFnACC, MTime time )
{
    MStatus status = MS::kSuccess;

    unsigned int index = 0;

    if( !(animCurveFnACC.pAnimCurveFn->find( time, index )))
        pluginError( "CurveCleanerCommand", 
                     "getKeyValue", "Failed to find the key index" );   

    status = animCurveFnACC.pAnimCurveFn->remove( index, animCurveFnACC.pAnimCache );

    if( status )
        numKeysRemoved++;

    return status;
}

//*********************************************************
// Name: cleanTangentsOnSelected
// Desc: Switches the tangents on peaks and valleys
//       to flat, while splining the remaining keys
//       for the anim curves on all selected objects
//*********************************************************
MStatus CurveCleanerCommand::cleanTangentsOnSelected()
{
    MStatus status = MS::kSuccess;

    // Traverse each curve and remove redundant keys
    if( !animCurveFnList.empty()) {
        animCurveListIter = animCurveFnList.begin();

        while( animCurveListIter != animCurveFnList.end() ) {
            // Remove redundant keys from each curve
            status = cleanTangentsOnAnimCurve( ( *animCurveListIter ) );

            if( !status ) {
                pluginError( "CurveCleanerCommand", 
                             "cleanTangentsOnSelected", "Failed to clean tangents on anim curve" );
                break;
            }
            
            numCurvesCleaned++;

            animCurveListIter++;
        }
    }

    return status;
}


//*********************************************************
// Name: cleanTangentsOnAnimCurve
// Desc: Switches the tangents on peaks and valleys
//       to flat, while splining the remaining keys
//       on an anim curve
//*********************************************************
MStatus CurveCleanerCommand::cleanTangentsOnAnimCurve( AnimCurveFnACC animCurveFnACC )
{
    MStatus status = MS::kSuccess;

    MFnAnimCurve *pAnimCurveFn = animCurveFnACC.pAnimCurveFn;

    // Get the time for each key on the anim curve
    std::vector<MTime> keyTimes( pAnimCurveFn->numKeys() );
    if( !(status = getKeyTimes( pAnimCurveFn, &keyTimes ) )) {
        pluginError( "CurveCleanerCommand", 
                     "cleanTangentsOnAnimCurve", "Failed to get key times" );
    }
    else {
        std::vector<bool> peakOrValley( keyTimes.size(), false );

        double currentValue, prevValue, nextValue, prevInequalValue, nextInequalValue;
        MAngle angleIn, angleOut;

        // First and last keys will be handed according to the tangent type flag
        if( keyTimes.size() > 0 ) {
            // Set the first tangent to spline
            updateTangents( animCurveFnACC, &keyTimes, 0, startEndTangentType, 0, 0, true, false );
            prevInequalValue = getKeyValue( pAnimCurveFn, keyTimes[0] );
        }
        if( keyTimes.size() > 1 ) {
            // Set the last tangent to spline
            updateTangents( animCurveFnACC, &keyTimes, 
                            (unsigned int)(keyTimes.size() - 1), startEndTangentType, 0, 0, true, false );
        }

        // Make a list that determines whether key is a peak or valley
        for( unsigned int i = 1; i < (keyTimes.size() - 1); i++ ) {
            prevValue = getKeyValue( pAnimCurveFn, keyTimes[i-1] );
            currentValue =  getKeyValue( pAnimCurveFn, keyTimes[i] );
            nextValue = getKeyValue( pAnimCurveFn, keyTimes[i+1] );

            // Keep track of last inequal values to determine if
            // the key is on a peak or a value
            if( currentValue != prevValue )
                prevInequalValue = prevValue;

            if( currentValue != nextValue )
                nextInequalValue = nextValue;
            else {
                // Find the next inequal value
                for( unsigned int j = i+2; j < (keyTimes.size()); j++ ) {
                    nextInequalValue = getKeyValue( pAnimCurveFn, keyTimes[j] );

                    // exit the loop when a new inequal value is found
                    if( nextInequalValue != currentValue )
                        break;
                }
            }

            if( (currentValue <= prevInequalValue && currentValue <= nextInequalValue) ||
                (currentValue >= prevInequalValue && currentValue >= nextInequalValue) )
            {
                peakOrValley[i] = true;
            }     
        }

        // Start on the second key and finish on the 
        // second last key
        for( unsigned int i = 1; i < (keyTimes.size() - 1); i++ ) {
            prevValue = getKeyValue( pAnimCurveFn, keyTimes[i-1] );
            currentValue =  getKeyValue( pAnimCurveFn, keyTimes[i] );
            nextValue = getKeyValue( pAnimCurveFn, keyTimes[i+1] );
            
            angleIn = getAngle( prevValue, currentValue, keyTimes[i-1].value(), keyTimes[i].value() );
            angleOut = getAngle( currentValue, nextValue, keyTimes[i].value(), keyTimes[i+1].value() );

            // Is the current key a valley or a peak, if so, the
            // tangent type will be set to flat

            if( peakOrValley[i] ) {
                updateTangents( animCurveFnACC, &keyTimes,
                                i, 
                                MFnAnimCurve::kTangentFlat,
                                angleIn, angleOut, true, false );
            }
            else {
                // Determine if the smoothing should be
                // applied to the spline.  If not in a smooth all
                // splines mode, only splines next to a peak or 
                // valley will be smoothed
                bool applySoftness = false;
                if( smoothAllSplines || peakOrValley[i-1] || peakOrValley[i+1] )
                    applySoftness = true;

                // Set the tangent type to spline
                updateTangents( animCurveFnACC, &keyTimes,
                                i,
                                MFnAnimCurve::kTangentSmooth,
                                angleIn, angleOut, false, applySoftness );
            }
                
        }
    }

    return status;
}

//*********************************************************
// Name: updateTangents
// Desc: Modifies the current tangent type and, if 
//       necessary, the tangent weight to avoid overshoots
//*********************************************************
MStatus CurveCleanerCommand::updateTangents( AnimCurveFnACC animCurveFnACC,
                                             std::vector<MTime> *keyTimes,
                                             unsigned int index,
                                             MFnAnimCurve::TangentType type,
                                             MAngle angleIn, MAngle angleOut,
                                             bool ignoreAngle,
                                             bool applySoftness )
{
    MStatus status = MS::kSuccess;

    MFnAnimCurve *pAnimCurveFn = animCurveFnACC.pAnimCurveFn;
    MAnimCurveChange *pAnimCache = animCurveFnACC.pAnimCache;

    // Update the tangent type for the key
    pAnimCurveFn->setInTangentType( index, type, pAnimCache );
    pAnimCurveFn->setOutTangentType( index, type, pAnimCache );

    // Handle the locked weights and tangents
    bool isTangentLocked = pAnimCurveFn->tangentsLocked( index );
    bool isWeightLocked = pAnimCurveFn->weightsLocked( index );
    if( isTangentLocked ) {
        pAnimCurveFn->setTangentsLocked( index, false, pAnimCache );
    }
    if( isWeightLocked ) {
        pAnimCurveFn->setWeightsLocked( index, false, pAnimCache );
    }

    // The smallest angle is used to avoid overshoots
    MAngle tangentAngle;
    if( fabs(angleOut.asRadians()) > fabs(angleIn.asRadians()) )
        tangentAngle = angleIn;
    else
        tangentAngle = angleOut;

    // Adjust angle for smoothing if necessary
    if( !ignoreAngle && applySoftness ) {
        // Add in the smoothing value (softness)
        double radianDiff = fabs( angleOut.asRadians() - angleIn.asRadians() );
        double softness = radianDiff * smoothingValue;

        // Add or subtract the softness depending on positive or negative slope
        if( angleIn.asRadians() > angleOut.asRadians() ) {
            // Positive Slope
            if( angleIn.asRadians() > 0 || (angleIn.asRadians() == 0 && angleOut.asRadians() > 0) )
                tangentAngle = MAngle( tangentAngle.asRadians() + softness );
            // Negative Slope
            else
                tangentAngle = MAngle( tangentAngle.asRadians() - softness );
        }
        else {
            // Positive Slope
            if( angleIn.asRadians() > 0 || (angleIn.asRadians() == 0 && angleOut.asRadians() > 0) )
                tangentAngle = MAngle( tangentAngle.asRadians() + softness );
            // Negative Slope
            else
                tangentAngle = MAngle( tangentAngle.asRadians() - softness );
            
        }

        // Set the new angle for the tangent
        pAnimCurveFn->setAngle( index, tangentAngle, true, pAnimCache );
        pAnimCurveFn->setAngle( index, tangentAngle, false, pAnimCache );

        double deltaTime = 0.0, deltaValue = 0.0, newWeight; 
        
        // Update the in-tangent weight
        if( index != 0 ) {
            deltaTime = (((*keyTimes)[index]).value() - ((*keyTimes)[index-1]).value());
            deltaValue = pAnimCurveFn->value( index ) - pAnimCurveFn->value( index - 1 );
            newWeight =  ( deltaTime / cos( tangentAngle.asRadians() )) * weightFactor;

            pluginTrace( "CurveCleanerCommand", "updateTangents", MString("In Weight: " ) + newWeight );

            pAnimCurveFn->setWeight( index, newWeight, true, pAnimCache );
        }
        if( index < ((unsigned int)keyTimes->size() - 1)) {
            deltaTime = (((*keyTimes)[index+1]).value() - ((*keyTimes)[index]).value());
            deltaValue = pAnimCurveFn->value( index+1 ) - pAnimCurveFn->value( index );
            newWeight =  ( deltaTime / cos( tangentAngle.asRadians() )) * weightFactor;

            pAnimCurveFn->setWeight( index, newWeight, false, pAnimCache );

            pluginTrace( "CurveCleanerCommand", "updateTangents", MString("Out Weight: " ) + newWeight );
        }
    }

    if( isTangentLocked ) {
        pAnimCurveFn->setTangentsLocked( index, true, pAnimCache );
    }
    if( isWeightLocked ) {
        pAnimCurveFn->setWeightsLocked( index, true, pAnimCache );
    }

    return status;
}

//*********************************************************
// Name: getAngle
// Desc: Returns the angle for two points on a curve
//*********************************************************
MAngle CurveCleanerCommand::getAngle( double value1 , double value2,
                                      double time1, double time2 )
{
    double valueDelta = value2 - value1;
    double timeDelta = time2 - time1;

    double radians = atan( valueDelta / timeDelta );

    return MAngle( radians, MAngle::kRadians ); 

}