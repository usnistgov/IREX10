
INC_PATHS=-I../../include
LIB_PATHS=-L. -L../.. -Wl,-rpath,.
CXX=g++ 
CXXFLAGS=-fPIC -std=c++11 $(INC_PATHS) $(LIB_PATHS)

all: ../../validate

../../validate: validate.cpp
	$(CXX) $(CXXFLAGS) validate.cpp -l IREX10 -o $@
clean:
	rm -f ../../validate

