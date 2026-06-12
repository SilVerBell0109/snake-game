# Research — Mission System 복원 현황

작성일: 2026-06-12

---

## 1. 참조 커밋

| 커밋 | 설명 |
|---|---|
| `8b8ecb1` | 정상 동작 버전 (미션 시스템 완비) |
| `fd8db49` | Mission 패널·struct 전부 제거 (회귀 시작) |
| `b11ab4c` | Poison 즉사로 변경 |
| `9ce7fad` | 클리어 조건 `food.allCollected()`로 단순화 |
| `6b5d5e1` | HEAD — README 최신화 (과제 위반 상태 반영) |

---

## 2. 현재 상태 vs 8b8ecb1 상태 비교

### 2-1. Common.h

| | 현재 (HEAD) | 8b8ecb1 |
|---|---|---|
| `struct Mission` | **없음** | `{ targetLength, targetGrowth, targetPoison, targetGate }` |
| `extern const Mission MISSIONS[4];` | **없음** | 있음 |
| 나머지 | 동일 | — |

### 2-2. Board.cpp

| | 현재 (HEAD) | 8b8ecb1 |
|---|---|---|
| MISSIONS[4] 정의 | **없음** | `{6,2,1,0}, {7,3,1,1}, {8,4,2,2}, {10,5,2,3}` |
| drawScoreBoard 상단 | row 1–10 동일 | row 1–10 동일 |
| `[ Mission ]` 블록 | **없음** (행 12부터 Growth) | row 12–16 (Mission 4줄) |
| `[ Growth N/9 ]` 헤더 | **row 12** | row 17 |
| Growth 체크박스 | **row 13 / 14** | row 18 / 19 |
| `[ Effects ]` | row 20 / 21 | row 20 / 21 동일 |

현재 drawScoreBoard 내 변경 필요 범위: 행 번호 3개 + Mission 블록 삽입 (총 9줄 변경).

### 2-3. Snake.cpp — Poison 처리 (move 함수 내부)

**현재 (위반):**
```cpp
bool grew = false;
// shrank 없음

} else if (headCell == CELL_POISON) {
    if (poison.consume(newHead.y, newHead.x, board))
        { poisonCount++; return false; }   // 즉사
}

if (!grew) {
    const Point& tail = body_.back();
    board.setCell(tail.y, tail.x, board.getBase(tail.y, tail.x));
    body_.pop_back();
}
// shrank 블록 없음
```

**8b8ecb1 (정상):**
```cpp
bool grew   = false;
bool shrank = false;

} else if (headCell == CELL_POISON) {
    if (poison.consume(newHead.y, newHead.x, board))
        { shrank = true; poisonCount++; }   // 즉사 아님
}

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

변경 포인트 3개: `bool shrank` 추가, Poison 분기 `return false` 제거, `if (shrank)` 블록 복원.

### 2-4. main.cpp — 클리어 조건

**현재 (위반):**
```cpp
// ── 클리어 조건: +1~+9 전부 수집 ──
if (food.allCollected())
    cleared = true;
```

**8b8ecb1 (정상):**
```cpp
const Mission& m = MISSIONS[stage];
if (snake.getMaxLength()     >= m.targetLength &&
    food.getCollectedCount() >= m.targetGrowth &&
    poisonCount              >= m.targetPoison &&
    gateCount                >= m.targetGate)
    cleared = true;
```

나머지 main.cpp (점수 계산, 스테이지 흐름) 변경 없음.

---

## 3. README 현황

HEAD README는 2026-06-12 과제 위반 상태로 재작성됨. 복원 후 다음 섹션 수정 필요:

| 섹션 | 현재 (위반) | 복원 목표 |
|---|---|---|
| § 2 ASCII | `[ Mission ]` 없음 | Mission 4줄 복원 |
| § 6-4 클리어 조건 | "+1~+9 전부 수집" | 미션 B/+/-/G 4조건 AND |
| § 7-2 Poison | "즉시 게임 오버" | "길이 -1, 3 미만 시 게임 오버" |
| § 9-1 점수 체계 | Poison -5 행 없음 | -5점 행 복원 |
| § 10 스테이지별 | 미션 목표 없음 | 각 스테이지 B/+/-/G 목표 복원 |

---

## 4. D-1 분석 — B 판정 기준 (maxLen vs curLen)

8b8ecb1 기준: `snake.getMaxLength() >= m.targetLength`

Poison으로 현재 길이가 줄어도 maxLen은 감소하지 않는다. 한 번 targetLength에 도달하면 이후 유지됨.

"현재 길이" 기준으로 바꿀 경우: Poison을 마지막에 채우면 길이가 targetLength 미만으로 떨어져 클리어 취소 가능 → 혼란 유발. 변경 불필요.

→ **8b8ecb1 그대로 maxLen 기준 유지.**

---

## 5. D-2 분석 — 스테이지별 달성 가능성

시작 길이: 3. Growth +1/Poison -1(현재 길이 기준), maxLen은 최고치 보존.

| Stage | B | + | - | G | 가능 | 안전 최소 경로 |
|---|---|---|---|---|---|---|
| 1 | ≥6 | ≥2 | ≥1 | ≥0 | ✅ | Growth 3개→길이6, Poison 1개→길이5(maxLen=6) |
| 2 | ≥7 | ≥3 | ≥1 | ≥1 | ✅ | Growth 4개→길이7, Poison 1개, Gate 1회 |
| 3 | ≥8 | ≥4 | ≥2 | ≥2 | ✅ | Growth 5개→길이8, Poison 2개→길이6, Gate 2회 |
| 4 | ≥10 | ≥5 | ≥2 | ≥3 | ✅ | Growth 7개→길이10, Poison 2개→길이8, Gate 3회 |

모든 스테이지 달성 가능. Poison은 targetLength 도달 후 먹어야 안전(길이 3 미만 방지).
