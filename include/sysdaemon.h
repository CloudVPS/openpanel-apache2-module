// This file is part of OpenPanel - The Open Source Control Panel
// OpenPanel is free software: you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation, using version 3 of the License.
//
// Please note that use of the OpenPanel trademark may be subject to additional 
// restrictions. For more information, please visit the Legal Information 
// section of the OpenPanel website on http://www.openpanel.com/

#ifndef _SYS_DAEMON_H
#define _SYS_DAEMON_H

#include <grace/str.h>

class sysdaemon
{

  public:

			 sysdaemon (const string &modulename) 
			 {
			 	_modulename = modulename;
			 }
			
			~sysdaemon (void) 
			 {
			 }

			 /// \param source
			 /// \param dest
			 /// \return ...
	int		 installfile (const string &, const string &);
	
			 /// \param conffile
			 /// \return ...
	int		 deletefile  (const string &);
	
			 /// \param username
			 /// \param homedir
			 /// \param shell
			 /// \param passwd_hash
			 /// \return ...
	int 	 adduser (const string &, const string &, 
					  const string &, const string &);
					  
			 /// \param username
			 /// \return ...
	int		 deluser (const string &);
	
	
			 /// \param username
			 /// \param softlimit
			 /// \param hardlimit
	int		 setquota (const string &, unsigned int, unsigned int);
	
	
			 /// \param servicename
			 /// \return ...
	int		 startservice (const string &);

			 /// \param servicename
			 /// \return ...
	int		 stopservice (const string &);
	
			 /// \param servicename
			 /// \return ...
	int		 reloadservice (const string &);
	
			 /// \param servicename
			 /// \param enabled true/false
	int 	 setserviceonboot (const string &, bool);
	
			 /// \param scriptname
			 /// \param params
			 /// \return ...
	int		 runscript (const string &, const string &);
	

  protected:
  			/*
  			 	protected members here
  			 */

  private:
  	string	 _modulename;
};

#endif
