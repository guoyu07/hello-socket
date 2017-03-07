#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

int main()
{
	union {
		int16_t s;
		char c[2];
	} un;

	un.s = 0x0102;
	printf("host byte order：0x0102 => %d-%d\n", un.c[0], un.c[1]);
	un.s = htons(0x0102);
	printf("net byte order：0x0102 => %d-%d\n", un.c[0], un.c[1]);

	return 0;
}
