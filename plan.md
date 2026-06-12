# Plan: 멀티파일 클래스 기반 Snake Game 전면 재작성

## 최종 파일 구조

```
snake-game/
├── Common.h          — 공유 상수, 구조체 (Point, Item, Mission, 셀값, 방향)
├── main.cpp          — 게임 루프, ncurses 초기화, Stage 흐름, 재시작
├── Board.h / Board.cpp    — 맵 배열, 스테이지 로드, 렌더링, 점수판
├── Snake.h / Snake.cpp    — 뱀 이동, 충돌 판정, 방향 버퍼
├── Item.h  / Item.cpp     — 아이템 추상 기반 클래스
├── Food.h  / Food.cpp     — Growth Item(5) 관리
├── Poison.h / Poison.cpp  — Poison Item(6) 관리
├── Special.h / Special.cpp — 추가 아이템 8~13 통합 관리
├── Gate.h  / Gate.cpp     — 게이트 생성/소멸/통과 방향
├── Makefile
├── Dockerfile
└── docker-compose.yml
```

---

## Common.h

```cpp
// Common.h
// 전 파일에서 공유하는 상수, 구조체 정의
// (방향 상수, 셀 값, 타이밍 상수, Point/Item/Mission)

#pragma once

// ── 맵 / 아이템 상수 ──────────────────────────────────────────────
const int MAP_SIZE   = 21;
const int ITEM_LIMIT = 3;     // Growth / Poison 각각 최대 동시 출현 수
const int ITEM_LIFE  = 300;   // Growth/Poison 수명 틱
const int GATE_LIFE  = 200;   // 게이트 수명 틱
const int BASE_TICK_MS = 200; // 기본 이동 주기(ms)

// 특수 아이템 공통
const int SPECIAL_LIFE = 150; // 맵 위 출현 수명 틱 (Reverse 제외)
const int SPECIAL_TICK = 20;  // 특수 아이템 생성 시도 간격 틱

// 각 효과 지속 틱 (50틱 × 200ms = 5초)
const int SPEED_DURATION  = 50;
const int SLOW_DURATION   = 50;
const int GHOST_DURATION  = 50;
const int MIRROR_DURATION = 50;

// 속도 변경값
const int SPEED_TICK_MS = 100;  // Speed Boost 중 이동 주기
const int SLOW_TICK_MS  = 400;  // Slow 중 이동 주기

// ── 방향 상수 ─────────────────────────────────────────────────────
const int UP    = 0;
const int DOWN  = 1;
const int LEFT  = 2;
const int RIGHT = 3;

// ── 맵 셀 값 ─────────────────────────────────────────────────────
const int CELL_EMPTY   = 0;
const int CELL_WALL    = 1;   // Gate 가능
const int CELL_IMMUNE  = 2;   // Gate 불가 (4구석)
const int CELL_HEAD    = 3;
const int CELL_BODY    = 4;
const int CELL_GROWTH  = 5;   // Growth Item  기호: ++  색: 초록
const int CELL_POISON  = 6;   // Poison Item  기호: --  색: 빨강
const int CELL_GATE    = 7;
const int CELL_SPEED   = 8;   // Speed Boost  기호: >>  색: 밝은파랑
const int CELL_SLOW    = 9;   // Slow         기호: <<  색: 청록
const int CELL_SHIELD  = 10;  // Shield       기호: **  색: 노랑
const int CELL_GHOST   = 11;  // Ghost        기호: @@  색: 보라
const int CELL_MIRROR  = 12;  // Mirror       기호: %%  색: 주황(RED/YELLOW)
const int CELL_REVERSE = 13;  // Reverse      기호: &&  색: 분홍

// ── 공유 구조체 ───────────────────────────────────────────────────
struct Point {
    int y, x;
};

struct Item {
    int y, x;
    int type;  // CELL_* 값
    int life;  // 남은 수명 틱
};

struct Mission {
    int targetLength;
    int targetGrowth;
    int targetPoison;
    int targetGate;
};

// MISSIONS 배열은 Board.cpp에서 정의
extern const Mission MISSIONS[4];
```

---

## 맵 구성 규칙 (원본 자료 기준)

