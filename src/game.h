#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define ROWS 7
#define COLS 7

typedef struct
{
    int x;
    int y;
} Coordinates;

typedef struct
{
    char board_display;
    int level;
    char occupied;
    Coordinates coordinates;
} Space;

typedef struct
{
    char name;
    int score;
    Coordinates curr;
    Coordinates next;
} Player;

typedef struct
{
    int *data;
    size_t len;
} IntArray;

typedef Space GameBoard[ROWS][COLS];

// 1.
GameBoard *initialize_gameboard(void);
void print_board(GameBoard *board);

// 2.
Player add_player(char name, Coordinates *user_location);
void print_starting_msg(void);
Coordinates get_coordinates_input(void);
Coordinates get_ai_starting_coordinates(Coordinates *user_location);
bool is_valid_coordinate(int coordinate);
IntArray get_allowed_directions(Coordinates *user_location);
Coordinates get_adjacent_coords(int random_number, Coordinates *user_location);
void set_board_display(Space *space);

// 3.
void next_move(Player *player, Coordinates *opp, GameBoard *board);
Space *get_board_space(Coordinates *coords, GameBoard *board);
void print_score(Player *user, Player *ai);
int get_move_direction(int xdiff, int ydiff);
bool is_occupied(Space *space);
bool is_not_straight_line_movement(int xdiff, int ydiff);
bool is_obstructed_path(int direction, Coordinates *curr, Coordinates *next, Coordinates *opp);
void update_spaces_in_path(int direction, bool increase, Coordinates *curr, Coordinates *next, GameBoard *board);

bool is_straight_line(Coordinates curr, Coordinates next);
int get_random_coordinate(void);
void update_level(Space space, bool is_ai);
void update_score(Player player);
void display_game_over_message(Player player);

// helper functions
char int_to_char(int n);
void set_space_board_display(Space *space);
int get_random_num(int min, int max);
bool random_num_in_allowed(int target, int *arr, size_t arr_len);

#endif