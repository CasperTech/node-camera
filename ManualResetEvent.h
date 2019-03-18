#pragma once

#include <stdint.h>
#include "ManualResetEventData.h"

// Manual reset synchronization event
class ManualResetEvent
{
private:
    ManualResetEvent( const ManualResetEvent& );
    ManualResetEvent& operator= ( const ManualResetEvent& );

public:
    ManualResetEvent( );
    ~ManualResetEvent( );

    void reset( );
    void signal( );
    void wait( );
    bool wait( uint32_t msec );
    bool isSignaled( );

private:
    ManualResetEventData* _data;
};