#include "spm.h"

/**
 * Basic case-insensitive interactive choice function
 * @param answer
 * @param answer_default
 * @return
 */
int spm_user_yesno(int answer, int empty_input_is_yes) {
    int result = 0;
    answer = tolower(answer);

    if (answer == 'y') {
        result = 1;
    } else if (answer == 'n') {
        result = 0;
    } else {
        if (empty_input_is_yes) {
            result = 1;
        } else {
            result = -1;
        }
    }

    return result;
}

int spm_prompt_user(const char *msg, int empty_input_is_yes) {
    int user_choice = 0;
    int status_choice = 0;
    char ch_yes = 'y';
    char ch_no = 'n';

    if (empty_input_is_yes) {
        ch_yes = 'Y';
    } else {
        ch_no = 'N';
    }

    printf("\n%s [%c/%c] ", msg, ch_yes, ch_no);
    while ((user_choice = getchar())) {
        status_choice = spm_user_yesno(user_choice, 1);
        if (status_choice == 0) { // No
            break;
        } else if (status_choice == 1) { // Yes
            break;
        } else { // Only triggers when spm_user_yesno's second argument is zero
            puts("Please answer 'y' or 'n'...");
        }
    }
    puts("");

    return status_choice;
}


void spm_user_yesno_test() {
    int choice;
    int status;
    while ((choice = getchar())) {
        status = spm_user_yesno(choice, 1);
        if (status == -1) {
            puts("Please answer Y or N");
            continue;
        }
        else if (status == 0) {
            puts("You answered no");
            break;
        }
        else if (status == 1) {
            puts("You answered yes");
            break;
        }
    }
}
