
#include "game.h"
#include <time.h>

// helpers
char int_to_char(int n)
{
    return '0' + n;
}

int get_random_num(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

void clear_console()
{
    system("clear");
}

//== 1.
GameBoard *initialize_gameboard()
{
    static Space board[ROWS][COLS];

    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            if (i == 0 && j == 0)
            {
                board[i][j].board_display = ' ';
            }
            else if (i == 0 && j != 0)
            {
                // label row
                board[i][j].board_display = int_to_char(j);
            }
            else if (j == 0 && i != 0)
            {
                // label column
                board[i][j].board_display = int_to_char(i);
            }
            else
            {
                // playable space
                board[i][j].coordinates.x = i;
                board[i][j].coordinates.y = j;
                board[i][j].level = 2;
                board[i][j].board_display = '2'; // level to start since user hasnt been prompted for starting space
            }
        }
    }
    return &board;
}

void print_starting_msg()
{
    printf("Welcome to santorini special edition! \nYour goal is to get 10 places on the board to level 4 before the AI.\n\n");
}

void print_board(GameBoard *board)
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            printf("%c ", (*board)[i][j].board_display);
        }
        printf("\n");
    }
    printf("\n");
}

//== 2.
Player add_player(char name, Coordinates *user_location)
{
    Player new_player;
    new_player.name = name;
    new_player.score = 0;

    if (name == 'P')
    {
        new_player.curr = get_coordinates_input();
    }
    else
    {
        new_player.curr = get_ai_starting_coordinates(user_location);
    }

    return new_player;
}

// TODO: update input format checking with regular expressions
Coordinates get_coordinates_input()
{
    int x, y;

    do
    {
        printf("Enter the coordinates for the space you'd like to move to using the format 'x,y': ");
        if (scanf("%d,%d", &x, &y) == 2)
        {
            if (is_valid_coordinate(x) && is_valid_coordinate(y))
            {
                Coordinates coords;
                coords.x = x;
                coords.y = y;
                return coords;
            }
            printf("invalid coordinates, try again...\n");
        }
    } while (true);
}

bool is_valid_coordinate(int coordinate)
{
    if (coordinate > 0 && coordinate < 7)
    {
        return true;
    }
    return false;
}

Coordinates get_ai_starting_coordinates(Coordinates *user_location)
{
    // [n: 1, ne: 2, e: 3, se: 4, s: 5, sw: 6, w: 7, nw: 8]
    int rn;
    IntArray allowed_directions = get_allowed_directions(user_location);
    do
    {
        rn = get_random_num(1, 8);
    } while (!random_num_in_allowed(rn, allowed_directions.data, allowed_directions.len));

    // printf("[n: 1, ne: 2, e: 3, se: 4, s: 5, sw: 6, w: 7, nw: 8]\n");
    // printf("dir: %d \n", rn);
    return get_adjacent_coords(rn, user_location);
}

