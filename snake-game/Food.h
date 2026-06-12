// Food.h
// Growth Item(셀 값 5) 출현, 수명 관리, 획득 처리 담당
// 최대 ITEM_LIMIT개 동시 출현, 수명 ITEM_LIFE틱 후 자동 소멸

#pragma once
#include "Common.h"
#include <vector>

class Board;

class Food {
public:
    // 빈 칸 중 무작위 위치에 Growth Item 1개 생성
    void spawn(Board& board);

    // 각 아이템 수명 1틱 감소, 만료된 아이템 제거 후 맵 복원
    void update(Board& board);

    // (y,x)에 Growth Item이 있으면 소비(제거)하고 true 반환
    bool consume(int y, int x, Board& board);

    int getCount() const;

private:
    std::vector<Item> items_;
};
