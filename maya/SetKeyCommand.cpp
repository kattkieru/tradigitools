//*********************************************************
// SetKeyCommand.cpp
//
// Copyright (C) 2007 Circus Ink Entertainment
// All rights reserved.
//
//*********************************************************

//*********************************************************
#include "SetKeyCommand.h"
#include "ErrorReporting.h"
//*********************************************************

//*********************************************************
// Constants
//*********************************************************
const char *SetKeyCommand::editFlag = "-e";
const char *SetKeyCommand::editLongFlag = "-edit";
const char *SetKeyCommand::ignoreUnkeyedFlag = "iuk";
const char *SetKeyCommand::ignoreUnkeyedLongFlag = "ignoreUnkeyed";
const char *SetKeyCommand::tickDrawSpecialFlag = "-tds";
const char *SetKeyCommand::tickDrawSpecialLongFlag = "-tickDrawSpecial";


//*********************************************************
// Name: SetKeyCommand
// Desc: Constructor
//*********************************************************
SetKeyCommand::SetKeyCommand()
{
    pluginTrace( "SetKeyCommand", "SetKeyCommand", "******* Set Key Command *******" );

    // Initialize the command flag defaults
    editMode = false;
    tickDrawSpecial = false;
    ignoreUnkeyed = false;

    // Get the playhead time when the command is called
    originalPlayheadTime = MAnimControl::currentTime();

    initialized = false;
    tickDrawSpecialCount = 0;
    animCurveFnList.clear();
}


//*********************************************************
// Name: ~SetKeyCommand
// Desc: Destructor
//*********************************************************
SetKeyCommand::~SetKeyCommand()
{
    // Cleanup all memory allocated for this command
    if( !animCurveFnList.empty()) {
        animCurveListIter = animCurveFnList.begin();

        while( animCurveListIter != animCurveFnList.end() ) {
            // pluginTrace( "SetKeyCommand", "~SetKeyCommand", "Deleting element from list" );

            delete (*animCurveListIter).pAnimCurveFn;
            delete (*animCurveListIter).pAnimCurveCache;
            animCurveListIter++;
        }
    }
}

//*********************************************************
// Name: doIt
// Desc: All of the one-time setup and initialization
//       code for the set key command.  doIt is called
//       by Maya when any command is executed in MEL.
//       Any code that changes the state of Maya is
//       handled by the redoIt method.
//*********************************************************
MStatus SetKeyCommand::doIt( const MArgList &args )
{
    MStatus status = MS::kFailure;
    
    // Set the command flag values appropriately
    if( !parseCommandFlags( args )) {
        pluginError( "SetKeyCommand", "doIt", "Failed to parse command flags" );
    }
    // Get a list of the currently selected objects
    else if( !getSelectedObjects() ) {
        pluginError( "SetKeyCommand", "doIt", "Failed to get selected objects" );
    }
    // When in create mode, set a keyframe first at the current frame
    // setKeys handles getting animCurveFnList when not in edit mode
    else if( !editMode && !setKeys() ) {
        pluginError( "SetKeyCommand", "doIt", "Failed to set keyframe" );
    }
    // Get a list of all of the anim curve function sets
    // edit mode must take care of gettting animCurveFnList
    else if( editMode && !getAnimCurveFnList() ) {
        pluginError( "SetKeyCommand", "doIt", "Failed to create AnimCurveFnList" );
    }
    else {
        // Execute all operations that change the state of Maya
        if( !(status = redoIt() )) {
            pluginError( "SetKeyCommand", "doIt", "Failed to redoIt" );
        }
        else {
            MString result( "Result: " );
            MGlobal::displayInfo( result + tickDrawSpecialCount );
        }
    }

    return status;
}

//*********************************************************
// Name: redoIt
// Desc: Contains the code that changes the internal state
//       of Maya.  It is called by Maya to redo.
//*********************************************************
MStatus SetKeyCommand::redoIt()
{
    MStatus status = MS::kSuccess;

    // In doIt, when the command is executed, the anim curve cache is
    // created.  All subsequent call to redo it will use the cache
    if( initialized == false )
        initialized = true;
    else {
        if( !animCurveFnList.empty()) {
            animCurveListIter = animCurveFnList.begin();

            while( animCurveListIter != animCurveFnList.end() ) {
                (*animCurveListIter).pAnimCurveCache->redoIt();
                animCurveListIter++;
            }
        }
    }
    

    // Modify the keyTickDrawSpecial (tick color) attributes
    if( !(status = setTickDrawSpecial( false ) )) {
        pluginError( "SetKeyCommand", "redoIt", "Failed to setTickDrawSpecial" );
    }

    // If no keys have been affected, there would have been
    // no keys set at the current time
    if( tickDrawSpecialCount == 0 ) {
        MGlobal::displayError( "No keys set at the current time" );
        status = MS::kFailure;
    }

    return status;
}


