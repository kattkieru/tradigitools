//*********************************************************
// ShotMaskCommand.cpp
//
// Copyright (C) 2007 Circus Ink Entertainment
// All rights reserved.
//
//*********************************************************

//*********************************************************
#include "ShotMaskCommand.h"
#include "ErrorReporting.h"
//*********************************************************

//*********************************************************
// Constants
//*********************************************************

// Command Flags
const char *ShotMaskCommand::cameraFlag = "-cam";
const char *ShotMaskCommand::cameraLongFlag = "-camera";
const char *ShotMaskCommand::aspectRatioFlag = "-ar";
const char *ShotMaskCommand::aspectRatioLongFlag = "-aspectRatio";
const char *ShotMaskCommand::cleanSceneFlag = "-cs";
const char *ShotMaskCommand::cleanSceneLongFlag = "-cleanScene";
const char *ShotMaskCommand::frameDigitsFlag = "-fd";
const char *ShotMaskCommand::frameDigitsLongFlag = "-frameDigits";
const char *ShotMaskCommand::keyTypeFlag = "-kt";
const char *ShotMaskCommand::keyTypeLongFlag = "-keyType";
const char *ShotMaskCommand::maskThicknessFlag = "-mt";
const char *ShotMaskCommand::maskThicknessLongFlag = "-maskThickness";

const char *ShotMaskCommand::titleFlag = "-t";
const char *ShotMaskCommand::titleLongFlag = "-title";
const char *ShotMaskCommand::text1Flag = "-t1";
const char *ShotMaskCommand::text1LongFlag = "-text1";
const char *ShotMaskCommand::text2Flag = "-t2";
const char *ShotMaskCommand::text2LongFlag = "-text2"; 

// Shot Mask Nodes/Elements
const char *ShotMaskCommand::mainGrpName = "atbShotMask_grp";
const char *ShotMaskCommand::frameCounterGrpName = "atbFrameCounter_grp";

const char *ShotMaskCommand::titleTextGrpName = "atbShotMaskTitleText_grp";
const char *ShotMaskCommand::bottomLeftTextGrpName = "atbShotMaskBLText_grp";
const char *ShotMaskCommand::bottomRightTextGrpName = "atbShotMaskBRText_grp";

// Shader Nodes and Groups
const char *ShotMaskCommand::borderShaderNodeName = "atbShotMaskBorder_shdr";
const char *ShotMaskCommand::borderShaderGroupName = "atbShotMaskBorder_shdrSG";
const char *ShotMaskCommand::textShaderNodeName = "atbShotMaskText_shdr";
const char *ShotMaskCommand::textShaderGroupName = "atbShotMaskText_shdrSG";
const char *ShotMaskCommand::keyIconShaderNodeName = "atbShotMaskKeyIcon_shdr";
const char *ShotMaskCommand::keyIconShaderGroupName = "atbShotMaskKeyIcon_shdrSG";
const char *ShotMaskCommand::bdIconShaderNodeName = "atbShotMaskBDIcon_shdr";
const char *ShotMaskCommand::bdIconShaderGroupName = "atbShotMaskBDIcon_shdrSG";
const char *ShotMaskCommand::ltbxShaderNodeName = "atbShotMaskLtbx_shdr";
const char *ShotMaskCommand::ltbxShaderGroupName = "atbxShotMaskLtbx_shdrSG";

const char *ShotMaskCommand::shotMaskExprName = "atbShotMaskFC_expr";


//*********************************************************
// Name: ShotMaskCommand
// Desc: Constructor
//*********************************************************
ShotMaskCommand::ShotMaskCommand()
{
    pluginTrace( "ShotMaskCommand", "ShotMaskCommand", "******* Shot Mask Command *******" );    

    // Initialize members
    queryMode = false;
    queryKeyType = false;
    queryDigits = false;

    cleanScene = false;

    pCameraFn = NULL;
    aspectRatio = 0.0;
    renderAspectRatio = 0.0;
    filmAspectRatio = 0.0;
    
    maskThickness = 0.05;
    minThickness = 0.02;
    maxThickness = 0.2;

    resultStr.set( "none" );
    objList.clear();

    shotMaskTitle.set( "Title" );
    shotMaskText1.set( "" );
    shotMaskText2.set( "" );

#ifdef NT_PLUGIN
	font = "Arial|h13|w700|c0";
#endif

// Apple stuff
#ifdef __APPLE__
	#if defined(__i386__)
		font = "Arial-Bold";
	#else
		font = "Arial-Bold|h13";
	#endif
#endif // __APPLE__

#ifdef LINUX_PLUGIN
    // hoping this is the right font for most distros
    font = "Utopia-Regular";
#endif

}

//*********************************************************
// Name: ShotMaskCommand
// Desc: Destructor
//*********************************************************
ShotMaskCommand::~ShotMaskCommand()
{
    // Cleanup
    if( pCameraFn != NULL )
        delete pCameraFn;
}

//*********************************************************
// Name: doIt
// Desc: All of the one-time setup and initialization
//       code for the set key command.  doIt is called
//       by Maya when any command is executed in MEL.
//*********************************************************
MStatus ShotMaskCommand::doIt(const MArgList &args)
{
    MStatus status = MS::kSuccess;

    // Set the command flag values appropriately
    if( !parseCommandFlags( args )) {
        pluginError( "ShotMaskCommand", "doIt", "Failed to parse command flags" );
    }
    else if( queryMode ) {
        if( queryKeyType ) {
            // Determine the key type, if no objects
            // were provided, it is obviously none
            if( objList.length() == 0 )
                setResult( "none" );
            else
                getKeyType();
        }
        else if( queryDigits ) {
            // Sets the result value with an array
            // of ints corresponding to the current frame
            generateFrameDigitArray();
        }
        else
            MGlobal::displayError( "No queriable flags provided" );
    }
    else {
        // Do we just want to delete the old mask
        if( cleanScene )
            cleanUpShotMask();

        // Otherwise, create the shot mask and expression
        else if( !(status = createShotMask())) {
            pluginError( "ShotMaskCommand", "doIt", "Failed to create the shot mask" );
        }
        else if( !(status = createShotMaskExpr())) {
            pluginError( "ShotMaskCommand", "doIt", "Failed to create the expression editor" );
        }
        // Deselect the shot mask
        MGlobal::executeCommand( "select -cl", false, true );
    }

    // If something fails, remove the whole shot mask
    if( !status )
        cleanUpShotMask();

    return status;
}

//*********************************************************
// Name: redoIt
// Desc: Contains the code that changes the internal state
//       of Maya.  It is called by Maya to redo.
//*********************************************************
MStatus ShotMaskCommand::redoIt()
{
    MStatus status = MS::kSuccess;

    // All of the undo/redo relates to MEL commands
    // which are handled by Maya

    return status;
}

//*********************************************************
// Name: undoIt
// Desc: Contains the code to undo the internal state 
//       changes made by the breakdown command (redoIt).
//       It is called by Maya to undo.
//*********************************************************
MStatus ShotMaskCommand::undoIt()
{
    MStatus status = MS::kSuccess;

    // All of the undo/redo relates to MEL commands
    // which are handled by Maya

    return status;
}


