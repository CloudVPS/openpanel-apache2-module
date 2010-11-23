# This file is part of OpenPanel - The Open Source Control Panel
# OpenPanel is free software: you can redistribute it and/or modify it 
# under the terms of the GNU General Public License as published by the Free 
# Software Foundation, using version 3 of the License.
#
# Please note that use of the OpenPanel trademark may be subject to additional 
# restrictions. For more information, please visit the Legal Information 
# section of the OpenPanel website on http://www.openpanel.com/

include makeinclude

OBJ	= main.o version.o

all: apache2module.exe module.xml
	./addflavor.sh
	grace mkapp apache2module 

version.cpp:
	grace mkversion version.cpp

module.xml: module.def
	mkmodulexml < module.def > module.xml

apache2module.exe: $(OBJ) module.xml
	$(LD) $(LDFLAGS) -o apache2module.exe $(OBJ) $(LIBS) \
	/usr/lib/openpanel-core/libcoremodule.a

clean:
	rm -f *.o *.exe
	rm -rf apache2module.app
	rm -f apache2module

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I/usr/include/opencore -c -g $<
