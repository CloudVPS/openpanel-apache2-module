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
	/usr/lib/opencore/libcoremodule.a

clean:
	rm -f *.o *.exe
	rm -rf apache2module.app
	rm -f apache2module

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I/usr/include/opencore -c -g $<
