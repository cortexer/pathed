//----------------------------------------------------------------------------
// main.cpp
//
// main for pathed
//
// Copyright (C) 2011 Neil Butterworth
//----------------------------------------------------------------------------

#include <iostream>
#include <set>
#include "cmdline.h"
#include "error.h"
#include "registry.h"
#include "util.h"

using namespace std;

//----------------------------------------------------------------------------
// Command and option names
//----------------------------------------------------------------------------

enum FlagName { fnNone, fnAdd, fnRemove, fnForce, fnGrep,
				fnQuery, fnVerify, fnPrune, fnList, fnSys, fnExpand, fnEnv };

//----------------------------------------------------------------------------
// Globals set dring command parsing
//----------------------------------------------------------------------------

FlagName CommandName = fnNone;
bool Expand = false, CheckExist = true, UseSys = false;
string CommandParam = "";

//----------------------------------------------------------------------------
// Lookup commands and options via string
//----------------------------------------------------------------------------

struct Flag {
	FlagName mName;
	const char * const mShort;
	const char * const mLong;
	bool mCmd;						// is this a command?
	int mParamCount;				// if so, how many params?
};

Flag CmdLineFlags[] = {
	{ fnAdd, 		"-a", "--add", true, 1 },
	{ fnRemove, 	"-r", "--remove", true, 1 },
	{ fnList, 		"-l", "--list", true, 0 },
	{ fnQuery, 		"-q", "--query", true, 1 },
	{ fnVerify, 	"-v", "--verify", true, 0 },
	{ fnPrune, 		"-p", "--prune", true, 0 },
	{ fnSys, 		"-s", "--system", false, 0 },
	{ fnExpand, 	"-x", "--expand", false, 0 },
	{ fnForce, 		"-f", "--force", false, 0 },
	{ fnGrep, 		"-g", "--grep", true, 1 },
	{ fnEnv,		"-e", "--env", true, 0 },
	{ fnNone, NULL, NULL, false, 0 }		// must be last
};


//----------------------------------------------------------------------------
// Get list of commands, separated by commas
//----------------------------------------------------------------------------

string CommandList() {
	int i = 0;
	string cl;
	while( CmdLineFlags[i].mName != fnNone ) {
		if ( CmdLineFlags[i].mCmd ) {
			if ( cl != "" ) {
				cl += ", ";
			}
			cl +=CmdLineFlags[i].mShort;
		}
		i++;
	}
	return cl;
}

//----------------------------------------------------------------------------
// Get flag name from string rep.
//----------------------------------------------------------------------------

FlagName StringToFlag( const string & s  ) {
	int i = 0;
	while( CmdLineFlags[i].mName != fnNone ) {
		if ( s == CmdLineFlags[i].mShort || s == CmdLineFlags[i].mLong ) {
			return CmdLineFlags[i].mName;
		}
		i++;
	}
	return fnNone;
}

//----------------------------------------------------------------------------
// Get values associated with an option
//----------------------------------------------------------------------------

pair <bool,int> GetFlagValues( FlagName f ) {
	int i = 0;
	while( CmdLineFlags[i].mName != fnNone ) {
		if (CmdLineFlags[i].mName == f ) {
			return make_pair( CmdLineFlags[i].mCmd, CmdLineFlags[i].mParamCount );
		}
		i++;
	}
	throw "wonky flag ";	// never happen
}

//----------------------------------------------------------------------------
// Parse supplied command line, setting globals.
//----------------------------------------------------------------------------

