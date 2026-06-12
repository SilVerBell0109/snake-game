// Item.h
// 아이템 추상 기반 클래스 — Food / Poison 공통 인터페이스 정의

#pragma once

#include "Common.h"
#include <vector>

class Board;

class ItemBase {
public:
    virtual ~ItemBase() = default;

    virtual void spawn(Board& board)              = 0;
    virtual void update(Board& board)             = 0;
    virtual bool consume(int y, int x, Board& board) = 0;
    virtual int  getCount() const                 = 0;

protected:
    std::vector<Item> items_;

    void spawnItem(int cellType, int life, int maxCount, Board& board);
    void updateItems(Board& board);
};
