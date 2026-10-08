# Santorini (230 version) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A single-file C99 program `src/Santorini.c` that plays the 230-version Santorini rules, human versus a deterministic greedy AI, proven by unit tests and end-to-end transcripts.

**Architecture:** One translation unit with zero globals: an `int board[6][6]` of levels, two `int[2]` builder positions, pure helper functions for geometry, ray updates, end-state detection and AI scoring, and a thin `main` that alternates turns. Unit tests include the `.c` file directly (main renamed) so every `static` function is reachable; end-to-end tests feed stdin scripts to the built binary and diff the transcript.

**Tech Stack:** C99, gcc 16 with `-Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wconversion -Wvla -Werror`, GNU make 4.4, bash, `gcc -fanalyzer`. No external libraries, no sanitizers, no valgrind.

**Spec:** `docs/spec.md` (course spec) and `docs/design.md` (decisions). Conventions: `docs/conventions.md`.

## Global Constraints

- Deliverables: `src/Santorini.c` and `README.txt` (spec "Submission"); `make dist` bundles them flat.
- Only first-week constructs in `src/Santorini.c`: arrays, multi-dim arrays, functions taking arrays, loops, `if`/`switch`, `scanf`, `printf`, `const`, `static`. "No use of disallowed C constructs such as structs or pointers." No `struct`, no `enum`, no pointer variables, no `malloc`. The only `&` is inside `scanf`.
- Zero global variables ("no global variables is the best").
- Board is `int board[BOARD_SIZE][BOARD_SIZE]`, `BOARD_SIZE` is 6; levels are 0..4 at all times, start at 2.
- Input is `row column` (e.g. `2 4`), 1-based; invalid input reprompts; EOF exits cleanly.
- Board printed after every move by either side, plus the final board and `Player wins!` / `AI wins!` / `Draw!`.
- AI is deterministic (no `rand`, no `time`): greedy one-ply as in `docs/design.md` section 3.4.
- Every function has a header comment; no narration inside bodies; `static` on every function except `main`.
- Compiles with `gcc -std=c99 -Wall` alone (the course VM) and with the full warning set as errors.
- Commits: conventional-commit subjects, one per task, never push, no AI-assistance mentions anywhere.

## Review Focus

1. A ray blocked by the other builder must stop there and the landing space must never change level (spec example P (1,2)->(2,3) with A at (1,3)). Pinned in Task 4 (`test_spec_example_after_move_to_2_3`, `test_ray_stops_before_blocker`, `test_landing_space_unchanged`).
2. Letters or a partial line (`a b`, `2 x`) must reprompt, not loop forever or consume the next line. Pinned in Task 7 (`test_read_coordinates_rejects_letters`, `test_read_coordinates_partial_line_does_not_leak`) and Task 8 (`invalid_inputs` case).
3. EOF on stdin at the first prompt or mid-game must print a message and exit 0 without printing garbage coordinates. Pinned in Task 8 (`eof_at_start`, `eof_mid_game` cases).
4. Coordinates `0`, `7` and negative numbers must be reported as off the board, not index the array. Pinned in Task 3 (`test_classify_move_off_board`) and Task 8 (`invalid_inputs` case).
5. The AI must never move onto the player or off the board, and must always find a move (every square has at least two free neighbours). Pinned in Task 6 (`test_choose_ai_move_only_legal`, `test_choose_ai_move_corner`).

---

### Task 1: Scaffold the build and test harness

**Files:**
- Create: `Makefile`, `test/unit/check.h`, `test/run_tests.sh`, `test/e2e/transcripts.sh`, `README.txt`, `src/Santorini.c` (stub), `test/unit/test_santorini.c` (stub)

**Interfaces:**
- Produces: `make all|debug|test|check|run|dist|clean`; `build/Santorini`; `build/test/test_santorini`; `test/run_tests.sh` exit status.

- [ ] **Step 1: Write the stub source that only compiles**

`src/Santorini.c`:

```c
/*
 * Santorini (230 version): a human player (P) against a deterministic AI (A)
 * on a 6x6 grid of building levels. See README.txt for the rules and the
 * AI strategy. Rows and columns are 1-based on screen, 0-based in the code.
 */
#include <stdio.h>
#include <stdlib.h>

#define BOARD_SIZE 6

int main(void) {
    return EXIT_SUCCESS;
}
```

- [ ] **Step 2: Write check.h exactly as in `docs/conventions.md` section 6**

- [ ] **Step 3: Write the stub unit test using the include trick**

`test/unit/test_santorini.c`:

```c
#define main program_main
int program_main(void);
#include "../../src/Santorini.c"
#undef main
#include "check.h"

int main(void) {
    CHECK(BOARD_SIZE == 6);
    CHECK_REPORT("test_santorini");
}
```

- [ ] **Step 4: Write the Makefile**

Targets `all debug test check run dist clean unit-tests`; pattern rules; `-MMD -MP`; `CC ?= gcc`; the dist Makefile generated with `$(file ...)`; each flag group and target commented (config files are exempt from the comment rule). See the Makefile in the repository for the exact text.

- [ ] **Step 5: Write test/run_tests.sh and test/e2e/transcripts.sh**

