# Makefile

CARM = arm-linux-gnueabihf-g++
CXXFLAGS = -std=c++11 -w -g -pthread

TARGET = ISR

SRC = $(wildcard ./Base/*.cpp) $(wildcard ./TRLIB_ISR/*.cpp) $(wildcard ./Driver/*.cpp)
OBJ = $(SRC:.cpp=.o)

$(TARGET): $(OBJ)
	$(CARM) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CARM) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)

