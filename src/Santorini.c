/*
 * Santorini (230 version): a human player (P) against a deterministic AI (A)
 * on a 6x6 grid of building levels. See README.txt for the rules and the
 * AI strategy. Rows and columns are 1-based on screen, 0-based in the code.
 */
#include <stdio.h>
#include <stdlib.h>

#define BOARD_SIZE 6
#define LEVEL_MIN 0
#define LEVEL_MAX 4
#define START_LEVEL 2
/* Spaces at LEVEL_MAX (player) or LEVEL_MIN (AI) needed to win. */
#define WIN_COUNT 10

/* Index of the row and the column inside a builder position array. */
#define ROW 0
#define COL 1
/* Position value meaning "this builder is not on the board (yet)". */
#define OFF_BOARD (-1)

#define PLAYER_SYMBOL 'P'
#define AI_SYMBOL 'A'

/* Verdicts of a move request: MOVE_OK or the first rule the move breaks. */
#define MOVE_OK 0
#define MOVE_OFF_BOARD 1
#define MOVE_NOT_ADJACENT 2
#define MOVE_OCCUPIED 3
#define MOVE_SAME_SPACE 4

/* Level change made by a move: the player builds, the AI destroys. */
#define PLAYER_DELTA 1
#define AI_DELTA (-1)

/* End states reported by game_result. */
#define RESULT_NONE 0
#define RESULT_PLAYER 1
#define RESULT_AI 2
#define RESULT_DRAW 3

/* Weights that turn the AI's three move counts into one lexicographic score;
 * no count can exceed the 35 other spaces, so the keys never overlap. */
#define SCORE_TO_ZERO 10000
#define SCORE_FROM_FOUR 100
/* Below every real score (scores are never negative), so the first legal
 * candidate always replaces it. */
#define NO_SCORE (-1)

/* Outcomes of reading one input line. */
#define INPUT_END 0
#define INPUT_OK 1
#define INPUT_NOT_NUMBERS 2

static void initialize_board(int board[][BOARD_SIZE]);
static int count_level(int board[][BOARD_SIZE], int level);
static void copy_board(int source[][BOARD_SIZE], int destination[][BOARD_SIZE]);
static int is_on_board(int row, int col);
static int is_occupied_by(const int builder[2], int row, int col);
static int distance(int from, int to);
static int is_adjacent(int from_row, int from_col, int to_row, int to_col);
static int classify_move(const int from[2], int to_row, int to_col, const int other[2]);
static int clamp_level(int level);
static void update_ray(int board[][BOARD_SIZE], int row, int col, int row_step, int col_step,
                       int delta, const int blocker[2]);
static void update_rays(int board[][BOARD_SIZE], int row, int col, int delta,
                        const int blocker[2]);
static void move_builder(int board[][BOARD_SIZE], int builder[2], int to_row, int to_col,
                         int delta, const int other[2]);
static int game_result(int board[][BOARD_SIZE]);
static int score_ai_move(int board[][BOARD_SIZE], int to_row, int to_col, const int player[2]);
static void choose_ai_move(int board[][BOARD_SIZE], const int ai[2], const int player[2],
                           int chosen[2]);
static void choose_ai_start(const int player[2], int chosen[2]);
static char cell_character(int board[][BOARD_SIZE], int row, int col,
                           const int player[2], const int ai[2]);
static void print_board(int board[][BOARD_SIZE], const int player[2], const int ai[2]);
static void print_score(int board[][BOARD_SIZE]);
static void show_state(int board[][BOARD_SIZE], const int player[2], const int ai[2]);
static void discard_rest_of_line(void);
static int read_coordinates(int typed[2]);
static void print_welcome(void);
static void print_number_hint(void);
static void explain_invalid_move(int reason, int row, int col);
static int place_typed_start(int player[2], const int typed[2]);
static int prompt_player_start(int player[2]);
static int apply_typed_move(int board[][BOARD_SIZE], int player[2], const int ai[2],
                            const int typed[2]);
static int prompt_player_move(int board[][BOARD_SIZE], int player[2], const int ai[2]);
static void play_ai_turn(int board[][BOARD_SIZE], int ai[2], const int player[2]);
static int end_of_input(void);
static void announce_result(int result);

/* ---- Board ---- */

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

/* Copies every level from source into destination. */
static void copy_board(int source[][BOARD_SIZE], int destination[][BOARD_SIZE]) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            destination[row][col] = source[row][col];
        }
    }
}

/* ---- Geometry and move legality ---- */

