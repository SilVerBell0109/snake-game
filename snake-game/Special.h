// Special.h
// 특수 아이템 6종(Speed/Slow/Shield/Ghost/Mirror/Reverse) 통합 관리
// 맵 아이템 출현·소멸, 활성 효과 관리, 효과 쿼리 담당

#pragma once
#include "Common.h"
#include <vector>
#include <string>

class Board;

// 현재 활성 중인 효과 (Shield: remaining=-1 → 충돌 1회 흡수 후 소멸)
struct ActiveEffect {
    int type;
    int remaining;
};

class Special {
public:
    Special();

    // 20틱마다 호출 — 각 종류별 스폰 조건 확인 후 배치
    void spawnAll(Board& board);

    // 매 틱 호출 — 맵 아이템 수명 감소/소멸, 활성 효과 카운트다운
    void updateAll(Board& board);

    // (y,x) 위치의 특수 아이템을 소비하고 효과를 effects_에 등록
    bool consume(int y, int x, Board& board);

    // Shield 효과를 소멸 (Wall/자기몸 충돌 흡수 시 호출)
    void consumeShield();

    // Reverse 소비 플래그 반환 및 초기화 (Snake::move 내부에서 확인)
    bool wasReversed();

    // ── 효과 쿼리 ──────────────────────────────────────────────
    bool isShieldActive() const;
    bool isGhostActive()  const;
    bool isMirrorActive() const;

    // 현재 틱 길이(ms) 반환 — Speed/Slow 효과 반영
    int  getCurrentTickMs() const;

    // 활성 효과 표시 문자열 반환 (e.g. "[SPEED 4s] [SHIELD]")
    std::string getActiveEffectStr() const;

private:
    std::vector<Item>         mapItems_;
    std::vector<ActiveEffect> effects_;
    bool                      reverseFlag_;

    bool hasMapItem(int cellType) const;
    bool hasEffect(int type)      const;
    void removeEffect(int type);

    // 지정 타입 아이템 1개를 빈 칸에 배치
    void placeItem(int cellType, Board& board);

    // Speed/Slow 충돌 체크 포함 스폰 시도
    void trySpawn(int cellType, int conflictType, Board& board);
};
