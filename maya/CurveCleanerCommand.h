//*********************************************************
// CurveCleanerCommand.h
//
// Copyright (C) 2007-2021 Skeletal Studios
// All rights reserved.
//
//*********************************************************

#ifndef __CURVE_CLEANER_COMMAND_H_
#define __CURVE_CLEANER_COMMAND_H_

//*********************************************************
#include <maya/MPxCommand.h>

#include <maya/MGlobal.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MAngle.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MAnimCurveChange.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnAnimCurve.h>

#include <maya/MItSelectionList.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MItKeyframe.h>

#include <list>
#include <vector>
#include <math.h>

//*********************************************************

//*********************************************************
// Class: CurveCleanerCommand
//
// Desc: Performs a number of operations to clean up
//       the animation curves associated with the selected
//       objects
//
// Command: cieCleanCurves
//
// Flags: -tangents (-t)
//
//        -removeRedundantKeys (-rrk)
//
//        -splineStartEnd (-sse)    (boolean)
//
//        -smoothness (-s)      (double)
//
//        -smoothAllSplines (-sas)    (boolean)
//
//        -weightFactor (-wf)       (double)
//
//*********************************************************
class CurveCleanerCommand : public MPxCommand
{
private:
    // Storage for the anim curve function set and its
    // curve change cache (for undo/redo)
    struct AnimCurveFnACC {
        MFnAnimCurve* pAnimCurveFn;
        MAnimCurveChange* pAnimCache;
    };

    // Command flag constants
    static const char *tangentsFlag, *tangentsLongFlag;
    static const char *removeRedundantKeysFlag, *removeRedundantKeysLongFlag;
    static const char *splineStartEndFlag, *splineStartEndLongFlag;
    static const char *smoothnessFlag, *smoothnessLongFlag;
    static const char *weightFactorFlag, *weightFactorLongFlag;
    static const char *smoothAllSplinesFlag, *smoothAllSplinesLongFlag;

    // Indicates that tangents should be updated
    // Flatten peaks and valleys and spline w/o overshoot
    // for all other keys
    bool cleanTangents;

    // Indicates that all keys that don't affect the
    // shape of the curve should be removed
    bool removeRedundantKeys;

    // The tangent type to set the start and end keys to
    MFnAnimCurve::TangentType startEndTangentType;

    // The smoothing value applied to tangent angles
    double smoothingValue;

    // indicates whether or not to apply the softness value
    // to all spline tangents or just those on keys immediately
    // before and after a peak/valley
    bool smoothAllSplines;

    // The weighting factor applied to tangents that
    // are not locked
    double weightFactor;

    // The number of redundant keys removed
    unsigned int numKeysRemoved;

    // The number of curves cleaned
    unsigned int numCurvesCleaned;

    // The objects currently selected in the Maya scene
    MSelectionList selectionList;

    // Indicates that the anim curve caches have been
    // calculated for undo/redo
    bool initialized;

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

    // Finds all of the times of keys on a given anim curve
    MStatus getKeyTimes( MFnAnimCurve *pAnimCurveFn, std::vector<MTime> *keyTimes );

    // Returns the value of a key at a given time
    double getKeyValue( MFnAnimCurve *pAnimCurveFn, MTime time );

    // Removes a key at the given time
    MStatus removeKey( AnimCurveFnACC animCurveFnACC, MTime time );

    // Removes the keys from the selected objects'
    // animation curves that don't affect the curve shape
    MStatus removeRedundantKeysFromSelected();

    // Removes the keys from the anim curve that
    // don't affect the shape
    MStatus removeRedundantKeysFromAnimCurve( AnimCurveFnACC animCurveFnACC );

    // Switches the tangents on peaks and valleys
    // to flat, while splining the remaining keys
    // for the anim curves on all selected objects
    MStatus cleanTangentsOnSelected();

    // Switches the tangents on peaks and valleys
    // to flat, while splining the remaining keys
    // on an anim curve
    MStatus cleanTangentsOnAnimCurve( AnimCurveFnACC animCurveFnACC );

    // Modifies the current tangent type and, if necessary,
    // the tangent weight to avoid overshoots
    MStatus updateTangents( AnimCurveFnACC animCurveFnACC,
                            std::vector<MTime> *keyTimes,
                            unsigned int index,
                            MFnAnimCurve::TangentType type,
                            MAngle angleIn, MAngle angleOut,
                            bool ignoreAngle,
                            bool applySoftness );

    // Returns the slope for two points on a curve
    MAngle getAngle( double value1, double value2, double time1, double time2 );

public:
    // Constructor/Destructor
    CurveCleanerCommand();
    ~CurveCleanerCommand();

    // Performs the command
    virtual MStatus doIt( const MArgList &args );

    // Performs the work that changes Maya's internal state
    virtual MStatus redoIt();

    // Undoes the changes to Maya's internal state
    virtual MStatus undoIt();

    // Indicates that Maya can undo/redo this command
    virtual bool isUndoable() const { return true; }

    // Allocates a command object to Maya (required)
    static void *creator() { return new CurveCleanerCommand; }

    // Defines the set of flags allowed by this command
    static MSyntax newSyntax();
};







#endif // __CURVE_CLEANER_COMMAND_H_