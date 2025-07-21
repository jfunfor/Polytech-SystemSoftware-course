#include "lchk.h"

char *lchk_build_result_json(const lchk_args_t *args, const int grade, const char* feedback) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "grade", grade);

    if (args->feedback && feedback != NULL) {
        cJSON_AddStringToObject(root, "feedback", feedback);
        LOG("Genrated feedback: %s\n", feedback);
    }

    if (args->task_token) {
        cJSON_AddStringToObject(root, "task_token", args->task_token);
        LOG("[LOG] Loaded task token: %s\n", args->task_token);
    }

#ifdef LCHK_EMBED_ENV_INFO
#include <pwd.h>
#include <time.h>
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    const char *user = getenv("USER");
    if (!user) {
        struct passwd *pw = getpwuid(getuid());
        if (pw) user = pw->pw_name;
    }
    char cwd[512];
    getcwd(cwd, sizeof(cwd));
    time_t now = time(NULL);

    cJSON_AddStringToObject(root, "user", user ? user : "unknown");
    cJSON_AddStringToObject(root, "hostname", hostname);
    cJSON_AddStringToObject(root, "working_dir", cwd);
    cJSON_AddNumberToObject(root, "timestamp", (double)now);
#endif /* LCHK_EMBED_ENV_INFO */

    char *json_string = cJSON_PrintUnformatted(root);

    if (args->hmac_secret) {
        char *sig = lchk_hmac_sign(json_string, args->hmac_secret);
        cJSON_AddStringToObject(root, "signature", sig);
    }

    json_string = cJSON_PrintUnformatted(root);

    LOG("Built JSON: %s\n", json_string);

    if (args->json_path) {
        FILE *f = fopen(args->json_path, "w");
        if (!f) {
            fprintf(stderr, "[ERROR] Failed to open %s for writing\n", args->json_path);
        } else {
            fprintf(f, "%s\n", json_string);
            fclose(f);
            LOG("JSON saved to %s\n", args->json_path);
        }
    }

    cJSON_Delete(root);

    return json_string; /* Caller should free */
}

