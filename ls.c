#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>

//--------------------------------------------------

int sort = 1;
int type = 1;
int count = 0;
int total = 0;
int inode = 0;
int humanRead = 0;
int format = 0;
int details = 0;
int details_n = 0;
int details_l = 0;
int listAll = 0;
int listDots = 0;
int noSort = 0;
int recursive = 0;
int exVariable = 0;
int directory =0;
int tim_t = 0;
int tim_u = 0;
int tim_c = 0;
long extraLength=0;
int maxFileSize=0;


//--------------------------------------------------

int compar_mtime(const struct dirent **d1, const struct dirent **d2) {

	struct stat buff1,buff2;
	stat((*d1)->d_name,&buff1);
	stat((*d2)->d_name,&buff2);
	if(buff1.st_mtime <= buff2.st_mtime)
	{
		if( buff1.st_mtime==buff2.st_mtime )			
			return sort * strcmp((*d1)->d_name,(*d2)->d_name);
		else

			return 1;    	
	} 

	else
		return -1;

}

int compar_atime(const struct dirent **d1, const struct dirent **d2) {

	struct stat buf1 , buf2;
	stat((*d1)->d_name,&buf1);
	stat((*d2)->d_name,&buf2);
	if(buf1.st_atime <= buf2.st_atime)
	{
		if( buf1.st_atime==buf2.st_atime )			
			return sort * strcmp((*d1)->d_name,(*d2)->d_name);
		
		else
			return 1;
		    	
	} 

	else	
		return -1;
	

}

int compar_ctime(const struct dirent **d1, const struct dirent **d2) {

	struct stat bf1 , bf2;
	stat((*d1)->d_name,&bf1);
	stat((*d2)->d_name,&bf2);
	if(bf1.st_ctime <= bf2.st_ctime)
	{
		if( bf1.st_ctime==bf2.st_ctime )	
			return sort * strcmp((*d1)->d_name,(*d2)->d_name);
		
		else
			return 1;
		   	
	} 

	else
		return -1;


}


int compar_size(const struct dirent **d1, const struct dirent **d2) {
	struct stat entry1,entry2;
	lstat((*d1)->d_name,&entry1);
	lstat((*d2)->d_name,&entry2);
	if(entry1.st_size<=entry2.st_size) {
		if(entry2.st_size == entry1.st_size)
			return (strcmp((*d1)->d_name,(*d2)->d_name));
		return 1;
	}
	else
		return -1;
}

int compar_lex(const struct dirent **d1, const struct dirent **d2) {

	return sort * strcmp((*d1)->d_name,(*d2)->d_name);

}


//--------------------------------------------------------------------------

int option(int argc,char **argv)
{ 


	int strsize = 0;
	int i, j;
	char *flag;

	for(i=0; i<argc; i++) {
		strsize += strlen(argv[i]);

		flag = malloc(strsize); 
	}
	for (i = 1; i < argc; i++) {
		strcat(flag, argv[i]);
		if (argc > i+1)
			break;
	}

	for(i=0; i<strlen(flag); i++) {

		if(flag[0] == '-') {

			if(flag[i] == 'r')  
				sort *=-1; 

			if(flag[i] == 'a') 
				listAll = 1; 	

			if(flag[i] == 'i'){
				exVariable = 1; 
				inode = 1; 
			}	

			if(flag[i] == 'A') 
				listDots=1; 

			if(flag[i] == 'd') 
				directory=1; 

			if(flag[i] == 'l'){ 
				details =1; 
				details_l =1; 
			}

			if(flag[i] == 'n'){ 
				details =1; 
				details_n =1; 
			}

			if(flag[i] == 't'){ 
				type =8; 
				tim_t = 1; 
			}

			if(flag[i] == 'S') 
				type =9; 

			if(flag[i] == 'u') 
				tim_u=1; 

			if(flag[i] == 'c') 
				tim_c = 1; 

			if(flag[i] == 'F'){ 
				exVariable = 1; 
				format =1; 
			}

			if(flag[i] == 'R')
				recursive = 1; 

			if(flag[i] == 'h') 
				humanRead =1; 

			if(flag[i] == 'f'){ 
				noSort =1; 
				listAll=1; 
			}

		}

	}
}


//------------------------------------------------------------------------

int isDir( char *file_path)
{
	struct stat s;
	stat(file_path, &s);
	return S_ISDIR(s.st_mode);
}


long numDigits(long n) {
	if (n < 10) return 1;
	return 1 + numDigits(n / 10);
}


//------------------------------------------------------------------------

void humanReadable(double size, char* buf) {
	const char* units[] = { "B", "K", "M", "G" };
	int i = 0;

	while (size > 1024) {
		size /= 1024;
		++i;
	}

	sprintf(buf, "%.*f%s", i, size, units[i]);
}


