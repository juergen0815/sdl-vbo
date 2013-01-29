/*
 * err.cpp
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#include <windows.h>

#include <cctype>
#include <cstring>

#include <iostream>
#include <sstream>

void ShowWindowsError( const char* msg, unsigned long err, const char* header /*= "Error!"*/ )
{
    const UINT bufSize(2048);
    LPSTR lpBuffer = (LPSTR)LocalAlloc( 0, bufSize + 1);
    if ( lpBuffer ) {
        if ( std::strlen( header) == 0 ) { header = "Error!"; }
        DWORD len = FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   err,
                   LANG_SYSTEM_DEFAULT,
                   lpBuffer,
                   bufSize,
                   NULL );
        if ( len > 0 ) {
    		std::stringstream text;
    		text << msg << "\nCode: " << err << " - " << lpBuffer << std::ends;
    		std::stringbuf *buf = text.rdbuf();
    		MessageBoxA(HWND_DESKTOP, (const char*)buf->str().c_str(), header, MB_OK);
        }
        LocalFree( lpBuffer );
    }
}

void ShowError( const char* msg, const char* header /*= "Error!"*/ )
{
	MessageBoxA(HWND_DESKTOP, msg, header, MB_OK);
}

