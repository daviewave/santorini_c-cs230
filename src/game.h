// GAME_H
#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define ROWS 7
#define COLS 7

typedef struct {
    int x;
    int y;
} Coordinate;

typedef struct {
    int level;
    char board_display;
    char occupied; // will initialized as Null and replaced with either "A" or "P"
    Coordinate coordinate;
} Space;

//1. in the playable board each board_display will be = to its level (2)
//2. every occupied will be ""
//3. coordinate x will be = i while y = j
//4. level = 2

typedef struct {
    char name;
    int score;
    Coordinate curr_space;
    Coordinate next_space;
} Player;

typedef Space GameBoard[ROWS][COLS];


GameBoard* initialize_gameboard(void);
void print_board(GameBoard* board);
void set_board_display(Space* space);
char int_to_char(int n);
void print_starting_msg(void);
void prompt_space_selection(bool is_first);
bool is_space_on_board(int x, int y);
bool is_straight_line(Coordinate curr, Coordinate next);
int get_random_coordinate(void);
bool is_unobstructed_path(Coordinate *path, Coordinate opp_coords);
void update_level(Space space, bool is_ai);
void update_score(Player player);
void display_game_over_message(Player player);


#endif