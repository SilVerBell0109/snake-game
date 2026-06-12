// Snake.cpp
// 뱀 초기화, 방향 버퍼, 1틱 이동·충돌·아이템 소비·Gate 통과·Reverse 적용

#include "Snake.h"
#include "Board.h"
#include "Food.h"
#include "Poison.h"
#include "Gate.h"
#include "Special.h"
#include <algorithm>

// ── 내부 헬퍼 ────────────────────────────────────────────────────

Point Snake::calcNextHead(const int d) const {
    Point h = body_[0];
    if (d == UP)    h.y--;
    if (d == DOWN)  h.y++;
    if (d == LEFT)  h.x--;
    if (d == RIGHT) h.x++;
    return h;
}

// ── init ─────────────────────────────────────────────────────────
void Snake::init(Board& board) {
    body_.clear();
    body_.push_back({10, 12});  // Head
    body_.push_back({10, 11});
    body_.push_back({10, 10});  // Tail

    dir_       = RIGHT;
    nextDir_   = RIGHT;
    maxLength_ = (int)body_.size();

    board.setCell(body_[0].y, body_[0].x, CELL_HEAD);
    for (int i = 1; i < (int)body_.size(); i++)
        board.setCell(body_[i].y, body_[i].x, CELL_BODY);
}

// ── setNextDir ───────────────────────────────────────────────────
// 반대 방향 입력 시 false 반환 → 즉시 게임 오버
bool Snake::setNextDir(const int d) {
    if (d == oppositeDir(dir_)) return false;
    nextDir_ = d;
    return true;
}

// ── move ─────────────────────────────────────────────────────────
bool Snake::move(Board& board, Food& food, Poison& poison,
                 Gate& gate, Special& special,
                 int& poisonCount, int& gateCount) {
    dir_ = nextDir_;
    Point newHead = calcNextHead(dir_);

    // 경계 밖
    if (newHead.y < 0 || newHead.y >= MAP_SIZE ||
        newHead.x < 0 || newHead.x >= MAP_SIZE)
        return false;

    const int cell = board.getCell(newHead.y, newHead.x);

    // ── 벽 충돌 ──────────────────────────────────────────────────
    if (cell == CELL_IMMUNE) return false;  // Immune Wall → 항상 게임 오버

    if (cell == CELL_WALL) {
        if (special.isShieldActive()) {
            special.consumeShield();
            return true;    // Shield가 흡수, 이번 틱 움직임 없음
        }
        return false;
    }

    // ── Gate 진입 ────────────────────────────────────────────────
    if (cell == CELL_GATE) {
        int exitDir = dir_;
        const Point exitPos = gate.getExitPos(newHead, dir_, board, exitDir);

        if (exitPos.y < 0 || exitPos.y >= MAP_SIZE ||
            exitPos.x < 0 || exitPos.x >= MAP_SIZE)
            return false;

        const int exitCell = board.getCell(exitPos.y, exitPos.x);
        if (exitCell == CELL_WALL || exitCell == CELL_IMMUNE) return false;

        newHead  = exitPos;
        dir_     = exitDir;
        nextDir_ = exitDir;
        gateCount++;
    }

    // ── 자기 몸 충돌 ─────────────────────────────────────────────
    // 꼬리는 이번 틱에 제거 예정이므로 마지막 한 칸 제외
    if (!special.isGhostActive()) {
        for (int i = 0; i < (int)body_.size() - 1; i++) {
            if (body_[i].y == newHead.y && body_[i].x == newHead.x) {
                if (special.isShieldActive()) {
                    special.consumeShield();
                    return true;
                }
                return false;
            }
        }
    }

    // ── 아이템 소비 ──────────────────────────────────────────────
    const int headCell = board.getCell(newHead.y, newHead.x);
    bool grew   = false;
    bool shrank = false;

    if (headCell == CELL_GROWTH) {
        if (food.consume(newHead.y, newHead.x, board))
            grew = true;
    } else if (headCell == CELL_POISON) {
        if (poison.consume(newHead.y, newHead.x, board))
            { shrank = true; poisonCount++; }
    } else if (headCell >= CELL_SPEED && headCell <= CELL_REVERSE) {
        special.consume(newHead.y, newHead.x, board);
    }

    // ── 꼬리 처리 ────────────────────────────────────────────────
    if (!grew) {
        const Point& tail = body_.back();
        board.setCell(tail.y, tail.x, board.getBase(tail.y, tail.x));
        body_.pop_back();
    }

    if (shrank) {
        if ((int)body_.size() <= 1) return false;
        const Point& tail = body_.back();
        board.setCell(tail.y, tail.x, board.getBase(tail.y, tail.x));
        body_.pop_back();
        if ((int)body_.size() < 3) return false;
    }

    // ── 기존 Head → Body, 새 Head 삽입 ──────────────────────────
    if (!body_.empty())
        board.setCell(body_[0].y, body_[0].x, CELL_BODY);

    body_.insert(body_.begin(), newHead);
    board.setCell(newHead.y, newHead.x, CELL_HEAD);

    if ((int)body_.size() > maxLength_)
        maxLength_ = (int)body_.size();

    // ── Reverse 처리 ─────────────────────────────────────────────
    if (special.wasReversed() && (int)body_.size() >= 2) {
        // 맵에서 Head/Body 전부 지움
        for (const auto& p : body_)
            board.setCell(p.y, p.x, board.getBase(p.y, p.x));

        std::reverse(body_.begin(), body_.end());
        dir_     = oppositeDir(dir_);
        nextDir_ = dir_;

        // 맵 재기록
        board.setCell(body_[0].y, body_[0].x, CELL_HEAD);
        for (int i = 1; i < (int)body_.size(); i++)
            board.setCell(body_[i].y, body_[i].x, CELL_BODY);
    }

    return true;
}

// ── getter ───────────────────────────────────────────────────────
int  Snake::getLength()    const { return (int)body_.size(); }
int  Snake::getMaxLength() const { return maxLength_; }

