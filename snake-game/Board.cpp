// Board.cpp
// Board 클래스 구현
// 스테이지 맵 데이터(4개 하드코딩), ncurses 윈도우 생성/파괴,
// 맵 렌더링(COLOR_PAIR), 점수판·미션 현황 출력, 활성 효과 표시, 메시지 표시

#include "Board.h"
#include <ncurses.h>
#include <cstring>

// ── 미션 조건 (extern 정의) ───────────────────────────────────────
const Mission MISSIONS[4] = {
    {  6,  2, 0, 0 },  // Stage 1
    {  7,  3, 0, 1 },  // Stage 2
    {  8,  4, 0, 2 },  // Stage 3
    { 10,  5, 0, 3 },  // Stage 4
};

// ── 스테이지 맵 데이터 ────────────────────────────────────────────
// 2 = Immune Wall(꼭짓점 + 대부분 테두리), 1 = Wall(테두리 중앙 3칸 + 내부), 0 = 빈 공간
// 테두리 Gate 가능 구간: top/bottom col 9-11, left/right row 9-11
static const int STAGE_MAPS[4][MAP_SIZE][MAP_SIZE] = {
    // Stage 1 — 빈 공간 (Border Only)
    {
        {2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2},
    },
    // Stage 2 — 단벽 2개 (가로벽 row6 + 세로벽 col14)
    {
        {2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,1,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2},
    },
    // Stage 3 — 4구석 3×3 기둥
    {
        {2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,2},
        {2,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,2},
        {2,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,2},
        {2,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,2},
        {2,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2},
    },
    // Stage 4 — 십자형(+) 벽 (중앙 개방, col10 세로 / row10 가로)
    {
        {2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
        {2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2},
    }
};

// ── 생성자 / 소멸자 ──────────────────────────────────────────────
Board::Board() {
    memset(label_, 0, sizeof(label_));
    winMap_   = newwin(MAP_SIZE + 2, MAP_SIZE * 2 + 2, 0, 0);
    winBoard_ = newwin(MAP_SIZE + 2, 30, 0, MAP_SIZE * 2 + 3);
}

Board::~Board() {
    delwin(winMap_);
    delwin(winBoard_);
}

// ── 스테이지 로드 ────────────────────────────────────────────────
void Board::loadStage(const int stage) {
    memset(label_, 0, sizeof(label_));
    for (int i = 0; i < MAP_SIZE; i++)
        for (int j = 0; j < MAP_SIZE; j++) {
            baseMap_[i][j] = STAGE_MAPS[stage][i][j];
            map_[i][j]     = STAGE_MAPS[stage][i][j];
        }
}

// ── 셀 접근자 ────────────────────────────────────────────────────
void Board::setCell(const int y, const int x, const int val) {
    map_[y][x] = val;
}

int Board::getCell(const int y, const int x) const {
    return map_[y][x];
}

int Board::getBase(const int y, const int x) const {
    return baseMap_[y][x];
}

void Board::setLabel(const int y, const int x, const char c) {
    label_[y][x] = c;
}

void Board::clearLabel(const int y, const int x) {
    label_[y][x] = 0;
}

WINDOW* Board::getWinMap() const {
    return winMap_;
}

// ── 맵 렌더링 ────────────────────────────────────────────────────
void Board::draw() const {
    werase(winMap_);
    box(winMap_, 0, 0);

    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            wmove(winMap_, i + 1, j * 2 + 1);
            const int cell = map_[i][j];
            switch (cell) {
            case CELL_EMPTY:
                wprintw(winMap_, "  ");
                break;
            case CELL_WALL:
                wattron(winMap_, COLOR_PAIR(2));
                wprintw(winMap_, "##");
                wattroff(winMap_, COLOR_PAIR(2));
                break;
            case CELL_IMMUNE:
                wattron(winMap_, COLOR_PAIR(3));
                wprintw(winMap_, "##");
                wattroff(winMap_, COLOR_PAIR(3));
                break;
            case CELL_HEAD:
                wattron(winMap_, COLOR_PAIR(4));
                wprintw(winMap_, "HH");
                wattroff(winMap_, COLOR_PAIR(4));
                break;
            case CELL_BODY:
                wattron(winMap_, COLOR_PAIR(5));
                wprintw(winMap_, "OO");
                wattroff(winMap_, COLOR_PAIR(5));
                break;
            case CELL_GROWTH:
                wattron(winMap_, COLOR_PAIR(6));
                if (label_[i][j] != 0)
                    wprintw(winMap_, "+%c", label_[i][j]);
                else
                    wprintw(winMap_, "++");
                wattroff(winMap_, COLOR_PAIR(6));
                break;
            case CELL_POISON:
                wattron(winMap_, COLOR_PAIR(7));
                wprintw(winMap_, "--");
                wattroff(winMap_, COLOR_PAIR(7));
                break;
            case CELL_GATE:
                wattron(winMap_, COLOR_PAIR(8));
                wprintw(winMap_, "GG");
                wattroff(winMap_, COLOR_PAIR(8));
                break;
            // ── 특수 아이템 (쌍 9~14) ──────────────────────────
            case CELL_SPEED:    // >> CYAN/BLACK
                wattron(winMap_, COLOR_PAIR(9));
                wprintw(winMap_, ">>");
                wattroff(winMap_, COLOR_PAIR(9));
                break;
            case CELL_SLOW:     // << WHITE/CYAN
                wattron(winMap_, COLOR_PAIR(10));
                wprintw(winMap_, "<<");
                wattroff(winMap_, COLOR_PAIR(10));
                break;
            case CELL_SHIELD:   // ** YELLOW/BLACK
                wattron(winMap_, COLOR_PAIR(11));
                wprintw(winMap_, "**");
                wattroff(winMap_, COLOR_PAIR(11));
                break;
            case CELL_GHOST:    // @@ MAGENTA/BLACK
                wattron(winMap_, COLOR_PAIR(12));
                wprintw(winMap_, "@@");
                wattroff(winMap_, COLOR_PAIR(12));
                break;
            case CELL_MIRROR:   // %% RED/YELLOW
                wattron(winMap_, COLOR_PAIR(13));
                wprintw(winMap_, "%%");
                wattroff(winMap_, COLOR_PAIR(13));
                break;
            case CELL_REVERSE:  // && MAGENTA/BLACK (볼드)
                wattron(winMap_, COLOR_PAIR(14) | A_BOLD);
                wprintw(winMap_, "&&");
                wattroff(winMap_, COLOR_PAIR(14) | A_BOLD);
                break;
            default:
                wprintw(winMap_, "  ");
                break;
            }
        }
    }
    wrefresh(winMap_);
}

