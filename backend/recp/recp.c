/* Copyright (c) Dmitry Ivanov, 2025
 * recp.c - v0.4 - Linux environment setup tool - MIT license */

#include <inttypes.h>
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
#include <getopt.h>
#include <ctype.h>

#define RECP_MAX_LINE_LENGTH        4096
#define RECP_MAX_ARGS               64
#define RECP_TEMP_SCRIPT_FILE       "/tmp/recp_script.sh"
#define RECP_MAX_FILE_SIZE          102400
#define RECP_INITIAL_SCRIPT_SIZE    1024
#define RECP_PP_MAX_VARS            100
#define RECP_PP_MAX_VAR_NAME        64
#define RECP_PP_MAX_VAR_VALUE       256
#define RECP_PP_VAR_PREFIX          '%'
#define RECP_PP_VAR_SUFFIX          '%'


static char msg[512];
static char *recipe_file;
static char *log_file;
static int enable_logging;


typedef enum {
    OP_CREATE_DIR,
    OP_CREATE_FILE,
    OP_MOVE,
    OP_COPY,
    OP_EXEC,
    OP_DELETE,
    OP_SCRIPT,
    OP_BACKGROUND,
    OP_DEFINE,
    OP_UNKNOWN
} Op_Type;


typedef struct {
    Op_Type type;
    char *args[RECP_MAX_ARGS];
    char *script_content;
    int arg_count;
} Command;


typedef struct {
    char name[RECP_PP_MAX_VAR_NAME];
    char value[RECP_PP_MAX_VAR_VALUE];
} Variable;


static Variable variables[RECP_PP_MAX_VARS];
static int var_count;

/* Currently unused */
/*
typedef struct {
    char* recipe_file;
    char* log_file;
    int enable_logging;
} Options;
*/

void log_message(const char *message, int is_error) {
    time_t now;
    time(&now);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp)-1] = '\0';

    if (enable_logging) {
        FILE *log = fopen(log_file, "a+");
        if (log) {
            fprintf(log, "[%s] %s: %s\n", timestamp, is_error ? "ERROR" : "INFO", message);
            fclose(log);
        }
    }

    if (is_error) {
        fprintf(stderr, "ERROR: %s\n", message);
    }
}


