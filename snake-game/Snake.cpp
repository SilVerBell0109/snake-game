// Snake.cpp
// Snake 클래스 구현
// 1틱 이동: 반대 방향 무시, 경계/Wall/자기몸 충돌 판정,
// Gate 통과, Growth/Poison/Speed 아이템 소비, 맵 반영

#include "Snake.h"
#include "Board.h"
#include "Food.h"
#include "Poison.h"
#include "Gate.h"
#include "Speed.h"

// ── 내부 유틸 ────────────────────────────────────────────────────

int Snake::oppositeDir(int d) const {
    if (d == UP)    return DOWN;
    if (d == DOWN)  return UP;
    if (d == LEFT)  return RIGHT;
    return LEFT;
}

Point Snake::calcNextHead(int d) const {
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

    dir_         = RIGHT;
    nextDir_     = RIGHT;
    maxLength_   = (int)body_.size();
    growthCount_ = 0;
    poisonCount_ = 0;
    gateCount_   = 0;
    speedEaten_  = false;

    board.setCell(body_[0].y, body_[0].x, CELL_HEAD);
    for (int i = 1; i < (int)body_.size(); i++)
        board.setCell(body_[i].y, body_[i].x, CELL_BODY);
}

// ── setNextDir ───────────────────────────────────────────────────
void Snake::setNextDir(int d) {
    // 반대 방향 입력은 무시
    if (d != oppositeDir(dir_)) nextDir_ = d;
}

// ── move ─────────────────────────────────────────────────────────
bool Snake::move(Board& board, Food& food, Poison& poison,
                 Gate& gate, Speed& speed) {
    dir_ = nextDir_;

    Point newHead = calcNextHead(dir_);

    // 경계 밖
    if (newHead.y < 0 || newHead.y >= MAP_SIZE ||
        newHead.x < 0 || newHead.x >= MAP_SIZE)
        return false;

    const int cell = board.getCell(newHead.y, newHead.x);

    // Immune Wall 또는 일반 Wall 충돌
    if (cell == CELL_IMMUNE || cell == CELL_WALL) return false;

    // 자기 몸 충돌 (tail은 이번 틱 제거 예정이므로 마지막 한 칸 제외)
    for (int i = 0; i < (int)body_.size() - 1; i++) {
        if (body_[i].y == newHead.y && body_[i].x == newHead.x)
            return false;
    }

    // Gate 진입 처리
    if (cell == CELL_GATE) {
        int exitDir = dir_;
        const Point exitPos = gate.getExitPos(newHead, dir_, board, exitDir);

        // 진출 위치 유효성 검사
        if (exitPos.y < 0 || exitPos.y >= MAP_SIZE ||
            exitPos.x < 0 || exitPos.x >= MAP_SIZE)
            return false;

        const int exitCell = board.getCell(exitPos.y, exitPos.x);
        if (exitCell == CELL_WALL || exitCell == CELL_IMMUNE)
            return false;

        newHead = exitPos;
        dir_    = exitDir;
        nextDir_ = exitDir;
        gateCount_++;
    }

    // 아이템 소비 판정 (셀 값으로 분기)
    const int headCell = board.getCell(newHead.y, newHead.x);
    bool grew   = false;
    bool shrank = false;

    if (headCell == CELL_GROWTH) {
        grew = food.consume(newHead.y, newHead.x, board);
        if (grew) growthCount_++;
    } else if (headCell == CELL_POISON) {
        shrank = poison.consume(newHead.y, newHead.x, board);
        if (shrank) poisonCount_++;
    } else if (headCell == CELL_SPEED) {
        if (speed.consume(newHead.y, newHead.x, board))
            speedEaten_ = true;
    }

    // 꼬리 제거 (Growth가 아닐 때)
    if (!grew) {
        const Point& tail = body_.back();
        board.setCell(tail.y, tail.x, board.getBase(tail.y, tail.x));
        body_.pop_back();
    }

    // Poison: 추가 꼬리 1칸 더 제거
    if (shrank) {
        if ((int)body_.size() <= 1) return false;
        const Point& tail = body_.back();
        board.setCell(tail.y, tail.x, board.getBase(tail.y, tail.x));
        body_.pop_back();
        if ((int)body_.size() < 3) return false;
    }

    // 기존 Head → Body
    if (!body_.empty())
        board.setCell(body_[0].y, body_[0].x, CELL_BODY);

    // 새 Head 삽입
    body_.insert(body_.begin(), newHead);
    board.setCell(newHead.y, newHead.x, CELL_HEAD);

    if ((int)body_.size() > maxLength_)
        maxLength_ = (int)body_.size();

    return true;
}

// ── getter ───────────────────────────────────────────────────────
int Snake::getLength() const    { return (int)body_.size(); }
int Snake::getMaxLength() const { return maxLength_; }
int Snake::getDir() const       { return dir_; }
int Snake::getGrowthCount() const { return growthCount_; }
int Snake::getPoisonCount() const { return poisonCount_; }
int Snake::getGateCount()   const { return gateCount_; }

bool Snake::wasSpeedEaten() {
    const bool val = speedEaten_;
    speedEaten_ = false;
    return val;
}

bool Snake::checkMission(const Mission& m) const {
    return getLength()   >= m.targetLength &&
           growthCount_  >= m.targetGrowth &&
           poisonCount_  >= m.targetPoison &&
           gateCount_    >= m.targetGate;
}
