/* Copyright (c) Dmitry Ivanov, 2025
 * recp.c - v0.1 - Linux environment setup tool - MIT license */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <sys/wait.h>


#define RECP_MAX_LINE_LENGTH 4096
#define RECP_MAX_ARGS 64
#define RECP_LOG_FILE "recp.log"
#define RECP_TEMP_SCRIPT_FILE "/tmp/recp_script.sh"
#define RECP_MAX_FILE_SIZE 4096
#define RECP_INITIAL_SCRIPT_SIZE 1024

char recp_msg[256];

typedef enum {
    OP_CREATE_DIR,
    OP_CREATE_FILE,
    OP_MOVE,
    OP_COPY,
    OP_EXEC,
    OP_DELETE,
    OP_SCRIPT,
    OP_UNKNOWN
} Op_Type;


typedef struct {
    Op_Type type;
    char *args[RECP_MAX_ARGS];
    char *script_content;
    int arg_count;
} Command;


void log_message(const char *message, int is_error) {
    time_t now;
    time(&now);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp)-1] = '\0';

    FILE *log = fopen(RECP_LOG_FILE, "a+");
    if (log) {
        fprintf(log, "[%s] %s: %s\n", timestamp, is_error ? "ERROR" : "INFO", message);
        fclose(log);
    }

    if (is_error) {
        fprintf(stderr, "ERROR: %s\n", message);
    }
}


Op_Type parse_op_type(const char *op_str) {
    if (strcmp(op_str, "CREATE_DIR") == 0) return OP_CREATE_DIR;
    if (strcmp(op_str, "CREATE_FILE") == 0) return OP_CREATE_FILE;
    if (strcmp(op_str, "MOVE") == 0) return OP_MOVE;
    if (strcmp(op_str, "COPY") == 0) return OP_COPY;
    if (strcmp(op_str, "EXEC") == 0) return OP_EXEC;
    if (strcmp(op_str, "DELETE") == 0) return OP_DELETE;
    if (strcmp(op_str, "SCRIPT") == 0) return OP_SCRIPT;
    return OP_UNKNOWN;
}


void free_command(Command *cmd) {
    for (int i = 0; i < cmd->arg_count; i++) {
        free(cmd->args[i]);
    }
    if (cmd->type == OP_SCRIPT && cmd->script_content) {
        free(cmd->script_content);
    }
}


char *read_script_block(FILE *file, int *line_num) {
    char buffer[RECP_MAX_LINE_LENGTH];
    size_t content_size = RECP_INITIAL_SCRIPT_SIZE;
    char *content = malloc(content_size);
    if (!content) return NULL;
    content[0] = '\0';

    while (fgets(buffer, sizeof(buffer), file)) {
        (*line_num)++;

        /* Check for end of script block */
        if (strcmp(buffer, "END_SCRIPT\n") == 0) {
            return content;
        }

        /* Check if we need to grow our buffer */
        size_t new_length = strlen(content) + strlen(buffer);
        if (new_length + 1 > content_size) {
            content_size *= 2;
            char *new_content = realloc(content, content_size);
            if (!new_content) {
                free(content);
                return NULL;
            }
            content = new_content;
        }

        strcat(content, buffer);
    }

    /* If we get here, we didn't find END_SCRIPT */
    free(content);
    return NULL;
}


