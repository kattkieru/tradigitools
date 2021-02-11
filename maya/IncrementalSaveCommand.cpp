//*********************************************************
// IncrementalSaveCommand.cpp
//
// Copyright (C) 2007-2021 Skeletal Studios
// All rights reserved.
//
//*********************************************************

//*********************************************************
#include "IncrementalSaveCommand.h"
#include "ErrorReporting.h"
//*********************************************************

//*********************************************************
// Name: IncrementalSaveCommand
// Desc: Constructor
//*********************************************************
IncrementalSaveCommand::IncrementalSaveCommand()
{
    pluginTrace( "IncrementalSaveCommand", "IncrementalSaveCommand", "******* Incremental Save Command *******" );
}

//*********************************************************
// Name: IncrementalSaveCommand
// Desc: Destructor
//*********************************************************
IncrementalSaveCommand::~IncrementalSaveCommand()
{

}

//*********************************************************
// Name: doIt
// Desc: Handles all of the complete incremental save
//       process.  This command is not undoable.
//*********************************************************
MStatus IncrementalSaveCommand::doIt( const MArgList &args )
{
    MStatus status = MS::kFailure;
    MString output( "" );
    MString incrExtension( "" );

    MString extensionSeparator( "." );

    // This is the full file name and path
    MString absFileName = MFileIO::currentFile();

    MStringArray parsedAbsFileName;
    absFileName.split( '/', parsedAbsFileName );

    unsigned int absArraySize = parsedAbsFileName.length();

    // Get the actual filename from the parsed string array
    MString fileNameAndExtension = parsedAbsFileName[absArraySize - 1];

    pluginTrace( "IncrementalSaveCommand", "~doIt", output + "filename & extension: " + fileNameAndExtension );

    if( fileNameAndExtension == "untitled" ) {
        status = MGlobal::executeCommand( "SaveSceneAs" );
    }
    else {
        // Get the extension (must be .ma or .mb)
        MStringArray parsedFileName;
        fileNameAndExtension.split( '.', parsedFileName );

        MString fileExtension = parsedFileName[parsedFileName.length() - 1];
        if( fileExtension != "ma" && fileExtension != "mb" ) {
            MGlobal::displayError( "Invalid file type (not .ma or .mb)" );
        }
        else {
            bool addNewExtension = false;

            // Piece the absolute path back together
            // Ignore the last element, this is the filename
            MString absPath( "" );

#ifdef MAC_PLUGIN
            absPath += "/";
#endif

            for( unsigned int i = 0; i < absArraySize - 1; i++ ) {
                absPath += parsedAbsFileName[i];
                absPath += "/";
            }
            pluginTrace( "IncrementalSaveCommand", "~doIt", output + "Absolute Path: " + absPath );

            // Knock off the last 3 characters (the file extension) for the filename
            MString fileName = fileNameAndExtension.substring( 0, fileNameAndExtension.length() - 4 );
            pluginTrace( "IncrementalSaveCommand", "~doIt", output + "filename: " + fileName );

            parsedFileName.clear();
            fileName.split( (extensionSeparator.asChar())[0], parsedFileName );
            
            // Determine if there is already an increment extension
            // Are there at least two parts to the parsed filename
            // and, can the last element be converted to an int (only 0-9)
            if( parsedFileName.length() < 2 ||
                !(parsedFileName[parsedFileName.length() - 1].isInt()) )
            {
                // If not, give it a 3-digit extension starting at 000
                incrExtension = MString( "000" );

                addNewExtension = true;
            }
            else {
                // Determine the current increment and the number of placeholders
                incrExtension = parsedFileName[parsedFileName.length() - 1];

                unsigned int numPlaceholders = incrExtension.length();
                int incrAsInt = incrExtension.asInt();

                // increment the value and convert back
                incrAsInt++;
                incrExtension.set( incrAsInt, 0 );

                // add back the appropriate number of placeholders
                while( incrExtension.length() < numPlaceholders ) {
                    MString padding( "0" );
                    incrExtension = padding + incrExtension;
                }
            }

            // Rebuild the full filename and path ignore the parsed
            // Leading underscores to the filename will be ignored
            // Always include the 0-index element to accomodate
            // files that didn't originally have an extension
            absPath += parsedFileName[0];
            absPath += extensionSeparator;

            for( unsigned int i = 1; i < parsedFileName.length() - 1; i++ ) {
                absPath += parsedFileName[i];
                absPath += extensionSeparator;
            }
            // When a new extension is created we must add back
            // the last part of the file name (this would be the
            // increment extension if it had existed)
            if( addNewExtension && parsedFileName.length() > 1 ) {
                absPath += parsedFileName[parsedFileName.length() - 1];
                absPath += extensionSeparator;
            }
  
            // add the extensions back
            absPath += incrExtension;
            absPath += ".";
            absPath += fileExtension;

            pluginTrace( "IncrementalSaveCommand", "~doIt", output + "Full New Path: " + absPath );
            
            // File type is needed when saving
            MString fileType( "" );
            if( fileExtension == "ma" )
                fileType.set( "mayaAscii" );
            else
                fileType.set( "mayaBinary" );

            status = MFileIO::saveAs( absPath, fileType.asChar(), true );

            // Output the new path
            if( status ) {
                MString result( "Result: " );
                MGlobal::displayInfo( result + absPath );
            }
        }
    }

    return status;
}