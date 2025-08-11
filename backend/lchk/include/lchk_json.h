#ifndef LCHK_JSON_H
#define LCHK_JSON_H

#include "lchk.h"

/* Builds response .json file according to the parameters in 'args' and 'grade' value with optional 'feedback'. */
char *lchk_build_result_json(const lchk_args_t *args, const int grade, const char* feedback);

#endif /* LCHK_JSON_H */
