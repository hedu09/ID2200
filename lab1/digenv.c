#include <stdio.h>
#include "stdlib.h"
/* ny ändring */
int main(int argc, char **argv, char **envp)
{
	int i;
	for (i = 0; envp[i] != NULL; ++i)
	{
		printf("%d:%s\n", i , envp[i]);
	}
	return 0;/*hej*/
}
