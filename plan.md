# Plan — Mission System 복원 (과제 원문 준수)

승인 후 구현. 체크되지 않은 항목은 미구현.

---

## 배경

커밋 `fd8db49` ~ `6b5d5e1` 사이에 Mission 시스템이 단계적으로 제거됨.
과제 원문 Game Rule #6 (B/+/-/G 미션 달성 시 스테이지 클리어) 위반.
참조 복원 커밋: `8b8ecb1`.

---

## 복원 방식 선택

### 옵션 A — `git checkout 8b8ecb1 -- <file>` 방식
4개 파일을 커밋 시점 그대로 복원. 이후 회귀한 커밋에서 추가된 특수 아이템 코드가 있을 경우 덮어씌워진다.

**확인:** 8b8ecb1 이후 커밋에서 4개 파일에 기능 추가가 있었는지 점검 필요.
```
git log 8b8ecb1..HEAD --oneline -- snake-game/Snake.cpp snake-game/main.cpp snake-game/Board.cpp snake-game/Common.h
```
→ 이 구간의 변경은 Mission 제거/Poison 즉사/클리어 단순화뿐. 특수 아이템 등 다른 기능 추가 없음.
→ 옵션 A 사용 가능하나, Common.h/Board.cpp는 완전 덮어쓰기보다 **수술적 편집이 더 안전**.

### 옵션 B — 수술적 편집 방식 (선택) ✅
각 파일에서 회귀된 hunk만 복원. 비관련 코드 건드리지 않음.
- 변경량이 적고 (4개 파일, 파일당 1~2 hunk)
- 의도치 않은 사이드 이펙트 없음

---

## 수정 파일 및 스니펫

### Step 1 — Common.h: struct Mission + extern 선언 추가

**위치:** `snake-game/Common.h`  
**삽입 위치:** `struct Item { ... };` 블록 바로 뒤

추가할 코드:
```cpp
struct Mission {
    int targetLength;
    int targetGrowth;
    int targetPoison;
    int targetGate;
};
```
그리고 파일 끝 `oppositeDir` 인라인 함수 아래:
```cpp
extern const Mission MISSIONS[4];
```

비고: `extern` 선언과 `oppositeDir` 순서는 8b8ecb1 그대로(oppositeDir 뒤에 extern).

---

### Step 2 — Board.cpp: MISSIONS 정의 + drawScoreBoard Mission 블록 복원

**위치:** `snake-game/Board.cpp`

**(a) MISSIONS[4] 정의 삽입**  
`#include <cstring>` 다음, `STAGE_MAPS` 배열 전에 삽입:
```cpp
// ── 미션 조건 (extern 정의) ───────────────────────────────────────
const Mission MISSIONS[4] = {
    {  6,  2, 1, 0 },  // Stage 1
    {  7,  3, 1, 1 },  // Stage 2
    {  8,  4, 2, 2 },  // Stage 3
    { 10,  5, 2, 3 },  // Stage 4
};
```

**(b) drawScoreBoard 내 행 번호 3곳 변경 + Mission 블록 삽입**

현재:
```cpp
    mvwprintw(winBoard_, 12, 2, "[ Growth %d/9 ]", growthCnt);

    // +1~+5: row 13,  +6~+9: row 14
    for (int row = 0; row < 2; row++) {
        wmove(winBoard_, 13 + row, 2);
```

복원 목표:
```cpp
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

    // +1~+5: row 18,  +6~+9: row 19
    for (int row = 0; row < 2; row++) {
        wmove(winBoard_, 18 + row, 2);
```

비고: `drawScoreBoard` 시그니처에 `stage` 파라미터가 이미 있으므로 추가 파라미터 불필요.

---

### Step 3 — Snake.cpp: Poison 처리 복원 (shrank 로직)

**위치:** `snake-game/Snake.cpp`, move 함수 내 아이템 소비 구역