/* Reports whether (row, col) is inside the 0-based board. */
static int is_on_board(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

/* Reports whether the builder stands on (row, col). */
static int is_occupied_by(const int builder[2], int row, int col) {
    return builder[ROW] == row && builder[COL] == col;
}

/* Gives the non-negative number of steps between two row or column indexes. */
static int distance(int from, int to) {
    return to >= from ? to - from : from - to;
}

/* Reports whether the two spaces are distinct octagonal (king-move) neighbours. */
static int is_adjacent(int from_row, int from_col, int to_row, int to_col) {
    int row_distance = distance(from_row, to_row);
    int col_distance = distance(from_col, to_col);
    return row_distance <= 1 && col_distance <= 1 && row_distance + col_distance > 0;
}

/* Judges a move of the builder at from to (to_row, to_col) while the other
 * builder stands at other. Returns MOVE_OK or the first failing MOVE_* reason. */
static int classify_move(const int from[2], int to_row, int to_col, const int other[2]) {
    if (!is_on_board(to_row, to_col)) {
        return MOVE_OFF_BOARD;
    }
    if (is_occupied_by(from, to_row, to_col)) {
        return MOVE_SAME_SPACE;
    }
    if (!is_adjacent(from[ROW], from[COL], to_row, to_col)) {
        return MOVE_NOT_ADJACENT;
    }
    if (is_occupied_by(other, to_row, to_col)) {
        return MOVE_OCCUPIED;
    }
    return MOVE_OK;
}

/* ---- Building levels ---- */

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

/* Changes by delta every space on one ray out of (row, col), excluding
 * (row, col) itself, stopping at the board edge or just before the blocker. */
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
static void update_rays(int board[][BOARD_SIZE], int row, int col, int delta,
                        const int blocker[2]) {
    for (int row_step = -1; row_step <= 1; row_step++) {
        for (int col_step = -1; col_step <= 1; col_step++) {
            if (row_step != 0 || col_step != 0) {
                update_ray(board, row, col, row_step, col_step, delta, blocker);
            }
        }
    }
}

/* Moves the builder to (to_row, to_col) and updates the levels along its rays,
 * which the other builder may block. */
static void move_builder(int board[][BOARD_SIZE], int builder[2], int to_row, int to_col,
                         int delta, const int other[2]) {
    builder[ROW] = to_row;
    builder[COL] = to_col;
    update_rays(board, to_row, to_col, delta, other);
}

/* ---- End of the game ---- */

/* Decides the end state: RESULT_PLAYER with WIN_COUNT spaces at LEVEL_MAX,
 * RESULT_AI with WIN_COUNT spaces at LEVEL_MIN, RESULT_DRAW with both at
 * once, else RESULT_NONE. */
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

/* ---- The AI ---- */

/* Scores an AI move to (to_row, to_col) by simulating it on a copy of the
 * board: spaces dropped to 0 count most, then 4 -> 3 drops, then any lowered
 * space (see README.txt, "AI strategy"). */
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

/* Picks the legal neighbour of ai with the highest score into chosen; ties go
 * to the first candidate in row-major scan order. Every space has at least
 * two free neighbours, so a move always exists. */
static void choose_ai_move(int board[][BOARD_SIZE], const int ai[2], const int player[2],
                           int chosen[2]) {
    int best_score = NO_SCORE;
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

/* Starts the AI directly right of the player, or directly left when the
 * player chose the last column; one of the two always exists. */
static void choose_ai_start(const int player[2], int chosen[2]) {
    chosen[ROW] = player[ROW];
    chosen[COL] = player[COL] + 1;
    if (!is_on_board(chosen[ROW], chosen[COL])) {
        chosen[COL] = player[COL] - 1;
    }
}

/* ---- Display ---- */

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

/* ---- Input ---- */

/* Throws away the rest of the current input line, including the newline. */
static void discard_rest_of_line(void) {
    int character = getchar();
    while (character != '\n' && character != EOF) {
        character = getchar();
    }
}

/* Reads one "row column" line into typed as the 1-based numbers typed.
 * Returns INPUT_OK, INPUT_NOT_NUMBERS (the line was not two numbers) or
 * INPUT_END (no more input). */
static int read_coordinates(int typed[2]) {
    int row = OFF_BOARD;   /* scanf leaves these untouched when it fails */
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

/* ---- Messages and turns ---- */

/* Prints the one-time rules summary. */
static void print_welcome(void) {
    printf("Santorini (230 version). You are %c, the AI is %c.\n", PLAYER_SYMBOL, AI_SYMBOL);
    printf("Moving raises (you) or lowers (AI) every space on the eight lines "
           "from the new space.\n");
    printf("You win with %d spaces at level %d; the AI wins with %d spaces at level %d.\n\n",
           WIN_COUNT, LEVEL_MAX, WIN_COUNT, LEVEL_MIN);
}

/* Tells the player what a well-formed input line looks like. */
static void print_number_hint(void) {
    printf("Please enter two numbers separated by a space, for example: 2 4\n");
}

/* Tells the player why the 1-based (row, col) was rejected. */
static void explain_invalid_move(int reason, int row, int col) {
    switch (reason) {
    case MOVE_OFF_BOARD:
        printf("Invalid move: (%d, %d) is off the board. Rows and columns run 1 to %d.\n",
               row, col, BOARD_SIZE);
        break;
    case MOVE_SAME_SPACE:
        printf("Invalid move: your builder is already on (%d, %d) and must move.\n", row, col);
        break;
    case MOVE_NOT_ADJACENT:
        printf("Invalid move: (%d, %d) is not adjacent to your builder.\n", row, col);
        break;
    case MOVE_OCCUPIED:
    default:
        printf("Invalid move: (%d, %d) is occupied by the AI's builder.\n", row, col);
        break;
    }
}

/* Places the player's builder at the typed 1-based start if it is on the board.
 * Returns 1 when placed, 0 after explaining the rejection. */
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
 * Returns 0 when the input ends first, else 1. */
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

/* Applies the typed 1-based move when it is legal, else explains why not.
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

/* Asks for the player's move until a legal one is made, then shows the result.
 * Returns 0 when the input ends first, else 1. */
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
    printf("You move to (%d, %d).\n", player[ROW] + 1, player[COL] + 1);
    show_state(board, player, ai);
    return 1;
}

/* Lets the AI choose and make its move, then shows the result. */
static void play_ai_turn(int board[][BOARD_SIZE], int ai[2], const int player[2]) {
    int chosen[2] = {OFF_BOARD, OFF_BOARD};
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
    case RESULT_DRAW:
    default:
        printf("Draw!\n");
        break;
    }
}

/* ---- Entry point ---- */

/* Sets up the board and both builders, then alternates player and AI turns
 * until the game ends. */
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
