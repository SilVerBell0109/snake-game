# Research: snake.cpp 현황 분석

## 1. 파일 구조

단일 파일 `snake.cpp` (799줄). 전역변수 다수, 클래스 없음.

---

## 2. 전역변수 목록 (과제 위반 — 전부 제거 대상)

| 변수 | 타입 | 역할 |
|------|------|------|
| `map[21][21]` | `int[][]` | 실시간 맵 배열 |
| `baseMap[21][21]` | `int[][]` | 원본 맵 (아이템 복원용) |
| `snake` | `vector<Point>` | 뱀 몸통 (front=Head) |
| `snakeDir` / `nextDir` | `int` | 현재/다음 방향 |
| `items` | `vector<Item>` | Growth+Poison 혼합 관리 |
| `gateA` / `gateB` | `Gate struct` | 게이트 쌍 |
| `gateTick` | `int` | 게이트 수명 카운터 |
| `score_maxLen/growth/poison/gate/time` | `int` x5 | 점수 지표 |
| `currentStage` | `int` | 현재 스테이지 (0~3) |
| `gameFailed/stageClear/gameOver` | `bool` | 게임 상태 플래그 |
| `winMap` / `winBoard` | `WINDOW*` | ncurses 윈도우 |

---

## 3. 함수별 로직 분석

### `loadMap(stage)`
- STAGE_MAPS 배열에서 baseMap, map 양쪽에 복사
- **버그**: 현재 모든 테두리가 Immune Wall(2)
- **수정 필요**: 4구석만 2, 나머지 테두리는 1(Wall)로 → Gate 출현 가능

### `initGame()`
- 뱀 (10,12)~(10,10) 오른쪽 방향으로 초기화
- 아이템·게이트·점수 전부 리셋

### `drawMap()`
- 셀값 0~7만 렌더링, 추가 아이템 케이스 없음

### `drawScoreBoard()`
- `winBoard`에 Stage/Time/B/+/-/G 출력
- Mission v/x 표시

### `spawnItem()`
- Growth/Poison 각각 최대 3개, 빈 칸 무작위 배치
- 두 타입이 단일 `vector<Item>`에 혼합 — 클래스 분리 대상

### `moveSnake()` — 핵심 변경 포인트

**반대 방향 처리 (수정 필요)**
```cpp
// 현재: 반대 방향 입력 무시하고 계속 진행
if (nextDir == getOppositeDir(snakeDir))
    nextDir = snakeDir;  // 덮어쓰고 계속

// 요구: 반대 방향 입력 → 즉시 gameFailed = true
```

충돌 판정 순서 (유지):
1. 경계 밖 → 실패
2. Immune Wall(2) → 실패
3. Wall(1) → 실패
4. 자기 몸 충돌 (tail 제외) → 실패
5. Gate(7) 진입: 우선순위 진입→CW→CCW→반대

아이템 소비:
- Growth(5): 꼬리 유지 (길이+1)
- Poison(6): 꼬리 2칸 제거, 길이<3 → 실패

Gate 진출 우선순위 — 이미 올바르게 구현됨:
```cpp
int priority[4] = {snakeDir, cw[snakeDir], ccw[snakeDir], opposite};
// CW 테이블: UP→RIGHT, DOWN→LEFT, LEFT→UP, RIGHT→DOWN
// CCW 테이블: UP→LEFT, DOWN→RIGHT, LEFT→DOWN, RIGHT→UP
```

### 조작키 (수정 필요)
```cpp
// 현재 (WASD)
if (key == 'w' || key == 'W') nextDir = UP;
if (key == 's' || key == 'S') nextDir = DOWN;
if (key == 'a' || key == 'A') nextDir = LEFT;
if (key == 'd' || key == 'D') nextDir = RIGHT;

// 요구 (방향키)
if (key == KEY_UP)    → setNextDir(UP)
if (key == KEY_DOWN)  → setNextDir(DOWN)
if (key == KEY_LEFT)  → setNextDir(LEFT)
if (key == KEY_RIGHT) → setNextDir(RIGHT)
// 반대 방향이면 setNextDir 내부에서 즉시 실패 처리
```

---

## 4. 맵 배열 값 규약 현황

```
현재: 0~7 (7종)
추가: 8~13 (추가 아이템 6종)
```

테두리 구성 수정 내용:
| 위치 | 현재 | 요구 |
|------|------|------|
| 4구석 (0,0)(0,20)(20,0)(20,20) | 2 | 2 (유지) |
| 나머지 테두리 | 2 | 1 (Wall — Gate 가능) |
| 내부 장애물 | 1 | 1 (유지) |

---

## 5. 없는 기능 (신규 구현 대상)

| 기능 | 설명 |
|------|------|
| 추가 아이템 6종 | Speed(8) / Slow(9) / Shield(10) / Ghost(11) / Mirror(12) / Reverse(13) |
| Item 추상 기반 클래스 | Food/Poison 상속 |
| Special 클래스 | 6종 특수 아이템 통합 관리 |
| `drawActiveEffects()` | 활성 효과 실시간 표시 |
| Docker 환경 | Dockerfile + docker-compose.yml |
| 방향키 조작 | WASD → KEY_UP/DOWN/LEFT/RIGHT |
| 반대방향=즉시실패 | Rule #1 원본 복원 |

---

## 6. ncurses 색상 쌍 현황 및 확장 계획

```
pair 1:  BLACK/BLACK   — 미사용
pair 2:  WHITE/WHITE   — Wall
pair 3:  CYAN/CYAN     — Immune Wall
pair 4:  YELLOW/YELLOW — Snake Head
pair 5:  GREEN/GREEN   — Snake Body
pair 6:  GREEN/BLACK   — Growth Item (+)
pair 7:  RED/BLACK     — Poison Item (-)
pair 8:  MAGENTA/MAGENTA — Gate (GG)
--- 신규 추가 ---
pair 9:  CYAN/BLACK    — Speed Boost (>)
pair 10: WHITE/CYAN    — Slow (<)  ← bg색으로 ImmuneWall과 시각 구분
pair 11: YELLOW/BLACK  — Shield (*)
pair 12: MAGENTA/BLACK — Ghost (@)
pair 13: RED/YELLOW    — Mirror (%) ← 주황 대체
pair 14: MAGENTA/BLACK — Reverse (&) ← Ghost와 동일 pair 공유 가능, 별도 pair로 분리
```

---

## 7. 게임 루프 구조 (현재)

```
srand → setlocale → initscr → 색상 정의 → 윈도우 생성 → showStartScreen
→ restart 루프
  → stage 루프 (0~3)
    → loadMap → initGame → spawnGate
    → 게임 루프
      → getch(WASD) → napms(200ms) → moveSnake → updateItems → spawnItem(15틱)
      → updateGate → spawnGate(만료시) → drawMap → drawScoreBoard → checkMission
    → GAME OVER 처리 or Stage Clear 처리
→ endwin
```

TICK_MS가 고정 200ms. 추가 아이템 Speed/Slow 효과로 동적으로 변해야 함.
→ `Special::getCurrentTickMs()` 반환값을 napms()에 사용.
