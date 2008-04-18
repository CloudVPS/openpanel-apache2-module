// --------------------------------------------------------------------------
// OpenPanel - The Open Source Control Panel
// Copyright (c) 2006-2007 PanelSix
//
// This software and its source code are subject to version 2 of the
// GNU General Public License. Please be aware that use of the OpenPanel
// and PanelSix trademarks and the IconBase.com iconset may be subject
// to additional restrictions. For more information on these restrictions
// and for a copy of version 2 of the GNU General Public License, please
// visit the Legal and Privacy Information section of the OpenPanel
// website on http://www.openpanel.com/
// --------------------------------------------------------------------------

#ifndef _networkingModule_H
#define _networkingModule_H 1

#include <moduleapp.h>
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
	
			 //  ============================================
			 /// Parsed and possibly dotted subdomain string
			 //  ============================================
	string   subdom;
};

#endif
