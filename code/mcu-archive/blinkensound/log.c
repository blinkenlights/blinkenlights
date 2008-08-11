#include <stdio.h>
#include <math.h>

int main (void) {

	int i;

	for (i = 1; i < 17; i++) {
		printf ("%d, ", (int) (256.0 * ((exp ((double) i / 5) - 1.0) / (exp (3.4) - 1))    ));
	}

        return 0;
}
