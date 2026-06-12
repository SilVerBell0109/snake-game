// main.cpp
// 게임 루프, ncurses 초기화, Stage 흐름 제어, 최고 점수 파일 저장/로드 [가산점]
//
// 조작법: 방향키(↑↓←→) 이동  Q 종료  반대 방향 입력 시 즉시 게임 오버
// 맵 값:  0=빈공간 1=Wall 2=ImmuneWall 3=Head 4=Body
//         5=Growth 6=Poison 7=Gate
//         8=Speed 9=Slow 10=Shield 11=Ghost 12=Mirror 13=Reverse [가산점]

#include "Common.h"
#include "Board.h"
#include "Snake.h"
#include "Food.h"
#include "Poison.h"
#include "Gate.h"
#include "Special.h"

#include <ncurses.h>
#include <locale.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>

// ── 최고 점수 파일 I/O [가산점] ──────────────────────────────────
static int loadHighScore() {
    std::ifstream ifs("highscore.txt");
    if (!ifs.is_open()) return 0;
    int score = 0;
    ifs >> score;
    return score;
}

static void saveHighScore(const int score) {
    std::ofstream ofs("highscore.txt");
    if (ofs.is_open()) ofs << score;
}

// ── 시작 화면 ────────────────────────────────────────────────────
static void showStartScreen(const int highScore) {
    clear();
    int row, col;
    getmaxyx(stdscr, row, col);
    const int cy = row / 2;
    const int cx = col / 2;

    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(cy - 8, cx - 10, "  S N A K E  G A M E  ");
    attroff(COLOR_PAIR(4) | A_BOLD);

    attron(COLOR_PAIR(3));
    mvprintw(cy - 7, cx - 14, "────────────────────────────────");
    attroff(COLOR_PAIR(3));

    mvprintw(cy - 5, cx - 9, "[ 조작법 ]");
    mvprintw(cy - 4, cx - 9, "  Arrow / WASD  :  이동");
    mvprintw(cy - 3, cx - 9, "  Q             :  게임 종료");
    mvprintw(cy - 2, cx - 9, "  반대 방향 입력 → 무시됨");

    attron(COLOR_PAIR(3));
    mvprintw(cy - 1, cx - 14, "────────────────────────────────");
    attroff(COLOR_PAIR(3));

    mvprintw(cy, cx - 9, "[ 아이템 ]");
    attron(COLOR_PAIR(6));  mvprintw(cy + 1, cx - 9, "  ++ Growth  : 길이+1");   attroff(COLOR_PAIR(6));
    attron(COLOR_PAIR(7));  mvprintw(cy + 2, cx - 9, "  -- Poison  : 길이-1");   attroff(COLOR_PAIR(7));
    attron(COLOR_PAIR(8));  mvprintw(cy + 3, cx - 9, "  GG Gate    : 순간이동"); attroff(COLOR_PAIR(8));
    attron(COLOR_PAIR(9));  mvprintw(cy + 4, cx - 9, "  >> Speed   : 속도 증가");attroff(COLOR_PAIR(9));
    attron(COLOR_PAIR(10)); mvprintw(cy + 5, cx - 9, "  << Slow    : 속도 감소");attroff(COLOR_PAIR(10));
    attron(COLOR_PAIR(11)); mvprintw(cy + 6, cx - 9, "  ** Shield  : 충돌 1회 방어");attroff(COLOR_PAIR(11));
    attron(COLOR_PAIR(12)); mvprintw(cy + 7, cx - 9, "  @@ Ghost   : 자체 통과");attroff(COLOR_PAIR(12));
    attron(COLOR_PAIR(13)); mvprintw(cy + 8, cx - 9, "  %% Mirror  : 좌우 반전");attroff(COLOR_PAIR(13));
    attron(COLOR_PAIR(14) | A_BOLD); mvprintw(cy + 9, cx - 9, "  && Reverse : 몸 뒤집기");attroff(COLOR_PAIR(14) | A_BOLD);

    attron(COLOR_PAIR(3));
    mvprintw(cy + 10, cx - 14, "────────────────────────────────");
    attroff(COLOR_PAIR(3));

    if (highScore > 0) {
        attron(COLOR_PAIR(4));
        mvprintw(cy + 11, cx - 9, "  Best Score: %d", highScore);
        attroff(COLOR_PAIR(4));
    }

    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(cy + 13, cx - 14, "  SPACE BAR 를 눌러 시작하세요  ");
    attroff(COLOR_PAIR(6) | A_BOLD);

    refresh();
    nodelay(stdscr, FALSE);
    while (getch() != ' ') {}
    clear();
    refresh();
}

