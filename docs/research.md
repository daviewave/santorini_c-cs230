# Research notes

Four topics were researched before writing `src/Santorini.c`. Each section lists
the sources actually consulted, the decisions the implementation follows, and
the bugs it must not contain. Claims marked "verified locally" were checked
with small throwaway programs compiled with `gcc 16.2 -std=c99` and the
conventions' full warning set (`-Wall -Wextra -Wpedantic -Wshadow
-Wstrict-prototypes -Wmissing-prototypes -Wconversion -Wvla -Werror`).

## 1. Safe `scanf("%d %d")` input loops in C99

### Sources

- https://port70.net/~nsz/c/c99/n1256.html (C99 draft N1256, 7.19.6.2
  paragraphs 9, 10 and 16): "the first character, if any, after the input item
  remains unread"; a conversion whose result cannot be represented in the
  object is undefined behaviour; return value is EOF only if input failure
  happens before any conversion, otherwise the number of items assigned.
  Also 6.5.3.2 paragraph 3: "the unary & operator yields the address of its
  operand".
- https://en.cppreference.com/w/c/io/fscanf : matching failure vs input
  failure, the `*` assignment-suppression flag, `%[^\n]` never skipping leading
  whitespace and failing on an empty line, and a whitespace character in the
  format consuming any amount of whitespace including newlines.
- https://man7.org/linux/man-pages/man3/scanf.3.html : return-value wording
  for glibc, and the caveat that scanf "cannot tell newlines from other
  whitespace", which matters for line-buffered stdin.
- https://c-faq.com/stdio/scanfprobs.html (FAQ 12.20) and
  https://c-faq.com/stdio/scanfinterlace.html (FAQ 12.18): why scanf on
  interactive stdin is fragile and why a trailing newline is left behind.
- https://c-faq.com/stdio/stdinflush.html and
  https://c-faq.com/stdio/stdinflush2.html (FAQ 12.26, 12.26b): `fflush(stdin)`
  is undefined; the portable discard loop is
  `while ((c = getchar()) != '\n' && c != EOF) ;`.
- https://cmu-sei.github.io/secure-coding-standards/sei-cert-c-coding-standard/recommendations/integers-int/int05-c
  (SEI CERT INT05-C): `%d` on out-of-range input is undefined; the robust fix
  is `fgets` + `strtol`, which this project cannot use (`strtol` needs a
  `char **` end pointer, and the course rule forbids pointer variables).

### Practices adopted

1. One reader function, `read_move(int out[2])`, owns all of stdin. It calls
   `scanf("%d %d", &out[0], &out[1])` once per attempt and returns a status
   code: got two numbers, got garbage, or hit EOF.
2. The return value is compared against exactly three cases: `EOF` (input
   failure before any conversion), `2` (both numbers read) and anything else
   (0 or 1: a matching failure on one of the tokens). Only `2` is success.
3. After every call, regardless of outcome, the rest of the line is drained
   with the FAQ 12.26b `getchar` loop. This removes the token that caused a
   matching failure (which the standard guarantees is still unread) and also
   removes extra tokens on the same line, so "2 4 7" is treated as "2 4".
4. The drain loop reports whether it hit EOF, so "9 9" followed by end of file
   is still handled as a complete input, and the next read sees EOF cleanly
   (verified locally: last line without a newline parsed, then EOF returned).
5. `scanf("%*[^\n]")` is not used for draining: on an empty line it is a
   matching failure that consumes nothing and leaves the newline in place, so
   a second call (`%*c`) is required, and the pair is harder to reason about
   than one `getchar` loop (verified locally: `drain r=0`, `nl r=0`).
6. EOF or a read error on stdin ends the program with a one-line message on
   stdout ("Input ended; game aborted.") and `EXIT_FAILURE`; it never spins.
7. Range validation (1..6 for both numbers) happens after the read, in a
   separate pure function, so the reader does not know about the board.
8. `&out[0]` is the unary address-of operator applied to an array element; the
   course lists `scanf` as allowed, and the spec's own example of taking "row
   column" implies it. No pointer variable is ever declared; the only `&` in
   the file is inside `scanf` arguments.
9. The prompt is printed before each read and the invalid-input message
   names what was wrong (not a number, off the board, occupied, not adjacent),
   then re-prompts.

### Pitfalls avoided

- Infinite loop on non-numeric input: without draining, `scanf("%d")` fails
  on `x` forever because `x` stays in the stream (N1256 7.19.6.2p9). Every
  path drains the line.
- Treating `0` or `1` as EOF: those are matching failures, not input
  failures; only `EOF` means the stream is gone.
- Silent wait on a half line: because `" "` in the format eats newlines, "2"
  then Enter makes scanf block for the second number without a new prompt
  (verified locally: "2\n3\n" reads as one move). The prompt text says "row
  column" explicitly; the e2e cases always provide both numbers on one line.
- `fflush(stdin)`: undefined by the standard (FAQ 12.26); never used.
- `%d` overflow: undefined per 7.19.6.2p10; glibc wraps modulo 2^32 (verified
  locally: 99999999999 read as 1215752191). The 1..6 range check rejects the
  wrapped value in practice, and the rubric guarantees two plain numbers, so
  this is an accepted residual risk documented here rather than fixed with
  `strtol`.
- Mixing `scanf` with `fgets`/`getchar` for data: only `getchar` is used, and
  only for discarding, so FAQ 12.18's leftover-newline bug cannot occur.

## 2. Grid ray-casting with blocking on a small board

### Sources

- https://en.wikipedia.org/wiki/Moore_neighborhood : the eight cells at
  Chebyshev distance 1 are the "octagonal" neighbours of the spec; adjacency
  is `max(|dr|, |dc|) == 1`.
- https://www.chessprogramming.org/Classical_Approach : sliding attacks are
  generated ray by ray; the first blocker on a ray cuts off everything beyond
  it. There the blocker square is included (capture); here it is excluded
  (the other builder's space never changes).
- https://www.chessprogramming.org/Direction : the eight directions as unit
  steps; in a 2-D array the natural encoding is the (row step, column step)
  pair rather than a mailbox offset.
- /var/home/slave/ai/cs230/docs/specs/project1.md : the worked example
  (P moves to (2,3) with A at (1,3)) is the acceptance test for this code.

### Practices adopted

1. Directions are enumerated with two nested loops `for dr in -1..1, for dc in
   -1..1`, skipping `dr == 0 && dc == 0`. No direction table is needed, there
   is nothing to keep in sync, and the construct is first-week C.
2. Adjacency test is a pure function `is_adjacent(r1, c1, r2, c2)` returning
   `|r1-r2| <= 1 && |c1-c2| <= 1 && !(same cell)`, written with plain
   comparisons rather than `abs` so no extra header is needed.
3. Ray walking: start at `(r0 + dr, c0 + dc)`, loop while the cell is inside
   the board and is not the other builder's cell, update it, then step. The
   origin is excluded by construction (the first visited cell is one step
   away), and the blocker is excluded because the loop condition fails on it.
4. `is_inside(r, c)` is its own function, used by the ray walk and by move
   validation, so the 0..5 bound appears once.
5. Level update is clamped in a separate function `clamp_level`, so the ray
   walker only says "+1" or "-1" and never produces 5 or -1.
6. Board indices are 0-based internally; the conversion from the 1-based
   "row column" the user types happens once, in the input layer, and the
   reverse conversion once, in the display layer.
7. The spec's worked example is a unit test: a fresh board, P at (2,3), A at
   (1,3), raise rays, expected board copied from the spec. Verified locally
   that the nested-loop walker produces exactly that board, including the
   unchanged (1,4)..(1,6) behind the blocker and the unchanged destination.

### Pitfalls avoided

- Updating the origin cell: starting the walk at the origin and skipping it
  with a flag is the classic off-by-one; starting one step out avoids it.
- Updating the blocker cell or continuing past it: the loop condition checks
  the blocker before the update, not after.
- Only blocking when the builder is adjacent: the other builder blocks at any
  distance along the ray, so the check is inside the loop, not before it.
- Off-board step before the bounds check: `is_inside` is evaluated on the
  candidate cell before it is read or written.
- Confusing row and column steps: the pair is always written `(dr, dc)` and
  applied as `r += dr; c += dc;` in that order, in one place.
- Stale builder positions: the ray walk receives the mover's new position and
  the opponent's position as arguments; it never derives them from the board,
  because the board holds levels only and the builders are not stored in it.

## 3. Greedy one-ply evaluation for a simple game AI

### Sources

- https://en.wikipedia.org/wiki/Minimax : depth-limited search scores leaf
  nodes with a heuristic; at depth 1 every child of the root is a leaf, so
  the search reduces to "score each immediate move, pick the best".
- https://www.chessprogramming.org/Ply : a ply is one side's move; one ply is
  exactly the AI's own move with no reply considered.
- https://www.chessprogramming.org/Evaluation : hand-written evaluations are
  weighted sums of features in which a dominant feature (material) outweighs
  all lesser ones combined, which is what makes a weighted int behave like a
  lexicographic key.
- https://en.wikipedia.org/wiki/Greedy_algorithm : locally optimal choice,
  never reconsidered; not globally optimal in general.
- https://en.wikipedia.org/wiki/Horizon_effect : what a shallow search cannot
  see, and why this is acceptable here.
- /var/home/slave/ai/cs230/docs/c-conventions.md section 6: e2e transcripts
  must be deterministic; project 1 has no seed because its AI has no
  randomness.

### Practices adopted

1. The AI is minimax at depth 1 with the maximising side only: for each of the
   up to 8 adjacent cells that are inside the board and not the player's
   cell, copy the board, apply the AI move (lower all rays), and score the
   result. Keep the best. The spec's "at least as smart as a 5 year old"
   requirement is "move so that at least one level drops 1 -> 0 or 4 -> 3 if
   possible", which the primary and secondary keys encode directly.
2. The score is a single `int` built as
   `score = 10000 * drops_to_zero + 100 * drops_from_four + lowered_total`,
   which orders moves exactly as the tuple (primary, secondary, tertiary)
   would: no count can exceed the 35 other cells of the board, so the
   tertiary count is below 100 and `100 * 35 + 35 = 3535` is below 10000.
   The weights are named constants (`SCORE_TO_ZERO`, `SCORE_FROM_FOUR`) and
   the bound is restated in `design.md`.
3. The scan visits candidates in the same nested `dr`/`dc` order used for
   rays, so the candidate order is one fact about the program, not two.
4. Counts are computed by comparing the simulated board against the current
   board cell by cell inside the scoring function, not by instrumenting the
   ray walker.
5. Ties are broken by scan order: candidates are visited by the same nested
   `dr`/`dc` loops, and a candidate replaces the best only on strictly greater
   score. The first best in scan order wins; this is stated in `design.md`
   and relied on by the e2e transcripts.
6. The AI's starting cell does not use the scan: it is the cell directly to
   the right of the player's start, or directly to the left when the player
   chose column 6 (one of the two always exists). The spec says only that
   "AI chooses a space adjacent", so any adjacent cell is legal; this rule is
   picked because it reproduces the spec's illustration (P at (1,2), A at
   (1,3)) and so lets the e2e suite replay the spec's worked example verbatim.
7. Because nothing is random, there is no `rand`, no `srand`, no seed
   variable, and every `test/e2e/cases/*.in` produces one fixed transcript.

### Pitfalls avoided

- Scoring on the live board: the move is simulated on a copy, so a rejected
  candidate leaves no trace. The copy is a local `int copy[6][6]` filled by a
  `copy_board` function, not an assignment (arrays cannot be assigned).
- Ties flipping between runs: with `>=` instead of `>` the last candidate
  wins, which is still deterministic but contradicts the documented rule;
  with any randomness the transcripts would not be reproducible.
- Key bleeding: the weights are justified against the maximum of 20 changed
  cells per move; a change to the board size must revisit them.
- Counting the destination cell: the destination never changes, so it never
  contributes; the comparison-based counting makes this automatic.
- Horizon effect: a one-ply AI cannot see that its move opens a 4 for the
  player. This is accepted (the spec asks for 5-year-old play, not minimax),
  and the secondary key (4 -> 3) already prefers taking 4s away.
- Candidate generation uses the same `is_legal_move` function as the human's
  input validation, so both sides obey identical rules.

## 4. Writing testable C without structs or pointers

### Sources

- https://c-faq.com/aryptr/pass2dary.html (FAQ 6.18): a 2-D array passed to
  a function decays once, to a pointer to its first row; the parameter must
  carry the column count (`int a[][NCOLUMNS]`).
- https://en.cppreference.com/w/c/language/array and
  https://en.cppreference.com/w/c/language/function_declaration : array
  parameters are adjusted to pointers to the element type; qualifiers inside
  `[]` apply to that pointer; sizing with a `const int` object makes a VLA in
  C99.
- https://port70.net/~nsz/c/c99/n1256.html : 6.7.5.3p7 (parameter array
  adjusted to "qualified pointer to type"), 6.7.5.2p4 (non-constant size means
  variable length array), 6.6p6 (integer constant expressions are integer,
  enumeration and character constants, `sizeof`, and casts of floating
  constants; `const` objects are not in the list).
- https://c-faq.com/ansi/constasconst.html (FAQ 11.8): `const int n = 5;
  int a[n];` is not valid C because `const` means read-only, not constant;
  use `#define` or `enum`.
- https://cmu-sei.github.io/secure-coding-standards/sei-cert-c-coding-standard/recommendations/declarations-and-initialization-dcl/dcl06-c
  (SEI CERT DCL06-C): comparison of `const` objects, enumeration constants
  and macros; only the last two are compile-time constants.
- https://wiki.samba.org/index.php/Writing_cmocka_Tests and
  https://github.com/whot/unit-tests-for-static-functions : including the
  `.c` file into the test translation unit to reach `static` functions; the
  need to rename `main` and to not link the production object as well.
- /var/home/slave/ai/cs230/docs/c-conventions.md section 5: the exact include
  idiom this repository uses.

### Practices adopted

1. The board is `int board[BOARD_SIZE][BOARD_SIZE]` with
   `#define BOARD_SIZE 6`. A macro rather than `enum` because the same name
   also sizes the `%d` output loop and is explained to the grader as a plain
   constant; `enum` is reserved for small value sets (`LEVEL_MIN`,
   `LEVEL_MAX`, the reader's status codes). `const int` is never used as an
   array dimension (verified locally: it produces `-Wvla` under the
   conventions' flags, and the dimension becomes a run-time VLA).
2. Functions take the board as `int board[BOARD_SIZE][BOARD_SIZE]`. The
   parameter is adjusted to pointer-to-array-of-6-int, which is why the inner
   dimension must be written; the outer one is kept for documentation.
3. No `const` on 2-D array parameters. Passing `int[6][6]` to a
   `const int[6][6]` parameter is an ISO C constraint violation before C23
   (pointer-to-array types with different qualifiers), and gcc rejects it
   under `-Wpedantic -Werror` (verified locally). Read-only intent is stated
   in the function header comment and enforced by the unit test, which
   compares the board before and after calling each pure function.
4. `const` is used on 1-D array parameters (`const int pos[2]`), where
   `int *` to `const int *` is a valid implicit conversion (verified locally).
5. Two values come back through a small output array: `int out[2]` for a
   position, `int counts[3]` for the three score components. The function
   writes every element before returning so the caller never reads garbage.
6. Every function other than `main` is `static` and pure where possible:
   board in, counts or a verdict out, no I/O. The I/O functions
   (`print_board`, `read_move`, `announce_result`) contain no game logic.
7. The unit test compiles with
   `int program_main(void); #define main program_main
   #include "../../src/Santorini.c" #undef main`. The prototype line is
   required: without it `-Wmissing-prototypes -Werror` rejects the renamed
   `main` (verified locally). The test binary is built from the test file
   alone; `src/Santorini.c` is never also linked as an object.
8. Builders are not stored in the board. They are two `int pos[2]` locals in
   `main` (player, ai) passed down explicitly, which keeps the board a plain
   level grid and makes "the other builder blocks the ray" an argument rather
   than a search.
9. Board copying is a function (`copy_board(from, to)`), since arrays cannot
   be assigned and `memcpy` would pull in `<string.h>` and pointer thinking.

### Pitfalls avoided

- `int **` parameter for the board: a 2-D array does not convert to it (FAQ
  6.18; gcc error verified locally). Only the array-parameter form is used.
- `sizeof board` inside a function to derive the row count: after adjustment
  the parameter is a pointer, so `sizeof` gives the pointer size. The
  dimension is always `BOARD_SIZE`, never computed from a parameter.
- `const int BOARD_SIZE` at file scope: not an integer constant expression,
  so every `int board[BOARD_SIZE][BOARD_SIZE]` would be a VLA and a
  function parameter of that type would be invalid.
- Duplicate `main` or duplicate symbols in tests: the rename macro plus never
  linking the production object avoids both.
- Hidden global state making tests order-dependent: zero file-scope
  variables; every function receives everything it reads as a parameter.