IntArray get_allowed_directions(Coordinates *user_location)
{
    IntArray allowed;

    if (user_location->x == 1 && user_location->y == 1)
    {
        static int directions[3] = {3, 4, 5};
        allowed.data = directions;
        allowed.len = 3;
    }
    else if (user_location->x == 1 && user_location->y == 6)
    {
        static int directions[3] = {5, 6, 7};
        allowed.data = directions;
        allowed.len = 3;
    }
    else if (user_location->x == 6 && user_location->y == 1)
    {
        static int directions[3] = {1, 2, 3};
        allowed.data = directions;
        allowed.len = 3;
    }
    else if (user_location->x == 6 && user_location->y == 6)
    {
        static int directions[3] = {1, 7, 8};
        allowed.data = directions;
        allowed.len = 3;
    }
    else if (user_location->x == 1)
    {
        static int directions[5] = {3, 4, 5, 6, 7};
        allowed.data = directions;
        allowed.len = 5;
    }
    else if (user_location->x == 6)
    {
        static int directions[5] = {1, 2, 3, 7, 8};
        allowed.data = directions;
        allowed.len = 5;
    }
    else if (user_location->y == 1)
    {
        static int directions[5] = {1, 2, 3, 4, 5};
        allowed.data = directions;
        allowed.len = 5;
    }
    else if (user_location->y == 6)
    {
        static int directions[5] = {1, 5, 6, 7, 8};
        allowed.data = directions;
        allowed.len = 5;
    }
    else
    {

        static int directions[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        allowed.data = directions;
        allowed.len = 8;
    }
    return allowed;
}

bool random_num_in_allowed(int target, int *arr, size_t arr_len)
{
    for (int i = 0; i < arr_len; i++)
    {
        if (arr[i] == target)
        {
            return true;
        }
    }
    return false;
}

Coordinates get_adjacent_coords(int random_number, Coordinates *user_location)
{
    Coordinates coords;
    switch (random_number)
    {
    case 1:
        // north
        coords.x = user_location->x - 1;
        coords.y = user_location->y;
        break;

    case 2:
        // northeast
        coords.x = user_location->x - 1;
        coords.y = user_location->y + 1;
        break;

    case 3:
        // east
        coords.x = user_location->x;
        coords.y = user_location->y + 1;
        break;

    case 4:
        // southeast
        coords.x = user_location->x + 1;
        coords.y = user_location->y + 1;
        break;

    case 5:
        // south
        coords.x = user_location->x + 1;
        coords.y = user_location->y;
        break;

    case 6:
        // southwest
        coords.x = user_location->x + 1;
        coords.y = user_location->y - 1;
        break;

    case 7:
        // west
        coords.x = user_location->x;
        coords.y = user_location->y - 1;
        break;

    case 8:
        // northwest
        coords.x = user_location->x - 1;
        coords.y = user_location->y - 1;
        break;

    default:
        break;
    }
    return coords;
}

void set_space_board_display(Space *space)
{
    // if occupied != ' ' set board_display as that and return
    if (space->occupied != ' ')
    {
        space->board_display = space->occupied;
    }
    else
    {
        space->board_display = space->level;
    }
}

//== 3.
void next_move(Player *player, Coordinates *opp, GameBoard *board)
{
    if (player->name == 'P')
    {
        // 1, prompt new space coords
        Coordinates coords = get_coordinates_input();
        Space *next_space = get_board_space(&coords, board);

        // 2, ensure space isnt occupied by other player
        if (is_occupied(next_space))
        {
            printf("\n\t(invalid move) -> other player already at this space. must select unoccupied space. \n\n");
            next_move(player, opp, board);
        }

        // 3, determine move direction
        int xdiff = coords.x - player->curr.x;
        int ydiff = coords.y - player->curr.y;
        int d = get_move_direction(xdiff, ydiff);

        // 4, ensure straight line if diagnol
        if (d == 2 || d == 4 || d == 6 || d == 8)
        {
            // diagnol movement -- must ensure straight line
            if (is_not_straight_line_movement(xdiff, ydiff))
            {
                printf("\n\t(invalid move) -> only straight line movements allowed. \n\n");
                next_move(player, opp, board);
            }
        }

        // 5, ensure unobstructed path
        if (is_obstructed_path(d, &player->curr, &coords, opp))
        {
            printf("\n\t(invalid move) -> opposing play obstructing path to new space entered. \n\n");
            next_move(player, opp, board);
        }

                //==* At this point its determined to be a valid move
        // 6, change levels on each space in the path, including where player lands

        // 7, set the next_space.occupied = player.name & set_board_display()

        // if (is_unobstructed_path(&player->curr, &coords, board))
        // {
        //     printf("\nis unobstructed path \n");
        // }

        // if (not_occupied(next_space) && is_straight_line_movement(&player->curr, &coords) && is_unobstructed_path(&player->curr, &coords, board))
        // {
        //     printf("\nhitttting\n");
        //     // set next_space occupied to player.name
        //     // set the previous occupied space to '\0'
        //     // set board displays for both next and previous
        //     // NOTE: have to determine when levels are updated, prob makes sense to break the unobstructed path to more granular functions so they can be used
        //     //          for this too
        // }
        // printf("\ninvalid movement\n");

        // printf("\nx: %d y: %d", coords.x, coords.y);
        // printf("\nns bd: %c", next_space->board_display);
        // printf("\nns level: %d", next_space->level);
        // printf("\nns bd: %c", next_space->occupied);
        // printf("\nns bd: %d", next_space->coordinates.x);
        // printf("\nns bd: %d", next_space->coordinates.y);
        // validate move
        // printf("\nns bd: %c", next_space->board_display);
        // printf("\nns level: %d", next_space->level);
        // printf("\nns bd: %c", next_space->occupied);
        // printf("\nns bd: %d", next_space->coordinates.x);
        // printf("\nns bd: %d", next_space->coordinates.y);
    }
    else
    {
        // ai automatically selects coords until valid spot is chosen
    }
}

Space *get_board_space(Coordinates *coords, GameBoard *board)
{
    Space *found = &(*board)[coords->x][coords->y];
    return found;
}

int get_move_direction(int xdiff, int ydiff)
{
    // printf("[n: 1, ne: 2, e: 3, se: 4, s: 5, sw: 6, w: 7, nw: 8]\n")
    // int xdiff = next->x - curr->y;
    // int ydiff = next->y - curr->y;
    if (ydiff == 0 && xdiff != 0)
    {
        // south or north
        return (xdiff > 0) ? 5 : 1;
    }
    else if (ydiff != 0 && xdiff == 0)
    {
        // east or west
        return (ydiff > 0) ? 3 : 7;
    }
    else
    {
        if (ydiff > 0 && xdiff < 0)
        {
            // northeast
            return 2;
        }
        else if (ydiff > 0 && xdiff > 0)
        {
            // southeast
            return 4;
        }
        else if (ydiff < 0 && xdiff > 0)
        {
            // southwest
            return 6;
        }
        else
        {
            // northwest
            return 8;
        }
    }
}

bool is_occupied(Space *space)
{
    if (space->occupied == 'P' || space->occupied == 'A')
        return true;

    return false;
}

bool is_not_straight_line_movement(int xdiff, int ydiff)
{
    int slope = abs((ydiff) / (xdiff));

    if (slope == 1)
        return false;
    return true;
}

bool is_obstructed_path(int direction, Coordinates *curr, Coordinates *next, Coordinates *opposing_player)
{
    switch (direction)
    {
    case 1:
        if (opposing_player->x > next->x)
            return true;
        break;
    case 2:
        if (((curr->x > opposing_player->x) && (next->x < opposing_player->x)) && ((curr->y < opposing_player->y) && (next->y > opposing_player->y)))
            return true;
        break;
    case 3:
        if (opposing_player->y < next->y)
            return true;
        break;

    case 4:
        if (((curr->x < opposing_player->x) && (next->x > opposing_player->x)) && ((curr->y > opposing_player->y) && (next->y > opposing_player->y)))
            return true;
        break;

    case 5:
        if (next->x > opposing_player->x)
            return true;
        break;

    case 6:
        if (((curr->x < opposing_player->x) && (next->x > opposing_player->x)) && ((curr->y > opposing_player->y) && (next->y < opposing_player->y)))
            return true;
        break;
    case 7:
        if (next->y < opposing_player->y)
            return true;
        break;
    case 8:
        if (((curr->x > opposing_player->x) && (next->x < opposing_player->x)) && ((curr->y > opposing_player->y) && (next->y < opposing_player->y)))
            return true;
        break;

    default:
        break;
    }
    return true;

    // int x_movement = next->x - curr->y;
    // int y_movement = next->y - curr->y;
    // Coordinates curr_coords;
    // Space *curr_space;

    // if (x_movement == 0)
    // {
    //     // non diagnol y movement
    //     int step_direction = (y_movement > 0) ? 1 : -1;
    //     for (int y = (curr->y += step_direction); y != next->y; y += step_direction)
    //     {
    //         curr_coords.y = y;
    //         curr_space = get_board_space(&curr_coords, board);
    //         if (curr_space->occupied != ' ')
    //         {
    //             return false;
    //         }
    //     }
    // }
    // else if (y_movement == 0)
    // {
    //     printf("\n correct condition \n");
    //     // non diagnol x movement
    //     // curr_coords.y = 0; -> automatically gets set to 0
    //     int step_direction = (x_movement > 0) ? 1 : -1;
    //     for (int x = (curr->x += step_direction); x != next->x; x += step_direction)
    //     {
    //         curr_coords.x = x;
    //         curr_space = get_board_space(&curr_coords, board);
    //         if (curr_space->occupied != ' ')
    //         {
    //             return false;
    //         }
    //     }
    // }
    // else
    // {
    //     // diagnol movement
    //     int x_step_dir = (x_movement > 0) ? 1 : -1;
    //     int y_step_dir = (y_movement > 0) ? 1 : -1;
    //     int x = (curr->x += x_step_dir);
    //     int y = (curr->y += y_step_dir);
    //     do
    //     {
    //         curr_coords.x = x;
    //         curr_coords.y = y;
    //         curr_space = get_board_space(&curr_coords, board);
    //         if (curr_space->occupied != ' ')
    //         {
    //             return false;
    //         }
    //         x += x_step_dir;
    //         y += y_step_dir;
    //     } while (x != next->x && y != next->y);
    // }
    // return true;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    // 1,
    clear_console();
    GameBoard *board = initialize_gameboard();
    print_starting_msg();
    print_board(board);

    // 2,a
    Player user = add_player('P', NULL);
    Space *starting_user_space = get_board_space(&user.curr, board);
    starting_user_space->occupied = user.name;
    set_space_board_display(starting_user_space);

    // 2,b
    Player ai = add_player('A', &user.curr);
    Space *starting_ai_space = get_board_space(&ai.curr, board);
    starting_ai_space->occupied = ai.name;
    set_space_board_display(starting_ai_space);

    // printf("\nns bd: %c", starting_user_space->board_display);
    // printf("\nns level: %d", starting_user_space->level);
    // printf("\nns occupied: %c", starting_user_space->occupied);
    // printf("\nns cx: %d", starting_user_space->coordinates.x);
    // printf("\nns cy: %d", starting_user_space->coordinates.y);

    // printf("\nai bd: %c", starting_ai_space->board_display);
    // printf("\nai level: %d", starting_ai_space->level);
    // printf("\nai occupied: %c", starting_ai_space->occupied);
    // printf("\nai cx: %d", starting_ai_space->coordinates.x);
    // printf("\nai cy: %d", starting_ai_space->coordinates.y);

    // printf("User Player: \n");
    // printf("\t name -> %c \n", user.name);
    // printf("\t score -> %d \n", user.score);
    // printf("\t curr.x -> %d \n", user.curr.x);
    // printf("\t curr.y -> %d \n", user.curr.y);
    // printf("\n");
    // printf("Ai Player: \n");
    // printf("\t name -> %c \n", ai.name);
    // printf("\t score -> %d \n", ai.score);
    // printf("\t curr.x -> %d \n", ai.curr.x);
    // printf("\t curr.y -> %d \n", ai.curr.y);
    // printf("\n");

    do
    {
        // keep running until a player has reached score=10
        clear_console();
        // print_score();
        print_board(board);
        next_move(&user, &ai.curr, board);
        // next_move(&ai, board);
        // check_for_winner()

        break;

    } while (true);

    // free(board);

    return 0;
}