// ── main ─────────────────────────────────────────────────────────
int main() {
    srand((unsigned)time(nullptr));

    // 한글/UTF-8 출력을 위해 initscr() 이전에 반드시 호출
    setlocale(LC_ALL, "");

    initscr();
    noecho();
    cbreak();
    curs_set(0);
    start_color();

    // ── 색상 쌍 정의 (14쌍) ──────────────────────────────────────
    init_pair(1,  COLOR_BLACK,   COLOR_BLACK);    // 미사용
    init_pair(2,  COLOR_WHITE,   COLOR_BLUE);     // Wall
    init_pair(3,  COLOR_WHITE,   COLOR_WHITE);    // Immune Wall
    init_pair(4,  COLOR_BLACK,   COLOR_YELLOW);   // Snake Head
    init_pair(5,  COLOR_BLACK,   COLOR_GREEN);    // Snake Body
    init_pair(6,  COLOR_GREEN,   COLOR_BLACK);    // Growth Item
    init_pair(7,  COLOR_RED,     COLOR_BLACK);    // Poison Item
    init_pair(8,  COLOR_MAGENTA, COLOR_MAGENTA);  // Gate
    init_pair(9,  COLOR_CYAN,    COLOR_BLACK);    // Speed  >>
    init_pair(10, COLOR_WHITE,   COLOR_CYAN);     // Slow   <<
    init_pair(11, COLOR_YELLOW,  COLOR_BLACK);    // Shield **
    init_pair(12, COLOR_MAGENTA, COLOR_BLACK);    // Ghost  @@
    init_pair(13, COLOR_RED,     COLOR_YELLOW);   // Mirror %%
    init_pair(14, COLOR_MAGENTA, COLOR_BLACK);    // Reverse &&

    int highScore = loadHighScore();

    Board board;
    keypad(board.getWinMap(), TRUE);   // 방향키 입력을 winMap 에서 수신

    showStartScreen(highScore);

    bool restart = true;
    while (restart) {
        restart = false;

        int stagesCleared = 0;
        bool gameQuit     = false;
        int  totalGrowth  = 0;
        int  totalGate    = 0;

        for (int stage = 0; stage < 4 && !gameQuit; stage++) {
            board.loadStage(stage);

            Snake   snake;
            Food    food;
            Poison  poison;
            Gate    gate;
            Special special;

            snake.init(board);
            gate.spawn(board);

            int itemTick   = 0;
            int specialTick = 0;
            int timeTick   = 0;
            int elapsedSec = 0;

            int growthCount = 0;
            int poisonCount = 0;
            int gateCount   = 0;

            bool failed  = false;
            bool cleared = false;

            nodelay(board.getWinMap(), TRUE);

            while (!failed && !cleared) {
                // ── 키 입력 ─────────────────────────────────────
                const int key = wgetch(board.getWinMap());

                if (key == 'q' || key == 'Q') {
                    failed = true; gameQuit = true; break;
                }

                if (key == KEY_UP    || key == KEY_DOWN ||
                    key == KEY_LEFT  || key == KEY_RIGHT ||
                    key == 'w' || key == 'W' ||
                    key == 's' || key == 'S' ||
                    key == 'a' || key == 'A' ||
                    key == 'd' || key == 'D') {
                    int dir = -1;
                    if (key == KEY_UP   || key == 'w' || key == 'W') dir = UP;
                    if (key == KEY_DOWN || key == 's' || key == 'S') dir = DOWN;
                    // Mirror 효과: 좌우 키 방향 반전
                    if (key == KEY_LEFT  || key == 'a' || key == 'A')
                        dir = special.isMirrorActive() ? RIGHT : LEFT;
                    if (key == KEY_RIGHT || key == 'd' || key == 'D')
                        dir = special.isMirrorActive() ? LEFT  : RIGHT;

                    snake.setNextDir(dir);
                }

                // ── 틱 대기 ─────────────────────────────────────
                const int tickMs = special.getCurrentTickMs();
                napms(tickMs);

                // ── 경과 시간 집계 ───────────────────────────────
                timeTick++;
                if (timeTick * tickMs >= 1000) {
                    elapsedSec++;
                    timeTick = 0;
                }

                // ── 뱀 이동 ─────────────────────────────────────
                if (!snake.move(board, food, poison, gate, special,
                                growthCount, poisonCount, gateCount)) {
                    failed = true; break;
                }

                // ── 아이템 수명 갱신 ─────────────────────────────
                food.update(board);
                poison.update(board);
                special.updateAll(board);

                // ── Gate 수명 갱신 / 재생성 ──────────────────────
                gate.update(board);
                if (!gate.isActive()) gate.spawn(board);

                // ── 일반 아이템 출현 (15틱마다) ──────────────────
                itemTick++;
                if (itemTick >= 15) {
                    food.spawn(board);
                    poison.spawn(board);
                    itemTick = 0;
                }

                // ── 특수 아이템 출현 (SPECIAL_TICK 마다) ─────────
                specialTick++;
                if (specialTick >= SPECIAL_TICK) {
                    special.spawnAll(board);
                    specialTick = 0;
                }

                // ── 화면 갱신 ────────────────────────────────────
                board.draw();
                board.drawScoreBoard(stage, elapsedSec,
                                     snake.getLength(), snake.getMaxLength(),
                                     growthCount, poisonCount, gateCount,
                                     MISSIONS[stage]);
                board.drawActiveEffects(special.getActiveEffectStr());

                // ── 미션 달성 확인 ────────────────────────────────
                if (snake.checkMission(MISSIONS[stage],
                                       growthCount, poisonCount, gateCount))
                    cleared = true;
            }

            totalGrowth += growthCount;
            totalGate   += gateCount;

            // ── 스테이지 결과 처리 ────────────────────────────────
            if (failed && !gameQuit) {
                board.showMessage(" ** GAME OVER **  R: 재시작 / Q: 종료 ");
                nodelay(board.getWinMap(), FALSE);
                while (true) {
                    const int k = wgetch(board.getWinMap());
                    if (k == 'r' || k == 'R') { restart = true; break; }
                    if (k == 'q' || k == 'Q') break;
                }
                const int finalScore = totalGrowth * 10 + totalGate * 20
                                       + stagesCleared * 100;
                if (finalScore > highScore) {
                    highScore = finalScore;
                    saveHighScore(highScore);
                }
                break;
            }

            if (cleared) {
                stagesCleared++;
                if (stage < 3) {
                    board.showMessage(" Stage Clear!  Next Stage... ");
                } else {
                    const int finalScore = totalGrowth * 10 + totalGate * 20
                                           + stagesCleared * 100;
                    if (finalScore > highScore) {
                        highScore = finalScore;
                        saveHighScore(highScore);
                    }
                    board.showMessage(" ** ALL CLEAR **  Congratulations! ");
                    nodelay(board.getWinMap(), FALSE);
                    wgetch(board.getWinMap());
                }
                napms(300);
            }
        }
    }

    endwin();
    return 0;
}
