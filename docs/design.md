# Santorini (230 version): design

This document records the data structures, the algorithms and every decision a
reviewer could question, with the reason. The spec is `docs/spec.md`; the
project-wide contract is `docs/conventions.md`. The code carries no rationale;
it carries one-line pointers here where a decision is not obvious.

## 1. Constraints that shape everything

The spec restricts the submitted file to first-week constructs and says
"No use of disallowed C constructs such as structs or pointers." The rubric
also says "no global variables is the best." So:

- **One file**, `src/Santorini.c`, zero file-scope variables.
- **No `struct`, no `enum`, no pointer variables.** Positions are two-element
  `int` arrays, the board is a two-dimensional `int` array, and both are passed
  to functions as array parameters ("functions and arrays, mutability" is on
  the allowed list). The `&row` in `scanf("%d %d", &row, &col)` is the one
  address-of in the file; `scanf` is on the allowed list and that is how it is
  called.
- **Constants are `#define`s.** `enum` is not on the allowed list, and a
  file-scope `const int` cannot size a non-VLA array in C99, so `#define` is
  the only construct that both names the magic numbers and sizes the arrays.
- **No randomness.** The spec never asks for it, the AI is a deterministic
  greedy search, and determinism is what makes the end-to-end transcripts
  reproducible ("we can reproduce the game shown in your video").

## 2. Data representation

| Thing | Representation | Why |
| --- | --- | --- |
| Board | `int board[BOARD_SIZE][BOARD_SIZE]`, 0-based internally, value is the level 0..4 | Spec requires "an array must be used to represent the game board". Levels only; builders are not stored in the board, so placing a builder cannot change a level (spec: "choosing starting points won't change any building level"). |
| Builder position | `int builder[2]`, index `ROW` (0) and `COL` (1), 0-based | The only way to pass and return two coupled numbers without a struct. Functions that produce a position fill an `int out[2]` parameter. |
| Coordinates at the boundary | 1-based on screen and in input, converted once in `prompt_*` | The spec labels rows and columns 1..6; the code indexes 0..5. The conversion lives in exactly one place per direction. |
| Move verdict | `int` reason code: `MOVE_OK`, `MOVE_OFF_BOARD`, `MOVE_SAME_SPACE`, `MOVE_NOT_ADJACENT`, `MOVE_OCCUPIED` | Lets the prompt tell the player *why* a move was rejected (rubric: "tell the player if she chooses an invalid move"). |
| Game result | `int` code: `RESULT_NONE`, `RESULT_PLAYER`, `RESULT_AI`, `RESULT_DRAW` | One function decides the end state; `main` only dispatches on it. |
| Level change | `int delta`, `PLAYER_DELTA = +1`, `AI_DELTA = -1` | The player builds, the AI destroys; the same ray code serves both. |

Two-dimensional array parameters are written `int board[][BOARD_SIZE]` and
are never `const`-qualified: in C99 an `int (*)[6]` does not convert implicitly
to `const int (*)[6]`, and the build uses `-Wpedantic -Werror`. One-dimensional
position parameters that are read-only are `const int builder[2]`.

## 3. Algorithms

### 3.1 Adjacency and move legality

A move from `(r, c)` to `(r2, c2)` is legal when `(r2, c2)` is on the board,
is not `(r, c)` itself (the spec singles this case out: "her builder needs to
be moved"), `max(|r2 - r|, |c2 - c|) == 1` (the eight king-move neighbours,
which is what the spec calls "octagonal"), and the other builder is not
standing there. Levels never restrict movement in the 230 version.
`classify_move` checks these in that order and returns the first failing
reason, so the message the player sees names the first thing wrong with the
input.

### 3.2 Ray update after a move

After a builder lands on `(r, c)` every space on each of the eight rays from
`(r, c)` changes level by `delta`, with three rules taken straight from the
spec:

1. the landing space itself is not changed;
2. a ray stops at the board edge;
3. a ray stops *before* the other builder, which blocks the rest of that ray
   (the spec example: with A on (1,3), P moving to (1,2) leaves (1,3)..(1,6)
   untouched).