//*********************************************************
// Name: undoIt
// Desc: Contains the code to undo the internal state 
//       changes made by the breakdown command (redoIt).
//       It is called by Maya to undo.
//*********************************************************
MStatus SetKeyCommand::undoIt()
{
    MStatus status = MS::kSuccess;

    // Traverse the curve cache and restore things to 
    // the way they were
    if( !animCurveFnList.empty()) {
        animCurveListIter = animCurveFnList.begin();

        while( animCurveListIter != animCurveFnList.end() ) {
            (*animCurveListIter).pAnimCurveCache->undoIt();
            animCurveListIter++;
        }
    }

    // Restore the keyTickDrawSpecial (tick color) attributes
    if( !(status = setTickDrawSpecial( true ) )) {
        pluginError( "SetKeyCommand", "undoIt", "Failed to setTickDrawSpecial" );
    }

    return status;
}

//*********************************************************
// Name: newSyntax
// Desc: Method for registering the command flags
//       with Maya
//*********************************************************
MSyntax SetKeyCommand::newSyntax()
{
    // Use MSyntax for a more robust solution to the
    // parsing the command flags
    MSyntax syntax;
    
    syntax.addFlag( ignoreUnkeyedFlag, ignoreUnkeyedLongFlag, MSyntax::kBoolean );
    syntax.addFlag( tickDrawSpecialFlag, tickDrawSpecialLongFlag, MSyntax::kBoolean );

    syntax.enableEdit();

    return syntax;
}

//*********************************************************
// Name: parseCommandFlags
// Desc: Parse the command flags and stores the values
//       in the appropriate variables
//*********************************************************
MStatus SetKeyCommand::parseCommandFlags( const MArgList &args )
{
    MStatus status = MS::kSuccess;

    // Create an object to parse the arguments
    // and then parse them
    MArgDatabase argData( syntax(), args, &status );
    if( !status ) {
        pluginError( "SetKeyCommand", "parseCommandFlags",
                     "Failed to create MArgDatabase for the set key command" );
    }
    else {
        editMode = argData.isEdit();

        if( argData.isFlagSet( ignoreUnkeyedFlag ))
            argData.getFlagArgument( ignoreUnkeyedFlag, 0, ignoreUnkeyed );

        if( argData.isFlagSet( tickDrawSpecialFlag ))
            argData.getFlagArgument( tickDrawSpecialFlag, 0, tickDrawSpecial );
    }

    return status;
}

//*********************************************************
// Name: getSelectedObjects
// Desc: Generates a list of all the selected
//       objects
//*********************************************************
MStatus SetKeyCommand::getSelectedObjects()
{
    MStatus status = MS::kFailure;
    MSelectionList characterSetList;
    
    selectionList.clear();
    
    // Selected Objects Include:
    //    1) The Active Character Set (if in use) and its subset
    //    2) Character Sets selected by the user (and their subsets)
    //    3) Objects selected by the user 

    // Get the character set if active (in Maya) and the associated character set subsets
    MStringArray characterSets;
    MGlobal::executeCommand( MString("cie_atbGetActiveCharacterSets"), characterSets, false, false );
    pluginTrace( "SetKeyCommand", "getSelectedObjects", MString("Count: ") + characterSets.length() );

    // Create a selection list with all the active character set/subsets
    for( unsigned int i = 0; i < characterSets.length(); i++ ) {
        MGlobal::getSelectionListByName( characterSets[i], characterSetList );
    }

    // Get the character sets selected (by user) and the associated character set subsets 
    MGlobal::executeCommand( MString("cie_atbGetSelectedCharacterSets"), characterSets, false, false);
    pluginTrace( "SetKeyCommand", "getSelectedObjects", MString("Count: ") + characterSets.length() );

    // Append to the selection list with all the selected character set/subsets
    for( unsigned int i = 0; i < characterSets.length(); i++ ) {
        MGlobal::getSelectionListByName( characterSets[i], characterSetList );
    }

    pluginTrace( "SetKeyCommand", "getSelectedObjects", MString("SL Count: ") + characterSetList.length() );

    // Retrive all of the currently selected objects
    if( !MGlobal::getActiveSelectionList( selectionList )) {
        pluginError( "SetKeyCommand", "getSelectedObjects", "Failed to get active selection list" );
    }
    // At least one object must be selected for this command
    else if( characterSetList.length() == 0 && selectionList.length() == 0 ) {
        pluginError( "SetKeyCommand", "getSelectedObjects", "No Objects Selected" );
        MGlobal::displayError( "No Objects Selected" );
    }
    else
        status = MS::kSuccess;

    selectionList.merge( characterSetList );

    pluginTrace( "SetKeyCommand", "getSelectedObjects", MString("Merged Count: ") + selectionList.length() );

    return status;
}


