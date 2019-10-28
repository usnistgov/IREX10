
INC_PATHS=-I. -I../../include
LIB_FILE=../../libIREX10.so
CXX=g++
CXXFLAGS=-fPIC -std=c++11 -shared $(INC_PATHS)

all: $(LIB_FILE)

$(LIB_FILE): stub.cpp stub.h
	$(CXX) $(CXXFLAGS) stub.cpp -o $@
clean:
	rm -f $(LIB_FILE)

