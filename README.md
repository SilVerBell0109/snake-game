# 🐍 Snake Game

국민대학교 소프트웨어융합대학 C++ 프로젝트
NCurses 라이브러리를 활용한 터미널 기반 Snake Game

---

## 📋 프로젝트 개요

| 항목 | 내용 |
|---|---|
| 언어 | C++ (C++11) |
| 라이브러리 | NCurses |
| 환경 | Linux / macOS 터미널 |
| 맵 크기 | 21 × 21 |
| Stage 수 | 4개 |

---

## 🎮 게임 화면

```
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                %  [ Score Board ]
%                                                %  Stage: 1
%                                                %  Time : 12s
%          ++                                    %
%                  --                            %  B: 5 / 5
%                        HHOOOOO                %  +: 2
%                                                %  -: 0
%              GG                               %  G: 0
%                                                %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  [ Mission ]
                                                    B>= 8 : x
                                                    +>= 3 : x
                                                    ->= 2 : x
                                                    G>= 1 : x
```

---

## 🗺️ 맵 구성 요소

| 기호 | 배열 값 | 설명 |
|---|---|---|
| `  ` | 0 | 빈 공간 |
| `##` | 1 | Wall — Gate로 변환 가능 |
| `%%` | 2 | Immune Wall — Gate 변환 불가 (테두리) |
| `HH` | 3 | Snake Head |
| `OO` | 4 | Snake Body |
| `++` | 5 | Growth Item — 획득 시 몸 길이 +1 |
| `--` | 6 | Poison Item — 획득 시 몸 길이 -1 |
| `GG` | 7 | Gate — 진입 시 다른 Gate로 순간이동 |

---

## 🎯 게임 규칙

### Rule 1 — 뱀 이동
- 일정 시간마다 Head 방향으로 자동 이동 (200ms/틱)
- 진행 방향의 반대 키 입력 시 무시
- 벽 또는 자기 몸 충돌 시 게임 오버

### Rule 2 — 아이템
- Growth Item(`++`) 획득 → 꼬리 1칸 증가
- Poison Item(`--`) 획득 → 꼬리 1칸 감소
- 길이가 3 미만이 되면 게임 오버
- 각 아이템은 최대 3개씩 독립적으로 출현
- 일정 시간 후 다른 위치에 재출현

### Rule 3, 4 — Gate
- Wall 위치에 한 쌍으로 출현
- 진입 시 다른 Gate로 순간이동
- 진출 방향 우선순위: 진입방향 → 시계방향 → 역시계방향 → 반대방향

### Rule 5 — Wall 구분
- `##` Wall: Gate로 변환 가능
- `%%` Immune Wall: Gate 변환 불가, 통과 불가

### Rule 6 — Score Board
- **B**: 현재 길이 / 게임 중 최대 길이
- **+**: 획득한 Growth Item 수
- **-**: 획득한 Poison Item 수
- **G**: Gate 사용 횟수

---

## 🏆 Stage 미션

| Stage | 목표 길이 (B) | Growth (+) | Poison (-) | Gate (G) |
|---|---|---|---|---|
| 1 | 8 이상 | 3개 이상 | 2개 이상 | 1회 이상 |
| 2 | 10 이상 | 5개 이상 | 2개 이상 | 2회 이상 |
| 3 | 12 이상 | 7개 이상 | 3개 이상 | 3회 이상 |
| 4 | 15 이상 | 10개 이상 | 3개 이상 | 5회 이상 |

4가지 조건을 **모두** 달성해야 다음 Stage로 진행됩니다

---

## ⌨️ 조작법

| 키 | 동작 |
|---|---|
| `W` | 위로 이동 |
| `S` | 아래로 이동 |
| `A` | 왼쪽으로 이동 |
| `D` | 오른쪽으로 이동 |
| `R` | 게임 오버 후 재시작 |
| `Q` | 게임 종료 |
| `Space` | 시작 화면에서 게임 시작 |

---

## 🛠️ 빌드 및 실행

### Linux (Ubuntu)
```bash
# NCurses 설치
sudo apt-get install libncurses5-dev libncursesw5-dev

# 빌드 및 실행
g++ -std=c++11 snake.cpp -o snake -lncurses && ./snake
```

### macOS
```bash
# NCurses 설치
brew install ncurses

# 빌드 및 실행
g++ -std=c++11 snake.cpp -o snake \
  -I$(brew --prefix ncurses)/include \
  -L$(brew --prefix ncurses)/lib \
  -lncursesw && ./snake
```

### Windows (WSL 권장)

Windows에서는 **WSL(Windows Subsystem for Linux)** 을 사용하는 것이 가장 간단합니다.

#### 1단계 — WSL 설치 (최초 1회)
PowerShell을 **관리자 권한**으로 열고 실행:
```powershell
wsl --install
```
설치 후 PC를 재부팅하면 Ubuntu가 기본으로 설치됩니다.

#### 2단계 — WSL 터미널에서 빌드 및 실행
```bash
# NCurses 설치
sudo apt-get install libncurses5-dev libncursesw5-dev

# 프로젝트 폴더로 이동 (예: C:\Users\이름\Desktop\snake 에 있는 경우)
cd /mnt/c/Users/이름/Desktop/snake

# 빌드 및 실행
g++ -std=c++11 snake.cpp -o snake -lncurses && ./snake
```

> WSL이 설치되어 있다면 파일 탐색기에서 프로젝트 폴더를 열고,
> 주소창에 `wsl` 을 입력하면 해당 경로에서 바로 WSL 터미널을 열 수 있습니다.

---

## 📁 파일 구조

```
snake-game/
├── snake.cpp     ← 전체 게임 소스코드 (단일 파일)
└── README.md
```

> 맵 데이터(Stage 1~4)는 외부 파일 없이 `snake.cpp` 내부 배열로 내장되어 있습니다

---

## 📚 참고 자료

- [GNU NCurses 공식 사이트](http://www.gnu.org/software/ncurses/)
- [NCURSES Programming HOWTO](http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/)