//*********************************************************
// Name: getAnimCurveFnList
// Desc: Generates a list of anim curve function sets to
//       be operated on
//*********************************************************
MStatus SetKeyCommand::getAnimCurveFnList()
{
    MStatus status = MS::kSuccess;

    MObject dependNode;
    MPlugArray plugArray;

    // Create an iterator to traverse the selection list
    MItSelectionList sIter( selectionList, MFn::kInvalid, &status );
    if( !status ) {
        pluginError( "SetKeyCommand", "getAnimCurveFnList", "Failed to creation SL iterator" );
    }
    else {
        // Traverse all of the dependency nodes for the selected objects
        for( ; !sIter.isDone(); sIter.next() ) {          

            // Get the current dependency node
            if( !sIter.getDependNode( dependNode )) {
                pluginError( "SetKeyCommand", "getAnimCurveFnList", "Couldn't get dependency node" );
                status = MS::kFailure;
                break;
            }
            
            // The call to get connections doesn't clear the array
            // so me must do it manually
            plugArray.clear();

            // Get all of the connections to this dependency node
            MFnDependencyNode dependFn( dependNode );
            if( !dependFn.getConnections( plugArray )) {
                pluginError( "SetKeyCommand", "getAnimCurveFnList", "Couldn't get connections" );
                MGlobal::displayWarning( "No keyable attributes" );
                status = MS::kFailure;
                break;
            }

            // Generate the list from the plugs
            if( !getAnimCurveFnListFromPlugs( plugArray )) {
                pluginError( "SetKeyCommand", "getAnimCurveFnList", "Failed to create list from Plugs" );
                status = MS::kFailure;
                break;
            }
        }
    }

    return status;
}

//*********************************************************
// Name: getAnimCurveFnListFromPlugs
// Desc: Generates a list of anim curve function sets from
//       an array of plugs
//*********************************************************
MStatus SetKeyCommand::getAnimCurveFnListFromPlugs( MPlugArray plugArray )
{
    MStatus status = MS::kSuccess;
    AnimCurveFnTDS animCurveFnTDS;

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
                pluginError( "SetKeyCommand", "getAnimCurveFnListFromPlugs", "DG Iterator error" );
            }
            else {
                // Retrieve the anim curve function sets
                // and store them in the list
                for( ; !dgIter.isDone(); dgIter.next() )
                {
                    MObjectArray nodePath;
                    dgIter.getNodePath( nodePath );

                    // At a depth of 1 in the DAG, the animation nodes are directly 
                    // connected to animated object.  However, if the depth is greater
                    // than one then we must accomodate both PairBlend nodes and 
                    // Character Set nodes (which sit between the transform node and
                    // the anim nodes.
                    int nodeParentIndex = 1;
                    if( nodePath.length() <= 2 || 
                        ( nodePath.length() == 3 && 
                            (nodePath[nodeParentIndex].apiType() == MFn::kPairBlend ||
                            nodePath[nodeParentIndex].apiType() == MFn::kCharacter ))
                        )
                    {
                        MObject anim = dgIter.thisNode( &status );
                        MFnAnimCurve *animCurveFn = new MFnAnimCurve( anim, &status );

                        // Avoid adding duplicate anim curves to the list
                        // Important when dealing with blend nodes
                        MString curveName = animCurveFn->name();
                        bool nameExists = false;

                        animCurveListIter = animCurveFnList.begin();
                        while( animCurveListIter != animCurveFnList.end() ) {
                            if( ((AnimCurveFnTDS)*animCurveListIter).pAnimCurveFn->name() == curveName ) {
                                nameExists = true;
                                break;
                            }
                            animCurveListIter++;
                        }


                        if( !nameExists ) {
                            if( !status ) {
                                pluginError( "SetKeyCommand", "getAnimCurveFnListFromPlugs", "Can't get AnimCurve function set" );
                                delete animCurveFn;
                            }
                            // If there are no problems, add the function set to the list
                            else {
                                animCurveFnTDS.pAnimCurveFn = animCurveFn;
                                animCurveFnTDS.pAnimCurveCache = new MAnimCurveChange();
                                animCurveFnTDS.previousTickDrawSpecial = false;
                                animCurveFnTDS.isBoolean = isBooleanDataType( plugArray[index] ); 
                                animCurveFnList.push_back( animCurveFnTDS );
                            }
                        }
                    }
                }
            }
        }
    }

    return status;
}