#define RECP_LOG(is_error, format, ...)                     \
    do {                                                    \
        snprintf(msg, sizeof(msg), format, ##__VA_ARGS__);  \
        log_message(msg, is_error);                         \
    } while (0)


int pp_var_name_valid(const char *name) {
    if (!name || !name[0] || !isalpha(name[0])) {
        return 0;
    }
    
    for (int i = 0; name[i]; ++i) {
        if (!isalnum(name[i]) && name[i] != '_') {
            return 0;
        }
    }

    return 1;
}


int pp_set_var(const char *name, const char *value) {
    if (var_count >= RECP_PP_MAX_VARS) {
        RECP_LOG(1, "Maximum variable limit reached");
        return 0;
    }

    if (!pp_var_name_valid(name)) {
        RECP_LOG(1, "Invalid variable name: %s", name);
        return 0;
    }

    /* Check if variable already exists */
    for (int i = 0; i < var_count; ++i) {
        if (strcmp(variables[i].name, name) == 0) {
            strncpy(variables[i].value, value, RECP_PP_MAX_VAR_VALUE - 1);
            variables[i].value[RECP_PP_MAX_VAR_VALUE - 1] = '\0';
            return 1;
        }
    }

    /* Add new variable */
    strncpy(variables[var_count].name, name, RECP_PP_MAX_VAR_NAME - 1);
    strncpy(variables[var_count].value, value, RECP_PP_MAX_VAR_VALUE - 1);
    variables[var_count].name[RECP_PP_MAX_VAR_NAME - 1] = '\0';
    variables[var_count].value[RECP_PP_MAX_VAR_VALUE - 1] = '\0';
    var_count++;

    return 1;
}


const char *pp_get_var(const char *name) {
    for (int i = 0; i < var_count; ++i) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    return NULL;
}


int pp_require_subst(const char *input) {
    if (!input) return 0;
    
    const char *pos = input;
    while (*pos) {
        if (*pos == RECP_PP_VAR_PREFIX) {
            const char *var_start = pos + 1;
            const char *var_end = strchr(var_start, RECP_PP_VAR_SUFFIX);
            if (var_end) {
                /* Potential variable found, check if it's defined */
                size_t var_name_len = var_end - var_start;
                if (var_name_len > 0 && var_name_len < RECP_PP_MAX_VAR_NAME) {
                    char var_name[RECP_PP_MAX_VAR_NAME];
                    strncpy(var_name, var_start, var_name_len);
                    var_name[var_name_len] = '\0';

                    if (pp_get_var(var_name)) {
                        /* Variable exists and needs susbstitution */
                        return 1;
                    }
                }
                /* Skipping past this variable */
                pos = var_end;
            }
        }
        pos++;
    }

    /* No susbstitution needed */
    return 0;
}


char *pp_subst_var(const char *input) {
    if (!input) return NULL;

    size_t input_len = strlen(input);
    size_t output_size = input_len * 2 + 1;
    char *output = malloc(output_size);
    if (!output) return NULL;

    output[0] = '\0';
    size_t output_pos = 0;
    const char *pos = input;
 
    while (*pos) {
        /* Look for variable start marker */
        if (*pos == RECP_PP_VAR_PREFIX) {
            const char *var_start = pos + 1;
            const char *var_end = strchr(var_start, RECP_PP_VAR_SUFFIX);

            if (var_end) {
                /* Extract variable name */
                size_t var_name_len = var_end - var_start;
                if (var_name_len > 0 && var_name_len < RECP_PP_MAX_VAR_NAME) {
                    char var_name[RECP_PP_MAX_VAR_NAME];
                    strncpy(var_name, var_start, var_name_len);
                    var_name[var_name_len] = '\0';

                    /* Get variable value */
                    const char *var_value = pp_get_var(var_name);
                    if (var_value) {
                        size_t value_len = strlen(var_value);
                        if (output_pos + value_len >= output_size) {
                            /* Means that we need to resize ouput buffer */
                            output_size = output_pos + value_len + 1;
                            char *new_output = realloc(output, output_size);
                            if (!new_output) {
                                free(output);
                                return NULL;
                            }
                            output = new_output;
                        }
                        
                        strcpy(output + output_pos, var_value);
                        output_pos += value_len;
                        pos = var_end + 1;
                        continue;

                    } else {
                        /* Variable not found, keeping original text */
                        RECP_LOG(1, "Variable %s not defined", var_name);
                    }
                }
            }
        }

        if (output_pos + 1 >= output_size) {
            output_size *= 2;
            char *new_output = realloc(output, output_size);
            if (!new_output) {
                free(output);
                return NULL;
            }
            output = new_output;
        }

        output[output_pos++] = *pos++;
        output[output_pos] = '\0';
    }
    
    return output;
}


void pp_log_vars() {
    if (var_count == 0) {
        RECP_LOG(0, "No variables defined");
        return;
    }

    RECP_LOG(0, "Defined variables (%d):", var_count);

    for (int i = 0; i < var_count; ++i) {
        RECP_LOG(0, "%.64s = %.256s", variables[i].name, variables[i].value);
    }
}


Op_Type parse_op_type(const char *op_str) {
    if (strcmp(op_str, "CREATE_DIR") == 0) return OP_CREATE_DIR;
    if (strcmp(op_str, "CREATE_FILE") == 0) return OP_CREATE_FILE;
    if (strcmp(op_str, "MOVE") == 0) return OP_MOVE;
    if (strcmp(op_str, "COPY") == 0) return OP_COPY;
    if (strcmp(op_str, "EXEC") == 0) return OP_EXEC;
    if (strcmp(op_str, "BACKGROUND") == 0) return OP_BACKGROUND;
    if (strcmp(op_str, "DELETE") == 0) return OP_DELETE;
    if (strcmp(op_str, "SCRIPT") == 0) return OP_SCRIPT;
    if (strcmp(op_str, "DEFINE") == 0) return OP_DEFINE;
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
        
        char *line_to_append = buffer;
        char *subst_line = NULL;

        /* Apply variable susbstitution if needed */
        if (pp_require_subst(buffer)) {
            subst_line = pp_subst_var(buffer);
            if (!subst_line) {
                RECP_LOG(1, "Substitution failed at line %d", *line_num);
                free(content);
                return NULL;
            }
            line_to_append = subst_line;
        }

        /* Check if we need to grow our buffer */
        size_t new_length = strlen(content) + strlen(line_to_append);
        if (new_length + 1 > content_size) {
            content_size *= 2;
            char *new_content = realloc(content, content_size);
            if (!new_content) {
                free(content);
                free(subst_line);
                return NULL;
            }
            content = new_content;
        }

        strcat(content, line_to_append);
        free(subst_line);
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
        RECP_LOG(1, "Unknown operation: %s", op_str);
        return 0;
    }

    if (cmd->type == OP_SCRIPT) {
        cmd->script_content = read_script_block(file, line_num);
        if (!cmd->script_content) {
            RECP_LOG(1, "Invalid or incomplete SCRIPT block");
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
        RECP_LOG(1, "CREATE_DIR requires at least one argument");
        return 0;
    }

    for (int i = 0; i < cmd->arg_count; i++) {
        const char *dir_path = cmd->args[i];

        /* Check if directory already exists */
        struct stat st;
        if (stat(dir_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                RECP_LOG(0, "Directory already exists: %s", dir_path);
                continue;
            } else {
                RECP_LOG(1, "Path exists but is not a directory: %s", dir_path);
                return 0;
            }
        }

        /* Create directory with full permissions */
        if (mkdir(dir_path, 0777) == -1) {
            RECP_LOG(1, "Could not create directory %s: %s", dir_path, strerror(errno));
            return 0;
        }
        
        RECP_LOG(0, "Created directory: %s", dir_path);
    }

    return 1;
}


int execute_create_file(Command *cmd) {
    if (cmd->arg_count < 1) {
        RECP_LOG(1, "CREATE_FILE requires at least one argument");
        return 0;
    }

    for (int i = 0; i < cmd->arg_count; i++) {
        const char *file_path = cmd->args[i];

        /* Check if file already exists */
        if (access(file_path, F_OK) == 0) {
            RECP_LOG(0, "File already exists: %s", file_path);
            continue;
        }

        /* Create empty file */
        int fd = open(file_path, O_WRONLY | O_CREAT, 0666);
        if (fd == -1) {
            RECP_LOG(1, "Could not create file %s: %s", file_path, strerror(errno));
            return 0;
        }
        close(fd);

        RECP_LOG(0, "Created file: %s", file_path);
    }

    return 1;
}


int execute_move(Command *cmd) {
    if (cmd->arg_count != 2) {
        RECP_LOG(1, "MOVE requires 2 arguments: source and destination");
        return 0;
    }

    const char *src = cmd->args[0];
    const char *dst = cmd->args[1];

    if (rename(src, dst) != 0) {
        RECP_LOG(1, "Could not move %s to %s: %s", src, dst, strerror(errno));
        return 0;
    }

    RECP_LOG(0, "Moved %s to %s", src, dst);

    return 1;
}


int execute_copy(Command *cmd) {
    if (cmd->arg_count != 2) {
        RECP_LOG(1, "COPY requires 2 arguments: source and destination");
        return 0;
    }

    const char *src = cmd->args[0];
    const char *dst = cmd->args[1];

    FILE *src_file = fopen(src, "rb");
    if (!src_file) {
        RECP_LOG(1, "Could not open source file %s: %s", src, strerror(errno));
        return 0;
    }

    FILE *dst_file = fopen(dst, "wb");
    if (!dst_file) {
        RECP_LOG(1, "Could not create destination file %s: %s", dst, strerror(errno));
        fclose(src_file);
        return 0;
    }

    char buffer[RECP_MAX_FILE_SIZE];
    size_t bytes;
        while ((bytes = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
        if (fwrite(buffer, 1, bytes, dst_file) != bytes) {
            
            RECP_LOG(1, "Could not write to destination file %s", dst);
            fclose(src_file);
            fclose(dst_file);
            return 0;
        }
    }

    fclose(src_file);
    fclose(dst_file);
    
    RECP_LOG(0, "Copied %s to %s", src, dst);

    return 1;
}


int execute_delete(Command *cmd) {
    if (cmd->arg_count < 1) {
        RECP_LOG(1, "DELETE requires at least one argument");
        return 0;
    }

    for (int i = 0; i < cmd->arg_count; i++) {
        const char *path = cmd->args[i];

        struct stat st;
        if (stat(path, &st) != 0) {
            RECP_LOG(1, "Path does not exist: %s", path);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            /* Directory - use rmdir (only works for empty directories) */
            if (rmdir(path) != 0) {
                RECP_LOG(1, "Could not delete directory %s: %s", path, strerror(errno));
                return 0;
            }
        } else {
            /* File - use unlink */
            if (unlink(path) != 0) {
                RECP_LOG(1, "Could not delete file %s: %s", path, strerror(errno));
                return 0;
            }
        }

        RECP_LOG(0, "Deleted: %s", path);
    }

    return 1;
}


int execute_exec(Command *cmd) {
    if (cmd->arg_count < 1) {
        RECP_LOG(1, "EXEC requires at least one argument");
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
        RECP_LOG(1, "Could not fork for command execution: %s", strerror(errno));
        return 0;
    }

    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", command, NULL);
        /* If we get here, exec failed */
        RECP_LOG(1, "Could not execute command: %s", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                RECP_LOG(1, "Command exited with status %d", exit_status);
                return 0;
            }
        } else if (WIFSIGNALED(status)) {
            RECP_LOG(1, "Command terminated by signal %d", WTERMSIG(status));
            return 0;
        }
        
        RECP_LOG(0, "Executed: %s", cmd->args[0]);
    }

    return 1;
}


