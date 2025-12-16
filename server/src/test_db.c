#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "../include/database.h"
#include "../include/utils.h"

int main() {
    srand(time(NULL));
    printf("Starting Database Tests (New Schema)...\n");
    log_init();

    // 1. Init DB
    if (db_init("database/altp.db") != 0) {
        fprintf(stderr, "Failed to init DB\n");
        return 1;
    }
    printf("[PASS] db_init\n");

    // 2. Register User
    char username[50];
    sprintf(username, "user_%d", rand());
    if (db_register(username, "pass")) {
        printf("[PASS] db_register (%s)\n", username);
    } else {
        printf("[FAIL] db_register\n");
    }

    // 3. Login
    int user_id = db_login(username, "pass");
    if (user_id > 0) {
        printf("[PASS] db_login (ID: %d)\n", user_id);
    } else {
        printf("[FAIL] db_login\n");
    }

    // 4. Create Room
    int room_id = db_create_room(user_id, "score");
    if (room_id > 0) {
        printf("[PASS] db_create_room (ID: %d)\n", room_id);
    } else {
        printf("[FAIL] db_create_room\n");
    }

    // 5. Start Game
    int game_id = db_create_game(room_id, "score");
    if (game_id > 0) {
        printf("[PASS] db_create_game (ID: %d)\n", game_id);
    } else {
        printf("[FAIL] db_create_game\n");
    }

    // 6. Get Questions
    Question qs[5];
    int q_count = db_get_random_questions(5, 1, qs);
    if (q_count >= 0) {
        printf("[PASS] db_get_random_questions (Count: %d)\n", q_count);
    } else {
         printf("[FAIL] db_get_random_questions error\n");
    }

    // 7. Create Round & Record Action
    if (q_count > 0) {
        int round_id = db_create_round(game_id, qs[0].id, 1);
        if (round_id > 0) {
            printf("[PASS] db_create_round (ID: %d)\n", round_id);
            
            if (db_record_action(round_id, user_id, 'A', 1, 10) == 0) {
                printf("[PASS] db_record_action\n");
            } else {
                printf("[FAIL] db_record_action\n");
            }
        } else {
            printf("[FAIL] db_create_round\n");
        }
    }

    // 8. Save History
    if (db_save_game_history(game_id, user_id, 100, "win") == 0) {
        printf("[PASS] db_save_game_history\n");
    } else {
        printf("[FAIL] db_save_game_history\n");
    }

    // 9. NEW: Question CRUD
    Question new_q = {0, "What is 2+2?", "1", "2", "3", "4", 'D', 1};
    int q_id = db_add_question(&new_q);
    if (q_id > 0) {
        printf("[PASS] db_add_question (ID: %d)\n", q_id);
        
        Question read_q;
        if (db_get_question(q_id, &read_q) == 0 && strcmp(read_q.question_text, new_q.question_text) == 0) {
            printf("[PASS] db_get_question\n");
            
            read_q.correct_ans = 'B'; // Change correct answer to 2
            if (db_update_question(&read_q) == 0) {
                printf("[PASS] db_update_question\n");
                
                if (db_delete_question(q_id) == 0) {
                    printf("[PASS] db_delete_question\n");
                } else {
                    printf("[FAIL] db_delete_question\n");
                }
            } else {
                printf("[FAIL] db_update_question\n");
            }
        } else {
            printf("[FAIL] db_get_question\n");
        }
    } else {
        printf("[FAIL] db_add_question\n");
    }

    // 10. NEW: User CRUD (Update Password)
    if (db_update_user_password(user_id, "newpass") == 0) {
        printf("[PASS] db_update_user_password\n");
        // Verify login with new pass
        if (db_login(username, "newpass") == user_id) {
            printf("[PASS] Login with new password\n");
        } else {
            printf("[FAIL] Login with new password\n");
        }
    } else {
        printf("[FAIL] db_update_user_password\n");
    }
    
    // 11. NEW: User CRUD (Delete separate user)
    char del_user_name[50];
    sprintf(del_user_name, "del_user_%d", rand());
    if (db_register(del_user_name, "pass")) {
         int del_id = db_login(del_user_name, "pass");
         if (del_id > 0) {
             if (db_delete_user(del_id) == 0) {
                printf("[PASS] db_delete_user\n");
                if (db_login(del_user_name, "pass") == -1) {
                    printf("[PASS] Deleted user cannot login\n");
                } else {
                    printf("[FAIL] Deleted user can still login\n");
                }
             } else {
                 printf("[FAIL] db_delete_user\n");
             }
         } else {
             printf("[FAIL] Setup delete user login failed\n");
         }
    } else {
        printf("[FAIL] Setup delete user register failed\n");
    }

    db_close();
    log_close();
    printf("All tests completed.\n");
    return 0;
}