`run_tests.sh`: `set -euo pipefail`; cd to repo root; build unit binaries if missing (`make -s unit-tests`); run each `build/test/test_*`, print `PASS`/`FAIL`; run each `test/e2e/*.sh`, print `PASS`/`FAIL`; exit 1 on any failure.

`transcripts.sh`: for every `test/e2e/cases/*.in`, run `timeout 5 build/Santorini < case.in > build/e2e/name.out`, require exit 0, `diff -u` against `cases/name.expected`. With no cases present it passes.

- [ ] **Step 6: Write README.txt skeleton** (overview placeholder, build/run, "Requirements map" heading, `Video: <VIDEO URL TO BE ADDED>`).

- [ ] **Step 7: Run the gate**

Run: `make clean && make && make check && make test && make dist`
Expected: all succeed; `test_santorini: 1 checks, 0 failures`.

- [ ] **Step 8: Commit**

```bash
git add Makefile test README.txt src/Santorini.c
git commit -m "build: scaffold Makefile, test harness and README skeleton"
```

---

### Task 2: Board initialisation, level counting and display

**Files:**
- Modify: `src/Santorini.c`
- Test: `test/unit/test_santorini.c`

**Interfaces:**
- Produces:
  - `#define LEVEL_MIN 0`, `LEVEL_MAX 4`, `START_LEVEL 2`, `ROW 0`, `COL 1`, `OFF_BOARD (-1)`, `PLAYER_SYMBOL 'P'`, `AI_SYMBOL 'A'`
  - `static void initialize_board(int board[][BOARD_SIZE]);`
  - `static int count_level(int board[][BOARD_SIZE], int level);`
  - `static int is_occupied_by(const int builder[2], int row, int col);`
  - `static char cell_character(int board[][BOARD_SIZE], int row, int col, const int player[2], const int ai[2]);`
  - `static void print_board(int board[][BOARD_SIZE], const int player[2], const int ai[2]);`
  - `static void print_score(int board[][BOARD_SIZE]);`
  - `static void show_state(int board[][BOARD_SIZE], const int player[2], const int ai[2]);`

- [ ] **Step 1: Write the failing tests**

```c
static void test_initialize_board_sets_every_space_to_start_level(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    initialize_board(board);
    CHECK_EQ_INT(count_level(board, START_LEVEL), BOARD_SIZE * BOARD_SIZE);
    CHECK_EQ_INT(count_level(board, LEVEL_MAX), 0);
}

static void test_cell_character_shows_builders_and_levels(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int player[2] = {0, 1};
    int ai[2] = {0, 2};
    initialize_board(board);
    board[5][5] = 4;
    CHECK_EQ_INT(cell_character(board, 0, 1, player, ai), 'P');
    CHECK_EQ_INT(cell_character(board, 0, 2, player, ai), 'A');
    CHECK_EQ_INT(cell_character(board, 0, 0, player, ai), '2');
    CHECK_EQ_INT(cell_character(board, 5, 5, player, ai), '4');
}

static void test_print_board_matches_spec_layout(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int player[2] = {0, 1};
    int ai[2] = {0, 2};
    char text[512] = "";
    initialize_board(board);
    capture_stdout_begin();
    print_board(board, player, ai);
    capture_stdout_end(text, sizeof text);
    CHECK_EQ_STR(text,
                 "   1 2 3 4 5 6\n"
                 "1  2 P A 2 2 2\n"
                 "2  2 2 2 2 2 2\n"
                 "3  2 2 2 2 2 2\n"
                 "4  2 2 2 2 2 2\n"
                 "5  2 2 2 2 2 2\n"
                 "6  2 2 2 2 2 2\n");
}
```

`capture_stdout_begin`/`capture_stdout_end` are test-only helpers in the test file: `freopen` a file under `build/test/` as `stdout`, then read it back with `fopen`/`fread`.

- [ ] **Step 2: Run to verify failure**

Run: `make test`
Expected: compile error, `initialize_board` undeclared.

- [ ] **Step 3: Implement**

```c
/* Sets every space of the board to the starting level. */
static void initialize_board(int board[][BOARD_SIZE]) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            board[row][col] = START_LEVEL;
        }
    }
}

/* Counts the spaces whose building level equals level. */
static int count_level(int board[][BOARD_SIZE], int level) {
    int count = 0;
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if (board[row][col] == level) {
                count++;
            }
        }
    }
    return count;
}

/* Reports whether the builder stands on (row, col). */
static int is_occupied_by(const int builder[2], int row, int col) {
    return builder[ROW] == row && builder[COL] == col;
}

/* Gives the character shown for one space: a builder symbol or the level digit. */
static char cell_character(int board[][BOARD_SIZE], int row, int col,
                           const int player[2], const int ai[2]) {
    if (is_occupied_by(player, row, col)) {
        return PLAYER_SYMBOL;
    }
    if (is_occupied_by(ai, row, col)) {
        return AI_SYMBOL;
    }
    return (char)('0' + board[row][col]);
}

/* Prints the board in the spec layout: column labels, then one row per line. */
static void print_board(int board[][BOARD_SIZE], const int player[2], const int ai[2]) {
    printf("  ");
    for (int col = 0; col < BOARD_SIZE; col++) {
        printf(" %d", col + 1);
    }
    printf("\n");
    for (int row = 0; row < BOARD_SIZE; row++) {
        printf("%d ", row + 1);
        for (int col = 0; col < BOARD_SIZE; col++) {
            printf(" %c", cell_character(board, row, col, player, ai));
        }
        printf("\n");
    }
}

/* Prints the two counts that decide the game. */
static void print_score(int board[][BOARD_SIZE]) {
    printf("Level-4 spaces (Player): %d   Level-0 spaces (AI): %d\n\n",
           count_level(board, LEVEL_MAX), count_level(board, LEVEL_MIN));
}

/* Prints the board followed by the score line. */
static void show_state(int board[][BOARD_SIZE], const int player[2], const int ai[2]) {
    print_board(board, player, ai);
    print_score(board);
}
```

