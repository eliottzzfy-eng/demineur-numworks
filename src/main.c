#include <ion.h>
#include <kandinsky.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define GRID_WIDTH 16
#define GRID_HEIGHT 10
#define TILE_SIZE 20
#define OFFSET_X ((320 - GRID_WIDTH * TILE_SIZE) / 2)
#define OFFSET_Y ((240 - GRID_HEIGHT * TILE_SIZE) / 2)
#define NUM_BOMBS 20

typedef struct {
    bool hasBomb;
    bool isRevealed;
    bool isFlagged;
    int neighborBombs;
} Tile;

Tile grid[GRID_WIDTH][GRID_HEIGHT];
int cursorX = 0, cursorY = 0;
bool gameOver = false;
bool gameWon = false;

void initGame() {
    // Initialize grid
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            grid[x][y].hasBomb = false;
            grid[x][y].isRevealed = false;
            grid[x][y].isFlagged = false;
            grid[x][y].neighborBombs = 0;
        }
    }

    // Place bombs
    int placedBombs = 0;
    while (placedBombs < NUM_BOMBS) {
        int x = rand() % GRID_WIDTH;
        int y = rand() % GRID_HEIGHT;
        if (!grid[x][y].hasBomb) {
            grid[x][y].hasBomb = true;
            placedBombs++;
        }
    }

    // Calculate neighbors
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            if (grid[x][y].hasBomb) continue;
            int count = 0;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT) {
                        if (grid[nx][ny].hasBomb) count++;
                    }
                }
            }
            grid[x][y].neighborBombs = count;
        }
    }
    gameOver = false;
    gameWon = false;
}

void drawTile(int x, int y, bool isCursor) {
    Tile t = grid[x][y];
    kd_color color = KD_COLOR_WHITE;
    char text[2] = {0, 0};

    int px = OFFSET_X + x * TILE_SIZE;
    int py = OFFSET_Y + y * TILE_SIZE;

    if (t.isRevealed) {
        if (t.hasBomb) {
            color = KD_COLOR_RED;
            text[0] = '*';
        } else {
            color = KD_COLOR_LIGHT_GREY;
            if (t.neighborBombs > 0) {
                text[0] = '0' + t.neighborBombs;
            }
        }
    } else {
        color = t.isFlagged ? KD_COLOR_ORANGE : KD_COLOR_DARK_GREY;
        if (t.isFlagged) text[0] = 'F';
    }

    kd_draw_rect(px, py, TILE_SIZE, TILE_SIZE, color);
    kd_draw_rect(px, py, TILE_SIZE, 1, KD_COLOR_BLACK);
    kd_draw_rect(px, py, 1, TILE_SIZE, KD_COLOR_BLACK);
    
    if (text[0] != 0) {
        kd_draw_string(text, px + 6, py + 2, KD_COLOR_BLACK, color);
    }

    if (isCursor) {
        kd_draw_rect(px, py, TILE_SIZE, 2, KD_COLOR_YELLOW);
        kd_draw_rect(px, py + TILE_SIZE - 2, TILE_SIZE, 2, KD_COLOR_YELLOW);
        kd_draw_rect(px, py, 2, TILE_SIZE, KD_COLOR_YELLOW);
        kd_draw_rect(px + TILE_SIZE - 2, py, 2, TILE_SIZE, KD_COLOR_YELLOW);
    }
}

void reveal(int x, int y) {
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT || grid[x][y].isRevealed || grid[x][y].isFlagged) return;

    grid[x][y].isRevealed = true;

    if (grid[x][y].hasBomb) {
        gameOver = true;
        return;
    }

    if (grid[x][y].neighborBombs == 0) {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                reveal(x + dx, y + dy);
            }
        }
    }
}

bool checkWin() {
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            if (!grid[x][y].hasBomb && !grid[x][y].isRevealed) return false;
        }
    }
    return true;
}

int main() {
    srand(ion_get_time());
    initGame();

    while (true) {
        kd_clear_screen(KD_COLOR_WHITE);
        for (int x = 0; x < GRID_WIDTH; x++) {
            for (int y = 0; y < GRID_HEIGHT; y++) {
                drawTile(x, y, (x == cursorX && y == cursorY));
            }
        }

        if (gameOver) {
            kd_draw_string("GAME OVER", 110, 220, KD_COLOR_RED, KD_COLOR_WHITE);
        } else if (gameWon) {
            kd_draw_string("YOU WIN!", 120, 220, KD_COLOR_GREEN, KD_COLOR_WHITE);
        }

        ion_event_t event = ion_get_event();

        if (event == ION_EVENT_UP && cursorY > 0) cursorY--;
        if (event == ION_EVENT_DOWN && cursorY < GRID_HEIGHT - 1) cursorY++;
        if (event == ION_EVENT_LEFT && cursorX > 0) cursorX--;
        if (event == ION_EVENT_RIGHT && cursorX < GRID_WIDTH - 1) cursorX++;

        if (!gameOver && !gameWon) {
            if (event == ION_EVENT_OK) {
                reveal(cursorX, cursorY);
                if (checkWin()) gameWon = true;
            }
            if (event == ION_EVENT_BACK) {
                grid[cursorX][cursorY].isFlagged = !grid[cursorX][cursorY].isFlagged;
            }
        }

        if (event == ION_EVENT_HOME) break;
        if ((gameOver || gameWon) && (event == ION_EVENT_OK || event == ION_EVENT_BACK)) {
            initGame();
        }
    }
    return 0;
}
