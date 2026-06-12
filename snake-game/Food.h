// Food.h
// Growth Item(5) 관리 — 수집 카운터 기반

#pragma once
#include "Item.h"

class Food : public ItemBase {
public:
    Food();

    void spawn(Board& board)                 override;
    void update(Board& board)                override;
    bool consume(int y, int x, Board& board) override;
    int  getCount() const                    override;

    int getCollectedCount() const;

private:
    int collectedCount_;
};
