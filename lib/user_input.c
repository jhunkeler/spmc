#include "spm.h"

int spm_prompt_user(const char *msg, int empty_input_is_yes) {
    int status_choice = 0;
    char ch_yes = 'y';
    char ch_no = 'n';

    if (empty_input_is_yes) {
        ch_yes = 'Y';
    } else {
        ch_no = 'N';
    }

    printf("\n%s [%c/%c] ", msg, ch_yes, ch_no);

    char ch[2] = {0,};
    int input_count = 0;
    while (scanf("%c", ch) == 1) {
        ch[1] = '\0';

        if (input_count != 0) {
            input_count = 0;
            ch[0] = '\0';
            continue;
        }

        if ((isempty(ch) && empty_input_is_yes != 0)) {
            return 1;
        } else if ((isempty(ch) && empty_input_is_yes == 0)) {
            return 0;
        } else if (tolower(ch[0]) == tolower(ch_yes)) { // Yes
            return 1;
        } else if (tolower(ch[0]) == tolower(ch_no)) { // No
            return 0;
        } else {
            printf("Please answer '%c' or '%c'...\n", tolower(ch_yes), tolower(ch_no));
        }
        input_count++;
    }
    return -1;
}
