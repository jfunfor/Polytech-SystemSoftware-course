#include "lchk.h"

int main(int argc, char *argv[]) {

    /* 1. Create 'lchk_args_t' structure and parse input arguments */
    lchk_args_t args = {0};
    lchk_parse_args(argc, argv, &args);

    /* 2. Optional: get task token and hmac secret from file */
    //args.task_token = lchk_read_task_token(LCHK_DEFAULT_TASK_TOKEN_PATH);
    //args.hmac_secret = lchk_read_secret(LCHK_DEFAULT_SECRET_PATH);

    /* 3. Perform check of the selected task */
    int grade = 100;
    char* feedback = "You are dumb lmao";

    /* 4. Build response .json file */
    char *json = lchk_build_result_json(&args, grade, feedback);

    /* 5. Send response to the frontend */
    if (!lchk_send_result(json, args.url)) {
        fprintf(stderr, "[ERROR] Could not send data to API endpoint\n");
    }

    /* 6. Optional: if doing smth else, better free dynamically allocated memory */
    lchk_free_args(&args);
    free(json);

    return 0;
}
