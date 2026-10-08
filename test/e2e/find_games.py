#!/usr/bin/env python3
"""Finds complete games for the e2e cases by driving build/Santorini.

Default mode searches for a game the player wins: every step reruns the binary
with the moves found so far, parses the last printed board, and picks the
player move that maximises the number of level-4 spaces, with a one-move
lookahead through the AI's (deterministic) reply when no move wins outright.
`--lose` instead always moves to the first legal neighbour in row-major order,
which lets the AI win. Both modes print the move list, one "row column" line
per move, and exit 0; they exit 1 when no game is found within MAX_MOVES.

Run from the repository root: python3 -I test/e2e/find_games.py [--lose]
"""
import subprocess
import sys

BINARY = "build/Santorini"
SIZE = 6
LEVEL_MAX = 4
HEADER = "   1 2 3 4 5 6"
MAX_MOVES = 200
START = "1 1"


def run_game(moves):
    """Returns the program's stdout for the given list of "row column" lines."""
    stdin = "".join(move + "\n" for move in moves)
    completed = subprocess.run([BINARY], input=stdin, capture_output=True, text=True, timeout=5)
    return completed.stdout


def parse_last_board(output):
    """Returns (levels, player, ai) from the last board printed; positions are 0-based."""
    lines = output.splitlines()
    start = max(i for i, line in enumerate(lines) if line.endswith(HEADER))
    levels = [[0] * SIZE for _ in range(SIZE)]
    player = ai = None
    for row in range(SIZE):
        cells = lines[start + 1 + row].split()[1:]
        for col, cell in enumerate(cells):
            if cell == "P":
                player = (row, col)
            elif cell == "A":
                ai = (row, col)
            else:
                levels[row][col] = int(cell)
    return levels, player, ai


def result_line(output):
    last = output.rstrip("\n").splitlines()[-1]
    return last if last.endswith("wins!") or last == "Draw!" else None


def neighbours(position, other):
    row, col = position
    for r in range(row - 1, row + 2):
        for c in range(col - 1, col + 2):
            if 0 <= r < SIZE and 0 <= c < SIZE and (r, c) != position and (r, c) != other:
                yield (r, c)


def raise_rays(levels, destination, blocker):
    """Returns a copy of levels after a player move to destination."""
    raised = [row[:] for row in levels]
    for dr in (-1, 0, 1):
        for dc in (-1, 0, 1):
            if dr == 0 and dc == 0:
                continue
            r, c = destination[0] + dr, destination[1] + dc
            while 0 <= r < SIZE and 0 <= c < SIZE and (r, c) != blocker:
                raised[r][c] = min(LEVEL_MAX, raised[r][c] + 1)
                r, c = r + dr, c + dc
    return raised


def count_fours(levels):
    return sum(cell == LEVEL_MAX for row in levels for cell in row)


def best_follow_up(levels, player, ai):
    """The most level-4 spaces one further player move can reach."""
    return max(count_fours(raise_rays(levels, to, ai)) for to in neighbours(player, ai))


def as_input(position):
    return "%d %d" % (position[0] + 1, position[1] + 1)


def choose_winning_move(moves, levels, player, ai):
    """Picks a move that wins now, else the one whose position after the AI
    reply leaves the best next move (ties broken by total level, then order)."""
    candidates = list(neighbours(player, ai))
    for to in candidates:
        if count_fours(raise_rays(levels, to, ai)) >= 10:
            return to
    best = None
    for to in candidates:
        output = run_game(moves + [as_input(to)])
        if result_line(output) == "AI wins!":
            continue
        next_levels, next_player, next_ai = parse_last_board(output)
        score = (best_follow_up(next_levels, next_player, next_ai), sum(map(sum, next_levels)))
        if best is None or score > best[0]:
            best = (score, to)
    return best[1] if best else candidates[0]


def choose_losing_move(moves, levels, player, ai):
    return next(iter(neighbours(player, ai)))


def search(start, choose, wanted):
    moves = [start]
    while len(moves) <= MAX_MOVES:
        output = run_game(moves)
        if result_line(output) == wanted:
            return moves
        if result_line(output) is not None:
            return None
        levels, player, ai = parse_last_board(output)
        moves.append(as_input(choose(moves, levels, player, ai)))
    return None


def main():
    if "--lose" in sys.argv[1:]:
        moves = search(START, choose_losing_move, "AI wins!")
    else:
        moves = search(START, choose_winning_move, "Player wins!")
    if moves is None:
        print("no game found within %d moves" % MAX_MOVES, file=sys.stderr)
        return 1
    print("\n".join(moves))
    return 0


if __name__ == "__main__":
    sys.exit(main())
