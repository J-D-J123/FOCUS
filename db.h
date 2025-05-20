#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <time.h>
#include "sqlite3.h"

sqlite3 *db_init(const char *filename);

int db_add(sqlite3 *db, const char *class_name, const char *topic, double hours, int session_number);

int db_add_class(sqlite3 *db, const char *class_name); 
int db_remove_latest(sqlite3 *db, const char *class_name); 
int db_get(sqlite3 * db, const char * class_name);
int db_remove(const char * filename, char * name);
int print_classes(sqlite3 *db);

int db_free(const char * filename);


void grab_total_time(sqlite3 * db); 

int db_update_class(sqlite3 *db, const char *class_name, int duration);