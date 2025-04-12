
#include "game.h"
#include <time.h>

// helpers
char int_to_char(int n){
    return '0' + n;
}

int get_random_num(int min, int max){
    srand(time(NULL));
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
Player* add_player(char name){
    Player new_player;
    new_player.name = name;

    if(name == 'P'){
        new_player.curr = get_coordinates_input();
    } else {

    }

    printf("%d", new_player.curr.x);
    printf("\n");
    printf("%d", new_player.curr.y);
    printf("\n");

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

Coordinates get_ai_starting_coordinates(Coordinates user_location){
    // [n, ne, e, se, s, sw, w, nw]
    int rn;
    int *allowed_directions = get_allowed_directions(user_location);

    int n = sizeof(allowed_directions) / sizeof(allowed_directions[0]);
    printf("\nsize of allowed=%d \n", n);
    // for(int i=0;i<n;i++){
    //     printf("\nallowed: %d \n", i);
    // }


    for (int i = 0; i < 3; i++) {
        printf("%d ", allowed_directions[i]);
    }

    // printf("\n rn: %d \n", rn);
    // do {
    //     rn = get_random_num(1, 8);
    // } while(!random_num_in_allowed(rn, allowed_directions));
    


    // Coordinates coords;
    // switch (rn)
    // {
    // case 1:
    //     // north
    //     coords.x = user_location.x + 1;
    //     coords.y = user_location.y;
    //     break;
    
    // case 2:
    //     // northeast
    //     coords.x = user_location.x + 1;
    //     coords.y = user_location.y + 1;
    //     break;

    // case 3:
    //     // east
    //     coords.x = user_location.x;
    //     coords.y = user_location.y + 1;
    //     break;

    // case 4:
    //     // southeast
    //     coords.x = user_location.x - 1;
    //     coords.y = user_location.y + 1;
    //     break;

    // case 5:
    //     // south
    //     coords.x = user_location.x - 1;
    //     coords.y = user_location.y;
    //     break;

    // case 6:
    //     // southwest
    //     coords.x = user_location.x - 1;
    //     coords.y = user_location.y - 1;
    //     break;

    // case 7:
    //     // west
    //     coords.x = user_location.x;
    //     coords.y = user_location.y - 1;
    //     break;
    
    // case 8:
    //     // northwest
    //     coords.x = user_location.x + 1;
    //     coords.y = user_location.y - 1;
    //     break;

    // default:
    //     break;
    // }
    // return coords;
}

int* get_allowed_directions(Coordinates user_location){
    // int *allowed;

    if(user_location.x == 1 && user_location.y == 1){
        static int allowed[3] = { 3, 4, 5 };
        return allowed;
        
        // allowed = malloc(3 * sizeof(int));
        // allowed[0] = 3, allowed[1] = 4, allowed[2] = 5; 
    }
    // else if(user_location.x == 1 && user_location.y == 7){
    //     allowed = malloc(3 * sizeof(int));
    //     allowed[0] = 5, allowed[1] = 6, allowed[2] = 7; 
    // }
    // else if(user_location.x == 7 && user_location.y == 1){
    //     allowed = malloc(3 * sizeof(int));
    //     allowed[0] = 1, allowed[1] = 2, allowed[2] = 3; 
    // }
    // else if(user_location.x == 7 && user_location.y == 7){
    //     allowed = malloc(3 * sizeof(int));
    //     allowed[0] = 1, allowed[1] = 7, allowed[2] = 8; 
    // }
    // else if(user_location.x == 1){
    //     allowed = malloc(5 * sizeof(int));
    //     allowed[0] = 3, allowed[1] = 4, allowed[2] = 5, allowed[3] = 6, allowed[4] = 7; 
    // }
    // else if(user_location.x == 7){
    //     allowed = malloc(5 * sizeof(int));
    //     allowed[0] = 1, allowed[1] = 2, allowed[2] = 3, allowed[3] = 7, allowed[4] = 8; 
    // }
    // else if(user_location.y == 1){
    //     allowed = malloc(5 * sizeof(int));
    //     allowed[0] = 1, allowed[1] = 2, allowed[2] = 3, allowed[3] = 4, allowed[4] = 5; 
    // }
    // else if(user_location.y == 7){
    //     allowed = malloc(5 * sizeof(int));
    //     allowed[0] = 1, allowed[1] = 5, allowed[2] = 6, allowed[3] = 7, allowed[4] = 8; 
    // }
    // else {
    //     allowed = malloc(8 * sizeof(int));
    //     allowed[0] = 1, allowed[1] = 2, allowed[2] = 3, allowed[3] = 4, allowed[4] = 5, allowed[5] = 6, allowed[6] = 7, allowed[7] = 8;
    // }
    
    // return allowed;
}

bool random_num_in_allowed(int target, int* arr){
    int num_elems = sizeof(arr) / sizeof(arr[0]);
    for(int i=0;i<num_elems;i++){
        if(arr[i] == target){
            return true;
        } 
    }
    return false;
}

int main(int argc, char *argv[]){
    // GameBoard *board = initialize_gameboard();
    // print_board(board);

    // add_player('P');

    Coordinates coords;
    coords.x = 1;
    coords.y = 1;
    get_ai_starting_coordinates(coords);

    return 0;
}