//*********************************************************
// Name: newSyntax
// Desc: Method for registering the command flags
//       with Maya
//*********************************************************
MSyntax ShotMaskCommand::newSyntax()
{
    // Use MSyntax for a more robust solution to the
    // parsing the command flags
    MSyntax syntax;
    syntax.addFlag( cameraFlag, cameraLongFlag, MSyntax::kString );
    syntax.addFlag( aspectRatioFlag, aspectRatioLongFlag, MSyntax::kDouble );
    syntax.addFlag( cleanSceneFlag, cleanSceneLongFlag, MSyntax::kNoArg );
    syntax.addFlag( frameDigitsFlag, frameDigitsLongFlag, MSyntax::kNoArg );
    syntax.addFlag( keyTypeFlag, keyTypeLongFlag, MSyntax::kNoArg );
    syntax.addFlag( maskThicknessFlag, maskThicknessLongFlag, MSyntax::kDouble );

    syntax.addFlag( titleFlag, titleLongFlag, MSyntax::kString );
    syntax.addFlag( text1Flag, text1LongFlag, MSyntax::kString );
    syntax.addFlag( text2Flag, text2LongFlag, MSyntax::kString );

    syntax.enableQuery();
    syntax.setObjectType( MSyntax::kSelectionList, 0 );

    return syntax;
}


//*********************************************************
// Name: parseCommandFlags
// Desc: Parse the command flags and stores the values
//       in the appropriate variables
//*********************************************************
MStatus ShotMaskCommand::parseCommandFlags( const MArgList &args )
{
    MStatus status = MS::kSuccess;
    
    // Create an object to parse the arguments
    // and then parse them
    MArgDatabase argData( syntax(), args, &status );

    if( !status ) {
        pluginError( "ShotMaskCommand", "parseCommandFlags", "Failed to get database" );
    }
    // Parse the flags - query flag (and queriable flags) take priority
    else if( (queryMode = argData.isQuery()) ) {
        if( argData.isFlagSet( keyTypeFlag )) {
            queryKeyType = true;
            argData.getObjects( objList );
        }
        else if( argData.isFlagSet( frameDigitsFlag ))
            queryDigits = true;
    }
    // If the cleanScene flag is set, ignore the rest.
    // When set, all nodes related to the Shot Mask
    // are cleared out of the scene, nothing is created
    else if( argData.isFlagSet( cleanSceneFlag )) {
        cleanScene = true;
    }
    else {
        // Continue parsing the remaing flags
        if( argData.isFlagSet( cameraFlag ))
            argData.getFlagArgument( cameraFlag, 0, camTransNodeName );
        if( argData.isFlagSet( aspectRatioFlag )) {
            argData.getFlagArgument( aspectRatioFlag, 0, aspectRatio );
            
            if( aspectRatio <= 0 )
                MGlobal::displayWarning( "Aspect Ratio must be greater than 0, Default film gate used" );
        }
        if( argData.isFlagSet( maskThicknessFlag )) {
            argData.getFlagArgument( maskThicknessFlag, 0, maskThickness );
            // Keep a fixed range for the mask size
            if( maskThickness < minThickness )
                maskThickness = minThickness;
            else if( maskThickness > maxThickness )
                maskThickness = maxThickness;
        }
        if( argData.isFlagSet( titleFlag ))
            argData.getFlagArgument( titleFlag, 0, shotMaskTitle );
        if( argData.isFlagSet( text1Flag ))
            argData.getFlagArgument( text1Flag, 0, shotMaskText1 );
        if( argData.isFlagSet( text2Flag ))
            argData.getFlagArgument( text2Flag, 0, shotMaskText2 );

        // We need a camera to perform this command on
        if( camTransNodeName.length() == 0 ) {
            MGlobal::displayError( "No camera selected" );
            pluginError( "ShotMaskCommand", "parseCommandFlags", "No Objects provided" );
            status = MS::kFailure;
        }
        else {
            // Get the name of the camera's shape node
            status = getCameraShapeNode();

            if( status ) {
                filmAspectRatio = pCameraFn->aspectRatio();

                if( aspectRatio <= 0 ) {
                aspectRatio = filmAspectRatio;

                // Get the render globals aspect ratio
                MGlobal::executeCommand( "getAttr defaultResolution.deviceAspectRatio", renderAspectRatio, false, false );
                }
                else
                    renderAspectRatio = aspectRatio;
            }
        }
    }

    return status;
}

//*********************************************************
// Name: getCameraShapeNode
// Desc: Retrieves the camera shape node from the given 
//       transform node
//*********************************************************
MStatus ShotMaskCommand::getCameraShapeNode()
{
    MStatus status = MS::kSuccess;

    MStringArray resultArray;
    MString getShapeCmd( "listRelatives -shapes -type camera " );
    MGlobal::executeCommand( getShapeCmd + camTransNodeName, resultArray );

    if( resultArray.length() == 0 ) {
        pluginError( "ShotMaskCommand", "parseCommandFlags", "failed to get camShapeNode" );
        MGlobal::displayError( "Invalid Camera Selected" );
        status = MS::kFailure;
    }
    else
        camShapeNodeName = resultArray[0];

    // Simple process to get the dependency node
    MSelectionList cameraList;
    if( !cameraList.add( camShapeNodeName )) {
        pluginError( "ShotMaskCommand", "parseCommandFlags", "Couldn't add camera to the list" );
    }
    else {
        // Get the dependency node for the camera
        MObject dependNode;
        if( !(status = cameraList.getDependNode( 0, dependNode ))) {
            pluginError( "ShotMaskCommand", "parseCommandFlags", "Failed to get dependency node" );
        }
        else {
            // Create the new camera function set
            pCameraFn = new MFnCamera( dependNode, &status );
            if( !status ) {
                MGlobal::displayError( "Camera Shape Node not provided" );
                pluginError( "ShotMaskCommand", "parseCommandFlags", "Object provided is not a camera shape node" );
                pluginError( "ShotMaskCommand", "parseCommandFlags", dependNode.apiTypeStr() );

                // Take care of the memory
                if( pCameraFn != NULL ) {
                    delete pCameraFn;
                    pCameraFn = NULL;
                }   
            }
        }
    }   

    return status;
}

//*********************************************************
// Name: cleanUpShotMask
// Desc: Deletes all shot mask elements from the
//       current scene
//*********************************************************
MStatus ShotMaskCommand::cleanUpShotMask()
{
    MStatus status = MS::kSuccess;

    MString cmdStr( "if( `objExists " );

    // Remove the geometry
    MGlobal::executeCommand( cmdStr + frameCounterGrpName + "` ) delete " + frameCounterGrpName, false, true );
    MGlobal::executeCommand( cmdStr + mainGrpName + "` ) delete " + mainGrpName, false, true );

    // Remove the shaders
    MGlobal::executeCommand( cmdStr + borderShaderGroupName + "` ) delete " + borderShaderGroupName, false, true );
    MGlobal::executeCommand( cmdStr + borderShaderNodeName + "` ) delete " + borderShaderNodeName, false, true );

    MGlobal::executeCommand( cmdStr + textShaderGroupName + "` ) delete " + textShaderGroupName, false, true );
    MGlobal::executeCommand( cmdStr + textShaderNodeName + "` ) delete " + textShaderNodeName, false, true );

    MGlobal::executeCommand( cmdStr + keyIconShaderGroupName + "` ) delete " + keyIconShaderGroupName, false, true );
    MGlobal::executeCommand( cmdStr + keyIconShaderNodeName + "` ) delete " + keyIconShaderNodeName, false, true );

    MGlobal::executeCommand( cmdStr + bdIconShaderGroupName + "` ) delete " + bdIconShaderGroupName, false, true );
    MGlobal::executeCommand( cmdStr + bdIconShaderNodeName + "` ) delete " + bdIconShaderNodeName, false, true );

    MGlobal::executeCommand( cmdStr + ltbxShaderGroupName + "` ) delete " + ltbxShaderGroupName, false, true );
    MGlobal::executeCommand( cmdStr + ltbxShaderNodeName + "` ) delete " + ltbxShaderNodeName, false, true );

    // Remove the expressions
    MGlobal::executeCommand( cmdStr + shotMaskExprName + "` ) delete " + shotMaskExprName, false, true );

    return status;
}

