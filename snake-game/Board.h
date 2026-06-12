// Board.h
// 맵 배열(21×21) 관리, 스테이지 로드, ncurses 화면 렌더링 담당
// winMap(게임 맵 윈도우)과 winBoard(점수판 윈도우) 생명주기를 소유

#pragma once
#include "Common.h"
#include <ncurses.h>
#include <string>

class Board {
public:
    Board();
    ~Board();

    // 지정 스테이지 맵 배열을 map_, baseMap_에 로드
    void loadStage(int stage);

    // ncurses로 맵 전체를 화면에 출력
    void draw() const;

    // 우측 점수판 윈도우에 점수·수집 현황 출력
    void drawScoreBoard(int stage, int elapsedSec,
                        int curLen, int maxLen,
                        int growthCnt,
                        int poison, int gate,
                        int score, int highScore) const;

    // 점수판 하단에 활성 특수 효과 문자열 표시
    void drawActiveEffects(const std::string& effects) const;

    void    setCell(int y, int x, int val);
    int     getCell(int y, int x) const;
    int     getBase(int y, int x) const;
    void    setLabel(int y, int x, char c);
    void    clearLabel(int y, int x);
    WINDOW* getWinMap() const;

    // 맵 중앙에 메시지를 napms(1800) 동안 표시
    void showMessage(const std::string& msg) const;

private:
    int     map_[MAP_SIZE][MAP_SIZE];
    int     baseMap_[MAP_SIZE][MAP_SIZE];
    char    label_[MAP_SIZE][MAP_SIZE];  // display char for numbered items
    WINDOW* winMap_;
    WINDOW* winBoard_;
};
