void simulation_step(Cell environment[ROWS * COLUMNS]) {
    Cell environment_next[ROWS * COLUMNS];

    // Copy the current environment to the next state
    for (int i = 0; i < ROWS * COLUMNS; i++) {
        environment_next[i] = environment[i];
    }

    // Iterate over all cells
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            Cell *source = &environment[j + COLUMNS * i];

            // Skip if not water or no water in the cell
            if (source->type != WATER_TYPE || source->fill_level <= 0) {
                continue;
            }

            // Try to flow downward
            if (i < ROWS - 1) {
                Cell *down = &environment[j + COLUMNS * (i + 1)];
                if (down->type == WATER_TYPE && down->fill_level < 1) {
                    double water_to_move = fmin(source->fill_level, 1 - down->fill_level);
                    environment_next[j + COLUMNS * i].fill_level -= water_to_move;
                    environment_next[j + COLUMNS * (i + 1)].fill_level += water_to_move;
                    continue;  // Skip horizontal flow if water successfully flows downward
                }
            }

            // If no downward flow, try to flow left
            if (j > 0) {
                Cell *left = &environment[(j - 1) + COLUMNS * i];
                if (left->type == WATER_TYPE && left->fill_level < 1) {
                    double water_to_move = (source->fill_level - left->fill_level) / 2;
                    water_to_move = fmax(0, fmin(water_to_move, source->fill_level));
                    environment_next[j + COLUMNS * i].fill_level -= water_to_move;
                    environment_next[(j - 1) + COLUMNS * i].fill_level += water_to_move;
                }
            }

            // If no downward flow, try to flow right
            if (j < COLUMNS - 1) {
                Cell *right = &environment[(j + 1) + COLUMNS * i];
                if (right->type == WATER_TYPE && right->fill_level < 1) {
                    double water_to_move = (source->fill_level - right->fill_level) / 2;
                    water_to_move = fmax(0, fmin(water_to_move, source->fill_level));
                    environment_next[j + COLUMNS * i].fill_level -= water_to_move;
                    environment_next[(j + 1) + COLUMNS * i].fill_level += water_to_move;
                }
            }
        }
    }

    // Copy the next environment back to the current environment
    for (int i = 0; i < ROWS * COLUMNS; i++) {
        environment[i] = environment_next[i];
    }
}