//*********************************************************
// BreakdownList.cpp
//
//
//*********************************************************

//*********************************************************
#include "BreakdownList.h"
#include "ErrorReporting.h"
//*********************************************************

//*********************************************************
// Name: BreakdownList
// Desc: Constructor
//
//*********************************************************
BreakdownList::BreakdownList()
{
    
}

//*********************************************************
// Name: ~BreakdownList
// Desc: Destructor - All breakdowns in the list will
//                    be deleted when called
//*********************************************************
BreakdownList::~BreakdownList()
{
    deleteAndClear();
}

//*********************************************************
// Name: getCurrent
// Desc: Returns the breakdown currently pointed to by
//       the iterator.  NULL is returned if the iterator
//       is at the end of the list.
//*********************************************************
Breakdown* BreakdownList::getCurrent()
{
    Breakdown* current;
    
    // Return NULL if at the end of the list
    // or else return the breakdown
    if( iter == breakdownList.end() )
        current = NULL;
    else
        current = *iter;

    return current;
}

//*********************************************************
// Name: getNext
// Desc: Moves the iterator to the next breakdown in
//       the list and returns that breakdown.  NULL is 
//       returned if the iterator is at the end of 
//       the list.
//*********************************************************
Breakdown* BreakdownList::getNext()
{
    Breakdown* next;

    // Move to the next breakdown in the list and
    // return it.  NULL is returned if at the end
    // of the list
    if( iter == breakdownList.end() || ++iter == breakdownList.end())
        next = NULL;
    else
        next = *iter;

    return next;
}

//*********************************************************
// Name: getPrevious
// Desc: Moves the iterator to the previous breakdown in
//       the list and returns that breakdown.  NULL is 
//       returned if the iterator is already pointing
//       to the first element.
//*********************************************************
Breakdown* BreakdownList::getPrevious()
{
    Breakdown* previous;

    // Move to the previous breakdown in the list and
    // return it.  NULL is returned if at the start
    // of the list
    if( iter == breakdownList.begin() )
        previous = NULL;
    else {
        --iter;
        previous = *iter;
    }

    return previous;
}

//*********************************************************
// Name: deleteAndClear
// Desc: All breakdown on the list are deleted and then
//       removed from the list
//*********************************************************
void BreakdownList::deleteAndClear()
{
    // Goto the start of the list and delete
    // each breakdown
    if( !empty() ) {
        iterBegin();
        
        while( iter != breakdownList.end()) {
            delete *iter;
            *iter = NULL;
            iter++;
        }
    }
    
    // Clear the list of all NULL pointers
    breakdownList.clear();
}

//*********************************************************
// Name: deleteBreakdowns
// Desc: Breakdowns with an id that matches the given
//       id are deleted and removed from the list
//*********************************************************
void BreakdownList::deleteBreakdowns( unsigned int id )
{
    if( !empty() ) {
        iterBegin();
    
        // delete all breakdowns with the specified id
        while( iter != breakdownList.end()) {
            if( ((Breakdown*)*iter)->getObjId() == id ) {
                delete *iter;
                iter = breakdownList.erase( iter );
            }
            else
                iter++;
        }
    }
}

//*********************************************************
// Name: areOriginalKeysUniform
// Desc: Tests to see if all of the attributes have
//       either a key set or no key set at the breakdown
//       time.  Stops partial ripples when in ripple mode.
//*********************************************************
MStatus BreakdownList::areOriginalKeysUniform()
{
    bool isFirstOriginal;
    bool isValid = true;

    MStatus status = MS::kSuccess;

    Breakdown* breakdown;

    // Traverse list to determine if all attributes have
    // a key set or all attributes do not have a key set
    if( !empty() ) {
        iterBegin();

        // All subsequent breakdowns will have to match
        // the original as to whether or not there is a
        // key set at the current time
        isFirstOriginal = ((Breakdown*)*iter)->hasOriginalKey();
        iter++;

        while( (breakdown = getNext()) != NULL ) {
            if( breakdown->hasOriginalKey() != isFirstOriginal ) {
                isValid = false;
                status = MS::kFailure;
                break;
            }
        }
    }

    return status;
}