- [ ] **Step 4: Run to verify pass** — `make test`, expected `0 failures`.

- [ ] **Step 5: Commit** — `git commit -m "feat: board initialisation, level counting and spec-layout display"`

---

### Task 3: Geometry and move legality

**Files:** Modify `src/Santorini.c`; test `test/unit/test_santorini.c`.

**Interfaces:**
- Produces: `#define MOVE_OK 0`, `MOVE_OFF_BOARD 1`, `MOVE_NOT_ADJACENT 2`, `MOVE_OCCUPIED 3`;
  `static int is_on_board(int row, int col);`
  `static int is_adjacent(int from_row, int from_col, int to_row, int to_col);`
  `static int classify_move(const int from[2], int to_row, int to_col, const int other[2]);`

- [ ] **Step 1: Write the failing tests**

```c
static void test_is_on_board(void) {
    CHECK(is_on_board(0, 0));
    CHECK(is_on_board(5, 5));
    CHECK(!is_on_board(-1, 0));
    CHECK(!is_on_board(0, 6));
    CHECK(!is_on_board(6, 0));
}

static void test_is_adjacent_is_king_move(void) {
    CHECK(is_adjacent(2, 2, 1, 1));
    CHECK(is_adjacent(2, 2, 3, 2));
    CHECK(is_adjacent(2, 2, 2, 3));
    CHECK(!is_adjacent(2, 2, 2, 2));
    CHECK(!is_adjacent(2, 2, 4, 2));
    CHECK(!is_adjacent(0, 1, 4, 5));
}

static void test_classify_move_off_board(void) {
    int from[2] = {0, 0};
    int other[2] = {5, 5};
    CHECK_EQ_INT(classify_move(from, -1, 0, other), MOVE_OFF_BOARD);
    CHECK_EQ_INT(classify_move(from, 0, 6, other), MOVE_OFF_BOARD);
    CHECK_EQ_INT(classify_move(from, -2, -2, other), MOVE_OFF_BOARD);
}

static void test_classify_move_spec_example(void) {
    int player[2] = {1, 2};
    int ai[2] = {0, 2};
    CHECK_EQ_INT(classify_move(player, 0, 1, ai), MOVE_OK);
    CHECK_EQ_INT(classify_move(player, 2, 3, ai), MOVE_OK);
    CHECK_EQ_INT(classify_move(player, 0, 2, ai), MOVE_OCCUPIED);
    CHECK_EQ_INT(classify_move(player, 1, 2, ai), MOVE_NOT_ADJACENT);
    CHECK_EQ_INT(classify_move(player, 4, 5, ai), MOVE_NOT_ADJACENT);
}
```

- [ ] **Step 2: Run to verify failure** — compile error.

- [ ] **Step 3: Implement**

```c
/* Reports whether (row, col) is inside the 0-based board. */
static int is_on_board(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

/* Reports whether the two spaces are distinct octagonal (king-move) neighbours. */
static int is_adjacent(int from_row, int from_col, int to_row, int to_col) {
    int row_distance = abs(to_row - from_row);
    int col_distance = abs(to_col - from_col);
    return row_distance <= 1 && col_distance <= 1 && row_distance + col_distance > 0;
}

/* Judges a move of the builder at from to (to_row, to_col) when the other builder
 * stands at other. Returns MOVE_OK or the first failing MOVE_* reason. */
static int classify_move(const int from[2], int to_row, int to_col, const int other[2]) {
    if (!is_on_board(to_row, to_col)) {
        return MOVE_OFF_BOARD;
    }
    if (!is_adjacent(from[ROW], from[COL], to_row, to_col)) {
        return MOVE_NOT_ADJACENT;
    }
    if (is_occupied_by(other, to_row, to_col)) {
        return MOVE_OCCUPIED;
    }
    return MOVE_OK;
}
```

- [ ] **Step 4: Run to verify pass** — `make test`.
- [ ] **Step 5: Commit** — `git commit -m "feat: adjacency and move legality with reason codes"`

---

### Task 4: Ray updates after a move

**Files:** Modify `src/Santorini.c`; test `test/unit/test_santorini.c`.

**Interfaces:**
- Produces: `#define PLAYER_DELTA 1`, `AI_DELTA (-1)`;
  `static int clamp_level(int level);`
  `static void update_ray(int board[][BOARD_SIZE], int row, int col, int row_step, int col_step, int delta, const int blocker[2]);`
  `static void update_rays(int board[][BOARD_SIZE], int row, int col, int delta, const int blocker[2]);`
  `static void move_builder(int board[][BOARD_SIZE], int builder[2], int to_row, int to_col, int delta, const int other[2]);`

- [ ] **Step 1: Write the failing tests**