```
테두리 4구석 (0,0)(0,20)(20,0)(20,20) = 2 (Immune Wall)
테두리 나머지 = 1 (Wall, Gate 출현 가능)
내부 장애물   = 1 (Wall)
내부 빈 공간  = 0
```

Stage별 맵 변경 (Board.cpp에 내장):
- Stage 1: 테두리만 (내부 전부 빈 공간)
- Stage 2: L자 Wall 2개 (내부)
- Stage 3: 4구석 기둥 3칸 + 가로 Wall 2줄
- Stage 4: 미로형 Wall (사각 프레임 + 내부 장벽)

---

## Board 클래스

### Board.h
```cpp
// Board.h
// 맵 배열(21×21) 관리, 스테이지 로드, ncurses 화면 렌더링 담당
// winMap(게임 맵)과 winBoard(점수판) 윈도우 생명주기를 소유

#pragma once
#include "Common.h"
#include <ncurses.h>
#include <string>

class Board {
public:
    Board();
    ~Board();

    void    loadStage(int stage);
    void    draw() const;
    void    drawScoreBoard(int stage, int elapsedSec,
                           int curLen, int maxLen,
                           int growth, int poison, int gate,
                           const Mission& m) const;
    void    drawActiveEffects(const std::string& effects) const;
    void    setCell(int y, int x, int val);
    int     getCell(int y, int x) const;
    int     getBase(int y, int x) const;
    WINDOW* getWinMap() const;
    void    showMessage(const std::string& msg) const;

private:
    int     map_[MAP_SIZE][MAP_SIZE];
    int     baseMap_[MAP_SIZE][MAP_SIZE];
    WINDOW* winMap_;
    WINDOW* winBoard_;
};
```

### Board.cpp 핵심 설계
- 생성자: `newwin(MAP_SIZE+2, MAP_SIZE*2+2, 0, 0)` + `newwin(MAP_SIZE+2, 26, 0, MAP_SIZE*2+3)`
- 소멸자: `delwin` 양쪽 호출
- `draw()`: 셀값 0~13 전부 switch 처리, 색상 pair 적용
- `drawActiveEffects()`: winBoard_ 하단에 활성 효과 문자열 출력
- 미션 배열 `MISSIONS[4]` Board.cpp에서 `extern const` 정의:
  ```cpp
  const Mission MISSIONS[4] = {
      {  8,  3, 2, 1 },  // Stage 1
      { 10,  5, 2, 2 },  // Stage 2
      { 12,  7, 3, 3 },  // Stage 3
      { 15, 10, 3, 5 },  // Stage 4
  };
  ```

---

## Item 추상 기반 클래스

### Item.h
```cpp
// Item.h
// Growth / Poison 아이템의 추상 기반 클래스
// spawn, update, consume, getCount 인터페이스 정의

#pragma once
#include "Common.h"
#include <vector>

class Board;

class ItemBase {
public:
    virtual ~ItemBase() = default;

    virtual void spawn(Board& board) = 0;
    virtual void update(Board& board) = 0;
    virtual bool consume(int y, int x, Board& board) = 0;
    virtual int  getCount() const = 0;

protected:
    std::vector<Item> items_;

    // 서브클래스에서 공통으로 사용하는 수명 감소 + 제거 로직
    void updateItems(Board& board);

    // 빈 칸 수집 후 무작위 위치에 아이템 배치
    void spawnItem(int cellType, int life, int maxCount, Board& board);
};
```

### Item.cpp
```cpp
// Item.cpp
// ItemBase 공통 구현: updateItems(), spawnItem()

void ItemBase::updateItems(Board& board) {
    for (auto it = items_.begin(); it != items_.end(); ) {
        it->life--;
        if (it->life <= 0) {
            board.setCell(it->y, it->x, board.getBase(it->y, it->x));
            it = items_.erase(it);
        } else {
            ++it;
        }
    }
}

void ItemBase::spawnItem(int cellType, int life, int maxCount, Board& board) {
    if ((int)items_.size() >= maxCount) return;
    // 빈 칸(CELL_EMPTY) 수집 후 무작위 배치
    std::vector<Point> empty;
    for (int i = 0; i < MAP_SIZE; i++)
        for (int j = 0; j < MAP_SIZE; j++)
            if (board.getCell(i, j) == CELL_EMPTY)
                empty.push_back({i, j});
    if (empty.empty()) return;
    Point p = empty[rand() % empty.size()];
    items_.push_back({p.y, p.x, cellType, life});
    board.setCell(p.y, p.x, cellType);
}
```

