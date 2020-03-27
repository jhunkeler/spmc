#include "spm.h"
#include "framework.h"

const char *testFmt = "case %zu: '%s' is length %zu, expected %zu\n";
const char *testFmtChars = "case %zu: '%s' got '%c' (pos: %zu), expected '%c'\n";

int main(int argc, char *argv[]) {
    char *tty_path = NULL;
    if ((tty_path = ttyname(STDOUT_FILENO)) == NULL) {
        perror("tty check");
        return 0;
    }

    const int max_banner = 'z';
    char buf[BUFSIZ] = {0,};
    char *buffer = buf;
    int stdout_save = 0;

    fflush(stdout);
    for (size_t i = 'A', letters = 'A'; i <= (size_t)max_banner; i++, letters++) {
        char letter[] = {letters, '\0'};
        size_t buffer_len = 0;

        // Redirect stdout
        stdout_save = dup(STDOUT_FILENO);
        freopen("/dev/null", "w", stdout);
        setvbuf(stdout, buffer, _IOLBF, sizeof(buf));

        // Do test
        print_banner(letter, i);
        fflush(stdout);

        strip(buffer);
        buffer_len = strlen(buffer);

        myassert(buffer_len == i, testFmt, i, buffer, buffer_len, i);
        for (size_t j = 0; j < buffer_len; j++) {
            myassert(buffer[j] == (int)letters, testFmtChars, i, buffer, j, buffer[j], (int)letters, buffer[j]);
        }

        // Restore stdout
        dup2(stdout_save, STDOUT_FILENO);
        setvbuf(stdout, NULL, _IONBF, 0);
    }
}
