#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 600

#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_BLUE 0xA8D2FF
#define COLOR_BLUE1 0x1C3471
#define COLOR_GRAY 0x1f1f1f1f

#define CELL_SIZE 20
#define LINE_WIDTH 2
#define COLUMNS (SCREEN_WIDTH / CELL_SIZE)
#define ROWS (SCREEN_HEIGHT / CELL_SIZE)

#define SOLID_TYPE 1
#define WATER_TYPE 0
#define DELETE_TYPE -1

typedef struct {
    int type;
    double fill_level;
    double pressure;
    int x;
    int y;
} Cell;

double max_pressure;

double min(double x, double y) { return x < y ? x : y; }
double max(double x, double y) { return x > y ? x : y; }

void clear_environment(Cell environment[ROWS * COLUMNS]);
void draw_cell(SDL_Surface *surface, Cell cell);
void init(Cell environment[ROWS * COLUMNS]);
void calculate_pressure(Cell environment[ROWS * COLUMNS]);
void draw_environment(SDL_Surface *surface, Cell environment[ROWS * COLUMNS]);
Uint32 interpolate_color(double pressure, double min_pressure, double max_pressure);

void draw_grid(SDL_Surface *surface) {
    for (int i = 0; i < COLUMNS; i++) {
        SDL_Rect column = (SDL_Rect){i * CELL_SIZE, 0, LINE_WIDTH, SCREEN_HEIGHT};
        SDL_FillRect(surface, &column, COLOR_GRAY);
    }
    for (int j = 0; j < ROWS; j++) {
        SDL_Rect row = (SDL_Rect){0, j * CELL_SIZE, SCREEN_WIDTH, LINE_WIDTH};
        SDL_FillRect(surface, &row, COLOR_GRAY);
    }
}

void draw_cell(SDL_Surface *surface, Cell cell) {
    int pixel_x = cell.x * CELL_SIZE;
    int pixel_y = cell.y * CELL_SIZE;
    SDL_Rect cell_rect = (SDL_Rect){pixel_x, pixel_y, CELL_SIZE, CELL_SIZE};
    SDL_FillRect(surface, &cell_rect, COLOR_BLACK);
    Uint32 COLOR = interpolate_color(cell.pressure, 0.0, max_pressure);
    if (cell.type == WATER_TYPE) {
        int water_height = cell.fill_level * CELL_SIZE;
        int empty_height = CELL_SIZE - water_height;
        SDL_Rect water_rect = (SDL_Rect){
            pixel_x,
            pixel_y + empty_height,
            CELL_SIZE,
            water_height,
        };
        SDL_FillRect(surface, &water_rect, COLOR);
    } else if (cell.type == SOLID_TYPE) {
        SDL_FillRect(surface, &cell_rect, COLOR_WHITE);
    }
}

void simulation_step(Cell environment[ROWS * COLUMNS]) {
    Cell environment_next[ROWS * COLUMNS];

    for (int i = 0; i < ROWS * COLUMNS; i++) {
        environment_next[i] = environment[i];
    }

    // Step 1: Handle downward flow
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            Cell *source = &environment[j + COLUMNS * i];

            if (source->type == WATER_TYPE && source->fill_level > 0) {
                // Check the cell below
                if (i < ROWS - 1) {
                    Cell *below = &environment_next[j + COLUMNS * (i + 1)];
                    if (below->type == WATER_TYPE && below->fill_level < 1) {
                        // Move water downward based on available space
                        double water_to_move = min(source->fill_level, 1 - below->fill_level);
                        environment_next[j + COLUMNS * i].fill_level -= water_to_move;
                        environment_next[j + COLUMNS * (i + 1)].fill_level += water_to_move;
                    }
                }
            }
        }
    }

    // Step 2: Redistribute water horizontally based on pressure
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            Cell *source = &environment[j + COLUMNS * i];

            if (source->type == WATER_TYPE && source->fill_level > 0) {
                // Check left neighbor
                if (j > 0) {
                    Cell *left = &environment_next[(j - 1) + COLUMNS * i];
                    if (left->type == WATER_TYPE) {
                        double pressure_difference = source->fill_level - left->fill_level;
                        double water_to_move = pressure_difference / 2;  // Move half the difference
                        water_to_move = max(0, min(water_to_move, source->fill_level));  // Clamp
                        environment_next[j + COLUMNS * i].fill_level -= water_to_move;
                        environment_next[(j - 1) + COLUMNS * i].fill_level += water_to_move;
                    }
                }

                // Check right neighbor
                if (j < COLUMNS - 1) {
                    Cell *right = &environment_next[(j + 1) + COLUMNS * i];
                    if (right->type == WATER_TYPE) {
                        double pressure_difference = source->fill_level - right->fill_level;
                        double water_to_move = pressure_difference / 2;  // Move half the difference
                        water_to_move = fmax(0, fmin(water_to_move, source->fill_level));  // Clamp
                        environment_next[j + COLUMNS * i].fill_level -= water_to_move;
                        environment_next[(j + 1) + COLUMNS * i].fill_level += water_to_move;
                    }
                }
            }
        }
    }

    // Step 3: Update the current environment with the new state
    for (int i = 0; i < ROWS * COLUMNS; i++) {
        environment[i] = environment_next[i];
    }

    // Step 4: Calculate pressure based on the updated state
    calculate_pressure(environment);
}