---

## Food 클래스

### Food.h
```cpp
// Food.h
// Growth Item(셀 값 5) 출현, 수명 관리, 획득 처리 담당
// 최대 ITEM_LIMIT개 동시 출현, 수명 ITEM_LIFE틱

#pragma once
#include "Item.h"

class Food : public ItemBase {
public:
    void spawn(Board& board) override;
    void update(Board& board) override;
    bool consume(int y, int x, Board& board) override;
    int  getCount() const override;
};
```

### Food.cpp
```cpp
void Food::spawn(Board& board)  { spawnItem(CELL_GROWTH, ITEM_LIFE, ITEM_LIMIT, board); }
void Food::update(Board& board) { updateItems(board); }
bool Food::consume(int y, int x, Board& board) {
    for (auto it = items_.begin(); it != items_.end(); ++it) {
        if (it->y == y && it->x == x) {
            board.setCell(y, x, board.getBase(y, x));
            items_.erase(it);
            return true;
        }
    }
    return false;
}
int Food::getCount() const { return (int)items_.size(); }
```

Poison.cpp는 `CELL_POISON(6)` 사용, 나머지 로직 동일.

---

## Gate 클래스

### Gate.h
```cpp
// Gate.h
// 게이트 쌍(posA_, posB_) 생성, 수명 관리, 통과 방향 계산 담당
// Wall(1) 위치에만 생성, GATE_LIFE틱 후 자동 재생성

#pragma once
#include "Common.h"

class Board;

class Gate {
public:
    Gate();
    void  spawn(Board& board);
    void  update(Board& board);
    bool  isActive() const;

    // 진입 위치/방향 → 출구 좌표/방향 계산
    // 우선순위: 진입→시계→역시계→반대
    Point getExitPos(const Point& entryPos, int entryDir,
                     const Board& board, int& exitDir) const;

private:
    Point posA_, posB_;
    bool  active_;
    int   tick_;

    int wallFacingDir(int y, int x) const;
    int oppositeDir(int d) const;
};
```

### Gate.cpp getExitPos 로직
```cpp
// 어느 게이트로 진입했는지 판별 → 상대 게이트 선택
const Point* exit = (posA_ == entryPos) ? &posB_ : &posA_;

// CW: UP→RIGHT, DOWN→LEFT, LEFT→UP, RIGHT→DOWN
// CCW: UP→LEFT, DOWN→RIGHT, LEFT→DOWN, RIGHT→UP
const int priority[4] = {entryDir, CW[entryDir], CCW[entryDir], opposite(entryDir)};

for (int i = 0; i < 4; i++) {
    // exit 위치에서 priority[i] 방향으로 한 칸 전진
    // 유효(경계 안, 비Wall) → exitDir = priority[i], return 좌표
}
```

---

## Snake 클래스

### Snake.h
```cpp
// Snake.h
// 뱀 몸통 관리, 이동, 충돌 판정, 방향 버퍼 담당

#pragma once
#include "Common.h"
#include <vector>

class Board;
class Food;
class Poison;
class Gate;
class Special;

class Snake {
public:
    void init(Board& board);
    // 반대 방향이면 즉시 실패 → returned bool: false=반대방향입력
    bool setNextDir(int d);
    // 이동 1틱. false 반환 = 게임오버
    bool move(Board& board, Food& food, Poison& poison,
              Gate& gate, Special& special,
              int& growthCount, int& poisonCount, int& gateCount);

    int  getLength()    const;
    int  getMaxLength() const;
    int  getDir()       const;
    bool checkMission(const Mission& m,
                      int growthCount, int poisonCount, int gateCount) const;

private:
    std::vector<Point> body_;
    int dir_;
    int nextDir_;
    int maxLength_;

    int   oppositeDir(int d) const;
    Point calcNextHead(int d) const;
};
```

