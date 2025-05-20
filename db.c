/**
 * @file db.c
 * @author J-D-J123
 * @brief This file contains the database functions for the main.c file
 */

#include "db.h"

sqlite3 *db_init(const char *filename) {
    sqlite3 *db = NULL;
    int rc = sqlite3_open(filename, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    // const char *sql_create_classes =
    //     "CREATE TABLE IF NOT EXISTS classes ("
    //     "name TEXT PRIMARY KEY, "
    //     "time_studied INTEGER, "
    //     "sessions INTEGER);";
    // char *errmsg = NULL;
    // rc = sqlite3_exec(db, sql_create_classes, NULL, NULL, &errmsg);
    // if (rc != SQLITE_OK) {
    //     fprintf(stderr, "Failed to create classes table: %s\n", errmsg);
    //     sqlite3_free(errmsg);
    //     sqlite3_close(db);
    //     return NULL;
    // }

    return db;
}


int db_remove_latest(sqlite3 *db, const char *class_name) {

    if (db == NULL || class_name == NULL) {
        return 1;
    }

    char sql_delete[256];

    snprintf(sql_delete, sizeof(sql_delete),
        "DELETE FROM \"%s\" WHERE session = (SELECT MAX(session) FROM \"%s\");",
        class_name, class_name);

    if (sqlite3_exec(db, sql_delete, 0, 0, 0) != SQLITE_OK) {
        fprintf(stderr, "Error deleting from table: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    return 0;
}


int print_classes(sqlite3 *db) {
    if (db == NULL) {
        return 1;
    }

    char *sql = "SELECT name FROM sqlite_master WHERE type='table';";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *name = (const char *)sqlite3_column_text(stmt, 0);
        if(strcmp(name, "classes") != 0) {
            printf("Class: %s\n", name);
        }
    }

    sqlite3_finalize(stmt);
    return 0; 
}


// returns 0 if the table of the class name exists 
// returns 1 if the table of the class name does not exist 
int db_get(sqlite3 * db, const char * class_name) {

    if (db == NULL || class_name == NULL) {
        return 1; 
    }

    char * sql = "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_bind_text(stmt, 1, class_name, -1, SQLITE_STATIC);
    int exists = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = 1; 
    } else {
        exists = 0; 
    }

    sqlite3_finalize(stmt);

    return exists;
}

// int db_add_class(sqlite3 *db, const char *class_name) {
//     if (db == NULL || class_name == NULL) {
//         return 1; 
//     }

//     char sql_create[256]; 

//     snprintf(sql_create, sizeof(sql_create), 
//         "CREATE TABLE \"%s\" ("
//         "topic TEXT, hours REAL, date TEXT, session INTEGER);", class_name);

//     if (sqlite3_exec(db, sql_create, 0, 0, 0) != SQLITE_OK) {
//         fprintf(stderr, "Error creating table: %s\n", sqlite3_errmsg(db));
//         return 1;
//     }

//     return 0;
// }

int db_add_class(sqlite3 *db, const char *class_name) {
    if (db == NULL || class_name == NULL) {
        return 1; 
    }

    // Create 'classes' table if it doesn't exist yet
    const char *sql_classes = 
        "CREATE TABLE IF NOT EXISTS classes ("
        "name TEXT PRIMARY KEY, "
        "time_studied INTEGER, "
        "sessions INTEGER);";

    if (sqlite3_exec(db, sql_classes, 0, 0, 0) != SQLITE_OK) {
        fprintf(stderr, "Error creating classes table: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // Add the new class-specific table
    char sql_create[256]; 
    snprintf(sql_create, sizeof(sql_create), 
        "CREATE TABLE \"%s\" ("
        "topic TEXT, hours REAL, date TEXT, session INTEGER);", class_name);

    if (sqlite3_exec(db, sql_create, 0, 0, 0) != SQLITE_OK) {
        fprintf(stderr, "Error creating table: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // Add this class to 'classes' tracking table
    char sql_insert[256];
    snprintf(sql_insert, sizeof(sql_insert),
        "INSERT INTO classes (name, time_studied, sessions) VALUES (?, 0, 0);");

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL) != SQLITE_OK)
        return 1;

    sqlite3_bind_text(stmt, 1, class_name, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE ? 0 : 1;
}


int db_update_class(sqlite3 *db, const char *class_name, int duration) {

    if (db == NULL || class_name == NULL) {
        return 1; 
    }

    // printf("Storing %.2f minutes for %s\n", duration, class_name);

    char sql[256];
    snprintf(sql, sizeof(sql),
        "INSERT INTO \"%s\" (topic, hours, date, session) "
        "VALUES (?, ?, ?, (SELECT IFNULL(MAX(session), 0) + 1 FROM \"%s\"));",
        class_name, class_name);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return 1;

    sqlite3_bind_text(stmt, 1, "General", -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, (double) duration);  // (store in double) 
    time_t now = time(NULL);
    sqlite3_bind_text(stmt, 3, ctime(&now), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // update the time in the table 
    if (rc == SQLITE_DONE) {
    char sql_update[256];
    snprintf(sql_update, sizeof(sql_update),
        "UPDATE classes SET time_studied = time_studied + ?, sessions = sessions + 1 WHERE name = ?;");

        sqlite3_stmt *update_stmt;
        if (sqlite3_prepare_v2(db, sql_update, -1, &update_stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_double(update_stmt, 1, (double)duration);
            sqlite3_bind_text(update_stmt, 2, class_name, -1, SQLITE_STATIC);
            sqlite3_step(update_stmt);
            sqlite3_finalize(update_stmt);
        }
    }

    return rc == SQLITE_DONE ? 0 : 1;
}


void grab_total_time(sqlite3 *db) {
    if (db == NULL) return;

    const char *sql = "SELECT name, SUM(time_studied), COUNT(*) FROM classes GROUP BY name;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    double totalTimeStudied = 0;
    int totalSessions = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *name = (const char *)sqlite3_column_text(stmt, 0);
        double sum = sqlite3_column_int(stmt, 1);
        int count = sqlite3_column_int(stmt, 2);

        printf("Class: %s TimeStudied: %.2f sessions: %d\n", name, sum, count);

        totalTimeStudied += sum;
        totalSessions += count;
    }

    sqlite3_finalize(stmt);

    printf("Total Time Studied: %.2f min across %d sessions\n", totalTimeStudied, totalSessions);
}



// int db_delete_table(sqlite3 * db, const char * class_name) {

// }