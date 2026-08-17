#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>

#define MAX_X 16
#define MAX_Y 16
#define MAX_MINES 40

typedef struct {
    int x, y;
    int revealed;
    int flagged;
    int mine;
    int neighbor_mines;
} Cell;

typedef struct {
    Cell grid[MAX_Y][MAX_X];
    int total_mines;
    int revealed_count;
    int game_over;
    int win;
} GameState;

void init_grid(GameState *game) {
    for (int y = 0; y < MAX_Y; y++) {
        for (int x = 0; x < MAX_X; x++) {
            game->grid[y][x].x = x;
            game->grid[y][x].y = y;
            game->grid[y][x].revealed = 0;
            game->grid[y][x].flagged = 0;
            game->grid[y][x].mine = 0;
            game->grid[y][x].neighbor_mines = 0;
        }
    }
    game->total_mines = 0;
    game->revealed_count = 0;
    game->game_over = 0;
    game->win = 0;
}

int count_neighbor_mines(GameState *game, int x, int y) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < MAX_X && ny >= 0 && ny < MAX_Y) {
                if (game->grid[ny][nx].mine) {
                    count++;
                }
            }
        }
    }
    return count;
}

void reveal_cell(GameState *game, int x, int y) {
    if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y) return;
    if (game->grid[y][x].revealed || game->grid[y][x].flagged) return;
    
    game->grid[y][x].revealed = 1;
    game->revealed_count++;
    
    if (game->grid[y][x].mine) {
        game->game_over = 1;
        game->win = 0;
        return;
    }
    
    int neighbors = count_neighbor_mines(game, x, y);
    game->grid[y][x].neighbor_mines = neighbors;
    
    if (neighbors == 0) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int nx = x + dx;
                int ny = y + dy;
                reveal_cell(game, nx, ny);
            }
        }
    }
}

void toggle_flag(GameState *game, int x, int y) {
    if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y) return;
    if (game->grid[y][x].revealed) return;
    
    game->grid[y][x].flagged = !game->grid[y][x].flagged;
}

void draw_grid(GameState *game) {
    printf("\n");
    printf("  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15\n");
    printf("   --------------------------------------------------------\n");
    
    for (int y = 0; y < MAX_Y; y++) {
        printf("%d | ", y);
        for (int x = 0; x < MAX_X; x++) {
            if (game->grid[y][x].revealed) {
                if (game->grid[y][x].mine) {
                    printf(" * ");
                } else if (game->grid[y][x].neighbor_mines > 0) {
                    printf(" %d ", game->grid[y][x].neighbor_mines);
                } else {
                    printf(" . ");
                }
            } else if (game->grid[y][x].flagged) {
                printf(" F ");
            } else {
                printf(" ? ");
            }
        }
        printf(" |\n");
    }
    
    printf("   --------------------------------------------------------\n");
    printf("Mines: %d | Revealed: %d/%d | ", game->total_mines, game->revealed_count, MAX_X * MAX_Y - MAX_MINES);
    
    if (game->game_over) {
        if (game->win) {
            printf("VICTOIRE!\n");
        } else {
            printf("DEFAITE!\n");
        }
    }
}

int place_mines(GameState *game, int start_x, int start_y) {
    int mines_placed = 0;
    srand(time(NULL));
    
    while (mines_placed < MAX_MINES) {
        int x = rand() % MAX_X;
        int y = rand() % MAX_Y;
        
        if (!game->grid[y][x].revealed && 
            !(x >= start_x - 1 && x <= start_x + 1 && y >= start_y - 1 && y <= start_y + 1)) {
            
            game->grid[y][x].mine = 1;
            mines_placed++;
            
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < MAX_X && ny >= 0 && ny < MAX_Y && !game->grid[ny][nx].mine) {
                        game->grid[ny][nx].neighbor_mines++;
                    }
                }
            }
        }
    }
    
    game->total_mines = mines_placed;
    return mines_placed;
}

int check_win(GameState *game) {
    int safe_cells = MAX_X * MAX_Y - MAX_MINES;
    return game->revealed_count >= safe_cells;
}

void process_input(GameState *game) {
    printf("Coordonnées (x y) ou 'f x y' pour flaguer, 'q' pour quitter: ");
    char input[30];
    if (!fgets(input, sizeof(input), stdin)) return;
    
    // Remove newline
    input[strcspn(input, "\n")] = 0;
    
    if (input[0] == 'q' || input[0] == 'Q') {
        game->game_over = 1;
        return;
    }
    
    char command = input[0];
    int x, y;
    
    if (command == 'f' || command == 'F') {
        // Flag/unflag command
        if (sscanf(input + 1, " %d %d", &x, &y) == 2) {
            toggle_flag(game, x, y);
        }
    } else {
        // Reveal cell
        if (sscanf(input, "%d %d", &x, &y) == 2) {
            reveal_cell(game, x, y);
            if (game->game_over) {
                game->win = check_win(game);
            }
        }
    }
}

int main() {
    GameState game;
    init_grid(&game);
    
    printf("=== Démineur pour NumWorks ===\n");
    printf("Entrez 'x y' pour révéler une cellule.\n");
    printf("Entrez 'f x y' pour flaguer/déflaguer.\n");
    printf("Entrez 'q' pour quitter.\n\n");
    
    game.game_over = 0;
    
    while (!game.game_over) {
        draw_grid(&game);
        process_input(&game);
        
        if (!game.game_over && game.total_mines == 0) {
            // First reveal - place mines avoiding the starting cell
            // We'll place mines after the first cell reveal in the loop
        }
    }
    
    draw_grid(&game);
    
    if (game.win) {
        printf("\nFélicitations ! Vous avez gagné !\n");
    } else {
        printf("\nDommage ! Vous avez perdu.\n");
        printf("Les mines étaient cachées à ces positions:\n");
        for (int y = 0; y < MAX_Y; y++) {
            for (int x = 0; x < MAX_X; x++) {
                if (game.grid[y][x].mine) {
                    printf("(%d, %d) ", x, y);
                }
            }
        }
        printf("\n");
    }
    
    printf("Merci d'avoir joué ! Appuyez sur une touche pour quitter...\n");
    getch();
    
    return 0;
}
