/* Unit tests for src/Santorini.c, included directly so static functions are reachable. */
int program_main(void);
#define main program_main
#include "../../src/Santorini.c"
#undef main
#include "check.h"

int main(void) {
    CHECK(BOARD_SIZE == 6);
    CHECK_REPORT("test_santorini");
}