int parse_command(FILE *file, char *line, Command *cmd, int *line_num) {
    cmd->arg_count = 0;
    cmd->script_content = NULL;
    char *arg;
    char *saveptr = NULL;
    char *op_str = strtok_r(line, " \t\n", &saveptr);
    if (!op_str) return 0;

    cmd->type = parse_op_type(op_str);
    if (cmd->type == OP_UNKNOWN) {
        snprintf(recp_msg, sizeof(recp_msg), "Unknown operation: %s", op_str);
        log_message(recp_msg, 1);
        return 0;
    }

    if (cmd->type == OP_SCRIPT) {
        cmd->script_content = read_script_block(file, line_num);
        if (!cmd->script_content) {
            log_message("Invalid or incomplete SCRIPT block", 1);
            return 0;
        }
        cmd->arg_count = 0;
        return 1;
    }

    /* This parses command line arguments as a single string */
    /*
    if (cmd->type == OP_EXEC) {
        if ((arg = strtok_r(NULL, " \t\n", &saveptr)) != NULL) {
            cmd->args[cmd->arg_count] = strdup(arg);
            cmd->arg_count++;
        }
        if ((arg = strtok_r(NULL, "\t\n", &saveptr)) != NULL) {
            cmd->args[cmd->arg_count] = strdup(arg);
            cmd->arg_count++;
        }
        cmd->args[cmd->arg_count] = NULL;
        return 1;
    }
    */

    while((arg = strtok_r(NULL, " \t\n", &saveptr)) != NULL && cmd->arg_count < RECP_MAX_ARGS - 1) {
        cmd->args[cmd->arg_count] = strdup(arg);
        cmd->arg_count++;
    }
    cmd->args[cmd->arg_count] = NULL;
    return 1;
}


int execute_create_dir(Command *cmd) {
    if (cmd->arg_count < 1) {
        log_message("CREATE_DIR requires at least one argument", 1);
        return 0;
    }

    for (int i = 0; i < cmd->arg_count; i++) {
        const char *dir_path = cmd->args[i];

        /* Check if directory already exists */
        struct stat st;
        if (stat(dir_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                snprintf(recp_msg, sizeof(recp_msg), "Directory already exists: %s", dir_path);
                log_message(recp_msg, 0);
                continue;
            } else {
                snprintf(recp_msg, sizeof(recp_msg), "Path exists but is not a directory: %s", dir_path);
                log_message(recp_msg, 1);
                return 0;
            }
        }

        /* Create directory with full permissions (adjust with umask) */
        if (mkdir(dir_path, 0777) == -1) {
            snprintf(recp_msg, sizeof(recp_msg), "Could not create directory %s: %s",
                    dir_path, strerror(errno));
            log_message(recp_msg, 1);
            return 0;
        }

        snprintf(recp_msg, sizeof(recp_msg), "Created directory: %s", dir_path);
        log_message(recp_msg, 0);
    }

    return 1;
}


int execute_create_file(Command *cmd) {
    if (cmd->arg_count < 1) {
        log_message("CREATE_FILE requires at least one argument", 1);
        return 0;
    }

    for (int i = 0; i < cmd->arg_count; i++) {
        const char *file_path = cmd->args[i];

        /* Check if file already exists */
        if (access(file_path, F_OK) == 0) {
            snprintf(recp_msg, sizeof(recp_msg), "File already exists: %s", file_path);
            log_message(recp_msg, 0);
            continue;
        }

        /* Create empty file */
        int fd = open(file_path, O_WRONLY | O_CREAT, 0666);
        if (fd == -1) {
            snprintf(recp_msg, sizeof(recp_msg), "Could not create file %s: %s",
                    file_path, strerror(errno));
            log_message(recp_msg, 1);
            return 0;
        }
        close(fd);

        snprintf(recp_msg, sizeof(recp_msg), "Created file: %s", file_path);
        log_message(recp_msg, 0);
    }

    return 1;
}


int execute_move(Command *cmd) {
    if (cmd->arg_count != 2) {
        log_message("MOVE requires 2 arguments: source and destination", 1);
        return 0;
    }

    const char *src = cmd->args[0];
    const char *dst = cmd->args[1];

    if (rename(src, dst) != 0) {
        snprintf(recp_msg, sizeof(recp_msg), "Could not move %s to %s: %s", src, dst, strerror(errno));
        log_message(recp_msg, 1);
        return 0;
    }

    snprintf(recp_msg, sizeof(recp_msg), "Moved %s to %s", src, dst);
    log_message(recp_msg, 0);

    return 1;
}


