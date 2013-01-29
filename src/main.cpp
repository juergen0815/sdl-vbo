
#include "app.h"
#include "err.h"

#include <exception>
#include <iostream>

static void HandleUnexpected()
{
    ShowError( "Application failed with unexpected Exception!", "Unexpected Exception" );
}

extern "C" int SDL_main( int argc, char* argv[] )
{
    std::set_unexpected( HandleUnexpected );
	try {
		App app;
		app.Init( argc, argv );
		return app.Run();
	}
	catch ( std::exception &ex ) {
		ShowError( ex.what(), "Application Error" );
	}
	return -1;
}
