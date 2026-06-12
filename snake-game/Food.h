// Food.h
// Growth Item(5) 관리 — 번호(1~9) 부여, 9개 모두 수집 시 스테이지 클리어

#pragma once
#include "Item.h"

class Food : public ItemBase {
public:
    Food();

    void spawn(Board& board)                 override;
    void update(Board& board)                override;
    bool consume(int y, int x, Board& board) override;
    int  getCount() const                    override;

    bool       allCollected()      const;
    int        getCollectedCount() const;
    const bool* getCollected()     const { return collected_; }

private:
    int  nextNum_;      // next number to assign (1-9, cycles through uncollected)
    bool collected_[9]; // collected_[i] = true if +{i+1} has been eaten
};
