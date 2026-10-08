/* Unit tests for src/Santorini.c, included directly so static functions are reachable. */
int program_main(void);
#define main program_main
#include "../../src/Santorini.c"
#undef main
#include "check.h"

#define CAPTURE_PATH "build/test/stdout.txt"

/* Redirects stdout into a scratch file until capture_stdout_end. */
static void capture_stdout_begin(void) {
    fflush(stdout);
    if (freopen(CAPTURE_PATH, "w", stdout) == NULL) {
        fprintf(stderr, "cannot redirect stdout to %s\n", CAPTURE_PATH);
    }
}

/* Ends the redirection and copies what was printed into text. */
static void capture_stdout_end(char text[], size_t capacity) {
    FILE *file;
    size_t length;
    fflush(stdout);
    file = fopen(CAPTURE_PATH, "r");
    if (file == NULL) {
        text[0] = '\0';
        return;
    }
    length = fread(text, 1, capacity - 1, file);
    text[length] = '\0';
    fclose(file);
}

static void test_initialize_board_sets_every_space_to_start_level(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    initialize_board(board);
    CHECK_EQ_INT(count_level(board, START_LEVEL), BOARD_SIZE * BOARD_SIZE);
    CHECK_EQ_INT(count_level(board, LEVEL_MAX), 0);
    CHECK_EQ_INT(count_level(board, LEVEL_MIN), 0);
}

static void test_count_level_counts_only_matching_spaces(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    initialize_board(board);
    board[0][0] = 4;
    board[5][5] = 4;
    board[2][3] = 0;
    CHECK_EQ_INT(count_level(board, 4), 2);
    CHECK_EQ_INT(count_level(board, 0), 1);
    CHECK_EQ_INT(count_level(board, 2), BOARD_SIZE * BOARD_SIZE - 3);
}

static void test_is_occupied_by(void) {
    int builder[2] = {3, 4};
    CHECK(is_occupied_by(builder, 3, 4));
    CHECK(!is_occupied_by(builder, 4, 3));
    CHECK(!is_occupied_by(builder, 3, 5));
}

static void test_cell_character_shows_builders_and_levels(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int player[2] = {0, 1};
    int ai[2] = {0, 2};
    initialize_board(board);
    board[5][5] = 4;
    board[4][4] = 0;
    CHECK_EQ_INT(cell_character(board, 0, 1, player, ai), 'P');
    CHECK_EQ_INT(cell_character(board, 0, 2, player, ai), 'A');
    CHECK_EQ_INT(cell_character(board, 0, 0, player, ai), '2');
    CHECK_EQ_INT(cell_character(board, 5, 5, player, ai), '4');
    CHECK_EQ_INT(cell_character(board, 4, 4, player, ai), '0');
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

static void test_print_board_with_no_builders_shows_levels_only(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int nobody[2] = {OFF_BOARD, OFF_BOARD};
    char text[512] = "";
    initialize_board(board);
    capture_stdout_begin();
    print_board(board, nobody, nobody);
    capture_stdout_end(text, sizeof text);
    CHECK(strstr(text, "P") == NULL);
    CHECK(strstr(text, "A") == NULL);
    CHECK(strncmp(text, "   1 2 3 4 5 6\n1  2 2 2 2 2 2\n", 30) == 0);
}

static void test_show_state_prints_board_then_score_line(void) {
    int board[BOARD_SIZE][BOARD_SIZE];
    int player[2] = {0, 0};
    int ai[2] = {0, 1};
    char text[512] = "";
    initialize_board(board);
    board[5][5] = 4;
    board[5][4] = 0;
    board[5][3] = 0;
    capture_stdout_begin();
    show_state(board, player, ai);
    capture_stdout_end(text, sizeof text);
    CHECK(strstr(text, "6  2 2 2 0 0 4\nLevel-4 spaces (Player): 1   Level-0 spaces (AI): 2\n\n") != NULL);
}

int main(void) {
    test_initialize_board_sets_every_space_to_start_level();
    test_count_level_counts_only_matching_spaces();
    test_is_occupied_by();
    test_cell_character_shows_builders_and_levels();
    test_print_board_matches_spec_layout();
    test_print_board_with_no_builders_shows_levels_only();
    test_show_state_prints_board_then_score_line();
    CHECK_REPORT("test_santorini");
}
