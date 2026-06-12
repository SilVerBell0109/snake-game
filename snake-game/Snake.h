// Snake.h
// 뱀 몸통 관리, 이동, 충돌 판정, 방향 버퍼, 게임 통계 담당
// 특수 효과(Shield/Ghost/Reverse) 연동을 위해 Special 참조 수신

#pragma once
#include "Common.h"
#include <vector>

class Board;
class Food;
class Poison;
class Gate;
class Special;

class Snake {
public:
    // 맵 중앙에 길이 3으로 초기화하고 Board에 반영
    void init(Board& board);

    // 다음 틱에 적용할 방향 버퍼 설정 (반대 방향 입력은 무시)
    bool setNextDir(int d);

    // 1틱 이동: 충돌 판정(Shield/Ghost 연동), 아이템 소비, Gate 통과, Reverse 적용
    // 반환값: false = 게임 오버
    bool move(Board& board, Food& food, Poison& poison,
              Gate& gate, Special& special,
              int& poisonCount, int& gateCount);

    int  getLength()    const;
    int  getMaxLength() const;
    int  getDir()       const;

private:
    std::vector<Point> body_;
    int dir_;
    int nextDir_;
    int maxLength_;

    Point calcNextHead(int d) const;
};
