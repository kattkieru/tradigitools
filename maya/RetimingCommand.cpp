//*********************************************************
// RetimingCommand.cpp
//
//
//*********************************************************

//*********************************************************
#include "RetimingCommand.h"
#include "ErrorReporting.h"
//*********************************************************

//*********************************************************
// Constants
//*********************************************************
const char *RetimingCommand::relativeFlag = "-rel";
const char *RetimingCommand::relativeLongFlag = "-relative";
const char *RetimingCommand::deltaFlag = "-d";
const char *RetimingCommand::deltaLongFlag = "-delta";
const char *RetimingCommand::nextKeyOnCompleteFlag = "-nkc";
const char *RetimingCommand::nextKeyOnCompleteLongFlag = "-nextKeyOnComplete";


//*********************************************************
// Name: RetimingCommand
// Desc: Constructor
//*********************************************************
RetimingCommand::RetimingCommand()
{
    pluginTrace( "RetimingCommand", "RetimingCommand", "******* Retiming Command *******" );    

    initialized = false;

    numRetimed = 0;

    origPlayheadTime = MAnimControl::currentTime();
    newPlayheadTime = origPlayheadTime;

    stripString = "No Keys Set";

    // Initialize the command flag defaults
    queryMode = false;
    relativeMode = false;
    timingDelta = 1;
    nextKeyOnComplete = false;

}


