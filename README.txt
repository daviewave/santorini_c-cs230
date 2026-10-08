Santorini (230 version) - CS230 Project 1
=========================================

Overview
--------
Santorini.c is a single-file C99 program in which a human player (P) plays the
230-version Santorini rules against a deterministic computer opponent (A) on a
6x6 board of building levels. The board is an int board[6][6] holding the
level (0..4) of every space; the two builders are int[2] arrays holding a row
and a column. Every function takes the board and the builder positions as
array arguments, there are no global variables, no structs and no pointer
variables (the only address-of is inside scanf). The player types moves as
"row column" (for example "2 4"); off-board, non-adjacent, occupied and
non-numeric inputs are explained and re-prompted, and end of input ends the
program cleanly. After each move the eight octagonal rays from the new space
are raised (player) or lowered (AI), stopping at the board edge or just
before the other builder, and never touching the landing space; levels are
clamped to 0..4. The board and the two decisive counts are printed after
every move; the game ends when ten spaces reach level 4 (Player wins!) or
level 0 (AI wins!), with a Draw! branch for both at once. The AI is a greedy
one-ply search: it simulates each legal neighbour and keeps the move that
drops the most spaces to 0, then the most 4s to 3, then lowers the most
spaces, so games are fully reproducible.

Build and run
-------------
    make            builds build/Santorini with the full warning set
    make run        builds and starts a game
    make test       builds and runs the unit tests and the e2e transcripts
    make check      runs gcc -fanalyzer over the source
    make dist       assembles the flat Gradescope bundle in dist/ and builds it

Plain course build (what the grader runs):

    gcc -std=c99 -Wall -o Santorini Santorini.c
    ./Santorini

Requirements map
----------------
Every bullet of the spec's "Requirements" and "Grading and Rubric" lists,
with the function in Santorini.c that satisfies it.

Spec requirements
  An array represents the game board ........ int board[BOARD_SIZE][BOARD_SIZE]
                                              in main; initialize_board
  Takes "row column" as user input .......... read_coordinates (scanf "%d %d"),
                                              prompt_player_start, prompt_player_move
  One or more functions are used ............ 30 functions, every one declared
                                              with a prototype at the top of the file
  Arrays are passed to functions ............ every board function takes
                                              int board[][BOARD_SIZE]; builder
                                              positions are int[2] parameters
                                              (e.g. classify_move, move_builder)
  Iteration traverses the board ............. initialize_board, count_level,
                                              copy_board, print_board,
                                              score_ai_move (nested for loops);
                                              update_rays / update_ray walk rays
  Determines the end state: win or lose ..... game_result, called after every
                                              move in main; announce_result
  An AI exists as described ................. choose_ai_move + score_ai_move
                                              (moves so that a level drops
                                              1 -> 0 or 4 -> 3 whenever possible);
                                              choose_ai_start picks the adjacent
                                              starting space
  Global variables minimised ................ zero global variables; all state
                                              lives in main and is passed down
  Shows the state after each move, final
  state and who won or drew ................. show_state (print_board +
                                              print_score) is called after the
                                              start, after every player move and
                                              after every AI move; announce_result
                                              prints "Player wins!", "AI wins!" or
                                              "Draw!" after the final board

Rubric: Delivery
  Data structures and algorithms for human
  vs AI with the described game flow ........ main (setup, then alternate
                                              prompt_player_move / play_ai_turn
                                              until game_result != RESULT_NONE)
  Takes "row column" input .................. read_coordinates
  Handles edge cases and invalid inputs,
  reprompting as necessary .................. classify_move returns MOVE_OFF_BOARD,
                                              MOVE_SAME_SPACE, MOVE_NOT_ADJACENT or
                                              MOVE_OCCUPIED; explain_invalid_move
                                              prints the reason; print_number_hint
                                              handles non-numeric lines; both
                                              prompt_* loops reprompt; INPUT_END
                                              (EOF) exits cleanly via end_of_input
  Determines end state ...................... game_result
  AI exists ................................. choose_ai_move, score_ai_move
  Shows state after each move, final state,
  who won or drew, with the recorded points,
  properly formatted and aligned ............ print_board reproduces the spec
                                              layout exactly ("   1 2 3 4 5 6",
                                              "1  2 P A 2 2 2"); print_score
                                              prints the level-4 and level-0
                                              counts; announce_result