int execute_copy(Command *cmd) {
    if (cmd->arg_count != 2) {
        log_message("COPY requires 2 arguments: source and destination", 1);
        return 0;
    }

    const char *src = cmd->args[0];
    const char *dst = cmd->args[1];

    FILE *src_file = fopen(src, "rb");
    if (!src_file) {
        snprintf(recp_msg, sizeof(recp_msg), "Could not open source file %s: %s",
                src, strerror(errno));
        log_message(recp_msg, 1);
        return 0;
    }

    FILE *dst_file = fopen(dst, "wb");
    if (!dst_file) {
        snprintf(recp_msg, sizeof(recp_msg), "Could not create destination file %s: %s",
                dst, strerror(errno));
        fclose(src_file);
        log_message(recp_msg, 1);
        return 0;
    }

    char buffer[RECP_MAX_FILE_SIZE];
    size_t bytes;
        while ((bytes = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
        if (fwrite(buffer, 1, bytes, dst_file) != bytes) {
            snprintf(recp_msg, sizeof(recp_msg), "Could not write to destination file %s", dst);
            log_message(recp_msg, 1);
            fclose(src_file);
            fclose(dst_file);
            return 0;
        }
    }

    fclose(src_file);
    fclose(dst_file);

    snprintf(recp_msg, sizeof(recp_msg), "Copied %s to %s", src, dst);
    log_message(recp_msg, 0);

    return 1;
}


int execute_delete(Command *cmd) {
    if (cmd->arg_count < 1) {
        log_message("DELETE requires at least one argument", 1);
        return 0;
    }

    for (int i = 0; i < cmd->arg_count; i++) {
        const char *path = cmd->args[i];

        struct stat st;
        if (stat(path, &st) != 0) {
            snprintf(recp_msg, sizeof(recp_msg), "Path does not exist: %s", path);
            log_message(recp_msg, 1);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            /* Directory - use rmdir (only works for empty directories) */
            if (rmdir(path) != 0) {
                snprintf(recp_msg, sizeof(recp_msg), "Could not delete directory %s: %s",
                        path, strerror(errno));
                log_message(recp_msg, 1);
                return 0;
            }
        } else {
            /* File - use unlink */
            if (unlink(path) != 0) {
                snprintf(recp_msg, sizeof(recp_msg), "Could not delete file %s: %s",
                        path, strerror(errno));
                log_message(recp_msg, 1);
                return 0;
            }
        }

        snprintf(recp_msg, sizeof(recp_msg), "Deleted: %s", path);
        log_message(recp_msg, 0);
    }

    return 1;
}


int execute_exec(Command *cmd) {
    if (cmd->arg_count < 1) {
        log_message("EXEC requires at least one argument", 1);
        return 0;
    }

    /* Join all arguments into a single command string */
    char command[RECP_MAX_LINE_LENGTH] = {0};
    for (int i = 0; i < cmd->arg_count; i++) {
        strcat(command, cmd->args[i]);
        if (i < cmd->arg_count - 1) {
            strcat(command, " ");
        }
    }

    pid_t pid = fork();
    if (pid == -1) {
        snprintf(recp_msg, sizeof(recp_msg), "Could not fork for command execution: %s",
                strerror(errno));
        log_message(recp_msg, 1);
        return 0;
    }

    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", command, NULL);
        /* If we get here, exec failed */
        snprintf(recp_msg, sizeof(recp_msg), "Could not execute command: %s", strerror(errno));
        log_message(recp_msg, 1);
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                snprintf(recp_msg, sizeof(recp_msg), "Command exited with status %d", exit_status);
                log_message(recp_msg, 1);
                return 0;
            }
        } else if (WIFSIGNALED(status)) {
            snprintf(recp_msg, sizeof(recp_msg), "Command terminated by signal %d", WTERMSIG(status));
            log_message(recp_msg, 1);
            return 0;
        }

        snprintf(recp_msg, sizeof(recp_msg), "Executed: %s", cmd->args[0]);
        log_message(recp_msg, 0);
    }

    return 1;
}