//*********************************************************
// Name: ~RetimingCommand
// Desc: Destructor
//*********************************************************
RetimingCommand::~RetimingCommand()
{
    // Cleanup all memory allocated for this command
    if( !animCurveFnList.empty()) {
        animCurveListIter = animCurveFnList.begin();

        while( animCurveListIter != animCurveFnList.end() ) {

            delete (*animCurveListIter).pAnimCurveFn;
            delete (*animCurveListIter).pAnimCache;
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
MStatus RetimingCommand::doIt(const MArgList &args)
{
    MStatus status = MS::kFailure;

    // Set the command flag values appropriately
    if( !parseCommandFlags( args )) {
        pluginError( "RetimingCommand", "doIt", "Failed to parse command flags" );
    }
    // Get a list of the currently selected objects
    else if( !getSelectedObjects() ) {
        // Query mode will still be a success if no objects are selected
        // but remaining processing is skipped
        if( queryMode ) {
            setResult( "No Objects Selected" );
            status = MS::kSuccess;
        }
        else {
            pluginError( "RetimingCommand", "doIt", "Failed to get selected objects" );
        }
    }
    // Get a list of all of the anim curve function sets
    else if( !getAnimCurveFnList() ) {
        // Again, only for query, we will consider this a success 
        // (ie no keys set) so that we can use the result value
        if( queryMode ) {
            setResult( stripString );
            status = MS::kSuccess;
        }
        else {
            pluginError( "RetimingCommand", "doIt", "Failed to create AnimCurveFnList" );
            MGlobal::displayError( "No Keys Set" );
        }  
    }

    // Get the time range to execute retiming over
    else if( !getRange() ) {
        pluginError( "RetimingCommand", "doIt", "Failed to get the time range" );
    }

    else {
        // Execute all operations that change the state of Maya
        if( !(status = redoIt() )) {
            pluginError( "RetimingCommand", "doIt", "Failed to redoIt" );
        }
        else {
            if( queryMode ) {
                setResult( stripString );
            }
            else {
                MString result( "Result: " );
                MGlobal::displayInfo( result + numRetimed );

                // avoid ambiguous error by casting
                setResult( (int)numRetimed );
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
MStatus RetimingCommand::redoIt()
{
    MStatus status = MS::kSuccess;

    if( !initialized ) {
        status = retime();
        initialized = true;
    }
    else {
        // Just use the anim curve cache to redo
        if( !animCurveFnList.empty()) {
            animCurveListIter = animCurveFnList.begin();

            while( animCurveListIter != animCurveFnList.end() ) {
                // Be sure to skip any NULL pointers
                if( (*animCurveListIter).pAnimCache != NULL )
                    ((*animCurveListIter).pAnimCache)->redoIt();

                animCurveListIter++;
            }
        }
    }
    // if there have been no errors and at least one key 
    // has been retimed
    // ** Note: Don't change the current time when querying **
    // It will break middle mouse timeline dragging
    if( status && !animCurveFnList.empty() && !queryMode )
        MAnimControl::setCurrentTime( newPlayheadTime );

    return status;
}


//*********************************************************
// Name: undoIt
// Desc: Contains the code to undo the internal state 
//       changes made by the breakdown command (redoIt).
//       It is called by Maya to undo.
//*********************************************************
MStatus RetimingCommand::undoIt()
{
    MStatus status = MS::kSuccess;

    // Use the anim curve cache to undo
    if( !animCurveFnList.empty()) {
        animCurveListIter = animCurveFnList.begin();

        while( animCurveListIter != animCurveFnList.end() ) {
            // Be sure to skip any NULL pointers
            if( (*animCurveListIter).pAnimCache != NULL )
                    ((*animCurveListIter).pAnimCache)->undoIt();

            animCurveListIter++;
        }
    }

    MAnimControl::setCurrentTime( origPlayheadTime );

    return status;
}


//*********************************************************
// Name: newSyntax
// Desc: Method for registering the command flags
//       with Maya
//*********************************************************
MSyntax RetimingCommand::newSyntax()
{
    // Use MSyntax for a more robust solution to the
    // parsing the command flags
    MSyntax syntax;
    syntax.addFlag( relativeFlag, relativeLongFlag, MSyntax::kBoolean );
    syntax.addFlag( deltaFlag, deltaLongFlag, MSyntax::kLong );
    syntax.addFlag( nextKeyOnCompleteFlag, nextKeyOnCompleteLongFlag, MSyntax::kBoolean );

    syntax.enableQuery();

    return syntax;
}


//*********************************************************
// Name: parseCommandFlags
// Desc: Parse the command flags and stores the values
//       in the appropriate variables
//*********************************************************
MStatus RetimingCommand::parseCommandFlags(const MArgList &args)
{
    MStatus status = MS::kSuccess;

    // Create an object to parse the arguments
    // and then parse them
    MArgDatabase argData( syntax(), args, &status );
    if( !status ) {
        pluginError( "RetimingCommand", "parseCommandFlags",
                     "Failed to create MArgDatabase for the set key command" );
    }
    else {
        queryMode = argData.isQuery();

        if( argData.isFlagSet( relativeFlag ) && !queryMode )
            argData.getFlagArgument( relativeFlag, 0, relativeMode );

        if( argData.isFlagSet( deltaFlag ) && !queryMode )
            argData.getFlagArgument( deltaFlag, 0, timingDelta );

        if( argData.isFlagSet( nextKeyOnCompleteFlag ) && !queryMode )
            argData.getFlagArgument( nextKeyOnCompleteFlag, 0, nextKeyOnComplete );
    }

    // Absolute value retimings cannot be < 1
    // Relative retimings can be positive or negative
    if( (timingDelta < 1) && !relativeMode ) {
        MGlobal::displayError( "Absolute retiming values must be greater than 0" );
        status = MS::kFailure;
    }
    
    return status;
}


//*********************************************************
// Name: getSelectedObjects
// Desc: Generates a list of all the selected
//       objects
//*********************************************************
MStatus RetimingCommand::getSelectedObjects()
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
        pluginError( "RetimingCommand", "getSelectedObjects", "Failed to get active selection list" );
    }
    // At least one object must be selected for this command
    else if( characterSetList.length() == 0 && selectionList.length() == 0 ) {
        if( !queryMode ) {
            pluginError( "RetimingCommand", "getSelectedObjects", "No Objects Selected" );
            MGlobal::displayError( "No Objects Selected" );
        }
    }
    else
        status = MS::kSuccess;

    selectionList.merge( characterSetList );

    return status;
}


//*********************************************************
// Name: getAnimCurveFnList
// Desc: Generates a list of anim curve function sets to
//       be operated on
//*********************************************************
MStatus RetimingCommand::getAnimCurveFnList()
{
    //pluginTrace( "RetimingCommand", "getAnimCurveFnList", "***" );

    MStatus status = MS::kSuccess;

    MObject dependNode;
    MPlugArray plugArray;

    // Create an iterator to traverse the selection list
    MItSelectionList sIter( selectionList, MFn::kInvalid, &status );
    if( !status ) {
        pluginError( "RetimingCommand", "getAnimCurveFnList", "Failed to creation SL iterator" );
    }
    else {

        // Traverse all of the dependency nodes for the selected objects
        for( ; !sIter.isDone(); sIter.next() ) {
            // Get the current dependency node
            if( !sIter.getDependNode( dependNode )) {
                pluginError( "RetimingCommand", "getAnimCurveFnList", "Couldn't get dependency node" );
                status = MS::kFailure;
                break;
            }

            // The call to get connections doesn't clear the array
            // so me must do it manually
            plugArray.clear();

            // Get all of the connections to this dependency node
            MFnDependencyNode dependFn( dependNode );
            if( !dependFn.getConnections( plugArray )) {
                // This object has no animation curves... no keys to worry about
                pluginTrace( "RetimingCommand", "getAnimCurveFnList", "No keys on object" );
                continue;
            }

            // Generate the list from the plugs
            if( !getAnimCurveFnListFromPlugs( plugArray )) {
                pluginError( "RetimingCommand", "getAnimCurveFnList", "Failed to create list from Plugs" );
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
MStatus RetimingCommand::getAnimCurveFnListFromPlugs( MPlugArray plugArray )
{
    //pluginTrace( "RetimingCommand", "getAnimCurveFnListFromPlugs", "***" );
    MStatus status = MS::kSuccess;
    AnimCurveFnACC animCurveFnACC;

    //for( unsigned int i = 0; i < plugArray.length(); i++ )
    //    pluginTrace( "RetimingCommand", "getAnimCurveFnListFromPlugs", MString("Plug Name: ") + plugArray[i].name() );

    // Check each connection for an upstream connections that is a kAnimCurve
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
                pluginError( "RetimingCommand", "getAnimCurveFnListFromPlugs", "DG Iterator error" );
            }
            else {
                // Retrieve the anim curve function sets
                // and store them in the list
                for( ; !dgIter.isDone(); dgIter.next() )
                {
                    MObjectArray nodePath;
                    dgIter.getNodePath( nodePath );
/*                  
                    // *** Original solution that didn't support Blend/Charater nodes ***
                    // Only use the anim curves directly connected to the attributes
                    // >1 on a breadth first search is level 2.  Root will always be
                    // last in path
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
                        MFnAnimCurve *animCurveFn = new MFnAnimCurve( anim, &status );

                        // Avoid adding duplicate anim curves to the list
                        // Important when dealing with blend nodes
                        MString curveName = animCurveFn->name();
                        bool nameExists = false;

                        animCurveListIter = animCurveFnList.begin();
                        while( animCurveListIter != animCurveFnList.end() ) {
                            if( ((AnimCurveFnACC)*animCurveListIter).pAnimCurveFn->name() == curveName ) {
                                nameExists = true;
                                break;
                            }
                            animCurveListIter++;
                        }

                        if( !nameExists ) {
                            //pluginTrace( "RetimingCommand", "getAnimCurveFnListFromPlugs", MString("adding curve:") + animCurveFn->name() );

                            if( !status ) {
                                pluginError( "RetimingCommand", "getAnimCurveFnListFromPlugs", "Can't get AnimCurve function set" );
                                delete animCurveFn;
                            }
                            // If there are no problems, add the function set to the list
                            else {
                                animCurveFnACC.pAnimCurveFn = animCurveFn;
                                animCurveFnACC.pAnimCache = new MAnimCurveChange();
                                animCurveFnList.push_back( animCurveFnACC );
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
// Name: getRange
// Desc: Determines the range (from the time slider)
//       over which retiming will occur
//*********************************************************
MStatus RetimingCommand::getRange()
{
    MStatus status = MS::kSuccess;
    MString rangeStr;

    if( !(status = MGlobal::executeCommand( "timeControl -q -rng $gPlayBackSlider", rangeStr ))) {
        pluginError( "RetimingCommand", "getRange", "Failed to get time range" );
    }
    else if( queryMode )
    {
        // We are only concerned about the current playhead position
        rangeStartTime = origPlayheadTime;
        rangeEndTime = rangeStartTime + 1;
    }
    else {
        // Parse the string for the first and last frame values
        MStringArray unquotedRangeArray, seperatedRangeArray;

        // Remove the quotes from the string
        rangeStr.split( '"', unquotedRangeArray );
        // Seperate the start and end values
        unquotedRangeArray[0].split( ':', seperatedRangeArray );

        // convert the range into floats
        double firstFrame = seperatedRangeArray[0].asFloat();
        double lastFrame = seperatedRangeArray[1].asFloat();

        //MString output( "First Frame: " );
        //pluginTrace( "RetimingCommand", "getRange", output + firstFrame + " Last Frame: " + lastFrame );

        // convert first/last frames into MTime values
        rangeStartTime.setValue( firstFrame );
        rangeEndTime.setValue( lastFrame );
    }

    return status;
}


//*********************************************************
// Name: retime
// Desc: Traverses all keyframes in the given range for
//       all curves and retimes them to the absolute
//       delta value 
//*********************************************************
MStatus RetimingCommand::retime()
{
    //pluginTrace( "RetimingCommand", "retimeAbsolute", "***" );

    MStatus status = MS::kSuccess;

    // Retime each individual anim curve in the list
    animCurveListIter = animCurveFnList.begin();
    while( (animCurveListIter != animCurveFnList.end()) && (status == MS::kSuccess) )
    {
        status = retimeAnimCurve( *animCurveListIter );

        animCurveListIter++;
    }

    //pluginTrace( "RetimingCommand", "retimeAbsolute", "completed" );
    return status;
}


//*********************************************************
// Name: retimeAnimCurve
// Desc: Traverses all keyframes in the given range and
//       retimes them to the absolute delta value 
//*********************************************************
MStatus RetimingCommand::retimeAnimCurve( AnimCurveFnACC &animCurveACC )
{
    //pluginTrace( "RetimingCommand", "retimeAnimCurve", "***" );
    
    MStatus status = MS::kSuccess;
    MFnAnimCurve *animCurve = animCurveACC.pAnimCurveFn;
    
    unsigned int numKeys = animCurve->numKeys( &status );
    unsigned int firstRetimingIndex = 0;
    unsigned int lastRetimingIndex = 0;

    // Find the index of the last key NOT to be moved -- the anchor
    unsigned int closestIndex = animCurve->findClosest( rangeStartTime, &status );
    if( !status ) {
        pluginError( "RetimingCommand", "retimeAnimCurve", "Couldn't find closest key" );
    }
    MTime closestTime = animCurve->time( closestIndex, &status );
    if( !status ) {
        pluginError( "RetimingCommand", "retimeAnimCurve", "Couldn't find closest time" );
    }

    firstRetimingIndex = closestIndex;

    if( (closestTime > rangeStartTime) && (closestIndex > 0) )
        firstRetimingIndex = closestIndex - 1;

    // Find the index of the last key TO BE RETIMED during this operation
    // All keys after this one will be shifted only
    closestIndex = animCurve->findClosest( rangeEndTime, &status );
    if( !status ) {
        pluginError( "RetimingCommand", "retimeAnimCurve", "Couldn't find closest key" );
    }
    closestTime = animCurve->time( closestIndex, &status );
    if( !status ) {
        pluginError( "RetimingCommand", "retimeAnimCurve", "Couldn't find closest time" );
    }

    lastRetimingIndex = closestIndex;
    
    // When the closest frame is inside the time range, we need to move
    // to the next frame (after the end of the time range) if possible
    if( (closestTime < rangeEndTime) && (closestIndex < (numKeys - 1)))
        lastRetimingIndex = closestIndex + 1;

    //MString output( "startingIndex : lastIndex ---> " );
    //pluginTrace( "RetimingCommand", "retimeAnimCurve",
    //             output + firstRetimingIndex + " : " + lastRetimingIndex );

    // The last key should never be less than the first key
    if( firstRetimingIndex > lastRetimingIndex ) {
        pluginError( "RetimingCommand", "retimeAnimCurve", "RetimingStartIndex should always be less" );
        status = MS::kFailure;
    }

    // We have all the required info to generate the query string
    else if( queryMode ) {
        generateStripString( *(animCurveACC.pAnimCurveFn), firstRetimingIndex, lastRetimingIndex );
    }
    else { 
        // If the start and last indexes are the same, then there
        // are no keys during/after the range.  Additional processing
        // can be skipped otherwise...
        if( firstRetimingIndex < lastRetimingIndex ) {
            MTime prevIndexMTime = animCurve->time( firstRetimingIndex );
            double prevIndexTime = prevIndexMTime.value();

            // Move the play head to the first key in the retiming
            // range when this flag is not set
            if( !nextKeyOnComplete )
                newPlayheadTime = prevIndexMTime;

            retimeAnimCurve( animCurveACC, firstRetimingIndex + 1, 
                             lastRetimingIndex, prevIndexTime, prevIndexTime );
        }
        else
            // When skipping, don't move the playhead
            newPlayheadTime = origPlayheadTime;
    }

    return status;
}

//*********************************************************
// Name: retimeAnimCurve
// Desc: Traverses all keyframes in the given range and
//       retimes them to the delta value 
//*********************************************************
MStatus RetimingCommand::retimeAnimCurve( AnimCurveFnACC &animCurveACC,
                                          unsigned int currentIndex,
                                          unsigned int lastRetimingIndex,
                                          double prevIndexOrigTime,
                                          double prevIndexNewTime )
{
    MStatus status = MS::kSuccess;
    MString outStr( "" );

    MFnAnimCurve *animCurve = animCurveACC.pAnimCurveFn;

    // Store the time in frames
    double currIndexOrigTime = 0.0;
    double currIndexNewTime = 0.0;
    double nextIndexOrigTime = 0.0;

    MTime currIndexMTime;
    MTime nextIndexMTime;

    currIndexMTime = animCurve->time( currentIndex );
    currIndexOrigTime = currIndexMTime.value();

    // Caculate the new time for the current key differently based
    // on absolute or relative values
    if( relativeMode ) {
        // Relative - shift the current key's time by the input delta *plus*
        //            the time shift of the previous key
        currIndexNewTime = currIndexOrigTime + (double)timingDelta +
                                (prevIndexNewTime - prevIndexOrigTime);

        // Make sure that when a relative retiming is negative,
        // the retiming will result in no less than one frame between
        // the current frame being shifted and its previous frame
        if( (currIndexNewTime - prevIndexNewTime) < 1 ) {
            currIndexNewTime = prevIndexNewTime + 1;
        }
    }
    else {
        // Absolute - The previous time plus the expected time between keys
        currIndexNewTime = prevIndexNewTime + (double)timingDelta;
    }

    MTime currIndexNewMTime( currIndexNewTime, MTime::uiUnit() );

    // All retiming keys, except for the last one are handled the
    // same.
    if( currentIndex != lastRetimingIndex ) {
        nextIndexMTime = animCurve->time( currentIndex + 1 );
        nextIndexOrigTime = nextIndexMTime.value();

        //pluginTrace( "RetimingCommand", "retimeAnimCurve",
        //             outStr + "[" + currentIndex + "]" + " Original Time: " + currIndexOrigTime + " New Time: " + currIndexNewTime );

        // If the new time for the current key is going to be
        // larger than the original time for the next frame,
        // we must move the next frame first to prevent problems
        if( currIndexNewTime >= nextIndexOrigTime ) {
            retimeAnimCurve( animCurveACC, currentIndex + 1, 
                             lastRetimingIndex, currIndexOrigTime, currIndexNewTime );
            animCurve->setTime( currentIndex, currIndexNewMTime, animCurveACC.pAnimCache );
        }
        // If the new time for the current key is smaller,
        // we will move it first to avoid problems with
        // the next key possibly having a smaller value once
        // it is retimed (if it were retimed first
        else {
            animCurve->setTime( currentIndex, currIndexNewMTime, animCurveACC.pAnimCache );
            retimeAnimCurve( animCurveACC, currentIndex + 1, 
                             lastRetimingIndex, currIndexOrigTime, currIndexNewTime );
        }

        // Keep track of the number of keys retimed
        numRetimed++;
    }

    // Once the last retiming key is reached, all keys after it
    // will need to be shifted as well
    else {
        // If there are no keys after the last key to be retimed,
        // adjust its time and we're done
        if( lastRetimingIndex == (animCurve->numKeys() - 1) ) {
            animCurve->setTime( currentIndex, currIndexNewMTime, animCurveACC.pAnimCache );
        }
        // Otherwise we need to shift all keys after the last retiming keys
        else {

            double lastRetimingKeyDelta = currIndexNewTime - currIndexOrigTime;

            //pluginTrace( "RetimingCommand", "retimeAnimCurveAbs",
            //             outStr + "Final Retiming Key Delta: " + lastRetimingKeyDelta );

            if( lastRetimingKeyDelta > 0 ) {
                // shift before updating the last retiming key
                shiftRemainingKeys( animCurveACC, currentIndex + 1, lastRetimingKeyDelta );
                animCurve->setTime( currentIndex, currIndexNewMTime, animCurveACC.pAnimCache );
            }
            else if( lastRetimingKeyDelta < 0 ) {
                // update the last retiming key, then shift
                animCurve->setTime( currentIndex, currIndexNewMTime, animCurveACC.pAnimCache );
                shiftRemainingKeys( animCurveACC, currentIndex + 1, lastRetimingKeyDelta );
            }
            // if the lastRetimingDelta == 0, there is no need to shift the remaining keys   
        }

        // Move the play head to the last key in the retiming
        // range when this flag is set
        if( nextKeyOnComplete )
            newPlayheadTime = currIndexNewMTime;

        // Keep track of the number of keys retimed (including the last one)
        numRetimed++;
    }

    return status;
}

//*********************************************************
// Name: shiftRemainingKeys
// Desc: Shift the keys that fall after the retiming range by
//       the specified number of frames
//*********************************************************
MStatus RetimingCommand::shiftRemainingKeys( AnimCurveFnACC &animCurveACC,
                                             unsigned int firstKeyIndex,
                                             double numFrames)
{
    MStatus status = MS::kSuccess;

    MFnAnimCurve *animCurve = animCurveACC.pAnimCurveFn;
 
    MTime currentKeyTime;

    // Shift the lowest index (smallest time value) first when shifting keys
    // to the left (smaller time values)
    if( numFrames < 0 ) {
        for( unsigned int index = firstKeyIndex; index < animCurve->numKeys(); index++ )
        {   
            currentKeyTime = animCurve->time( index, &status );
            currentKeyTime += numFrames;
            animCurve->setTime( index, currentKeyTime, animCurveACC.pAnimCache );
        }
    }
    // Shift the highest index (largest time value) first when shifting keys
    // to the right (larger time values)
    else if( numFrames > 0 ) {
        for( unsigned int index = (animCurve->numKeys() - 1); index >= firstKeyIndex; index-- )
        {   
            currentKeyTime = animCurve->time( index, &status );
            currentKeyTime += numFrames;
            animCurve->setTime( index, currentKeyTime, animCurveACC.pAnimCache );
        }
    }
    return status;
}

//*********************************************************
// Name: generateStripString
// Desc: Creates the strip string. The info related to the
//       current timing at the current playhead position
//*********************************************************
MStatus RetimingCommand::generateStripString( MFnAnimCurve &animCurve,
                                              unsigned int firstRetimingIndex,
                                              unsigned int lastRetimingIndex)
{
    MStatus status = MS::kSuccess;

    MTime time = animCurve.time( firstRetimingIndex );
    double firstFrame = time.value();

    // Create the first half of the string
    if( (firstRetimingIndex == lastRetimingIndex) && (origPlayheadTime < time))
        stripString = "None";
    else
        stripString = firstFrame;

    time = animCurve.time( lastRetimingIndex );
    double lastFrame = time.value();

    if( (firstRetimingIndex == lastRetimingIndex) && (origPlayheadTime >= time)) {
        // We are on or after the last key
        stripString += " on End";
    }
    else if( firstRetimingIndex == lastRetimingIndex ) {
        // We are before the first key (None on first frame in the timeline)
        // Determine what the animation start time is
        time = MAnimControl::animationStartTime();
        double animStartFrame = time.value();

        stripString += " on ";
        stripString += (firstFrame - animStartFrame);
    }
    else {
        double delta = lastFrame - firstFrame;
        stripString += " on ";
        stripString += delta;
    }

    return status;
}