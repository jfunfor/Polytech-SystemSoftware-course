#ifndef LCHK_ARGS_H
#define LCHK_ARGS_H

typedef struct _lchk_args_t {
    char **filepaths;                           /* Optional, can be used in checking logic             (-f) */
    char **strings;                             /* Optional, can be used in checking logic             (-s) */
    int *ints;                                  /* Optional, can be used in checking logic             (-n) */
    int file_count, string_count, int_count;    /* Number of optional args above (calculated automatically) */

    int task_id;                                /* ID of task to check                                (-id) */
    int feedback;                               /* Flag indicating feedback is provided        (--feedback) */
    int tt_cli;                                 /* Flag indicating task token had been passed via cli (-ct) */
    int hs_cli;                                 /* Flag indicating secret had been passed via cli     (-cs) */

    char *url;                                  /* REST API endpoint (frontend)                        (-u) */
    char *json_path;                            /* Filepath to save .json locally                      (-j) */

    char *task_token;                           /* Task token (from file or from cli)                       */
    char *hmac_secret;                          /* HMAC secret (from file or from cli)                      */
    char *task_token_path;                      /* Task token path if overriden                       (-ot) */
    char *hmac_secret_path;                     /* HMAC secret path if overriden                      (-os) */

} lchk_args_t;

/* Parses input arguments and provides basic cli interface. Must be called in main() before any logic. */
void lchk_parse_args(int argc, char *argv[], lchk_args_t *args);

/* Frees dynamcally allocated memory for storing input arguments. */
void lchk_free_args(lchk_args_t *args);

/* Prints all arguments to stdout. Use for debug purposes only. */
void lchk_print_args(const lchk_args_t *args);

#endif /* LCHK_ARGS_H */
