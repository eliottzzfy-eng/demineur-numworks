#include <eadk.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define GRID_W 10
#define GRID_H 10
#define MINES_COUNT 15
#define CELL_SIZE 20
#define OFFSET_X ((320 - (GRID_W * CELL_SIZE)) / 2)
#define OFFSET_Y ((240 - (GRID_H * CELL_SIZE)) / 2)

// Couleurs (RGB565)
static const eadk_color_t COLOR_BLACK  = 0x0000;
static const eadk_color_t COLOR_WHITE  = 0xFFFF;
static const eadk_color_t COLOR_GRAY   = 0xC618;
static const eadk_color_t COLOR_DGRAY  = 0x8410;
static const eadk_color_t COLOR_RED    = 0xF800;
static const eadk_color_t COLOR_BLUE   = 0x001F;
static const eadk_color_t COLOR_CURSOR = 0x07FF; // Cyan

uint8_t grid[GRID_H][GRID_W];  // 0-8: voisins, 9: mine
uint8_t state[GRID_H][GRID_W]; // 0: caché, 1: révélé, 2: drapeau
int cx = 0, cy = 0; // Position du curseur
bool game_over = false;
bool win = false;

// Fonction utilitaire pour dessiner un rectangle de couleur unie
void draw_rect(int x, int y, int w, int h, eadk_color_t color) {
    eadk_rect_t rect = {(uint16_t)x, (uint16_t)y, (uint16_t)w, (uint16_t)h};
    eadk_display_push_rect_uniform(rect, color);
}

// Initialisation de la grille
void init_game() {
    game_over = false;
    win = false;
    cx = 0; cy = 0;
    
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            grid[y][x] = 0;
            state[y][x] = 0;
        }
    }

    // Placement des mines
    int mines_placed = 0;
    while (mines_placed < MINES_COUNT) {
        int r = eadk_random() % (GRID_W * GRID_H);
        int mx = r % GRID_W;
        int my = r / GRID_W;
        
        if (grid[my][mx] != 9) {
            grid[my][mx] = 9;
            mines_placed++;
        }
    }

    // Calcul des chiffres (voisins)
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            if (grid[y][x] == 9) continue;
            int count = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < GRID_W && ny >= 0 && ny < GRID_H) {
                        if (grid[ny][nx] == 9) count++;
                    }
                }
            }
            grid[y][x] = count;
        }
    }
}

// Révélation récursive des cases vides
void reveal(int x, int y) {
    if (x < 0 || x >= GRID_W || y < 0 || y >= GRID_H) return;
    if (state[y][x] != 0) return; // Déjà révélé ou drapeau

    state[y][x] = 1;

    if (grid[y][x] == 0) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx != 0 || dy != 0) reveal(x + dx, y + dy);
            }
        }
    }
}

// Vérification de la condition de victoire
void check_win() {
    int hidden_count = 0;
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            if (state[y][x] != 1) hidden_count++;
        }
    }
    if (hidden_count == MINES_COUNT) {
        game_over = true;
        win = true;
    }
}

// Affichage du jeu
void render() {
    // Fond d'écran
    draw_rect(0, 0, 320, 240, COLOR_BLACK);

    // Dessin de la grille
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            int px = OFFSET_X + x * CELL_SIZE;
            int py = OFFSET_Y + y * CELL_SIZE;

            if (state[y][x] == 0) {
                // Caché
                draw_rect(px, py, CELL_SIZE - 1, CELL_SIZE - 1, COLOR_GRAY);
            } else if (state[y][x] == 2) {
                // Drapeau
                draw_rect(px, py, CELL_SIZE - 1, CELL_SIZE - 1, COLOR_RED);
            } else {
                // Révélé
                draw_rect(px, py, CELL_SIZE - 1, CELL_SIZE - 1, COLOR_DGRAY);
                if (grid[y][x] == 9) {
                    draw_rect(px + 4, py + 4, CELL_SIZE - 9, CELL_SIZE - 9, COLOR_RED); // Mine
                } else if (grid[y][x] > 0) {
                    char num[2];
                    snprintf(num, sizeof(num), "%d", grid[y][x]);
                    eadk_point_t p = {(uint16_t)(px + 6), (uint16_t)(py + 2)};
                    eadk_display_draw_string(num, p, false, COLOR_WHITE, COLOR_DGRAY);
                }
            }
        }
    }

    // Dessin du curseur (cadre cyan)
    int px = OFFSET_X + cx * CELL_SIZE;
    int py = OFFSET_Y + cy * CELL_SIZE;
    draw_rect(px, py, CELL_SIZE - 1, 2, COLOR_CURSOR);
    draw_rect(px, py + CELL_SIZE - 3, CELL_SIZE - 1, 2, COLOR_CURSOR);
    draw_rect(px, py, 2, CELL_SIZE - 1, COLOR_CURSOR);
    draw_rect(px + CELL_SIZE - 3, py, 2, CELL_SIZE - 1, COLOR_CURSOR);

    // Affichage des messages de fin
    if (game_over) {
        eadk_point_t p_msg = {100, 10};
        if (win) {
            eadk_display_draw_string("VICTOIRE !", p_msg, true, COLOR_WHITE, COLOR_BLACK);
        } else {
            eadk_display_draw_string("BOOM ! PERDU !", p_msg, true, COLOR_RED, COLOR_BLACK);
        }
        eadk_point_t p_restart = {60, 220};
        eadk_display_draw_string("Appuie sur OK pour rejouer", p_restart, false, COLOR_GRAY, COLOR_BLACK);
    }
}

int main(int argc, char * argv[]) {
    init_game();
    render();

    eadk_keyboard_state_t last_kb = 0;

    while (1) {
        eadk_keyboard_state_t kb = eadk_keyboard_scan();

        if (kb != last_kb) {
            if (eadk_keyboard_key_down(kb, eadk_key_home) || eadk_keyboard_key_down(kb, eadk_key_back)) {
                break; // Quitter le jeu
            }

            if (!game_over) {
                if (eadk_keyboard_key_down(kb, eadk_key_left) && cx > 0) cx--;
                if (eadk_keyboard_key_down(kb, eadk_key_right) && cx < GRID_W - 1) cx++;
                if (eadk_keyboard_key_down(kb, eadk_key_up) && cy > 0) cy--;
                if (eadk_keyboard_key_down(kb, eadk_key_down) && cy < GRID_H - 1) cy++;

                // Bouton OK pour révéler
                if (eadk_keyboard_key_down(kb, eadk_key_ok)) {
                    if (state[cy][cx] == 0) {
                        if (grid[cy][cx] == 9) {
                            state[cy][cx] = 1;
                            game_over = true; // Perdu
                        } else {
                            reveal(cx, cy);
                            check_win();
                        }
                    }
                }
                
                // Bouton Shift pour placer un drapeau
                if (eadk_keyboard_key_down(kb, eadk_key_shift)) {
                    if (state[cy][cx] == 0) {
                        state[cy][cx] = 2; // Placer drapeau
                    } else if (state[cy][cx] == 2) {
                        state[cy][cx] = 0; // Enlever drapeau
                    }
                }
            } else {
                // Si la partie est finie, OK pour recommencer
                if (eadk_keyboard_key_down(kb, eadk_key_ok)) {
                    init_game();
                }
            }
            
            render();
        }
        last_kb = kb;
        eadk_timing_msleep(20); // Anti-rebond et économie d'énergie
    }

    return 0;
}
