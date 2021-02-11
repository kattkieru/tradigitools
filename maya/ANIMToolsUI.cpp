//*********************************************************
// ANIMToolsUI.cpp
//
// Copyright (C) 2007-2021 Skeletal Studios
// All rights reserved.
//
//*********************************************************

//*********************************************************
#include "ANIMToolsUI.h"
//*********************************************************

//*********************************************************
// Name: ANIMToolsUI
// Desc: Constructor
//*********************************************************
ANIMToolsUI::ANIMToolsUI()
{
	animToolsMenuName = "cie_tradigitoolsMenu";
	animToolsMenuLabel = "tradigiTOOLS...";
}

//*********************************************************
// Name: ~ANIMToolsUI
// Desc: Destructor
//*********************************************************
ANIMToolsUI::~ANIMToolsUI()
{

}

//*********************************************************
// Name: addMenuItems
// Desc: Adds the ANIMTools menu item to Maya's main menu.
//*********************************************************
MStatus ANIMToolsUI::addMenuItems()
{
    MGlobal::executeCommand( "menu -l \"Tradigitools\" -to true -parent $gMainWindow TradigitoolsMainMenu", animToolsMenuName );
    MGlobal::executeCommand( "menuItem -l \"" + animToolsMenuLabel + "\" -c \"cie_tradigitools()\" " );

	return MS::kSuccess;

}


//*********************************************************
// Name: removeMenuItems
// Desc: Removes the ANIMTools menu item to Maya's main menu.
//*********************************************************
MStatus ANIMToolsUI::removeMenuItems()
{
    MGlobal::executeCommand( "deleteUI -menu " + animToolsMenuName );

	return MS::kSuccess;
}

//*********************************************************
// Name: deleteUI
// Desc: Removes the ANIMTools UI
//*********************************************************
MStatus ANIMToolsUI::deleteUI()
{
    // Delete the About Window
    MGlobal::executeCommand( "if( `window -ex $g_cieAboutUIWindowName` ) deleteUI $g_cieAboutUIWindowName;" );

    // Delete the UI if it is up
    MGlobal::executeCommand( "if( `window -ex $g_cieMainUIWindowName` ) deleteUI $g_cieMainUIWindowName;" );

    return MS::kSuccess;
}