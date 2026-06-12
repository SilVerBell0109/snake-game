// main.cpp
// 게임 루프, ncurses 초기화, Stage 흐름 제어, 최고 점수 파일 저장/로드 [가산점]
//
// 조작법: W/A/S/D 이동  Q 종료  R Game Over 후 재시작  SPACE 시작 화면
// 맵 값:  0=빈공간 1=Wall 2=ImmuneWall 3=Head 4=Body
//         5=Growth 6=Poison 7=Gate 8=Speed(가산점)

#include "Common.h"
#include "Board.h"
#include "Snake.h"
#include "Food.h"
#include "Poison.h"
#include "Gate.h"
#include "Speed.h"

#include <ncurses.h>
#include <locale.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>

// ── 점수 계산 ────────────────────────────────────────────────────
static int calcScore(int growth, int gate, int stagesCleared) {
    return growth * 10 + gate * 20 + stagesCleared * 100;
}

// ── 최고 점수 파일 I/O [가산점] ──────────────────────────────────
static int loadHighScore() {
    std::ifstream ifs("highscore.txt");
    if (!ifs.is_open()) return 0;
    int score = 0;
    ifs >> score;
    return score;
}

static void saveHighScore(int score) {
    std::ofstream ofs("highscore.txt");
    if (ofs.is_open()) ofs << score;
}

