#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 1024
#define MAX_PATH_LENGTH 512
#define MAX_COMMAND_LENGTH 1024

/* Recipe path pattern */
#define DEFAULT_RECIPE_PATH_PATTERN "/etc/recp/recipe_%d.txt"

/* Trim leading/trailing whitespace */
char *trim(char *str) {
    while (*str == ' ' || *str == '\t') str++;
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == '\n' || *end == ' ' || *end == '\t')) *end-- = '\0';
    return str;
}

/* CREATE <path> */
void handle_create(const char *path) {
    if (mkdir(path, 0755) != 0) {
        if (errno == EEXIST) {
            printf("Directory already exists: %s\n", path);
        } else {
            perror("mkdir");
        }
    } else {
        printf("Created directory: %s\n", path);
    }
}

/* MOVE <src> <dst> */
void handle_move(const char *src, const char *dst) {
    if (rename(src, dst) != 0) {
        perror("rename");
    } else {
        printf("Moved file: %s -> %s\n", src, dst);
    }
}

/* EXEC <command> */
void handle_exec(const char *command) {
    int ret = system(command);
    if (ret == -1) {
        perror("system");
    } else {
        printf("Executed command: %s\n", command);
    }
}

/* Parse and handle one line from the recipe */
void parse_line(char *line) {
    line = trim(line);
    if (line[0] == '\0' || line[0] == '#') return;  /* Ignore empty or comment lines */

    char *cmd = strtok(line, " ");
    if (!cmd) return;

    if (strcmp(cmd, "CREATE") == 0) {
        char *path = strtok(NULL, "");
        if (path) handle_create(trim(path));
        else fprintf(stderr, "CREATE requires a path\n");

    } else if (strcmp(cmd, "MOVE") == 0) {
        char *src = strtok(NULL, " ");
        char *dst = strtok(NULL, "");
        if (src && dst) handle_move(trim(src), trim(dst));
        else fprintf(stderr, "MOVE requires source and destination paths\n");

    } else if (strcmp(cmd, "EXEC") == 0) {
        char *command = strtok(NULL, "");
        if (command) handle_exec(trim(command));
        else fprintf(stderr, "EXEC requires a shell command\n");

    } else {
        fprintf(stderr, "Unknown command: %s\n", cmd);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <recipe_id>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int recipe_id = atoi(argv[1]);
    char recipe_path[MAX_PATH_LENGTH];
    //snprintf(recipe_path, sizeof(recipe_path), DEFAULT_RECIPE_PATH_PATTERN, recipe_id);
    // TODO: Remove hardcode below
    snprintf(recipe_path, sizeof(recipe_path), "recipe_1.txt");
    FILE *fp = fopen(recipe_path, "r");
    if (!fp) {
        perror("Failed to open recipe file");
        return EXIT_FAILURE;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), fp)) {
        parse_line(line);
    }

    fclose(fp);
    return EXIT_SUCCESS;
}
