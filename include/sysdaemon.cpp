// This file is part of OpenPanel - The Open Source Control Panel
// OpenPanel is free software: you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation, using version 3 of the License.
//
// Please note that use of the OpenPanel trademark may be subject to additional 
// restrictions. For more information, please visit the Legal Information 
// section of the OpenPanel website on http://www.openpanel.com/
#include "sysdaemon.h"

namespace sysd
{
	enum	result
	{
		ok				=	0x00,	// No error
		err_sysdaemon	=	0x01,	// Catched error from the sysdaemon
		err_connect		=	0x02,	// Error connecting to sysd
		err_auth		=	0x03,	// Authentication error
		err_cookie		=	0x04,	// Cookie unusable
		err_restrict	=	0x05,	// Operation restricted
		unkown			=	0x99	// Unknown error
	};
}


//	===========================================================================
//	METHOD: sysdaemon::installfile
//	===========================================================================
int sysdaemon::installfile (const string &source, const string &dest)
{

	return sysd::ok;
}


//	===========================================================================
//	METHOD: sysdaemon::deletefile
//	===========================================================================
int sysdaemon::deletefile  (const string &conffile)
{

	return sysd::ok;
}


//	===========================================================================
//	METHOD: sysdaemon::adduser
//	===========================================================================
int sysdaemon::adduser (const string &username, const string &homedir, 
				  		const string &shell, const string &passwd_hash)
{

	return sysd::ok;
}
				  

//	===========================================================================
//	METHOD: sysdaemon::deluser
//	===========================================================================
int sysdaemon::deluser (const string &username)
{

	return sysd::ok;
}


//	===========================================================================
//	METHOD: sysdaemon::setquota
//	===========================================================================
int sysdaemon::setquota (const string &username, unsigned int softlimit, 
						 unsigned int hardlimit)
{

	return sysd::ok;
}


//	===========================================================================
//	METHOD: sysdaemon::startservice
//	===========================================================================
int sysdaemon::startservice (const string &servicename)
{

	return sysd::ok;
}


//	===========================================================================
//	METHOD: sysdaemon::stopservice
//	===========================================================================
int sysdaemon::stopservice (const string &servicename)
{

	return sysd::ok;
}


//	===========================================================================
//	METHOD: sysdaemon::reloadservice
//	===========================================================================
int sysdaemon::reloadservice (const string &servicename)
{

	return sysd::ok;
}


//	===========================================================================
//	METHOD: sysdaemon::setserviceonboot
//	===========================================================================
int sysdaemon::setserviceonboot (const string &servicename, bool enabled)
{

	return sysd::ok;
}


//	===========================================================================
//	METHOD: sysdaemon::runscript
//	===========================================================================
int sysdaemon::runscript (const string &scriptname, const string &params)
{

	return sysd::ok;
}

