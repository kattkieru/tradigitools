//*********************************************************
// BreakdownCommand.cpp
//
// Copyright (C) 2007-2021 Skeletal Studios
// All rights reserved.
//
//*********************************************************

//*********************************************************
#include "BreakdownCommand.h"
#include "ErrorReporting.h"
//*********************************************************

//*********************************************************
// Constants
//*********************************************************
const char *BreakdownCommand::weightFlag = "-w";
const char *BreakdownCommand::weightLongFlag = "-weight";
const char *BreakdownCommand::selectedAttrFlag = "-sa";
const char *BreakdownCommand::selectedAttrLongFlag = "-selectedAttr";
const char *BreakdownCommand::modeFlag = "-m";
const char *BreakdownCommand::modeLongFlag = "-mode";
const char *BreakdownCommand::invalidAttrOpFlag = "-iao";
const char *BreakdownCommand::invalidAttrOpLongFlag = "-invalidAttrOp";
const char *BreakdownCommand::ignoreRippleCheckFlag = "-irc";
const char *BreakdownCommand::ignoreRippleCheckLongFlag = "-ignoreRippleCheck";
const char *BreakdownCommand::tickDrawSpecialFlag = "-tds";
const char *BreakdownCommand::tickDrawSpecialLongFlag = "-tickDrawSpecial";


//*********************************************************
// Name: BreakdownCommand
// Desc: Constructor
//*********************************************************
BreakdownCommand::BreakdownCommand()
{
    pluginTrace( "BreakdownCommand", "BreakdownCommand", "******* Insert Breakdown Command *******" );

    // Get the current time/frame 
    currentAnimationFrame = MAnimControl::currentTime();

    // Initialize the command flag defaults
    breakdownWeight = 0.5;
    breakdownMode = Breakdown::kOverwrite;
    selectedAttrOnly = false;
    invalidAttrOp = kSkipAll;
    ignoreRippleCheck = false;
    tickDrawSpecial = false;

    attributesSkipped = false;
    objectsSkipped = false;
}


//*********************************************************
// Name: ~BreakdownCommand
// Desc: Destructor
//*********************************************************
BreakdownCommand::~BreakdownCommand()
{
    // Delete all breakdowns from the list
    breakdownList.deleteAndClear();
}


//*********************************************************
// Name: doIt
// Desc: All of the one-time setup and initialization
//       code for the breakdown command.  doIt is called
//       by Maya when any command is executed in MEL.
//       Any code that changes the state of Maya is
//       handled by the redoIt method.
//*********************************************************
MStatus BreakdownCommand::doIt( const MArgList &args )
{
    parseCommandFlags( args );

	getSelectedObjects();

    if( selectionList.length() == 0 ) {
		MGlobal::displayError( "No Objects Selected" );
        status = MS::kFailure;
    }
    else if( selectedAttrOnly && (populateSelectedAttributeList() == 0) ) {
        MGlobal::displayError( "No Attributes Selected" );
        status = MS::kFailure;
    }
	else
    {
		MStringArray results;
		selectionList.getSelectionStrings( results );

        if( !createBreakdownList()) {
            // Create breakdown will display its own error
        }

        // When in ripple mode, the default behaviour is to verify that
        // all attributes have a key set at the current time or all keys
        // have no keys set at the current time.  The ripple breakdown
        // will fail if this is not the case unless the check is disabled.
        else if( breakdownMode == Breakdown::kRipple && 
                 !ignoreRippleCheck &&
                 !(status = breakdownList.areOriginalKeysUniform()) )
        {
            MGlobal::displayError( "Breakdown Failed. (Ripple Mode)All attributes must have a key set or no keys set at the current time." );
        }
        else {
            if( !redoIt() ) {
                pluginError( "BreakdownCommand", "doIt", "Failed to redoIt" );
            }
            else {
                MString output( "Result: " );

                output += breakdownList.size();

                if( attributesSkipped )
                    output += "   (See Script Editor for skipped attributes)";
                else if( objectsSkipped )
                    output += "   (See Script Editor for skipped objects)";
                MGlobal::displayInfo( output );
            }
        }
	}

	return status;
}

//*********************************************************
// Name: redoIt
// Desc: Contains the code that changes the internal state
//       of Maya.  It is called by Maya to redo.
//*********************************************************
MStatus BreakdownCommand::redoIt()
{
    // Traverse the breakdown list and call redo on each
    // breakdown object
    if( !breakdownList.empty() ) {
        breakdownList.iterBegin();
        
        for( Breakdown* breakdown = breakdownList.getCurrent();
             breakdown != NULL;
             breakdown = breakdownList.getNext() )
        {
            status = breakdown->redoIt();
            if( !status )
                pluginError( "BreakdownCommand", "redoIt", "Failed to redoIt" );
        }
    }

    return status;
}