```c
static void test_clamp_level(void) {
    CHECK_EQ_INT(clamp_level(-1), 0);
    CHECK_EQ_INT(clamp_level(0), 0);
    CHECK_EQ_INT(clamp_level(4), 4);
    CHECK_EQ_INT(clamp_level(5), 4);
}

static void test_ray_stops_at_edge_and_skips_origin(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int nobody[2] = {OFF_BOARD, OFF_BOARD};
    initialize_board(board);
    update_ray(board, 2, 2, 0, 1, PLAYER_DELTA, nobody);
    CHECK_EQ_INT(board[2][2], 2);
    CHECK_EQ_INT(board[2][3], 3);
    CHECK_EQ_INT(board[2][4], 3);
    CHECK_EQ_INT(board[2][5], 3);
    CHECK_EQ_INT(count_level(board, 3), 3);
}

static void test_ray_stops_before_blocker(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int blocker[2] = {2, 4};
    initialize_board(board);
    update_ray(board, 2, 2, 0, 1, PLAYER_DELTA, blocker);
    CHECK_EQ_INT(board[2][3], 3);
    CHECK_EQ_INT(board[2][4], 2);
    CHECK_EQ_INT(board[2][5], 2);
}

static void test_spec_example_after_move_to_2_3(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int player[2] = {0, 1};
    int ai[2] = {0, 2};
    int expected[BOARD_SIZE][BOARD_SIZE] = {
        {2, 3, 2, 3, 2, 2},
        {3, 3, 2, 3, 3, 3},
        {2, 3, 3, 3, 2, 2},
        {3, 2, 3, 2, 3, 2},
        {2, 2, 3, 2, 2, 3},
        {2, 2, 3, 2, 2, 2},
    };
    initialize_board(board);
    move_builder(board, player, 1, 2, PLAYER_DELTA, ai);
    CHECK_EQ_INT(player[ROW], 1);
    CHECK_EQ_INT(player[COL], 2);
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            CHECK_EQ_INT(board[row][col], expected[row][col]);
        }
    }
}

static void test_blocker_shields_rest_of_row_and_landing_unchanged(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int player[2] = {1, 2};
    int ai[2] = {0, 2};
    initialize_board(board);
    board[0][1] = 2;
    move_builder(board, player, 0, 1, PLAYER_DELTA, ai);
    CHECK_EQ_INT(board[0][1], 2);
    CHECK_EQ_INT(board[0][2], 2);
    CHECK_EQ_INT(board[0][3], 2);
    CHECK_EQ_INT(board[0][5], 2);
    CHECK_EQ_INT(board[0][0], 3);
    CHECK_EQ_INT(board[1][2], 3);
    CHECK_EQ_INT(board[5][1], 3);
}

static void test_ai_delta_lowers_and_clamps(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int ai[2] = {0, 0};
    int player[2] = {5, 5};
    initialize_board(board);
    board[0][1] = 0;
    board[1][1] = 4;
    update_rays(board, 0, 0, AI_DELTA, player);
    CHECK_EQ_INT(board[0][1], 0);
    CHECK_EQ_INT(board[1][1], 3);
    CHECK_EQ_INT(board[1][0], 1);
    CHECK_EQ_INT(board[4][4], 1);
    CHECK_EQ_INT(board[5][5], 2);
    CHECK_EQ_INT(count_level(board, LEVEL_MIN), 1);
}
```

- [ ] **Step 2: Run to verify failure** — compile error.

- [ ] **Step 3: Implement**

```c
/* Pins a level into the legal range LEVEL_MIN..LEVEL_MAX. */
static int clamp_level(int level) {
    if (level < LEVEL_MIN) {
        return LEVEL_MIN;
    }
    if (level > LEVEL_MAX) {
        return LEVEL_MAX;
    }
    return level;
}

/* Changes by delta every space on one ray from (row, col), excluding (row, col),
 * stopping at the board edge or just before the blocker. */
static void update_ray(int board[][BOARD_SIZE], int row, int col, int row_step, int col_step,
                       int delta, const int blocker[2]) {
    int ray_row = row + row_step;
    int ray_col = col + col_step;
    while (is_on_board(ray_row, ray_col) && !is_occupied_by(blocker, ray_row, ray_col)) {
        board[ray_row][ray_col] = clamp_level(board[ray_row][ray_col] + delta);
        ray_row += row_step;
        ray_col += col_step;
    }
}

/* Applies update_ray in all eight octagonal directions from (row, col). */
static void update_rays(int board[][BOARD_SIZE], int row, int col, int delta, const int blocker[2]) {
    for (int row_step = -1; row_step <= 1; row_step++) {
        for (int col_step = -1; col_step <= 1; col_step++) {
            if (row_step != 0 || col_step != 0) {
                update_ray(board, row, col, row_step, col_step, delta, blocker);
            }
        }
    }
}

/* Moves the builder to (to_row, to_col) and updates the levels along its rays. */
static void move_builder(int board[][BOARD_SIZE], int builder[2], int to_row, int to_col,
                         int delta, const int other[2]) {
    builder[ROW] = to_row;
    builder[COL] = to_col;
    update_rays(board, to_row, to_col, delta, other);
}
```

