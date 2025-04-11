// GAME_H
#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    int x;
    int y;
} Coordinate;

typedef struct {
    int level;
    bool is_occupied;
    Coordinate coordinate;
} Space;

typedef struct {
    char* name;
    Coordinate curr_space;
    Coordinate next_space;
    int score;
} Player;

void initialize_gameboard(void);
void print_board(void);
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