//*********************************************************
// Name: undoIt
// Desc: Contains the code to undo the internal state 
//       changes made by the breakdown command (redoIt).
//       It is called by Maya to undo.
//*********************************************************
MStatus BreakdownCommand::undoIt()
{
    // Traverse the breakdown list and call undo on each
    // breakdown object
    if( !breakdownList.empty() ) {
        breakdownList.iterBegin();
        
        for( Breakdown* breakdown = breakdownList.getCurrent();
             breakdown != NULL;
             breakdown = breakdownList.getNext() )
        {
            status = breakdown->undoIt();
            if( !status )
                pluginError( "BreakdownCommand", "undoIt", "Failed to undoIt" );
        }
    }

    return status;
}


//*********************************************************
// Name: newSyntax
// Desc: Method for registering the command flags
//       with Maya
//*********************************************************
MSyntax BreakdownCommand::newSyntax()
{
    // Use MSyntax for a more robust solution to the
    // parsing the command flags
    MSyntax syntax;
    syntax.addFlag( weightFlag, weightLongFlag, MSyntax::kDouble );
    syntax.addFlag( selectedAttrFlag, selectedAttrLongFlag, MSyntax::kBoolean );
    syntax.addFlag( modeFlag, modeLongFlag, MSyntax::kString );
    syntax.addFlag( invalidAttrOpFlag, invalidAttrOpLongFlag, MSyntax::kString );
    syntax.addFlag( ignoreRippleCheckFlag, ignoreRippleCheckLongFlag, MSyntax::kBoolean );
    syntax.addFlag( tickDrawSpecialFlag, tickDrawSpecialLongFlag, MSyntax::kBoolean );

    return syntax;
}


//*********************************************************
// Name: parseCommandFlags
// Desc: Parsed the command flags and stores the values
//       in the appropriate variables
//*********************************************************
void BreakdownCommand::parseCommandFlags( const MArgList &args )
{
    MArgDatabase argData( syntax(), args, &status );
    if( !status ) {
        pluginError( "BreakdownCommand", "parseCommandFlags",
                     "Failed to create MArgDatabase for the breakdown command" );
    }
    else {
        if( argData.isFlagSet( weightFlag ))
            argData.getFlagArgument( weightFlag, 0, breakdownWeight );
        if( argData.isFlagSet( selectedAttrFlag ))
            argData.getFlagArgument( selectedAttrFlag, 0, selectedAttrOnly );
        if( argData.isFlagSet( ignoreRippleCheckFlag ))
            argData.getFlagArgument( ignoreRippleCheckFlag, 0, ignoreRippleCheck );
        if( argData.isFlagSet( tickDrawSpecialFlag ))
            argData.getFlagArgument( tickDrawSpecialFlag, 0, tickDrawSpecial );

        if( argData.isFlagSet( invalidAttrOpFlag )) {
            MString strAttrOp;
            argData.getFlagArgument( invalidAttrOpFlag, 0, strAttrOp );

            if( strAttrOp == "skipAll" )
                invalidAttrOp = kSkipAll;
            else if( strAttrOp == "skipObject" )
                invalidAttrOp = kSkipObject;
            else if( strAttrOp == "skipAttr" )
                invalidAttrOp = kSkipAttr;
            else
                MGlobal::displayWarning( "Invalid arguement for -invalidAttrOp.  Using default value." );
        }

        if( argData.isFlagSet( modeFlag )) {
            MString strMode;
            argData.getFlagArgument( modeFlag, 0, strMode );

            if( strMode == "overwrite" )
                breakdownMode = Breakdown::kOverwrite;
            else if( strMode == "ripple" )
                breakdownMode = Breakdown::kRipple;
        }

        // Ripple mode ignores the selectedAttrOnly flag.
        // All attributes will be affected.
        if( breakdownMode == Breakdown::kRipple ) {
            if( selectedAttrOnly == true ) {
                selectedAttrOnly = false;
                MGlobal::displayWarning( "Key Selected flag is ignored in Ripple Mode" );
            }
        }
    }
}

