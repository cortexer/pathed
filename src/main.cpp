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
#include "util.h"

using namespace std;

//----------------------------------------------------------------------------
// Flags controling behaviour in various ways.
//----------------------------------------------------------------------------

const char * const ADD_FLAG = "-a";			// add to path
const char * const REM_FLAG = "-r";			// remove from path
const char * const SYS_FLAG = "-s";			// use system instead of user path
const char * const EXIST_FLAG = "-f";		// check path exists on disk
const char * const LIST_FLAG = "-l";		// list current path
const char * const QUERY_FLAG = "-q";		// list current path
const char * const VERIFY_FLAG = "-v";		// verify path
const char * const EXPAND_FLAG = "-x";		// expand path

static bool Remove = false,
			Add = false,
			UseSys = false,
			List = false,
			CheckExist = true,
			QueryPath = false,
			Verify = false,
			Expand = false;

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
			else if ( s == EXPAND_FLAG ) {
				Expand = true;
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
			else if ( s == VERIFY_FLAG ) {
				Verify = true;
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
		cout << ( Expand ? ExpandPath( path.At(i) ) : path.At(i) ) << "\n";
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
	if ( path.Find( cl.Argv(1), RegPath::NoExpand ) ) {
		throw Error( cl.Argv(1) + " is already on the path" );
	}
	path.Add( cl.Argv(1) );
	SendMessageTimeout( HWND_BROADCAST, WM_SETTINGCHANGE, 0, 0, SMTO_ABORTIFHUNG, 0, 0  );
}

//----------------------------------------------------------------------------
// Remove entry from the path
//----------------------------------------------------------------------------

void RemovePath( CmdLine & cl ) {
	RegPath path( UseSys ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER );
	if ( ! path.Find( cl.Argv(1) , RegPath::NoExpand ) ) {
		throw Error( cl.Argv(1) + " is not on the path" );
	}
	path.Remove( cl.Argv(1) );
	SendMessageTimeout( HWND_BROADCAST, WM_SETTINGCHANGE, 0, 0, SMTO_ABORTIFHUNG, 0, 0  );
}

//----------------------------------------------------------------------------
// See if directory is on the path, if so return success code (not boolean!)
//----------------------------------------------------------------------------

int FindPath( CmdLine & cl ) {
	RegPath path( UseSys ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER );
	return  path.Find( cl.Argv(1), Expand ? RegPath::Expand : RegPath::NoExpand ) ? 0 : 1;
}

//----------------------------------------------------------------------------
// Verify directories on path exist.
//----------------------------------------------------------------------------

int VerifyPath() {
	RegPath path( UseSys ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER );
	int bad = 0;
	for ( unsigned int i = 0; i < path.Count(); i++ ) {
		string epath = ExpandPath( path.At(i) );
		DWORD attr = GetFileAttributes( epath.c_str() );
		if ( attr == INVALID_FILE_ATTRIBUTES ) {
			cout << "No such directory: " << epath << "\n";
			bad++;
		}
		else if ( ! (attr & FILE_ATTRIBUTE_DIRECTORY ) ) {
			cout << "Not a directory: " << epath << "\n";
			bad++;
		}
	}
	return bad == 0 ? 0 : 1;
}

//----------------------------------------------------------------------------
// Display help
//----------------------------------------------------------------------------

void Help() {

	cout <<

	"pathed is a command-line tool for changing the Windows path in the registry\n"
	"Version 0.1\n"
	"Copyright (C) 2011 Neil Butterworth\n\n"
	"usage: pathed [-a | -r | -l  | -q | -v] [-s] [-f]  [-x] [dir]\n\n"
	"pathed -a dir    adds dir to the path in  the registry\n"
	"pathed -r dir    removes  dir from the path in the registry\n"
	"pathed -l        lists the entries on the current path\n"
	"pathed -q dir    queries registry, returns 0 if dir is on path, 1 otherwise\n"
	"pathed -v        verifies that all directories on the path exist\n\n"
	"By default, pathed works on the path in HKEY_CURRENT_USER. You can make it use\n"
	"the system path in HKEY_LOCAL_MACHINE by using the -s flag.\n\n"
	"Normally, pathed will check a directory exists on disk before adding it to the\n"
	"path. To prevent this, use the -f flag.\n\n"
	"Paths containing environment variables such as %systemroot% will not normally have\n"
	"the variables expanded to their values. To expand them, use the -x flag\n\n"
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

		if ( cl.Argc() == 1 && ! ( List || Verify) ) {
			Help();
			return 0;
		}
		else if ( ! (List || Add || QueryPath || Remove || Verify)  ) {
			throw Error( "Need one of -a, -r, -l, -q or -v" );
		}

		if ( List ) {
			ListPath();
		}
		else if ( Verify ) {
			return VerifyPath();
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