//*********************************************************
// Name: setKeys
// Desc: Sets a new key for each attribute (subject to
//       the flags) at the current time
//*********************************************************
MStatus SetKeyCommand::setKeys()
{
    MStatus status = MS::kSuccess;

    MStringArray characterSet;
    MGlobal::executeCommand( MString("currentCharacters"), characterSet, false, false );

    // If all attributes are to have keys set, just use Maya's
    // built in function. No reason to reinvent the wheel.
    if( !ignoreUnkeyed ) {

        MString melCommand = MString("setKeyframe -bd 0 -hi \"none\" -cp 0 -s 0");

        // Determine the current manipulator
        // For character sets if there are extra user selected objects
        // they are only keyed on attributes that match the current
        // manipulator
        MString manipType;
        MGlobal::executeCommand( "currentCtx", manipType, false, false );

        // Special rules apply when there is an active character set
        if( characterSet.length() > 0 )
        {
            // Only set keys on attributes for the active manipulator
            MString manipName = "";
            if( manipType == "moveSuperContext" )
                manipName = " -at translate";
            else if( manipType == "RotateSuperContext" )
                manipName = " -at rotate";
            else if( manipType == "scaleSuperContext" )
                manipName = " -at scale";

            if( manipName != "" ) {
                MGlobal::executeCommand(melCommand + manipName , false, true);
            }          
        }
        else
        {
            // Key selected objects
            MGlobal::executeCommand(melCommand , false, true);
        }

        // Key character set(s) if it is selected
        MStringArray characterSets;
        MGlobal::executeCommand( MString("cie_atbGetActiveCharacterSets"), characterSets, false, false );

        MString extendedCommand;
        for( unsigned int i = 0; i < characterSets.length(); i++ ) {
            extendedCommand = melCommand + " { \"" + characterSets[i] + "\" }";
            MGlobal::executeCommand(extendedCommand , false, true);
        }
    }
    else {
        // Keyframe only those attributes that have been keyed previously
        // NOTE: Does not support Character Sets
        status = MGlobal::executeCommand("string $selected[] = `ls -sl`; \
                                         for( $selObj in $selected ) { \
                                            string $animCurves[] = `keyframe -q -n $selObj`; \
                                            for( $curve in $animCurves ) \
                                               setKeyframe `listConnections -p 1 $curve`; \
                                         }", false, true );
    }

    // Get a list of all of the anim curve function sets
    if( !(status = getAnimCurveFnList()) ) {
        pluginError( "SetKeyCommand", "setKeys", "Failed to create AnimCurveFnList (!ignoreUnkeyed)" );
    }


    /*
    else {
        // Get a list of all of the anim curve function sets
        // Unlike above where we want a full list after all attributes
        // have been set (some may not have been keyed previously),
        // Here we need the list to set the keys, not just the final color
        if( !(status = getAnimCurveFnList()) ) {
            pluginError( "SetKeyCommand", "setKeys", "Failed to create AnimCurveFnList (ignoreUnkeyed)" );
        }

        // Traverse the list and set a key on each curve at the current time
        double value;

        if( !animCurveFnList.empty()) {
            animCurveListIter = animCurveFnList.begin();

            while( (animCurveListIter != animCurveFnList.end()) && (status == MS::kSuccess) ) {
                // Get the current value at the current time
                if( !(*animCurveListIter).pAnimCurveFn->evaluate( originalPlayheadTime, value )) {
                    pluginError( "SetKeyCommand", "setKeys", "Failed to evaluate the current value" );
                    status = MS::kFailure;
                }
                else {
                    // Use stepped out tangents for keys on boolean attributes
                    MFnAnimCurve::TangentType outTangent;

                    if( (*animCurveListIter).isBoolean )
                        outTangent = MFnAnimCurve::kTangentStep;
                    else
                        outTangent = MFnAnimCurve::kTangentGlobal;

                    // Add in a new key at the current time for the current value
                    (*animCurveListIter).pAnimCurveFn->addKey( originalPlayheadTime,
                                                               value,
                                                               MFnAnimCurve::kTangentGlobal,
                                                               outTangent,
                                                               (*animCurveListIter).pAnimCurveCache,
                                                               &status );
                }

                animCurveListIter++;
            }
        }
    }
    */

    return status;
}

