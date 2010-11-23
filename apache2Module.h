// This file is part of OpenPanel - The Open Source Control Panel
// OpenPanel is free software: you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation, using version 3 of the License.
//
// Please note that use of the OpenPanel trademark may be subject to additional 
// restrictions. For more information, please visit the Legal Information 
// section of the OpenPanel website on http://www.openpanel.com/

#ifndef _networkingModule_H
#define _networkingModule_H 1

#include <openpanel-core/moduleapp.h>
#include <grace/system.h>
#include <grace/configdb.h>


typedef configdb<class apache2Module> appconfig;

//  -------------------------------------------------------------------------
/// Main application class.
//  -------------------------------------------------------------------------
class apache2Module : public moduleapp
{
public:
		 	 apache2Module (void) :
				moduleapp ("openpanel.module.apache2"),
				conf (this)
			 {
			 }
			~apache2Module (void)
			 {
			 }

	int		 main (void);



protected:

	appconfig		conf;			///< Modules configuration data
	
			 //	 =============================================
			 /// Configuration handler 
			 //	 =============================================
	bool     confSystem (config::action act, keypath &path, 
					  	 const value &nval, const value &oval);	
	
			 //	=============================================
			 /// validate the given configuration
			 /// \return true on ok / false on failure
			 //	=============================================
	bool	 checkconfig (value &ibody);	
	
			 //	=============================================
			 /// Read the current loaded network module
			 /// and sends the network configuration to the 
			 /// standard output or gives an error.
			 /// \return true on ok / false on failure
			 //	=============================================
	bool	 readconfiguration (void);
	
			 //	=============================================
			 /// Add a server alias configuration file
			 /// to the vhosts inclusion directory
			 //	=============================================
	bool 	 addserveralias (value &v);
				
			 //	=============================================
			 /// Removes a server alias configuration file
			 /// from the apache vhost's configuration dir
			 //	=============================================
	bool	 delserveralias (value &v);

			 //	=============================================
			 /// Writes the atual configuration
			 /// \param v given post data
			 /// \return true on ok / false on failure
			 //	=============================================
	bool 	 writeapache2conf (const value &v);
	bool	 writephpini (const value &);
	bool	 writevhost (value &);
	
			 //  ============================================
			 /// Delete file from the etc directory
			 //  ============================================
	bool	 removevhost (value &);
	
	void	 getconfig (void);
	
			 //  ============================================
			 /// Parsed and possibly dotted subdomain string
			 //  ============================================
	string   subdom;
	string	 confpath;
};

#endif
