#include <stdio.h>
#include <ctype.h>
#include <conio.h>

#ifndef __STDC__
int main(argc, argv) 
    int argc;
    char **argv;
#else
int main(int argc, char **argv)
#endif
{
	int c;
	while(1) {
		c=getch();
		if(isprint(c)) {
			printf("- %02xH (%3d) %c\n",c,c,c);
		} else if(c>=0 && c<' ') {
			printf("- %02xH (%3d) ^%c\n",c,c,c+'A'-1);
		} else {
			printf("- %02xH (%3d)\n",c,c);
		}
		if(c=='q') {
			break;
		}
	}
	return 0;
}
