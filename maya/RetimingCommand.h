//*********************************************************
// RetimingCommand.h
//
// Copyright (C) 2007 Circus Ink Entertainment
// All rights reserved.
//
//*********************************************************

#ifndef __RETIMING_COMMAND_H_
#define __RETIMING_COMMAND_H_

//*********************************************************
#include <maya/MPxCommand.h>

#include <maya/MGlobal.h>
#include <maya/MTime.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MAnimControl.h>
#include <maya/MAnimCurveChange.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnAnimCurve.h>

#include <maya/MItSelectionList.h>
#include <maya/MItDependencyGraph.h>

#include <list>
//*********************************************************

//*********************************************************
// Class: RetimingCommand
//
// Desc: Creates a new MEL command adjusts the current
//       timing between selected keys
//
// Command: cieRetiming
//
// Flags: -query (-q)
//
//        -relative (-rel)      (boolean)
//
//        -delta (-d)      (int)
//
//        -nextKeyOnComplete (-nkc)  (boolean)
//
//*********************************************************
class RetimingCommand : public MPxCommand
{
private:
    // Storage for the anim curve function set and its
    // curve change cache (for undo/redo)
    struct AnimCurveFnACC {
        MFnAnimCurve* pAnimCurveFn;
        MAnimCurveChange* pAnimCache;
    };

    // Constants for setting up the command's flags
    static const char *relativeFlag, *relativeLongFlag;
    static const char *deltaFlag, *deltaLongFlag;
    static const char *nextKeyOnCompleteFlag, *nextKeyOnCompleteLongFlag;

    // Indicates if the command is in query mode
    bool queryMode;

    // Indicates whether retiming delta should be treated as
    // an absolute or relative value
    bool relativeMode;

    // The change to the current timing between keys
    // Abs/Rel determined by the relative flag
    int timingDelta;

    // Determines where the playhead will be placed after
    // retiming.  If this is true, it is moved to the last
    // key in the current retiming strip, if false, it will
    // move to the first key
    bool nextKeyOnComplete;

    // The objects currently selected in the Maya scene
    MSelectionList selectionList;

    // Indicates that the anim curve caches have been
    // calculated for undo/redo
    bool initialized;

    // The first frame in the range
    MTime rangeStartTime;

    // The last frame in the range
    MTime rangeEndTime;

    // The original playhead time (needed for undo)
    MTime origPlayheadTime;

    // The new playhead time
    MTime newPlayheadTime;

    // The number of keys retimed
    unsigned int numRetimed;

    // The current strip string returned in query mode
    MString stripString;

    // The list of all anim curves/cache for the selected objects
    std::list<AnimCurveFnACC> animCurveFnList;

    // Iterator for the anim curve function set list
    std::list<AnimCurveFnACC>::iterator animCurveListIter;


    // Method to setup the command flags
    MStatus parseCommandFlags( const MArgList &args );

    // Generates a list of all the selected objects/attributes
    MStatus getSelectedObjects();

    // Generates a list of all the anim curves for the
    // selected objects
    MStatus getAnimCurveFnList();

    // Generates a list of anim curves from an array of plugs
    MStatus getAnimCurveFnListFromPlugs( MPlugArray plugArray );

    // Determines the range (from the time slider) over which
    // retiming will occur
    MStatus getRange();

    // Traverses all keyframes on all curves in the given 
    // range and retimes them
    MStatus retime();

    // Traverses all keyframes in the given range and retimes
    // them
    MStatus retimeAnimCurve( AnimCurveFnACC &animCurveACC );

    // Traverses all keyframes in the given range and retimes
    // them
    MStatus retimeAnimCurve( AnimCurveFnACC &animCurveACC, 
                             unsigned int currentIndex,
                             unsigned int lastRetimingIndex,
                             double prevIndexOrigTime,
                             double prevIndexNewTime );

    // Shift the keys that fall after the retiming range by
    // the specified number of frames
    MStatus shiftRemainingKeys( AnimCurveFnACC &animCurveACC,
                                unsigned int firstKeyIndex,
                                double numFrames );

    // Creates the strip string. The info related to the
    // current timing
    MStatus generateStripString( MFnAnimCurve &animCurve,
                                 unsigned int firstRetimingIndex,
                                 unsigned int lastRetimingIndex );

public:
    // Constructor/Destructor
    RetimingCommand();
    ~RetimingCommand();

    // Performs the command
    virtual MStatus doIt( const MArgList &args );

    // Performs the work that changes Maya's internal state
    virtual MStatus redoIt();

    // Undoes the changes to Maya's internal state
    virtual MStatus undoIt();

    // Indicates that Maya can undo/redo this command
    virtual bool isUndoable() const { return (queryMode ? false : true); }

    // Allocates a command object to Maya (required)
    static void *creator() { return new RetimingCommand; }

    // Defines the set of flags allowed by this command
    static MSyntax newSyntax();

};

#endif