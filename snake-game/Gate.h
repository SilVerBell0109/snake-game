// Gate.h
// 게이트 쌍(posA_, posB_) 생성, 수명 관리, 통과 방향 계산 담당
// Wall(1) 위치에만 생성, GATE_LIFE틱 후 자동 재생성

#pragma once
#include "Common.h"

class Board;

class Gate {
public:
    Gate();

    // baseMap의 Wall(1) 중 2곳을 무작위 선택해 게이트 쌍 생성
    void spawn(Board& board);

    // 수명 1틱 감소, 만료 시 게이트 제거 (재생성은 호출자가 판단)
    void update(Board& board);

    bool isActive() const;

    // entryPos(진입 게이트 위치)와 entryDir로 출구 위치/방향 계산
    // exitDir 에 출구 방향을 기록하고 출구에서 한 칸 나온 좌표를 반환
    // 유효한 출구가 없으면 active_ = false를 통해 실패를 알림
    Point getExitPos(const Point& entryPos, int entryDir,
                     const Board& board, int& exitDir) const;

private:
    Point posA_, posB_;
    bool  active_;
    int   tick_;

    // 게이트 위치 벽의 '안쪽 방향' 반환 (테두리 Wall 기본 진출 방향)
    int wallFacingDir(int y, int x) const;
    int oppositeDir(int d) const;
};
