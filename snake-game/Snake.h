// Snake.h
// 뱀 몸통 관리, 이동, 충돌 판정, 방향 버퍼, 게임 통계(Growth·Poison·Gate 횟수) 담당

#pragma once
#include "Common.h"
#include <vector>

class Board;
class Food;
class Poison;
class Gate;
class Speed;

class Snake {
public:
    // 맵 중앙에 길이 3으로 초기화하고 Board에 반영
    void init(Board& board);

    // 다음 틱에 적용할 방향 버퍼 설정 (반대 방향이면 무시)
    void setNextDir(int d);

    // 1틱 이동: 충돌 판정, 아이템 소비, 게이트 통과, 맵 갱신
    // 반환값: false = 게임 오버(충돌), true = 정상 이동
    bool move(Board& board, Food& food, Poison& poison,
              Gate& gate, Speed& speed);

    int  getLength()    const;
    int  getMaxLength() const;
    int  getDir()       const;

    // 누적 통계 — 스테이지 내 총 획득 횟수
    int  getGrowthCount() const;
    int  getPoisonCount() const;
    int  getGateCount()   const;

    // Speed Item을 이번 틱에 획득했는지 확인 (호출 후 플래그 리셋)
    bool wasSpeedEaten();

    // 현재 스테이지 미션 달성 여부
    bool checkMission(const Mission& m) const;

private:
    std::vector<Point> body_;
    int dir_;
    int nextDir_;
    int maxLength_;
    int growthCount_;
    int poisonCount_;
    int gateCount_;
    bool speedEaten_;

    int   oppositeDir(int d) const;
    Point calcNextHead(int d) const;
};
