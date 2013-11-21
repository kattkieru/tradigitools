//*********************************************************
// Breakdown.h
//
// Copyright (C) 2007 Circus Ink Entertainment
// All rights reserved.
//
//*********************************************************

#ifndef __BREAKDOWN_H_
#define __BREAKDOWN_H_

//*********************************************************
#include <maya/MGlobal.h>
#include <maya/MTime.h>
#include <maya/MAnimControl.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MAnimCurveChange.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnNumericData.h>

#include <maya/MFnIntArrayData.h>
#include <maya/MFnUInt64ArrayData.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
//*********************************************************

//*********************************************************
// Class: Breakdown
//
// Desc:  Provides the ability to create a new breakdown
//        for an animatable attribute.
//*********************************************************
class Breakdown
{
public: 
    // The types of breakdown modes available
    enum BreakdownMode {
         kOverwrite,      
         kRipple,
    };

private:
    // The frame/time where this breakdown was created
    MTime breakdownTime;

    // The frame/time of the playhead when command was called
    MTime originalPlayheadTime;

    // The value to be used when setting the new breakdown
    double breakdownValue;

    // The weighting to favour either the previous key (0.0)
    // or next key (1.0).  0.5 is the midpoint.
    double breakdownWeight;

    // The mode used to create this breakdown
    BreakdownMode breakdownMode; 

    // Use the alternate color (green) for the tick color
    bool keyTickDrawSpecial;

    // The value of keyTickDraw special when the command is undone
    bool undoKeyTickDrawSpecial;

    // The function set for the curve this key is set on
    MFnAnimCurve fnAnimCurve;

    // The value of the original key (if it existed)
    double originalKeyValue;

    // The index of the original key (-1 if no key exists)
    int originalKeyIndex;

    // The index of the previous key on the curve (-1 if no key exists)
    int previousKeyIndex;

    // The index of the next key on the curve (-1 if no key exists)
    int nextKeyIndex;

    // Indicates that the attribute should be treated as a boolean
    bool isBooleanAttr;

    // The index of the new breakdown (-1 if no key exists)
    int breakdownIndex;

    // The number of keys set on the curve
    unsigned int numKeys;

    // The closest key index to the breakdown time
    unsigned int closestIndex;

    // The time for the closest key to the breakdown time
    MTime closestTime;

    // A unique number to relate attributes on a object
    unsigned int objID;

    // A string representing the error that occurred
    MString errorMsg;

    // When initialized redo methods will use the cached
    // animation curve data instead of recalulating each
    // time.
    bool initialized;

    // Anim curve caching to simplify the process of undo/redo
    MAnimCurveChange animCache;

    // Status
    MStatus breakdownStatus;
 
    // Determines if there is a key set at the current time
    // and extracts the details (if it exists).  Must be called
    // before evaluatePreviousKey and evaluateNextKey.
    bool evaluateOriginalKey();

    // Determines if there is a key set prior to the current time
    // and extracts the details (if it exists).  In ripple mode
    // the originalKey (if it exists) will be used as the previous key. 
    bool evaluatePreviousKey();

    // Determines if there is a key set after the current time
    // and extracts the dtails (if it exists).
    bool evaluateNextKey();

    // Redo when in overwrite mode
    void redoOverwrite();

    // Undo when in overwrite mode
    void undoOverwrite();

    // Redo when in ripple mode
    void redoRipple();

    // Undo when in ripple mode
    void undoRipple();

    // Calculates the value of the new breakdown
    void calcNewBreakdownValue();

    // Sets the special drawing value for the timeline ticks
    // isUndo will restore the previous state
    MStatus setTickDrawSpecial( bool isUndo = false );


public:

    // Constructor
	// 2010 edit: doesn't compile under current XCode
	//    Breakdown::Breakdown( MFnAnimCurve &animCurve,  // Anim Curve Function Set 
	Breakdown( MFnAnimCurve &animCurve,  // Anim Curve Function Set 
						double weight,            // The weighting of the breakdown
						BreakdownMode mode,       // Ripple or Overwrite mode
                        bool tickDrawSpecial,     // Use the alternate tick color
                        MTime time,               // The time to set the breakdown
                        bool isBoolean,           // Indicate if attribute is a boolean
                        unsigned int id,          // Group Id of this breakdown
                        MStatus* status = 0 );
    // Destructor
	// 2010 edit: doesn't compile under current XCode
	//Breakdown::~Breakdown();

	~Breakdown();
	
    // Method to undo the changes to Maya's state when
    // creating the breakdown.
    MStatus undoIt();

    // Method to redo the changes to Maya's state when
    // creating the breakdown.
    MStatus redoIt();

    // Returns true if a key exists at the current time (before setting the breakdown)
    bool hasOriginalKey() const { return originalKeyIndex > -1 ? true : false; }

    // Returns true if a key exists before the current time
    bool hasPreviousKey() const { return previousKeyIndex > -1 ? true : false; }

    // Returns true if a key exists after the current time
    bool hasNextKey() const     { return nextKeyIndex > -1 ? true : false; }

    // Returns the id assigned when the breakdown was created
    unsigned int getObjId() const { return objID; }

    // Returns an error message if the breakdown creation failed
    MString getErrorMsg() const { return errorMsg; }

    const MFnAnimCurve& getAnimCurveFn() const { return fnAnimCurve; }
};

#endif