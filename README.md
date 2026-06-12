# Snake Game

국민대학교 소프트웨어융합대학 C++ 프로젝트  
NCurses 라이브러리를 활용한 터미널 기반 Snake Game  
OOP 멀티파일 클래스 구조로 완전 재작성, 특수 아이템 6종 및 Docker 환경 포함

---

## 프로젝트 개요

| 항목 | 내용 |
|---|---|
| 언어 | C++ (C++11) |
| 라이브러리 | NCurses (libncursesw) |
| 환경 | Linux (Ubuntu 22.04) / macOS (Apple Silicon) |
| 맵 크기 | 21 × 21 |
| Stage 수 | 4개 |
| 아키텍처 | OOP 멀티파일 클래스 분리 (전역 변수 없음) |

---

## 게임 화면

```
+------------------------------------------+  [ Score Board ]
|##############################################|  Stage  : 1
|##                                        ##|  Time   : 23s
|##                                        ##|
|##        ++                              ##|  B: 6 / 6
|##                  --                    ##|  +: 2
|##              HHOOOOO                   ##|  -: 0
|##                                        ##|  G: 0
|##           GG                           ##|
|##                                        ##|  [ Mission ]
|############################################|  B>= 8 : x
                                               +>= 3 : x
                                               ->= 2 : x
                                               G>= 1 : x

                                               [ Effects ]
                                               SPEED(38) SHIELD
```

---

## 아키텍처 — 클래스 구조

```
Common.h          ← 전 클래스 공유 상수·열거형·구조체 (Point, Item, Mission, oppositeDir)
│
├── Board         ← 21×21 맵 배열 관리, ncurses 윈도우 렌더링
├── ItemBase      ← 아이템 추상 기반 클래스 (순수 가상: spawn/update/consume/getCount)
│   ├── Food      ← Growth Item(5) 스폰·수명·소비
│   └── Poison    ← Poison Item(6) 스폰·수명·소비
├── Gate          ← Wall 위치 Gate 쌍 생성·수명·진출 방향 계산
├── Special       ← 특수 아이템 6종 통합 관리 (맵 아이템 + 활성 효과 + 플래그)
└── Snake         ← 몸통 벡터, 방향 버퍼, 이동·충돌·Gate·Reverse 처리
```

### 설계 원칙
- **전역 변수 없음** — 모든 상태는 클래스 멤버로 보유
- **`const` 적극 적용** — getter, 파라미터, 배열 참조에 const 명시
- **ItemBase 추상 클래스** — `spawnItem()` / `updateItems()` 공통 헬퍼를 protected로 제공하여 Food·Poison 코드 중복 제거
- **파일별 역할 주석** — 각 `.h` / `.cpp` 상단에 파일 역할 설명 포함

---

## 맵 구성 요소

| 기호 | 배열 값 | 색상 | 설명 |
|---|---|---|---|
| `  ` | 0 | — | 빈 공간 |
| `##` | 1 | 흰색/파랑 | Wall — Gate로 변환 가능 |
| `##` | 2 | 흰색/흰색 | Immune Wall — Gate 불가, 항상 치명적 |
| `HH` | 3 | 검정/노랑 | Snake Head |
| `OO` | 4 | 검정/초록 | Snake Body |
| `++` | 5 | 초록 | Growth Item — 길이 +1 |
| `--` | 6 | 빨강 | Poison Item — 길이 -1 |
| `GG` | 7 | 마젠타 | Gate — 순간이동 |
| `>>` | 8 | 청록 | Speed Item — 이동 속도 증가 |
| `<<` | 9 | 흰색/청록 | Slow Item — 이동 속도 감소 |
| `**` | 10 | 노랑 | Shield Item — 충돌 1회 방어 |
| `@@` | 11 | 마젠타 | Ghost Item — 자체 몸통 통과 |
| `%%` | 12 | 빨강/노랑 | Mirror Item — 좌우 키 반전 |
| `&&` | 13 | 마젠타(굵게) | Reverse Item — 몸 즉시 뒤집기 |

### 테두리 규칙
- **4 꼭짓점** `(0,0) (0,20) (20,0) (20,20)` → Immune Wall (2) — Gate 불가
- **나머지 테두리** → Wall (1) — Gate 생성 가능

---

## 특수 아이템 상세 (가산점)

모든 특수 아이템: 수명 **150틱**, **20틱마다** 스폰 시도

| 아이템 | 효과 | 지속 |
|---|---|---|
| Speed `>>` | 이동 틱을 200ms → 100ms로 단축 | 50틱 |
| Slow `<<` | 이동 틱을 200ms → 400ms로 연장 | 50틱 |
| Shield `**` | 벽 또는 자체 충돌 1회 흡수 후 소멸 | 충돌 전까지 무제한 |
| Ghost `@@` | 자기 몸통 충돌 판정 완전 무시 | 50틱 |
| Mirror `%%` | 좌우 방향키 입력 반전 | 50틱 |
| Reverse `&&` | `std::reverse()`로 몸통 즉시 역전, 진행 방향 반전 | 즉시 (1회) |

