//----------------------------------------------------------------------------
// registry.cpp
//
// registry manipulation stuff
//
// Copyright (C) 2011 Neil Butterworth
//----------------------------------------------------------------------------

#include <iostream>
#include <algorithm>
#include "registry.h"
#include "error.h"
using std::string;

//----------------------------------------------------------------------------
// Create a RegPath object from a registry key, which must be either 
// HKEY_CURRENT_USER or HKEY_LOCAL_MACHINE. 
//----------------------------------------------------------------------------

RegPath :: RegPath( HKEY root ) : mRoot( root ), mPathKey( 0 ) {
	long res = 0;
	if ( mRoot == HKEY_CURRENT_USER ) {
		res = RegOpenKeyEx( mRoot, "Environment", 0, KEY_ALL_ACCESS, & mPathKey);
	}
	else {
		throw Error( "not yet implemented ") ;
	}
	if ( res != ERROR_SUCCESS ) {
		throw Error( "Could not get registry key - " + LastWinError() );
	}

	const int BUFSIZE = 2048;
	BYTE buffer[ BUFSIZE + 1 ];
	DWORD bufflen = BUFSIZE;
	DWORD type = 0;
	res = RegQueryValueEx( mPathKey, "PATH", NULL, & type, buffer, & bufflen   );
	if ( res != ERROR_SUCCESS ) {
		throw Error( "Could not get registry value - " + LastWinError() );
	}
	buffer[ bufflen ] = 0;
	SplitPath( (const char *) buffer );
	std::cout << "[" << buffer << "]" << std::endl;
}

//----------------------------------------------------------------------------
// Close the path key opened in ctor.
//----------------------------------------------------------------------------

RegPath :: ~RegPath() {
	if ( mPathKey ) {
		RegCloseKey( mPathKey );
	}
}

//----------------------------------------------------------------------------
// How many directories on path?
//----------------------------------------------------------------------------

unsigned int RegPath :: Count() const {
	return mPath.size();
}

//----------------------------------------------------------------------------
// Get zero-based directory
//----------------------------------------------------------------------------

string RegPath :: At( unsigned int  i) const {
	return mPath.at( i );
}

//----------------------------------------------------------------------------
// Helper to split path at ';' character
//----------------------------------------------------------------------------

void RegPath :: SplitPath( const std::string & path ) {
	string::size_type pos = 0;

	while( pos != string::npos ) {
		string::size_type fpos = path.find( ";", pos );
		string s;
		if ( fpos == string::npos ) {
			s = path.substr( pos );
			pos = string::npos;
		}
		else {
			s = path.substr( pos, fpos - pos );
			pos = fpos + 1;
		}
		if ( s.find_first_not_of( " \t" ) != string::npos ) {
			mPath.push_back( s );
		}
	}
}

//----------------------------------------------------------------------------
// See if path contains adir
//----------------------------------------------------------------------------

bool RegPath :: Find( const string & adir ) const {

	return std::find( mPath.begin(), mPath.end(), adir ) != mPath.end();
}

//----------------------------------------------------------------------------
// Add directory to path - no check for multiple adds of same directory
//----------------------------------------------------------------------------

bool RegPath :: Add( const string & adir ) {
	mPath.push_back( adir );
	UpdateReg();
	return true;
}

//----------------------------------------------------------------------------
// Helper to update registry with current path.
//----------------------------------------------------------------------------

void RegPath :: UpdateReg() {
	string newpath;
	for ( unsigned int i = 0; i < mPath.size(); i++ ) {
		newpath += mPath[i] + ";";
	}

	long res = RegSetValueEx( mPathKey, "PATH", 0, REG_EXPAND_SZ,
								(BYTE *)newpath.c_str(), newpath.size() + 1 );
	if ( res != ERROR_SUCCESS ) {
		throw Error( "Could not add update path in registry - " + LastWinError() );
	}
}

//----------------------------------------------------------------------------
// Remove single instance of adir from path, updating registry.
//----------------------------------------------------------------------------

bool RegPath :: Remove( const string & adir ) {
	VecType::iterator it = std::find( mPath.begin(), mPath.end(), adir );
	if ( it != mPath.end() ) {
		mPath.erase( it );
	}
	UpdateReg();
	return true;
}