### Snake.cpp move() 흐름
```
1. dir_ = nextDir_

2. newHead = calcNextHead(dir_)

3. 경계 밖 → return false

4. cell = board.getCell(newHead)
   - CELL_IMMUNE → return false
   - CELL_WALL   → false (Shield 활성 시 1회 무효화, Shield 소멸)
   - CELL_BODY   → false (Ghost 활성 시 통과)

5. CELL_GATE → gate.getExitPos() → newHead/dir_ 갱신, gateCount++

6. 아이템 소비:
   - CELL_GROWTH  → food.consume()   → grew=true,  growthCount++
   - CELL_POISON  → poison.consume() → shrank=true, poisonCount++
   - CELL_SPEED~CELL_REVERSE → special.consume(y,x,board)

7. 꼬리 제거 (!grew), 추가 꼬리 제거 (shrank), 길이<3 → false

8. Head 삽입, maxLength_ 갱신
```

---

## Special 클래스

### Special.h
```cpp
// Special.h
// 추가 아이템 6종(Speed/Slow/Shield/Ghost/Mirror/Reverse) 통합 관리
// 맵 위 아이템 출현/소멸, 효과 지속 시간, 상태 조회 담당

#pragma once
#include "Common.h"
#include <vector>
#include <string>

class Board;

// 맵 위에 놓인 특수 아이템 하나
struct SpecialItem {
    int y, x;
    int cellType;
    int life;   // 남은 출현 수명 틱
};

// 현재 활성화된 효과 하나
struct ActiveEffect {
    int type;        // CELL_* 값
    int remaining;   // 남은 효과 지속 틱 (Shield는 -1 = 충돌 전까지)
};

class Special {
public:
    Special();

    // 20틱마다 호출: 각 타입별 출현 조건 확인 후 spawn
    void spawnAll(Board& board);

    // 매틱 호출: 맵 위 아이템 수명 감소, 효과 지속 틱 감소
    void updateAll(Board& board);

    // (y,x)에 특수 아이템이 있으면 소비 후 효과 활성화, true 반환
    bool consume(int y, int x, Board& board);

    // Score Board 아래 표시용 효과 문자열 생성
    // 예: "[SHIELD] [GHOST 3s]"
    std::string getActiveEffectStr() const;

    // 현재 TICK_MS 반환 (Speed/Slow 효과 반영)
    int  getCurrentTickMs() const;

    bool isShieldActive() const;
    bool isGhostActive()  const;
    bool isMirrorActive() const;

private:
    std::vector<SpecialItem>  mapItems_;    // 맵 위 아이템
    std::vector<ActiveEffect> effects_;    // 현재 활성 효과

    // 맵 위 해당 타입 아이템이 없고 활성 효과도 없을 때 배치
    void trySpawn(int cellType, int conflictType, Board& board);

    // 빈 칸 무작위 선택 배치
    bool placeItem(int cellType, Board& board);

    bool hasMapItem(int cellType)   const;
    bool hasEffect(int cellType)    const;
    int  getEffectRemaining(int cellType) const;
};
```

### Special.cpp 핵심 설계

**spawnAll() 규칙:**
```cpp
// Speed: 맵에 없고 Slow 효과 미활성 시
trySpawn(CELL_SPEED, CELL_SLOW, board);

// Slow: 맵에 없고 Speed 효과 미활성 시
trySpawn(CELL_SLOW, CELL_SPEED, board);

// Shield/Ghost/Mirror/Reverse: 맵에 없고 효과 미활성 시 각각 시도
trySpawn(CELL_SHIELD,  -1, board);
trySpawn(CELL_GHOST,   -1, board);
trySpawn(CELL_MIRROR,  -1, board);
trySpawn(CELL_REVERSE, -1, board);
```

**consume() 효과 적용:**
```cpp
switch (cellType) {
    case CELL_SPEED:   effects_.push_back({CELL_SPEED,  SPEED_DURATION}); break;
    case CELL_SLOW:    effects_.push_back({CELL_SLOW,   SLOW_DURATION});  break;
    case CELL_SHIELD:  effects_.push_back({CELL_SHIELD, -1});             break; // 충돌 전까지
    case CELL_GHOST:   effects_.push_back({CELL_GHOST,  GHOST_DURATION}); break;
    case CELL_MIRROR:  effects_.push_back({CELL_MIRROR, MIRROR_DURATION});break;
    case CELL_REVERSE: // 즉시 적용 — Snake에게 Reverse 신호 전달 (flag)
}
```