//------------------------------------------------------------------------

void printList(char*path,char *name){
	int index = 0;
	struct winsize w;
	struct dirent **namelist;
	struct passwd *pw;
	struct group *gr;
	struct tm* atime;
	int n,i,len,x;
	int max=0;
	char formatTime[100] = "";
	struct stat *buf, b2,dirstat;


	ioctl(0, TIOCGWINSZ, &w);
	int width = w.ws_col;
	buf = malloc(sizeof(*buf)*100);

	char *full = malloc(strlen(path)+strlen(name)+2);
	struct stat stats;
	strcpy(full,path);
	strcat(full,"/");
	strcat(full,name);
	//stat(nana, &stats);
	long * timing;
	lstat(full, buf);

	if(tim_u == 1 )  
		timing = &buf->st_atime; 
	else if(tim_c == 1 ) 
		timing = &buf->st_ctime; 
	else 
		timing = &buf->st_mtime; 

	atime = localtime(timing);
	strftime(formatTime, 100, "%b %d  %H:%M", atime);

	if(type == 1 || type == 8 || type == 9 || type == 10 || type == 11 || noSort==1) {

		pw = getpwuid(buf->st_uid);
		gr = getgrgid(buf->st_gid);


		if(inode==1){
			//printf("%d\n", extraLength);

			for (int i = 0; i < extraLength-numDigits((long)buf->st_ino)-1; ++i)
			{
				printf(" ");
			}

			printf("%ld ", (long) buf->st_ino);


		}
		if(details == 1) {
			printf( (S_ISDIR(buf->st_mode)) ? "d" : "-");
			printf( (buf->st_mode & S_IRUSR) ? "r" : "-");
			printf( (buf->st_mode & S_IWUSR) ? "w" : "-");
			printf( (buf->st_mode & S_IXUSR) ? "x" : "-");
			printf( (buf->st_mode & S_IRGRP) ? "r" : "-");
			printf( (buf->st_mode & S_IWGRP) ? "w" : "-");
			printf( (buf->st_mode & S_IXGRP) ? "x" : "-");
			printf( (buf->st_mode & S_IROTH) ? "r" : "-");
			printf( (buf->st_mode & S_IWOTH) ? "w" : "-");
			printf( (buf->st_mode & S_IXOTH) ? "x" : "-");

			char bh1[100];


			printf(" %3d ", (int)buf->st_nlink);

			if(details_n == 1)
				printf("%d  %d", pw->pw_uid, buf->st_gid);
			else if(details_l == 1) 
				printf("%s  %s",pw->pw_name, gr->gr_name);
			
			if(humanRead) {
				humanReadable(buf->st_size,bh1);
				printf(" %8s ", bh1);
			} 
			else {
				for (int i = 0; i < maxFileSize-numDigits((int)buf->st_size); ++i)
				{
					printf(" ");
				}

				printf("  %d",(int)buf->st_size);

			} 
			printf(" %s ", formatTime);


		}

		printf("%s", name); 

		if(exVariable==0) 
			printf(" ");

		if(format == 1 ) { 		
			char c;
			if(S_ISDIR(buf->st_mode))
			 	c = '/'; 
			else if((S_ISREG(buf->st_mode)))  { 
				if(buf->st_mode & 0111) 
					c = '*'; 
				else  
					c = ' '; 
			}
			else if(S_ISLNK(buf->st_mode))  
				c = '@'; 
			else if(S_ISSOCK(buf->st_mode))  
				c = '='; 
			else if(S_ISFIFO(buf->st_mode))  
				c = '|'; 
			printf("%c",c); 
		}
			if(details)
			 	printf("\n");

			free(buf);

		}

	}


//-------------------------------------------------------------

