#include "tests.h"
#include "math.h"

#include "../../src/patterns/patterns.c"

/*

    Test bit depth with powers of 2

    bit_depth will only be used on resolutions which are powers of 2

*/
int test_bit_depth() {
    int r = 0;

    for(int n = 0; n < 16; n++) {
        int power = (int)pow(2,n);
        r |= assertEquals(bit_depth(power), n);
    }

    return r;
}

int main() {
    int r = 0;

    r |= test_bit_depth();

    return r;
}