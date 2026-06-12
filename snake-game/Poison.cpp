// Poison.cpp
// Poison 클래스 구현 — Poison Item(6) 빈 칸 무작위 출현, 수명 감소 및 소멸, 획득 처리
// Food.cpp와 타입 값(CELL_POISON=6)만 다르고 구조는 동일

#include "Poison.h"
#include "Board.h"
#include <cstdlib>
#include <vector>

void Poison::spawn(Board& board) {
    if ((int)items_.size() >= ITEM_LIMIT) return;

    std::vector<Point> empty;
    for (int i = 0; i < MAP_SIZE; i++)
        for (int j = 0; j < MAP_SIZE; j++)
            if (board.getCell(i, j) == CELL_EMPTY)
                empty.push_back({i, j});

    if (empty.empty()) return;

    const Point pos = empty[rand() % (int)empty.size()];
    items_.push_back({pos.y, pos.x, CELL_POISON, ITEM_LIFE});
    board.setCell(pos.y, pos.x, CELL_POISON);
}

void Poison::update(Board& board) {
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

bool Poison::consume(int y, int x, Board& board) {
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
