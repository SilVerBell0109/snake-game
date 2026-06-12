// Poison.h
// Poison Item(6) 관리 클래스 — ItemBase 상속
// 최대 ITEM_LIMIT개 동시 출현, 수명 ITEM_LIFE틱 후 자동 소멸, 획득 시 뱀 길이-1

#pragma once
#include "Item.h"

class Poison : public ItemBase {
public:
    void spawn(Board& board)                 override;
    void update(Board& board)                override;
    bool consume(int y, int x, Board& board) override;
    int  getCount() const                    override;
};
