#include "./lchk.h"

// EXAMPLE: Using linker sections.
static bool foo(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(result); UNUSED(config);
    printf("Foo\n");
    return true;
}
LCHK_LD_REGISTER_FUNC(foo);

static bool bar(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(result); UNUSED(config);
    printf("Bar\n");
    return true;
}
LCHK_LD_REGISTER_FUNC(bar);
LCHK_LD_REGISTER_ALIAS(bar, also_bar);


// EXAMPLE: Using runtime modular approach.
static bool check_task1(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);
    result->grade = 0;
    lchk_add_feedback(result, "this does not work");
    lchk_add_feedback(result, "zero outta ten");
    lchk_add_feedback(result, "lmao you dumb");    
    return true;
}
LCHK_REGISTER_MODULE_FULL(task_1_10_1, "1", "Module for checking task1.10.1", "0.1", 
        check_task1, NULL, NULL, true)

// EXAMPLE: Adding pre/post-checking callbacks.
/*
static void *hello(void) {
    unsigned char image[PGM_MAX_HEIGHT][PGM_MAX_WIDTH];
    int width, height, max_val;
    if (!read_pgm_image("image22.pgm", image, &width, &height, &max_val)) return NULL;
    convert_pgm_to_ascii(image, width, height, max_val, 64);
    return NULL;
}
*/

static bool check_task2(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);
    result->grade = 100;
    lchk_add_feedback(result, "all tests ok");
    return true;
}
LCHK_REGISTER_MODULE(task_net_2, "2", "Module for checking task-net-2", "0.1", check_task2)


