# Research — 제출 전 정리 현황 조사

조사 일자: 2026-06-12

---

## 항목 A — 루트 snake.cpp

| 항목 | 현황 |
|------|------|
| 파일 경로 | `/snake.cpp` (저장소 루트) |
| 줄 수 | 798줄 |
| 구조 | 전역변수 + 함수 기반, 클래스 없음, WASD 전용 |
| 반대 방향 처리 | 무시(반응 없음) — 과제 요구(게임오버)와 다름 |
| 아이템 제한 | Growth 3개 + Poison 3개 각각 독립 — 합산 3개 제한 아님 |
| 미션 시스템 | 없음 — allCollected() 기반 클리어만 존재 |

**참조 여부 (grep 결과: 0건)**

| 파일 | 결과 |
|------|------|
| README.md | snake.cpp 언급 없음 |
| Makefile (루트) | `$(MAKE) -C snake-game` 위임 — snake.cpp 빌드 안 함 |
| Dockerfile | `RUN cd snake-game && make clean && make` — 언급 없음 |
| .gitignore | `snake`, `snake-game/snake`, `*.o`, `.DS_Store` — snake.cpp 없음 |
| docker-compose.yml | context/dockerfile 설정뿐 — 언급 없음 |

결론: 루트 snake.cpp를 참조하는 파일 없음. 삭제 또는 이동해도 빌드/README/Docker에 영향 없음.

---

## 항목 B — highscore.txt 및 Dockerfile 실행 경로

### B-1. highscore.txt

| 항목 | 현황 |
|------|------|
| 경로 | `snake-game/highscore.txt` |
| git 추적 여부 | **추적 중** (`git ls-files` 확인) |
| 현재 내용 | `0` (초기값) |
| .gitignore 등재 여부 | **없음** |
| 프로그램 읽기 | `main.cpp:26` — `std::ifstream ifs("highscore.txt")` (cwd 기준 상대경로) |
| 프로그램 쓰기 | `main.cpp:34` — `std::ofstream ofs("highscore.txt")` (동일 cwd) |

### B-2. Dockerfile 실행 경로 불일치

| 항목 | 현황 |
|------|------|
| WORKDIR | `/app` |
| 빌드 | `RUN cd snake-game && make clean && make` |
| CMD | `CMD ["./snake-game/snake"]` |
| 실행 시 cwd | `/app` (WORKDIR 유지) |
| 실제 highscore.txt 경로 | `/app/highscore.txt` |
| 의도한 경로 | `snake-game/highscore.txt` → `/app/snake-game/highscore.txt` |
| **불일치** | CMD가 `/app`에서 실행되므로 highscore가 `/app/highscore.txt`에 생성됨 |

---

## 항목 C — 데드 코드

| 함수 | 정의 | 선언 | 전체 소스 grep 호출 건수 |
|------|------|------|--------------------------|
| `Gate::wallFacingDir()` | Gate.cpp:16 | Gate.h:37 | **0건** |
| `Snake::getDir()` | Snake.cpp:166 | Snake.h:31 | **0건** |

grep 범위: `snake-game/*.cpp`, `snake-game/*.h` 전체.  
멤버 함수라 `-Wall -Wextra`로는 경고 미발생.

---

## 항목 D — 미션 설계 확인 (코드 변경 없음, 보고만)

### D-1. Mission B 판정: 최대 길이(getMaxLength) 기준

- `main.cpp:266` — `snake.getMaxLength() >= m.targetLength`
- `Snake::maxLength_` = `body_.size()` 역대 최댓값 (Poison 섭취 후 감소해도 유지)
- 동작: Poison으로 길이가 줄어도 한 번 B 조건 충족 시 이후에도 [v] 유지

### D-2. Stage 4 미션 {10, 5, 2, 3} — B/+ 조건 중복

- `Board.cpp:15` — `{ 10, 5, 2, 3 }`
- 초기 길이 3, Growth 1개당 +1 → maxLength 10 달성에 Growth 7개 필요
- +≥5 목표는 B≥10 달성 전에 항상 먼저 충족됨 → 사실상 B 조건이 + 조건을 포함