// ── 시작 화면 ────────────────────────────────────────────────────
static void showStartScreen(int highScore) {
    clear();
    int row, col;
    getmaxyx(stdscr, row, col);
    const int cy = row / 2;
    const int cx = col / 2;

    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(cy - 7, cx - 10, "  S N A K E  G A M E  ");
    attroff(COLOR_PAIR(4) | A_BOLD);

    attron(COLOR_PAIR(3));
    mvprintw(cy - 6, cx - 14, "────────────────────────────────");
    attroff(COLOR_PAIR(3));

    mvprintw(cy - 4, cx - 9, "[ 조작법 ]");
    mvprintw(cy - 3, cx - 9, "  W  :  위로 이동");
    mvprintw(cy - 2, cx - 9, "  S  :  아래로 이동");
    mvprintw(cy - 1, cx - 9, "  A  :  왼쪽으로 이동");
    mvprintw(cy,     cx - 9, "  D  :  오른쪽으로 이동");
    mvprintw(cy + 1, cx - 9, "  Q  :  게임 종료");

    attron(COLOR_PAIR(3));
    mvprintw(cy + 2, cx - 14, "────────────────────────────────");
    attroff(COLOR_PAIR(3));

    mvprintw(cy + 4, cx - 9, "[ 아이템 ]");
    attron(COLOR_PAIR(6));  mvprintw(cy + 5, cx - 9, "  ++ Growth : 길이+1");
    attroff(COLOR_PAIR(6));
    attron(COLOR_PAIR(7));  mvprintw(cy + 6, cx - 9, "  -- Poison : 길이-1");
    attroff(COLOR_PAIR(7));
    attron(COLOR_PAIR(8));  mvprintw(cy + 7, cx - 9, "  GG Gate   : 순간이동");
    attroff(COLOR_PAIR(8));
    attron(COLOR_PAIR(9));  mvprintw(cy + 8, cx - 9, "  ** Speed  : 속도 부스트");
    attroff(COLOR_PAIR(9));

    attron(COLOR_PAIR(3));
    mvprintw(cy + 9, cx - 14, "────────────────────────────────");
    attroff(COLOR_PAIR(3));

    if (highScore > 0) {
        attron(COLOR_PAIR(4));
        mvprintw(cy + 10, cx - 9, "  Best Score: %d", highScore);
        attroff(COLOR_PAIR(4));
    }

    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(cy + 12, cx - 14, "  SPACE BAR 를 눌러 시작하세요  ");
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

    // 한글 출력을 위한 로케일 설정 — initscr() 이전에 반드시 호출
    setlocale(LC_ALL, "");

    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);
    start_color();

    // 색상 쌍 정의
    init_pair(1, COLOR_BLACK,   COLOR_BLACK);    // 미사용
    init_pair(2, COLOR_WHITE,   COLOR_WHITE);    // Wall
    init_pair(3, COLOR_CYAN,    COLOR_CYAN);     // Immune Wall
    init_pair(4, COLOR_YELLOW,  COLOR_YELLOW);   // Snake Head
    init_pair(5, COLOR_GREEN,   COLOR_GREEN);    // Snake Body
    init_pair(6, COLOR_GREEN,   COLOR_BLACK);    // Growth Item
    init_pair(7, COLOR_RED,     COLOR_BLACK);    // Poison Item
    init_pair(8, COLOR_MAGENTA, COLOR_MAGENTA);  // Gate
    init_pair(9, COLOR_BLUE,    COLOR_BLACK);    // Speed Item [가산점]

    int highScore = loadHighScore();

    Board board;
    showStartScreen(highScore);

    bool restart = true;
    while (restart) {
        restart = false;

        int stagesCleared = 0;
        bool gameQuit     = false;

        for (int stage = 0; stage < 4 && !gameQuit; stage++) {
            board.loadStage(stage);

            Snake  snake;
            Food   food;
            Poison poison;
            Gate   gate;
            Speed  speed;

            snake.init(board);
            gate.spawn(board);

            int itemTick      = 0;
            int timeTick      = 0;
            int elapsedSec    = 0;
            int speedBoostTick = 0;

            bool failed  = false;
            bool cleared = false;

            nodelay(board.getWinMap(), TRUE);

            while (!failed && !cleared) {
                // ── 키 입력 ─────────────────────────────────────
                const int key = wgetch(board.getWinMap());
                if (key == 'w' || key == 'W') snake.setNextDir(UP);
                if (key == 's' || key == 'S') snake.setNextDir(DOWN);
                if (key == 'a' || key == 'A') snake.setNextDir(LEFT);
                if (key == 'd' || key == 'D') snake.setNextDir(RIGHT);
                if (key == 'q' || key == 'Q') { failed = true; gameQuit = true; break; }

                // ── 틱 대기 [가산점: 스테이지별 속도 + Speed 부스트] ──
                const int tickMs = (speedBoostTick > 0) ? 100 : STAGE_TICK_MS[stage];
                napms(tickMs);
                if (speedBoostTick > 0) speedBoostTick--;

                // ── 경과 시간 ────────────────────────────────────
                timeTick++;
                // 200ms 기준: 5틱 = 1초. Speed 부스트 중엔 10틱=1초
                if (timeTick * tickMs >= 1000) {
                    elapsedSec++;
                    timeTick = 0;
                }

                // ── 뱀 이동 ─────────────────────────────────────
                if (!snake.move(board, food, poison, gate, speed)) {
                    failed = true;
                    break;
                }

                // Speed 획득 감지
                if (snake.wasSpeedEaten())
                    speedBoostTick = SPEED_BOOST_TICKS;

                // ── 아이템 수명 갱신 ─────────────────────────────
                food.update(board);
                poison.update(board);
                speed.update(board);

                // ── 게이트 수명 갱신 / 재생성 ────────────────────
                gate.update(board);
                if (!gate.isActive()) gate.spawn(board);

                // ── 아이템 출현 (15틱마다 각각 독립 시도) ────────
                itemTick++;
                if (itemTick >= 15) {
                    if (food.getCount()   < ITEM_LIMIT)  food.spawn(board);
                    if (poison.getCount() < ITEM_LIMIT)  poison.spawn(board);
                    if (speed.getCount()  < SPEED_LIMIT) speed.spawn(board);
                    itemTick = 0;
                }

                // ── 화면 갱신 ────────────────────────────────────
                const int curScore = calcScore(snake.getGrowthCount(),
                                               snake.getGateCount(),
                                               stagesCleared);
                board.draw();
                board.drawScoreBoard(stage, elapsedSec,
                                     snake.getLength(), snake.getMaxLength(),
                                     snake.getGrowthCount(), snake.getPoisonCount(),
                                     snake.getGateCount(),
                                     MISSIONS[stage],
                                     curScore, highScore);

                // ── 미션 달성 확인 ────────────────────────────────
                if (snake.checkMission(MISSIONS[stage]))
                    cleared = true;
            }

            // ── 스테이지 결과 처리 ────────────────────────────────
            if (!gameQuit && failed) {
                board.showMessage(" ** GAME OVER **  R: 재시작 / Q: 종료 ");
                nodelay(board.getWinMap(), FALSE);
                while (true) {
                    const int k = wgetch(board.getWinMap());
                    if (k == 'r' || k == 'R') { restart = true; break; }
                    if (k == 'q' || k == 'Q') break;
                }
                // 최고 점수 저장
                const int finalScore = calcScore(snake.getGrowthCount(),
                                                 snake.getGateCount(),
                                                 stagesCleared);
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
                    // ALL CLEAR
                    const int finalScore = calcScore(snake.getGrowthCount(),
                                                     snake.getGateCount(),
                                                     stagesCleared);
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
