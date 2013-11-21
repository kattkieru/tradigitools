//*********************************************************
// AboutCommand.h
//
// Copyright (C) 2007 Circus Ink Entertainment
// All rights reserved.
//
//*********************************************************

#ifndef __ABOUT_COMMAND_H_
#define __ABOUT_COMMAND_H_

//*********************************************************
#include <maya/MPxCommand.h>

#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
//*********************************************************

//*********************************************************
// Class: AboutCommand
//
// Desc: Returns information related ANIMToolbox
//
// Command: cieAbout
//
// Flags: -name (-n)
//
//        -version (-v)
//
//        -copyright (-c)
//
//*********************************************************
class AboutCommand : public MPxCommand
{
private:
    // Constants for setting up the command's flags
    static const char *nameFlag, *nameLongFlag;
    static const char *versionFlag, *versionLongFlag;
    static const char *copyrightFlag, *copyrightLongFlag;

    // About Constants
    static const char *name;
    static const char *version;
    static const char *copyright;

    // Method to setup the command flags
    MStatus parseCommandFlags( const MArgList &args );

public:
    // Constructor/Destructor
    AboutCommand();
    ~AboutCommand();

    // Performs the command
    virtual MStatus doIt( const MArgList &args );

    // Indicates that Maya can undo/redo this command
    virtual bool isUndoable() const { return false; }

    // Allocates a command object to Maya (required)
    static void *creator() { return new AboutCommand; }

    // Defines the set of flags allowed by this command
    static MSyntax newSyntax();
};



#endif