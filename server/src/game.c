#include<stdio.h>
#include "database.h"


int main() {
    sqlite3* db = db_init("../../database/database.db");
    // get_user(db);
    add_user(db, "minhnd", "123456789");
    get_user(db);
    delete_record(db, "users", "username = \"minhnd\"");
    get_user(db);

    get_questions(db, 1, 10);

    get_user(db);
    update_user_score(db, 1, 2000000);
    get_user(db);

    save_history(db, "AAA", 1, 2, "testing");
    save_player_stat(db, 1, 10, 100, 1 );
    db_close(db);

}