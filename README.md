# Snake Game

국민대학교 소프트웨어융합대학 C++ 프로젝트  
NCurses 라이브러리를 활용한 터미널 기반 Snake Game  
OOP 멀티파일 클래스 구조로 완전 재작성, 특수 아이템 6종·점수 시스템·Docker 환경 포함

---

## 목차

1. [프로젝트 개요](#1-프로젝트-개요)
2. [게임 화면](#2-게임-화면)
3. [아키텍처 — 클래스 구조](#3-아키텍처--클래스-구조)
4. [맵 구성 요소](#4-맵-구성-요소)
5. [조작법](#5-조작법)
6. [게임 규칙](#6-게임-규칙)
7. [아이템 시스템](#7-아이템-시스템)
8. [특수 아이템 상세](#8-특수-아이템-상세-가산점)
9. [점수 체계](#9-점수-체계)
10. [스테이지별 상세](#10-스테이지별-상세)
11. [게임 오버 및 재시작](#11-게임-오버-및-재시작)
12. [빌드 및 실행](#12-빌드-및-실행)
13. [Docker로 실행](#13-docker로-실행)
14. [파일 구조](#14-파일-구조)

---

## 1. 프로젝트 개요

| 항목 | 내용 |
|---|---|
| 언어 | C++ (C++11 표준) |
| 라이브러리 | NCurses (`libncursesw` — 유니코드 지원) |
| 빌드 환경 | Linux (Ubuntu 22.04) / macOS (Apple Silicon, Homebrew) |
| 실행 환경 | 터미널 (tty 필요) |
| 맵 크기 | 21 × 21 셀 |
| 스테이지 수 | 4단계 (Stage 1 → 4) |
| 아키텍처 | OOP 멀티파일 클래스 분리, 전역 변수 없음 |
| 가산점 항목 | 특수 아이템 6종, 최고 점수 파일 저장, Docker 환경 |

이 프로그램은 전통적인 단일 파일 Snake 구현을 **완전히 객체지향으로 재설계**한 과제 결과물입니다. 각 개념(맵, 뱀, 아이템, 게이트, 특수효과)을 독립된 클래스로 분리하고, `ItemBase` 추상 클래스를 통해 공통 아이템 로직을 상속 구조로 통합했습니다.

---

## 2. 게임 화면

```
+------------------------------------------+  ┌──────────────────────────────┐
|##  ##  ##  ##  ##  ##  ##  ##  ##  ##  ##|  │ [ Score Board ]              │
|##                                      ##|  │ Stage  : 2                   │
|##                                      ##|  │ Time   : 47s                 │
|##                                      ##|  │ Score  : 185                 │
|##      +3          >>                  ##|  │ Best   : 320                 │
|##                                      ##|  │                              │
|##  GG              +7                  ##|  │ B: 7 / 8                     │
|##                                      ##|  │ +: 3                         │
|##         HHOOOOOOO                    ##|  │ -: 1                         │
|##                          --          ##|  │ G: 2                         │
|##                                      ##|  │                              │
|##                  +2                  ##|  │ [ Growth 3/9 ]               │
|##  ##  ##  ##  ##  ##  ##  ##  ##  ##  ##|  │ +1 +2 +3 +4 +5               │
                                             │ +6 +7 +8 +9                  │
                                             │ [ Effects ]                  │
                                             │ SPEED(38) SHIELD             │
                                             └──────────────────────────────┘
```

- **좌측**: 21×21 게임 맵 (각 셀 2문자 너비)
- **우측**: 실시간 점수판
  - 스테이지·시간·점수·최고점수
  - 현재 뱀 상태 (B: 현재/최대 길이, +: Growth 수집 수, -: Poison 접촉 수, G: Gate 통과 수)
  - `[ Growth N/9 ]` — +1~+9 수집 현황 (초록=수집, 빨강=미수집)
  - `[ Effects ]` — 현재 활성 특수 효과

---

## 3. 아키텍처 — 클래스 구조

```
Common.h              ← 전체 공유 상수·방향·셀값·Point/Item 구조체
│
├── Board             ← 21×21 맵 배열 + label 배열 관리, ncurses 2윈도우 렌더링
│
├── ItemBase          ← 아이템 추상 기반 클래스
│   │                    순수 가상: spawn() / update() / consume() / getCount()
│   │                    protected: spawnItem() / updateItems() 공통 헬퍼
│   ├── Food          ← Growth Item (+1~+9) 스폰·수명·소비·수집 추적
│   └── Poison        ← Poison Item 스폰·수명·소비
│
├── Gate              ← Wall 위치 게이트 쌍 생성·수명·진출 방향 우선순위 계산
├── Special           ← 특수 아이템 6종 통합 (맵 아이템 + 활성 효과 + 소비 카운터)
└── Snake             ← 몸통 벡터·방향 버퍼·1틱 이동·충돌·Gate 통과·Reverse 처리
```

### 설계 원칙

| 원칙 | 구현 방식 |
|---|---|
| **전역 변수 없음** | 모든 게임 상태를 클래스 멤버로 캡슐화 |
| **추상 클래스 상속** | `ItemBase`가 공통 스폰/갱신 헬퍼를 `protected`로 제공, `Food`·`Poison`이 구체 구현 |
| **const 적극 사용** | 불변 getter, 배열 파라미터, 지역 변수에 `const` 명시하여 수정 범위 제한 |
| **label 배열 분리** | `Board`의 `char label_[21][21]`이 셀 값과 독립적으로 표시 문자(+1~+9 번호)를 보관 |
| **소비 카운터 패턴** | `Special::takeConsumed()`가 틱당 소비 횟수를 반환 후 즉시 초기화 → `main.cpp`에서 점수 계산에 활용 |
| **delta 기반 점수** | 매 틱 이동 전후의 카운터 차이로 점수를 계산하여 이중 집계 방지 |

### 클래스 간 의존 관계

```
main.cpp
  └─► Board, Snake, Food, Poison, Gate, Special
        Snake::move() ──► Board, Food, Poison, Gate, Special (참조 수신)
        Food::spawn() ──► Board (setCell, setLabel)
        Gate::spawn() ──► Board (getBase, setCell)
```

`Snake::move()`는 이동·충돌·아이템 소비를 단일 함수에서 처리하며, 필요한 객체를 모두 참조로 수신합니다.

---

## 4. 맵 구성 요소

| 화면 표시 | 배열 값 | 색상 | 설명 |
|---|---|---|---|
| `  ` (공백) | 0 | — | 빈 공간 — 이동 가능 |
| `##` | 1 | 흰색 배경 파랑 글자 | **Wall** — Gate로 변환 가능, 충돌 시 게임 오버 |
| `##` | 2 | 흰색 배경 흰색 글자 | **Immune Wall** — Gate 불가, Shield 무효, 항상 게임 오버 |
| `HH` | 3 | 노랑 배경 | **Snake Head** — 뱀의 머리 |
| `OO` | 4 | 초록 배경 | **Snake Body** — 뱀의 몸통 |
| `+N` | 5 | 초록 글자 | **Growth Item** — 번호 N(1~9) 표시, 수집 시 길이 +1 |
| `--` | 6 | 빨강 글자 | **Poison Item** — 수집 시 즉시 게임 오버 |
| `GG` | 7 | 마젠타 배경 | **Gate** — 순간이동 통로 |
| `>>` | 8 | 청록 글자 | **Speed Item** — 이동 속도 증가 |
| `<<` | 9 | 흰색 배경 청록 글자 | **Slow Item** — 이동 속도 감소 |
| `**` | 10 | 노랑 글자 | **Shield Item** — 충돌 1회 방어 |
| `@@` | 11 | 마젠타 글자 | **Ghost Item** — 자체 몸통 통과 |
| `%%` | 12 | 빨강 배경 노랑 글자 | **Mirror Item** — 좌우 방향키 반전 |
| `&&` | 13 | 마젠타 굵게 | **Reverse Item** — 몸 즉시 뒤집기 |

### 테두리 구분 규칙

맵 테두리는 두 종류의 Wall로 구성됩니다.

```
[2][2][2][2][2][2][2][2][2][1][1][1][2][2][2][2][2][2][2][2][2]  ← 상단 row 0
[2]                                                          [2]
...          (내부: 스테이지에 따라 Wall/빈공간)              ...
[2]                                                          [2]
[2][2][2][2][2][2][2][2][2][1][1][1][2][2][2][2][2][2][2][2][2]  ← 하단 row 20
 ↑                           ↑↑↑                           ↑
Immune                   Gate 가능 구간(col 9-11)        Immune
```

- **4 꼭짓점 및 대부분의 모서리 셀** → `Immune Wall (2)` : Gate 생성 불가, Shield 무효
- **상하 col 9–11, 좌우 row 9–11 (각 3칸)** → `Wall (1)` : Gate 생성 가능
- 내부 장애물(스테이지별)도 `Wall (1)` → Gate 생성 가능

---

## 5. 조작법

| 키 | 동작 |
|---|---|
| `↑` / `W` | 위로 이동 |
| `↓` / `S` | 아래로 이동 |
| `←` / `A` | 왼쪽으로 이동 (Mirror 활성 시 오른쪽으로 처리됨) |
| `→` / `D` | 오른쪽으로 이동 (Mirror 활성 시 왼쪽으로 처리됨) |
| `Space` | 시작 화면에서 게임 시작 |
| `Q` | 게임 중 즉시 종료 |
| `R` | 게임 오버 후 처음 Stage 1부터 재시작 (점수 초기화) |
| `C` | 게임 오버 후 현재 스테이지부터 재시작 (이전 스테이지 점수 유지) |

> **반대 방향 입력은 즉시 게임 오버입니다.** 예를 들어 오른쪽으로 이동 중에 왼쪽 키를 누르면 그 즉시 게임 오버가 발생합니다.

---

## 6. 게임 규칙

### 6-1. 기본 이동

뱀은 매 **틱(기본 200ms)** 마다 현재 방향으로 한 칸 이동합니다.

- 뱀의 머리(`HH`)가 이동 방향으로 한 칸 전진합니다.
- 성장하지 않은 경우, 꼬리 마지막 칸이 제거됩니다 (길이 유지).
- Growth Item을 먹은 경우, 꼬리가 제거되지 않아 길이가 1 증가합니다.
- Poison Item을 먹은 경우, 즉시 게임 오버가 발생합니다.

### 6-2. 방향 버퍼

`Snake::setNextDir()`에 저장된 방향은 **다음 틱 이동 시작 시** 적용됩니다. 현재 방향의 반대 방향이 입력되면 즉시 게임 오버가 됩니다.

```
현재 진행 방향: RIGHT
  ← 입력  → 즉시 게임 오버 (반대 방향)
  ↑ 입력  → nextDir_ = UP (다음 틱에 적용)
  → 입력  → nextDir_ = RIGHT (동일 방향, 변화 없음)
```

### 6-3. 충돌 판정

| 충돌 대상 | 기본 결과 | Shield 보유 시 | Ghost 활성 시 |
|---|---|---|---|
| Immune Wall (2) | 즉시 게임 오버 | **무효** (게임 오버) | 게임 오버 |
| Wall (1) | 게임 오버 | 충돌 흡수, 이동 취소 | 게임 오버 |
| 자기 몸통 | 게임 오버 | 충돌 흡수, 이동 취소 | **통과** (무시) |
| Poison Item | 즉시 게임 오버 | — | — |
| 맵 경계 밖 | 즉시 게임 오버 | — | — |

- Shield가 충돌을 흡수하면 **해당 틱의 이동 자체가 취소**됩니다. 뱀은 제자리에 머물고 Shield 효과가 소멸합니다.
- Immune Wall은 Shield·Ghost 등 모든 방어 효과를 무효화합니다.
- 꼬리 마지막 1칸은 자기 몸통 충돌 판정에서 제외됩니다 (이번 틱에 제거 예정이므로).

### 6-4. 스테이지 클리어 조건

**+1~+9 Growth Item 9개를 모두 수집**하면 스테이지 클리어가 발생합니다.

점수판 우측의 `[ Growth N/9 ]` 섹션에서 수집 현황을 실시간으로 확인할 수 있습니다 (초록=수집 완료, 빨강=미수집).

---

## 7. 아이템 시스템

### 7-1. Growth Item (`+N`)

```
스폰 주기 : 15틱마다 1개 시도
수명       : 300틱 (약 60초 @ 200ms/틱)
효과       : 뱀 길이 +1, 해당 번호를 수집 완료로 표시
번호 범위  : +1 ~ +9
동시 존재  : Growth + Poison 합산 최대 3개
```

**번호 할당 방식:**  
`Food::spawn()`은 단순 1→9 순환이 아니라, *미수집이면서 현재 맵에 없는* 번호 중 `nextNum_`에서 가장 가까운 것을 선택합니다. 이 방식으로 모든 번호가 맵에 오를 기회를 보장합니다.

```
예시: +3, +5가 맵에 있고, +1·+2·+7이 이미 수집된 상태
  nextNum_ = 4 → 4는 맵에 없고 미수집 → +4 스폰
  nextNum_ = 5 → 5는 이미 맵에 있음
  nextNum_ = 6 → 6은 맵에 없고 미수집 → +6 스폰 (다음 차례)
```

`Board`의 `label_[y][x]` 배열에 숫자 문자('1'~'9')를 저장하여, 셀 값(`CELL_GROWTH = 5`)과 분리된 표시 문자를 관리합니다.

### 7-2. Poison Item (`--`)

```
스폰 주기 : 15틱마다 1개 시도
수명       : 300틱
효과       : 즉시 게임 오버
동시 존재  : Growth + Poison 합산 최대 3개
```

**합산 제한 동작 원리:**

```
스폰 시도 시 (Growth 개수 + Poison 개수) < 3 인 경우에만 스폰 허용
예: Growth 2개 + Poison 0개 → 합산 2 < 3 → Poison 스폰 가능
    Growth 2개 + Poison 1개 → 합산 3 = 3 → 추가 스폰 불가
```

### 7-3. Gate (`GG`)

```
스폰 방식 : 게임 시작 시 및 이전 Gate 소멸 후 즉시 Wall(1) 위치 2곳에 쌍으로 생성
수명       : 200틱
효과       : 한쪽 게이트 진입 → 반대쪽 게이트로 순간이동
```

**진출 방향 우선순위:**  
Gate에서 나올 때는 다음 순서로 진출 가능한 방향을 검색합니다.

```
1순위: 진입 방향 그대로 (직진)
2순위: 시계 방향 90°
3순위: 반시계 방향 90°
4순위: 반대 방향 (U턴)
```

진출 위치가 Wall·Immune Wall·다른 Gate라면 다음 우선순위로 넘어갑니다. 모든 방향이 막혀 있으면 반대 방향을 반환하며, 이후 Snake가 벽으로 처리합니다.

**Gate 진입 예시:**

```
             ↑ (진입)
  [WALL]  [GATE A]  [WALL]
  
  [WALL]  [GATE B]  [WALL]
             ↓
  → 출구: 아래(직진) 시도 → 벽이므로 왼쪽(시계90°) 시도 → 성공
```

---

## 8. 특수 아이템 상세 (가산점)

모든 특수 아이템의 공통 속성:

```
수명(맵)      : 150틱 (아이템이 먹히지 않아도 자동 소멸)
스폰 주기     : 20틱마다 각 종류 스폰 시도
동시 존재     : 종류별 1개 (같은 종류 중복 스폰 없음)
```

| 아이템 | 화면 | 효과 | 지속 시간 | 특이사항 |
|---|---|---|---|---|
| **Speed** | `>>` 청록 | 이동 틱 200ms → 100ms | 50틱 | Slow 활성 중엔 스폰 안 됨 |
| **Slow** | `<<` 흰/청록 | 이동 틱 200ms → 400ms | 50틱 | Speed 활성 중엔 스폰 안 됨 |
| **Shield** | `**` 노랑 | 벽 또는 자체충돌 1회 흡수 후 소멸 | 충돌 전까지 무제한 | Immune Wall에는 무효 |
| **Ghost** | `@@` 마젠타 | 자기 몸통 충돌 판정 완전 무시 | 50틱 | 벽 충돌엔 무효 |
| **Mirror** | `%%` 빨강/노랑 | 좌우 방향키 반전 (← → 가 뒤바뀜) | 50틱 | WASD도 동일하게 반전 |
| **Reverse** | `&&` 마젠타 굵게 | 몸통 즉시 역전, 진행 방향 반전 | 즉시 1회 | 먹는 순간 적용, 꼬리가 새 머리가 됨 |

**Reverse 동작 상세:**  
`std::reverse(body_)`로 몸통 벡터를 뒤집고, 방향을 반전시킵니다.

```
Before: H O O O O (→ 방향)
After:  O O O O H (← 방향)  ← 꼬리였던 부분이 새 머리
```

**Speed/Slow 상호 배제:**  
Speed가 활성 효과로 남아 있으면 Slow가 맵에 스폰되지 않고, 반대도 마찬가지입니다.

---

## 9. 점수 체계

### 9-1. 점수 부여 기준

| 행동 | 점수 |
|---|---|
| Growth Item (+N) 수집 | **+10점** |
| Gate 통과 | **+15점** |
| 특수 아이템 소비 (Speed·Slow·Shield·Ghost·Mirror·Reverse) | **+5점** |
| 스테이지 클리어 보너스 | **+200점** |

### 9-2. 점수 집계 방식

점수는 매 틱마다 **delta(차이)** 방식으로 계산됩니다. 이동 전 카운터 값을 저장하고, `snake.move()` 실행 후 차이를 구해 즉시 `currentScore`에 반영합니다.

```cpp
// 매 틱 계산 (main.cpp 내부)
currentScore += (food.getCollectedCount() - prevCollected) * 10;  // Growth
currentScore += (gateCount - prevGate) * 15;                       // Gate
currentScore += special.takeConsumed() * 5;                        // Special
```

### 9-3. 점수 구조

```
totalScore = prevStagesScore + currentScore
│               │                   │
│               └ 이전 스테이지     └ 현재 스테이지 누적
│                 완료 시 누적
└ 점수판에 실시간 표시, 최고점수와 비교
```

스테이지 클리어 시:

```
currentScore += 200  (클리어 보너스)
prevStagesScore += currentScore  (이전 스테이지 점수에 편입)
currentScore = 0  (다음 스테이지용 초기화)
```

### 9-4. 최고 점수 저장

최고 점수는 `snake-game/highscore.txt` 파일에 저장됩니다. 게임 중 `totalScore`가 `highScore`를 초과하는 매 틱마다 즉시 파일에 갱신되므로, 도중에 종료되어도 최고 점수가 보존됩니다. 시작 화면에서 최고 점수를 확인할 수 있습니다.

---

## 10. 스테이지별 상세

**공통 클리어 조건: +1~+9 Growth Item 9개를 모두 수집**

### Stage 1 — Border Only (테두리만)

```
██  ##  ##  ##  ##  ##  ##  ##  ##  ██  ██  ██  ##  ##  ##  ##  ##  ##  ##  ##  ██
██                                                                              ██
██                                                                              ██
██                                                                              ██
██                                                                              ██
██                                                                              ██
██                                                                              ██
██                                                                              ██
██                                                                              ██
##                          (내부 완전 개방)                                     ##
##                                                                              ##
##                                                                              ##
██                                                                              ██
██                                                                              ██
██                                                                              ██
██                                                                              ██
██                                                                              ██
██                                                                              ██
██                                                                              ██
██                                                                              ██
██  ##  ##  ##  ##  ##  ##  ##  ##  ██  ██  ██  ##  ##  ##  ##  ##  ##  ##  ##  ██

  ██ = Immune Wall(2),  ## = Wall(1, Gate 가능),  빈칸 = 빈 공간
  Gate 가능 구간: 상하 col 9-11, 좌우 row 9-11 (각 3칸)
```

**특징:**
- 내부 장애물이 전혀 없는 가장 단순한 맵
- 뱀이 자유롭게 이동할 수 있는 최대 면적 (19×19 = 361칸)
- Gate가 테두리에만 생성됨

---

### Stage 2 — 단벽 2개 (가로벽 + 세로벽)

```
██  ##  ##  ##  ##  ##  ##  ##  ##  ██  ██  ██  ##  ##  ##  ##  ##  ##  ##  ##  ██
██                                                                              ██
██                                                                              ██
██                                                                              ██
██                                                                              ██
██                                                                              ██
██      ##  ##  ##      ##  ##  ##                                              ██  ← row 6 가로벽
██                                                                              ██
██                                                                              ██
##                                                                              ##
##                                                                              ##
##                                                                              ##
██                                                                              ██
██                                              ##                              ██  ─┐ col 14
██                                              ##                              ██   │ 세로벽
██                                              ##                              ██   │ row 13-17
██                                              ##                              ██   │
██                                              ##                              ██  ─┘
██                                                                              ██
██                                                                              ██
██  ##  ##  ##  ##  ##  ##  ##  ##  ██  ██  ██  ##  ##  ##  ##  ##  ##  ##  ##  ██
```

**특징:**
- **가로벽:** row 6, col 2-4 (3칸) + col 6-8 (3칸) — col 5에 틈이 있어 상하 이동 가능
- **세로벽:** col 14, row 13-17 (5칸) — 맵 오른쪽 하단을 좌우로 분리
- 내부 Wall에도 Gate 생성 가능 → 예상치 못한 순간이동 발생 가능

---

### Stage 3 — 4구석 3×3 기둥

```
██  ##  ##  ##  ##  ##  ##  ##  ##  ██  ██  ██  ##  ##  ##  ##  ##  ##  ##  ##  ██
██                                                                              ██
██                                                                              ██
██          ##  ##  ##                              ##  ##  ##                  ██  ─┐ row 3-5
██          ##  ##  ##                              ##  ##  ##                  ██   │ 3×3 기둥
██          ##  ##  ##                              ##  ##  ##                  ██  ─┘
██                                                                              ██
██                                                                              ██
██                                                                              ██
##                                                                              ##
##                                                                              ##
##                                                                              ##
██                                                                              ██
██                                                                              ██
██                                                                              ██
██          ##  ##  ##                              ##  ##  ##                  ██  ─┐ row 15-17
██          ##  ##  ##                              ##  ##  ##                  ██   │ 3×3 기둥
██          ##  ##  ##                              ##  ##  ##                  ██  ─┘
██                                                                              ██
██                                                                              ██
██  ##  ##  ##  ##  ##  ##  ##  ##  ██  ██  ██  ##  ##  ##  ##  ##  ##  ##  ##  ██

  4구석: (row 3-5, col 3-5) / (row 3-5, col 15-17) / (row 15-17, col 3-5) / (row 15-17, col 15-17)
```

**특징:**
- 각 구석에 3×3 Wall 기둥 → 맵을 5개 구역(중앙, 상좌, 상우, 하좌, 하우)으로 분할
- 기둥 사이 통로 폭 2~4칸 → 긴 뱀은 방향 전환에 제약
- 기둥 Wall에도 Gate 생성 가능

---

### Stage 4 — 십자형(+) 벽

```
██  ##  ##  ##  ##  ##  ##  ##  ##  ██  ██  ██  ##  ##  ##  ##  ##  ##  ##  ##  ██
██                                                                              ██
██                                                                              ██
██                                      ##                                      ██  ─┐
██                                      ##                                      ██   │ col 10
██                                      ##                                      ██   │ 세로벽
██                                      ##                                      ██   │ row 3-7
██                                      ##                                      ██  ─┘
██                                                                              ██
##                                                                              ##
##      ##  ##  ##  ##  ##                  ##  ##  ##  ##  ##                  ##  ← row 10 가로벽
##                                                                              ##
██                                                                              ██
██                                      ##                                      ██  ─┐
██                                      ##                                      ██   │ col 10
██                                      ##                                      ██   │ 세로벽
██                                      ##                                      ██   │ row 13-17
██                                      ##                                      ██  ─┘
██                                                                              ██
██                                                                              ██
██  ##  ##  ##  ##  ##  ##  ##  ##  ██  ██  ██  ##  ##  ##  ##  ##  ##  ##  ##  ██

  세로벽: col 10, row 3-7 / row 13-17  (중앙 row 8-12 개방)
  가로벽: row 10, col 3-7 / col 14-18  (중앙 col 8-13 개방)
```

**특징:**
- 십자형 벽이 맵을 9개 구역(3×3 격자)으로 나눔
- 각 벽의 중앙은 개방 → 구역 간 이동은 가능하지만 경로가 좁음
- 가장 복잡한 장애물 배치, Gate 생성 위치가 다양

---

## 11. 게임 오버 및 재시작

### 게임 오버 발생 조건

- Wall(1) 또는 경계 외부 충돌 (Shield 없는 경우)
- Immune Wall(2) 충돌
- 자기 몸통 충돌 (Ghost 비활성 + Shield 없는 경우)
- **Poison Item(`--`) 수집**
- **반대 방향 키 입력**

### 게임 오버 화면

게임 오버 시 현재 점수와 최고 점수를 보여주고 다음 세 가지 선택지를 제공합니다:

```
   **** GAME OVER ****

 Score : 185
 Best  : 320

 R : 처음부터 (점수 초기화)
 C : 현재 스테이지 재시작
 Q : 종료
```

| 키 | 동작 | 점수 처리 |
|---|---|---|
| `R` | Stage 1부터 완전 처음 시작 | `prevStagesScore = 0`으로 초기화, 0점부터 시작 |
| `C` | 죽은 스테이지부터 재시작 | 이전 스테이지 누적 점수 유지, 현재 스테이지 점수만 0으로 초기화 |
| `Q` | 프로그램 종료 | — |

**`C` 선택 예시:**

```
Stage 1 클리어 (300점) → Stage 2에서 게임 오버 → C 선택
  → Stage 2 재시작, 점수판: Score = 300 (Stage 1 점수)
  → Stage 2에서 얻는 점수가 300점에 추가로 누적됨
```

---

## 12. 빌드 및 실행

### macOS (Homebrew ncurses)

```bash
# 의존 패키지 설치
brew install ncurses

# 빌드
cd snake-game
make

# 실행
./snake
```

### Linux (Ubuntu / Debian 계열)

```bash
# 의존 패키지 설치
sudo apt-get install libncursesw5-dev

# 빌드
cd snake-game
make

# 실행
./snake
```

### Makefile 자동 분기

```makefile
ifeq ($(shell uname), Darwin)
    BREW_NCURSES := $(shell brew --prefix ncurses 2>/dev/null)
    ifneq ($(BREW_NCURSES),)
        CXXFLAGS += -I$(BREW_NCURSES)/include
        LDFLAGS   = -L$(BREW_NCURSES)/lib -lncursesw
    else
        LDFLAGS   = -lncurses   # Homebrew ncurses 미설치 시 macOS 기본 사용
    endif
else
    LDFLAGS   = -lncursesw      # Linux
endif
```

| 환경 | 동작 |
|---|---|
| macOS + brew ncurses 설치 | Homebrew 경로로 `libncursesw` 링크 |
| macOS + brew ncurses 미설치 | macOS 기본 `libncurses` 링크 |
| Linux / Docker | 시스템 `libncursesw` 링크 |

소스 파일 각각을 독립 `.o` 오브젝트로 컴파일하고 링크합니다. 단일 파일 수정 시 해당 파일만 재컴파일됩니다.

---

## 13. Docker로 실행 (Ubuntu 22.04 — 권장, 협업용)

ncurses 터미널 게임 특성상 `docker run`에 `-it` 옵션(TTY + stdin)이 필요합니다. `docker compose up`은 pseudo-TTY를 할당하지 않으므로 **반드시 `docker compose run`** 을 사용해야 합니다.

### 처음 빌드 후 실행

```bash
git clone https://github.com/SilVerBell0109/snake-game.git
cd snake-game
docker compose build
docker compose run --rm snake
```

### 재실행 (빌드 없이)

```bash
docker compose run --rm snake
```

### 소스 수정 후 재빌드 + 실행

```bash
docker compose build && docker compose run --rm snake
```

### docker-compose.yml 핵심 설정

```yaml
services:
  snake:
    build:
      context: .
      dockerfile: Dockerfile
      platform: linux/amd64
    stdin_open: true   # docker run -i
    tty: true          # docker run -t
```

`platform: linux/amd64`는 Apple Silicon(M1/M2/M4) Mac에서 amd64 이미지를 Rosetta로 실행하기 위해 명시합니다. `stdin_open`·`tty`는 ncurses 화면 입출력에 필수입니다.

---

## 14. 파일 구조

```
snake/
├── Dockerfile              ← Ubuntu 22.04 기반 빌드 이미지 정의
├── docker-compose.yml      ← tty/stdin 설정 포함 컴포즈 파일
└── snake-game/
    ├── Makefile            ← OS 자동 감지(Darwin/Linux), brew fallback 포함
    ├── Common.h            ← 상수·방향·셀값·Point/Item 구조체
    ├── Board.h / Board.cpp ← 맵 배열·label 배열·ncurses 2윈도우·14색상쌍 렌더링
    ├── Item.h  / Item.cpp  ← ItemBase 추상 클래스 (spawnItem/updateItems 공통 헬퍼)
    ├── Food.h  / Food.cpp  ← Growth Item: 번호 할당·수집 추적
    ├── Poison.h / Poison.cpp ← Poison Item: 스폰·수명·소비
    ├── Gate.h  / Gate.cpp  ← Gate 쌍 생성·수명·진출 방향 우선순위 계산
    ├── Special.h / Special.cpp ← 특수 아이템 6종·활성 효과·소비 카운터
    ├── Snake.h / Snake.cpp ← 몸통 벡터·방향 버퍼·이동·충돌·Gate·Reverse 처리
    ├── main.cpp            ← ncurses 초기화·게임 루프·점수 집계·스테이지 흐름 제어
    └── highscore.txt       ← 최고 점수 저장 파일 (게임 실행 중 자동 생성/갱신)
```

### 주요 상수 (Common.h)

| 상수 | 값 | 의미 |
|---|---|---|
| `MAP_SIZE` | 21 | 맵 한 변 셀 수 |
| `ITEM_LIMIT` | 3 | Growth + Poison 합산 동시 최대 개수 |
| `ITEM_LIFE` | 300 | Growth/Poison 수명 (틱) |
| `GATE_LIFE` | 200 | Gate 수명 (틱) |
| `BASE_TICK_MS` | 200 | 기본 이동 속도 (ms) |
| `SPECIAL_LIFE` | 150 | 특수 아이템 수명 (틱) |
| `SPECIAL_TICK` | 20 | 특수 아이템 스폰 간격 (틱) |
| `SPEED_TICK_MS` | 100 | Speed 적용 시 이동 속도 (ms) |
| `SLOW_TICK_MS` | 400 | Slow 적용 시 이동 속도 (ms) |
| `SPEED_DURATION` | 50 | Speed 지속 시간 (틱) |
| `SLOW_DURATION` | 50 | Slow 지속 시간 (틱) |
| `GHOST_DURATION` | 50 | Ghost 지속 시간 (틱) |
| `MIRROR_DURATION` | 50 | Mirror 지속 시간 (틱) |

---

## 참고 자료

- [GNU NCurses 공식 사이트](http://www.gnu.org/software/ncurses/)
- [NCURSES Programming HOWTO](http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/)
- [cppreference — std::vector](https://en.cppreference.com/w/cpp/container/vector)
- [cppreference — std::reverse](https://en.cppreference.com/w/cpp/algorithm/reverse)
