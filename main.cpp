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

#include <moduleapp.h>
#include "apache2Module.h"

#include <grace/file.h>
#include <grace/filesystem.h>


APPOBJECT(apache2Module);

// =========================================================================
// Main method.
// =========================================================================
int apache2Module::main (void)
{
	string conferr;

	// Add configuration watcher
	conf.addwatcher ("config", &apache2Module::confSystem);

   // Load will fail if watchers did not valiate.
    if (! conf.load ("openpanel.module.apache2", conferr))
    {   
        ferr.printf ("%% Error loading configuration: %s\n", conferr.str());
        return 1;
    } 	
	
	confpath = "%[htservice:confdir]s/%[htservice:conffile]s" %format (conf["config"]);

	if (command != "delete")
	{
		if (! checkconfig (data)) return 0;
	}
	
    subdom = data["Domain:Vhost"]["id"];
	string pdom = data["Domain"]["id"];
	subdom = subdom.left(subdom.strlen() - pdom.strlen() - 1);
	if (subdom != "")
		subdom.strcat(".");
	
	statstring classid = data["OpenCORE:Session"]["classid"];
	
	//
	// Depend the actions which has to 
	// be executed
	//
	caseselector (classid)
	{
		incaseof ("Domain:Vhost") :
			caseselector (command)
			{
				incaseof ("create") :
					if (! apache2Module::writevhost (data)) return 0;
					break;
					
				incaseof ("update") :
					if (! apache2Module::writevhost (data)) return 0;
					break;
					
				incaseof ("delete") :
					if (! apache2Module::removevhost (data)) return 0;
					
				defaultcase:
					sendresult (moderr::err_command, "Not supported");
					return 0;
			}
			break;
		
		incaseof ("System:ApachePrefs") :
			caseselector (command)
			{
				incaseof ("update") :
					if (! apache2Module::writeapache2conf (data)) return 0;
					break;
				
				defaultcase :
					sendresult (moderr::err_command, "Not supported");
					return 0;
			}
			break;

		defaultcase :
			if (command == "getconfig")
			{
				apache2Module::getconfig();
				return 0;
			}
			sendresult (moderr::err_command, "Class not supported");
			return 0;
	}
	// send quit
	if (authd.quit ())
	{
		sendresult (moderr::err_authdaemon, 
					"Error authd/quit cmd, possible rollback done");
			
		return false;		
	}

	
	sendresult (moderr::ok, "");
	return 0;
}
	

