/*
 * Snake Game - C++ with ncurses
 * 국민대학교 소프트웨어융합대학 C++ 프로젝트
 *
 * 실행 방법 (VSCode):
 *   settings.json 에 아래 두 줄 추가 후 Run Code (Ctrl+Alt+N)
 *   "code-runner.runInTerminal": true,
 *   "code-runner.executorMap": {
 *       "cpp": "cd $dir && g++ -std=c++11 $fileName -o $fileNameWithoutExt -lncurses && ./$fileNameWithoutExt"
 *   }
 *
 * 조작법:
 *   W / A / S / D  : 이동
 *   Q              : 게임 종료
 *   SPACE          : 시작 화면에서 게임 시작
 *
 * 맵 배열 값 규약:
 *   0 = 빈 공간
 *   1 = Wall       (Gate로 변환 가능)
 *   2 = Immune Wall (Gate 불가)
 *   3 = Snake Head
 *   4 = Snake Body
 *   5 = Growth Item
 *   6 = Poison Item
 *   7 = Gate
 */

#include <ncurses.h>
#include <locale.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>

// ───────────── 상수 정의 ─────────────

const int MAP_SIZE   = 21;   // 맵 크기 (21×21)
const int TICK_MS    = 200;  // 뱀 이동 주기 (밀리초)
const int ITEM_LIMIT = 3;    // 동시 출현 아이템 최대 개수 (Growth/Poison 각각)
const int ITEM_LIFE  = 300;  // 아이템 생존 틱 수
const int GATE_LIFE  = 200;  // 게이트 생존 틱 수

// 방향 상수
const int UP    = 0;
const int DOWN  = 1;
const int LEFT  = 2;
const int RIGHT = 3;

// ───────────── Stage 맵 데이터 (코드 내장) ─────────────
// 2 = Immune Wall (테두리), 1 = Wall (내부), 0 = 빈 공간

