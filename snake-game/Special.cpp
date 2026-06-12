// Special.cpp
// 특수 아이템 6종(Speed/Slow/Shield/Ghost/Mirror/Reverse) 스폰/갱신/소비/효과 관리

#include "Special.h"
#include "Board.h"
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <sstream>

Special::Special() : reverseFlag_(false) {}

// ── 내부 헬퍼 ──────────────────────────────────────────────────────────────

bool Special::hasMapItem(const int cellType) const {
    for (const auto& it : mapItems_)
        if (it.type == cellType) return true;
    return false;
}

bool Special::hasEffect(const int type) const {
    for (const auto& e : effects_)
        if (e.type == type) return true;
    return false;
}

void Special::removeEffect(const int type) {
    effects_.erase(
        std::remove_if(effects_.begin(), effects_.end(),
                       [type](const ActiveEffect& e){ return e.type == type; }),
        effects_.end());
}

void Special::placeItem(const int cellType, Board& board) {
    std::vector<Point> empty;
    for (int i = 0; i < MAP_SIZE; i++)
        for (int j = 0; j < MAP_SIZE; j++)
            if (board.getCell(i, j) == CELL_EMPTY)
                empty.push_back({i, j});
    if (empty.empty()) return;
    const Point pos = empty[rand() % (int)empty.size()];
    mapItems_.push_back({pos.y, pos.x, cellType, SPECIAL_LIFE, 0});
    board.setCell(pos.y, pos.x, cellType);
}

// Speed/Slow 는 서로 충돌하면 스폰 금지
void Special::trySpawn(const int cellType, const int conflictType, Board& board) {
    if (hasMapItem(cellType)) return;
    if (conflictType != -1 && hasEffect(conflictType)) return;
    if (conflictType != -1 && hasMapItem(conflictType)) return;
    placeItem(cellType, board);
}

// ── 공개 인터페이스 ────────────────────────────────────────────────────────

void Special::spawnAll(Board& board) {
    trySpawn(CELL_SPEED,   CELL_SLOW,  board);
    trySpawn(CELL_SLOW,    CELL_SPEED, board);
    trySpawn(CELL_SHIELD,  -1,         board);
    trySpawn(CELL_GHOST,   -1,         board);
    trySpawn(CELL_MIRROR,  -1,         board);
    trySpawn(CELL_REVERSE, -1,         board);
}

void Special::updateAll(Board& board) {
    // 맵 아이템 수명 감소
    for (auto it = mapItems_.begin(); it != mapItems_.end(); ) {
        it->life--;
        if (it->life <= 0) {
            board.setCell(it->y, it->x, board.getBase(it->y, it->x));
            it = mapItems_.erase(it);
        } else {
            ++it;
        }
    }
    // 활성 효과 카운트 감소 (remaining==-1 은 Shield, 횟수 기반이므로 변경 없음)
    for (auto it = effects_.begin(); it != effects_.end(); ) {
        if (it->remaining > 0) {
            it->remaining--;
            if (it->remaining == 0) {
                it = effects_.erase(it);
                continue;
            }
        }
        ++it;
    }
}

bool Special::consume(const int y, const int x, Board& board) {
    for (auto it = mapItems_.begin(); it != mapItems_.end(); ++it) {
        if (it->y != y || it->x != x) continue;

        const int type = it->type;
        board.setCell(y, x, board.getBase(y, x));
        mapItems_.erase(it);

        switch (type) {
        case CELL_SPEED:
            removeEffect(CELL_SPEED);
            effects_.push_back({CELL_SPEED, SPEED_DURATION});
            break;
        case CELL_SLOW:
            removeEffect(CELL_SLOW);
            effects_.push_back({CELL_SLOW, SLOW_DURATION});
            break;
        case CELL_SHIELD:
            if (!hasEffect(CELL_SHIELD))
                effects_.push_back({CELL_SHIELD, -1});   // -1 = 충돌 1회까지 유지
            break;
        case CELL_GHOST:
            removeEffect(CELL_GHOST);
            effects_.push_back({CELL_GHOST, GHOST_DURATION});
            break;
        case CELL_MIRROR:
            removeEffect(CELL_MIRROR);
            effects_.push_back({CELL_MIRROR, MIRROR_DURATION});
            break;
        case CELL_REVERSE:
            reverseFlag_ = true;   // Snake::move()에서 wasReversed()로 확인 후 처리
            break;
        }
        return true;
    }
    return false;
}

// Shield 1회 소비 (벽/자기 충돌 시 Snake에서 호출)
void Special::consumeShield() {
    removeEffect(CELL_SHIELD);
}

// Reverse 플래그 확인 후 리셋
bool Special::wasReversed() {
    if (reverseFlag_) { reverseFlag_ = false; return true; }
    return false;
}

bool Special::isShieldActive() const { return hasEffect(CELL_SHIELD); }
bool Special::isGhostActive()  const { return hasEffect(CELL_GHOST);  }
bool Special::isMirrorActive() const { return hasEffect(CELL_MIRROR); }

int Special::getCurrentTickMs() const {
    if (hasEffect(CELL_SPEED)) return SPEED_TICK_MS;
    if (hasEffect(CELL_SLOW))  return SLOW_TICK_MS;
    return BASE_TICK_MS;
}

std::string Special::getActiveEffectStr() const {
    std::ostringstream oss;
    for (const auto& e : effects_) {
        switch (e.type) {
        case CELL_SPEED:   oss << "SPEED("   << e.remaining << ") "; break;
        case CELL_SLOW:    oss << "SLOW("    << e.remaining << ") "; break;
        case CELL_SHIELD:  oss << "SHIELD ";                          break;
        case CELL_GHOST:   oss << "GHOST("   << e.remaining << ") "; break;
        case CELL_MIRROR:  oss << "MIRROR("  << e.remaining << ") "; break;
        }
    }
    return oss.str();
}