//*********************************************************
// Name: createShotMask
// Desc: Creates a shot mask for a specified camera
//*********************************************************
MStatus ShotMaskCommand::createShotMask()
{
    pluginTrace( "ShotMaskCommand", "createShotMask", "***" );

    MStatus status = MS::kSuccess;

    // Remove all elements of any previous shot masks
    if( !(status = cleanUpShotMask() )) {
        pluginTrace( "ShotMaskCommand", "createShotMask", "Failed to clean up the previous shot mask" );
    }
    else {
        // Generate the shaders first
        createShotMaskShaders();

        // The width and height of the shot mask
        double width, height;

        double nearClip = pCameraFn->nearClippingPlane();
        if( nearClip < 0.1 )
            nearClip = 0.1;

        // The location of the shot mask is slightly offset
        // to minimize clipping problems with the
        // labels, frame counter, etc...
        double shotMaskZPos = nearClip + (nearClip * 0.1);

        // The z-position for the 
        double textZPos = shotMaskZPos - (nearClip * 0.006);

        // The z-position for icons
        double iconZPos = shotMaskZPos - (nearClip * 0.003);

        // The z-position for the frame counter numbers
        double frameNumZPos = textZPos;

        // Determine how the film back is mapped to 
        // the render globals aspect ratio
        MFnCamera::FilmFit filmFit = pCameraFn->filmFit();

        // Handle Overscan & Fill fit options
        if( filmFit == MFnCamera::kFillFilmFit && 
            (filmAspectRatio <= renderAspectRatio || filmAspectRatio == aspectRatio) )
            filmFit = MFnCamera::kHorizontalFilmFit;

        else if( filmFit == MFnCamera::kOverscanFilmFit &&
            (filmAspectRatio >= renderAspectRatio || filmAspectRatio == aspectRatio) )
            filmFit = MFnCamera::kHorizontalFilmFit;


        // Is this an ortho camera
        if( pCameraFn->isOrtho() ) {
            width = pCameraFn->orthoWidth();
            height = width / aspectRatio;
        }
        else {
            // Get the necessary camera attributes
            double vertFOV = pCameraFn->verticalFieldOfView();
            double horizontalFOV = pCameraFn->horizontalFieldOfView();
            
            // Calculate the width & height of the gate at the
            // Z position of shot mask based on the camera's
            // film fit option (for perspective cameras)    
            if( filmFit == MFnCamera::kVerticalFilmFit ) {
                // The vertFOV will remain fixed
                height = 2 * (tan(vertFOV / 2.0) * shotMaskZPos);
                width = height * aspectRatio;
            }
            else if( filmFit == MFnCamera::kHorizontalFilmFit ) {
                // The horizontal FOV will remain fixed
                width = 2 * (tan(horizontalFOV / 2.0) * shotMaskZPos);
                height = width / aspectRatio;
            }
            else if( filmFit == MFnCamera::kFillFilmFit ) {
                // When the film aspect ratio > render aspect ratio 
                // we need to adjust the size of the mask
                double scaling = renderAspectRatio / filmAspectRatio;
                width = 2 * (tan(horizontalFOV / 2.0) * scaling * shotMaskZPos);
                height = width / aspectRatio;
            }
            else if( filmFit == MFnCamera::kOverscanFilmFit ) {
                // When the render aspect ratio > film aspect ratio 
                // we need to adjust the size of the mask
                double scaling = renderAspectRatio / filmAspectRatio;
                width = 2 * (tan(horizontalFOV / 2.0) * scaling * shotMaskZPos);
                height = width / aspectRatio;
            }
        }

        // The corners (size) of the shot mask, based
        // when it is positioned at the near clipping
        // plan of the camera
        double left, right, top, bottom;
        right = width / 2.0;
        left = -right;
        top = height / 2.0;
        bottom = -top;

        // The thickness of horizontal edges
        double hThickness = maskThickness * height;
        // The thickness of vertical edges
        double vThickness = maskThickness * width;

        double iconScale = hThickness * 0.6;
#ifdef NT_PLUGIN
        double counterScale = hThickness * 0.17;
        double titleScale = hThickness * 0.25;
        double subtitleScale = hThickness * 0.22;
#endif


#ifdef LINUX_PLUGIN
        // with Utopia, it seems to be the same as PPC-mac
        double counterScale = hThickness * 0.50;
        double titleScale = hThickness * 0.53;
        double subtitleScale = hThickness * 0.50;
#endif

// Apple stuff
// #FIXME! This is a spot that'll need to be added to when
//		   64-bit Maya hits the Mac, maybe
#ifdef __APPLE__
	//##NEW
//	#if defined(TDT_MAYA_2011)
	#if defined(__i386__)
       	double counterScale = hThickness * 0.11;
       	double titleScale = hThickness * 0.15;
       	double subtitleScale = hThickness * 0.15;
	#else
       	double counterScale = hThickness * 0.50;
       	double titleScale = hThickness * 0.53;
      	double subtitleScale = hThickness * 0.50;
	#endif
	/*
	#elif defined(TDT_MAYA_2009)
		// I know these are the same as Maya 2008, but I have reason
		// for keeping them separate
       	double counterScale = hThickness * 0.11;
       	double titleScale = hThickness * 0.15;
       	double subtitleScale = hThickness * 0.15;
	#elif defined(TDT_MAYA_2008)
       	double counterScale = hThickness * 0.11;
       	double titleScale = hThickness * 0.15;
       	double subtitleScale = hThickness * 0.15;
	#elif defined(TDT_MAYA_85)
       	double counterScale = hThickness * 0.11;
       	double titleScale = hThickness * 0.15;
       	double subtitleScale = hThickness * 0.15;
	#else
		// other ones are ppc only, 8.0 and 7.0.
       	double counterScale = hThickness * 0.034;
       	double titleScale = hThickness * 0.046;
       	double subtitleScale = hThickness * 0.042;
	#endif
	 */
#endif

        MString topPlaneName( "atbFrameTop_geo" );
        MString bottomPlaneName( "atbFrameBottom_geo" );
        MString leftPlaneName( "atbFrameRight_geo" );
        MString rightPlaneName( "atbFrameLeft_geo" );

        MString mainGroupName( mainGrpName );
        MString topFrameGroupName( "atbFrameTop_grp" );
        MString bottomFrameGroupName( "atbFrameBottom_grp" );
        MString sideFrameGroupName( "atbFrameSides_grp" );

        // How far to move the mask edges to fit the gate
        double edgeVertTrans = top - (hThickness/2.0);
        double edgeHoriTrans = right - (vThickness/2.0);

        MString nullStr( "" );
        MDoubleArray boundingBox;

        // Create the Top Border (and labels/icons)
        MGlobal::executeCommand( nullStr + "polyPlane -w " + width + " -h " + hThickness + "-sx 1 -sy 1 -ch 0 -n " + topPlaneName + "; " +
                                 "setAttr " + topPlaneName + "Shape.overrideEnabled 1; setAttr " + topPlaneName + "Shape.overrideDisplayType 2; " +
                                 "rotate -a 90 0 0 " + topPlaneName + "; " +
                                 "move -a 0 " + edgeVertTrans + " " + -shotMaskZPos + " " + topPlaneName + "; " +
                                 "select -r " + topPlaneName + "; sets -e -forceElement " + borderShaderGroupName + "; " +
                                 "group -n " + topFrameGroupName + "; ", false, true );

        if( shotMaskTitle != "" ) {
            MString titleGeoName( "atbTitleText" );
            createText( titleGeoName, titleTextGrpName, shotMaskTitle, font );
            MGlobal::executeCommand( nullStr + "xform -cp " + titleGeoName + "; " +
                                     "move -rpr 0 " + edgeVertTrans + " " + -textZPos + " " + titleGeoName + "; " + 
                                     "scale -a " + titleScale + " " + titleScale + " " + titleScale + " " + titleGeoName + "; " +
                                     "makeIdentity -apply true -t 1 -r 1 -s 1 -n 0 " + titleGeoName + "; " +
                                     "xform -cp " + titleTextGrpName + "; " +                                      
                                     "parent -a " + titleTextGrpName + " " + topFrameGroupName + "; ", false, true                               
                                      );
            // Move the scale pivot to the top edge
            //MGlobal::executeCommand( nullStr + "exactWorldBoundingBox " + titleGeoName, boundingBox, false, true );
            //MGlobal::executeCommand( nullStr + "move -a " + 0.0 + " " + boundingBox[4] + " " + boundingBox[5] + " " + 
            //                                   titleGeoName + ".scalePivot " + titleGeoName + ".rotatePivot; ", false, true ); 
        }

        // Add the Key Icon (oval)
        MGlobal::executeCommand( nullStr + "circle -n atbTempCircle_geo; scale -a 1 0.6 1 atbTempCircle_geo; " +
                                 "planarSrf -n atbKeyIcon_geo -ch 0 -d 3 -ko 0 -tol 0.01 -rn 0 -po 1 atbTempCircle_geo;" +
                                 "setAttr " + "atbKeyIcon_geoShape.overrideEnabled 1; setAttr " + "atbKeyIcon_geoShape.overrideDisplayType 2; " +
                                 "scale -a " + iconScale + " " + iconScale + " " + iconScale + " atbKeyIcon_geo; " +
                                 "move -rpr " + edgeHoriTrans + " " + edgeVertTrans + " " + -iconZPos + " atbKeyIcon_geo;" +
                                 "delete atbTempCircle_geo;" + 
                                 "select -r atbKeyIcon_geo; sets -e -forceElement " + keyIconShaderGroupName + "; ", false, true );

        // Add the Breakdown Icon (underline)
        MGlobal::executeCommand( nullStr + "nurbsPlane -w 2.0 -lr 0.1 -ch 0 -n atbBreakdownIcon_geo; " +
                                 "setAttr " + "atbBreakdownIcon_geoShape.overrideEnabled 1; setAttr " + "atbBreakdownIcon_geoShape.overrideDisplayType 2; " +
                                 "rotate -a 0 90 0 atbBreakdownIcon_geo; " +
                                 "scale -a " + iconScale + " " + iconScale + " " + iconScale + " atbBreakdownIcon_geo; " +
                                 "move -rpr " + edgeHoriTrans + " " + (edgeVertTrans - (0.3 *hThickness)) + " " + -iconZPos + " atbBreakdownIcon_geo; " + 
                                 "select -r atbBreakdownIcon_geo; sets -e -forceElement " + bdIconShaderGroupName + "; ", false, true );

        // Create the Frame Counter Group and move to the right edge
        MGlobal::executeCommand( nullStr + "select -r atbKeyIcon_geo atbBreakdownIcon_geo; " +
                                 "group -n " + frameCounterGrpName + "; xform -os -piv 0 0 0; "
                                 
                                 //"scale -a 1.2 1.2 1.2 atbFrameCounter_grp;" +
                                 "xform -cp " + frameCounterGrpName + "; " +  
                                 "scale -a 1.2 1.2 1.0 atbFrameCounter_grp;" +
                                 // Parent the Frame Counter Group to the Top Border
                                 "parent -a " + frameCounterGrpName + " " + topFrameGroupName + "; ", false, true );                

        // Create the digits of the frame counter
        createFrameCounter( counterScale, frameNumZPos, hThickness, edgeHoriTrans, edgeVertTrans );
        // Slight the counter slightly left and down from the edges
        MGlobal::executeCommand( nullStr +
                                 "move -r " + (-0.13 * vThickness) + " " + (-0.04 * hThickness) + " 0 " + frameCounterGrpName + "; " + 
                                 "makeIdentity -apply true -t 1 -r 1 -s 1 -n 0 " + frameCounterGrpName + "; ", false, true );

        // Move pivot to top right corner for the frame counter
        MGlobal::executeCommand( nullStr + "exactWorldBoundingBox " + frameCounterGrpName, boundingBox, false, true );
        MGlobal::executeCommand( nullStr + "move -a " + boundingBox[3] + " " + boundingBox[4] + " " + boundingBox[5] + " " + 
                                               frameCounterGrpName + ".scalePivot " + frameCounterGrpName + ".rotatePivot; ", false, true ); 

        // Create the Bottom Border (and labels/icons)
        MGlobal::executeCommand( nullStr + "polyPlane -w " + width + " -h " + hThickness + "-sx 1 -sy 1 -ch 0 -n " + bottomPlaneName + "; " +
                                 "setAttr " + bottomPlaneName + "Shape.overrideEnabled 1; setAttr " + bottomPlaneName + "Shape.overrideDisplayType 2; " +
                                 "rotate -a 90 0 0 " + bottomPlaneName + "; " +
                                 "move -a 0 " + -edgeVertTrans + " " + -shotMaskZPos + " " + bottomPlaneName + "; " + 
                                 "select -r " + bottomPlaneName + "; sets -e -forceElement " + borderShaderGroupName + "; " + 
                                 "group -n " + bottomFrameGroupName + "; ", false, true );

        double subtitleVertPos, subtitleHoriPos, textHeight, textWidth;

        double padding = 0.03 * width;

        // Create the text if necessary
        if( shotMaskText1 != "" ) {
            MString textGeoName( "atbBottomLeftText" );
            createText( textGeoName, bottomLeftTextGrpName, shotMaskText1, font );

            // We want to vertically align the text in the middle of the mask edge
            MGlobal::executeCommand( nullStr + "exactWorldBoundingBox " + bottomLeftTextGrpName, boundingBox, false, true );

            textHeight = boundingBox[4] - boundingBox[1]; 
            subtitleVertPos = (-edgeVertTrans - (0.25 * textHeight * subtitleScale ));

            // Caculate the horizontal position with a fixed padding          
            subtitleHoriPos = left + padding;

            // Create the bottom-left aligned text
            MGlobal::executeCommand( nullStr + 
                                     "move -r " + subtitleHoriPos + " " + subtitleVertPos + " " + -textZPos + " " + textGeoName + "; " + 
                                     "scale -a " + subtitleScale + " " + subtitleScale + " " + subtitleScale + " " + textGeoName + "; " +
                                     "makeIdentity -apply true -t 1 -r 1 -s 1 -n 0 " + textGeoName + "; " +
                                     "xform -cp " + bottomLeftTextGrpName + "; " +
                                     "parent -a " + bottomLeftTextGrpName + " " + bottomFrameGroupName + "; ", false, true                                  
                                     );

            // Move the scale pivot to the top edge
            MGlobal::executeCommand( nullStr + "exactWorldBoundingBox " + textGeoName, boundingBox, false, true );
            MGlobal::executeCommand( nullStr + "move -a " + boundingBox[0] + " " + (boundingBox[4] - (textHeight * subtitleScale * 0.5))+ " " + boundingBox[5] + " " + 
                                               textGeoName + ".scalePivot " + textGeoName + ".rotatePivot; ", false, true ); 
        }

        if( shotMaskText2 != "" ) {
            MString textGeoName( "atbBottomRightText" );
            createText( textGeoName, bottomRightTextGrpName, shotMaskText2, font );

            // We want to calculate the right alignment
            MGlobal::executeCommand( nullStr + "exactWorldBoundingBox " + bottomRightTextGrpName, boundingBox, false, true );

            // Caculate the vertical alignment (it may not have been done before)
            textHeight = boundingBox[4] - boundingBox[1]; 
            subtitleVertPos = (-edgeVertTrans - (0.25 * textHeight * subtitleScale ));

            textWidth = boundingBox[3] - boundingBox[0];
            subtitleHoriPos = right - (textWidth * subtitleScale) - padding;

            // Create the bottom-right aligned text
            MGlobal::executeCommand( nullStr + 
                                     "move -r " + subtitleHoriPos + " " + subtitleVertPos + " " + -textZPos + " " + textGeoName + "; " + 
                                     "scale -a " + subtitleScale + " " + subtitleScale + " " + subtitleScale + " " + textGeoName + "; " +
                                     "makeIdentity -apply true -t 1 -r 1 -s 1 -n 0 " + textGeoName + "; " +
                                     "xform -cp " + bottomRightTextGrpName + "; " +
                                     "parent -a " + bottomRightTextGrpName + " " + bottomFrameGroupName + "; ", false, true                                  
                                     );

            // Move the scale pivot to the top edge
            MGlobal::executeCommand( nullStr + "exactWorldBoundingBox " + textGeoName, boundingBox, false, true );
            MGlobal::executeCommand( nullStr + "move -a " + boundingBox[3] + " " + (boundingBox[4] - (textHeight * subtitleScale * 0.5))+ " " + boundingBox[5] + " " + 
                                               textGeoName + ".scalePivot " + textGeoName + ".rotatePivot; ", false, true ); 
        }

        // Create the side border edges
        double sideBorderHeight = height - (2 * hThickness);

        // left edge
        MGlobal::executeCommand( nullStr + "polyPlane -w " + vThickness + " -h " + sideBorderHeight + "-sx 1 -sy 1 -ch 0 -n " + leftPlaneName + "; " +
                                 "setAttr " + leftPlaneName + "Shape.overrideEnabled 1; setAttr " + leftPlaneName + "Shape.overrideDisplayType 2; " +
                                 "rotate -a 90 0 0 " + leftPlaneName + "; " +
                                 "move -a " + -edgeHoriTrans + " 0 " + -shotMaskZPos + " " + leftPlaneName + "; " +
                                 "select -r " + leftPlaneName + "; sets -e -forceElement " + borderShaderGroupName + "; " +
                                 "group -n " + sideFrameGroupName + "; ", false, true );

        // right edge
        MGlobal::executeCommand( nullStr + "polyPlane -w " + vThickness + " -h " + sideBorderHeight + "-sx 1 -sy 1 -ch 0 -n " + rightPlaneName + "; " +
                                 "setAttr " + rightPlaneName + "Shape.overrideEnabled 1; setAttr " + rightPlaneName + "Shape.overrideDisplayType 2; " +
                                 "rotate -a 90 0 0 " + rightPlaneName + "; " +
                                 "move -a " + edgeHoriTrans + " 0 " + -shotMaskZPos + " " + rightPlaneName + "; " +
                                 "select -r " + rightPlaneName + "; sets -e -forceElement " + borderShaderGroupName + "; " + 
                                 "parent -a " + rightPlaneName + " " + sideFrameGroupName + "; ", false, true );


        // Create the letterbox geo
        MString letterboxGrpName( "atbLetterbox_grp" );
        MString letterboxTopName( "atbLetterboxTop_geo" );
        MString letterboxBottomName( "atbLetterboxBottom_geo" );
        MString letterboxLeftName( "atbLetterboxLeft_geo" );
        MString letterboxRightName( "atbLetterboxRight_geo" );

        double lbWidth = 3 * width;
        double lbHeight = height;

        // Letterboxing should be located on the outer edge
        // of the gate
        edgeVertTrans = top + (lbHeight/2.0);
        
        // top letterbox edge
        MGlobal::executeCommand( nullStr + "polyPlane -w " + lbWidth + " -h " + lbHeight + "-sx 1 -sy 1 -ch 0 -n " + letterboxTopName + "; " +
                                 "setAttr " + letterboxTopName + "Shape.overrideEnabled 1; setAttr " + letterboxTopName + "Shape.overrideDisplayType 2; " +
                                 "rotate -a 90 0 0 " + letterboxTopName + "; " +
                                 "move -a 0 " + edgeVertTrans + " " + -shotMaskZPos + " " + letterboxTopName + "; " +
                                 "select -r " + letterboxTopName + "; sets -e -forceElement " + ltbxShaderGroupName + "; ", false, true );
        // bottom leterbox edge
        MGlobal::executeCommand( nullStr + "polyPlane -w " + lbWidth + " -h " + lbHeight + "-sx 1 -sy 1 -ch 0 -n " + letterboxBottomName + "; " +
                                 "setAttr " + letterboxBottomName + "Shape.overrideEnabled 1; setAttr " + letterboxBottomName + "Shape.overrideDisplayType 2; " +
                                 "rotate -a 90 0 0 " + letterboxBottomName + "; " +
                                 "move -a 0 " + -edgeVertTrans + " " + -shotMaskZPos + " " + letterboxBottomName + "; " +
                                 "select -r " + letterboxBottomName + "; sets -e -forceElement " + ltbxShaderGroupName + "; ", false, true );

        lbWidth = width;
        lbHeight = height;

        edgeHoriTrans = right + (0.5 * lbWidth);

        // left letterbox edge
        MGlobal::executeCommand( nullStr + "polyPlane -w " + lbWidth + " -h " + lbHeight + "-sx 1 -sy 1 -ch 0 -n " + letterboxLeftName + "; " +
                                 "setAttr " + letterboxLeftName + "Shape.overrideEnabled 1; setAttr " + letterboxLeftName + "Shape.overrideDisplayType 2; " +
                                 "rotate -a 90 0 0 " + letterboxLeftName + "; " +
                                 "move -a " + -edgeHoriTrans + " 0 " + -shotMaskZPos + " " + letterboxLeftName + "; " +
                                 "select -r " + letterboxLeftName + "; sets -e -forceElement " + ltbxShaderGroupName + "; ", false, true );

        // right letterbox edge
        MGlobal::executeCommand( nullStr + "polyPlane -w " + lbWidth + " -h " + lbHeight + "-sx 1 -sy 1 -ch 0 -n " + letterboxRightName + "; " +
                                 "setAttr " + letterboxRightName + "Shape.overrideEnabled 1; setAttr " + letterboxRightName + "Shape.overrideDisplayType 2; " +
                                 "rotate -a 90 0 0 " + letterboxRightName + "; " +
                                 "move -a " + edgeHoriTrans + " 0 " + -shotMaskZPos + " " + letterboxRightName + "; " +
                                 "select -r " + letterboxRightName + "; sets -e -forceElement " + ltbxShaderGroupName + "; ", false, true );

        // Group the letterbox geo together
        MGlobal::executeCommand( "select -r " + letterboxTopName + " " + 
                                                letterboxBottomName + " " + 
                                                letterboxLeftName + " " + 
                                                letterboxRightName + "; " +
                                 "group -n " + letterboxGrpName + "; xform -os -piv 0 0 0; ", false, true );

        // Group the mask border together
        MGlobal::executeCommand( "select -r " + topFrameGroupName + " " + 
                                                bottomFrameGroupName + " " + 
                                                sideFrameGroupName +  " " + 
                                                letterboxGrpName + "; " +
                                 "group -n " + mainGrpName + "; xform -os -piv 0 0 0; ", false, true);

        // Constrain the mask to the camera
        MGlobal::executeCommand( "select -r " + camTransNodeName + "; select -add " + mainGrpName + "; " +  
                                 "parentConstraint -weight 1", false, true );
    }

    return status;
}

