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

/* Outcomes of reading one input line. */
#define INPUT_END 0
#define INPUT_OK 1
#define INPUT_NOT_NUMBERS 2

static void initialize_board(int board[][BOARD_SIZE]);
static int count_level(int board[][BOARD_SIZE], int level);
static int is_occupied_by(const int builder[2], int row, int col);
static int is_on_board(int row, int col);
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
static void choose_ai_start(const int player[2], int chosen[2]);
static int end_of_input(void);

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

/* Reports whether (row, col) is inside the 0-based board. */
static int is_on_board(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
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

/* Prints the one-time rules summary. */
static void print_welcome(void) {
    printf("Santorini (230 version). You are %c, the AI is %c.\n", PLAYER_SYMBOL, AI_SYMBOL);
    printf("Moving raises (you) or lowers (AI) every space on the eight lines from the new space.\n");
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
    case MOVE_NOT_ADJACENT:
        printf("Invalid move: (%d, %d) is not adjacent to your builder.\n", row, col);
        break;
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

/* Starts the AI directly right of the player, or directly left when the
 * player chose the last column; one of the two always exists. */
static void choose_ai_start(const int player[2], int chosen[2]) {
    chosen[ROW] = player[ROW];
    chosen[COL] = player[COL] + 1;
    if (!is_on_board(chosen[ROW], chosen[COL])) {
        chosen[COL] = player[COL] - 1;
    }
}

/* Reports that the input ended before the game did. @return the exit status */
static int end_of_input(void) {
    printf("\nEnd of input: the game was abandoned.\n");
    return EXIT_SUCCESS;
}

/* Sets up the board and both builders; the turns arrive in a later task. */
int main(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int player[2] = {OFF_BOARD, OFF_BOARD};
    int ai[2] = {OFF_BOARD, OFF_BOARD};

    initialize_board(board);
    print_welcome();
    show_state(board, player, ai);
    if (!prompt_player_start(player)) {
        return end_of_input();
    }
    choose_ai_start(player, ai);
    printf("AI starts at (%d, %d).\n", ai[ROW] + 1, ai[COL] + 1);
    show_state(board, player, ai);
    return EXIT_SUCCESS;
}
