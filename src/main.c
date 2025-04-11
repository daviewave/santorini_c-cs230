
#include "game.h"

void set_board_display(Space* space){
    // 
}

char int_to_char(int n){
    return '0' + n;
}

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
                board[i][j].coordinate.x = i;
                board[i][j].coordinate.y = j;
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

int main(int argc, char *argv[]){
    GameBoard *board = initialize_gameboard();
    print_board(board);

    return 0;
}