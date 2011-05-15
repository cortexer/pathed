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

//----------------------------------------------------------------------------
// Flags controling behaviour in various ways.
//----------------------------------------------------------------------------

const char * const ADD_FLAG = "-r";			// add to path
const char * const REM_FLAG = "-r";			// remove from path
const char * const SYS_FLAG = "-s";			// use system instead of user path
const char * const EXIST_FLAG = "-f";		// check path exists on disk
const char * const LIST_FLAG = "-l";		// list current path
const char * const QUERY_FLAG = "-q";		// list current path

static bool Remove = false,
			Add = false,
			UseSys = false,
			List = false,
			CheckExist = true,
			QueryPath = false;

//----------------------------------------------------------------------------
// Set flags from the command line, shifting them off as they are set.
//----------------------------------------------------------------------------

void SetFlags( CmdLine & cl ) {
	while( cl.Argc() > 1 ) {
		string s = cl.Argv(1);
		if ( s.size() && s[0] == '-' ) {
			if ( s == REM_FLAG ) {
				Remove = true;
			}
			else if ( s == ADD_FLAG ) {
				Add = true;
			}
			else if ( s == SYS_FLAG ) {
				UseSys = true;
			}
			else if ( s == EXIST_FLAG ) {
				CheckExist = false;
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

//----------------------------------------------------------------------------
// List PATH to stdout, one directory per line
//----------------------------------------------------------------------------

void ListPath() {
	RegPath path( UseSys ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER );
	for ( unsigned int i = 0; i < path.Count(); i++ ) {
		cout << path.At(i) << "\n";
	}
}

//----------------------------------------------------------------------------
// Add an entry to the path
//----------------------------------------------------------------------------

void AddPath( CmdLine & cl ) {
	if ( CheckExist ) {
		DWORD attr = GetFileAttributes( cl.Argv(1).c_str() );
		if ( attr == INVALID_FILE_ATTRIBUTES || ! (attr & FILE_ATTRIBUTE_DIRECTORY ) ) {
			throw Error( "No such directory: " + cl.Argv(1));
		}
	}

	RegPath path( UseSys ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER );
	if ( path.Find( cl.Argv(1) ) ) {
		throw Error( cl.Argv(1) + " is already on the path" );
	}
	path.Add( cl.Argv(1) );
}

//----------------------------------------------------------------------------
// Remove entry from the path
//----------------------------------------------------------------------------

void RemovePath( CmdLine & cl ) {
	RegPath path( UseSys ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER );
	if ( ! path.Find( cl.Argv(1) ) ) {
		throw Error( cl.Argv(1) + " is not on the path" );
	}
	path.Remove( cl.Argv(1) );
}

//----------------------------------------------------------------------------
// See if directory is on the path, if so return success code (not boolean!)
//----------------------------------------------------------------------------

int FindPath( CmdLine & cl ) {
	RegPath path( UseSys ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER );
	return  path.Find( cl.Argv(1) ) ? 0 : 1;
}

//----------------------------------------------------------------------------
// Display help
//----------------------------------------------------------------------------

void Help() {

	cout <<

	"pathed is a command-line tool for changing the Windows path in the registry\n"
	"Copyright (C) 2011 Neil Butterworth\n\n"
	"usage: pathed [-a | -r | -l  | -q ] [-s] [-f] [dir]\n\n"
	"pathed -a dir    adds dir to the path in  the registry\n"
	"pathed -r dir    removes  dir from the path in the registry\n"
	"pathed -l        lists the entries on the current path\n"
	"pathed -q dir    queries registry, returns 0 if dir is on path, 1 otherwise\n\n"
	"By default, pathed works on the path in HKEY_CURRENT_USER. You can make it use\n"
	"the system path in HKEY_LOCAL_MACHINE by using the -s flag.\n\n"
	"Normally, pathed will check a directory exists on disk before adding it to the\n"
	"path. To prevent this, use the -f flag.\n\n"
	"AS WITH ALL COMMANDS THAT CHANGE THE REGISTRY, PATHED CAN CAUSE DAMAGE IF YOU\n"
	"DO NOT KNOW WHAT YOU ARE DOING. IF IN DOUBT, DO NOT USE IT!\n"

	<< endl;
}

//----------------------------------------------------------------------------
// Main for pathed
//----------------------------------------------------------------------------

int main( int argc, char *argv[] )
{
	try {
		CmdLine cl( argc, argv );

		SetFlags( cl );

		if ( cl.Argc() == 1 && ! List ) {
			Help();
			return 0;
		}
		else if ( ! (List || Add || QueryPath || Remove )  ) {
			throw Error( "Need one of -a, -r, -l or -q" );
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
		cerr << "Unexpected exception" << endl;
		return 1;
	}
}
