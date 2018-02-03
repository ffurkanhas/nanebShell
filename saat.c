#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

int main(void) {

    time_t current_time;
    struct tm * time_info;
    char timeString[9];  // space for "HH:MM:SS\0"

    time(&current_time);
    time_info = localtime(&current_time);

    strftime(timeString, sizeof(timeString), "%H:%M:%S", time_info);
    puts(timeString);

    return 0;
}