- [ ] **Step 4: Run to verify pass** — `make test`.
- [ ] **Step 5: Commit** — `git commit -m "feat: octagonal ray updates with edge and builder blocking"`

---

### Task 5: End of game detection

**Interfaces:**
- Produces: `#define WIN_COUNT 10`, `RESULT_NONE 0`, `RESULT_PLAYER 1`, `RESULT_AI 2`, `RESULT_DRAW 3`; `static int game_result(int board[][BOARD_SIZE]);`

- [ ] **Step 1: Write the failing tests**

```c
static void fill_count(int board[][BOARD_SIZE], int level, int how_many) {
    for (int index = 0; index < how_many; index++) {
        board[index / BOARD_SIZE][index % BOARD_SIZE] = level;
    }
}

static void test_game_result(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    initialize_board(board);
    CHECK_EQ_INT(game_result(board), RESULT_NONE);
    fill_count(board, LEVEL_MAX, 9);
    CHECK_EQ_INT(game_result(board), RESULT_NONE);
    fill_count(board, LEVEL_MAX, 10);
    CHECK_EQ_INT(game_result(board), RESULT_PLAYER);
    initialize_board(board);
    fill_count(board, LEVEL_MIN, 10);
    CHECK_EQ_INT(game_result(board), RESULT_AI);
    for (int index = 10; index < 20; index++) {
        board[index / BOARD_SIZE][index % BOARD_SIZE] = LEVEL_MAX;
    }
    CHECK_EQ_INT(game_result(board), RESULT_DRAW);
}
```

- [ ] **Step 2: Run to verify failure** — compile error.

- [ ] **Step 3: Implement**

```c
/* Decides the end state: RESULT_PLAYER with WIN_COUNT spaces at LEVEL_MAX,
 * RESULT_AI with WIN_COUNT at LEVEL_MIN, RESULT_DRAW with both, else RESULT_NONE. */
static int game_result(int board[][BOARD_SIZE]) {
    int player_won = count_level(board, LEVEL_MAX) >= WIN_COUNT;
    int ai_won = count_level(board, LEVEL_MIN) >= WIN_COUNT;
    if (player_won && ai_won) {
        return RESULT_DRAW;
    }
    if (player_won) {
        return RESULT_PLAYER;
    }
    if (ai_won) {
        return RESULT_AI;
    }
    return RESULT_NONE;
}
```

- [ ] **Step 4: Run to verify pass** — `make test`.
- [ ] **Step 5: Commit** — `git commit -m "feat: win, loss and draw detection"`

---

### Task 6: The greedy AI

**Interfaces:**
- Produces: `#define SCORE_TO_ZERO 10000`, `SCORE_FROM_FOUR 100`;
  `static void copy_board(int source[][BOARD_SIZE], int destination[][BOARD_SIZE]);`
  `static int score_ai_move(int board[][BOARD_SIZE], int to_row, int to_col, const int player[2]);`
  `static void choose_ai_move(int board[][BOARD_SIZE], const int ai[2], const int player[2], int chosen[2]);`
  `static void choose_ai_start(const int player[2], int chosen[2]);`

- [ ] **Step 1: Write the failing tests**

```c
static void test_score_ai_move_keys(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int player[2] = {5, 5};
    initialize_board(board);
    board[0][1] = 1;
    board[0][2] = 4;
    board[1][0] = 0;
    CHECK_EQ_INT(score_ai_move(board, 0, 0, player),
                 SCORE_TO_ZERO * 1 + SCORE_FROM_FOUR * 1 + 15);
}

static void test_score_ai_move_does_not_change_board(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int player[2] = {5, 5};
    initialize_board(board);
    score_ai_move(board, 2, 2, player);
    CHECK_EQ_INT(count_level(board, START_LEVEL), BOARD_SIZE * BOARD_SIZE);
}

static void test_choose_ai_move_prefers_drop_to_zero(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int ai[2] = {2, 2};
    int player[2] = {5, 5};
    int chosen[2];
    initialize_board(board);
    board[0][5] = 1;
    choose_ai_move(board, ai, player, chosen);
    CHECK_EQ_INT(chosen[ROW], 1);
    CHECK_EQ_INT(chosen[COL], 3);
}

static void test_choose_ai_move_ties_go_to_scan_order(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int ai[2] = {2, 2};
    int player[2] = {5, 5};
    int chosen[2];
    initialize_board(board);
    choose_ai_move(board, ai, player, chosen);
    CHECK_EQ_INT(chosen[ROW], 1);
    CHECK_EQ_INT(chosen[COL], 1);
}

static void test_choose_ai_move_only_legal(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int ai[2] = {0, 0};
    int player[2] = {0, 1};
    int chosen[2];
    initialize_board(board);
    choose_ai_move(board, ai, player, chosen);
    CHECK(is_on_board(chosen[ROW], chosen[COL]));
    CHECK(!is_occupied_by(player, chosen[ROW], chosen[COL]));
    CHECK(is_adjacent(ai[ROW], ai[COL], chosen[ROW], chosen[COL]));
}

static void test_choose_ai_start_right_then_left(void) {
    int player[2] = {0, 1};
    int edge[2] = {3, 5};
    int chosen[2];
    choose_ai_start(player, chosen);
    CHECK_EQ_INT(chosen[ROW], 0);
    CHECK_EQ_INT(chosen[COL], 2);
    choose_ai_start(edge, chosen);
    CHECK_EQ_INT(chosen[ROW], 3);
    CHECK_EQ_INT(chosen[COL], 4);
}
```

