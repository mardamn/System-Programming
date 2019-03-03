#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#define BUFSIZE 4096

void cutter(char s[], int i, int n)
{
    int j;
    int len;

    if ((s == NULL) || (i == 0))
        return;
    len = strlen(s);
 if (i + n > len)
        n = len - i;
    for (j = 0 ; (j < n) && (s[j + i - 1] != '\0') ; j++)
        s[j + i] = s[j + i + n];
}

int main(int argc, char **argv) {	
char buf[BUFSIZE];
        int n;
        int o1 = open(argv[1], O_RDONLY);

        while((n=read(o1, buf, BUFSIZE))>0) {
   
	int x = atoi(argv[2]);
	int y = atoi(argv[3]); 
	char *s= buf;

    cutter(s, x, y);

	        if(x > y ){
        printf("Out of bounds\n");
         } else {
    printf("%s\n", s); }
    return 0;
}}
