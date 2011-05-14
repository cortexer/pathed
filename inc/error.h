//----------------------------------------------------------------------------
// error.h
//
// We throw these kind of exceptions
//
// Copyright (C) 2011 Neil Butterworth
//----------------------------------------------------------------------------

#ifndef INC_A2P_ERROR_H
#define INC_A2P_ERROR_H

#include <exception>
#include <string>

class Error : public std::exception {

	public:

		Error( const std::string & msg ) : mMsg( msg ) {}
		~Error() throw() {}

		const char *what() const throw() {
			return mMsg.c_str();
		}

	private:
		std::string mMsg;
};

#endif

