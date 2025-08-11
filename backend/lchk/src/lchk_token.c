#include "lchk.h"

char *lchk_read_task_token(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        perror("[ERROR] Failed to open task token file");
        return NULL;
    }

    char *token = malloc(LCHK_MAX_TOKEN_LEN);
    if (!token) {
        fclose(f);
        fprintf(stderr, "[ERROR] Memory allocation failed\n");
        return NULL;
    }

    if (!fgets(token, LCHK_MAX_TOKEN_LEN, f)) {
        fclose(f);
        free(token);
        fprintf(stderr, "[ERROR] Failed to read task token\n");
        return NULL;
    }

    fclose(f);

    /* Strip trailing newline */
    token[strcspn(token, "\n")] = 0;
    return token;
}
