#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
int read_file_flag = 0;
int lineNumber = 5;

void alarm_handler( int sig ) {
    read_file_flag = 1;
}

int main() {
    signal(SIGALRM,alarm_handler );
    alarm(2);
    FILE *fp;
    char line[256];
    char *interrupt_string;
    int interrupt_now;
    int interrupt_before = 0;
    while(1) {
        if (read_file_flag) {
            int count = 0;
            fp = fopen("/proc/stat", "r");
            if ( fp != NULL ){ 
                while (fgets(line, sizeof line, fp) != NULL){
                    if (count == lineNumber){
                        interrupt_string = strtok (line," ");
                        interrupt_string = strtok (NULL," ");
                        interrupt_now = atoi(interrupt_string);
                        printf("Total interrupt: %d\n",interrupt_now);
                        printf("Total interrupts at the last 2 seconds: %d\n" ,interrupt_now - interrupt_before);
                        interrupt_before = interrupt_now;
                        break;
                    }
                    else{
                        count++;
                    }
                }
                fclose(fp);
            }
            else{
                //file doesn't exist
            }
            read_file_flag = 0;
            alarm(2);
        }
    }
}
