#ifndef LCHK_H
#define LCHK_H

/* Common includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <openssl/hmac.h>

/* Library headers */
#include "lchk_args.h"
#include "lchk_token.h"
#include "lchk_crypto.h"
#include "lchk_json.h"
#include "lchk_post.h"

/* Constants */
#define LCHK_INITIAL_CAPACITY 8                                 /* Initial capacity of dynamic arrays */
#define LCHK_MAX_SECRET_LEN 256                                 /* Max length of task token string */
#define LCHK_MAX_TOKEN_LEN 256                                  /* Max length of task token string */
#define LCHK_MAX_RETRIES 3                                      /* Max number of retry attempts in exponential backoff algorithm */

#define LCHK_DEFAULT_TASK_TOKEN_PATH "/etc/lchk_task_token"     /* Default env task token path */
#define LCHK_DEFAULT_SECRET_PATH "/etc/lchk_secret"             /* Default env secret path */

#define LCHK_EMBED_ENV_INFO                                     /* Embed additional info (pwd, timestamp, uid) in .json */

#define LCHK_ENABLE_DEBUG                                       /* Enable debug logging */

#define UNUSED(x)   (void)x

#ifdef LCHK_ENABLE_DEBUG
#define LOG(...) fprintf(stderr, "[DEBUG] " __VA_ARGS__)
#else
#define LOG(...) do {} while (0)
#endif /* LCHK_ENABLE_DEBUG */


#endif /* LCHK_H */
