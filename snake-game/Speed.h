// Speed.h
// Speed Item(셀 값 8) 출현, 수명 관리, 획득 처리 담당 [가산점]
// 최대 SPEED_LIMIT개(1개) 동시 출현, 획득 시 SPEED_BOOST_TICKS 동안 속도 2배

#pragma once
#include "Common.h"
#include <vector>

class Board;

class Speed {
public:
    // 빈 칸 중 무작위 위치에 Speed Item 1개 생성
    void spawn(Board& board);

    // 수명 1틱 감소, 만료 시 제거 후 맵 복원
    void update(Board& board);

    // (y,x)에 Speed Item이 있으면 소비(제거)하고 true 반환
    bool consume(int y, int x, Board& board);

    int getCount() const;

private:
    std::vector<Item> items_;
};
