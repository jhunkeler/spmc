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