The scan-order tie test: from (2,2) with a uniform board, (1,1) is the first legal neighbour and every neighbour lowers a different number of spaces, so the test instead must assert the true maximum. Replace the expected values after computing them by hand (see the task notes in the commit) — the rule under test is "first strictly greater wins".

- [ ] **Step 2: Run to verify failure** — compile error.

- [ ] **Step 3: Implement**

```c
/* Copies every level from source into destination. */
static void copy_board(int source[][BOARD_SIZE], int destination[][BOARD_SIZE]) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            destination[row][col] = source[row][col];
        }
    }
}

/* Scores an AI move to (to_row, to_col) by simulating it on a copy of the board:
 * spaces dropped to 0 count most, then 4 -> 3 drops, then any lowered space. */
static int score_ai_move(int board[][BOARD_SIZE], int to_row, int to_col, const int player[2]) {
    int trial[BOARD_SIZE][BOARD_SIZE];
    int dropped_to_zero = 0;
    int dropped_from_four = 0;
    int lowered = 0;
    copy_board(board, trial);
    update_rays(trial, to_row, to_col, AI_DELTA, player);
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if (trial[row][col] < board[row][col]) {
                lowered++;
                if (trial[row][col] == LEVEL_MIN) {
                    dropped_to_zero++;
                }
                if (board[row][col] == LEVEL_MAX) {
                    dropped_from_four++;
                }
            }
        }
    }
    return SCORE_TO_ZERO * dropped_to_zero + SCORE_FROM_FOUR * dropped_from_four + lowered;
}

/* Picks the legal neighbour of ai with the highest score into chosen;
 * ties go to the first candidate in row-major scan order. */
static void choose_ai_move(int board[][BOARD_SIZE], const int ai[2], const int player[2],
                           int chosen[2]) {
    int best_score = -1;
    for (int row = ai[ROW] - 1; row <= ai[ROW] + 1; row++) {
        for (int col = ai[COL] - 1; col <= ai[COL] + 1; col++) {
            if (classify_move(ai, row, col, player) == MOVE_OK) {
                int score = score_ai_move(board, row, col, player);
                if (score > best_score) {
                    best_score = score;
                    chosen[ROW] = row;
                    chosen[COL] = col;
                }
            }
        }
    }
}

/* Starts the AI directly right of the player, or directly left from column 6. */
static void choose_ai_start(const int player[2], int chosen[2]) {
    chosen[ROW] = player[ROW];
    chosen[COL] = player[COL] + 1;
    if (!is_on_board(chosen[ROW], chosen[COL])) {
        chosen[COL] = player[COL] - 1;
    }
}
```

- [ ] **Step 4: Run to verify pass** — `make test`.
- [ ] **Step 5: Commit** — `git commit -m "feat: deterministic greedy AI move and start choice"`

---

### Task 7: Reading `row column` input safely

**Interfaces:**
- Produces: `#define INPUT_END 0`, `INPUT_OK 1`, `INPUT_NOT_NUMBERS 2`;
  `static void discard_rest_of_line(void);`
  `static int read_coordinates(int typed[2]);` (1-based values as typed)

- [ ] **Step 1: Write the failing tests** (the test file substitutes a temporary file for `stdin` with `freopen`)

```c
static void feed_stdin(const char *text) {
    FILE *file = fopen("build/test/stdin.txt", "w");
    fputs(text, file);
    fclose(file);
    freopen("build/test/stdin.txt", "r", stdin);
}

static void test_read_coordinates_reads_two_numbers(void) {
    int typed[2] = {0, 0};
    feed_stdin("2 4\n");
    CHECK_EQ_INT(read_coordinates(typed), INPUT_OK);
    CHECK_EQ_INT(typed[ROW], 2);
    CHECK_EQ_INT(typed[COL], 4);
}

static void test_read_coordinates_rejects_letters_then_reads_next_line(void) {
    int typed[2] = {0, 0};
    feed_stdin("a b\n3 3\n");
    CHECK_EQ_INT(read_coordinates(typed), INPUT_NOT_NUMBERS);
    CHECK_EQ_INT(read_coordinates(typed), INPUT_OK);
    CHECK_EQ_INT(typed[ROW], 3);
}

static void test_read_coordinates_partial_line_does_not_leak(void) {
    int typed[2] = {0, 0};
    feed_stdin("2 x\n5 5 extra words\n");
    CHECK_EQ_INT(read_coordinates(typed), INPUT_NOT_NUMBERS);
    CHECK_EQ_INT(read_coordinates(typed), INPUT_OK);
    CHECK_EQ_INT(typed[ROW], 5);
    CHECK_EQ_INT(typed[COL], 5);
    CHECK_EQ_INT(read_coordinates(typed), INPUT_END);
}

static void test_read_coordinates_reports_end_of_input(void) {
    int typed[2] = {0, 0};
    feed_stdin("");
    CHECK_EQ_INT(read_coordinates(typed), INPUT_END);
}
```

- [ ] **Step 2: Run to verify failure** — compile error.

- [ ] **Step 3: Implement**

