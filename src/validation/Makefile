
INC_PATHS=-I../../include
LIB_PATHS=-L.
CXX=g++ 
CXXFLAGS=-fPIC -std=c++11 $(INC_PATHS) $(LIB_PATHS)

all: validate

validate: validate.cpp libIREX10.so
	$(CXX) $(CXXFLAGS) -l IREX10 $^ -o $@

clean:
	rm -f ./validate

