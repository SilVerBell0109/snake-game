// Poison.cpp
// Poison 클래스 구현 — Poison Item(6) 빈 칸 무작위 출현, 수명 감소/소멸, 획득 처리

#include "Poison.h"
#include "Board.h"

void Poison::spawn(Board& board) {
    spawnItem(CELL_POISON, ITEM_LIFE, ITEM_LIMIT, board);
}

void Poison::update(Board& board) {
    updateItems(board);
}

bool Poison::consume(const int y, const int x, Board& board) {
    for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (it->y == y && it->x == x) {
            board.setCell(y, x, board.getBase(y, x));
            items_.erase(it);
            return true;
        }
    }
    return false;
}

int Poison::getCount() const {
    return (int)items_.size();
}