Rubric: Design
  Functions declared and used properly ...... prototype block at the top; every
                                              helper is static; main only
                                              orchestrates
  Arrays passed to functions ................ see above
  Iteration traverses the board ............. see above
  Data structures and types used properly ... int board[6][6], int builder[2]
                                              indexed by ROW / COL, int reason
                                              and result codes named by #define
  Clear naming .............................. e.g. is_adjacent, update_rays,
                                              move_builder, game_result,
                                              score_ai_move, read_coordinates
  Globals minimised ......................... none
  Control flow used properly ................ while loops for reprompting and
                                              for the game, nested for loops for
                                              the board, switch for messages
  Algorithms clear, efficient, no extra
  looping or unreachable code ............... rays are walked once per
                                              direction; the AI evaluates at most
                                              8 candidates on a copied board;
                                              the Draw! branch is required by the
                                              rubric wording (see Design notes)
  No structs or pointers .................... none; only arrays, ints and chars
Rubric: Coding style ........................ K&R braces on every if/for/while/
                                              switch, 4-space indentation, one
                                              statement per line
Rubric: Comments ............................ every function has a header
                                              comment stating its purpose and
                                              its non-obvious parameters/return;
                                              every #define group is documented;
                                              the AI scoring is explained here
                                              and in the score_ai_move comment
Rubric: README .............................. this file
Rubric: Video ............................... see the last line of this file

Design notes
------------
AI strategy. choose_ai_move scans the up to eight neighbours of the AI's
builder in row-major order, skips illegal ones (classify_move), simulates each
legal move on a copy of the board (copy_board + update_rays with AI_DELTA,
blocked by the player's builder) and scores the result as
    10000 * (spaces that dropped to 0)
    + 100 * (spaces that dropped from 4 to 3)
    +   1 * (spaces lowered at all).
The weights make the three counts a lexicographic key (no count can exceed 35),
so the AI always takes a 1 -> 0 drop when one exists, otherwise a 4 -> 3 drop,
otherwise the move that lowers the most spaces. Ties go to the first candidate
in scan order. Nothing is random, so a given sequence of player moves always
produces the same game, which is what the test transcripts rely on.

AI starting space. The AI starts directly to the right of the player's
builder, or directly to the left if the player chose column 6. This
reproduces the spec's example (player at (1,2), AI at (1,3)).

Draw. A draw (ten 4s and ten 0s at once) cannot actually arise, because a
player move never creates a 0 and an AI move never creates a 4, and the game
is checked after every move. The branch exists because the rubric asks for
"who won or who drew"; it is one comparison in game_result.

Spec example. With the player starting at (1,2) and moving to (2,3), the
program prints the spec's board exactly, and the AI's first reply reproduces
the first board of the spec's "example of the game playing". In the spec's
second example board the space the player just left shows 4 where this
program shows 3; that figure is only consistent with raising the landing
space on the first move, which the spec's text forbids ("the level of the
octagon that builders move onto does not increase/decrease"), so the text
rule is followed.

Input. read_coordinates calls scanf("%d %d") once and then discards the rest
of the line, so each prompt consumes one line, trailing words are ignored,
and a line like "a b" is reported and re-prompted instead of looping forever.
EOF at any prompt prints "End of input: the game was abandoned." and exits 0.

Tests. test/unit/test_santorini.c includes Santorini.c directly and checks
every function (380 checks), including the spec's worked example board and
the blocking rule. test/e2e/cases/ holds stdin scripts with golden transcripts
for the spec example, invalid inputs, end of input, a complete game the
player wins and a complete game the AI wins. Run them with make test.

Video: <VIDEO URL TO BE ADDED>
