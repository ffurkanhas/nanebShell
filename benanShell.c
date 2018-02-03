#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#define COMMAND_SIZE 1024 //1024 byte
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

char *read_line(void);
char **parse_line(char *line);
int execute_command(char **args);
int ampersand_control(char **args);
void child_process_handler(int sig);
int check_redirection(char **args);
void cd(char *d);
char *input,*output;

extern int errno;

void syserr(char * msg){
  fprintf(stderr, "%s: %s\n", strerror(errno), msg);
  abort();
}

char * substr(char *s, int start, int end)
{
       int size = end - start + 2; // 1 for the inclusive limits and another 1 for the \0
       char * r = (char*)malloc(size);
       strncpy(r,s+start, size-1);
       r[size-1]=0;
       return r;
}

int main(int argc, char *argv[]){
	int devam = 1;
	char *command = malloc(sizeof(char) * COMMAND_SIZE);
	char command_history[10][sizeof(char) * COMMAND_SIZE];
	char **args;
	pid_t pid;
	int status;
	int wait;
	int k,j;
	char childClock[20];
	int count = 0;
	while(devam){
		int pip[2];
		pipe(pip);
		int pip2[2];
		pipe(pip2);
		printf("yenishell> ");
		fflush(stdout);
		command = read_line();
		args = parse_line(command);
		if(k == 10)
			k = 0;
		strcpy(command_history[k++],*args);
		char firs_letter = *args[0];
		if(firs_letter == '!'){
					char *number = substr(*args,1,1);
					int command_no = *number - '0';
					strcpy(*args,command_history[command_no]);
		}	
		
		if(args[0] == NULL)
			continue;
		if(!strcmp(*args,"exit")){
			return 0;
		}
		if(!strcmp(*args,"gecmis")){
			for(j=0;j<k;j++){
				printf("%d. %s\n",j,command_history[j]);
			}
			continue;
		}

		if(!strcmp(*args,"saatkac")){
			time_t current_time;
    		struct tm * time_info;
    		char timeString[9];  // space for "HH:MM:SS\0"

			pid = getpid();
			switch(pid = fork()){
    		case -1:
        		perror("pipe");
        		exit(1);
    		case 0: 
        		close(pip[0]);
        		time(&current_time);
   		 		time_info = localtime(&current_time);
    			strftime(childClock, sizeof(childClock), "%H:%M:%S", time_info);
	            size_t length = strlen( childClock );
	            write( pip[1], childClock, length );
	            exit(1);
    		default:
    			/*parent process*/
    			close(pip[1]);
                waitpid(pid, &status, WUNTRACED);
                printf("%d nolu cocuk %d kodu ile sonlandi.\n" ,pid,status);
            	ssize_t count = read( pip[0], childClock, sizeof(childClock)-1 );
            	if ( count <= 0 ) {
                	perror( "read" );
                	exit( 1 );
            	}
            	childClock[count] = '\0';
            	printf( "Parent read clock from child: %s\n", childClock );
            	char timeString[9];  // space for "HH:MM:SS\0"

    			time(&current_time);
    			time_info = localtime(&current_time);

    			strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);
   				printf("Parent clock: %s\n" ,timeString);
	            break;
    		}
    		continue;
		}

		if(!strcmp(*args,"pinpon")){
			clock_t t1, t2 ,okunan;  
    		float diff;
    		int exitCode = -1;
    		int clockCount;

			pid = getpid();
			switch(pid = fork()){
    		case -1:
        		perror("pipe");
        		exit(1);
    		case 0: //child
    			close(pip[1]);
    			close(pip2[0]);
    			while(1==1){
    				read(pip[0], &okunan, sizeof(okunan));
    				t2 = clock();
    				diff = ((float)(okunan - t2) / 1000000.0F ) * 1000;
    				if(diff < 10){
    					printf("%f\n",diff);
    					write(pip2[1],&diff,sizeof(diff));
    				}
    				else {
    					write(pip2[1],&exitCode,sizeof(exitCode));
    					break;
    				}
    			}
    			close(pip[0]);
    			close(pip2[1]);
	            exit(1);
    		default: //parent
    			close(pip[0]);
    			close(pip2[1]);
    			while(1==1){
    				t1 = clock();
    				write(pip[1],&t1,sizeof(t1));
    				read(pip2[0],&clockCount,sizeof(clockCount));
    				if(clockCount != -1){
    					printf("***%d\n",clockCount);
    					count++;
    				}
    				else{
    					printf("%d\n",count);
    					break;
    				}
    			}
                close(pip[1]);
    			close(pip2[0]);
	            break;
    		}
    		continue;
		}

		wait = ampersand_control(args);

		if(wait){
			signal(SIGCHLD, child_process_handler);
		}
		else{
			signal(SIGCHLD, NULL); //ampersand yoksa handler nulllandi
		}
		int redirection;
		if (!strcmp(args[0],"cd")) {
          cd(args[1]);
          continue;
		}


		pid = getpid();   
        	switch (pid = fork ()) { 
            	case -1:
              		printf("fork"); 
            	case 0:
            		redirection = check_redirection(args);
					if(redirection == 0){
            				freopen(input, "r", stdin);
       				}
       				else if( redirection == 1){
       					freopen(output, "w", stdout); 
       				}
              		execvp (args[0], args);
              		syserr("exec");
            	default:                
              		if (!wait){ //ampersand yoksa eger islemin bitmesini bekle
                		waitpid(pid, &status, WUNTRACED);
                		printf("%d nolu cocuk %d kodu ile sonlandi.\n" ,pid,status);
              		}
        }

		free(command);
		free(args);
	}
	return 0;
}

char *read_line(void){
	char *line = NULL;
	ssize_t bufsize = 0;
	getline(&line, &bufsize, stdin);
	return line;
}

char **parse_line(char *line){
	int bufsize = LSH_TOK_BUFSIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if (!tokens) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, LSH_TOK_DELIM);
	while (token != NULL) {
		tokens[position] = token;
		position++;
		token = strtok(NULL, LSH_TOK_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}

int ampersand_control(char **args) {
	int i = 1;
	while(args[i] != NULL){
    	if (!strcmp(args[i], "&")){
    		args[i] = NULL;
    		return 1;
    	}
    	i++;
  	}
	return 0;
}

void child_process_handler(int sig){
  pid_t pid;
  pid = wait(NULL);
  printf("\n%d nolu cocuk arkaplanda sonlandi.\nyenishell> ", pid);
  fflush(stdout);
}

int check_redirection(char **args){ //2 ise yonlendirme yok 0 ise input < , 1 ise output >
  int i = 0;
  while(args[i] != NULL){
    if (!strcmp(args[i], "<")){
      strcpy(args[i], "\0");
      input = args[i+1];
      return 0;
    }
    else if (!strcmp(args[i], ">")){
      output = args[i+1];
      args[i] = NULL;
      return 1;
    }
    i++;
  }
  return 0;
}

void cd(char *directory){
  chdir(directory);  
}