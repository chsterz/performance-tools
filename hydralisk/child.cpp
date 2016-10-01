#include <iostream>
#include <unistd.h>

int main(int argc, char *argv[])
{
	float cur = 0;
	for (int i = 0; i < 1000; i++) {
		cur += 0.1;
		sleep(1);
		write(STDOUT_FILENO, &cur, sizeof(cur));
	}
}
