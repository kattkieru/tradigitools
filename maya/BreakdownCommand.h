//*********************************************************
// BreakdownCommand.h
//
//
//*********************************************************

#ifndef __BREAKDOWN_COMMAND_H_
#define __BREAKDOWN_COMMAND_H_

//*********************************************************
#include <maya/MPxCommand.h>

#include <maya/MSelectionList.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MAnimControl.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MObjectArray.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnNumericAttribute.h>

#include <maya/MItSelectionList.h>
#include <maya/MItDependencyGraph.h>

#include "BreakdownList.h"
//*********************************************************

//*********************************************************
// Class: BreakdownCommand
//
// Desc:  Creates a new MEL command that inserts 
//        breakdowns, for keyframable attributes, on
//        selected objects.
//
// Command: cieInsertBreakdown [flags]
//
// Flags: -weight (-w)       (double)
//        Specifies the weighting of the breakdown between
//        the key prior to and after the current time.
//        0.5 is the default.
//
//        -selectedAttr (-sa)       (boolean)
//        Only attributes that are highlighted in the
//        channelBox will have breakdowns set.
//        false is the default
//
//        -mode (-m)        (string)
//        Indicates whether the breakdown will be set in
//        ripple ("ripple") or overwrite ("overwrite") mode
//        "overwrite" is the default.
//
//        -invalidAttrOpFlag (-iao)      (string)
//        Dictates how the command will proceed if an
//        attribute cannot have a key set on it.
//        "skipAll" - (default)The command will fail.
//                    No breakdowns will be set.
//        "skipObject" - No breakdowns will be set on the
//                       object with the invalid attribute
//        "skipAttr" - No breakdowns will be set on the
//                     invalid attributes.  All others
//                     will have a breakdown set.
//
//        -ignoreRippleCheck (-irc)     (boolean)
//        This check verifies that, when in ripple mode,
//        if a single key exists at the current time, all 
//        attributes must have a key at the current time.
//        This is to prevent a partial ripple where keys
//        that don't exist are created at the current
//        time and keys that do exist are rippled forward.
//
//        -tickDrawSpecial (-tds)       (boolean)
//        Sets the special drawing state for the breakdowns
//        when it is drawn in as a tick in the timeline.
//
//*********************************************************
class BreakdownCommand : public MPxCommand
{
private:
    // The types of invalid attribute operations that
    // can be performed
    enum InvalidAttrOpType {
        kSkipAll,
        kSkipObject,
        kSkipAttr
    };

    // The current status
	MStatus status;

    // A list containing breakdown information for each
    // attibute that is deemed "valid" and can have a
    // breakdown set at the current time
    BreakdownList breakdownList;

    // The current frame/time when this command was called
    MTime currentAnimationFrame;

    // The weighting of the breakdown (favour the previous or next key)
    double breakdownWeight;

    // Set breakdowns on selected attributes flag
    bool selectedAttrOnly;

    // Ignore the ripple mode test for uniform keys
    bool ignoreRippleCheck;

    // Determines how to proceed after finding an attribute that
    // cannot have a breakdown set on it at the current time
    InvalidAttrOpType invalidAttrOp;

    // The mode for setting breakdowns (overwrite or ripple)
    Breakdown::BreakdownMode breakdownMode;

    // Use the special drawing state for the breakdowns
    bool tickDrawSpecial;

    // Constants for setting up the command's flags
    static const char *weightFlag, *weightLongFlag;
    static const char *selectedAttrFlag, *selectedAttrLongFlag;
    static const char *modeFlag, *modeLongFlag;
    static const char *invalidAttrOpFlag, *invalidAttrOpLongFlag;
    static const char *ignoreRippleCheckFlag, *ignoreRippleCheckLongFlag;
    static const char *tickDrawSpecialFlag, *tickDrawSpecialLongFlag;

    // The objects currently selected in the Maya scene
    MSelectionList selectionList;

    // The attributes currently selected in the channel box
    MStringArray selectedAttributeList;

    // Flags to indicate what has been skipped if an invalid attibute
    // was encountered
    bool attributesSkipped;
    bool objectsSkipped;

    // Populates selectionList with the objects selected in the scene
	void getSelectedObjects();

    // Creates a list of selected attributes from the Maya channel box
    unsigned int populateSelectedAttributeList();

    // Determines if a string is on the selectedAttributeList
    bool isStrOnSelectedAttrList( const MString &str );

    // Determine if enough keyframes have been set for an inbetween
    void checkKeyframes( MFnAnimCurve &animCurve );

    // Generate the list of breakdowns to be inserted
    MStatus createBreakdownList();

    // Process the list of plugs to find animation.
    // Creates appropriate Breakdowns and adds them to the list.
    MStatus processConnections( MPlugArray &connections, unsigned int objID, MString objName );

    // Determine if the current attribute is a boolean
    bool isBooleanDataType( const MPlug &connection );

    // Determine if the current attribute is an enum
    bool isEnumDataType( const MPlug &connection );

    // Method to retrive the command flag values
    void parseCommandFlags( const MArgList &args );


public:
    // Constructor/Destructor
	BreakdownCommand();
	~BreakdownCommand();

    // Performs the breakdown command
	virtual MStatus doIt( const MArgList &args );

    // Performs the work that changes Maya's internal state
    virtual MStatus redoIt();

    // Undoes the changes to Maya's internal state made in redoIt.
    virtual MStatus undoIt();

    // Indicates that Maya can undo/redo this command
    virtual bool    isUndoable() const { return true; }
	
    // Alocates a command object to Maya (required)
	static void *creator() { return new BreakdownCommand; }

    // Defines the set of flags allowed by this command
    static MSyntax newSyntax();
};



#endif