현재 코드 (행 106–123):
```cpp
    bool grew = false;

    if (headCell == CELL_GROWTH) {
        if (food.consume(newHead.y, newHead.x, board))
            grew = true;
    } else if (headCell == CELL_POISON) {
        if (poison.consume(newHead.y, newHead.x, board))
            { poisonCount++; return false; }
    } else if (headCell >= CELL_SPEED && headCell <= CELL_REVERSE) {
        special.consume(newHead.y, newHead.x, board);
    }

    // ── 꼬리 처리 ────────────────────────────────────────────────
    if (!grew) {
        const Point& tail = body_.back();
        board.setCell(tail.y, tail.x, board.getBase(tail.y, tail.x));
        body_.pop_back();
    }
```

복원 목표:
```cpp
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
```

비고: `if (shrank)` 블록은 일반 꼬리 처리 뒤, Head 삽입 전에 위치해야 함.

---

### Step 4 — main.cpp: 클리어 조건 복원

**위치:** `snake-game/main.cpp`, 게임 루프 내 클리어 판정 구역

현재:
```cpp
                // ── 클리어 조건: +1~+9 전부 수집 ──
                if (food.allCollected())
                    cleared = true;
```

복원 목표:
```cpp
                // ── 클리어 조건: 미션 B/+/-/G 전부 달성 ──
                const Mission& m = MISSIONS[stage];
                if (snake.getMaxLength()     >= m.targetLength &&
                    food.getCollectedCount() >= m.targetGrowth &&
                    poisonCount              >= m.targetPoison &&
                    gateCount                >= m.targetGate)
                    cleared = true;
```

비고: `food.allCollected()` 기반 경로는 완전 제거.

---

### Step 5 — README.md: 복원 내용 반영

수정 필요 섹션 (코드와 일치 확인 후 어긋난 부분만):

| 섹션 | 현재 | 복원 |
|---|---|---|
| § 2 게임 화면 ASCII | Mission 없음 | `[ Mission ]` 4줄 추가 |
| § 6-4 클리어 조건 | +1~+9 전부 수집 | 미션 B/+/-/G 4조건 AND 설명 |
| § 7-2 Poison | "즉시 게임 오버" | "길이 -1 (꼬리 2칸 제거), 길이 3 미만 시 게임 오버" |
| § 9-1 점수 체계 | Poison -5 없음 | `-5점/개` 행 복원 |
| § 10 스테이지별 | 미션 목표 없음 | 각 스테이지 B/+/-/G 값 복원 |

---

### Step 6 — 빌드 검증

```bash
cd snake-game && make clean && make 2>&1 | grep -E "error:|warning:"
```
- -Wall -Wextra 경고 0개 확인
- 선택: Docker build로 Linux 환경 확인

---

### Step 7 — git 커밋 & 푸시

```
Restore mission-based stage clear and poison shrink per assignment spec
```

---

## 트레이드오프

| 항목 | 옵션 A (git checkout) | 옵션 B (수술적 편집, 채택) |
|---|---|---|
| 정확성 | 8b8ecb1 완벽 복원 | hunk 단위 수동 복원 |
| 안전성 | 비관련 변경도 덮어씀 위험 | 변경 범위 명시적 |
| 검토 용이성 | diff 전체 보여야 함 | diff가 작고 명확 |
| 채택 이유 | — | 4개 파일 모두 해당 구간 이외 변경 없음 확인됨 |

---

## 체크리스트

- [x] Step 1: Common.h — struct Mission + extern 추가
- [x] Step 2a: Board.cpp — MISSIONS[4] 정의 추가 (easier 값 적용)
- [x] Step 2b: Board.cpp — drawScoreBoard Mission 블록 + 행 번호 복원
- [x] Step 3: Snake.cpp — shrank 로직 복원
- [x] Step 4: main.cpp — 미션 기반 클리어 조건 복원
- [x] Step 5: README.md — 해당 섹션 수정
- [x] Step 6: `make` 빌드 성공 (-Wall -Wextra 경고 0)
- [ ] Step 7: git commit & push