//*********************************************************
// Name: getSelectedObjects
// Desc: 
//*********************************************************
void BreakdownCommand::getSelectedObjects()
{
    status = MS::kFailure;
    MSelectionList characterSetList;

    selectionList.clear();

    // Selected Objects Include:
    //    1) The Active Character Set (if in use) and its subset
    //    2) Character Sets selected by the user (and their subsets)
    //    3) Objects selected by the user

    // Get the character set if active (in Maya) and the associated character set subsets
    MStringArray characterSets;
    MGlobal::executeCommand( MString("cie_atbGetActiveCharacterSets"), characterSets, false, false );

    // Create a selection list with all the character set/subsets
    for( unsigned int i = 0; i < characterSets.length(); i++ ) {
        MGlobal::getSelectionListByName( characterSets[i], characterSetList );
    }

    // Get the character sets selected (by user) and the associated character set subsets 
    MGlobal::executeCommand( MString("cie_atbGetSelectedCharacterSets"), characterSets, false, false);

    // Append to the selection list with all the selected character set/subsets
    for( unsigned int i = 0; i < characterSets.length(); i++ ) {
        MGlobal::getSelectionListByName( characterSets[i], characterSetList );
    }

    // Retrive all of the currently selected objects
    if( !MGlobal::getActiveSelectionList( selectionList )) {
        pluginError( "BreakdownCommand", "getSelectedObjects", "Failed to get active selection list" );
    }
    /*
    // At least one object must be selected for this command
    else if( characterSetList.length() == 0 && selectionList.length() == 0 ) {
            pluginError( "BreakdownCommand", "getSelectedObjects", "No Objects Selected" );
            MGlobal::displayError( "No Objects Selected" );
        }
    }
    */
    else
        status = MS::kSuccess;

    selectionList.merge( characterSetList );
}


//*********************************************************
// Name: createBreakdownList
// Desc: 
//*********************************************************
MStatus BreakdownCommand::createBreakdownList()
{
    MObject dependNode;
    unsigned int objID = 0;

    MItSelectionList sIter( selectionList, MFn::kInvalid, &status );
	for( ; !sIter.isDone(); sIter.next() ) {
		
		sIter.getDependNode( dependNode );
		MFnDependencyNode dependFn( dependNode );
		
		MPlugArray connections;
		dependFn.getConnections( connections );

        if( !processConnections( connections, objID, dependFn.name()) ) {
            pluginWarning( "BreakdownCommand", "createBreakdownList", "processConnections Error if *not* Skipping All Objects" );
            break;
        }

        objID++;
    }

    
    if( breakdownList.size() == 0 && status ) {
        pluginTrace( "BreakdownCommand", "createBreakdownList", "There are no breakdowns on the list" );
        MGlobal::displayError( "No attributes were found to set breakdowns on. (See Script Editor)" );
        status = MS::kFailure;
    }
        

    return status;
}

