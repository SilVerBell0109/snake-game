# 수정 계획 (Plan) — 과제 불일치 5개 항목

승인 후 구현. 체크되지 않은 항목은 미구현.

---

## 수정 1 — 반대 방향 입력 시 Game Over

- [x] 구현 완료

### 수정 파일

| 파일 | 위치 | 변경 내용 |
|------|------|-----------|
| `snake-game/Snake.cpp` | 42번줄 | `return true;` → `return false;` |
| `snake-game/main.cpp` | 187번줄 | 반환값 체크 추가 |
| `snake-game/Snake.h` | 21번줄 주석 | "무시" → "즉시 게임 오버" |
| `snake-game/main.cpp` | 57번줄 | 시작 화면 문구 변경 |

### 변경 전/후

**Snake.cpp:40-45**
```cpp
// 변경 전
bool Snake::setNextDir(const int d) {
    if (d == oppositeDir(dir_)) return true;
    nextDir_ = d;
    return true;
}

// 변경 후
bool Snake::setNextDir(const int d) {
    if (d == oppositeDir(dir_)) return false;
    nextDir_ = d;
    return true;
}
```

**main.cpp:183-188 (키 입력 처리 블록)**
```cpp
// 변경 전
                    snake.setNextDir(dir);

// 변경 후
                    if (!snake.setNextDir(dir)) { failed = true; break; }
```

**main.cpp:57**
```cpp
// 변경 전
    mvprintw(cy - 2, cx - 9, "  반대 방향 입력 → 무시됨");

// 변경 후
    mvprintw(cy - 2, cx - 9, "  반대 방향 입력 → 게임 오버");
```

**Snake.h:21 (setNextDir 선언 위 주석)**
```cpp
// 변경 전
    // 반대 방향 입력은 무시

// 변경 후
    // 반대 방향 입력 시 false 반환 → 즉시 게임 오버
```

---

## 수정 2 — 미션 기반 스테이지 클리어 조건

- [x] 구현 완료

### 수정 파일

| 파일 | 위치 | 변경 내용 |
|------|------|-----------|
| `snake-game/main.cpp` | 262-264번줄 | `food.allCollected()` → MISSIONS 4조건 검사 |

### 변경 전/후

**main.cpp:262-264**
```cpp
// 변경 전
                // ── 클리어 조건: +1~+9 전부 수집 ────────────────
                if (food.allCollected())
                    cleared = true;

// 변경 후
                // ── 클리어 조건: 스테이지 미션 4가지 모두 달성 ────
                const Mission& m = MISSIONS[stage];
                if (snake.getMaxLength()        >= m.targetLength &&
                    food.getCollectedCount()    >= m.targetGrowth &&
                    poisonCount                 >= m.targetPoison &&
                    gateCount                   >= m.targetGate)
                    cleared = true;
```

### 주의사항

- `MISSIONS` 배열은 `Common.h`에 `extern const Mission MISSIONS[4];` 선언 → `main.cpp` 포함 시 접근 가능
- `food.getCollectedCount()` 이미 217번줄에서 사용 중 → 재사용 가능
- `poisonCount`, `gateCount` 스테이지 시작 시 0으로 초기화되므로 누적값 정확

---

## 수정 3 — Score Board 미션 달성 현황 표시

- [x] 구현 완료

### 수정 파일

| 파일 | 위치 | 변경 내용 |
|------|------|-----------|
| `snake-game/Board.cpp` | drawScoreBoard (259-305) | `+: N` 행 추가, Mission 섹션 신설, Growth/Effects 행 이동 |
| `snake-game/Board.cpp` | drawActiveEffects (309-310) | row 15→19, row 16→20 |

### 새 레이아웃