int execute_background(Command *cmd) {
    if (cmd->arg_count < 1) {
        RECP_LOG(1, "BACKGROUND requires at least one argument");
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
        RECP_LOG(1, "Could not fork for command execution: %s", strerror(errno));
        return 0;
    }
    
    if (pid == 0) {
        /* Detach from terminal */
        setsid();
        /* Redirect I/O */
        int null_fd = open("/dev/null", O_RDWR);
        if (null_fd != -1) {
            dup2(null_fd, STDIN_FILENO);
            dup2(null_fd, STDOUT_FILENO);
            dup2(null_fd, STDERR_FILENO);
            close(null_fd);
        }
        execl("/bin/sh", "sh", "-c", command, NULL);
        /* If we get here, exec failed */
        RECP_LOG(1, "Could not execute command: %s", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        RECP_LOG(0, "Started background process %s (PID: %d)", command, pid);
    }   

    return 1;
}


int execute_script(Command *cmd) {
    if (!cmd->script_content) {
        RECP_LOG(1, "No script content provided");
        return 0;
    }

    /* Create temporary script file */
    FILE *script_file = fopen(RECP_TEMP_SCRIPT_FILE, "w");
    if (!script_file) {
        RECP_LOG(1, "Could not create temporary script file: %s", strerror(errno));
        return 0;
    }

    /* Write script content */
    fprintf(script_file, "%s", cmd->script_content);
    fclose(script_file);

    /* Make script executable */
    if (chmod(RECP_TEMP_SCRIPT_FILE, 0755) == -1) {
        RECP_LOG(1, "Could not make script executable: %s", strerror(errno));
        unlink(RECP_TEMP_SCRIPT_FILE);
        return 0;
    }

    /* Execute the script */
    pid_t pid = fork();
    if (pid == -1) {
        RECP_LOG(1, "Could not fork for script execution: %s", strerror(errno));
        unlink(RECP_TEMP_SCRIPT_FILE);
        return 0;
    }

    if (pid == 0) {
        execl(RECP_TEMP_SCRIPT_FILE, RECP_TEMP_SCRIPT_FILE, NULL);
        /* If we get here, exec failed */
        RECP_LOG(1, "Could not execute script: %s", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
        unlink(RECP_TEMP_SCRIPT_FILE);

        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                RECP_LOG(1, "Script exited with status %d", exit_status);
                return 0;
            }
        } else if (WIFSIGNALED(status)) {
            RECP_LOG(1, "Script terminated by signal %d", WTERMSIG(status));
            return 0;
        }
        
        RECP_LOG(0, "Script executed successfully");
    }

    return 1;
}