`update_ray` walks one direction given a `(row_step, col_step)` pair.
`update_rays` enumerates the eight directions with two nested loops over
`-1..1` that skip `(0, 0)`; this needs no direction table and therefore no
file-scope array. Every write goes through `clamp_level`, so the board can
never hold a level outside 0..4 (spec: "should not contain any building with
a building level more than 4 or less than 0").

### 3.3 End of game

`count_level(board, level)` counts spaces at one level by iterating the whole
board. `game_result` reports `RESULT_PLAYER` when at least `WIN_COUNT` (10)
spaces are at level 4, `RESULT_AI` when at least 10 are at level 0,
`RESULT_DRAW` when both hold, else `RESULT_NONE`. It is called after every
move by either side.

A draw cannot actually arise: a player move only raises levels, so it cannot
create a tenth zero, and the game would already have ended if ten zeros
existed before the move; symmetrically for the AI. The branch exists because
the rubric asks to "display ... who won or who drew", and it costs one
comparison. The README says the same thing so a grader does not mistake it
for a missing case.

### 3.4 The AI

Deterministic greedy one-ply search ("at least as smart as a 5 year old"):

1. Enumerate the eight neighbours of the AI builder in row-major scan order
   and keep the legal ones (`classify_move == MOVE_OK`).
2. For each candidate, copy the board into a local trial board, apply
   `update_rays` with `AI_DELTA`, blocked by the player, and score the result.
3. Score = `SCORE_TO_ZERO * (spaces that went 1 -> 0)`
   `+ SCORE_FROM_FOUR * (spaces that went 4 -> 3)`
   `+ (spaces lowered at all)`.
   The weights (10000, 100, 1) make the three counts a lexicographic key
   because no count can exceed 35.
4. Keep the first candidate with the strictly highest score; ties go to the
   earlier one in scan order.

Why these keys: dropping a space to 0 is the AI's win condition; dropping a 4
to 3 takes a point away from the player; lowering anything at all is progress.
This is exactly the behaviour the spec names ("decrease at least one building
level from 4 to 3 or 1 to 0 if possible"). Minimax was considered and rejected:
the spec says it is not required, and a one-ply search is simpler to read and
to test.

**AI starting space.** The AI starts in the space directly to the right of the
player's builder; if the player chose column 6 it starts directly to the left.
One of the two always exists. This rule is chosen because it reproduces the
spec's worked example (player at (1,2) gives AI at (1,3)), which lets the
end-to-end suite replay that example byte for byte.

### 3.5 Input

`read_coordinates(out)` calls `scanf("%d %d", &row, &col)` once and then
always discards the rest of the line with a `getchar` loop, so each prompt
consumes exactly one line and a trailing token cannot leak into the next
prompt. Return values:

- `2` from `scanf`: both numbers read, returns 1 with the raw 1-based values.
- `0` or `1`: the offending token is still in the stream; the line drain
  removes it; returns 1 with `out` set to an off-board sentinel so the caller
  reports the input as invalid and reprompts. This is what makes letters
  safe instead of an infinite loop.
- `EOF`: returns 0. The caller prints a message and the program exits
  cleanly with `EXIT_SUCCESS`; no board state is corrupted.

Range checking (1..6) happens in `classify_move` via `is_on_board` after the
1-based to 0-based conversion, so a value like 0 or 7 reaches the same
"off the board" message as any other bad coordinate.

### 3.6 Output

The board printer reproduces the spec layout exactly:

```
   1 2 3 4 5 6
1  2 P A 2 2 2
```

Three spaces, then the column labels separated by single spaces; each row is
the row label, two spaces, then six cells separated by single spaces. A cell
shows `P` or `A` when a builder stands on it, else its level digit
(`cell_character`). After each board a score line reports the two counts that
decide the game (`Level-4 spaces (Player): n   Level-0 spaces (AI): m`); this
is the "recorded points" the rubric mentions. The last line of a game is one
of `Player wins!`, `AI wins!`, `Draw!`.

The board is printed at start, after the starting positions, after every
player move and after every AI move; the final board precedes the result line.

### 3.7 The spec's example sequence

With the player starting at (1,2) and moving to (2,3), the program prints the
spec's board exactly, and the AI's reply (to (2,4)) reproduces the first board
of the spec's "example of the game playing" cell for cell. After the player's
next move to (3,2) the spec's second board shows 4 on (2,3), the space the
player just left, where this program shows 3. For that cell to be 4 the level
under P at (2,3) must have been 3, which only happens if the reference
implementation raised the landing space on the first move; the spec's text
forbids that ("the level of the octagon that builders move onto does not
increase/decrease") and the spec's third board is consistent with the text
rule (the AI's vacated (2,4) shows 1, not 0). The text rule is implemented;
the figure is treated as a typo. `test/e2e/cases/spec_example` pins the part
that agrees.

## 4. Function map

| Function | Pure? | Responsibility |
| --- | --- | --- |
| `initialize_board` | writes board | every space to `START_LEVEL` |
| `is_on_board`, `is_adjacent`, `is_occupied_by` | yes | geometry predicates |
| `copy_board` | writes destination | the AI's trial board |
| `classify_move` | yes | legality with a reason code |
| `clamp_level` | yes | pin to 0..4 |
| `update_ray`, `update_rays` | writes board | section 3.2 |
| `move_builder` | writes board and position | sets the position, then `update_rays` |
| `count_level`, `game_result` | yes | section 3.3 |
| `score_ai_move`, `choose_ai_move`, `choose_ai_start` | yes (fills `out[2]`) | section 3.4 |
| `cell_character`, `print_board`, `print_score` | output | section 3.6 |
| `discard_rest_of_line`, `read_coordinates` | input | section 3.5 |
| `place_typed_start`, `apply_typed_move` | writes position (and board) | one attempt: validate, apply or explain |
| `prompt_player_start`, `prompt_player_move` | input + output | reprompt loops; return 0 on EOF |
| `play_ai_turn` | writes board and position | choose, move, report |
| `print_welcome`, `print_number_hint`, `explain_invalid_move`, `end_of_input`, `announce_result` | output | messages |
| `main` | | setup, then alternate turns until `game_result != RESULT_NONE` |

Every function other than `main` is `static` and declared in a prototype block
at the top of the file; definitions are grouped under section banners (Board,
Geometry and move legality, Building levels, End of the game, The AI, Display,
Input, Messages and turns, Entry point) in the same order as the prototypes.

## 5. Testing strategy

- **Unit tests** (`test/unit/test_santorini.c`) include `src/Santorini.c`
  with `main` renamed, so every `static` function is called directly. They
  cover geometry, clamping, each ray rule (edge stop, blocker stop, landing
  space untouched), the spec's worked example board, win/lose/draw detection,
  AI scoring keys and tie-breaking, the AI start rule, cell formatting, and
  `read_coordinates` against a temporary file substituted for `stdin`
  (good input, letters, a partial line, EOF).
- **End-to-end tests** (`test/e2e/`) run the built binary under `timeout`
  with stdin from `cases/*.in` and diff stdout against `cases/*.expected`.
  Cases: the spec's worked example, each invalid input class (off-board,
  not adjacent, occupied, letters, partial line, EOF at start, EOF mid-game),
  a complete game the player wins and a complete game the AI wins. Golden
  transcripts are generated once from the program and then hand-checked at the
  decisive boards; the result line and the board counts are also asserted with
  `grep` so a regenerated golden cannot silently encode a wrong result.
- **Static analysis**: `make check` runs `gcc -fanalyzer`; sanitizers and
  valgrind are not available on this machine (see `docs/conventions.md`).

## 6. The struct-based prototype

The original attempt (`docs/prototype/game.h`, `docs/prototype/main.c`,
`docs/prototype-notes.md`, `docs/prototype-requirements.md`) modelled the
board as a 7x7 array of `Space` structs with a label row and column, tracked
players in `Player` structs, chose the AI's moves at random and allowed
multi-space straight-line moves. It is kept for reference only: structs,
pointers and randomness are outside the spec's allowed constructs, and the
movement rule it implemented is not the 230 version's one-space move. What
survives is the structuring taste: pseudo-code first, small named helper
functions, one concern per function.

## 7. Deviations from the conventions document

- Two-dimensional array parameters are not `const` (section 2, C99 rule).
- No `enum` constants; `#define` is used instead (section 1, spec allow-list).
- `size_t` is not used for indices: every index is a small `int` that is also
  compared with signed steps (`-1`), and `size_t` is not on the first-week
  allow-list. All loops are bounded by `BOARD_SIZE`.