```c
/* Throws away the rest of the current input line, including the newline. */
static void discard_rest_of_line(void) {
    int character = getchar();
    while (character != '\n' && character != EOF) {
        character = getchar();
    }
}

/* Reads one "row column" line into typed as 1-based numbers.
 * Returns INPUT_OK, INPUT_NOT_NUMBERS (line was not two numbers) or INPUT_END. */
static int read_coordinates(int typed[2]) {
    int row = OFF_BOARD;
    int col = OFF_BOARD;
    int read_count = scanf("%d %d", &row, &col);
    if (read_count == EOF) {
        return INPUT_END;
    }
    discard_rest_of_line();
    if (read_count != 2) {
        return INPUT_NOT_NUMBERS;
    }
    typed[ROW] = row;
    typed[COL] = col;
    return INPUT_OK;
}
```

- [ ] **Step 4: Run to verify pass** — `make test`.
- [ ] **Step 5: Commit** — `git commit -m "feat: row-column input with line draining and EOF detection"`

---

### Task 8: Prompts, messages, the game loop and the first transcripts

**Files:** Modify `src/Santorini.c`; create `test/e2e/cases/spec_example.in/.expected`, `invalid_inputs.in/.expected`, `eof_at_start.in/.expected`, `eof_mid_game.in/.expected`.

**Interfaces:**
- Produces: `print_welcome`, `explain_invalid_move`, `print_number_hint`, `place_typed_start`, `prompt_player_start`, `apply_typed_move`, `prompt_player_move`, `play_ai_turn`, `end_of_input`, `announce_result`, `main`.

- [ ] **Step 1: Write the failing e2e case `spec_example`**

`cases/spec_example.in`:
```
1 2
2 3
```
`cases/spec_example.expected` contains the welcome text, the initial board, the board with `P` at (1,2) and `A` at (1,3), the prompt, then exactly the spec's board after the move to (2,3):
```
   1 2 3 4 5 6
1  2 3 A 3 2 2
2  3 3 P 3 3 3
3  2 3 3 3 2 2
4  3 2 3 2 3 2
5  2 2 3 2 2 3
6  2 2 3 2 2 2
```
followed by the AI's reply board and the end-of-input message. The expected file is produced by running the program once the task passes and hand-checking the two boards above against the spec; the `transcripts.sh` diff then pins it.

- [ ] **Step 2: Run to verify failure** — `make test` fails in `transcripts.sh` (no output yet).

- [ ] **Step 3: Implement the interaction layer and `main`**

```c
/* Prints the one-time rules summary. */
static void print_welcome(void) {
    printf("Santorini (230 version). You are P, the AI is A.\n");
    printf("Moving raises (you) or lowers (AI) every space on the eight lines from the new space.\n");
    printf("You win with %d spaces at level %d; the AI wins with %d spaces at level %d.\n\n",
           WIN_COUNT, LEVEL_MAX, WIN_COUNT, LEVEL_MIN);
}

/* Tells the player why the 1-based (row, col) was rejected. */
static void explain_invalid_move(int reason, int row, int col) {
    switch (reason) {
    case MOVE_OFF_BOARD:
        printf("Invalid move: (%d, %d) is off the board. Rows and columns run 1 to %d.\n",
               row, col, BOARD_SIZE);
        break;
    case MOVE_NOT_ADJACENT:
        printf("Invalid move: (%d, %d) is not adjacent to your builder.\n", row, col);
        break;
    default:
        printf("Invalid move: (%d, %d) is occupied by the AI's builder.\n", row, col);
        break;
    }
}

/* Tells the player what a well-formed line looks like. */
static void print_number_hint(void) {
    printf("Please enter two numbers separated by a space, for example: 2 4\n");
}

/* Places the player's builder at the typed 1-based start if it is on the board.
 * Returns 1 when placed. */
static int place_typed_start(int player[2], const int typed[2]) {
    int row = typed[ROW] - 1;
    int col = typed[COL] - 1;
    if (!is_on_board(row, col)) {
        explain_invalid_move(MOVE_OFF_BOARD, typed[ROW], typed[COL]);
        return 0;
    }
    player[ROW] = row;
    player[COL] = col;
    return 1;
}

/* Asks for the player's starting space until a valid one is typed.
 * Returns 0 when the input ends first. */
static int prompt_player_start(int player[2]) {
    int typed[2];
    int placed = 0;
    while (!placed) {
        int status;
        printf("Choose the starting space for your builder (row column): ");
        status = read_coordinates(typed);
        if (status == INPUT_END) {
            return 0;
        }
        if (status == INPUT_NOT_NUMBERS) {
            print_number_hint();
        } else {
            placed = place_typed_start(player, typed);
        }
    }
    return 1;
}

/* Applies the typed 1-based move when legal, else explains why not.
 * Returns 1 when the builder moved. */
static int apply_typed_move(int board[][BOARD_SIZE], int player[2], const int ai[2],
                            const int typed[2]) {
    int row = typed[ROW] - 1;
    int col = typed[COL] - 1;
    int reason = classify_move(player, row, col, ai);
    if (reason != MOVE_OK) {
        explain_invalid_move(reason, typed[ROW], typed[COL]);
        return 0;
    }
    move_builder(board, player, row, col, PLAYER_DELTA, ai);
    return 1;
}

/* Asks for the player's move until a legal one is made and shows the result.
 * Returns 0 when the input ends first. */
static int prompt_player_move(int board[][BOARD_SIZE], int player[2], const int ai[2]) {
    int typed[2];
    int moved = 0;
    while (!moved) {
        int status;
        printf("Your move (row column): ");
        status = read_coordinates(typed);
        if (status == INPUT_END) {
            return 0;
        }
        if (status == INPUT_NOT_NUMBERS) {
            print_number_hint();
        } else {
            moved = apply_typed_move(board, player, ai, typed);
        }
    }
    show_state(board, player, ai);
    return 1;
}

/* Lets the AI choose and make its move, then shows the result. */
static void play_ai_turn(int board[][BOARD_SIZE], int ai[2], const int player[2]) {
    int chosen[2];
    choose_ai_move(board, ai, player, chosen);
    move_builder(board, ai, chosen[ROW], chosen[COL], AI_DELTA, player);
    printf("AI moves to (%d, %d).\n", chosen[ROW] + 1, chosen[COL] + 1);
    show_state(board, player, ai);
}

/* Reports that the input ended before the game did. @return the exit status */
static int end_of_input(void) {
    printf("\nEnd of input: the game was abandoned.\n");
    return EXIT_SUCCESS;
}

/* Prints the final line naming the winner, or the draw. */
static void announce_result(int result) {
    switch (result) {
    case RESULT_PLAYER:
        printf("Player wins!\n");
        break;
    case RESULT_AI:
        printf("AI wins!\n");
        break;
    default:
        printf("Draw!\n");
        break;
    }
}

/* Sets up the board and both builders, then alternates turns until the game ends. */
int main(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int player[2] = {OFF_BOARD, OFF_BOARD};
    int ai[2] = {OFF_BOARD, OFF_BOARD};
    int result = RESULT_NONE;

    initialize_board(board);
    print_welcome();
    show_state(board, player, ai);
    if (!prompt_player_start(player)) {
        return end_of_input();
    }
    choose_ai_start(player, ai);
    printf("AI starts at (%d, %d).\n", ai[ROW] + 1, ai[COL] + 1);
    show_state(board, player, ai);

    while (result == RESULT_NONE) {
        if (!prompt_player_move(board, player, ai)) {
            return end_of_input();
        }
        result = game_result(board);
        if (result == RESULT_NONE) {
            play_ai_turn(board, ai, player);
            result = game_result(board);
        }
    }
    announce_result(result);
    return EXIT_SUCCESS;
}
```