**Reverse 처리:**
- `consume()` 호출 시 `reverseFlag_ = true` 설정
- Snake::move()에서 Special::wasReversed() 확인 후 body_ 역방향 재배열 + dir_ 반전

**getCurrentTickMs():**
```cpp
int Special::getCurrentTickMs() const {
    if (hasEffect(CELL_SPEED)) return SPEED_TICK_MS;  // 100ms
    if (hasEffect(CELL_SLOW))  return SLOW_TICK_MS;   // 400ms
    return BASE_TICK_MS;  // 200ms
}
```

**getActiveEffectStr():**
```cpp
// 각 활성 효과를 "[TYPE]" 또는 "[TYPE Xs]" 형태로 조합
// Shield: "[SHIELD]"
// Ghost:  "[GHOST 3s]"  (remaining / (1000/BASE_TICK_MS) 로 초 변환)
```

---

## main.cpp 구조

```cpp
// main.cpp
// 게임 루프, ncurses 초기화, Stage 흐름 제어

// ncurses 색상 쌍 (14쌍)
init_pair(1,  COLOR_BLACK,   COLOR_BLACK);    // 미사용
init_pair(2,  COLOR_WHITE,   COLOR_WHITE);    // Wall
init_pair(3,  COLOR_CYAN,    COLOR_CYAN);     // Immune Wall
init_pair(4,  COLOR_YELLOW,  COLOR_YELLOW);   // Snake Head
init_pair(5,  COLOR_GREEN,   COLOR_GREEN);    // Snake Body
init_pair(6,  COLOR_GREEN,   COLOR_BLACK);    // Growth (+)
init_pair(7,  COLOR_RED,     COLOR_BLACK);    // Poison (-)
init_pair(8,  COLOR_MAGENTA, COLOR_MAGENTA);  // Gate
init_pair(9,  COLOR_CYAN,    COLOR_BLACK);    // Speed (>)
init_pair(10, COLOR_WHITE,   COLOR_CYAN);     // Slow (<)
init_pair(11, COLOR_YELLOW,  COLOR_BLACK);    // Shield (*)
init_pair(12, COLOR_MAGENTA, COLOR_BLACK);    // Ghost (@)
init_pair(13, COLOR_RED,     COLOR_YELLOW);   // Mirror (%)
init_pair(14, COLOR_MAGENTA, COLOR_BLACK);    // Reverse (&)

// 게임 루프 핵심 흐름
while (!failed && !cleared) {
    int key = wgetch(board.getWinMap());
    if (key == KEY_UP)    { if (!snake.setNextDir(UP))    { failed=true; break; } }
    if (key == KEY_DOWN)  { if (!snake.setNextDir(DOWN))  { failed=true; break; } }
    if (key == KEY_LEFT)  { if (!snake.setNextDir(LEFT))  { failed=true; break; } }
    if (key == KEY_RIGHT) { if (!snake.setNextDir(RIGHT)) { failed=true; break; } }
    // Mirror 활성 시 LEFT↔RIGHT 반전은 Special::isMirrorActive() 확인
    if (key == 'q' || key == 'Q') { failed = true; break; }

    int tickMs = special.getCurrentTickMs();
    napms(tickMs);

    itemTick++;
    specialTick++;
    timeTick++;
    if (timeTick * tickMs >= 1000) { elapsedSec++; timeTick = 0; }

    if (!snake.move(board, food, poison, gate, special,
                    growthCount, poisonCount, gateCount)) {
        failed = true; break;
    }

    food.update(board);
    poison.update(board);
    special.updateAll(board);
    gate.update(board);
    if (!gate.isActive()) gate.spawn(board);

    if (itemTick >= 15) {
        if (food.getCount()   < ITEM_LIMIT) food.spawn(board);
        if (poison.getCount() < ITEM_LIMIT) poison.spawn(board);
        itemTick = 0;
    }
    if (specialTick >= SPECIAL_TICK) {
        special.spawnAll(board);
        specialTick = 0;
    }

    board.draw();
    board.drawScoreBoard(stage, elapsedSec,
        snake.getLength(), snake.getMaxLength(),
        growthCount, poisonCount, gateCount, MISSIONS[stage]);
    board.drawActiveEffects(special.getActiveEffectStr());

    if (snake.checkMission(MISSIONS[stage], growthCount, poisonCount, gateCount))
        cleared = true;
}
```

