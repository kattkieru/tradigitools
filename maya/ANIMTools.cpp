//*********************************************************
// ANIMTools.cpp
//
// Copyright (C) 2007-2013 Circus Ink Entertainment
// All rights reserved.
//
//*********************************************************

//*********************************************************
#include <maya/MFnPlugin.h>

#include "ANIMToolsUI.h"

#include "AboutCommand.h"
#include "BreakdownCommand.h"
#include "SetKeyCommand.h"
#include "RetimingCommand.h"
#include "IncrementalSaveCommand.h"
#include "ShotMaskCommand.h"
#include "CurveCleanerCommand.h"

#include "ErrorReporting.h"

// #include "config.h"

//*********************************************************

//*********************************************************
// Globals
//*********************************************************
ANIMToolsUI g_animToolsUI;

//*********************************************************
// Constants
//*********************************************************
// The version number being built
const char* versionNumber = "1.3a";

const char *aboutCmdName = "cieAbout";

const char *insertBreakdownCmdName = "cieInsertBreakdown";
const char *setKeyCmdName = "cieSetKeyframe";
const char *retimingCmdName = "cieRetiming";
const char *incrementalSaveCmdName = "cieIncrementalSave";

const char *shotMaskCmdName = "cieShotMask";
const char *curveCleanerCmdName = "cieCleanCurves";

//*********************************************************
// Functions
//*********************************************************
MStatus initializePlugin( MObject obj );
MStatus uninitializePlugin( MObject obj );
MStatus registerCommands( MObject obj );
MStatus deregisterCommands( MObject obj );

//*********************************************************
// Name: initializePlugin
// Desc: Register the commands, devices, etc..., 
//       defined by the plugin, with Maya.
//*********************************************************
MStatus initializePlugin( MObject obj )
{
    MStatus status = MS::kSuccess;

    pluginTrace( "ANIMTools", "initializePlugin", "Initializing ANIMToolbox" );

    // Create the function set for registering and deregistering the plugin
	// MFnPlugin animToolsPlugin( obj, "FUNhouse Interactive", versionNumber, "Any", &status );
 //    if( !status ) {
	// 	pluginError( "ANIMTools", "initializePlugin", "Failed to initialize" );
	// } else 

    // Register all new commands included with the plugin
    if( !registerCommands( obj )) {
        status = MS::kFailure;
        pluginError( "ANIMTools", "initializePlugin", "Failed to register commands" );
    }

    // Add the script path and source required scripts
    else {

        // Add the UI to Maya's menu
        if( !g_animToolsUI.addMenuItems()) {
            pluginError( "ANIMTools", "initializePlugin", "Failed to add menu items" );
        }

/* Legacy prior to using a Module and some helpful dev code
            MString pluginPath = animToolsPlugin.loadPath();

            MString scriptPath;
	        MGlobal::executeCommand( "getenv \"MAYA_SCRIPT_PATH\"", scriptPath );

            scriptPath += ";";
            scriptPath += pluginPath;
            scriptPath += "scripts";

            // pluginTrace( "ANIMTools", "initializePlugin", scriptPath );
            // MGlobal::displayInfo( scriptPath );
	        MGlobal::executeCommand( "putenv \"MAYA_SCRIPT_PATH\" \"" + scriptPath + "\"" );
*/

        // Don't source until after all commands have been registered
        // Otherwise Maya won't understand them (even if they've since been registered)
		// ##NEW Changed for Maya 2012.
        MString sourceStr = "source cie_tradigitools.mel;";
        MString errorMsg = "Failed to source cie_tradigitools.mel.";

        if( !MGlobal::executeCommand( sourceStr ) ) {
            pluginError( "ANIMTools", "initializePlugin", errorMsg);
        }

        // Launch the UI on Load
        else if( !MGlobal::executeCommand( "cie_tradigitools" ) ) {
            pluginError( "ANIMTools", "initializePlugin", "Failed to launch cie_tradigitools" );
        }
    }

	return status;
}

//*********************************************************
// Name: uninitializePlugin
// Desc: Deregister the commands, devices, etc..., 
//       defined by the plugin, from Maya
//*********************************************************
MStatus uninitializePlugin( MObject obj )
{
    MStatus status = MS::kSuccess;

    // Delete the shot mask if it exists
    MGlobal::executeCommand( "cieShotMask -cs" );

    // Kill any scriptJobs
    if( !MGlobal::executeCommand( "cie_atbKillScriptJobs" )) {
        pluginError( "ANIMTools", "uninitializePlugin", "Failed to kill scriptJobs" );
    }
    // Remove any plugin specific menu additons from Maya
    if( !g_animToolsUI.removeMenuItems()) {
		pluginError( "ANIMTools", "uninitializePlugin", "Failed to remove menu items" );
    }
    if( !g_animToolsUI.deleteUI()) {
        pluginError( "ANIMTools", "uninitializePlugin", "Failed to delete UI" );
    }

    if( !deregisterCommands( obj )) {
        status = MS::kFailure;
        pluginError( "ANIMTools", "uninitializePlugin", "Failed to Deregister Commands" );
    }

	return status;
}


