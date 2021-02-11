//*********************************************************
// BreakdownList.h
//
// Copyright (C) 2007-2021 Skeletal Studios
// All rights reserved.
//
//*********************************************************

#ifndef __BREAKDOWN_LIST_H_
#define __BREAKDOWN_LIST_H_

//*********************************************************
#include <list>
#include "Breakdown.h"
//*********************************************************

//*********************************************************
// Class: BreakdownList
//
// Desc:  A container for Breakdown objects. A number
//        of helper methods are available to traverse
//        and manage those objects.
//*********************************************************
class BreakdownList
{
private:
    // List used to hold the breakdowns
    std::list<Breakdown*> breakdownList;

    // Iterator to traverse the list
    std::list<Breakdown*>::iterator iter;

public:
    // Constructor/Destructor
    BreakdownList();
    ~BreakdownList();

    // Returns the number of breakdowns in the list
    unsigned int size()  { return (unsigned int)breakdownList.size(); }

    // Returns true if the list is empty
    bool empty()         { return breakdownList.empty(); }

    // Deletes all of the breakdowns from the list and clears the list
    // All breakdowns are expected to be created on the heap and exclusively
    // managed by the list after being added.
    void deleteAndClear();

    // Deletes and removes all of the breakdowns from the list with 
    // the matching id
    void deleteBreakdowns( unsigned int id );

    // Adds a new breakdown to the list
    void add( Breakdown* breakdown ) { breakdownList.push_back(breakdown); }

    // Moves the iterator to the first item in the list
    void iterBegin() { iter = breakdownList.begin(); }

    // Moves the iterator to AFTER the last item in the list
    void iterEnd()   { iter = breakdownList.end(); }

    // Gets the current breakdown pointed to by the iterator.
    // NULL is returned if at the end of the list.
    Breakdown* getCurrent();
    
    // Moves the iterator to the next breakdown and returns 
    // that breakdown.  NULL is returned if at the end of the list
    Breakdown* getNext();

    // Moves the iterator to the previous breakdown and returns 
    // that breakdown.  NULL is returned if at the start of the list
    Breakdown* getPrevious();

    // Tests to see if all of the attributes have either
    // a key set or no key set at the breakdown time.
    // Stops partial ripples when using ripple mode.
    MStatus areOriginalKeysUniform();
};


#endif