void process( char *path,struct dirent **namelist,int n){

	struct winsize w;                                  
	ioctl(0, TIOCGWINSZ, &w);                           
	int columns = w.ws_col;                             
	int rows = w.ws_row;                                
	int index = 0;
	struct stat bufPath,buf123, b2,dirstat;
	struct passwd *pw;
	struct group *gr;
	struct tm* atime;
	int i,x;
	int max=0;
	//char timeStr[100] = "";
	int len=0; 
	char *nana;
	int totalFiles=0;
	char *array[n]; 
	int arraynum=0;
	long extraFormat=0;
	char  *arrayForI[n];
	int extra=0;
	int varEx=0;
	int resize=0;
	index=0;
	while(n>index) { 
		char * name = namelist[index]->d_name;
		if(len<strlen(name)) {
			len=strlen(name); 
			nana=name;
		}
		if(listAll)  
			array[arraynum] = namelist[index]->d_name;
		 
		else if(listDots == 1 && !(namelist[index]->d_name[0] == '.' && (strlen(namelist[index]->d_name)<3))) 
			array[arraynum] = namelist[index]->d_name;
		

		else if(namelist[index]->d_name[0] != '.') 
			array[arraynum] = namelist[index]->d_name;

		else { 
			index++; 
			continue;
		}


		char *fullPath = malloc(strlen(path)+strlen(array[arraynum])+2);
		struct stat stats;
		strcpy(fullPath,path);
		strcat(fullPath,"/");
		strcat(fullPath,array[arraynum]);
		lstat(fullPath, &bufPath);
		count += bufPath.st_blocks;  totalFiles++;



		if(inode==1 && extraLength<numDigits((long) bufPath.st_ino)){
			//printf("%d\n", (int) bufPath.st_ino);

			extraLength = numDigits((long) bufPath.st_ino)+1;		
			extra=1;
			if(format==1) { varEx=1; }
			resize=7;

		}
		if(maxFileSize<numDigits((int)bufPath.st_size))
			maxFileSize = numDigits((int)bufPath.st_size);	

		if(format==1) { 
			extraFormat=1;
			resize=7;
		}

		index++; 
		arraynum++;

	}

	len=len+extraLength+extraFormat + resize ;
	//printf("%d\n",len );


	if(details) 	
		printf("total %d\n", count);
	
	count=0;

	int num =columns/(8*((len+7)/8));
	int filesize=(totalFiles+num-1)/num;
	int maxtab=(len+7)/8;

	if(details) {
		num=1;
		filesize=totalFiles;
	}



	int j=0;
	i=0;
	index = 0;

	for(j=0;filesize>j;j++){

		index=j;

		for(i=0;num>i;i++){
			if (totalFiles > index)
			{
				if(type == 1 || type == 8 || type == 9 || type == 10 || type == 11 || noSort==1) {

					printList(path,array[index]); 
					if(details==0){

						for (int t = 0; ((len-(strlen(array[index])+extraLength  + varEx)+7)/8)>t; t++) {
							printf("\t");
						}

					}
				}

				else if(humanRead==1) 
					printf("%s", array[index]);

			}

			index =index+filesize;

		}	
		if(details==0)
			printf("\n");
	}

	printf("\n");
} 

//--------------------------------------------

void listdirt(char*name,struct dirent **namelist2,int m)
{
	struct dirent **namelist3;
	process(name,namelist2, m);

	int ind=0;
	while (m>ind) {
		if(namelist2[ind]->d_name[0] != '.'){ 
			char *fpath = malloc(strlen(name)+strlen(namelist2[ind]->d_name)+2);
			struct stat stats;
			strcpy(fpath,name);
			strcat(fpath,"/");
			strcat(fpath,namelist2[ind]->d_name);
			stat(fpath, &stats);

			if (S_ISDIR(stats.st_mode)) {
				printf(" \n%s:\n", fpath);
				struct dirent **namelist3;
				int d;
				d = scandir(fpath, &namelist3, 0, compar_lex);
				listdirt(fpath,namelist3,d);
			} 
		}
		free(namelist3);
		ind++;
	}
	ind=0;
}


//--------------------MAIN------------------------

int main(int argc,char **argv){	

		struct dirent **namelist;	
		option(argc,argv);
		int n,i,x;
		long len;
		char *path;
		char  *argv2= argv[1];

		if(argc < 1)
		{
			exit(EXIT_FAILURE);
		}

		else 
		{  
			path=".";
			if(argc>1){
				char *argv2= argv[1];

				if(argv2[0] != '-')  
					path=argv2;
				if(argv2[0] == '-'&& argc>2)
					path=argv[2];
				else if(argv2[0] == '-'&& argc<3) 
					path=".";
			}
			char *memPath = malloc(strlen(path));
			struct stat stats;
			strcpy(memPath,path);
			stat(memPath, &stats);

			if (!S_ISDIR(stats.st_mode)) 
				printf("No such file or directory\n");


			if (type == 8 ) {

				if (tim_u == 1) { n = scandir(path, &namelist, 0, compar_atime);}
				else if (tim_c == 1) {n = scandir(path, &namelist, 0, compar_ctime);}
				else n = scandir(path, &namelist, 0, compar_mtime);// 

			} 
			else if (type == 9 ) 
				n = scandir(path, &namelist, 0, compar_size);
			else if (noSort == 1 ) 
				n = scandir(path, &namelist, 0, 0);
			else 
				n = scandir(path, &namelist, 0, compar_lex);

			if(n < 0) 
				exit(EXIT_FAILURE);
			else
			{
				if(directory)
					recursive=0;
				
				char*nm;
				nm="";

				if(directory==1) {
					printList(nm,path); 
					printf("\n");
				}
				else if(recursive==0) 
					process(path,namelist,n);
				else if(recursive==1)
					listdirt(path,namelist,n);

				free(namelist);
			} 
		}
		

	}