int execute_define(Command *cmd) {
    if (cmd->arg_count != 2) {
        RECP_LOG(1, "DEFINE requires 2 arguments: name and value");
        return 0;
    }

    const char *var_name = cmd->args[0];
    const char *var_value = cmd->args[1];

    if (!pp_set_var(var_name, var_value)) {
        RECP_LOG(1, "Failed to define variable %s", var_name);
        return 0;
    }
    
    RECP_LOG(0, "Defined variable: %s = %s", var_name, var_value);
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
        case OP_BACKGROUND:
            return execute_background(cmd);
        case OP_SCRIPT:
            return execute_script(cmd);
        case OP_DEFINE:
            return execute_define(cmd);
        default:
            return 0;
    }
}


int process_recipe(const char *filename) {
    FILE *recipe_file = fopen(filename, "r");
    if (!recipe_file) {
        RECP_LOG(1, "Could not open recipe file %s: %s", filename, strerror(errno));
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
        
        /* Apply variable susbstitution */
        char *current_line = line;
        char *subst_line = NULL;
        if (pp_require_subst(line)) {
            subst_line = pp_subst_var(line);
            if (!subst_line) {
                RECP_LOG(1, "Substitution failed at line %d", line_num);
                success = 0;
                continue;
            }
            current_line = subst_line;
        }

        if (!parse_command(recipe_file, current_line, &cmd, &line_num)) {
            RECP_LOG(1, "Syntax error in recipe file at line %d", line_num);
            free(subst_line);
            success = 0;
            continue;
        }

        if (!execute_command(&cmd)) {
            RECP_LOG(1, "Command failed at line %d", line_num);
            free(subst_line);
            free_command(&cmd);
            success = 0;
            continue;
        }

        free(subst_line);
        free_command(&cmd);
    }

    fclose(recipe_file);
    return success;
}