//*********************************************************
// Name: createShotMaskShaders
// Desc: Create the shot mask shaders
//*********************************************************
MStatus ShotMaskCommand::createShotMaskShaders()
{
    MStatus status = MS::kSuccess;

    MString newShadingNodeStr( "shadingNode -asShader lambert -name " );

    // The border shader
    MGlobal::executeCommand( newShadingNodeStr + borderShaderNodeName + "; " +
                             "sets -r true -nss true -em -n " + borderShaderGroupName + "; " +
                             "connectAttr -f " + borderShaderNodeName + ".outColor " + borderShaderGroupName + ".surfaceShader; " +
                             "setAttr \"" + borderShaderNodeName + ".color\" -type double3 0.0 0.0 0.0; " +
                             "setAttr \"" + borderShaderNodeName + ".transparency\" -type double3 0.85 0.85 0.85; ", false, true );
    // The text shader
    MGlobal::executeCommand( newShadingNodeStr + textShaderNodeName + "; " +
                             "sets -r true -nss true -em -n " + textShaderGroupName + "; " +
                             "connectAttr -f " + textShaderNodeName + ".outColor " + textShaderGroupName + ".surfaceShader; " +
                             "setAttr \"" + textShaderNodeName + ".color\" -type double3 1.0 1.0 1.0; " +
                             "setAttr \"" + textShaderNodeName + ".transparency\" -type double3 0.0 0.0 0.0; ", false, true );
    // The key shader
    MGlobal::executeCommand( newShadingNodeStr + keyIconShaderNodeName + "; " +
                             "sets -r true -nss true -em -n " + keyIconShaderGroupName + "; " +
                             "connectAttr -f " + keyIconShaderNodeName + ".outColor " + keyIconShaderGroupName + ".surfaceShader; " +
                             "setAttr \"" + keyIconShaderNodeName + ".color\" -type double3 0.8 0.0 0.0; " +
                             "setAttr \"" + keyIconShaderNodeName + ".transparency\" -type double3 0.0 0.0 0.0; ", false, true );
    // The breakdown shader
    MGlobal::executeCommand( newShadingNodeStr + bdIconShaderNodeName + "; " +
                             "sets -r true -nss true -em -n " + bdIconShaderGroupName + "; " +
                             "connectAttr -f " + bdIconShaderNodeName + ".outColor " + bdIconShaderGroupName + ".surfaceShader; " +
                             "setAttr \"" + bdIconShaderNodeName + ".color\" -type double3 0.0 0.8 0.0; " +
                             "setAttr \"" + bdIconShaderNodeName + ".transparency\" -type double3 0.0 0.0 0.0; ", false, true );
    // The letterbox shader
    MGlobal::executeCommand( newShadingNodeStr + ltbxShaderNodeName + "; " +
                             "sets -r true -nss true -em -n " + ltbxShaderGroupName + "; " +
                             "connectAttr -f " + ltbxShaderNodeName + ".outColor " + ltbxShaderGroupName + ".surfaceShader; " +
                             "setAttr \"" + ltbxShaderNodeName + ".color\" -type double3 0.0 0.0 0.0; " +
                             "setAttr \"" + ltbxShaderNodeName + ".transparency\" -type double3 0.0 0.0 0.0; ", false, true );


    return status;
}

