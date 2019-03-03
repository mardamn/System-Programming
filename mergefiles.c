#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFSIZE 4096

int main(int argc, char **argv) {
	char buf[BUFSIZE];
	char buf1[BUFSIZE];
        int n,m;
        int o1 = open(argv[1], O_RDONLY);
        int o2 = open(argv[2], O_RDONLY);
	int o3 = open(argv[3], O_WRONLY | O_APPEND | O_TRUNC);

	if( o1== -1 || o2 == -1 ) {

	printf("file does not exist\n");
	exit(0);
	
}	
        while((n=read(o1, buf, BUFSIZE))>0) {
        write(o3, buf, n);

      
}	while((m=read(o2, buf1, BUFSIZE))>0) {
        write(o3, buf1, m);

}
	close(o1);
	close(o2);
	close(o3);

}
