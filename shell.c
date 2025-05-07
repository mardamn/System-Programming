#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>

#define COMMAND_SIZE 50 

int plofand=0;
int size = 0;
int p_size = 0;
int shell = 1;
int buf = 1024;
char *paths[50];
pid_t pid;
int a=0;
int status;

int commandCd(char *args[]){        
   
    char*cmd=args[0];

    if(strcmp(cmd,"cd")==0){
       
        if(size<2){
            args[1]="/home";
        }
        if(chdir(args[1])!=0){
            perror("ERROR");
        }
        return 0;
    }
    return 1;
}

int breakIntoTokens(char *line, char *args[]){
    char * str;
    size = 0;
    str = strtok (line," \t");
    if(strcmp(str,"\n")==0)
        return 0;
        while (str != NULL){
            if(str[strlen(str)-1]=='\n'){
                str[strlen(str)-1] ='\0';
            }
    
            if(strcmp(str,"\n")!=0 && strcmp(str,"")!=0 && strcmp(str,"\t")!=0 && strcmp(str," ")!=0 && str!=NULL){
                args[size++] = str;
            }

            str = strtok(NULL, " \t");
        }
        args[size] = NULL;
        return 1;
}



void setPath(){

    char *p = getenv("PATH");
    char *str ;
    str = strtok(p ,":");
    while(str != NULL){
        paths[p_size++] = str;
        str = strtok(NULL ,":");
    }
}

char *fullPath(char *first){

    int i;
    char *p;
    for(i = 0 ; i < p_size ; i++){
        p = NULL;
        p = (char *) malloc(strlen(paths[i])+ strlen("/") + strlen(first) + 1);
        strcpy(p, paths[i]);
        strcat(p, "/");
        strcat(p, first);

        int acc = access(p , F_OK);

        if(acc != -1){
            return p;
        }
    }

    return first;
}


int backgroundProcess(char *args[]){

    char *ending = args[size-1];
    int len = strlen(ending);
    if(len==1){
        if(ending[0]=='&'){
            args[size-1] = NULL;
            size--;
            return 1;
        }
    }
    return 0;
}


int executeCommand(char *args[]){
    
    int flag = backgroundProcess(args);

    pid = fork();       
    if(pid < 0){
        printf("ERROR");
    }
    else if (pid == 0){ 

        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);

        if(execv(fullPath(args[0]),args)==-1){
            printf("%s: command not found\n",args[0]);
        }
        exit(EXIT_FAILURE);
    } 
    else if(flag==0){ 
        waitpid(pid, &status, 0);
        return 1;
    }
    else if(flag==0){
        return 1;
    }
}

int forwardOut(int i,char * args[]) {
  
    char *arr[100];
    int j;
    for(j=0; j<i; j++) {
        arr[j] = args[j]; 
    }
    arr[j] = NULL;

    char * fname = args[i+1];

    int fd1 = dup(1);
    int fd2 = dup(2);

    int file;

    if(strcmp(args[i], ">") == 0||strcmp(args[i], "&>") == 0)
        file = open(fname, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRGRP | S_IROTH);
    else if(strcmp(args[i], ">>") == 0)
        file = open(fname, O_WRONLY | O_CREAT |O_APPEND, S_IRWXU | S_IRGRP | S_IROTH);


    if(strcmp(args[i], ">") == 0 ||strcmp(args[i], ">>") == 0){
        
        int pid;
        if(file < 0)   
            return 1;
        if(dup2(file,1) < 0)    
            return 1;

        

        if( (pid = fork()) == 0) {
            close(file);
            close(fd1);
            execv(fullPath(arr[0]),arr);
            return 0;
        }

        dup2(fd1,1);
        close(file);
        close(fd1);
    }

    else if (strcmp(args[i], "&>") == 0){
        if(file < 0)    
            return 1;
        if(dup2(file,2) < 0)    
            return 1;
        int pid;
        if( (pid = fork()) == 0) {
            close(file);
            close(fd1);      
            execv(fullPath(arr[0]),arr);
            return 0;
        }
        dup2(fd2, 2);
        close(file);
        close(fd2);
    }

    close(file);
    close(fd2);
    wait(NULL);
    close(file);
    return 0;
}

int executePipe(int i,char * args[]) {

    char *arr1[100];
    char *arr2[100];
    char *n1, *n2;
    int maxsize;

    if(plofand!=0)
        maxsize= plofand;
    else 
        maxsize =size;

    int m;
    for(m=0;m<i;m++) {
        arr1[m] = args[m];
     
    }
    arr1[m]=NULL;
    n1 = arr1[0];

    int j,n=0;

    for(j=i+1; j<maxsize; j++) {
        arr2[n] = args[j];
        n++;
    }
    arr2[n]=NULL;
    n2 = arr2[0];

    int pid, status;
    fflush(stdout);

    if((pid = fork())<0){
        perror("ERROR");


    }
    else if (pid==0){  
        int fd[2];
        int pid1;

        if(pipe(fd)) {
            perror("ERROR");
        }

        if((pid1 = fork())<0){
            perror("ERROR");
        }
        else if (pid1==0){ 
            close(fd[0]);  
            dup2(fd[1], 1);  
            close(fd[1]);  

            execv(fullPath(n1),arr1);

            perror(n1);
         }
        else if (pid1 > 0) {
            close(fd[1]);  
            dup2(fd[0], 0);  
            close(fd[0]);  
            execv(fullPath(n2),arr2);

            perror(n2);
        }

    }

    else if (pid > 0) {
         pid = wait(&status);
    }

    plofand=0;
    return 1;
}


