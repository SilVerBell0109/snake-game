// Gate.cpp
// Gate 클래스 구현
// Wall(1) 위치 2곳에 게이트 쌍 생성, 수명 관리,
// 진입 방향 기반 진출 우선순위(진입→시계→역시계→반대) 계산

#include "Gate.h"
#include "Board.h"
#include <cstdlib>
#include <vector>

Gate::Gate() : active_(false), tick_(0) {
    posA_ = {0, 0};
    posB_ = {0, 0};
}

int Gate::wallFacingDir(const int y, const int x) const {
    if (y == 0)            return DOWN;
    if (y == MAP_SIZE - 1) return UP;
    if (x == 0)            return RIGHT;
    if (x == MAP_SIZE - 1) return LEFT;
    return DOWN;
}

void Gate::spawn(Board& board) {
    if (active_) {
        board.setCell(posA_.y, posA_.x, board.getBase(posA_.y, posA_.x));
        board.setCell(posB_.y, posB_.x, board.getBase(posB_.y, posB_.x));
        active_ = false;
    }

    std::vector<Point> walls;
    for (int i = 0; i < MAP_SIZE; i++)
        for (int j = 0; j < MAP_SIZE; j++)
            if (board.getBase(i, j) == CELL_WALL)
                walls.push_back({i, j});

    if ((int)walls.size() < 2) return;

    int idxA = rand() % (int)walls.size();
    int idxB = rand() % (int)walls.size();
    while (idxB == idxA) idxB = rand() % (int)walls.size();

    posA_   = walls[idxA];
    posB_   = walls[idxB];
    active_ = true;
    tick_   = GATE_LIFE;

    board.setCell(posA_.y, posA_.x, CELL_GATE);
    board.setCell(posB_.y, posB_.x, CELL_GATE);
}

void Gate::update(Board& board) {
    if (!active_) return;
    tick_--;
    if (tick_ <= 0) {
        board.setCell(posA_.y, posA_.x, board.getBase(posA_.y, posA_.x));
        board.setCell(posB_.y, posB_.x, board.getBase(posB_.y, posB_.x));
        active_ = false;
    }
}

bool Gate::isActive() const {
    return active_;
}

Point Gate::getExitPos(const Point& entryPos, const int entryDir,
                       const Board& board, int& exitDir) const {
    const Point* exit = (posA_.y == entryPos.y && posA_.x == entryPos.x)
                        ? &posB_ : &posA_;

    // CW: UP→RIGHT, DOWN→LEFT, LEFT→UP, RIGHT→DOWN
    const int CW[4]  = {RIGHT, LEFT,  UP,   DOWN};
    // CCW: UP→LEFT, DOWN→RIGHT, LEFT→DOWN, RIGHT→UP
    const int CCW[4] = {LEFT,  RIGHT, DOWN, UP};

    const int priority[4] = {
        entryDir,
        CW[entryDir],
        CCW[entryDir],
        oppositeDir(entryDir)
    };

    for (int i = 0; i < 4; i++) {
        const int d = priority[i];
        int ny = exit->y;
        int nx = exit->x;
        if (d == UP)    ny--;
        if (d == DOWN)  ny++;
        if (d == LEFT)  nx--;
        if (d == RIGHT) nx++;

        if (ny < 0 || ny >= MAP_SIZE || nx < 0 || nx >= MAP_SIZE) continue;

        const int cell = board.getCell(ny, nx);
        if (cell == CELL_WALL || cell == CELL_IMMUNE || cell == CELL_GATE) continue;

        exitDir = d;
        return {ny, nx};
    }

    // 모든 방향 막힘 — 반대 방향 fallback (이후 Snake에서 벽 충돌로 처리됨)
    exitDir = oppositeDir(entryDir);
    int ny = exit->y;
    int nx = exit->x;
    if (exitDir == UP)    ny--;
    if (exitDir == DOWN)  ny++;
    if (exitDir == LEFT)  nx--;
    if (exitDir == RIGHT) nx++;
    return {ny, nx};
}
