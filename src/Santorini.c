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

/* Index of the row and the column inside a builder position array. */
#define ROW 0
#define COL 1
/* Position value meaning "this builder is not on the board (yet)". */
#define OFF_BOARD (-1)

#define PLAYER_SYMBOL 'P'
#define AI_SYMBOL 'A'

static void initialize_board(int board[][BOARD_SIZE]);
static int count_level(int board[][BOARD_SIZE], int level);
static int is_occupied_by(const int builder[2], int row, int col);
static char cell_character(int board[][BOARD_SIZE], int row, int col,
                           const int player[2], const int ai[2]);
static void print_board(int board[][BOARD_SIZE], const int player[2], const int ai[2]);
static void print_score(int board[][BOARD_SIZE]);
static void show_state(int board[][BOARD_SIZE], const int player[2], const int ai[2]);

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

/* Entry point; the game loop arrives in a later task. */
int main(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int player[2] = {OFF_BOARD, OFF_BOARD};
    int ai[2] = {OFF_BOARD, OFF_BOARD};
    initialize_board(board);
    show_state(board, player, ai);
    return EXIT_SUCCESS;
}
