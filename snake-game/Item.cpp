// Item.cpp
// ItemBase 추상 클래스 공통 헬퍼 구현 — 아이템 배치 및 수명 감소

#include "Item.h"
#include "Board.h"
#include <cstdlib>
#include <vector>

void ItemBase::spawnItem(const int cellType, const int life,
                         const int maxCount, Board& board) {
    if ((int)items_.size() >= maxCount) return;

    std::vector<Point> empty;
    for (int i = 0; i < MAP_SIZE; i++)
        for (int j = 0; j < MAP_SIZE; j++)
            if (board.getCell(i, j) == CELL_EMPTY)
                empty.push_back({i, j});

    if (empty.empty()) return;

    const Point pos = empty[rand() % (int)empty.size()];
    items_.push_back({pos.y, pos.x, cellType, life, 0});
    board.setCell(pos.y, pos.x, cellType);
}

void ItemBase::updateItems(Board& board) {
    for (auto it = items_.begin(); it != items_.end(); ) {
        it->life--;
        if (it->life <= 0) {
            board.setCell(it->y, it->x, board.getBase(it->y, it->x));
            it = items_.erase(it);
        } else {
            ++it;
        }
    }
}
