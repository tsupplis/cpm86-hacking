#include <stdio.h>
int main(int argc, char ** argv)
{
	float x, y;
	float ca, cb;
	float a, b, t;
	int i;

	for (y = -12; y <= 12; ++y) {
		for (x = -39; x <= 39; ++x) {
			ca = x * 0.0458;
			cb = y * 0.08333;
			a = ca;
			b = cb;
			for (i = 0; i <= 15; ++i) {
				t = a * a - b * b + ca;
				b = 2 * a * b + cb;
				a = t;
				if ((a * a + b * b) > 4) {
					break;
				}
			}
			if (i < 9) {
				putchar('0' + i);
			} else if (i < 16) {
				putchar('A' + i - 10);
			} else {
				putchar(' ');
			}
		}
		putchar('\n');
	}
}
