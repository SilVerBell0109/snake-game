# Makefile
# macOS(Darwin) / Linux(Ubuntu) 공용 빌드
# macOS: Homebrew ncurses 경로 자동 감지
# Linux: 시스템 설치 ncurses 사용
# 사용법: make        빌드
#         make clean  오브젝트 및 실행 파일 제거

CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

ifeq ($(shell uname), Darwin)
    BREW_NCURSES := $(shell brew --prefix ncurses 2>/dev/null)
    ifneq ($(BREW_NCURSES),)
        CXXFLAGS += -I$(BREW_NCURSES)/include
        LDFLAGS   = -L$(BREW_NCURSES)/lib -lncursesw
    else
        LDFLAGS   = -lncurses
    endif
else
    LDFLAGS   = -lncursesw
endif

TARGET = snake
SRCS   = main.cpp Board.cpp Snake.cpp \
         Item.cpp Food.cpp Poison.cpp \
         Gate.cpp Special.cpp
OBJS   = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
