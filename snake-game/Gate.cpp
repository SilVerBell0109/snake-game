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

// ── 내부 유틸 ────────────────────────────────────────────────────

int Gate::oppositeDir(int d) const {
    if (d == UP)    return DOWN;
    if (d == DOWN)  return UP;
    if (d == LEFT)  return RIGHT;
    return LEFT;
}

// 벽이 경계에 있을 때 맵 안쪽을 향하는 기본 방향
int Gate::wallFacingDir(int y, int x) const {
    if (y == 0)            return DOWN;
    if (y == MAP_SIZE - 1) return UP;
    if (x == 0)            return RIGHT;
    if (x == MAP_SIZE - 1) return LEFT;
    return DOWN;
}

// ── spawn ────────────────────────────────────────────────────────
void Gate::spawn(Board& board) {
    // 기존 게이트 제거
    if (active_) {
        board.setCell(posA_.y, posA_.x, board.getBase(posA_.y, posA_.x));
        board.setCell(posB_.y, posB_.x, board.getBase(posB_.y, posB_.x));
        active_ = false;
    }

    // baseMap의 Wall(1) 위치 수집
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

// ── update ───────────────────────────────────────────────────────
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

// ── getExitPos ───────────────────────────────────────────────────
// 진입 게이트 위치를 확인 후 반대쪽 게이트에서 진출 방향 탐색
// 우선순위: entryDir → 시계(CW) → 역시계(CCW) → 반대
Point Gate::getExitPos(const Point& entryPos, int entryDir,
                       const Board& board, int& exitDir) const {
    // 어느 게이트로 진입했는지 판별 → 상대 게이트 선택
    const Point* exit = nullptr;
    if (posA_.y == entryPos.y && posA_.x == entryPos.x)
        exit = &posB_;
    else
        exit = &posA_;

    // 시계 방향 / 역시계 방향 테이블
    // UP→RIGHT, DOWN→LEFT, LEFT→UP, RIGHT→DOWN
    const int CW[4]  = {RIGHT, LEFT, UP,   DOWN};
    // UP→LEFT, DOWN→RIGHT, LEFT→DOWN, RIGHT→UP
    const int CCW[4] = {LEFT,  RIGHT, DOWN, UP};

    const int priority[4] = {
        entryDir,
        CW[entryDir],
        CCW[entryDir],
        oppositeDir(entryDir)
    };

    for (int i = 0; i < 4; i++) {
        const int d  = priority[i];
        int ny = exit->y, nx = exit->x;
        if (d == UP)    ny--;
        if (d == DOWN)  ny++;
        if (d == LEFT)  nx--;
        if (d == RIGHT) nx++;

        if (ny < 0 || ny >= MAP_SIZE || nx < 0 || nx >= MAP_SIZE) continue;

        const int cell = board.getCell(ny, nx);
        // Wall, ImmuneWall, Gate는 진출 불가
        if (cell == CELL_WALL || cell == CELL_IMMUNE || cell == CELL_GATE) continue;

        exitDir = d;
        return {ny, nx};
    }

    // 모든 방향이 막힌 경우(맵 구조상 거의 발생 안 함): 반대 방향 fallback
    exitDir = oppositeDir(entryDir);
    int ny = exit->y, nx = exit->x;
    if (exitDir == UP)    ny--;
    if (exitDir == DOWN)  ny++;
    if (exitDir == LEFT)  nx--;
    if (exitDir == RIGHT) nx++;
    return {ny, nx};
}