//*********************************************************
// Name: createText 
// Desc: Create text for the shot mask
//*********************************************************
MStatus ShotMaskCommand::createText( MString name, MString grpNodeName, MString text, MString font )
{
    MString nullStr( "" );
    MStringArray nurbsCurveArray;

    MGlobal::executeCommand( nullStr + "textCurves -ch 0 -f \"" + font + "\" -t \"" + text + "\"; ", nurbsCurveArray, false, true );

    MStringArray letters;
    MGlobal::executeCommand( "listRelatives -c ", letters );

    // Create a group for the text
    MGlobal::executeCommand( "group -em -n " + grpNodeName + "; " +
                             "group -em -n " + name + "; " +
                             "parent -a " + name + " " + grpNodeName + "; ", false, true );

    MString letterName;
    for( unsigned int i = 0; i < letters.length(); i++ ) {
        letterName = name;
        letterName = letterName + "_" + i + "_" + letters[i] + "_geo";
        MGlobal::executeCommand( nullStr + "select -r " + letters[i] + "; " + 
                                 "planarSrf -name \"" + letterName + "\" -ch 0 -tol 0.01 -o on -po 1; " +  
                                 "setAttr " + letterName + "Shape.overrideEnabled 1; setAttr " + letterName + "Shape.overrideDisplayType 2; " +
                                 "sets -e -forceElement " + textShaderGroupName + "; " +
                                 "parent -a " + letterName + " " + name + "; ", false, true );
    }

    MGlobal::executeCommand( nullStr + "delete " + nurbsCurveArray[0] + "; ", false, true );

    return MStatus::kSuccess;
}


