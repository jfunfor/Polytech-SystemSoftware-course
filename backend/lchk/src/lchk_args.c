#include "lchk.h"

static void print_help(const char *progname) {
    printf("Usage: %s [-f filepath] [-n integer] [-s string] ...\n", progname);
    printf("  -f <path>      Filepath argument (optional, can be repeated)\n");
    printf("  -n <number>    Integer argument (optional, can be repeated)\n");
    printf("  -s <string>    String argument (optional, can be repeated)\n");
    printf("  -u <url>       REST endpoint\n");
    printf("  -j <filename>  Save .json file locally\n");
    printf("  -id <number>   ID of task to check\n");
    printf("  -ot <path>     Override task token path specified in 'lchk_token.h'\n");
    printf("  -os <path>     Override secret path specified in 'lchk_crypto.h'\n");
    printf("  -ct <token>    Provide task token via cli\n");
    printf("  -cs <secret>   Provide secret via cli\n");
    printf("  --feedback     Embed detailed feedback in JSON\n");
    printf("  --help         Show this help message\n");
}

void lchk_parse_args(int argc, char *argv[], lchk_args_t *args) {
    args->filepaths = malloc(sizeof(char*) * LCHK_INITIAL_CAPACITY);
    args->strings = malloc(sizeof(char*) * LCHK_INITIAL_CAPACITY);
    args->ints = malloc(sizeof(int) * LCHK_INITIAL_CAPACITY);
    args->file_count = args->string_count = args->int_count = 0;
    args->task_id = 0;
    args->feedback = 0;
    args->tt_cli = 0;
    args->hs_cli = 0;

    int i = 1;
    while (i < argc) {
        /* '--help' argument */
        if (strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            exit(0);
        }
        /* All other arguments */
        if (strcmp(argv[i], "-f") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "[ERROR] Missing filepath after -f\n");
                exit(1);
            }
            args->filepaths[args->file_count++] = strdup(argv[++i]);
        } else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "[ERROR] Missing string after -s\n");
                exit(1);
            }
            args->strings[args->string_count++] = strdup(argv[++i]);
        } else if (strcmp(argv[i], "-n") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "[ERROR] Missing integer after -n\n");
                exit(1);
            }
            char *end;
            int val = strtol(argv[++i], &end, 10);
            if (*end != '\0') {
                fprintf(stderr, "[ERROR] Invalid integer: %s\n", argv[i]);
                exit(1);
            }
            args->ints[args->int_count++] = val;
        } else if (strcmp(argv[i], "-u") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "[ERROR] Missing URL after -u\n");
                exit(1);
            }
            args->url = strdup(argv[++i]);
            LOG("Set REST API URL to: %s\n", args->url);
        } else if (strcmp(argv[i], "-j") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "[ERROR] Missing filepath after -j\n");
                exit(1);
            }
            args->json_path = strdup(argv[++i]);
            LOG("Will save JSON to: %s\n", args->json_path);
        } else if (strcmp(argv[i], "-ot") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "[ERROR] Missing filepath after -ot\n");
                exit(1);
            }
            args->task_token_path = strdup(argv[++i]);
            LOG("Set task token path to: %s\n", args->task_token_path);
        } else if (strcmp(argv[i], "-os") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "[ERROR] Missing filepath after -os\n");
                exit(1);
            }
            args->hmac_secret_path = strdup(argv[++i]);
            LOG("Set secret path to: %s\n", args->hmac_secret_path);
        } else if (strcmp(argv[i], "-id") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "[ERROR] Missing task id after -id\n");
                exit(1);
            }
            char *end;
            int val = strtol(argv[++i], &end, 10);
            if (*end != '\0') {
                fprintf(stderr, "[ERROR] Invalid task id: %s\n", argv[i]);
                exit(1);
            }
            args->task_id = val;
            LOG("Set task_id to: %d\n", args->task_id);
        } else if (strcmp(argv[i], "--feedback") == 0) {
            args->feedback = 1;
            LOG("Feedback on tasks is now provided\n");
        } else if (strcmp(argv[i], "-ct") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "[ERROR] Missing task token after -ct\n");
                exit(1);
            }
            args->task_token = strdup(argv[++i]);
            args->tt_cli = 1;
            LOG("Set task token: %s\n", args->task_token);
        } else if (strcmp(argv[i], "-cs") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "[ERROR] Missing secret after -ct\n");
                exit(1);
            }
            args->hmac_secret = strdup(argv[++i]);
            args->hs_cli = 1;
            LOG("Set secret: %s\n", args->hmac_secret);
        } else {
            fprintf(stderr, "[ERROR] Unknown argument: %s\n", argv[i]);
            exit(1);
        }

        /* Reallocate if needed (for simplicity, double the capacity each time) */
        if (args->file_count % LCHK_INITIAL_CAPACITY == 0)
            args->filepaths = realloc(args->filepaths, sizeof(char*) * (args->file_count + LCHK_INITIAL_CAPACITY));
        if (args->int_count % LCHK_INITIAL_CAPACITY == 0)
            args->ints = realloc(args->ints, sizeof(int) * (args->int_count + LCHK_INITIAL_CAPACITY));
        if (args->string_count % LCHK_INITIAL_CAPACITY == 0)
            args->strings = realloc(args->strings, sizeof(char*) * (args->string_count + LCHK_INITIAL_CAPACITY));

        i++;
    }

    /* Resolve ambiguity if -cs/-ct and -os/-ot are both present for some reason */
    if (args->tt_cli && args->task_token_path != NULL) {
        args->task_token_path = NULL;
    }
    if (args->hs_cli && args->hmac_secret_path != NULL) {
        args->hmac_secret_path = NULL;
    }
}

void lchk_free_args(lchk_args_t *args) {
    for (int i = 0; i < args->file_count; ++i)
        free(args->filepaths[i]);
    for (int i = 0; i < args->string_count; ++i)
        free(args->strings[i]);
    free(args->filepaths);
    free(args->strings);
    free(args->ints);
}

void lchk_print_args(const lchk_args_t *args) {
    printf("Filepaths (%d):\n", args->file_count);
    for (int i = 0; i < args->file_count; ++i)
        printf("  [%d] %s\n", i, args->filepaths[i]);

    printf("Strings (%d):\n", args->string_count);
    for (int i = 0; i < args->string_count; ++i)
        printf("  [%d] %s\n", i, args->strings[i]);

    printf("Integers (%d):\n", args->int_count);
    for (int i = 0; i < args->int_count; ++i)
        printf("  [%d] %d\n", i, args->ints[i]);
}