const int STAGE_MAPS[4][MAP_SIZE][MAP_SIZE] = {
    // Stage 1 — 빈 맵 (테두리만)
    {
        {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    },
    // Stage 2 — L자 Wall 2개
    {
        {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    },
    // Stage 3 — 4구석 기둥 + 가로 Wall
    {
        {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,2},
        {2,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,2},
        {2,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,2},
        {2,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,2},
        {2,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    },
    // Stage 4 — 미로형 Wall
    {
        {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,2},
        {2,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,2},
        {2,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,2},
        {2,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,2},
        {2,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,2},
        {2,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    }
};

// ───────────── 미션 조건 ─────────────

struct Mission {
    int targetLength;   // 목표 뱀 길이
    int targetGrowth;   // 목표 Growth 획득 수
    int targetPoison;   // 목표 Poison 획득 수 (이상)
    int targetGate;     // 목표 Gate 사용 횟수
};

const Mission MISSIONS[4] = {
    {  8,  3, 2, 1 },   // Stage 1
    { 10,  5, 2, 2 },   // Stage 2
    { 12,  7, 3, 3 },   // Stage 3
    { 15, 10, 3, 5 },   // Stage 4
};

// ───────────── 전역 변수 ─────────────

int map[MAP_SIZE][MAP_SIZE];      // 현재 맵 배열 (실시간 변경)
int baseMap[MAP_SIZE][MAP_SIZE];  // 원본 맵 (아이템·게이트 복원용)

// 뱀 몸통 — 앞쪽이 Head
struct Point { int y, x; };
std::vector<Point> snake;
int snakeDir;   // 현재 진행 방향
int nextDir;    // 다음 틱에 적용할 방향 (입력 버퍼)

// 아이템
struct Item {
    int y, x;
    int type;   // 5 = Growth, 6 = Poison
    int life;   // 남은 생존 틱
};
std::vector<Item> items;

// 게이트
struct Gate {
    int y, x;
    bool active;
};
Gate gateA, gateB;
int  gateTick = 0;

// 점수
int score_maxLen = 0;
int score_growth = 0;
int score_poison = 0;
int score_gate   = 0;
int score_time   = 0;

// 게임 상태
int  currentStage = 0;
bool gameFailed   = false;
bool stageClear   = false;
bool gameOver     = false;

// ncurses 윈도우
WINDOW* winMap;
WINDOW* winBoard;

// ───────────── 유틸리티 함수 ─────────────

int getOppositeDir(int dir) {
    if (dir == UP)    return DOWN;
    if (dir == DOWN)  return UP;
    if (dir == LEFT)  return RIGHT;
    return LEFT;
}

Point getNextHead(int dir) {
    Point h = snake[0];
    if (dir == UP)    h.y--;
    if (dir == DOWN)  h.y++;
    if (dir == LEFT)  h.x--;
    if (dir == RIGHT) h.x++;
    return h;
}

// Gate가 위치한 벽의 기본 진출 방향 (맵 안쪽을 향하는 방향)
int getWallDir(int y, int x) {
    if (y == 0)            return DOWN;
    if (y == MAP_SIZE - 1) return UP;
    if (x == 0)            return RIGHT;
    if (x == MAP_SIZE - 1) return LEFT;
    return DOWN;
}

// ───────────── 맵 초기화 ─────────────

void loadMap(int stage) {
    for (int i = 0; i < MAP_SIZE; i++)
        for (int j = 0; j < MAP_SIZE; j++) {
            baseMap[i][j] = STAGE_MAPS[stage][i][j];
            map[i][j]     = STAGE_MAPS[stage][i][j];
        }
}

// ───────────── 게임 초기화 ─────────────

void initGame() {
    // 뱀 초기 위치 — 맵 중앙, 오른쪽 방향
    snake.clear();
    snake.push_back({10, 12});  // Head
    snake.push_back({10, 11});  // Body
    snake.push_back({10, 10});  // Tail

    snakeDir = RIGHT;
    nextDir  = RIGHT;

    items.clear();
    gateA.active = false;
    gateB.active = false;
    gateTick = 0;

    score_growth = 0;
    score_poison = 0;
    score_gate   = 0;
    score_time   = 0;
    score_maxLen = (int)snake.size();

    gameFailed = false;
    stageClear = false;

    // 맵에 뱀 반영
    map[snake[0].y][snake[0].x] = 3;
    for (int i = 1; i < (int)snake.size(); i++)
        map[snake[i].y][snake[i].x] = 4;
}

// ───────────── 그리기: 맵 ─────────────

void drawMap() {
    werase(winMap);
    box(winMap, 0, 0);

    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            wmove(winMap, i + 1, j * 2 + 1);
            switch (map[i][j]) {
                case 0:  // 빈 공간
                    wprintw(winMap, "  ");
                    break;
                case 1:  // Wall
                    wattron(winMap, COLOR_PAIR(2));
                    wprintw(winMap, "##");
                    wattroff(winMap, COLOR_PAIR(2));
                    break;
                case 2:  // Immune Wall
                    wattron(winMap, COLOR_PAIR(3));
                    wprintw(winMap, "%%");
                    wattroff(winMap, COLOR_PAIR(3));
                    break;
                case 3:  // Snake Head
                    wattron(winMap, COLOR_PAIR(4));
                    wprintw(winMap, "HH");
                    wattroff(winMap, COLOR_PAIR(4));
                    break;
                case 4:  // Snake Body
                    wattron(winMap, COLOR_PAIR(5));
                    wprintw(winMap, "OO");
                    wattroff(winMap, COLOR_PAIR(5));
                    break;
                case 5:  // Growth Item
                    wattron(winMap, COLOR_PAIR(6));
                    wprintw(winMap, "++");
                    wattroff(winMap, COLOR_PAIR(6));
                    break;
                case 6:  // Poison Item
                    wattron(winMap, COLOR_PAIR(7));
                    wprintw(winMap, "--");
                    wattroff(winMap, COLOR_PAIR(7));
                    break;
                case 7:  // Gate
                    wattron(winMap, COLOR_PAIR(8));
                    wprintw(winMap, "GG");
                    wattroff(winMap, COLOR_PAIR(8));
                    break;
                default:
                    wprintw(winMap, "  ");
                    break;
            }
        }
    }
    wrefresh(winMap);
}

// ───────────── 그리기: 점수판 ─────────────

void drawScoreBoard() {
    werase(winBoard);
    box(winBoard, 0, 0);

    const Mission& m = MISSIONS[currentStage];

    mvwprintw(winBoard, 1,  2, "[ Score Board ]");
    mvwprintw(winBoard, 2,  2, "Stage: %d", currentStage + 1);
    mvwprintw(winBoard, 3,  2, "Time : %ds", score_time);

    mvwprintw(winBoard, 5,  2, "B: %d / %d", (int)snake.size(), score_maxLen);
    mvwprintw(winBoard, 6,  2, "+: %d", score_growth);
    mvwprintw(winBoard, 7,  2, "-: %d", score_poison);
    mvwprintw(winBoard, 8,  2, "G: %d", score_gate);

    mvwprintw(winBoard, 10, 2, "[ Mission ]");
    mvwprintw(winBoard, 11, 2, "B>=%2d : %s", m.targetLength,
              (int)snake.size() >= m.targetLength ? "v" : "x");
    mvwprintw(winBoard, 12, 2, "+>=%2d : %s", m.targetGrowth,
              score_growth >= m.targetGrowth ? "v" : "x");
    mvwprintw(winBoard, 13, 2, "->=%2d : %s", m.targetPoison,
              score_poison >= m.targetPoison ? "v" : "x");
    mvwprintw(winBoard, 14, 2, "G>=%2d : %s", m.targetGate,
              score_gate >= m.targetGate ? "v" : "x");

    wrefresh(winBoard);
}

// ───────────── 아이템 ─────────────

void spawnItem() {
    // Growth와 Poison 각각의 현재 개수를 따로 카운트
    int growthCount = 0, poisonCount = 0;
    for (auto& it : items) {
        if (it.type == 5) growthCount++;
        if (it.type == 6) poisonCount++;
    }

    // 둘 다 최대치면 스킵
    if (growthCount >= ITEM_LIMIT && poisonCount >= ITEM_LIMIT) return;

    // 빈 칸 목록 수집
    std::vector<Point> empty;
    for (int i = 0; i < MAP_SIZE; i++)
        for (int j = 0; j < MAP_SIZE; j++)
            if (map[i][j] == 0)
                empty.push_back({i, j});

    if (empty.empty()) return;

    // 생성 가능한 타입 결정
    // 둘 다 가능하면 랜덤, 하나만 가능하면 그것만 생성
    int typ = 0;
    bool canGrowth = (growthCount < ITEM_LIMIT);
    bool canPoison = (poisonCount < ITEM_LIMIT);

    if (canGrowth && canPoison)
        typ = (rand() % 2 == 0) ? 5 : 6;
    else if (canGrowth)
        typ = 5;
    else
        typ = 6;

    Point pos = empty[rand() % empty.size()];
    items.push_back({pos.y, pos.x, typ, ITEM_LIFE});
    map[pos.y][pos.x] = typ;
}

void updateItems() {
    for (auto it = items.begin(); it != items.end(); ) {
        it->life--;
        if (it->life <= 0) {
            map[it->y][it->x] = baseMap[it->y][it->x];
            it = items.erase(it);
        } else {
            ++it;
        }
    }
}

// ───────────── 게이트 ─────────────

void spawnGate() {
    if (gateA.active && gateB.active) return;

    // Gate 가능한 Wall(1) 위치 수집
    std::vector<Point> walls;
    for (int i = 0; i < MAP_SIZE; i++)
        for (int j = 0; j < MAP_SIZE; j++)
            if (baseMap[i][j] == 1)
                walls.push_back({i, j});

    if ((int)walls.size() < 2) return;

    // 기존 게이트 제거
    if (gateA.active) map[gateA.y][gateA.x] = baseMap[gateA.y][gateA.x];
    if (gateB.active) map[gateB.y][gateB.x] = baseMap[gateB.y][gateB.x];

    int idxA = rand() % walls.size();
    int idxB = rand() % walls.size();
    while (idxB == idxA) idxB = rand() % walls.size();

    gateA = {walls[idxA].y, walls[idxA].x, true};
    gateB = {walls[idxB].y, walls[idxB].x, true};
    map[gateA.y][gateA.x] = 7;
    map[gateB.y][gateB.x] = 7;
    gateTick = GATE_LIFE;
}

void updateGate() {
    if (!gateA.active) return;
    gateTick--;
    if (gateTick <= 0) {
        map[gateA.y][gateA.x] = baseMap[gateA.y][gateA.x];
        map[gateB.y][gateB.x] = baseMap[gateB.y][gateB.x];
        gateA.active = false;
        gateB.active = false;
    }
}

// ───────────── 뱀 이동 ─────────────

void moveSnake() {
    // 반대 방향 입력은 무시하고 현재 방향 유지
    if (nextDir == getOppositeDir(snakeDir))
        nextDir = snakeDir;
    snakeDir = nextDir;

    Point newHead = getNextHead(snakeDir);

    // 경계 밖 → 실패
    if (newHead.y < 0 || newHead.y >= MAP_SIZE ||
        newHead.x < 0 || newHead.x >= MAP_SIZE) {
        gameFailed = true;
        return;
    }

    int cellVal = map[newHead.y][newHead.x];

    // Immune Wall 충돌 → 실패
    if (cellVal == 2) { gameFailed = true; return; }

    // 일반 Wall 충돌 → 실패
    if (cellVal == 1) { gameFailed = true; return; }

    // 자기 몸 충돌 → 실패 (Tail은 이번 틱에 빠지므로 제외)
    for (int i = 0; i < (int)snake.size() - 1; i++) {
        if (snake[i].y == newHead.y && snake[i].x == newHead.x) {
            gameFailed = true;
            return;
        }
    }

    // Gate 진입 처리
    if (cellVal == 7) {
        // 진입 Gate와 쌍인 출구 Gate 결정
        Gate* exitGate = nullptr;
        if (gateA.active && gateA.y == newHead.y && gateA.x == newHead.x)
            exitGate = &gateB;
        else if (gateB.active && gateB.y == newHead.y && gateB.x == newHead.x)
            exitGate = &gateA;

        if (exitGate && exitGate->active) {
            // 진출 방향 우선순위: 진입방향 → 시계방향 → 역시계방향 → 반대방향
            int priority[4];
            priority[0] = snakeDir;
            // 시계방향
            int cw[4] = {RIGHT, LEFT, UP, DOWN};  // UP→RIGHT, DOWN→LEFT, LEFT→UP, RIGHT→DOWN
            priority[1] = cw[snakeDir];
            // 역시계방향
            int ccw[4] = {LEFT, RIGHT, DOWN, UP};
            priority[2] = ccw[snakeDir];
            priority[3] = getOppositeDir(snakeDir);

            int exitDir = priority[3];  // 기본값: 반대 방향
            for (int i = 0; i < 4; i++) {
                int d = priority[i];
                int ny = exitGate->y, nx = exitGate->x;
                if (d == UP)    ny--;
                if (d == DOWN)  ny++;
                if (d == LEFT)  nx--;
                if (d == RIGHT) nx++;
                // 맵 안쪽이고 Wall이 아닌 경우만 허용
                if (ny >= 0 && ny < MAP_SIZE && nx >= 0 && nx < MAP_SIZE &&
                    map[ny][nx] != 1 && map[ny][nx] != 2 && map[ny][nx] != 7) {
                    exitDir = d;
                    break;
                }
            }

            // 출구 Gate에서 exitDir 방향으로 한 칸 전진
            newHead.y = exitGate->y;
            newHead.x = exitGate->x;
            if (exitDir == UP)    newHead.y--;
            if (exitDir == DOWN)  newHead.y++;
            if (exitDir == LEFT)  newHead.x--;
            if (exitDir == RIGHT) newHead.x++;

            if (newHead.y < 0 || newHead.y >= MAP_SIZE ||
                newHead.x < 0 || newHead.x >= MAP_SIZE ||
                map[newHead.y][newHead.x] == 1 ||
                map[newHead.y][newHead.x] == 2) {
                gameFailed = true;
                return;
            }
            snakeDir = exitDir;
            score_gate++;
        }
    }

    // Growth Item 획득
    bool grew = false;
    if (map[newHead.y][newHead.x] == 5) {
        grew = true;
        score_growth++;
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (it->y == newHead.y && it->x == newHead.x) {
                items.erase(it);
                break;
            }
        }
    }

    // Poison Item 획득
    bool shrank = false;
    if (map[newHead.y][newHead.x] == 6) {
        shrank = true;
        score_poison++;
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (it->y == newHead.y && it->x == newHead.x) {
                items.erase(it);
                break;
            }
        }
    }

    // Tail 제거 (Growth가 아닐 때)
    if (!grew) {
        Point tail = snake.back();
        map[tail.y][tail.x] = baseMap[tail.y][tail.x];
        snake.pop_back();
    }

    // Poison: 꼬리 한 칸 추가 제거
    if (shrank) {
        if ((int)snake.size() <= 1) { gameFailed = true; return; }
        Point tail = snake.back();
        map[tail.y][tail.x] = baseMap[tail.y][tail.x];
        snake.pop_back();
        // 길이 3 미만 → 실패
        if ((int)snake.size() < 3) { gameFailed = true; return; }
    }

    // 기존 Head → Body
    if (!snake.empty())
        map[snake[0].y][snake[0].x] = 4;

    // 새 Head 삽입
    snake.insert(snake.begin(), newHead);
    map[newHead.y][newHead.x] = 3;

    // 최대 길이 갱신
    if ((int)snake.size() > score_maxLen)
        score_maxLen = (int)snake.size();
}

