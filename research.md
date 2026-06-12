# 현황 분석 (Research) — 과제 불일치 5개 항목

기준 코드: snake-game/ 내 현재 파일

---

## 수정 1 — 반대 방향 입력 시 Game Over

### 현재 동작 위치

**Snake.cpp:40-45**
```cpp
// 현재: 반대 방향이면 무시하고 true 반환
bool Snake::setNextDir(const int d) {
    if (d == oppositeDir(dir_)) return true;  // ← 무시
    nextDir_ = d;
    return true;
}
```

**main.cpp:187**
```cpp
snake.setNextDir(dir);  // 반환값 버림
```

**Snake.h:21** — 주석: "반대 방향 입력은 무시"

**main.cpp:57** — 시작 화면: `"  반대 방향 입력 → 무시됨"`

**README.md:113** — "반대 방향 입력은 무시됩니다"

### 필요한 변경

`setNextDir()`이 이미 `bool` 반환형이므로:
- `Snake.cpp:42` → `return true;` → `return false;` (반대 방향 시)
- `main.cpp:187` → 반환값 체크하여 `failed = true`
- `Snake.h:21`, `main.cpp:57`, `README.md:113` 문구 수정

---

## 수정 2 — 미션 기반 스테이지 클리어 조건

### 현재 클리어 판정 위치

**main.cpp:262-264**
```cpp
// 클리어 조건: +1~+9 전부 수집
if (food.allCollected())
    cleared = true;
```

### MISSIONS 정의 (Board.cpp:11-16)

```cpp
const Mission MISSIONS[4] = {
    {  6,  2, 1, 0 },  // Stage 1: B≥6, +≥2, -≥1, G≥0
    {  7,  3, 1, 1 },  // Stage 2: B≥7, +≥3, -≥1, G≥1
    {  8,  4, 2, 2 },  // Stage 3: B≥8, +≥4, -≥2, G≥2
    { 10,  5, 2, 3 },  // Stage 4: B≥10, +≥5, -≥2, G≥3
};
```

→ 정의만 있고 **main.cpp에서 전혀 참조하지 않음**

### 판정에 필요한 현재 변수 (main.cpp 이미 존재)

| 미션 조건 | 사용할 변수 | 의미 |
|---|---|---|
| B: 목표 길이 | `snake.getMaxLength()` | 이력 최대 길이 (줄어도 유지) |
| +: Growth 획득 수 | `food.getCollectedCount()` | 수집한 번호 수(최대 9) |
| -: Poison 획득 수 | `poisonCount` | 이번 스테이지 누적 |
| G: Gate 사용 수 | `gateCount` | 이번 스테이지 누적 |

→ 4가지 변수 모두 이미 존재. 판정식만 교체하면 됨.

### 충돌 검토

- `food.allCollected()` 제거 후에도 `Food::allCollected()` 함수 자체는 Food.cpp에 남음
- +1~+9 Growth 번호 UI(`collected[]` 기반)는 drawScoreBoard 파라미터로 독립적으로 전달 → 클리어 조건과 무관하게 유지됨
- `food.getCollectedCount()`는 기존에 점수 계산(main.cpp:217)에서 이미 사용 중 → 미션 판정에도 재사용 가능
- Stage 1 미션 `targetGate=0`은 항상 자동 달성(0 이상 = 언제나 true)

---

## 수정 3 — Score Board 미션 달성 현황 표시

### 현재 drawScoreBoard 레이아웃 (Board.cpp:259-312)

```
row  1: [ Score Board ]
row  2: Stage  : N
row  3: Time   : Xs
row  4: Score  : N     (COLOR_PAIR(4)|A_BOLD, yellow)
row  5: Best   : N     (COLOR_PAIR(9), cyan)
row  7: B: N / N       (curLen / maxLen)
row  8: -: N           (poison)
row  9: G: N           (gate)
row 11: [ Growth N/9 ]
row 12: +1 +2 +3 +4 +5
row 13: +6 +7 +8 +9
```
drawActiveEffects()가 row 15-16에 추가 기록

**누락: `+: N` (Growth 획득 수) 없음, Mission 섹션 없음**

### 윈도우 공간 분석

`winBoard_ = newwin(MAP_SIZE+2, 30, ...)` → 내부 유효 행: row 1~21 (21행)  
현재 최대 row 16(effectsstring) 사용 → 5행 여유 있음

### 파라미터 분석

현재 시그니처:
```cpp
void Board::drawScoreBoard(int stage, int elapsedSec,
                           int curLen, int maxLen,
                           const bool collected[9],
                           int poison, int gate,
                           int score, int highScore)
```

