//*********************************************************
// AboutCommand.cpp
//
// Copyright (C) 2007-2021 Skeletal Studios
// All rights reserved.
//
//*********************************************************

//*********************************************************
#include "AboutCommand.h"
#include "ErrorReporting.h"
//*********************************************************

//*********************************************************
// Constants
//*********************************************************
const char* AboutCommand::nameFlag = "-n";
const char* AboutCommand::nameLongFlag = "-name";
const char* AboutCommand::versionFlag = "-v";
const char* AboutCommand::versionLongFlag = "-version";
const char* AboutCommand::copyrightFlag = "-c";
const char* AboutCommand::copyrightLongFlag = "-copyright";

const char* AboutCommand::name = "tradigitools";
const char* AboutCommand::version = "1.5";

const char* AboutCommand::copyright = "Copyright 2007-2021 Skeletal Studios";


//*********************************************************
// Name: AboutCommand
// Desc: Constructor
//*********************************************************
AboutCommand::AboutCommand()
{

}

//*********************************************************
// Name: ~AboutCommand
// Desc: Destructor
//*********************************************************
AboutCommand::~AboutCommand()
{

}

//*********************************************************
// Name: doIt
// Desc: All of the one-time setup and initialization
//       code for the set key command.  doIt is called
//       by Maya when any command is executed in MEL.
//*********************************************************
MStatus AboutCommand::doIt( const MArgList &args )
{
    parseCommandFlags( args );

    return MS::kSuccess;
}

//*********************************************************
// Name: newSyntax
// Desc: Method for registering the command flags
//       with Maya
//*********************************************************
MSyntax AboutCommand::newSyntax()
{
    // Use MSyntax for a more robust solution to the
    // parsing the command flags
    MSyntax syntax;
    
    syntax.addFlag( nameFlag, nameLongFlag, MSyntax::kNoArg );
    syntax.addFlag( versionFlag, versionLongFlag, MSyntax::kNoArg );
    syntax.addFlag( copyrightFlag, copyrightLongFlag, MSyntax::kNoArg );

    return syntax;
}

//*********************************************************
// Name: parseCommandFlags
// Desc: Parse the command flags and stores the values
//       in the appropriate variables
//*********************************************************
MStatus AboutCommand::parseCommandFlags( const MArgList &args )
{
    MArgDatabase argData( syntax(), args );

    // No flags set returns the name
    if( argData.isFlagSet( versionFlag ))
        setResult( version );
    else if( argData.isFlagSet( copyrightFlag ))
        setResult( copyright );
    else
        setResult( name );

    return MS::kSuccess;
}