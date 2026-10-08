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

#define STDIN_PATH "build/test/stdin.txt"

/* Replaces stdin with a file holding text. */
static void feed_stdin(const char *text) {
    FILE *file = fopen(STDIN_PATH, "w");
    if (file == NULL) {
        fprintf(stderr, "cannot write %s\n", STDIN_PATH);
        return;
    }
    fputs(text, file);
    fclose(file);
    if (freopen(STDIN_PATH, "r", stdin) == NULL) {
        fprintf(stderr, "cannot redirect stdin from %s\n", STDIN_PATH);
    }
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

static void test_is_on_board(void) {
    CHECK(is_on_board(0, 0));
    CHECK(is_on_board(5, 5));
    CHECK(!is_on_board(-1, 0));
    CHECK(!is_on_board(0, -1));
    CHECK(!is_on_board(0, 6));
    CHECK(!is_on_board(6, 0));
}


static void test_read_coordinates_reads_two_numbers(void) {
    int typed[2] = {0, 0};
    feed_stdin("2 4\n");
    CHECK_EQ_INT(read_coordinates(typed), INPUT_OK);
    CHECK_EQ_INT(typed[ROW], 2);
    CHECK_EQ_INT(typed[COL], 4);
}

static void test_read_coordinates_accepts_extra_whitespace_and_negatives(void) {
    int typed[2] = {0, 0};
    feed_stdin("  -3   7  \n");
    CHECK_EQ_INT(read_coordinates(typed), INPUT_OK);
    CHECK_EQ_INT(typed[ROW], -3);
    CHECK_EQ_INT(typed[COL], 7);
}

static void test_read_coordinates_rejects_letters_then_reads_next_line(void) {
    int typed[2] = {0, 0};
    feed_stdin("a b\n3 3\n");
    CHECK_EQ_INT(read_coordinates(typed), INPUT_NOT_NUMBERS);
    CHECK_EQ_INT(read_coordinates(typed), INPUT_OK);
    CHECK_EQ_INT(typed[ROW], 3);
    CHECK_EQ_INT(typed[COL], 3);
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
    feed_stdin("   \n\n");
    CHECK_EQ_INT(read_coordinates(typed), INPUT_END);
}

static void test_read_coordinates_last_line_without_newline(void) {
    int typed[2] = {0, 0};
    feed_stdin("6 6");
    CHECK_EQ_INT(read_coordinates(typed), INPUT_OK);
    CHECK_EQ_INT(typed[ROW], 6);
    CHECK_EQ_INT(typed[COL], 6);
    CHECK_EQ_INT(read_coordinates(typed), INPUT_END);
}

static void test_place_typed_start_converts_to_zero_based(void) {
    int player[2] = {OFF_BOARD, OFF_BOARD};
    int typed[2] = {1, 2};
    CHECK_EQ_INT(place_typed_start(player, typed), 1);
    CHECK_EQ_INT(player[ROW], 0);
    CHECK_EQ_INT(player[COL], 1);
}

static void test_place_typed_start_rejects_off_board(void) {
    int player[2] = {OFF_BOARD, OFF_BOARD};
    int zero[2] = {0, 3};
    int seven[2] = {3, 7};
    int negative[2] = {-1, -1};
    char text[256] = "";
    capture_stdout_begin();
    CHECK_EQ_INT(place_typed_start(player, zero), 0);
    CHECK_EQ_INT(place_typed_start(player, seven), 0);
    CHECK_EQ_INT(place_typed_start(player, negative), 0);
    capture_stdout_end(text, sizeof text);
    CHECK_EQ_INT(player[ROW], OFF_BOARD);
    CHECK(strstr(text, "(0, 3) is off the board") != NULL);
    CHECK(strstr(text, "(3, 7) is off the board") != NULL);
    CHECK(strstr(text, "(-1, -1) is off the board") != NULL);
}

static void test_prompt_player_start_reprompts_until_valid(void) {
    int player[2] = {OFF_BOARD, OFF_BOARD};
    char text[512] = "";
    feed_stdin("x y\n0 0\n1 2\n");
    capture_stdout_begin();
    CHECK_EQ_INT(prompt_player_start(player), 1);
    capture_stdout_end(text, sizeof text);
    CHECK_EQ_INT(player[ROW], 0);
    CHECK_EQ_INT(player[COL], 1);
    CHECK(strstr(text, "Please enter two numbers") != NULL);
    CHECK(strstr(text, "(0, 0) is off the board") != NULL);
}

static void test_prompt_player_start_reports_end_of_input(void) {
    int player[2] = {OFF_BOARD, OFF_BOARD};
    char text[512] = "";
    feed_stdin("9 9\n");
    capture_stdout_begin();
    CHECK_EQ_INT(prompt_player_start(player), 0);
    capture_stdout_end(text, sizeof text);
    CHECK_EQ_INT(player[ROW], OFF_BOARD);
}

static void test_choose_ai_start_right_then_left(void) {
    int player[2] = {0, 1};
    int edge[2] = {3, 5};
    int corner[2] = {5, 5};
    int chosen[2] = {OFF_BOARD, OFF_BOARD};
    choose_ai_start(player, chosen);
    CHECK_EQ_INT(chosen[ROW], 0);
    CHECK_EQ_INT(chosen[COL], 2);
    choose_ai_start(edge, chosen);
    CHECK_EQ_INT(chosen[ROW], 3);
    CHECK_EQ_INT(chosen[COL], 4);
    choose_ai_start(corner, chosen);
    CHECK_EQ_INT(chosen[ROW], 5);
    CHECK_EQ_INT(chosen[COL], 4);
}

static void test_choose_ai_start_is_always_adjacent_and_on_board(void) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            int player[2] = {row, col};
            int chosen[2] = {OFF_BOARD, OFF_BOARD};
            choose_ai_start(player, chosen);
            CHECK(is_on_board(chosen[ROW], chosen[COL]));
            CHECK(!is_occupied_by(player, chosen[ROW], chosen[COL]));
            CHECK(abs(chosen[COL] - col) == 1 && chosen[ROW] == row);
        }
    }
}

int main(void) {
    test_initialize_board_sets_every_space_to_start_level();
    test_count_level_counts_only_matching_spaces();
    test_is_occupied_by();
    test_cell_character_shows_builders_and_levels();
    test_print_board_matches_spec_layout();
    test_print_board_with_no_builders_shows_levels_only();
    test_show_state_prints_board_then_score_line();
    test_is_on_board();
    test_read_coordinates_reads_two_numbers();
    test_read_coordinates_accepts_extra_whitespace_and_negatives();
    test_read_coordinates_rejects_letters_then_reads_next_line();
    test_read_coordinates_partial_line_does_not_leak();
    test_read_coordinates_reports_end_of_input();
    test_read_coordinates_last_line_without_newline();
    test_place_typed_start_converts_to_zero_based();
    test_place_typed_start_rejects_off_board();
    test_prompt_player_start_reprompts_until_valid();
    test_prompt_player_start_reports_end_of_input();
    test_choose_ai_start_right_then_left();
    test_choose_ai_start_is_always_adjacent_and_on_board();
    CHECK_REPORT("test_santorini");
}
