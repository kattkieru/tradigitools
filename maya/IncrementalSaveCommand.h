//*********************************************************
// IncrementalSaveCommand.h
//
// Copyright (C) 2007-2021 Skeletal Studios
// All rights reserved.
//
//*********************************************************

#ifndef __INCREMENTAL_SAVE_COMMAND_H_
#define __INCREMENTAL_SAVE_COMMAND_H_

//*********************************************************
#include <maya/MPxCommand.h>

#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MFileIO.h>
//*********************************************************

//*********************************************************
// Class: IncrementalSaveCommand
//
// Desc: Incrementally saves the current file
//
// Command: cieIncrementalSave
//
// Flags: none
//
//*********************************************************
class IncrementalSaveCommand : public MPxCommand
{
private:


public:
    IncrementalSaveCommand();
    ~IncrementalSaveCommand();

    // Performs the command
    virtual MStatus doIt( const MArgList &args );

    // Allocates a command object to Maya (required)
    static void *creator() { return new IncrementalSaveCommand; }

};

#endif