//*********************************************************
// Name: createFrameCounter
// Desc: Create the shot mask shaders
//*********************************************************
MStatus ShotMaskCommand::createFrameCounter( double scaleVal,
                                             double digitZPos, 
                                             double borderThickness,
                                             double hEdgeTrans,
                                             double vEdgeTrans )
{
    MStatus status = MS::kSuccess;

    double textHeight = 0, textWidth = 0;
    double vPos = 0, hPos = 0, hOffset = 0;
    MDoubleArray boundingBox;

    MString numberStr;
    MString nullStr( "" );

    for( unsigned int i = 0; i < 4; i++ ) {
        MString groupName( "atbDigitColumn_" );
        groupName = groupName + i + "_grp";
        MGlobal::executeCommand( "group -em -n " + groupName + "; ", false, true );

        hPos += hOffset;
        for( unsigned int j = 0; j <= 9; j++ ) {
            MStringArray nurbsCurveArray;
            MStringArray surfaceArray;

            MString digitSurfaceName( "atbShotMaskDigit_" );
            digitSurfaceName = digitSurfaceName + i + "_" + j + "_geo";

            numberStr.set( j, 1 );
            MGlobal::executeCommand( nullStr + "textCurves -ch 0 -f \"" + font + "\" -t " + numberStr.asInt() + "; ", nurbsCurveArray, false, true );

            MGlobal::executeCommand( nullStr + "planarSrf -n " + digitSurfaceName + " -ch 0 -tol 0.01 -o on -po 1 " + nurbsCurveArray[0] + "; ",
                                     surfaceArray, false, true);
            
            MGlobal::executeCommand( nullStr + "select -r " + surfaceArray[0] + "; sets -e -forceElement " + textShaderGroupName + "; ", false, true );

            MGlobal::executeCommand( nullStr + "delete " + nurbsCurveArray[0] + "; ", false, true );

            MGlobal::executeCommand( nullStr +
                                     "setAttr " + surfaceArray[0] + "Shape.overrideEnabled 1; setAttr " + surfaceArray[0] + "Shape.overrideDisplayType 2; " );

            MGlobal::executeCommand( "parent -a " + surfaceArray[0] + " " + groupName + "; ", false, true );
        }

        MGlobal::executeCommand( nullStr + 
                                 "scale -a " + scaleVal + " " + scaleVal + " " + scaleVal + " " + groupName + "; " +
                                 "move -r " + hEdgeTrans + " " + vEdgeTrans + " " + -digitZPos + " " + groupName + "; "  +
                                 "parent -a " + groupName + " " + frameCounterGrpName + "; ", false, true
                                  );
        if( textWidth == 0 ) {
                MGlobal::executeCommand( nullStr + "exactWorldBoundingBox " + groupName, boundingBox, false, true );
                textHeight = boundingBox[4] - boundingBox[1];
                vPos = -textHeight * 0.5;

                textWidth = boundingBox[3] - boundingBox[0];
                double spacing = textWidth * 0.1;
                hPos = (textWidth) + (spacing);
                hOffset = -(textWidth + (textWidth * 0.1));
        }
        MGlobal::executeCommand( nullStr + 
                                 "move -r " + hPos + " " + vPos + " 0 " + groupName + "; ", false, true );

    }

    return status;
}

