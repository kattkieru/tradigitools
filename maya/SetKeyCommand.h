//*********************************************************
// SetKeyCommand.h
//
// Copyright (C) 2007-2021 Skeletal Studios
// All rights reserved.
//
//*********************************************************

#ifndef __SET_KEY_COMMAND_H_
#define __SET_KEY_COMMAND_H_

//*********************************************************
#include <maya/MPxCommand.h>

#include <maya/MGlobal.h>
#include <maya/MTime.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MAnimControl.h>
#include <maya/MAnimCurveChange.h>
#include <maya/MSelectionList.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnAnimCurve.h>

#include <maya/MItSelectionList.h>
#include <maya/MItDependencyGraph.h>

#include <list>
//*********************************************************

//*********************************************************
// Class: SetKeyCommand
//
// Desc: Creates a new MEL command that sets new keys
//       and can edit the value of keys
//
// Command: cieSetKey
//
// Flags: -edit (-e)
//
//        -ignoreUnkeyed (-iuk)        (boolean)
//
//        -tickDrawSpecial (-tds)      (boolean)
//
//*********************************************************
class SetKeyCommand : public MPxCommand
{
private:
    // Storage for the anim curve function set and its
    // previous TDS state (for undo purposes)
    struct AnimCurveFnTDS {
        MFnAnimCurve* pAnimCurveFn;
        MAnimCurveChange* pAnimCurveCache;
        bool previousTickDrawSpecial;
        bool isBoolean;
    };

    // Constants for setting up the command's flags
    static const char *editFlag, *editLongFlag;
    static const char *ignoreUnkeyedFlag, *ignoreUnkeyedLongFlag;
    static const char *tickDrawSpecialFlag, *tickDrawSpecialLongFlag;

    // Update existing keys
    bool editMode;

    // Don't create a key on attributes that don't have
    // any keys set
    bool ignoreUnkeyed;

    // Use the special drawing state for the keys
    bool tickDrawSpecial;

    // The objects currently selected in the Maya scene
    MSelectionList selectionList;

    // The time of the playhead when this object was created
    MTime originalPlayheadTime;

    // The number of tickDrawSpecial attributes affected
    unsigned int tickDrawSpecialCount;

    // When initialized, redoIt will use the animCurve cache
    bool initialized;

    // The list of all anim curves/TDS for the selected objects
    std::list<AnimCurveFnTDS> animCurveFnList;

    // Iterator for the anim curve function set list
    std::list<AnimCurveFnTDS>::iterator animCurveListIter;

    // Method to setup the command flags
    MStatus parseCommandFlags( const MArgList &args );

    // Generates a list of all the selected objects/attributes
    MStatus getSelectedObjects();

    // Generates a list of all the anim curves for the
    // selected objects
    MStatus getAnimCurveFnList();

    // Generates a list of anim curves from an array of plugs
    MStatus getAnimCurveFnListFromPlugs( MPlugArray plugArray );

    // Sets a new key for each attribute (subject to the flags)
    // at the current time
    MStatus setKeys();

    // Sets the special drawing value for the timeline ticks
    // isUndo will restore the previous state
    MStatus setTickDrawSpecial( bool undo = false );

    // Returns the logical index of the key at the time 
    // when the command was called
    int getKeyLogicalIndex( MFnAnimCurve &animCurveFn, MStatus *status );

    // Determines if the current plug is a boolean data type
    bool isBooleanDataType( const MPlug &plug );

public:
    // Constructor/Destructor
    SetKeyCommand();
    ~SetKeyCommand();

    // Performs the command
    virtual MStatus doIt( const MArgList &args );

    // Performs the work that changes Maya's internal state
    virtual MStatus redoIt();

    // Undoes the changes to Maya's internal state
    virtual MStatus undoIt();

    // Indicates that Maya can undo/redo this command
    virtual bool isUndoable() const { return true; }

    // Allocates a command object to Maya (required)
    static void *creator() { return new SetKeyCommand; }

    // Defines the set of flags allowed by this command
    static MSyntax newSyntax();
};


#endif //__SET_KEY_COMMAND_H_