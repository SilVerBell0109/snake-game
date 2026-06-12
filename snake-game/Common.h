// Common.h
// 전 파일에서 공유하는 상수, 구조체 정의
// (Point, Item, Mission, 방향 상수, 셀 값 상수, 타이밍 상수)

#pragma once

// ── 맵 / 아이템 상수 ──────────────────────────────────────────────
const int MAP_SIZE         = 21;
const int ITEM_LIMIT       = 3;    // Growth / Poison 각각 최대 동시 출현 수
const int ITEM_LIFE        = 300;  // 아이템 생존 틱
const int GATE_LIFE        = 200;  // 게이트 생존 틱
const int SPEED_LIMIT      = 1;    // Speed Item 최대 동시 출현 수
const int SPEED_LIFE       = 200;  // Speed Item 생존 틱
const int SPEED_BOOST_TICKS = 15;  // Speed 획득 시 부스트 지속 틱 (~3초)

// 스테이지별 이동 주기(ms) — 단계적 난이도 증가 [가산점]
const int STAGE_TICK_MS[4] = {200, 175, 150, 125};

// ── 방향 상수 ─────────────────────────────────────────────────────
const int UP    = 0;
const int DOWN  = 1;
const int LEFT  = 2;
const int RIGHT = 3;

// ── 맵 셀 값 ─────────────────────────────────────────────────────
const int CELL_EMPTY  = 0;  // 빈 공간
const int CELL_WALL   = 1;  // Wall (Gate 가능)
const int CELL_IMMUNE = 2;  // Immune Wall (Gate 불가)
const int CELL_HEAD   = 3;  // Snake Head
const int CELL_BODY   = 4;  // Snake Body
const int CELL_GROWTH = 5;  // Growth Item
const int CELL_POISON = 6;  // Poison Item
const int CELL_GATE   = 7;  // Gate
const int CELL_SPEED  = 8;  // Speed Item [가산점]

// ── 공유 구조체 ───────────────────────────────────────────────────
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

// MISSIONS 배열은 Board.cpp에서 정의, 전체 파일에서 사용 가능하도록 선언
extern const Mission MISSIONS[4];