```
row  1: [ Score Board ]
row  2: Stage  : N
row  3: Time   : Xs
row  4: Score  : N
row  5: Best   : N
row  6: (공백)
row  7: B: N / N
row  8: +: N           ← 신규
row  9: -: N
row 10: G: N
row 11: (공백)
row 12: [ Mission ]    ← 신규 섹션 헤더
row 13: B: 6   [v]     ← 신규 (조건 미달성이면 [ ])
row 14: +: 2   [v]     ← 신규
row 15: -: 1   [v]     ← 신규
row 16: G: 0   [v]     ← 신규
row 17: [ Growth N/9 ] ← 기존 row 11에서 이동
row 18: +1 +2 +3 +4 +5 ← 기존 row 12에서 이동
row 19: +6 +7 +8 +9    ← 기존 row 13에서 이동
row 20: [ Effects ]    ← 기존 row 15에서 이동 (drawActiveEffects)
row 21: effects string ← 기존 row 16에서 이동
```

윈도우 `winBoard_` = `newwin(MAP_SIZE+2, 30, ...)` → 유효 행 row 1~21, 정확히 맞음

### 변경 전/후

**Board.cpp drawScoreBoard 전체 교체 (259-305)**

```cpp
// 변경 전 (row 7-9, 11-13 블록)
    mvwprintw(winBoard_,  7, 2, "B: %d / %d",   curLen, maxLen);
    mvwprintw(winBoard_,  8, 2, "-: %d",         poison);
    mvwprintw(winBoard_,  9, 2, "G: %d",         gate);

    int cnt = 0;
    for (int i = 0; i < 9; i++) if (collected[i]) cnt++;
    mvwprintw(winBoard_, 11, 2, "[ Growth %d/9 ]", cnt);

    for (int row = 0; row < 2; row++) {
        wmove(winBoard_, 12 + row, 2);
        ...
    }
```

```cpp
// 변경 후 (row 7-10 + Mission row 12-16 + Growth row 17-19)
    int growthCnt = 0;
    for (int i = 0; i < 9; i++) if (collected[i]) growthCnt++;

    mvwprintw(winBoard_,  7, 2, "B: %d / %d",   curLen, maxLen);
    mvwprintw(winBoard_,  8, 2, "+: %d",         growthCnt);
    mvwprintw(winBoard_,  9, 2, "-: %d",         poison);
    mvwprintw(winBoard_, 10, 2, "G: %d",         gate);

    mvwprintw(winBoard_, 12, 2, "[ Mission ]");
    const Mission& m = MISSIONS[stage];
    mvwprintw(winBoard_, 13, 2, "B: %-3d  [%c]", m.targetLength,
              (maxLen    >= m.targetLength) ? 'v' : ' ');
    mvwprintw(winBoard_, 14, 2, "+: %-3d  [%c]", m.targetGrowth,
              (growthCnt >= m.targetGrowth) ? 'v' : ' ');
    mvwprintw(winBoard_, 15, 2, "-: %-3d  [%c]", m.targetPoison,
              (poison    >= m.targetPoison) ? 'v' : ' ');
    mvwprintw(winBoard_, 16, 2, "G: %-3d  [%c]", m.targetGate,
              (gate      >= m.targetGate)   ? 'v' : ' ');

    mvwprintw(winBoard_, 17, 2, "[ Growth %d/9 ]", growthCnt);
    for (int row = 0; row < 2; row++) {
        wmove(winBoard_, 18 + row, 2);
        const int start = row * 5;
        const int end   = (row == 0) ? 5 : 9;
        for (int i = start; i < end; i++) {
            if (collected[i]) {
                wattron(winBoard_, COLOR_PAIR(6) | A_BOLD);
                wprintw(winBoard_, "+%d ", i + 1);
                wattroff(winBoard_, COLOR_PAIR(6) | A_BOLD);
            } else {
                wattron(winBoard_, COLOR_PAIR(7));
                wprintw(winBoard_, "+%d ", i + 1);
                wattroff(winBoard_, COLOR_PAIR(7));
            }
        }
    }
```