미션 달성 판정에 필요한 모든 정보:
- `stage` → `MISSIONS[stage]` 접근 가능 (Board.cpp에 MISSIONS 이미 정의됨)
- `maxLen` → B 조건
- `collected[9]` → count로 + 조건 계산 가능 (`for(int i=0;i<9;i++) if(collected[i]) cnt++;`)
- `poison` → - 조건
- `gate` → G 조건

→ **시그니처 변경 불필요**, 내부에서 `MISSIONS[stage]`와 비교

### 새 레이아웃 (row 20까지 사용)

```
row  1: [ Score Board ]
row  2: Stage  : N
row  3: Time   : Xs
row  4: Score  : N     (yellow)
row  5: Best   : N     (cyan)
row  6: (빈)
row  7: B: N / N
row  8: +: N           ← 신규
row  9: -: N
row 10: G: N
row 11: [ Mission ]    ← 신규 섹션
row 12: B: 6    [v] or [ ]   ← 신규
row 13: +: 2    [v] or [ ]   ← 신규
row 14: -: 1    [v] or [ ]   ← 신규
row 15: G: 0    [v] or [ ]   ← 신규
row 16: [ Growth N/9 ] ← row 11에서 이동
row 17: +1 +2 +3 +4 +5 ← row 12에서 이동
row 18: +6 +7 +8 +9    ← row 13에서 이동
row 19: [ Effects ]    ← row 15에서 이동
row 20: effects string ← row 16에서 이동
```

→ drawActiveEffects()의 row 번호도 15→19, 16→20으로 수정 필요

---

## 수정 4 — Growth + Poison 합산 3개 제한

### 현재 각자 독립 제한

**Food.cpp:14**
```cpp
void Food::spawn(Board& board) {
    if ((int)items_.size() >= ITEM_LIMIT) return;  // Food 자체 3개 제한
    ...
}
```

**Item.cpp:11** (Poison이 사용하는 spawnItem)
```cpp
void ItemBase::spawnItem(..., int maxCount, ...) {
    if ((int)items_.size() >= maxCount) return;  // 각자 3개 제한
    ...
}
```

**main.cpp:239-244**
```cpp
if (itemTick >= 15) {
    food.spawn(board);
    poison.spawn(board);
    itemTick = 0;
}
```

→ 현재 최대 Food 3개 + Poison 3개 = **동시 6개 가능**

### 구현 방식 비교

**방법 A (main.cpp 외부 체크):**  
main.cpp 스폰 호출 전에 합산 조건 체크. Food/Poison 내부 코드 변경 없음.

```cpp
if (itemTick >= 15) {
    if (food.getCount() + poison.getCount() < ITEM_LIMIT)
        food.spawn(board);
    if (food.getCount() + poison.getCount() < ITEM_LIMIT)
        poison.spawn(board);
    itemTick = 0;
}
```

- 장점: 변경 최소, Food/Poison 내부 ITEM_LIMIT 체크는 방어 코드로 유지
- 단점: Food 스폰 후 카운트가 올라가므로 두 번째 `food.getCount()`가 갱신된 값 반환 → 자연스러운 흐름

**방법 B (spawn() 파라미터 추가):**  
`food.spawn(board, externalCount)` 식으로 외부 카운터 전달. 클래스 인터페이스 변경 수반.

→ 방법 A 채택. 변경 최소화.

---

## 수정 5 — Makefile macOS 링크 보완

### 현재 Darwin 브랜치 (Makefile:11-13)

```makefile
ifeq ($(shell uname), Darwin)
    CXXFLAGS += -I$(shell brew --prefix ncurses)/include
    LDFLAGS   = -L$(shell brew --prefix ncurses)/lib -lncursesw
```

### 문제

`brew --prefix ncurses`가 실패(ncurses 미설치)하면 빈 문자열 반환:
- `-I/include` → 헤더 못 찾음
- `-L/lib -lncursesw` → 라이브러리 못 찾음

현재 환경 확인:
```
$ brew --prefix ncurses 2>/dev/null
/opt/homebrew/opt/ncurses
$ ls /opt/homebrew/opt/ncurses/lib/libncursesw*   ← 존재함 (문제 없음)
```

→ 현재 환경에서는 동작하지만, ncurses 미설치 환경 또는 경로가 다른 환경에서 실패.

### 수정 방향

`brew --prefix ncurses 2>/dev/null` 결과가 비어 있으면 macOS 기본 ncurses(-lncurses) 사용:

```makefile
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

Docker: `uname` = Linux → else 브랜치(`-lncursesw`) → 변경 없음
