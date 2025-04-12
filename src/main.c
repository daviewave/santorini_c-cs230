
#include "game.h"
#include <time.h>

// helpers
char int_to_char(int n){
    return '0' + n;
}

int get_random_num(int min, int max){
    return (rand() % (max - min + 1)) + min;
}


//== 1.
GameBoard* initialize_gameboard() {
    static Space board[ROWS][COLS];
    
    for(int i=0;i<ROWS;i++){
        for(int j=0;j<COLS;j++){
            if(i == 0 && j == 0){
                board[i][j].board_display = ' ';
            } else if(i == 0 && j != 0){ 
                // label row
                board[i][j].board_display = int_to_char(j);
            } else if(j == 0 && i != 0){ 
                // label column
                board[i][j].board_display = int_to_char(i);
            } else {
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

void print_board(GameBoard *board) {
    for(int i=0;i<ROWS;i++){
        for(int j=0;j<COLS;j++){
            printf("%c ", (*board)[i][j].board_display);
        }
        printf("\n");
    }
}

//== 2.
Player add_player(char name, Coordinates *user_location){
    Player new_player;
    new_player.name = name;
    new_player.score = 0;

    if(name == 'P'){
        new_player.curr = get_coordinates_input();
    } else {
        new_player.curr = get_ai_starting_coordinates(user_location);
    }

    return new_player;
}

// TODO: update input format checking with regular expressions
Coordinates get_coordinates_input() {
    int x, y;

    do {
        printf("Enter the coordinates for the space you'd like to move to using the format 'x,y': ");
        if(scanf("%d,%d", &x, &y) == 2){
            if(is_valid_coordinate(x) && is_valid_coordinate(y)){
                Coordinates coords;
                coords.x = x;
                coords.y = y;
                return coords;
            }
            printf("invalid coordinates, try again...\n");
        }
    } while(true);

}

bool is_valid_coordinate(int coordinate){
    if(coordinate > 0 && coordinate < 7){
        return true;
    }
    return false;
}

Coordinates get_ai_starting_coordinates(Coordinates *user_location){
    // [n: 1, ne: 2, e: 3, se: 4, s: 5, sw: 6, w: 7, nw: 8]
    int rn;
    IntArray allowed_directions = get_allowed_directions(user_location);
    do {
        rn = get_random_num(1, 8);
    } while(!random_num_in_allowed(rn, allowed_directions.data, allowed_directions.len));
    
    printf("[n: 1, ne: 2, e: 3, se: 4, s: 5, sw: 6, w: 7, nw: 8]\n");
    printf("dir: %d \n", rn);
    return get_adjacent_coords(rn, user_location);
}

IntArray get_allowed_directions(Coordinates *user_location){
    IntArray allowed;

    if(user_location->x == 1 && user_location->y == 1){
        static int directions[3] = { 3, 4, 5 };
        allowed.data = directions;
        allowed.len = 3;
    }
    else if(user_location->x == 1 && user_location->y == 6){
        static int directions[3] = { 5, 6, 7 };
        allowed.data = directions;
        allowed.len = 3;
    }
    else if(user_location->x == 6 && user_location->y == 1){
        static int directions[3] = { 1, 2, 3 };
        allowed.data = directions;
        allowed.len = 3;
    }
    else if(user_location->x == 6 && user_location->y == 6){
        printf("\nhitting correctly\n");
        static int directions[3] = { 1, 7, 8 };
        allowed.data = directions;
        allowed.len = 3;
    }
    else if(user_location->x == 1){
        static int directions[5] = { 3, 4, 5, 6, 7 };
        allowed.data = directions;
        allowed.len = 5;
    }
    else if(user_location->x == 6){
        static int directions[5] = { 1, 2, 3, 7, 8 };
        allowed.data = directions;
        allowed.len = 5;
    }
    else if(user_location->y == 1){
        static int directions[5] = { 1, 2, 3, 4, 5 };
        allowed.data = directions;
        allowed.len = 5;
    }
    else if(user_location->y == 6){
        static int directions[5] = { 1, 5, 6, 7, 8 };
        allowed.data = directions;
        allowed.len = 5;
    }
    else {
        
        static int directions[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
        allowed.data = directions;
        allowed.len = 8;
    }
    return allowed;
}

bool random_num_in_allowed(int target, int* arr, size_t arr_len){
    for(int i=0;i<arr_len;i++){
        if(arr[i] == target){
            return true;
        } 
    }
    return false;
}

Coordinates get_adjacent_coords(int random_number, Coordinates *user_location){
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

int main(int argc, char *argv[]){
    srand(time(NULL));
    GameBoard *board = initialize_gameboard();
    print_board(board);

    printf("\n");
    Player user = add_player('P', NULL);
    Player ai = add_player('A', &user.curr);

    printf("User Player: \n");
    printf("\t name -> %c \n", user.name);
    printf("\t score -> %d \n", user.score);
    printf("\t curr.x -> %d \n", user.curr.x);
    printf("\t curr.y -> %d \n", user.curr.y);
    printf("\n");
    printf("Ai Player: \n");
    printf("\t name -> %c \n", ai.name);
    printf("\t score -> %d \n", ai.score);
    printf("\t curr.x -> %d \n", ai.curr.x);
    printf("\t curr.y -> %d \n", ai.curr.y);
    printf("\n");

    return 0;
}