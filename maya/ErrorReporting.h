//*********************************************************
// ErrorReporting.h
//
//
//*********************************************************

#ifndef __ERROR_REPORTING_H_
#define __ERROR_REPORTING_H_

//*********************************************************
#include <iostream>
#include <maya/MString.h>
//*********************************************************

//*********************************************************
// Constants
//*********************************************************
#ifdef _DEBUG
static const char *__pluginName = "[ANIMToolbox]";
#endif
//*********************************************************
// Functions
//*********************************************************

//*********************************************************
// Func: pluginErrorMsg
//
// Desc: Helper function for displaying error messages
//       to Maya's output window.
//*********************************************************
inline void pluginErrorMsg( const MString &className,
                            const MString &methodName,
                            const MString &msg )
{
#ifdef _DEBUG
    MString txt( __pluginName );
    txt += "ERROR: ";
    txt += "(";
    txt += className;
    txt += "::";
    txt += methodName;
    txt += ") ";
    txt += msg;
    

    std::cout << txt.asChar() << std::endl;
#endif
}

//*********************************************************
// Func: pluginWarningMsg
//
// Desc: Helper function for displaying warning messages
//       to Maya's output window.
//*********************************************************
inline void pluginWarningMsg( const MString &className,
                              const MString &methodName,
                              const MString &msg )
{
#ifdef _DEBUG
    MString txt( __pluginName );
    txt += "WARNING: ";
    txt += "(";
    txt += className;
    txt += "::";
    txt += methodName;
    txt += ") ";
    txt += msg;

    std::cout << txt.asChar() << std::endl;
#endif
}

//*********************************************************
// Func: pluginTraceMsg
//
// Desc: Helper function for displaying trace messages
//       to Maya's output window.
//*********************************************************
inline void pluginTraceMsg( const MString &className,
                            const MString &methodName,
                            const MString &msg )
{
#ifdef _DEBUG
    MString txt( __pluginName );
    txt += "Trace: ";
    txt += "(";
    txt += className;
    txt += "::";
    txt += methodName;
    txt += ") ";
    txt += msg;

    std::cout << txt.asChar() << std::endl;
#endif
}


//*********************************************************
// MACROS
//*********************************************************
#define pluginError( className, methodName, msg ) \
    { \
        pluginErrorMsg( className, methodName, msg ); \
    }

#define pluginWarning( className, methodName, msg ) \
    { \
        pluginWarningMsg( className, methodName, msg ); \
    }

#define pluginTrace( className, methodName, msg ) \
    { \
        pluginTraceMsg( className, methodName, msg ); \
    }



#endif