**충돌 규칙:**
- Speed가 활성화된 상태에서 Slow는 스폰되지 않음 (반대도 동일)
- Immune Wall에는 Shield 효과 없음 — 항상 게임 오버
- Shield가 Wall 또는 자체 충돌을 흡수하면 해당 틱의 이동은 취소됨

---

## 게임 규칙

### 이동
- 일정 시간(기본 **200ms**/틱)마다 Head 방향으로 자동 이동
- **반대 방향 입력 시 즉시 게임 오버** (무시 아님)
- Speed/Slow 아이템으로 틱 속도 변동

### 충돌 판정
| 충돌 대상 | 결과 |
|---|---|
| Immune Wall (2) | 즉시 게임 오버 |
| Wall (1) | 게임 오버 (Shield 보유 시 흡수) |
| 자기 몸통 | 게임 오버 (Ghost 활성 시 무시 / Shield 보유 시 흡수) |
| 길이 3 미만 | 즉시 게임 오버 |

### Gate 진출 방향 우선순위
진입 방향 → 시계방향 → 역시계방향 → 반대방향

진출 위치에 빈 공간이 없으면 다음 우선순위로 대체됩니다.

### Score Board
| 항목 | 설명 |
|---|---|
| B | 현재 길이 / 게임 중 최대 길이 |
| + | 획득한 Growth Item 누적 수 |
| - | 획득한 Poison Item 누적 수 |
| G | Gate 통과 횟수 |
| Effects | 현재 활성화된 특수 효과 및 남은 틱 |

---

## Stage 맵 & 미션

| Stage | 맵 특징 | 목표 길이 | Growth | Poison | Gate |
|---|---|---|---|---|---|
| 1 | 테두리만 | 6 이상 | 2 이상 | 1 이상 | 0 이상 |
| 2 | 단벽 2개 (가로+세로) | 7 이상 | 3 이상 | 1 이상 | 1 이상 |
| 3 | 4구석 3×3 기둥 | 8 이상 | 4 이상 | 2 이상 | 2 이상 |
| 4 | 십자형(+) 벽 | 10 이상 | 5 이상 | 2 이상 | 3 이상 |

4가지 조건을 **모두** 달성해야 다음 Stage로 진행됩니다.

---

## 조작법

| 키 | 동작 |
|---|---|
| `↑` | 위로 이동 |
| `↓` | 아래로 이동 |
| `←` | 왼쪽으로 이동 (Mirror 활성 시 오른쪽) |
| `→` | 오른쪽으로 이동 (Mirror 활성 시 왼쪽) |
| `Q` | 게임 종료 |
| `R` | 게임 오버 후 재시작 |
| `Space` | 시작 화면에서 게임 시작 |

---

## 빌드 및 실행

### macOS (Homebrew ncurses)

```bash
brew install ncurses

cd snake-game
make
./snake
```

### Linux (Ubuntu)

```bash
sudo apt-get install libncursesw5-dev

cd snake-game
make
./snake
```

Makefile이 `uname` 으로 OS를 자동 감지하여 ncurses 경로를 분기합니다.

---

## Docker로 실행 (Ubuntu 22.04 — 권장, 협업용)

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

> `docker compose up` 은 ncurses 화면 출력이 안 됨  
> 반드시 `docker compose run --rm snake` 로 실행할 것

---

## 파일 구조

```
snake/
├── Dockerfile            ← Ubuntu 22.04 빌드 환경
├── docker-compose.yml    ← tty/stdin 설정 포함
└── snake-game/
    ├── Common.h          ← 상수, 열거형, Point/Item/Mission 구조체, oppositeDir()
    ├── Board.h / .cpp    ← 맵 배열, ncurses 윈도우, 렌더링(14 색상 쌍)
    ├── Item.h / .cpp     ← ItemBase 추상 클래스 (spawnItem/updateItems 헬퍼)
    ├── Food.h / .cpp     ← Growth Item (ItemBase 상속)
    ├── Poison.h / .cpp   ← Poison Item (ItemBase 상속)
    ├── Gate.h / .cpp     ← Gate 쌍 생성, 수명, 진출 방향 계산
    ├── Special.h / .cpp  ← 특수 아이템 6종 통합 관리
    ├── Snake.h / .cpp    ← 몸통 벡터, 방향 버퍼, 이동/충돌/아이템 처리
    ├── main.cpp          ← ncurses 초기화, 게임 루프, Stage 흐름 제어
    └── Makefile          ← OS 자동 감지 (Darwin/Linux)
```

---

## 참고 자료

- [GNU NCurses 공식 사이트](http://www.gnu.org/software/ncurses/)
- [NCURSES Programming HOWTO](http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/)