void usage(const char *progname) {
    printf("Usage: %s -r <recipe> [options]\n", progname);
    printf("Options:\n");
    printf("  -r, --recipe  Recipe file (required)\n");
    printf("  -l, --log     Save logs to file\n");
    printf("  -h, -- help   Show this help message\n");
}


void parse_options(int argc, char **argv) {

    static struct option long_options[] = {
        {"recipe", required_argument, 0, 'r'},
        {"log", required_argument, 0, 'l'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "r:l:h", long_options, &option_index))) {
        if (opt == -1) break;

        switch (opt) {
            case 'r':
                recipe_file = strdup(optarg);
                break;
            case 'l':
                enable_logging = 1;
                log_file = strdup(optarg);
                break;
            case 'h':
                usage(argv[0]);
                exit(0);
            case '?':
                usage(argv[0]);
                exit(1);
            default:
                exit(1);
        }  
    }
    if (!recipe_file) {
        fprintf(stderr, "Recipe file is required\n");
        usage(argv[0]);
        exit(1);
    }

}


int main(int argc, char **argv) {

    parse_options(argc, argv);
    
    RECP_LOG(0, "Started cooking");

    if (!process_recipe(recipe_file)) {
        RECP_LOG(1, "Cooking completed with errors");
        return 1;
    }

    RECP_LOG(0, "Cooking completed successfully");
    printf("OK\n");
    return 0;
}
