#== core functionalities ==#
1. game board creation
2. starting spaces for players
3. player movement logic
4. build level increases/decreases + constraint checking
5. win scenario definition and checking


#- 1. game board creation -#
- initialize board as 2d array, `int gameboard[7][7];` --> use custom "Space" struct instead of int
- loop through each element in the 2d array and handle the following:
    -> the 1st row as the column label row (potential values=1-6)
    -> the 1st column as the row label column (potential values=1-6)
    -> the first element (0,0) will be empty
    -> all other spaces are 2

#- 2. starting spaces for players -#
- create well thought out code for the prompting of the user for the next space to move to (try to make it so that it can be re-used in):
    -> if its the start of the game make prompt different
    -> making sure to handle all invalid inputs meaning must be in x,y format
- will need to use a random selection of n,s,e,w,ne,nw,se,sw to determine the ai's starting space as it must be 1 away from the players to start
- initialize the score

#- 3. player movement logic -#
- all proceeding movements must checking
    -> the specified spot is a valid playable space on the board (cant be off the board or in a label column) 
    -> the specified spot is not occupied by the other player
    -> is a straight line, by knowing that slope=1 in straight lines
    -> the other player is not within the path from the current space to the next space

#- 4. increasing/descreasing building levels -#
- for each space in the validated path to the new space will be updated
- the function will accept an argument to determine if its a user movement or an ai movement, and increase or decrease each space

#- 5. win scenario -#
- using the score tracking method developed in the 2nd core functionality when the game is initialized, check if either players score has reached 10 and if so display a 
    winner/loser message, and end the game


#== structs ==#
1. Coordinates {
    int x;
    int y;
}

2. Space {
    int level;
    bool is_occupied;
}

3. Player {
    char[2] name;
    Coordinates curr_space;
    Coordinates next_space;
    int score;
}


#== functions ==#
1. <2darray> initialize_gameboard(){ creates the 2d list and sets its elements to correct starting values }
a. void print_board(){ as the function name says (using loop) }

3. next_turn(bool is_first, bool is_ai){
    print_board()
    if(is_first){
        if(!is_ai){
        a, display starting message to user (function print_starting_message())
        b, prompt user for starting space coordinates (function prompt_space_selection(bool is_first))
            -> ensure x and y value is within 1-6 (function is_space_on_board(x, y))
        c, update the space with the users 'P' do something to remember the its original value (function first_move(bool is_ai, int x, int y))
            -> definitely seems like a struct could help with this
        } 
        
        else {
            a, use the user's players current coordinates as an argument and randomly select adjacent direction (function first_move(bool is_ai, int x, int y))
        }
    } else {
        if(!is_ai){
            a, prompt user for starting space coordinates (function prompt_space_selection(bool is_first))
                -> ensure x and y value is within 1-6 (function is_space_on_board(x, y))
                -> ensure new space is not the other player (function is_occupied())
                -> ensure movement is straight line (bool is_unobstructed_path(array[Space] path, oppositions_x, oppositions_y))
                -> ensure other player is not obstructing the path (function is_line_obstructed(array coordinates_in_path, other_players_coordinates))
            b, update each space in the paths building level including the starting space but not the new current space, this will update the current players score as each building level is updated (function update_path_levels(bool is_ai, array spaces))
            c, check the current players score to see if they won and display a winning message (function show_winner_message())
        } else {
            a, randomly choose an x and y cooridinate 1-6 until a valid selection occurs (Coordinates get_random_coordinates())
                -> ensure x and y value is within 1-6 (function is_space_on_board(x, y))
                -> ensure new space is not the other player (function is_occupied())
                -> ensure movement is straight line (function is_straight_line(int curr_x, int curr_y, int new_x, int new_y))
                -> ensure other player is not obstructing the path (function is_line_obstructed(array coordinates_in_path, other_players_coordinates))
            b, update each space in the paths building level including the starting space but not the new current space, this will update the current players score as each building level is updated (function update_path_levels(bool is_ai, array spaces))
            c, check the current players score to see if they won and display a winning message (function show_winner_message())
        }
    }
}

a. void print_starting_message() { simple welcome to the game, prompts for name and maybe a short overview on how to play/win }

b. void prompt_space_selection(bool is_first) { gets user input for next space's x + y coordinates and updates the value in the users Player struct }

c. bool is_space_on_board(x, y){ return true if both x and y coordinates are within greater than 0 but less than 7 }

d. bool is_straight_line(int curr_x, int curr_y, int new_x, int new_y) { calculate the slope using curr and next Space coordinates, return true if slope=1 false otherwise }

e. void get_random_coordinates() { generates a random number between 1-6 for x + y coordinates and saves as the next_space in the ai's Player dict, performs the checks to validate, and updates the ai's Player struct curr_space if valid  }

f. bool is_unobstructed_path(array[Space] path) {check each x+y coord in path against the other users current_space, returning true if no matches, false otherwise }

4. void update_level(bool is_ai, Space space) { if is_ai, ensure the Space level isnt 0 and decrease level, else if its the user ensure the current Space level isnt 4 and increase by 1 }
a. update_score(bool increase) {simply use the boolean argument to increase or decrease the score}

5. void show_winner_message() {if the name of the Player with 10 points is not "ai"}