void calculate_pressure(Cell environment[ROWS * COLUMNS]) {
    max_pressure = 0.0;

    for (int j = 0; j < COLUMNS; j++) {
        double cumulative_pressure = 0.0;

        for (int i = 0; i < ROWS; i++) {
            Cell *cell = &environment[j + COLUMNS * i];
            if (cell->type == WATER_TYPE) {
                cumulative_pressure += cell->fill_level;
                cell->pressure = cumulative_pressure;

                if (cumulative_pressure > max_pressure) {
                    max_pressure = cumulative_pressure;
                }
            } else {
                cumulative_pressure = 0.0;
            }
        }
    }
}

int main() {
    SDL_Window *window = SDL_CreateWindow("@csld~Liquid Simulation", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    SDL_Surface *surface = SDL_GetWindowSurface(window);

    SDL_Event e;
    int simulation_running = 1;
    int delete_mode = 0;
    int clear_mode = 0;
    int curr_type = SOLID_TYPE;

    Cell environment[ROWS * COLUMNS];
    init(environment);

    while (simulation_running) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                simulation_running = 0;
            }
            if (e.type == SDL_MOUSEMOTION) {
                if (e.motion.state) {
                    int cell_x = e.motion.x / CELL_SIZE;
                    int cell_y = e.motion.y / CELL_SIZE;
                    if (cell_x >= 0 && cell_x < COLUMNS && cell_y >= 0 && cell_y < ROWS) {
                        Cell *target_cell = &environment[cell_x + COLUMNS * cell_y];

                        if (delete_mode) {
                            target_cell->type = WATER_TYPE;
                            target_cell->fill_level = 0;
                        } else if (curr_type == WATER_TYPE) {
                            if (target_cell->type != SOLID_TYPE) {
                                target_cell->type = WATER_TYPE;
                                target_cell->fill_level = 1;
                            }
                        } else if (curr_type == SOLID_TYPE) {
                            target_cell->type = SOLID_TYPE;
                            target_cell->fill_level = 0;
                        }
                    }
                }
            }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_SPACE:
                        curr_type = !curr_type;
                        break;
                    case SDLK_BACKSPACE:
                        delete_mode = !delete_mode;
                        break;
                    case SDLK_c:
                        clear_environment(environment);
                        break;
                }
            }
        }
        simulation_step(environment);
        draw_environment(surface, environment);
        draw_grid(surface);
        SDL_UpdateWindowSurface(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void clear_environment(Cell environment[ROWS * COLUMNS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            environment[i * COLUMNS + j] = (Cell){
                .type = WATER_TYPE,
                .fill_level = 0.0,
                .pressure = 0.0,
                .x = j,
                .y = i,
            };
        }
    }
}

void init(Cell environment[ROWS * COLUMNS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            environment[i * COLUMNS + j] = (Cell){
                WATER_TYPE, 0, 0, j, i,
            };
        }
    }
}

void draw_environment(SDL_Surface *surface, Cell environment[ROWS * COLUMNS]) {
    for (int i = 0; i < ROWS * COLUMNS; i++) {
        draw_cell(surface, environment[i]);
    }
}

Uint32 interpolate_color(double pressure, double min_pressure, double max_pressure) {
    Uint8 light_blue_r = 0xA8;
    Uint8 light_blue_g = 0xD2;
    Uint8 light_blue_b = 0xFF;

    Uint8 dark_blue_r = 0x1C;
    Uint8 dark_blue_g = 0x34;
    Uint8 dark_blue_b = 0x71;

    double normalized_pressure = (pressure - min_pressure) / (max_pressure - min_pressure);
    if (normalized_pressure < 0.0) normalized_pressure = 0.0;
    if (normalized_pressure > 1.0) normalized_pressure = 1.0;

    Uint8 r =
        (Uint8)((1.0 - normalized_pressure) * light_blue_r + normalized_pressure * dark_blue_r);
    Uint8 g =
        (Uint8)((1.0 - normalized_pressure) * light_blue_g + normalized_pressure * dark_blue_g);
    Uint8 b =
        (Uint8)((1.0 - normalized_pressure) * light_blue_b + normalized_pressure * dark_blue_b);

    Uint32 color = (0xFF << 24) | (r << 16) | (g << 8) | b;
    return color;
}