//*********************************************************
// Name: createShotMaskExpr
// Desc: Creates the expression to update the shot mask
//*********************************************************
MStatus ShotMaskCommand::createShotMaskExpr()
{
    pluginTrace( "ShotMaskCommand", "createShotMaskExpr", "***" );

    MString expression;

    expression = 

"int $visibleDigits[] = `cieShotMask -q -fd`;\
\
atbShotMaskDigit_0_0_geo.visibility = ($visibleDigits[0] == 0 ? 1 : 0); \
atbShotMaskDigit_0_1_geo.visibility = ($visibleDigits[0] == 1 ? 1 : 0); \
atbShotMaskDigit_0_2_geo.visibility = ($visibleDigits[0] == 2 ? 1 : 0); \
atbShotMaskDigit_0_3_geo.visibility = ($visibleDigits[0] == 3 ? 1 : 0); \
atbShotMaskDigit_0_4_geo.visibility = ($visibleDigits[0] == 4 ? 1 : 0); \
atbShotMaskDigit_0_5_geo.visibility = ($visibleDigits[0] == 5 ? 1 : 0); \
atbShotMaskDigit_0_6_geo.visibility = ($visibleDigits[0] == 6 ? 1 : 0); \
atbShotMaskDigit_0_7_geo.visibility = ($visibleDigits[0] == 7 ? 1 : 0); \
atbShotMaskDigit_0_8_geo.visibility = ($visibleDigits[0] == 8 ? 1 : 0); \
atbShotMaskDigit_0_9_geo.visibility = ($visibleDigits[0] == 9 ? 1 : 0); \
\
atbShotMaskDigit_1_0_geo.visibility = ($visibleDigits[1] == 0 ? 1 : 0); \
atbShotMaskDigit_1_1_geo.visibility = ($visibleDigits[1] == 1 ? 1 : 0); \
atbShotMaskDigit_1_2_geo.visibility = ($visibleDigits[1] == 2 ? 1 : 0); \
atbShotMaskDigit_1_3_geo.visibility = ($visibleDigits[1] == 3 ? 1 : 0); \
atbShotMaskDigit_1_4_geo.visibility = ($visibleDigits[1] == 4 ? 1 : 0); \
atbShotMaskDigit_1_5_geo.visibility = ($visibleDigits[1] == 5 ? 1 : 0); \
atbShotMaskDigit_1_6_geo.visibility = ($visibleDigits[1] == 6 ? 1 : 0); \
atbShotMaskDigit_1_7_geo.visibility = ($visibleDigits[1] == 7 ? 1 : 0); \
atbShotMaskDigit_1_8_geo.visibility = ($visibleDigits[1] == 8 ? 1 : 0); \
atbShotMaskDigit_1_9_geo.visibility = ($visibleDigits[1] == 9 ? 1 : 0); \
\
atbShotMaskDigit_2_0_geo.visibility = ($visibleDigits[2] == 0 ? 1 : 0); \
atbShotMaskDigit_2_1_geo.visibility = ($visibleDigits[2] == 1 ? 1 : 0); \
atbShotMaskDigit_2_2_geo.visibility = ($visibleDigits[2] == 2 ? 1 : 0); \
atbShotMaskDigit_2_3_geo.visibility = ($visibleDigits[2] == 3 ? 1 : 0); \
atbShotMaskDigit_2_4_geo.visibility = ($visibleDigits[2] == 4 ? 1 : 0); \
atbShotMaskDigit_2_5_geo.visibility = ($visibleDigits[2] == 5 ? 1 : 0); \
atbShotMaskDigit_2_6_geo.visibility = ($visibleDigits[2] == 6 ? 1 : 0); \
atbShotMaskDigit_2_7_geo.visibility = ($visibleDigits[2] == 7 ? 1 : 0); \
atbShotMaskDigit_2_8_geo.visibility = ($visibleDigits[2] == 8 ? 1 : 0); \
atbShotMaskDigit_2_9_geo.visibility = ($visibleDigits[2] == 9 ? 1 : 0); \
\
atbShotMaskDigit_3_0_geo.visibility = ($visibleDigits[3] == 0 ? 1 : 0); \
atbShotMaskDigit_3_1_geo.visibility = ($visibleDigits[3] == 1 ? 1 : 0); \
atbShotMaskDigit_3_2_geo.visibility = ($visibleDigits[3] == 2 ? 1 : 0); \
atbShotMaskDigit_3_3_geo.visibility = ($visibleDigits[3] == 3 ? 1 : 0); \
atbShotMaskDigit_3_4_geo.visibility = ($visibleDigits[3] == 4 ? 1 : 0); \
atbShotMaskDigit_3_5_geo.visibility = ($visibleDigits[3] == 5 ? 1 : 0); \
atbShotMaskDigit_3_6_geo.visibility = ($visibleDigits[3] == 6 ? 1 : 0); \
atbShotMaskDigit_3_7_geo.visibility = ($visibleDigits[3] == 7 ? 1 : 0); \
atbShotMaskDigit_3_8_geo.visibility = ($visibleDigits[3] == 8 ? 1 : 0); \
atbShotMaskDigit_3_9_geo.visibility = ($visibleDigits[3] == 9 ? 1 : 0); \
\
atbKeyIcon_geo.visibility = 0;\
atbBreakdownIcon_geo.visibility = 0;\
\
string $keyType;\
\
if( $g_cieATBShotMaskRootObj != \\\"\\\" ) { \
    string $objArray[] = stringToStringArray( $g_cieATBShotMaskRootObj, \\\" \\\"  ); \
    $keyType = `cieShotMask -q -kt $objArray`; \
} \
else \
       $keyType = \\\"none\\\"; \
\
if( $keyType == \\\"key\\\" ) \
	atbKeyIcon_geo.visibility = 1; \
else if( $keyType == \\\"breakdown\\\" ) \
	atbBreakdownIcon_geo.visibility = 1; \
";
    
    return MGlobal::executeCommand( "expression -s \"" + expression + "\"" +
                                    " -o \"\" -ae 1 -uc all -n " + shotMaskExprName + "; ", false, true );
}


