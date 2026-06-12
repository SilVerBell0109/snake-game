# Makefile
# macOS (Apple Silicon) + Homebrew ncurses 빌드
# 사용법: make        빌드
#         make clean  오브젝트 및 실행 파일 제거

CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra \
           -I$(shell brew --prefix ncurses)/include
LDFLAGS  = -L$(shell brew --prefix ncurses)/lib -lncursesw

TARGET = snake
SRCS   = main.cpp Board.cpp Snake.cpp Food.cpp Poison.cpp Gate.cpp Speed.cpp
OBJS   = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