---

## Makefile

```makefile
# Makefile
# macOS 로컬: brew ncurses 경로 사용
# Docker(Linux): 시스템 ncurses 사용 (-lncursesw 공통)

CXX      = g++
CXXFLAGS = -std=c++11 -Wall

# macOS: brew ncurses 경로 자동 감지
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
    CXXFLAGS += -I$(shell brew --prefix ncurses)/include
    LDFLAGS   = -L$(shell brew --prefix ncurses)/lib -lncursesw
else
    LDFLAGS   = -lncursesw
endif

TARGET = snake
SRCS   = main.cpp Board.cpp Snake.cpp Item.cpp \
         Food.cpp Poison.cpp Special.cpp Gate.cpp
OBJS   = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
```

---

## Dockerfile

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libncurses5-dev \
    libncursesw5-dev \
    locales \
    && locale-gen ko_KR.UTF-8 \
    && rm -rf /var/lib/apt/lists/*

ENV LANG=ko_KR.UTF-8
ENV LC_ALL=ko_KR.UTF-8
ENV TERM=xterm-256color

WORKDIR /app
COPY snake-game/ .
RUN make

CMD ["./snake"]
```

---

## docker-compose.yml

```yaml
version: "3"
services:
  snake:
    build: .
    stdin_open: true
    tty: true
```

---

## .gitignore 추가 항목

```
snake-game/snake
snake-game/*.o
*.DS_Store
```

---

## README.md Docker 섹션 추가 내용

```markdown
## 실행 방법

### Docker (Ubuntu 22.04 — 권장)
git clone https://github.com/SilVerBell0109/snake-game.git
cd snake-game
docker compose up --build

# 재실행
docker compose up

### macOS 로컬 빌드
cd snake-game
make && ./snake

### Linux 로컬 빌드
sudo apt-get install libncurses5-dev libncursesw5-dev
cd snake-game && make && ./snake
```

---

## 의존 관계 다이어그램

```
Common.h
  ├─ Item.h / Item.cpp    (Board 참조)
  │    ├─ Food.h / Food.cpp
  │    └─ Poison.h / Poison.cpp
  ├─ Special.h / Special.cpp (Board 참조)
  ├─ Gate.h / Gate.cpp       (Board 참조)
  ├─ Board.h / Board.cpp
  └─ Snake.h / Snake.cpp     (Board, Food, Poison, Gate, Special 참조)
main.cpp (모든 클래스 사용)
```

순환 의존 없음 — 단방향.

---

## 구현 순서

1. `Common.h` 작성
2. `Board.h / Board.cpp` (맵 데이터 포함)
3. `Item.h / Item.cpp` (추상 기반)
4. `Food.h / Food.cpp` + `Poison.h / Poison.cpp`
5. `Gate.h / Gate.cpp`
6. `Special.h / Special.cpp`
7. `Snake.h / Snake.cpp`
8. `main.cpp`
9. `Makefile` + `Dockerfile` + `docker-compose.yml`
10. `make` 빌드 확인
11. `docker build` + `docker compose up` 확인

---

## 채점 기준 준수 체크리스트

| 기준 | 구현 방법 |
|------|-----------|
| 전역변수 금지 | 모든 상태는 클래스 멤버 (main은 stack 지역변수) |
| const 매개변수 | 읽기 전용 참조: `const Board&`, `const Mission&` 등 |
| const 멤버함수 | `getLength()`, `getCell()`, `isActive()`, `draw()` 등 |
| const 변수 | Common.h의 모든 상수 + MISSIONS 배열 |
| 파일 첫 줄 역할 주석 | 각 파일 최상단 // 주석 필수 |
| 의미있는 주석 | 비명확 로직에만, 줄 수 채우기 금지 |
| 방향키 조작 | KEY_UP/DOWN/LEFT/RIGHT |
| 반대 방향 즉시 실패 | setNextDir() false 반환 → main에서 failed=true |
| Mirror 적용 | main의 key 처리 시 special.isMirrorActive() 확인하여 LEFT↔RIGHT 교환 |
