include makeinclude

OBJ	= main.o version.o

all: apache2module.exe module.xml
	./addflavor.sh
	mkapp apache2module 

version.cpp:
	mkversion version.cpp

module.xml: module.def
	mkmodulexml < module.def > module.xml

apache2module.exe: $(OBJ) module.xml
	$(LD) $(LDFLAGS) -o apache2module.exe $(OBJ) $(LIBS) \
	../opencore/api/c++/lib/libcoremodule.a

clean:
	rm -f *.o *.exe
	rm -rf apache2module.app
	rm -f apache2module

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I../opencore/api/c++/include -c -g $<