//*********************************************************
// Name: registerCommands
// Desc: Registers all of the commands for the plugin
//*********************************************************
MStatus registerCommands( MObject obj )
{
    MStatus status = MS::kSuccess;
    MString errorMsg( "Failed to Register Command: " );

    MFnPlugin pluginFn(obj, "FUNhouse Interactive", versionNumber, "Any", &status );

    // Register the insert breakdown command
    if( !pluginFn.registerCommand( insertBreakdownCmdName,
                                   BreakdownCommand::creator,
                                   BreakdownCommand::newSyntax ))
    {
        status = MS::kFailure;
        pluginError( "ANIMTools", "registerCommands", errorMsg + insertBreakdownCmdName );
    }
    // Register the set key command
    else if( !pluginFn.registerCommand( setKeyCmdName,
                                        SetKeyCommand::creator,
                                        SetKeyCommand::newSyntax ))
    {
        status = MS::kFailure;
        pluginError( "ANIMTools", "registerCommands", errorMsg + setKeyCmdName );
    }

    // Register the retiming command
    else if( !pluginFn.registerCommand( retimingCmdName,
                                        RetimingCommand::creator,
                                        RetimingCommand::newSyntax ))
    {
        status = MS::kFailure;
        pluginError( "ANIMTools", "registerCommands", errorMsg + retimingCmdName );
    }

    // Register the incremental save command
    else if( !pluginFn.registerCommand( incrementalSaveCmdName,
                                        IncrementalSaveCommand::creator ))
    {
        status = MS::kFailure;
        pluginError( "ANIMTools", "registerCommands", errorMsg + incrementalSaveCmdName );
    }

    // Register the camera overlay command
    else if( !pluginFn.registerCommand( shotMaskCmdName,
                                        ShotMaskCommand::creator,
                                        ShotMaskCommand::newSyntax ))
    {
        status = MS::kFailure;
        pluginError( "ANIMTools", "registerCommands", errorMsg + shotMaskCmdName );
    }

    // Register the curve cleaner command
    else if( !pluginFn.registerCommand( curveCleanerCmdName,
                                        CurveCleanerCommand::creator,
                                        CurveCleanerCommand::newSyntax ))
    {
        status = MS::kFailure;
        pluginError( "ANIMTools", "registerCommands", errorMsg + curveCleanerCmdName );
    }

    // Register the about command
    else if( !pluginFn.registerCommand( aboutCmdName,
                                        AboutCommand::creator,
                                        AboutCommand::newSyntax ))
    {
        status = MS::kFailure;
        pluginError( "ANIMTools", "registerCommands", errorMsg + aboutCmdName );
    }

    return status;
}


//*********************************************************
// Name: deregisterCommands
// Desc: Deregisters all of the commands for the plugin
//*********************************************************
MStatus deregisterCommands( MObject obj )
{
    MStatus status = MS::kSuccess;
    MString errorMsg( "Failed to Deregister Command: " );

    // Deregister all plugin specific commands from Maya
    MFnPlugin pluginFn(obj);

    // Deregister the insert breakdown command
    if( !pluginFn.deregisterCommand( insertBreakdownCmdName ))
    {
        status = MS::kFailure;
        pluginError( "ANIMTools", "deregisterCommands", errorMsg + insertBreakdownCmdName );
    }
    // Deregister the set key command
    if( !pluginFn.deregisterCommand( setKeyCmdName ))
    {
        status = MS::kFailure;
        pluginError( "ANIMTools", "deregisterCommands", errorMsg + setKeyCmdName );
    }
    // Deregister the retiming command
    if( !pluginFn.deregisterCommand( retimingCmdName ))
    {
        status = MS::kFailure;
        pluginError( "ANIMTools", "deregisterCommands", errorMsg + retimingCmdName );
    }

    // Deregister the incremental save command
    if( !pluginFn.deregisterCommand( incrementalSaveCmdName ))
    {
        status = MS::kFailure;
        pluginError( "ANIMTools", "deregisterCommands", errorMsg + incrementalSaveCmdName );
    }

    // Deregister the incremental save command
    if( !pluginFn.deregisterCommand( shotMaskCmdName ))
    {
        status = MS::kFailure;
        pluginError( "ANIMTools", "deregisterCommands", errorMsg + shotMaskCmdName );
    }

    // Deregister the curve cleaner command
    if( !pluginFn.deregisterCommand( curveCleanerCmdName ))
    {
        status = MS::kFailure;
        pluginError( "ANIMTools", "deregisterCommands", errorMsg + curveCleanerCmdName );
    }

    // Deregister the about command
    if( !pluginFn.deregisterCommand( aboutCmdName ))
    {
        status = MS::kFailure;
        pluginError( "ANIMTools", "deregisterCommands", errorMsg + aboutCmdName );
    }

    return status;
}
