#define LCHK_IMPLEMENTATION
#include "./lchk.h"

int main(int argc, char **argv) {

    // EXAMPLE: Reading files.
    /* 
    File_Content content = read_file_content("main.c");
    if (!content.data) return 1;
    printf("Read %zu bytes\n", content.length);
    free_file_content(&content);
    */

    // EXAMPLE: Controlling modules in runtime.
    /*
    lchk_enable_module(lchk_find_module_auto("test"), false);
    lchk_set_module_config(lchk_find_module_auto("1"), hello, NULL);
    */

    // EXAMPLE: Explicitly getting json result.
    /*
    char *json = lchk_generate_json(&options, &result);
    lchk_print_json(json, stdout);
    lchk_free_json(json);
    */

    Lchk_Result result  = {0};
    Lchk_Options options = {0};

    lchk_parse_options(&options, argc, argv);

    lchk_run_module(&options, &result, options.task_id);

    lchk_generate_and_print_json(&options, &result, stdout);
    
    // EXAMPLE: Using linker sections approach.
    /*
    lchk_list_func();
    lchk_call_func(&options, &result, "foo", NULL);
    lchk_call_func(&options, &result, "also_bar", NULL);
    */

    return 0;
}
