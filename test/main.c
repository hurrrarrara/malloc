#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    char *ptr = malloc(100);

	   if (!ptr)
	{
	       perror("malloc");
	       return 2;
	   }


    strcpy(ptr, "Hello from custom malloc!\n\0");
	write(1, ptr, 27);

	char *a,*b;
	a = NULL;
	b = NULL;
	for (int i = 0; i < 1000; i++) {
		a = malloc(10 + (1 * 11 & 71));
		free(b);
		b = NULL;
		b = a;
	};
	if (a != NULL)
		free(a);
	if (b != NULL)
		free(b);

    free(ptr);

    return 0;
}
