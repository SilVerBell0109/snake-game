// Common.h
// 전 클래스가 공유하는 상수, 열거형, 기본 구조체 정의

#pragma once

// ── 맵 / 아이템 상수 ─────────────────────────────────────────────
const int MAP_SIZE   = 21;
const int ITEM_LIMIT = 3;
const int ITEM_LIFE  = 300;
const int GATE_LIFE  = 200;
const int BASE_TICK_MS = 200;

// ── 특수 아이템 상수 ─────────────────────────────────────────────
const int SPECIAL_LIFE    = 150;
const int SPECIAL_TICK    = 20;
const int SPEED_DURATION  = 50;
const int SLOW_DURATION   = 50;
const int GHOST_DURATION  = 50;
const int MIRROR_DURATION = 50;
const int SPEED_TICK_MS   = 100;
const int SLOW_TICK_MS    = 400;

// ── 방향 상수 ────────────────────────────────────────────────────
const int UP    = 0;
const int DOWN  = 1;
const int LEFT  = 2;
const int RIGHT = 3;

// ── 셀 값 ────────────────────────────────────────────────────────
const int CELL_EMPTY   = 0;
const int CELL_WALL    = 1;
const int CELL_IMMUNE  = 2;
const int CELL_HEAD    = 3;
const int CELL_BODY    = 4;
const int CELL_GROWTH  = 5;
const int CELL_POISON  = 6;
const int CELL_GATE    = 7;
const int CELL_SPEED   = 8;
const int CELL_SLOW    = 9;
const int CELL_SHIELD  = 10;
const int CELL_GHOST   = 11;
const int CELL_MIRROR  = 12;
const int CELL_REVERSE = 13;

// ── 기본 구조체 ──────────────────────────────────────────────────
struct Point {
    int y, x;
};

struct Item {
    int y, x;
    int type;
    int life;
};

struct Mission {
    int targetLength;
    int targetGrowth;
    int targetPoison;
    int targetGate;
};

// ── 반대 방향 헬퍼 ───────────────────────────────────────────────
inline int oppositeDir(const int d) {
    if (d == UP)    return DOWN;
    if (d == DOWN)  return UP;
    if (d == LEFT)  return RIGHT;
    return LEFT;
}

extern const Mission MISSIONS[4];
