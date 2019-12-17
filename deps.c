//
// Created by jhunk on 12/16/19.
//
#include "spm.h"

int exists(const char *filename) {
    return access(filename, F_OK);
}

int dep_seen(Dependencies **deps, const char *name) {
    if (!deps) {
        return -1;
    }
    for (int i = 0; i != (*deps)->records; i++) {
        if (strstr((*deps)->list[i], name) != NULL) {
            return 1;
        }
    }
    return 0;
}

int dep_init(Dependencies **deps) {
    (*deps) = (Dependencies *)calloc(1, sizeof(Dependencies));
    (*deps)->__size = 2;
    (*deps)->records = 0;
    (*deps)->list = (char **)calloc((*deps)->__size, sizeof(char *));
    if (!(*deps)->list) {
        return -1;
    }
    return 0;
}

void dep_free(Dependencies **deps) {
    if ((*deps) != NULL) {
        return;
    }
    for (int i = 0; i < (*deps)->__size; i++) {
        if ((*deps)->list[i] != NULL) {
            free((*deps)->list[i]);
        }
    }
    free((*deps));
}

int dep_append(Dependencies **deps, char *name) {
    if (!(*deps)) {
        return -1;
    }
    (*deps)->__size++;
    (*deps)->list = (char **)realloc((*deps)->list, sizeof(char *) * (*deps)->__size);
    if (!(*deps)->list) {
        return -1;
    }
    (*deps)->list[(*deps)->records] = (char *)calloc(strlen(name) + 1, sizeof(char));
    if (!(*deps)->list[(*deps)->records]) {
        return -1;
    }
    strcpy((*deps)->list[(*deps)->records], name);//, strlen(name));
    (*deps)->records++;
    return 0;
}

int dep_solve(Dependencies **deps, const char *filename) {
    if (!(*deps)) {
        return -1;
    }
    if (exists(filename) != 0) {
        return -1;
    }
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror(filename);
        return -1;
    }

    char data[BUFSIZ];
    memset(data, '\0', sizeof(data));

    char *line = data;
    int line_count = 0;
    while (fgets(line, BUFSIZ, fp) != NULL) {
        size_t line_length = strlen(line);
        if (line_length > 1 && strstr(line, "\r\n")) {
            line[line_length - 2] = '\0';
        }
        if (line_length > 1 && line[line_length - 1] == '\n') {
            line[line_length - 1] = '\0';
        }
        if (strcmp(line, "") == 0) {
            continue;
        }
        line_count++;
        if (dep_seen(deps, line) > 0) {
            // Already seen this dependency. Skip it.
            continue;
        }
        else {
            // Have not seen this dependency before
            if (dep_append(deps, line) == 0) {
                dep_solve(deps, line);
            }
        }
    }
    fclose(fp);
    return line_count;
}

void dep_show(Dependencies **deps) {
    if ((*deps) == NULL) {
        return;
    }
    for (int i = 0; i < (*deps)->records; i++) {
        printf("%d: %s\n", i, (*deps)->list[i]);
    }
}