int execute_script(Command *cmd) {
    if (!cmd->script_content) {
        log_message("No script content provided", 1);
        return 0;
    }

    /* Create temporary script file */
    FILE *script_file = fopen(RECP_TEMP_SCRIPT_FILE, "w");
    if (!script_file) {
        snprintf(recp_msg, sizeof(recp_msg), "Could not create temporary script file: %s",
                strerror(errno));
        log_message(recp_msg, 1);
        return 0;
    }

    /* Write script content */
    fprintf(script_file, "%s", cmd->script_content);
    fclose(script_file);

    /* Make script executable */
    if (chmod(RECP_TEMP_SCRIPT_FILE, 0755) == -1) {
        snprintf(recp_msg, sizeof(recp_msg), "Could not make script executable: %s",
                strerror(errno));
        log_message(recp_msg, 1);
        unlink(RECP_TEMP_SCRIPT_FILE);
        return 0;
    }

    /* Execute the script */
    pid_t pid = fork();
    if (pid == -1) {
        snprintf(recp_msg, sizeof(recp_msg), "Could not fork for script execution: %s",
                strerror(errno));
        log_message(recp_msg, 1);
        unlink(RECP_TEMP_SCRIPT_FILE);
        return 0;
    }

    if (pid == 0) {
        execl(RECP_TEMP_SCRIPT_FILE, RECP_TEMP_SCRIPT_FILE, NULL);
        /* If we get here, exec failed */
        snprintf(recp_msg, sizeof(recp_msg), "Could not execute script: %s", strerror(errno));
        log_message(recp_msg, 1);
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
        unlink(RECP_TEMP_SCRIPT_FILE);

        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                snprintf(recp_msg, sizeof(recp_msg), "Script exited with status %d", exit_status);
                log_message(recp_msg, 1);
                return 0;
            }
        } else if (WIFSIGNALED(status)) {
            snprintf(recp_msg, sizeof(recp_msg), "Script terminated by signal %d", WTERMSIG(status));
            log_message(recp_msg, 1);
            return 0;
        }

        log_message("Script executed successfully", 0);
    }

    return 1;
}

int execute_command(Command *cmd) {
    switch (cmd->type) {
        case OP_CREATE_DIR:
            return execute_create_dir(cmd);
        case OP_CREATE_FILE:
            return execute_create_file(cmd);
        case OP_MOVE:
            return execute_move(cmd);
        case OP_COPY:
            return execute_copy(cmd);
        case OP_DELETE:
            return execute_delete(cmd);
        case OP_EXEC:
            return execute_exec(cmd);
        case OP_SCRIPT:
            return execute_script(cmd);
        default:
            return 0;
    }
}


int process_recipe(const char* filename) {
    FILE *recipe_file = fopen(filename, "r");
    if (!recipe_file) {
        snprintf(recp_msg, sizeof(recp_msg), "Could not open recipe file %s: %s",
                filename, strerror(errno));
        log_message(recp_msg, 1);
        return 0;
    }

    Command cmd;
    char line[RECP_MAX_LINE_LENGTH];
    int line_num = 0;
    int success = 1;

    while (fgets(line, sizeof(line), recipe_file)) {
        line_num++;

        /* Skip empty lines and comments */
        if (line[0] == '\n' || line[0] == '#' || line[0] == '\0') {
            continue;
        }

        /* Remove trailing newline */
        line[strcspn(line, "\n")] = '\0';

        if (!parse_command(recipe_file, line, &cmd, &line_num)) {
            snprintf(recp_msg, sizeof(recp_msg), "Syntax error in recipe file at line %d", line_num);
            log_message(recp_msg, 1);
            success = 0;
            continue;
        }

        if (!execute_command(&cmd)) {
            snprintf(recp_msg, sizeof(recp_msg), "Command failed at line %d", line_num);
            log_message(recp_msg, 1);
            success = 0;
        }

        free_command(&cmd);
    }

    fclose(recipe_file);
    return success;
}


int main(int argc, char **argv) {
    // TODO: Usage
    // TODO: Search for recipes
    (void) argc;
    (void) argv;

    log_message("Started cooking", 0);

    if (!process_recipe("recipe.txt")) {
        log_message("Cooking completed with errors", 1);
        return 1;
    }

    log_message("Cooking completed successfully", 0);
    printf("OK\n");
    return 0;
}
