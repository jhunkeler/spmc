/**
 * @file environment.h
 */
#ifndef SPM_ENVIRONMENT_H
#define SPM_ENVIRONMENT_H

typedef StrList RuntimeEnv;

ssize_t runtime_contains(RuntimeEnv *env, const char *key);
RuntimeEnv *runtime_copy(char **env);
char *runtime_get(RuntimeEnv *env, const char *key);
void runtime_set(RuntimeEnv *env, const char *_key, const char *_value);
char *runtime_expand_var(RuntimeEnv *env, const char *input);
void runtime_export(RuntimeEnv *env, char **keys);
void runtime_apply(RuntimeEnv *env);
void runtime_free(RuntimeEnv *env);

#endif //SPM_ENVIRONMENT_H