- [ ] **Step 4: Generate and hand-check the four transcripts**

`invalid_inputs.in`: `0 0`, `7 1`, `a b`, `1 2` (start), then `1 3` (occupied), `5 6` (not adjacent), `2 x`, `9 9`, `2 3`. The expected file must contain one `off the board` line for `0 0`, `7 1` and `9 9`, one `Please enter two numbers` for `a b` and `2 x`, one `occupied` line, one `not adjacent` line, and then the spec board after (2,3).

`eof_at_start.in`: empty file. Expected: welcome, initial board, prompt, end-of-input line; exit 0.

`eof_mid_game.in`: `1 2` only. Expected ends with the prompt and the end-of-input line; exit 0.

- [ ] **Step 5: Run to verify pass** — `make test`; every transcript diffs clean.
- [ ] **Step 6: Commit** — `git commit -m "feat: prompts, game loop and first end-to-end transcripts"`

---

### Task 9: Complete games and result assertions

**Files:** Create `test/e2e/cases/player_wins.in/.expected`, `test/e2e/cases/ai_wins.in/.expected`, `test/e2e/results.sh`.

- [ ] **Step 1: Write `results.sh`** — for each named case assert with `grep` that the last line equals the expected result line (`Player wins!` / `AI wins!` / `End of input: the game was abandoned.`), that the program exited 0, and that for `player_wins` the final board has at least ten `4` cells (count the digits in the last seven board lines). Fail if any case is missing.

- [ ] **Step 2: Run to verify failure** — `make test` fails: cases missing.

- [ ] **Step 3: Produce the move lists** — write a small driver (`test/e2e/find_games.py`, python3 standard library, run with `python3 -I`) that re-implements only the player-side search: it repeatedly runs `build/Santorini` with a candidate prefix and chooses, by lookahead over the program's own output, moves that lead to `Player wins!`; and a second list where the player always moves to the first legal neighbour so the AI wins. Save the two move lists as the `.in` files and the program output as the `.expected` files. Spot-check by hand that the final boards contain ten 4s or ten 0s.

- [ ] **Step 4: Run to verify pass** — `make test`.
- [ ] **Step 5: Commit** — `git commit -m "test: complete-game transcripts for both outcomes"`

---

### Task 10: README requirements map and docs

**Files:** Modify `README.txt`, `docs/design.md` (if anything changed), `docs/plan.md` (tick boxes).

- [ ] **Step 1: Write README.txt** — overview paragraph, build/run instructions, requirements map (every bullet of the spec's Requirements and Rubric lists with file and function), design notes (AI, draw branch, no structs/pointers/globals), `Video: <VIDEO URL TO BE ADDED>`.
- [ ] **Step 2: Run the gate** — `make clean && make && make check && make test && make dist`.
- [ ] **Step 3: Commit** — `git commit -m "docs: README requirements map and final design notes"`
