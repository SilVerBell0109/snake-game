CXX      = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS  = -lncursesw
TARGET   = snake
SRC      = snake.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