void ParseCommandLine( CmdLine & cl  ) {
	while( cl.Argc() > 1 ) {
		string sflag = cl.Argv(1);
		cl.Shift();
		FlagName fn = StringToFlag( sflag );
		if ( fn == fnNone ) {
			throw Error( "Invalid command line option: " + sflag );
		}
		pair <bool,int> vals = GetFlagValues( fn );
		if ( vals.second ) {
			if ( cl.Argc() == 1 ) {
				throw Error( "Missing command line parameter for " + sflag );
			}
			CommandParam = cl.Argv(1);
			cl.Shift();
		}
		if ( vals.first ) {
			if ( CommandName != fnNone ) {
				throw Error( "Only one command option allowed" );
			}
			CommandName = fn;
		}
		else {
			switch( fn ) {
				case fnExpand:		Expand = true; break;
				case fnForce:		CheckExist = false ; break;
				case fnSys:			UseSys  = true; break;
				default:			throw Error( "bad option " );
			}
		}
	}
	if ( CommandName == fnNone ) {
		throw Error( "Need one of " + CommandList() );
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

void AddPath() {
	if ( CommandParam == "" ) {
		throw Error( "Need directory to add" );
	}
	if ( CheckExist ) {
		DWORD attr = GetFileAttributes( CommandParam.c_str() );
		if ( attr == INVALID_FILE_ATTRIBUTES || ! (attr & FILE_ATTRIBUTE_DIRECTORY ) ) {
			throw Error( "No such directory: " + CommandParam );
		}
	}

	RegPath path( UseSys ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER );
	if ( path.Find( CommandParam, RegPath::NoExpand ) ) {
		throw Error( CommandParam + " is already on the path" );
	}
	path.Add( CommandParam );
}

//----------------------------------------------------------------------------
// Remove entry from the path
//----------------------------------------------------------------------------

void RemovePath() {
	if ( CommandParam == "" ) {
		throw Error( "Need directory to remove" );
	}
	RegPath path( UseSys ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER );
	if ( ! path.Find( CommandParam , RegPath::NoExpand ) ) {
		throw Error( CommandParam + " is not on the path" );
	}
	path.Remove( CommandParam );
}

//----------------------------------------------------------------------------
// Prune duplicates and non-existent dirs from path. Use a set to detect
// dupes, but actually work with vector to meaintain path order.
//----------------------------------------------------------------------------

void PrunePath() {
	RegPath path( UseSys ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER );

	typedef std::set <string> DirSet;
	DirSet uniq;
	std::vector <string> ordered;

	for ( unsigned int i = 0; i < path.Count(); i++ ) {
		string dir = path.At( i );
		std::pair<DirSet::iterator, bool> ok = uniq.insert( dir );
		if ( ok.second ) {
			ordered.push_back( dir );
		}
		else {
			cout << "Pruned: " << dir << endl;
		}
	}

	string entry;

	for ( unsigned int i = 0; i < ordered.size(); i++ ) {
		string dir = ExpandPath( ordered[i] );
		DWORD attr = GetFileAttributes( dir.c_str() );
		if ( attr == INVALID_FILE_ATTRIBUTES ||
						! (attr & FILE_ATTRIBUTE_DIRECTORY ) ) {
			cout << "Pruned: " << ordered[i] << endl;
		}
		else {
			if ( entry != "" ) {
				entry += ";";
			}
			entry += ordered[i];
		}

	}
	path.ReplaceAll( entry );
	std::cout << entry << std::endl;
}

//----------------------------------------------------------------------------
// See if directory is on the path, if so return success code (not boolean!)
//----------------------------------------------------------------------------

int FindPath() {
	if ( CommandParam == "" ) {
		throw Error( "Need directory name" );
	}
	RegPath path( UseSys ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER );
	return  path.Find( CommandParam, Expand ? RegPath::Expand : RegPath::NoExpand ) ? 0 : 1;
}

//----------------------------------------------------------------------------
// Search path for file
//----------------------------------------------------------------------------

int GrepPath() {
	if ( CommandParam == "" ) {
		throw Error( "Need file name" );
	}
	RegPath path( UseSys ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER );
	int found = 0;
	for ( unsigned int i = 0; i < path.Count(); i++ ) {
		string epath = ExpandPath( path.At(i) );
		if ( epath == "" || epath[epath.size()-1] != '\\' ) {
			epath += '\\';
		}
		epath += CommandParam;
		DWORD attr = GetFileAttributes( epath.c_str() );
		if ( attr != INVALID_FILE_ATTRIBUTES ) {
			found++;
			cout << epath << endl;
		}
	}
	return found == 0 ? 1 : 0;
}

//----------------------------------------------------------------------------
// List path in PATH environment variable - flags have no effect.
//----------------------------------------------------------------------------

int EnvPath() {
	const char * p = getenv( "PATH" );
	if ( p == 0 ) {
		throw Error( "No PATH variable in environment!" );
	}
	while( * p ) {
		if ( * p == ';' ) {
			cout << '\n';
		}
		else {
			cout << * p;
		}
		p++;
	}
	cout << endl;
	return 0;
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

	"\npathed is a command-line tool for changing and querying the path in the registry\n\n"
	"Version 0.8\n"
	"Copyright (C) 2011 Neil Butterworth\n\n"
	"usage: pathed [-a dir | -r dir | -e | -l | -q dir | -v | -p | -g file] [-s] [-f] [-x] \n\n"
	"pathed -a dir    adds dir to the path in  the registry\n"
	"pathed -r dir    removes  dir from the path in the registry\n"
	"pathed -l        lists the entries on the current path in the registry\n"
	"pathed -e        lists the entries on the current path in the PATH environment variable\n"
	"pathed -q dir    queries registry, returns 0 if dir is on path, 1 otherwise\n"
	"pathed -g file   searches (greps) the path for all occurrences of file\n"
	"pathed -v        verifies that all directories on the path exist\n"
	"pathed -p        prunes the path by removing duplicates and non-existent directories\n\n"
	"By default, pathed works on the path in HKEY_CURRENT_USER. You can make it use\n"
	"the system path in HKEY_LOCAL_MACHINE by using the -s flag.\n\n"
	"Normally pathed will check a directory exists on disk before adding it to the\n"
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
		if ( cl.Argc() == 1 ) {
			Help();
			return 0;
		}

		ParseCommandLine( cl );

		switch( CommandName ) {
			case fnAdd:		AddPath(); break;
			case fnRemove:	RemovePath(); break;
			case fnQuery:	return FindPath();
			case fnList:	ListPath(); break;
			case fnVerify:	return VerifyPath(); break;
			case fnPrune:	PrunePath(); break;
			case fnGrep:	GrepPath(); break;
			case fnEnv:	    EnvPath(); break;
			default:		throw Error( "bad command switch" );
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
