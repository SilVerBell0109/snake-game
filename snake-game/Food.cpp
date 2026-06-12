// Food.cpp
// Food 클래스 구현 — Growth Item(5) 빈 칸 무작위 출현, 수명 감소/소멸, 획득 처리

#include "Food.h"
#include "Board.h"

void Food::spawn(Board& board) {
    spawnItem(CELL_GROWTH, ITEM_LIFE, ITEM_LIMIT, board);
}

void Food::update(Board& board) {
    updateItems(board);
}

bool Food::consume(const int y, const int x, Board& board) {
    for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (it->y == y && it->x == x) {
            board.setCell(y, x, board.getBase(y, x));
            items_.erase(it);
            return true;
        }
    }
    return false;
}

int Food::getCount() const {
    return (int)items_.size();
}
