//----------------------------------------------------------------------------
// main.cpp
//
// main for pathed
//
// Copyright (C) 2011 Neil Butterworth
//----------------------------------------------------------------------------

#include <iostream>
#include "cmdline.h"
#include "error.h"
#include "registry.h"

using namespace std;

const char * const REM_FLAG = "-r";			// remove from path
const char * const SYS_FLAG = "-s";			// use system instead of user path
const char * const EXIST_FLAG = "-e";		// check path exists on disk
const char * const LIST_FLAG = "-l";		// list current path
const char * const QUERY_FLAG = "-q";		// list current path


static bool Remove = false,
			UseSys = false,
			List = false,
			CheckExist = false,
			QueryPath = false;


void SetFlags( CmdLine & cl ) {
	while( cl.Argc() > 1 ) {
		string s = cl.Argv(1);
		if ( s.size() && s[0] == '-' ) {
			if ( s == REM_FLAG ) {
				Remove = true;
			}
			else if ( s == SYS_FLAG ) {
				UseSys = true;
			}
			else if ( s == EXIST_FLAG ) {
				CheckExist = true;
			}
			else if ( s == LIST_FLAG ) {
				List = true;
			}
			else if ( s == QUERY_FLAG ) {
				QueryPath = true;
			}
			else {
				throw Error( "Invalid flag: " + s );
			}
			cl.Shift(1);
		}
		else {
			break;
		}
	}
}

void ListPath() {
	RegPath path( UseSys ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER );
	for ( unsigned int i = 0; i < path.Count(); i++ ) {
		cout << path.At(i) << "\n";
	}
}

void AddPath( CmdLine & cl ) {
	RegPath path( UseSys ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER );
	if ( path.Find( cl.Argv(1) ) ) {
		throw Error( cl.Argv(1) + " is already on the path" );
	}
	path.Add( cl.Argv(1) );
}

void RemovePath( CmdLine & cl ) {
	RegPath path( UseSys ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER );
	if ( ! path.Find( cl.Argv(1) ) ) {
		throw Error( cl.Argv(1) + " is not on the path" );
	}
	path.Remove( cl.Argv(1) );
}

int FindPath( CmdLine & cl ) {
	RegPath path( UseSys ? HKEY_CLASSES_ROOT : HKEY_CURRENT_USER );
	return  path.Find( cl.Argv(1) ) ? 0 : 1;
}

int main( int argc, char *argv[] )
{
	try {
		CmdLine cl( argc, argv );

		SetFlags( cl );

		if ( cl.Argc() == 1 && ! List ) {
			throw Error( "usage: add2path [[-r] [-s] [-e] path]" );
		}

		if ( List ) {
			ListPath();
		}
		else if ( QueryPath ) {
			return FindPath( cl );
		}
		else if ( Remove ) {
			RemovePath( cl );
		}
		else {
			AddPath( cl );
		}

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