int forwardIn(int i, char * args[]) {

    char *arr[100];

    int j;
    for(j=0; j<i; j++) {
        arr[j] = args[j];
    }

    arr[j] = NULL;

    int pid, status;
    fflush(stdout);


    if((pid = fork())<0){
        perror("ERROR");
    }

    else if (pid==0){  
        int fd[2];

        if(pipe(fd)) {
            perror("Pipe Error");
        }

        char * fName=args[i+1];
        int pid1;

        if((pid1 = fork())<0){
            perror("fork()");

        }
        else if (pid1==0){  

            char str1;
            close(fd[0]); 
            dup2(fd[1], 1);
            close(fd[1]); 
            FILE * file;
            
            file = fopen (fName , "r");
            if (file == NULL) perror ("ERROR");
            else {
                while ((str1=fgetc(file)) != EOF) {
                        putchar(str1);
                    }
                fclose (file);
                }
            exit(EXIT_SUCCESS);
        }

        else if (pid1 > 0) {
            close(fd[1]);  
            dup2(fd[0], 0);  
            close(fd[0]);  

            execv(fullPath(arr[0]),arr);

            perror(arr[0]);
        }
    }
    else if (pid > 0) 
        pid = wait(&status);

    return 0;
}

void combineCheck(int s, char * args[]){

    int check=0;
    plofand=s;

    for(int i=0;i<s; i++) {   

        if(strcmp(args[i], ">") == 0||strcmp(args[i], ">>") == 0 ||strcmp(args[i], "&>") == 0) {
            forwardOut(i, args);
            check=1;
        }

        else if(strcmp(args[i], "<") == 0) {
           forwardIn(i, args);
           check=1;
       }

       else if(strcmp(args[i], "|") == 0) {
           executePipe(i,args);
           check=1;
       }
   }

   int flag = commandCd(args);

   if(flag==1 && check==0){
        execv(fullPath(args[0]),args);
    }
}

void executeAnd(int i,char * args[]) {

    char *arr1[100];
    char *arr2[100];

    int m;

    for(m=0;m<i;m++) {
        arr1[m] = args[m];

    }

    arr1[m]=NULL;
    int j,n=0;

    for(j=i+1; j<size-a; j++) {
        arr2[n] = args[j];
        n++;
    }

    arr2[n]=NULL;
    if((pid = fork()) == -1 ) { 
        printf("ERROR"); 
    }
    else if ( pid == 0) {

        combineCheck(i,arr1);

    } 
    else if (pid > 0) 
        waitpid(pid, &status, 0);

    int check=0; 
    int k=0;
    for( k=0;k<n; k++) { 
        if(strcmp(arr2[k], "&&") == 0){
            a+=i+1;
            check=1;
            break;
        }
    }

    if(check==1) {
        executeAnd(k, arr2); 
    } 
    else {
        if((pid = fork()) == -1 ) { 
            printf("ERROR"); 
        }
        else if ( pid == 0) {
            combineCheck(k,arr2);
        } 
        else if (pid > 0) 
            waitpid(pid, &status, 0);
    }
}

void signalHandler(int signo){

    if(signo == SIGINT ){
        printf("\n");
    }
    else if(signo == SIGQUIT){
        printf("\n");
    }

}


int main(int argc, char *argv[]){

    char *args[COMMAND_SIZE/2 + 1];
    
    setPath();

    char * com = malloc(COMMAND_SIZE);
    char * com2 = malloc(80*COMMAND_SIZE);

    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);

    while (shell){

            if (strcmp(com2,"")==0) printf("Mini-Shell$ ");
            char * line = malloc(COMMAND_SIZE+1);
            fgets(line, buf , stdin);	

            if(strcmp(line,"\n")==0 || strcmp(line,"\r\n")==0){
			  continue;
			}	

            strcat(com2," ");
            strcat(com2,line);
            strcat(com,com2);
            breakIntoTokens(com,args);

            com="";
            com = malloc(10*COMMAND_SIZE);

            if( strcmp(args[size-1],"\\")==0) {
                com2="";
                com2 = malloc(10*COMMAND_SIZE);

                for(int i=0;i<size-1; i++) { 
                    strcat(com2," ");
                    strcat(com2,args[i]);
                }
               	printf(">");
               	continue;
			}

            int check=0; 
            int andand=0;

            for(int i=0;i<size; i++) { 

                if(strcmp(args[i], "&&") == 0){
                    a=0;            
                    andand=1;                
                    check=1;
                    executeAnd(i, args);
                    break;
                }
            }

            if(andand==0){
                for(int i=0;i<size; i++) {      

                    if(strcmp(args[i], ">") == 0||strcmp(args[i], ">>") == 0 ||strcmp(args[i], "&>") == 0) {
                      
                        forwardOut(i, args);
                        check=1;

                    }
                    else if(strcmp(args[i], "<") == 0) {
                        forwardIn(i, args);
                        check=1;

                    }
                    
                    else if(strcmp(args[i], "|") == 0) {
                        executePipe(i,args);
                        check=1;

                    }
                }
            }

            int flag = commandCd(args);
            
            if(flag==1 && check==0){
                if(executeCommand(args)==-1){
                    printf("%s : command not found!!\n",com);
                    continue;
                }
                   com=""; 
                   com2="";         
            }
            com = malloc(10*COMMAND_SIZE);
            com2 = malloc(10*COMMAND_SIZE);
    }
    
    return 0;
}