// ───────────── 미션 확인 ─────────────

bool checkMission() {
    const Mission& m = MISSIONS[currentStage];
    return (int)snake.size() >= m.targetLength &&
           score_growth      >= m.targetGrowth &&
           score_poison      >= m.targetPoison &&
           score_gate        >= m.targetGate;
}

// ───────────── 메시지 표시 ─────────────

void showMessage(const std::string& msg) {
    int y = MAP_SIZE / 2 + 1;
    int x = MAP_SIZE + 1 - (int)msg.size() / 2;
    if (x < 1) x = 1;
    mvwprintw(winMap, y, x, "%s", msg.c_str());
    wrefresh(winMap);
    napms(1800);
}

// ───────────── 시작 화면 ─────────────

void showStartScreen() {
    clear();
    int row, col;
    getmaxyx(stdscr, row, col);
    int cy = row / 2;
    int cx = col / 2;

    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(cy - 5, cx - 9,  " S N A K E  G A M E ");
    attroff(COLOR_PAIR(4) | A_BOLD);

    attron(COLOR_PAIR(3));
    mvprintw(cy - 4, cx - 13, "──────────────────────────────");
    attroff(COLOR_PAIR(3));

    mvprintw(cy - 2, cx - 8,  "[ 조작법 ]");
    mvprintw(cy - 1, cx - 8,  "  W  :  위로 이동");
    mvprintw(cy,     cx - 8,  "  S  :  아래로 이동");
    mvprintw(cy + 1, cx - 8,  "  A  :  왼쪽으로 이동");
    mvprintw(cy + 2, cx - 8,  "  D  :  오른쪽으로 이동");
    mvprintw(cy + 3, cx - 8,  "  Q  :  게임 종료");

    attron(COLOR_PAIR(3));
    mvprintw(cy + 4, cx - 13, "──────────────────────────────");
    attroff(COLOR_PAIR(3));

    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(cy + 6, cx - 13, "  SPACE BAR 를 눌러 시작하세요  ");
    attroff(COLOR_PAIR(6) | A_BOLD);

    refresh();

    // 스페이스바 대기
    nodelay(stdscr, FALSE);
    while (getch() != ' ') {}
    clear();
    refresh();
}

