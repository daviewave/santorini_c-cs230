#== Notes ==#

#- basic -#
- exactly 2 players per game where 1 is the user and the other is an automated (ai) player
- use 'P' for the user
- use 'A' for the ai 
- movements are made into adjacent spaces, such that the adjacent space is not occupied by another player
- the users player ALWAYS increases the building level 
- the ai player ALWAYS decreases the building level
- build levels range: 0-4

#- movement -#
- when moving, the player/ai will construct a 'building level' for each space that is before where the player stops
    -> movements can be in any octagonal direction (n,s,e,w,ne,nw,se,sw)
    -> must be in a straight line
    -> a movement path cannot be obstructed by another player

#- win scenarios -#
- the user wins after converting 10 spaces on the board to level 4
- the ai wins after converting 10 spaces on the board to level 0

#- game board -#
- a 2d list 
- 6x6 suspended grid
- the first row is a 'label' row specifying each column 1-6
- the first column is a 'label' column specifying each row 1-6
    -> this makes me think that the playable game board should stored in a different value that the full game board
- each playable space starts at 2
- ex: `
           1 2 3 4 5 6
        1  2 2 2 2 2 2
        2  2 2 2 2 2 2
        3  2 2 2 2 2 2
        4  2 2 2 2 2 2
        5  2 2 2 2 2 2
        6  2 2 2 2 2 2
    `

#- game play -#
- prompts user for starting spot (using coordinates)
- the ai's starting space will always be adjacent to the users - randomly select the space using (n,s,e,w,ne,nw,se,sw)
- after starting spaces selected, user makes first move
    -> movement prompting is done by '<row>,<column>' inputs from the user (this is a core functionality, aka individual function)
- after each move the score for each player is calculated based on num building levels == 4 for user and == 1 for ai, checking to see if either
    player has won