//*********************************************************
// ShotMaskCommand.h
//
// Copyright (C) 2007 Circus Ink Entertainment
// All rights reserved.
//
//*********************************************************

#ifndef __SHOT_MASK_COMMAND_
#define __SHOT_MASK_COMMAND_

//*********************************************************
#include <maya/MPxCommand.h>

#include <maya/MGlobal.h>
#include <maya/MSyntax.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MSelectionList.h>
#include <maya/MObject.h>
#include <maya/MTime.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MAnimControl.h>
#include <maya/MObjectArray.h>
#include <maya/MIntArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>

#include <maya/MFnCamera.h>
#include <maya/MFnDependencyNode.h>

#include <maya/MItSelectionList.h>
#include <maya/MItDependencyGraph.h>

#include <math.h>
//*********************************************************

//*********************************************************
// Class: ShotMaskCommand
//
// Desc: Creates an overlay in 3D space that is used
//       to display a timecode and current shot details
//
// Command: cieShotMask
//
// Flags: -query (-q)
//
//        -camera (-c)    (string)
//
//        -aspectRatio (-ar)    (double)
//
//        -cleanScene (-cs)   (boolean)
//
//        -maskThickness (-mt) (double)
//
//        -title (-t)   (string)
//
//        -text1 (-t1)    (string)
//        
//        -text2 (-t2)    (string)
//
//        -keyType (-kt) (query only)
//
//        -frameDigits (-fd) (query only)
//
//*********************************************************
class ShotMaskCommand : public MPxCommand
{
private:
    // Constants for setting up the command's flags
    static const char *cameraFlag, *cameraLongFlag;
    static const char *aspectRatioFlag, *aspectRatioLongFlag;
    static const char *cleanSceneFlag, *cleanSceneLongFlag;
    static const char *frameDigitsFlag, *frameDigitsLongFlag;
    static const char *keyTypeFlag, *keyTypeLongFlag;
    static const char *maskThicknessFlag, *maskThicknessLongFlag;

    static const char *titleFlag, *titleLongFlag;
    static const char *text1Flag, *text1LongFlag;
    static const char *text2Flag, *text2LongFlag;

    // Constants for the shot mask nodes and elements
    // Group Nodes
    static const char *mainGrpName;
    static const char *frameCounterGrpName;

    static const char *titleTextGrpName;
    static const char *bottomLeftTextGrpName;
    static const char *bottomRightTextGrpName;

    // Shader Nodes & Groups
    static const char *borderShaderNodeName;
    static const char *borderShaderGroupName;
    static const char *textShaderNodeName;
    static const char *textShaderGroupName;
    static const char *keyIconShaderNodeName;
    static const char *keyIconShaderGroupName;
    static const char *bdIconShaderNodeName;
    static const char *bdIconShaderGroupName;
    static const char *ltbxShaderNodeName;
    static const char *ltbxShaderGroupName;

    // Expressions
    static const char *shotMaskExprName;

    // Determines if this is only querying information
    bool queryMode;
    
    // Aspect ratio for the overlay
    double aspectRatio;

    // Aspect ratio in render globals
    double renderAspectRatio;

    // Aspect ratio for the film back
    double filmAspectRatio;

    // clean scene flag
    bool cleanScene;

    // Set if the keyType is being queried
    bool queryKeyType;

    // Query for the visible digits in each column
    bool queryDigits;

    // The resultant string to be returned
    MString resultStr;

    // The title of the shot mask
    MString shotMaskTitle;

    // Additional text variables
    MString shotMaskText1, shotMaskText2;

    // The thickness of the border as a fraction of
    // the total width/height
    double maskThickness;

    // The min/max thickness values allowed for the shot mask
    double minThickness;
    double maxThickness;

    // The font used on the shot mask
    MString font;

    // List of objects to perform command on
    MSelectionList objList;

    // Function set for the camera getting the overlay
    MFnCamera *pCameraFn;

    // The camera transform node name
    MString camTransNodeName;

    // The camera shape node name
    MString camShapeNodeName;

    // Method to setup the command flags
    MStatus parseCommandFlags( const MArgList &args );

    // Retrieves the camera shape node from the given transform node
    MStatus getCameraShapeNode();

    // Creates the shot mask for a specified camere
    MStatus createShotMask();

    // Create the shot mask shaders
    MStatus createShotMaskShaders();

    // Creates the digits 0-9 for a counter
    MStatus createFrameCounter( double scaleVal, 
                                double digitZPos,
                                double borderThickness,
                                double hEdgeTrans,
                                double vEdgeTrans );

    // Creates text for the shot mask
    MStatus createText( MString name, MString grpNodeName, MString text, MString font );

    // Creates the expression to update the shot mask
    MStatus createShotMaskExpr();

    // Deletes all shot mask elements from the current scene
    MStatus cleanUpShotMask();

    // Converts the current frame into an int array where
    // index 0 represents the 1s column, index 2 represents
    // the 10s column, etc...
    MStatus generateFrameDigitArray();

    // Finds the key type at the current frame and sets
    // the result to the type "none", "key", "breakdown"
    MStatus getKeyType();

    // Finds the key type at the current frame and sets
    // the result to the type "none", "key", "breakdown"
    MStatus getKeyTypeFromPlugArray( MPlugArray plugArray );

    // Finds the key type on a given anim curve
    MStatus getKeyTypeFromCurve( MFnAnimCurve &animCurveFn );

    // Returns the logical index of a key at the 
    // current time
    int getKeyLogicalIndex( MFnAnimCurve &animCurveFn, MStatus *status );

public:
    // Constructor/Destructor
    ShotMaskCommand();
    ~ShotMaskCommand();

    // Performs the command
    virtual MStatus doIt( const MArgList &args );

    // Performs the work that changes Maya's internal state
    virtual MStatus redoIt();

    // Undoes the changes to Maya's internal state
    virtual MStatus undoIt();

    // Indicates that Maya can undo/redo this command
    virtual bool isUndoable() const { return queryMode ? false : true; }

    // Allocates a command object to Maya (required)
    static void *creator() { return new ShotMaskCommand; }

    // Defines the set of flags allowed by this command
    static MSyntax newSyntax();

};


#endif //__SHOT_MASK_COMMAND_