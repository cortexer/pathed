//----------------------------------------------------------------------------
// main.cpp
//
// main for add2path
//
// Copyright (C) 2011 Neil Butterworth
//----------------------------------------------------------------------------

#include <iostream>
#include "cmdline.h"
#include "error.h"
#include "registry.h"

using namespace std;

int main( int argc, char *argv[] )
{
	try {
		CmdLine cl( argc, argv );
		if ( cl.Argc() != 2 ) {
			throw Error( "usage: add2path path" );
		}

		RegPath path( HKEY_CURRENT_USER );
		for ( unsigned int i = 0; i < path.Count(); i++ ) {
			cout << i << ": " << path.At(i) << endl;
		}

		if ( path.Find( cl.Argv(1) ) ) {
			throw Error( cl.Argv(1) + " is already on the path" );
		}
		path.Add( cl.Argv(1) );

		return 0;
	}
	catch( const Error & e ) {
		cerr << e.what() << endl;
		return 1;
	}
	catch( ... ) {
		cerr << "unexpected exception" << endl;
		return 1;
	}
}