//*********************************************************
// Name: setTickDrawSpecial
// Desc: Sets the color of the tick on the timeline
//*********************************************************
MStatus SetKeyCommand::setTickDrawSpecial( bool undo )
{
    MStatus status = MS::kSuccess;
    MPlug tdsPlugArray;
    MPlug tdsPlug;

    int logicalIndex;

    tickDrawSpecialCount = 0;

    // Traverse the animCurveFn list and update the
    // tick color accordingly
    animCurveListIter = animCurveFnList.begin();
    while( (animCurveListIter != animCurveFnList.end()) && (status == MS::kSuccess) )
    {
        // Get the logical index of the key at the current time (if it doesn't exist, skip)
        logicalIndex = getKeyLogicalIndex( (*(*animCurveListIter).pAnimCurveFn), &status );
        if( logicalIndex >= 0 && (status == MS::kSuccess) ) 
        {
            // Get the Plug array for keyTickDrawSpecial
            tdsPlugArray = (*animCurveListIter).pAnimCurveFn->findPlug( "keyTickDrawSpecial", &status );
            if( !status ) {
                pluginError( "SetKeyCommand", "setTickDrawSpecial", "No MPlug with name keyTickDrawSpecial" );
            }
            else {
                // Get the specific plug for keyTickDrawSpecial at the current time
                tdsPlug = tdsPlugArray.elementByLogicalIndex( logicalIndex, &status );
                if( !status ) {
                    pluginError( "SetKeyCommand", "setTickDrawSpecial", "Failed to get logical index" );
                }
                else {
                    // Update the tick color appropriately
                    // Are we redoing or undoing the command
                    if( !undo ) {
                        // Redo: Store the previous value for undoing and set the new value
                        tdsPlug.getValue( (*animCurveListIter).previousTickDrawSpecial );
                        tdsPlug.setValue( tickDrawSpecial );
                    }
                    else {
                        // Undo: Restore the previous value
                        tdsPlug.setValue( (*animCurveListIter).previousTickDrawSpecial );
                    }

                    tickDrawSpecialCount++;
                }
            }
        }
        // No need to proceed if something failed (get out of the loop)
        if( !status )
            break;
        
        animCurveListIter++;
    }

    MString count( "NumTicks colored: " );
    pluginTrace( "SetKeyCommand", "setTickDrawSpecial", count + tickDrawSpecialCount );

    return status;
}

//*********************************************************
// Name: getKeyLogicalIndex
// Desc: Returns the logical index of the key 
//       at the time when the command was called
//*********************************************************
int SetKeyCommand::getKeyLogicalIndex( MFnAnimCurve &animCurveFn, MStatus *status )
{
    // Returns -1 if there isn't a key set at the playhead time
    int logicalIndex = -1;

    unsigned int closestIndex = animCurveFn.findClosest( originalPlayheadTime, status );
    if( !(*status)) {
        pluginError( "SetKeyCommand", "getKeyLogicalIndex", "Couldn't find closest key" );
    }
    else {
        // if the times match, the indexes are the same
        if( originalPlayheadTime == animCurveFn.time( closestIndex, status ) )
            logicalIndex = (int)closestIndex;           
    }

    return logicalIndex;
}

//*********************************************************
// Name: isBooleanDataType
// Desc: 
//*********************************************************
bool SetKeyCommand::isBooleanDataType( const MPlug &plug )
{
    MStatus status;
    MFnNumericAttribute fnNumAttr;
    MObject attrObj;

    bool isBool = false;

    attrObj = plug.attribute( &status );
    if( attrObj.apiType() == MFn::kNumericAttribute ) {

        status = fnNumAttr.setObject( attrObj );
        if( fnNumAttr.unitType() == MFnNumericData::kBoolean )
            isBool = true;
    }

    return isBool;
}