void apache2Module::getconfig (void)
{
	value ap = $attr("type","object") ->
			   $attr("parent","prefs") ->
			   $attr("parentclass","OpenCORE:Prefs") ->
			   $("keepalive","off") ->
			   $("keepalivetime",60) ->
			   $("maxclients",200) ->
			   $("maxrequests",0);
	
	string apacheconf = fs.load (confpath);
	value conflines = strutil::splitlines (apacheconf);
	bool skipthis = false;
	
	foreach (line, conflines)
	{
		string trimmed = line.sval().trim (" \t");
		value splt = strutil::splitspace (trimmed);
		
		if (skipthis && (splt[0].sval().strncasecmp ("</ifmodule>") != 0))
			continue;
			
		if (skipthis) skipthis = false;
		
		caseselector (splt[0])
		{
			incaseof ("<IfModule") :
				if (splt[1].sval() != "prefork.c>") skipthis = true;
				break;
				
			incaseof ("KeepAlive") :
				if (splt[1].sval().strcasecmp ("on") == 0)
				{
					ap["keepalive"] = "on";
				}
				break;
			
			incaseof ("KeepAliveTimeout") :
				ap["keepalivetime"] = splt[1].ival();
				break;
			
			incaseof ("MaxClients") :
				ap["maxclients"] = splt[1].ival();
				break;
			
			incaseof ("MaxRequestsPerChild") :
				ap["maxrequests"] = splt[1].ival();
				break;
			
			defaultcase :
				break;
		}
	}
	
	value ini;
	ini.loadini (conf["config"]["phpini"]);
	
	value phpp = $attr("type","object")->
				 $attr("parent","prefs")->
				 $attr("parentclass","OpenCORE:Prefs")->
				 $("outputbuffering",ini["PHP"]["output_buffering"]=="On")->
				 $("compression",ini["PHP"]["zlib.output_compression"]=="On")->
				 $("safemode",ini["PHP"]["safe_mode"]=="On")->
				 $("maxtime",ini["PHP"]["max_execution_time"].ival())->
				 $("memory",ini["PHP"]["memory_limit"].ival());


	sendresult (moderr::ok, "OK",
					$("System:ApachePrefs",
						$attr("type","class") ->
						$("apache", ap)
					 )->
					$("System:PHPPrefs",
						$attr("type","class") ->
						$("php", phpp
					 ));
}

bool apache2Module::writeapache2conf (const value &data)
{
	const value &o = data["System:ApachePrefs"];
	string fname = conf["config"]["htservice:confdir"];
	string instpath = conf["config"]["htservice:conffile"];
	
	string apacheconf = fs.load (confpath);
	value conflines = strutil::splitlines (apacheconf);
	foreach (line, conflines)
	{
		string trimmed = line.sval().trim (" \t");
		value splt = strutil::splitspace (trimmed);
		
		caseselector (splt[0])
		{
			incaseof ("KeepAlive") :
				line = "KeepAlive %s" %format (o["keepalive"]);
				break;
			
			incaseof ("KeepAliveTimeout") :
				line = "KeepAliveTimeout %i" %format (o["keepalivetime"]);
				break;
			
			incaseof ("MaxClients") :
				line = "MaxClients %i" %format (o["maxclients"]);
				break;
			
			incaseof ("MaxRequestsPerChild") :
				line = "MaxRequestsPerChild %i" %format (o["maxrequests"]);
				break;
			
			defaultcase :
				break;
		}
	}
	
	string fnpath = "/var/opencore/conf/staging/Apache2/%s" %format (fname);
	fs.save (fnpath, conflines.join ("\n"));

	if (authd.installfile (fname,instpath))
	{
		authd.rollback ();
		sendresult (moderr::err_authdaemon, "Error installing httpd.conf");
		return false;
	}
	
	if (authd.reloadservice (conf["config"]["htservice:name"]))
	{
		authd.rollback ();
		sendresult (moderr::err_authdaemon, "Error reloading service");
		return false;
	}
	
	return true;
}

// =========================================================================
// METHOD apache2Module::readconfiguration
// =========================================================================
bool apache2Module::readconfiguration (void)
{

	return true;
}



// =========================================================================
// METHOD apache2Module::writephpini
// =========================================================================
bool apache2Module::writephpini 	 (const value &v)
{

	return true;
}


// =========================================================================
// METHOD apache2Module::writevhost
// =========================================================================
bool apache2Module::writevhost 		 (value &v)
{
	//
	// Write the config files for all vhosts given
	//
	value vhost;
	vhost = v["Domain:Vhost"];

	string 	fname;	
	file	f;

	// Construct file to write
	fname.printf ("%s/%s%s.conf", 
				  conf["config"]["varpath"].cval(),
				  subdom.cval(),
				  v["Domain"]["id"].cval());

	string username = v["Domain:Vhost"]("owner");
	if (! username)
	{
		sendresult (moderr::err_value, "Object has no owner");
		return false;
	}
	
	value pw = kernel.userdb.getpwnam (username);
	if (! pw)
	{
		string err;
		err.printf ("Owner <%S> not found on system", username.str());
		sendresult (moderr::err_value, err);
		return false;
	}
	
	string homedir = pw["home"];

	if (f.openwrite (fname))
	{
		f.printf ("<VirtualHost *:80>\n");
		if (vhost["admin"].sval().strlen()) f.printf ("   ServerAdmin        \"%S\"\n", vhost["admin"].cval());
		f.printf ("   DocumentRoot       %s/web/%s%s\n", 
				  homedir.cval(),
				  subdom.cval(),
				  v["Domain"]["id"].cval());

		f.printf ("   ServerName         %s%s\n", 
				  subdom.cval(),
				  v["Domain"]["id"].cval());
				  
		if (subdom == "www.")
		{
			f.printf ("   ServerAlias        %s\n", v["Domain"]["id"].cval());
		}
	
		// handle aliasdomains
		foreach(aliasdoms, data["Domain"]["Domain:Alias"])
		{
			f.printf ("   ServerAlias         %s%s\n", 
				      subdom.cval(),
				      aliasdoms["id"].cval());
			
			if (subdom == "www")
			{
				f.printf ("   ServerAlias        %s\n", aliasdoms["id"].cval());
			}
		}

		if (vhost["mod_perl"] == "true")
		{
			f.printf ("    <Files ~ (\\.pl$)>\n"
					  "        SetHandler perl-script\n"
					  "        PerlHandler ModPerl::Registry\n"
					  "        Options ExecCGI\n"
					  "        Allow from all\n"
					  "        PerlSendHeader On\n"
					  "    </Files>\n");
		}

		//
		// Add handler types which are supported for
		// this Virtual host
		//
		f.printf ("   <Directory %s/web/%s%s>\n",
				  homedir.cval(),
				  subdom.cval(),
				  v["Domain"]["id"].cval());
		
		f.printf ("   AllowOverride      All\n");
		
		if (vhost["mod_php"] == "false")
		{
			f.printf ("       AddType text/plain .php .php3 .phtml\n");
			f.printf ("       AddType text/plain .phps\n");
		}
		
		if (vhost["mod_cgi"] == "true")
			f.printf ("       AddHandler cgi-script .cgi\n");

		// TODO: Take a close look to disable execution of perl scripts
		// (trac:22)
		
		f.printf ("   </Directory>\n");
		
		// Add include directive
		f.printf ("   Include %s/%s%s.inc/[^.#]*\n",
				  conf["config"]["htservice:vhosts_dir"].cval(),
				  subdom.cval(),
				  v["Domain"]["id"].cval());
				  
		f.printf ("</VirtualHost>\n");
		
		// Close file
		f.close ();
		
		//  TODO: this todo left unintentionally blank
		string ifile;
		ifile.printf ("%s%s.conf",  subdom.cval(),
					  v["Domain"]["id"].cval());
		
		// Install the file to the apache2 directory
		if( authd.installfile (ifile, conf["config"]["htservice:vhosts_dir"]))
		{
			// Do rollback
			authd.rollback ();

			sendresult (moderr::err_authdaemon, 
						"Error reloading service");
			
			return false;
		}
		
		// Also create a directory beside the config file
		// for configuration files to include from other 
		// modules
		//
		// TODO:
		//
		string adirname;
		adirname.printf ("%s/%s%s.inc", 
						 conf["config"]["htservice:vhosts_dir"].cval(),
						 subdom.cval(),
					  	v["Domain"]["id"].cval());
					  
		if( authd.makedir (adirname))
		{
			// Do rollback
			authd.rollback ();

			sendresult (moderr::err_authdaemon, 
						"Error creating directory");
			
			return false;
		}		

		string userdir;
		userdir.printf ("web/%s%s", subdom.cval(),
						v["Domain"]["id"].str());
		if( authd.makeuserdir (username, "0711", userdir))
		{
			// Do rollback
			authd.rollback ();
			
			string errorstr;
			errorstr.printf ("Error creating %s/%s", homedir.str(),
							 userdir.str());

			sendresult (moderr::err_authdaemon, errorstr);
			//			"Error creating /home/html/[domain] directory");
			
			return false;
		}		
		
		
		if ( authd.reloadservice (conf["config"]["htservice:name"]) )
		{
			// Do rollback
			authd.rollback ();

			sendresult (moderr::err_authdaemon, 
						"Error reloading service");
			
			return false;
		}
	}
	else
	{
		sendresult (moderr::err_writefile, 
					"Error opening file for writing");

		return false;
	}
	
	return true;			
}

// =========================================================================
// METHOD apache2Module::removevhost
// =========================================================================
bool apache2Module::removevhost (value &v)
{

	foreach (vhost, v["Domain"]["Domain:Vhost"])
	{
		string 	fname;	

		// Construct file to write
		fname.printf ("%s/vhosts/%s%s.conf", 
				   	  conf["config"]["htservice:vhosts_dir"].cval(),
				   	  subdom.cval(),
				   	  v["Domain"]["id"].cval());

        authd.deletefile (fname);

		// Try to remove the given file
		string ifile;
		ifile.printf ("%s%s",  subdom.cval(),
					  v["Domain"]["id"].cval());		
		string adirname;
		adirname.printf ("%s/%s.inc", 
						 conf["config"]["htservice:vhosts_dir"].cval(),
						 ifile.str());

        authd.deletedir (adirname);

        authd.reloadservice (conf["config"]["htservice:name"]);
	}

	// Actions were ok
	return true;
}

// =========================================================================
// METHOD apache2Module::delserveralias
// =========================================================================
bool apache2Module::delserveralias (value &v)
{
	string fname;

	// Construct file to write
	fname.printf ("%s/%s%s.inc/%s.alias", 
				  conf["config"]["htservice:vhosts_dir"].cval(),
				  subdom.cval(),
				  v["Domain"]["id"].cval(),
				  v["Vhost:Alias"]["id"].cval());

	// Remove this file from the filesystem
	if (authd.deletefile (fname))
	{
		// Do rollback 
		authd.rollback ();
		
		sendresult (moderr::err_authdaemon, 
					"Error deleting system file");
		
		return false;
	}		

	return true;
}

// =========================================================================
// METHOD apache2Module::addserveralias
// =========================================================================
bool apache2Module::addserveralias (value &v)
{
	string 	fname, dname, sname;
	file	f;
	
	// Construct file to write
	fname.printf ("%s/%s.alias", 
				  conf["config"]["varpath"].cval(),
				  v["Vhost:Alias"]["id"].cval());

	// Construct file to write
	sname.printf ("%s.alias", 
				  v["Vhost:Alias"]["id"].cval());


	dname.printf ("%s/%s%s.inc", 
				  conf["config"]["htservice:vhosts_dir"].cval(),
				  subdom.cval(),
				  v["Domain"]["id"].cval());


	// Error if failed to open file for writing
	if (!f.openwrite (fname))
	{
		sendresult (moderr::err_writefile, "Error opening file for writing");
		return false;		
	}

	// Install the file
	if( authd.installfile (sname, dname))
	{
		// Do rollback
		authd.rollback ();

		sendresult (moderr::err_authdaemon, 
					"Error installing ");
		
		return false;
	}
	
	if ( authd.reloadservice (conf["config"]["htservice:name"]) )
	{
		// Do rollback
		authd.rollback ();

		sendresult (moderr::err_authdaemon, 
					"Error reloading service");
		
		return false;
	}	
	
	f.printf ("# Openpanel ServerAlias for inclusion in the main \n");
	f.printf ("# virtual host configuration file \n\n");
	f.printf ("ServerAlias %s\n", v["Vhost:Alias"]["id"].cval());

	f.close ();
	
	return true;
}

// =========================================================================
// METHOD apache2Module::checkconfig
// =========================================================================
bool apache2Module::checkconfig (value &v)
{

	// Check given xml on errors, if an error has been found
	// print the error and return false so the main function 
	// knows to exit.
	
	// We only check if the body data is correct and do not any 
	// xml validation checks
	
	string classid = v["OpenCORE:Session"]["classid"];
	
	caseselector (classid)
	{
		incaseof ("Domain:Vhost") :
			if (v.exists("Domain"))
			{
				if (v.exists("Domain:Vhost"))
				{
					string username = v["Domain:Vhost"]("owner");
					if (! username)
					{
						sendresult (moderr::err_value, "Object has no owner");
						return false;
					}
					
				}
			}
			sendresult (moderr::err_context, 
						"Context body does not exists");	
			return false;
			
		incaseof ("VHost:Alias") : return true;
		incaseof ("System:ApachePrefs") : return true;
		incaseof ("System:PHPPrefs") : return true;
		defaultcase :
			if (command != "getconfig")
			{
				sendresult (moderr::err_context, "Context not supported");
				return false;
			}
			return true;
	}
	
	// unreachable
	return false;
}

// =========================================================================
// METHOD apache2Module::confSystem
// =========================================================================
bool apache2Module::confSystem (config::action act, keypath &kp,
                const value &nval, const value &oval)
{
	switch (act)
	{
		case config::isvalid:
			return true;

		case config::create:
			return true;		
	}

	return false;
}
