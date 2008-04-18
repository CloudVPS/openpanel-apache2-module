// ==========================================================================
// Module application class
// -----------------------------------
// Copyright (C) 2006 OpenPanel V.O.F.
// ==========================================================================


#ifndef _MODULE_APP_H
#define _MODULE_APP_H 1


#include <grace/application.h>
#include <grace/str.h>


class moduleapp : public application
{

public:
					 moduleapp (const char *name) 
					 			: application (name)
					 {
					 	string 	indata, size;
					 	
					 	// Read line from stdin
					 	size  = fin.gets ();
					 	
					 	// If the size of the data > 0
					 	if (size.toint() > 0 )
					 	{
					 		// read data from stdin with a
					 		// 5 seconds timeout
							indata = fin.read (size.toint(), 10);					 		
							
							// Read grace xml
							data.fromxml (indata);
					 	}	 	
					 }
					~moduleapp (void)
					 {
					 }

	virtual int		 main (void)
					 {
						return 1;
					 }
					
protected:
	value			data;		///< The data xml received through stdin
	
					// Send result to stdout
	void			sendresult (int code, const string &text)
					{
						value outdata;
						
						// Set type is application id
						outdata.type (creator);
						
						// If ok...
						if (code == 1000)
						{
							outdata["OpenCORE:Result"]["code"] 	 = code;
							outdata["OpenCORE:Result"]["message"] = "OK";
							
						}
						else
						{
							outdata["OpenCORE:Result"]["error"]		= text;
							outdata["OpenCORE:Result"]["message"]	= code;
						}
						
						// Write xml to standard output
						string out;
						
						out = outdata.toxml();
						
						fout.printf ("%u\n", out.strlen());
						
						
						fout.puts (out);
					}
};



#endif