// ───────────── main ─────────────

int main() {
    srand((unsigned)time(nullptr));

    // 한글 출력을 위한 로케일 설정 (반드시 initscr() 이전에 호출)
    setlocale(LC_ALL, "");

    // ncurses 초기화
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);
    start_color();

    // 색상 쌍 정의 (pair번호, 전경색, 배경색)
    init_pair(1, COLOR_BLACK,   COLOR_BLACK);    // 빈 공간 (미사용)
    init_pair(2, COLOR_WHITE,   COLOR_WHITE);    // Wall
    init_pair(3, COLOR_CYAN,    COLOR_CYAN);     // Immune Wall
    init_pair(4, COLOR_YELLOW,  COLOR_YELLOW);   // Snake Head
    init_pair(5, COLOR_GREEN,   COLOR_GREEN);    // Snake Body
    init_pair(6, COLOR_GREEN,   COLOR_BLACK);    // Growth Item
    init_pair(7, COLOR_RED,     COLOR_BLACK);    // Poison Item
    init_pair(8, COLOR_MAGENTA, COLOR_MAGENTA);  // Gate

    // 윈도우 생성
    winMap   = newwin(MAP_SIZE + 2, MAP_SIZE * 2 + 2, 0, 0);
    winBoard = newwin(MAP_SIZE + 2, 22, 0, MAP_SIZE * 2 + 3);

    // 시작 화면 (스페이스바 대기)
    showStartScreen();

    // 재시작 루프 — R키를 누르면 Stage 1부터 다시 시작
    bool restart = true;
    while (restart) {
        restart   = false;
        gameOver  = false;
        gameFailed = false;

        // Stage 루프
        for (currentStage = 0; currentStage < 4; currentStage++) {
            loadMap(currentStage);
            initGame();
            spawnGate();

            int itemTick = 0;
            int timeTick = 0;

            // nodelay: getch()가 입력 없어도 즉시 반환 (게임 루프용)
            nodelay(winMap, TRUE);

            while (!gameFailed && !stageClear) {
                // 키 입력 (WASD)
                int key = wgetch(winMap);
                if ((key == 'w' || key == 'W') && snakeDir != DOWN)  nextDir = UP;
                if ((key == 's' || key == 'S') && snakeDir != UP)    nextDir = DOWN;
                if ((key == 'a' || key == 'A') && snakeDir != RIGHT) nextDir = LEFT;
                if ((key == 'd' || key == 'D') && snakeDir != LEFT)  nextDir = RIGHT;
                if (key == 'q' || key == 'Q') { gameFailed = true; break; }

                // 한 틱 대기
                napms(TICK_MS);
                itemTick++;
                timeTick++;

                // 경과 시간 계산
                if (timeTick >= (1000 / TICK_MS)) {
                    score_time++;
                    timeTick = 0;
                }

                // 뱀 이동
                moveSnake();
                if (gameFailed) break;

                // 아이템 수명 갱신 및 신규 출현
                updateItems();
                if (itemTick >= 15) {
                    spawnItem();
                    itemTick = 0;
                }

                // 게이트 수명 갱신 및 재생성
                updateGate();
                if (!gateA.active) spawnGate();

                // 화면 갱신
                drawMap();
                drawScoreBoard();

                // 미션 달성 확인
                if (checkMission()) stageClear = true;
            }

            // Stage 결과 처리
            if (gameFailed) {
                // GAME OVER 메시지 출력
                showMessage(" ** GAME OVER **  R: 재시작  Q: 종료 ");
                nodelay(winMap, FALSE);

                // R 또는 Q 입력 대기
                int key;
                while (true) {
                    key = wgetch(winMap);
                    if (key == 'r' || key == 'R') { restart = true;  break; }
                    if (key == 'q' || key == 'Q') { restart = false; break; }
                }
                break;   // Stage 루프 탈출
            }

            if (stageClear) {
                if (currentStage < 3) {
                    showMessage(" Stage Clear!  Next Stage... ");
                } else {
                    showMessage(" ** ALL CLEAR **  Congratulations! ");
                    gameOver = true;
                }
                napms(500);
            }
        }
    }

    // 종료 처리
    nodelay(winMap, FALSE);
    if (gameOver) wgetch(winMap);

    delwin(winMap);
    delwin(winBoard);
    endwin();
    return 0;
}
