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
    freopen("con:", "r", stdin);
    freopen("con:", "w", stdout);
    fprintf (stderr,"INF: Keyboard Scanner. type letter 'q' to leave ...\n");
	while(1) {
		c=getch();
		if(isprint(c)) {
			fprintf(stderr, "INF: 0x%02x [%-3d] -> %c\n",c,c,c);
		} else if(c>=0 && c<' ') {
			fprintf(stderr, "INF: 0x%02x [%-3d] -> ^%c\n",c,c,c+'A'-1);
		} else {
			fprintf(stderr, "INF: 0x%02x [%-3d]\n",c,c);
		}
		if(c=='q') {
            fprintf (stderr,"INF: Bye ...\n");
			break;
		}
	}
	return 0;
}
