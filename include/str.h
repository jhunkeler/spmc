#ifndef SPM_STR_H
#define SPM_STR_H

int num_chars(const char *sptr, int ch);
int startswith(const char *sptr, const char *pattern);
int endswith(const char *sptr, const char *pattern);
char *normpath(const char *path);
void strchrdel(char *sptr, const char *chars);
long int strchroff(const char *sptr, int ch);
void strdelsuffix(char *sptr, const char *suffix);
char** split(char *sptr, const char* delim);
void split_free(char **ptr);
char *join(char **arr, const char *separator);
char *substring_between(char *sptr, const char *delims);
void strsort(char **arr);
int find_in_file(const char *filename, const char *pattern);
int isrelational(char ch);
void print_banner(const char *s, int len);
char *strstr_array(char **arr, const char *str);
char **strdeldup(char **arr);
char *lstrip(char *sptr);
char *strip(char *sptr);
int isempty(char *sptr);
int isquoted(char *sptr);
char *normalize_space(char *s);
char **strdup_array(char **array);

#endif //SPM_STR_H
