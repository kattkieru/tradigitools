//*********************************************************
// ANIMToolsUI.h
//
// Copyright (C) 2007-2021 Skeletal Studios
// All rights reserved.
//
//*********************************************************

#ifndef __ANIMTOOLS_UI_H_
#define __ANIMTOOLS_UI_H_

//*********************************************************
#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
//*********************************************************

//*********************************************************
// Class: ANIMToolsUI
//
// Desc:
//
//*********************************************************
class ANIMToolsUI
{
private:

    // The name assigned to the menu item
	MString animToolsMenuName;

    // The label of the menu item
	MString animToolsMenuLabel;		

    // The full pathname to the menu item
	MString animToolsMenuItemPath;

    // The full pathname to the menu divider
	MString animToolsMenuDividerPath;	

public:
    // Constructor/Destructor
	ANIMToolsUI();
	~ANIMToolsUI();

    // Adds the ANIMTools menu item to Maya's main menu
	MStatus addMenuItems();

    // Removes the ANIMTools menu item from Maya's main menu
	MStatus removeMenuItems();

    // Removes the ANIMTools UIs
    MStatus deleteUI();

};




#endif