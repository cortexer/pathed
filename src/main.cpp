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

int main()
{
	try {
		RegPath k( HKEY_CURRENT_USER );
		for ( unsigned int i = 0; i < k.Count(); i++ ) {
			cout << i << ": " << k.At(i) << endl;
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
