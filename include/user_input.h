/**
 * @file user_input.h
 */
#ifndef SPM_USER_INPUT_H
#define SPM_USER_INPUT_H

int spm_user_yesno(int answer, int empty_input_is_yes);
int spm_prompt_user(const char *msg, int empty_input_is_yes);
void spm_user_yesno_test();

#endif //SPM_USER_INPUT_H
