#include <mutex>
#include "ManualResetEvent.h"

ManualResetEvent::ManualResetEvent( ) :
        _data( new ManualResetEventData( ) )
{
}

ManualResetEvent::~ManualResetEvent( )
{
    delete _data;
}

// Set event to not signalled state
void ManualResetEvent::reset( )
{
    std::unique_lock<std::mutex> lock( _data->_mutex );
    _data->_triggered = false;
}

// Set event to signalled state
void ManualResetEvent::signal( )
{
    std::unique_lock<std::mutex> lock( _data->_mutex );

    _data->_triggered = true;
    _data->_counter++;
    _data->_condVariable.notify_all( );
}

// Wait till the event gets into signalled state
void ManualResetEvent::wait( )
{
    std::unique_lock<std::mutex> lock( _data->_mutex );
    uint32_t           lastCounterValue = _data->_counter;

    while ( ( !_data->_triggered ) && ( _data->_counter == lastCounterValue ) )
    {
        _data->_condVariable.wait( lock );
    }
}

// Wait the specified amount of time (milliseconds) till the event gets signalled
bool ManualResetEvent::wait( uint32_t msec )
{
    std::chrono::steady_clock::time_point waitTill = std::chrono::steady_clock::now( ) + std::chrono::milliseconds( msec );
    std::unique_lock<std::mutex>       lock( _data->_mutex );
    uint32_t                 lastCounterValue = _data->_counter;

    if ( !_data->_triggered )
    {
        _data->_condVariable.wait_until( lock, waitTill );
    }

    return ( ( _data->_triggered ) || ( _data->_counter != lastCounterValue ) );
}

// Check current state of the event
bool ManualResetEvent::isSignaled( )
{
    std::unique_lock<std::mutex> lock( _data->_mutex );
    return _data->_triggered;
}
