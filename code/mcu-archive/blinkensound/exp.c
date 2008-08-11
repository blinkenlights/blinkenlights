#include <stdio.h>
#include <math.h>

int main (void) {

	int i;

	for (i = 0; i <= 255; i++) {
		printf ("%d, ",
                        (int) (255.0 * (exp (((double)i-255.0)/32.0) / (exp (0.0)))));
	}

        return 0;
}
