/**
 * Author: J-D-J123
 * Date:   4/14/2025
 * FIle:   main.c 
 * DESC:   Starts the prgram and runs the main loop. 
 */

#include <windows.h>
#include "db.h"

#define WHITE 7
#define RED 12
#define GREEN 10
#define YELLOW 14
#define CYAN 11

void set_color(int color) {

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void get_user_options(void) {
    set_color(WHITE); 
    printf("\n=======OPTIONS:======\n");
    printf("1. Start study timer\n");
    printf("2. Grab info\n");
    // printf("3. Settings\n");
    printf("3. Exit\n");
    printf("=====================\n");
}

void print_version(char * version, char * author) {
    printf("Starting FOCUS v");
    for(int i = 0; version[i] != '\0'; i++) {

        printf("%c", version[i]);
    }
    // printf("\n");
    printf(" by ");
    
    // print author and date
    for(int i = 0; author[i] != '\0'; i++) {
        printf("%c", author[i]);
    }
    printf("\n----------------------------------\n");
}

// duration in seconds so have to convert to minutes 
void start_timer(sqlite3 * db, const char * class_name, int duration) {

    duration = duration * 60; 

    time_t start_time = time(NULL); 
    while (time(NULL) - start_time < duration) {
        int remaning = duration - (time(NULL) - start_time);
        int min = remaning / 60;
        int sec = remaning % 60;

        set_color(CYAN); // BBL
        printf( "\rTime remaining: %02d:%02d   ", min, sec);
        fflush(stdout);
        Sleep(1000); 

    }

    set_color(GREEN); // green
    printf("\nBreak Time!\n");

    // save the session to the database - duration in seconds 
    // printf("DEBUG: Duration = %d, Class = %s\n", duration, class_name);

    int class_update = db_update_class(db, class_name, duration / 60); // store in min  
    if (class_update) {

        set_color(RED); // red
        printf("Error updating class %s\n", sqlite3_errmsg(db));
        return; 
    } // else {
    //     set_color(GREEN); // green
    //     printf(Class updated successfully\n");
    // }
}

// option 1: 
void start_study_timer(sqlite3 * db) {

    // read the classes in the database 
    // if user wants to add a new class then add it to the database
    // print out current classes that the user can choose from 
    int output = print_classes(db); 
    if (output) {

        set_color(RED); // red
        printf("Error printing classes\n");
        return; 
    } // else {
    //     printf(GREEN "Classes printed successfully\n");
    // }

    char class_name[64]; 
    set_color(WHITE); // white
    printf("Enter the class name: ");
    scanf("%s", class_name); printf("\n");

    // check if the class name is in the database 
    // if not then add it to the database 
    int class_exists = db_get(db, class_name); 

    if(!class_exists) {

        set_color(RED); // red
        printf("Class does not exist\n");
        set_color(WHITE); // white
        printf("Adding class to database...\n");
        if (db_add_class(db, class_name)) {

            set_color(RED); // red
            printf("Failed to add class\n");
            return; 
        }
    } 
    
    int duration = 0; 
    set_color(WHITE); // red
    printf("How long do you want to study ");
    for(int i = 0; class_name[i] != '\0'; i++) {
        printf("%c", class_name[i]);
    }

    printf(" for (in minutes): ");
    scanf("%d", &duration); printf("\n");

    start_timer(db, class_name, duration);
}

// option 2: 
void print_info(sqlite3 * db) {

    set_color(WHITE); // white
    printf("\n=======OPTIONS:======\n");
    printf("1. Grab Classes\n");
    printf("2. Grab total time studied\n");
    printf("3. Exit\n");
    printf("=====================\n");

    // get user input
    int user_input; 
    set_color(WHITE); // white
    printf("    Enter a command: ");
    scanf("%d", &user_input); printf("\n");

    // check if the user input is valid
    if(user_input < 1 || user_input > 4) {

        set_color(RED); // red  
        printf("Invalid input\n");
        return; 
    }

    switch(user_input) {
        case 1: 
            // printf("Grabing classes...\n");
            print_classes(db);
            break; 
        case 2: 
            grab_total_time(db);
            break; 
        case 3: 
            break; 
        default: 
            printf("Invalid input\n");
            break;
    }
}

// // option 3: 
// void settings(sqlite3 * db) {

//     // change settings for the program 
// }

// option 4: 
void exit_program(sqlite3 * db) {

    // close the file pointer 
    sqlite3_close(db);
    printf("Exiting program...\n");
}

int main (void) {

    char * version = "0.0.1"; 
    char * author  = "J-D-J123";

    print_version(version, author);

    // open the database file 
    sqlite3 * db = db_init("focus.db"); 
    if(db) {

    } else {
        set_color(RED); // red
        printf("Error opening database\n");
        return 1;
    }

    // manage user input with the main loop 
    // get user input 
    int user_input; 

    int loop = 1; 
    while(loop) {

        get_user_options();

        set_color(WHITE); // white
        printf("    Enter a command: ");
        scanf("%d", &user_input); printf("\n");

        switch(user_input) {
            case 1: 
                // printf("You entered 1\n");
                start_study_timer(db);
                break; 
            case 2: 
                // printf("grab info\n");
                print_info(db);
                break; 
            case 3: 
                exit_program(db);
                loop = 0; 
                break;
            // case 4:
                // exit_program(db);
                // loop = 0; 
                // break;
            default: 
                printf("Invalid input\n");
                break;
        }
    }
    return 0; 
}