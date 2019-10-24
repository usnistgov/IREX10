
INC_PATHS=-I../../include
LIB_PATHS=-L../..
CXX=g++ 
CXXFLAGS=-fPIC -std=c++11 $(INC_PATHS) $(LIB_PATHS)

all: ../../validate

../../validate: validate.cpp
	$(CXX) $(CXXFLAGS) -l IREX10 validate.cpp -o $@

clean:
	rm -f ../../validate

