/*
 * err.h
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#ifndef ERR_H_
#define ERR_H_

#include <exception>
#include <stdexcept>

#define THROW( ... ) \
{                                           \
    char buffer[ 256 ];                     \
    snprintf( buffer, 256, __VA_ARGS__ );   \
    throw std::runtime_error( buffer );     \
}

#define ASSERT( cond, ... ) if ( !(cond) ) THROW( __VA_ARGS__ );

void ShowWindowsError( const char* msg, unsigned long err, const char* header = "SystemError!" );

void ShowError( const char* msg, const char* header = "Error!" );

const char* glErrMessage( int code );

#endif /* ERR_H_ */