**Board.cpp drawActiveEffects (309-310)**
```cpp
// 변경 전
    mvwprintw(winBoard_, 15, 2, "[ Effects ]");
    mvwprintw(winBoard_, 16, 2, "%-26s", effects.c_str());

// 변경 후
    mvwprintw(winBoard_, 20, 2, "[ Effects ]");
    mvwprintw(winBoard_, 21, 2, "%-26s", effects.c_str());
```

### 파라미터 변경 없음

`drawScoreBoard` 시그니처 그대로 유지. `stage`에서 `MISSIONS[stage]` 접근, `growthCnt`는 내부에서 `collected[9]` 순회로 계산.

---

## 수정 4 — Growth + Poison 합산 3개 제한

- [x] 구현 완료

### 수정 파일

| 파일 | 위치 | 변경 내용 |
|------|------|-----------|
| `snake-game/main.cpp` | 239-244번줄 | 합산 체크 추가 |

### 변경 전/후

**main.cpp:239-244**
```cpp
// 변경 전
                // ── 일반 아이템 출현 (15틱마다) ──────────────────
                itemTick++;
                if (itemTick >= 15) {
                    food.spawn(board);
                    poison.spawn(board);
                    itemTick = 0;
                }

// 변경 후
                // ── 일반 아이템 출현 (15틱마다) ──────────────────
                itemTick++;
                if (itemTick >= 15) {
                    if (food.getCount() + poison.getCount() < ITEM_LIMIT)
                        food.spawn(board);
                    if (food.getCount() + poison.getCount() < ITEM_LIMIT)
                        poison.spawn(board);
                    itemTick = 0;
                }
```

### 동작 원리

- 첫 번째 체크: Food 스폰 전 합산 < 3이면 Food 스폰 시도
- 두 번째 체크: Food 스폰 후 갱신된 합산 < 3이면 Poison 스폰 시도
- Food/Poison 내부의 `ITEM_LIMIT` 체크는 방어 코드로 유지 (변경 없음)
- 결과: 맵 위 Growth + Poison 합계가 항상 3 이하로 유지됨

---

## 수정 5 — Makefile macOS 링크 보완

- [x] 구현 완료

### 수정 파일

| 파일 | 위치 | 변경 내용 |
|------|------|-----------|
| `snake-game/Makefile` | 11-13번줄 Darwin 블록 | brew 경로 변수화, fallback 추가 |

### 변경 전/후

**Makefile:10-16**
```makefile
# 변경 전
ifeq ($(shell uname), Darwin)
    CXXFLAGS += -I$(shell brew --prefix ncurses)/include
    LDFLAGS   = -L$(shell brew --prefix ncurses)/lib -lncursesw
else
    LDFLAGS   = -lncursesw
endif

# 변경 후
ifeq ($(shell uname), Darwin)
    BREW_NCURSES := $(shell brew --prefix ncurses 2>/dev/null)
    ifneq ($(BREW_NCURSES),)
        CXXFLAGS += -I$(BREW_NCURSES)/include
        LDFLAGS   = -L$(BREW_NCURSES)/lib -lncursesw
    else
        LDFLAGS   = -lncurses
    endif
else
    LDFLAGS   = -lncursesw
endif
```

### 동작 매트릭스

| 환경 | brew ncurses | 결과 |
|------|-------------|------|
| macOS (brew ncurses 설치) | 경로 반환 | `-I.../include -L.../lib -lncursesw` |
| macOS (brew ncurses 미설치) | 빈 문자열 | `-lncurses` (macOS 기본) |
| Linux / Docker | uname≠Darwin | else 브랜치 → `-lncursesw` |

---

## README.md 업데이트 (구현 후)

수정 1, 2, 4 완료 후 해당 섹션 갱신:
- "반대 방향 입력 → 무시됨" 문구 수정
- Stage Clear 조건 설명 (미션 4가지 달성)
- 아이템 제한 설명 (Growth+Poison 합산 3개)