// ── 점수판 렌더링 ────────────────────────────────────────────────
void Board::drawScoreBoard(const int stage, const int elapsedSec,
                           const int curLen, const int maxLen,
                           const bool collected[9],
                           const int poison, const int gate,
                           const int score, const int highScore) const {
    werase(winBoard_);
    box(winBoard_, 0, 0);

    mvwprintw(winBoard_,  1, 2, "[ Score Board ]");
    mvwprintw(winBoard_,  2, 2, "Stage  : %d",    stage + 1);
    mvwprintw(winBoard_,  3, 2, "Time   : %ds",   elapsedSec);

    wattron(winBoard_, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(winBoard_,  4, 2, "Score  : %-7d", score);
    wattroff(winBoard_, COLOR_PAIR(4) | A_BOLD);
    wattron(winBoard_, COLOR_PAIR(9));
    mvwprintw(winBoard_,  5, 2, "Best   : %-7d", highScore);
    wattroff(winBoard_, COLOR_PAIR(9));

    int growthCnt = 0;
    for (int i = 0; i < 9; i++) if (collected[i]) growthCnt++;

    mvwprintw(winBoard_,  7, 2, "B: %d / %d",   curLen, maxLen);
    mvwprintw(winBoard_,  8, 2, "+: %d",         growthCnt);
    mvwprintw(winBoard_,  9, 2, "-: %d",         poison);
    mvwprintw(winBoard_, 10, 2, "G: %d",         gate);

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
        const int start = row * 5;
        const int end   = (row == 0) ? 5 : 9;
        for (int i = start; i < end; i++) {
            if (collected[i]) {
                wattron(winBoard_, COLOR_PAIR(6) | A_BOLD);
                wprintw(winBoard_, "+%d ", i + 1);
                wattroff(winBoard_, COLOR_PAIR(6) | A_BOLD);
            } else {
                wattron(winBoard_, COLOR_PAIR(7));
                wprintw(winBoard_, "+%d ", i + 1);
                wattroff(winBoard_, COLOR_PAIR(7));
            }
        }
    }

    wrefresh(winBoard_);
}

// ── 활성 특수 효과 표시 ─────────────────────────────────────────
void Board::drawActiveEffects(const std::string& effects) const {
    mvwprintw(winBoard_, 20, 2, "[ Effects ]");
    mvwprintw(winBoard_, 21, 2, "%-26s", effects.c_str());
    wrefresh(winBoard_);
}

// ── 메시지 표시 ──────────────────────────────────────────────────
void Board::showMessage(const std::string& msg) const {
    const int y = MAP_SIZE / 2 + 1;
    int x = (MAP_SIZE * 2 - (int)msg.size()) / 2 + 1;
    if (x < 1) x = 1;
    mvwprintw(winMap_, y, x, "%s", msg.c_str());
    wrefresh(winMap_);
    napms(1800);
}
