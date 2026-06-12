// Food.cpp
// Food 클래스 구현 — 번호 부여(1~9), 라벨 관리, 수집 추적, 전체 수집 시 클리어

#include "Food.h"
#include "Board.h"
#include <cstdlib>
#include <vector>

Food::Food() : nextNum_(1) {
    for (int i = 0; i < 9; i++) collected_[i] = false;
}

void Food::spawn(Board& board) {
    if ((int)items_.size() >= ITEM_LIMIT) return;

    // 현재 맵 위에 있는 번호 파악
    bool onMap[9] = {};
    for (const auto& it : items_) onMap[it.num - 1] = true;

    // nextNum_부터 순환하며 미수집 & 맵에 없는 번호 선택
    int num = 0;
    for (int i = 0; i < 9; i++) {
        const int candidate = (nextNum_ - 1 + i) % 9 + 1;
        if (!collected_[candidate - 1] && !onMap[candidate - 1]) {
            num = candidate;
            break;
        }
    }
    if (num == 0) return;  // 모두 수집됐거나 맵에 이미 올라와 있음

    std::vector<Point> empty;
    for (int i = 0; i < MAP_SIZE; i++)
        for (int j = 0; j < MAP_SIZE; j++)
            if (board.getCell(i, j) == CELL_EMPTY)
                empty.push_back({i, j});
    if (empty.empty()) return;

    const Point pos = empty[rand() % (int)empty.size()];
    items_.push_back({pos.y, pos.x, CELL_GROWTH, ITEM_LIFE, num});
    board.setCell(pos.y, pos.x, CELL_GROWTH);
    board.setLabel(pos.y, pos.x, '0' + num);
    nextNum_ = num % 9 + 1;
}

void Food::update(Board& board) {
    for (auto it = items_.begin(); it != items_.end(); ) {
        it->life--;
        if (it->life <= 0) {
            board.setCell(it->y, it->x, board.getBase(it->y, it->x));
            board.clearLabel(it->y, it->x);
            it = items_.erase(it);
        } else {
            ++it;
        }
    }
}

bool Food::consume(const int y, const int x, Board& board) {
    for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (it->y == y && it->x == x) {
            collected_[it->num - 1] = true;
            board.setCell(y, x, board.getBase(y, x));
            board.clearLabel(y, x);
            items_.erase(it);
            return true;
        }
    }
    return false;
}

int Food::getCount() const {
    return (int)items_.size();
}

bool Food::allCollected() const {
    for (int i = 0; i < 9; i++)
        if (!collected_[i]) return false;
    return true;
}

int Food::getCollectedCount() const {
    int cnt = 0;
    for (int i = 0; i < 9; i++) if (collected_[i]) cnt++;
    return cnt;
}
