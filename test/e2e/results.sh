#!/usr/bin/env bash
# Plays the two complete games in cases/ and asserts their outcomes: the
# result line, the decisive counts on the final board, and the absence of a
# draw or an abandoned game. Independent of the golden transcripts so a
# regenerated .expected file cannot silently encode a wrong result.
set -euo pipefail
cd "$(dirname "$0")/../.."

binary=build/Santorini
mkdir -p build/e2e
status=0

fail() {
    echo "results: $1"
    status=1
}

# Prints the six rows of the last board; the header may share its line with
# the move prompt, which prints no newline.
final_board() {
    tac "$1" | sed -n '0,/   1 2 3 4 5 6$/p' | tac | sed -n '2,7p'
}

# check_game <case> <result line> <digit> : the digit must fill at least ten
# cells of the final board.
check_game() {
    local name=$1 wanted=$2 digit=$3
    local input="test/e2e/cases/$name.in" actual="build/e2e/$name.results.out"
    if [ ! -f "$input" ]; then
        fail "$name: missing $input"
        return
    fi
    if ! timeout 5 "$binary" <"$input" >"$actual"; then
        fail "$name: exited non-zero"
        return
    fi
    if [ "$(tail -n 1 "$actual")" != "$wanted" ]; then
        fail "$name: last line is not '$wanted'"
    fi
    local cells
    cells=$(final_board "$actual" | cut -c4- | tr -cd "$digit" | wc -c)
    if [ "$cells" -lt 10 ]; then
        fail "$name: final board has $cells cells at level $digit, expected at least 10"
    fi
    if grep -q 'Draw!' "$actual"; then
        fail "$name: reports a draw"
    fi
    if grep -q 'End of input' "$actual"; then
        fail "$name: the game was abandoned"
    fi
}

check_game player_wins 'Player wins!' 4
check_game ai_wins 'AI wins!' 0

exit $status
