// This file is part of OpenPanel - The Open Source Control Panel
// OpenPanel is free software: you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation, using version 3 of the License.
//
// Please note that use of the OpenPanel trademark may be subject to additional 
// restrictions. For more information, please visit the Legal Information 
// section of the OpenPanel website on http://www.openpanel.com/
#include <openpanel-core/moduleapp.h>
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
			
		incaseof ("System:PHPPrefs") :
			caseselector (command)
			{
				incaseof ("update") :
					if (! apache2Module::writephpini (data)) return 0;
					break;
					
				defaultcase :
					sendresult (moderr::err_command, "Not supported");
					return 0;
			}

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
				 $("outputbuffering",ini["PHP"]["output_buffering"]=="On")->
				 $("exposephp",ini["PHP"]["expose_php"]=="On")->
				 $("compression",ini["PHP"]["zlib.output_compression"]=="On")->
				 $("displayerrors",ini["PHP"]["display_errors"]=="On")->
				 $("safemode",ini["PHP"]["safe_mode"]=="On")->
				 $("urlopen",ini["PHP"]["allow_url_fopen"]=="On")->
				 $("enabled",ini["PHP"]["enable_dl"]=="On")->
				 $("uploads",ini["PHP"]["file_uploads"]=="On")->
				 $("maxtime",ini["PHP"]["max_execution_time"].ival())->
				 $("postsize",ini["PHP"]["post_max_size"].ival())->
				 $("memory",ini["PHP"]["memory_limit"].ival());


	ap["System:PHPPrefs"] = $attr("type","class") -> $("php", phpp);

	sendresult (moderr::ok, "OK",
					$("System:ApachePrefs",
						$attr("type","class") ->
						$("apache", ap)
					 ));
}

bool apache2Module::writephpini (const value &data)
{
	const value &o = data["System:PHPPrefs"];
	string inifn = conf["config"]["phpini"];
	string inipath = inifn.cutatlast ('/');
	string tmpini = "/var/openpanel/conf/staging/Apache2/%s" %format (inifn);
	
	string in = fs.load (conf["config"]["phpini"]);
	value lines = strutil::splitlines (in);
	foreach (line, lines)
	{
		#define SETBOOL(mykey,inikey) \
			if (li.strncmp (inikey " ", strlen(inikey)+1) == 0) { \
				if (o[mykey]) line = inikey " = On"; \
				else line = inikey " = Off"; \
				continue; \
			}
		
		#define SETINT(mykey,inikey) \
			if (li.strncmp (inikey " ", strlen(inikey)+1) == 0) { \
				line = inikey " = %i" %format(o[mykey]); \
				continue; \
			}
		
		const string &li = line.sval();
		
		SETBOOL("outputbuffering","output_buffering");
		SETBOOL("exposephp","expose_php");
		SETBOOL("compression","zlib.output_compression");
		SETBOOL("displayerrors","display_errors");
		SETBOOL("safemode","safe_mode");
		SETBOOL("uploads","file_uploads");
		SETBOOL("urlopen","allow_url_fopen");
		SETBOOL("enabled","enable_dl");
		SETINT("maxtime","max_execution_time");
		SETINT("postsize","post_max_size");
		SETINT("memory","memory_limit");
		
		#undef SETBOOL
		#undef SETINT
	}
	
	fs.save (tmpini, lines.join ("\n"));
	if (authd.installfile (inifn, inipath))
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

bool apache2Module::writeapache2conf (const value &data)
{
	const value &o = data["System:ApachePrefs"];
	string fname = conf["config"]["htservice:conffile"];
	string instpath = conf["config"]["htservice:confdir"];
	
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
	
	string fnpath = "/var/openpanel/conf/staging/Apache2/%s" %format (fname);
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
	    f.writeln ("################################################################################");
	    f.writeln ("## WARNING: AUTOMATICALLY GENERATED                                           ##");
	    f.writeln ("## This file was automatically generated by OpenPanel. Manual changes to this ##");
	    f.writeln ("## file will be lost the next time this file is generated.                    ##");
	    f.writeln ("################################################################################");
	    f.writeln ("\n");
	    
	
		f.printf ("<VirtualHost *:80>\n");
		if (vhost["admin"].sval().strlen()) f.printf ("   ServerAdmin        \"%S\"\n", vhost["admin"].cval());
		f.printf ("   DocumentRoot       %s/sites/%s%s/public_html\n", 
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

		//
		// Add handler types which are supported for
		// this Virtual host
		//
		f.printf ("   <Directory %s/sites/%s%s/public_html>\n",
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
		
		// Add include directives
		f.printf ("   Include %s/%s%s.inc/[^.#]*\n",
				  conf["config"]["htservice:vhosts_dir"].cval(),
				  subdom.cval(),
				  v["Domain"]["id"].cval());
		
		f.printf ("   Include %s/global.inc\n",
				  conf["config"]["htservice:vhosts_dir"].cval());
		
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

		string userdir = "sites/%s%s/public_html"
								%format (subdom,v["Domain"]["id"]);

		string webroot = "%s/%s" %format (homedir,userdir);
		if (! fs.exists (webroot))
		{
			if( authd.makeuserdir (username, "0711", userdir))
			{
				// Do rollback
				authd.rollback ();
				
				string errorstr = "Error creating %s" %format (webroot);
				sendresult (moderr::err_authdaemon, errorstr);
				return false;
			}
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

	foreach (vhost, v["Domain:Vhost"])
	{
		string 	fname;	

		// Construct file to write
		fname.printf ("%s/%s%s.conf", 
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

    f.writeln ("################################################################################");
    f.writeln ("## WARNING: AUTOMATICALLY GENERATED                                           ##");
    f.writeln ("## This file was automatically generated by OpenPanel. Manual changes to this ##");
    f.writeln ("## file will be lost the next time this file is generated.                    ##");
    f.writeln ("################################################################################");
    f.writeln ("\n");

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
					return true;
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