//*********************************************************
// Name: processConnections
// Desc: 
//*********************************************************
MStatus BreakdownCommand::processConnections( MPlugArray &connections, unsigned int objID, MString objName )
{
    status = MS::kSuccess;
    bool skipProcessing = false;

    // Under certain conditions, the specified weight must be modified
    double actualBreakdownWeight = breakdownWeight;
    bool isBooleanValue = false;

	for( unsigned int j = 0; j < connections.length(); j++ ) {

        // If the attribute is a boolean or enum, keep its
        // breakdown value the same as its previous key value.
        // Weirdness can occur in things like visibility
        if( isBooleanDataType(connections[j]) || isEnumDataType(connections[j]) )
        {
            // favour the previous key completely
            actualBreakdownWeight = 0.0;
            isBooleanValue = true;
        }
        else {
            actualBreakdownWeight = breakdownWeight;
            isBooleanValue = false;
        }
        
        // When the selectedAttrOnly flag is set, only process
        // attributes that have been selected in the channel box
        if( !selectedAttrOnly || 
            isStrOnSelectedAttrList( connections[j].partialName() ))
        {
		    if( connections[j].isKeyable() && !connections[j].isLocked() ) {

                MPlug currentPlug = connections[j];
			    MItDependencyGraph dgIter( currentPlug,
									       MFn::kAnimCurve,
                                           MItDependencyGraph::kUpstream,
                                           MItDependencyGraph::kBreadthFirst,
                                           MItDependencyGraph::kNodeLevel,
                                           &status );

                for( ; !dgIter.isDone(); dgIter.next() )
                {
                    MObjectArray nodePath;
                    dgIter.getNodePath( nodePath );
 
/*
                    // *** Original solution that didn't support Blend/Charater nodes ***
                    // Only use the anim curves directly connected to the attributes
                    // >1 on a breath first search is level 2.  Root will always be
                    // first in the path
                    if( nodePath.length() > 2 )
                        break;
*/
                    
                    // At a depth of 1 in the DAG, the animation nodes are directly 
                    // connected to animated object.  However, if the depth is greater
                    // than one then we must accomodate both PairBlend nodes and 
                    // Character Set nodes (which sit between the transform node and
                    // the anim nodes.
                    int nodeParentIndex = 1;
                    if( nodePath.length() <= 2 || 
                        (nodePath.length() == 3 && 
                            (nodePath[nodeParentIndex].apiType() == MFn::kPairBlend ||
                            nodePath[nodeParentIndex].apiType() == MFn::kCharacter ))
                        )
                    {

                        MObject anim = dgIter.thisNode( &status );
                        MFnAnimCurve animCurve( anim, &status );

                        // Avoid adding duplicate anim curves to the list
                        // Important when dealing with blend nodes
                        MString curveName = animCurve.name();
                        bool nameExists = false;
                       
                        breakdownList.iterBegin();
                        Breakdown* bkdn = breakdownList.getCurrent();
                        while( bkdn != NULL )
                        {
                            if( bkdn->getAnimCurveFn().name() == curveName ) {
                                nameExists = true;
                                break;
                            }
                            bkdn = breakdownList.getNext();
                        }


                        if( !nameExists )
                        {
                            // Create a breakdown and add it to the list
                            Breakdown* newBreakdown = new Breakdown( animCurve,
                                                                     breakdownWeight,
                                                                     breakdownMode,
                                                                     tickDrawSpecial,
                                                                     currentAnimationFrame,
                                                                     isBooleanValue,
                                                                     objID,
                                                                     &status );
                            // On success, add new breakdown to the list
                            if( status == MS::kSuccess ) {
                                // pluginTrace( "BreakdownCommand", "processConnections", "***Adding Breakdown****" );
                                breakdownList.add( newBreakdown );
                            }

                            // If the breakdown failed, determine how to proceed
                            // from the invalidAttrOp flag
                            else {
                                status = MS::kSuccess;

                                // The command fails if there is an invalid
                                // attribute.  No breakdowns are set.
                                if( invalidAttrOp == kSkipAll ) {
                                    pluginTrace( "BreakdownCommand", "processConnections", "Skipping all objects" );
                                    MGlobal::displayInfo( connections[j].partialName(true) + " --> " + newBreakdown->getErrorMsg());
                                    MGlobal::displayError( "Skipping All Objects (See Script Editor for Invalid Attribute)" );

                                    skipProcessing = true;
                                    status = MS::kFailure;

                                    break;
                                }

                                // The object is skipped.  All breakdowns already
                                // added for attributes on this object will need
                                // to be removed from the list
                                else if( invalidAttrOp == kSkipObject ) {
                                    pluginTrace( "BreakdownCommand", "processConnections", "Skipping object: " + objName );
                                    MGlobal::displayInfo( "Skipping Object: " + objName );
                                
                                    breakdownList.deleteBreakdowns( objID );
                                    skipProcessing = true;
                                    objectsSkipped = true;

                                    break;
                                }

                                // If only invalid attributes are to be skipped
                                // just delete the breakdown and carry on
                                else if( invalidAttrOp == kSkipAttr ) {
                                    pluginTrace( "BreakdownCommand", "processConnections", "Skipping attribute: " + connections[j].partialName( true ));
                                    MGlobal::displayInfo( "Skipping Attribute: " +
                                                              connections[j].partialName( true ) +
                                                              " (" + newBreakdown->getErrorMsg() + ")" );
                                    attributesSkipped = true;
                                }

                                // Clean up memory for discarded breakdowns
                                delete newBreakdown;
                            }
                        }
                    }
                }
            }
        }
        // Don't keep processing connections if the object
        // should be skipped (invalid attribute flag)
        if( skipProcessing )
            break;

	}

    return status;
}

//*********************************************************
// Name: populateSelectedAttributeList
// Desc: 
//*********************************************************
unsigned int BreakdownCommand::populateSelectedAttributeList()
{
    status = MGlobal::executeCommand( "channelBox -q -sma mainChannelBox", selectedAttributeList );

    return selectedAttributeList.length();
}

//*********************************************************
// Name: isStrOnSelectedAttrList
// Desc: 
//*********************************************************
bool BreakdownCommand::isStrOnSelectedAttrList( const MString &str )
{
    bool result = false;

    for( unsigned int i = 0; i < selectedAttributeList.length(); i++ ) {
        if( selectedAttributeList[i] == str ) {
            result = true;
            break;
        }
    }

    return result;
}

//*********************************************************
// Name: isBooleanDataType
// Desc: 
//*********************************************************
bool BreakdownCommand::isBooleanDataType( const MPlug &connection )
{
    MFnNumericAttribute fnNumAttr;
    MObject attrObj;

    bool isBool = false;

    attrObj = connection.attribute( &status );
    if( attrObj.apiType() == MFn::kNumericAttribute ) {

        status = fnNumAttr.setObject( attrObj );
        if( fnNumAttr.unitType() == MFnNumericData::kBoolean )
            isBool = true;
    }

    return isBool;
}

//*********************************************************
// Name: isEnumDataType
// Desc: 
//*********************************************************
bool BreakdownCommand::isEnumDataType(const MPlug &connection)
{
    MObject attrObj;

    bool isEnum = false;

    attrObj = connection.attribute( &status );
    if( attrObj.apiType() == MFn::kEnumAttribute ) {
        isEnum = true;
    }

    return isEnum;
}