//*********************************************************
// Name: generateFrameDigitArray
// Desc: Converts the current frame into an int array where
//       index 0 represents the 1s column, index 2 represents
//       the 10s column, etc...
//*********************************************************
MStatus ShotMaskCommand::generateFrameDigitArray()
{
    MTime currentTime = MAnimControl::currentTime();
    double currentFrame = currentTime.value();

    MString frameStr;
    frameStr.set( currentFrame );
    
    unsigned int frameNumLength = frameStr.length();

    // If the string has no length there was an error getting
    // the current time
    if( frameNumLength == 0 ) {
        pluginError( "ShotMaskCommand", "getCurrentDigit", "Couldn't get current frame" );
    }

    MIntArray resultArray( 4, 0 );
    MString singleDigitStr;

    for( unsigned int i = 0; i < frameNumLength; i++ ) {
        singleDigitStr = frameStr.substring(i, i);
        resultArray[frameNumLength - 1 - i] = singleDigitStr.asInt();
    }

    setResult( resultArray );

    return MS::kSuccess;
}

//*********************************************************
// Name: getKeyType
// Desc: Finds the key type at the current frame and sets
//       the result to the type "none", "key", "breakdown"
//*********************************************************
MStatus ShotMaskCommand::getKeyType()
{
    MStatus status = MS::kSuccess;

    MObject dependNode;
    MPlugArray plugArray;

    // Create an iterator to traverse the selection list
    MItSelectionList sIter( objList, MFn::kInvalid, &status );
    if( !status ) {
        pluginError( "ShotMaskCommand", "getKeyType", "Failed to creation SL iterator" );
    }
    else {
        // Traverse all of the dependency nodes for the selected objects
        for( ; !sIter.isDone(); sIter.next() ) {          

            // Get the current dependency node
            if( !sIter.getDependNode( dependNode )) {
                pluginError( "ShotMaskCommand", "getKeyType", "Couldn't get dependency node" );
                status = MS::kFailure;
                break;
            }
            
            // The call to get connections doesn't clear the array
            // so me must do it manually
            plugArray.clear();

            // Get all of the connections to this dependency node
            MFnDependencyNode dependFn( dependNode );
            if( !dependFn.getConnections( plugArray )) {
                // No connections means no animCurves
                // therefore, no keys
                break;
            }

            // Generate the list from the plugs
            if( !getKeyTypeFromPlugArray( plugArray )) {
                pluginError( "ShotMaskCommand", "getKeyType", "Failed to create list from Plugs" );
                status = MS::kFailure;
                break;
            }

            // If one attribute is a breakdown (tickDrawSpecial == true)
            // we are done.  When tds is true for one attribute, the
            // tick color will aways be green.
            if( resultStr == "breakdown" )
                break;
        }
    }

    // Set the result
    setResult( resultStr );

    return status;
}

//*********************************************************
// Name: getKeyTypeFromPlugArray
// Desc: Finds the key type at the current frame and sets
//       the result to the type "none", "key", "breakdown"
//*********************************************************
MStatus ShotMaskCommand::getKeyTypeFromPlugArray( MPlugArray plugArray )
{
    MStatus status = MS::kSuccess;

    // Check each connection for an upstream connection that is a kAnimCurve
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
                pluginError( "ShotMaskCommand", "getKeyTypeFromPlugArray", "DG Iterator error" );
            }
            else {
                // Retrieve the anim curve function sets
                // and store them in the list
                for( ; !dgIter.isDone(); dgIter.next() )
                {
                    MObjectArray nodePath;
                    dgIter.getNodePath( nodePath );
                    
                    // Only use the anim curves directly connected to the attributes
                    // >1 on a breath first search is level 2.  Root will always be
                    // first in the path
                    if( nodePath.length() > 2 )
                        break;

                    MObject anim = dgIter.thisNode( &status );
                    MFnAnimCurve animCurveFn( anim, &status );

                    if( !status ) {
                        pluginError( "ShotMaskCommand", "getKeyTypeFromPlugArray", "Can't get AnimCurve function set" );
                    }
                    else {
                        // Get the value of the key from the anim curve
                        if( !(status = getKeyTypeFromCurve( animCurveFn ))) {
                            pluginError( "ShotMaskCommand", "getKeyTypeFromPlugArray", "Failed to get key type" );
                        }
                    }
                }
            }
        }
    }

    return status;
}


//*********************************************************
// Name: getKeyTypeFromCurve
// Desc: Finds the key type on a given anim curve
//*********************************************************
MStatus ShotMaskCommand::getKeyTypeFromCurve(MFnAnimCurve &animCurveFn )
{
    MStatus status = MS::kSuccess;

    MPlug tdsPlugArray;
    MPlug tdsPlug;

    bool tds;
    
    int logicalIndex = getKeyLogicalIndex( animCurveFn, &status );

    if( logicalIndex >= 0 && (status == MS::kSuccess) ) 
    {
        // Get the Plug array for keyTickDrawSpecial
         tdsPlugArray = animCurveFn.findPlug( "keyTickDrawSpecial", &status );
        if( !status ) {
            pluginError( "ShotMaskCommand", "getKeyTypeFromCurve", "No MPlug with name keyTickDrawSpecial" );
        }
        else {
            // Get the specific plug for keyTickDrawSpecial at the current time
            tdsPlug = tdsPlugArray.elementByLogicalIndex( logicalIndex, &status );
            if( !status ) {
                pluginError( "ShotMaskCommand", "getKeyTypeFromCurve", "Failed to get element by logical index" );
            }
            else {
                tdsPlug.getValue( tds );
                if( tds )
                    resultStr = "breakdown";
                else
                    resultStr = "key";
            }
        }
    }
    

    return status;
}

//*********************************************************
// Name: getKeyLogicalIndex
// Desc: Returns the logical index of the key 
//       at the time when the command was called
//*********************************************************
int ShotMaskCommand::getKeyLogicalIndex( MFnAnimCurve &animCurveFn, MStatus *status )
{
    MTime currentTime = MAnimControl::currentTime();

    // Returns -1 if there isn't a key set at the playhead time
    int logicalIndex = -1;

    unsigned int closestIndex = animCurveFn.findClosest( currentTime, status );
    if( !(*status)) {
        pluginError( "ShotMaskCommand", "getKeyLogicalIndex", "Couldn't find closest key" );
    }
    else {
        // if the times match, the indexes are the same
        if( currentTime == animCurveFn.time( closestIndex, status ) )
            logicalIndex = (int)closestIndex;           
    }

    return logicalIndex;

}