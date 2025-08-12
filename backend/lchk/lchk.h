/* Copyright (c) Dmitry Ivanov, 2025
 * lchk.h - v0.1 - Linux assignment checking framework - MIT license */

#ifndef LCHK_H
#define LCHK_H

#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <time.h>


#define LCHK_MAX_FEEDBACK_ITEMS 20
#define LCHK_MAX_FEEDBACK_LENGTH 256
#define LCHK_MAX_OUTPUT_LENGTH 4096
// TODO: Provide path to log file via command line arguments
#define LCHK_LOG_FILE "lchk.log"

#define UNUSED(x) (void)x

typedef struct {
    char *data;
    size_t length;
} File_Content;

File_Content read_file_content(const char *path);

void free_file_content(File_Content *fc);


#define PGM_MAX_HEIGHT 1024 
#define PGM_MAX_WIDTH 1024


bool read_pgm_image(const char *path, unsigned char image[PGM_MAX_HEIGHT][PGM_MAX_WIDTH],
        int *width, int *height, int *max_val); 

void convert_pgm_to_ascii(unsigned char image[PGM_MAX_HEIGHT][PGM_MAX_WIDTH],
        int width, int height, int max_val, int output_width); 


#define lchk_log_message(options, message, error)                       \
    do {                                                                \
        if ((options)->enable_logging) log_message(message, error);     \
    } while (0)


#define lchk_add_feedback(result, string)                               \
    do {                                                                \
        if ((result)->feedback_count < LCHK_MAX_FEEDBACK_ITEMS) {       \
            snprintf((result)->feedback[(result)->feedback_count],      \
                    LCHK_MAX_FEEDBACK_LENGTH, string);                  \
            (result)->feedback_count++;                                 \
        }                                                               \
    } while (0)


typedef enum {
    LCHK_SEARCH_BY_NAME,
    LCHK_SEARCH_BY_TASK_ID,
    LCHK_SEARCH_AUTO,
} Lchk_Search_Mode;

typedef struct {
    char *task_id;
    bool enable_logging;
    bool enable_feedback;
    Lchk_Search_Mode search_mode;
} Lchk_Options;

typedef struct {
    int grade;
    char feedback
        [LCHK_MAX_FEEDBACK_ITEMS]
        [LCHK_MAX_FEEDBACK_LENGTH];
    int feedback_count;
    void *module_data;
} Lchk_Result;

typedef struct {
    const char *name;
    const char *task_id;
    const char *description;
    const char *version;
    bool enabled;
} Lchk_Module_Info;

typedef struct Lchk_Module {
    Lchk_Module_Info info;
    bool (*check)(Lchk_Options *options, Lchk_Result *result, void *config);
    void *(*create_config)(void);
    void (*free_config)(void *config);
    struct Lchk_Module *next;
} Lchk_Module;

typedef bool (*Lchk_Func)(Lchk_Options *options, Lchk_Result *result, void *config);

typedef struct {
    const char *name;
    Lchk_Func func;
} Lchk_Func_Entry;


void lchk_register_module(Lchk_Module *module);

void lchk_set_module_config(Lchk_Module *module, void *(*create)(void), void (*free)(void*));

void lchk_enable_module(Lchk_Module *module, bool enable);

Lchk_Module *lchk_find_module_by_name(const char *name);

Lchk_Module *lchk_find_module_by_task_id(const char *task_id);

Lchk_Module *lchk_find_module_auto(const char *identifier);

bool lchk_run_module(Lchk_Options *options, Lchk_Result *result, const char *identifier);

void lchk_list_modules(void);


#define LCHK_REGISTER_MODULE(name, task_id, desc, ver, check_fn)                \
    static Lchk_Module name##_module = {                                        \
        .info = {#name, task_id, desc, ver, true},                              \
        .check = check_fn,                                                      \
        .create_config = NULL,                                                  \
        .free_config = NULL                                                     \
    };                                                                          \
    static void name##_register(void) __attribute__((constructor));             \
    static void name##_register(void) { lchk_register_module(&name##_module); } \


 #define LCHK_REGISTER_MODULE_FULL(name, t_id, desc, ver, ch_f, cr_c, fr_c, en) \
    static Lchk_Module name##_module = {                                        \
        .info = {#name, t_id, desc, ver, en},                                   \
        .check = ch_f,                                                         \
        .create_config = cr_c,                                                  \
        .free_config = fr_c                                                     \
    };                                                                          \
    static void name##_register(void) __attribute__((constructor));             \
    static void name##_register(void) { lchk_register_module(&name##_module); } \
  

#define LCHK_LD_REGISTER_FUNC(func)                                             \
    __attribute__((used, section("func_registry")))                             \
    static const Lchk_Func_Entry _reg_##func = {#func, (Lchk_Func)func}


#define LCHK_LD_REGISTER_ALIAS(func, alias)                                     \
    __attribute__((used, section("func_registry")))                             \
    static const Lchk_Func_Entry _reg_##alias = {#alias, (Lchk_Func)func}


void lchk_parse_options(Lchk_Options *options, int argc, char **argv);

char *lchk_generate_json(Lchk_Options *options, Lchk_Result *result);

void lchk_print_json(const char *json, FILE *stream);

void lchk_generate_and_print_json(Lchk_Options *options, Lchk_Result *result, FILE *stream);

#define lchk_free_json(json) free(json);

static inline bool lchk_call_func(Lchk_Options *options, Lchk_Result *result, 
        const char *name, void *config) {
    extern const Lchk_Func_Entry __start_func_registry;
    extern const Lchk_Func_Entry __stop_func_registry;
    for (const Lchk_Func_Entry *entry = &__start_func_registry;
            entry < &__stop_func_registry;
            entry++) {
        if (strcmp(name, entry->name) == 0) {
            entry->func(options, result, config);
            return true;
        }
    }
    // TODO: Probably, there is no easy way to log from inside this function.
    // Or we can move it to implementation section and assume that it is never
    // called from any translation unit where LCHK_IMPLEMENTATION is not defined.
    // But that's gross...
    //snprintf(lchk_msg, sizeof(lchk_msg), "No function found by name '%s'\n", name);
    //lchk_log_message(options, lchk_msg, 1);
    return false;
}

static inline void lchk_list_func(void) {
    extern const Lchk_Func_Entry __start_func_registry;
    extern const Lchk_Func_Entry __stop_func_registry;

    printf("Available functions:\n");
    printf("%-15s |\n", "     name");
    printf("----------------+\n");
    for (const Lchk_Func_Entry *entry = &__start_func_registry;
            entry <&__stop_func_registry;
            entry++) {
        printf("%-15.15s |\n", entry->name);
    }
}

#endif /* LCHK_H */




#ifdef LCHK_IMPLEMENTATION

#define JIM_IMPLEMENTATION
#include "./thirdparty/jim2.h"

Lchk_Module *lchk_module_list = NULL;
static char lchk_msg[256];

static void log_message(const char *message, int is_error) {
    time_t now;
    time(&now);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp)-1] = '\0';   

    FILE *log = fopen(LCHK_LOG_FILE, "a+");
    if (log) {
        fprintf(log, "[%s] %s: %s\n", timestamp, is_error ? "ERROR" : "INFO", message);
        fclose(log);
    }

    if (is_error) {
        fprintf(stderr, "ERROR: %s\n", message);
    }
}

void lchk_register_module(Lchk_Module *module) {
    if (!module || !module->info.name) return;
    module->next = lchk_module_list;
    lchk_module_list = module;
}

void lchk_enable_module(Lchk_Module *module, bool enable) {
    if (!module) return;
    module->info.enabled = enable;
}

void lchk_set_module_config(Lchk_Module *module, void *(*create)(void), void (*free)(void*)) {
    if (!module) return;
    module->create_config = create;
    module->free_config = free;
}

static Lchk_Module *find_module(const char *identifier, Lchk_Search_Mode mode) {
    if (!identifier) return NULL;

    switch (mode) {
        case LCHK_SEARCH_BY_NAME:
            return lchk_find_module_by_name(identifier);
        case LCHK_SEARCH_BY_TASK_ID:
            return lchk_find_module_by_task_id(identifier);
        case LCHK_SEARCH_AUTO:
        default: {
            Lchk_Module *module = lchk_find_module_by_name(identifier);
            if (!module) module = lchk_find_module_by_task_id(identifier);
            return module;
         }
    }
}

Lchk_Module *lchk_find_module_by_name(const char *name) {
    if (!name) return NULL;

    for (Lchk_Module *module = lchk_module_list; module; module = module->next) {
        if (strcmp(module->info.name, name) == 0) {
            return module;
        }
    }
    return NULL;
}

Lchk_Module *lchk_find_module_by_task_id(const char *task_id) {
    if (!task_id) return NULL;

    for (Lchk_Module *module = lchk_module_list; module; module = module->next) {
        if (strcmp(module->info.task_id, task_id) == 0) {
            return module;
        }
    }
    return NULL;
}

Lchk_Module *lchk_find_module_auto(const char *identifier) {
    return find_module(identifier, LCHK_SEARCH_AUTO);
}

bool lchk_run_module(Lchk_Options *options, Lchk_Result *result, const char *identifier) {
    if (!identifier || !result || !options) return false;

    memset(result, 0, sizeof(Lchk_Result));
    
    Lchk_Module *module = find_module(identifier, options->search_mode);
    if (!module) {
        snprintf(lchk_msg, sizeof(lchk_msg), "No module found for '%s' (search mode: %d)\n", 
                identifier, options->search_mode);
        lchk_log_message(options, lchk_msg, 1);
        return false;
    }

    if (!module->info.enabled) {
        snprintf(lchk_msg, sizeof(lchk_msg), "Module '%s' is disabled", module->info.name);
        lchk_log_message(options, lchk_msg, 1);
        return false;
    }

    /* Callback for creating config if set */
    void *config = module->create_config ? module->create_config() : NULL;
    
    /* Perform main logic */
    bool success = module->check(options, result, config);
    
    /* Callback for freeing config if set */
    if (module->free_config && config) module->free_config(config);

    return success;
}

void lchk_list_modules(void) {
    printf("Available modules:\n");
    printf("%-15s | %-10s | %-30s | %-10s | %-10s\n",
            "      name", " task id", "          description", " version", "  state");
    printf("----------------+------------+--------------------------------+"
            "------------+------------\n");

    for (Lchk_Module *module = lchk_module_list; module; module = module->next) {
        printf("%-15.15s | %-10.10s | %-30.30s | %-10.10s | %-10.10s\n",
                module->info.name,
                module->info.task_id,
                module->info.description,
                module->info.version,
                module->info.enabled ? "enabled" : "disabled");
    }
}


#define da_append(arr, item)                                                            \
    do {                                                                                \
        if ((arr) == NULL || (da_header(arr)->capacity == da_header(arr)->length)) {    \
            da_grow((void**)&(arr), sizeof(*(arr)));                                    \
        }                                                                               \
        (arr)[da_header(arr)->length++] = (item);                                       \
    } while (0)                                                                         \

#define da_free(arr) ((arr) ? (free(da_header(arr)), (arr) = NULL) : 0)

#define da_length(arr) ((arr) ? da_header(arr)->length : 0)


typedef struct {
    size_t capacity;
    size_t length;
} Da_Header;


static Da_Header *da_header(void *arr) {
    return ((Da_Header*)(arr)) - 1;
}

static void da_grow(void **arr, size_t elem_size) {
    size_t new_cap = (arr && *arr) ? da_header(*arr)->capacity * 2 : 16;
    Da_Header *new_header = realloc(
            (arr && *arr) ? da_header(*arr) : NULL,
            sizeof(Da_Header) + new_cap * elem_size);
    if (!new_header) {
        // TODO: Logging?
        fprintf(stderr, "Buy more RAM lol\n");
        exit(1);
    }

    new_header->capacity = new_cap;
    if (!arr || !*arr) {
        new_header->length = 0;
    }

    *arr = (void*)(new_header + 1);
}

File_Content read_file_content(const char *path) {
    File_Content result = {0};
    FILE *file = NULL;

    file = fopen(path, "rb");
    if (!file) {
        // TODO: Logging?
        fprintf(stderr, "Could not open %s: %s\n", path, strerror(errno));
        return result;
    }

    char *buffer = NULL;
    char chunk[1024];
    size_t bytes_read;

    while ((bytes_read = fread(chunk, 1, sizeof(chunk), file)) > 0) {
        for (size_t i = 0; i < bytes_read; ++i) {
            da_append(buffer, chunk[i]);
        }
    }

    if (ferror(file)) {
        // TODO: Logging?
        fprintf(stderr, "Error reading file %s: %s\n", path, strerror(errno));
        da_free(buffer);
        fclose(file);
        return result;
    }

    da_append(buffer, '\0');
    result.data = buffer;
    result.length = da_length(buffer) - 1;
    fclose(file);
    return result;
}

void free_file_content(File_Content *fc) {
    if (fc && fc->data) {
        da_free(fc->data);
        fc->data = NULL;
        fc->length = 0;
    }
}


bool read_pgm_image(const char *path, unsigned char image[PGM_MAX_HEIGHT][PGM_MAX_WIDTH],
        int *width, int *height, int *max_val) {
    FILE *file = fopen(path, "rb");
    if (!file) {  
        fprintf(stderr, "Could not open %s: %s\n", path, strerror(errno));
        return false;
    }

    char magic[3];
    if (fscanf(file, "%2s", magic) != 1 || strcmp(magic, "P5") != 0) {
        fprintf(stderr, "Not a binary PGM file (P5): %s\n", path);
        fclose(file);
        return false;
    }

    if (fscanf(file, "%d %d %d", width, height, max_val) != 3) {
        fprintf(stderr, "Invalid PGM header: %s\n", path);
        fclose(file);
        return false;
    }

    /* Skip whitespace after header */
    fgetc(file);

    /* Read pixel data */
    for (int y = 0; y < *height; ++y) {
        for (int x = 0; x < *width; ++x) {
            int pixel = fgetc(file);
            if (pixel == EOF) {
                fprintf(stderr, "Unexpected end of file: %s\n", path);
                fclose(file);
                return false;
            }
            image[y][x] = (unsigned char)pixel;
        }
    }

    fclose(file);
    return true;
}

void convert_pgm_to_ascii(unsigned char image[PGM_MAX_HEIGHT][PGM_MAX_WIDTH],
        int width, int height, int max_val, int output_width) {
    
    const char *ASCII_CHARS = " .,:;+*?%S#@";
    const int ascii_len = strlen(ASCII_CHARS) - 1;
    
    //float aspect_ratio = (float)height / (float)width;
    int output_height = (output_width * height * 45) / (width * 100);

    //float x_step = (float)width / (float)output_width;
    //float y_step = (float)height / (float)output_height;

    for (int y = 0; y < output_height; ++y) {
        for (int x = 0; x < output_width; ++x) {
            int src_x = (x * width / output_width);
            int src_y = (y * height / output_height);

            int gray = image[src_y][src_x] * ascii_len / max_val;
            
            if (gray < 0) gray = 0;
            if (gray > ascii_len) gray = ascii_len;

            putchar(ASCII_CHARS[gray]);
        }
        putchar('\n');
    }
}


static void usage(const char *progname) {
    printf("Usage: %s -t <task_id> [options]\n", progname);
    printf("Options:\n");
    printf("  -t, --task       Task ID / module (required)\n");
    printf("  -l, --log        Enable logging\n");
    printf("  -f, --feedback   Enable feedback in output\n");
    printf("  -m, --modules    List all modules\n");
    printf("  -h, --help       Show this help message\n");
}

void lchk_parse_options(Lchk_Options *options, int argc, char **argv) {
    options->enable_logging = false;
    options->enable_feedback = false;
    /* Auto search by default */
    options->search_mode = LCHK_SEARCH_AUTO;

    static struct option long_options[] = {
        {"task", required_argument, 0, 't'},
        {"log", no_argument, 0, 'l'},
        {"feedback", no_argument, 0, 'f'},
        {"modules", no_argument, 0, 'm'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "t:lfmh", long_options, &option_index))) {
        if (opt == -1) break;

        switch (opt) {
            case 't':
                options->task_id = strdup(optarg);
                break;
            case 'l':
                options->enable_logging = true;
                break;
            case 'f':
                options->enable_feedback = true;
                break;
            case 'm':
                lchk_list_modules();
                exit(0);
            case 'h':
                usage(argv[0]);
                exit(0);
            case '?':
                usage(argv[0]);
                exit(1);
            default:
                /* In theory, we should never get here */
                exit(1);
        }
    }

    if (!options->task_id) {
        fprintf(stderr, "Task ID / module is required\n");
        usage(argv[0]);
        exit(1);
    }
}

char *lchk_generate_json(Lchk_Options *options, Lchk_Result *result) {
    Jim jim = { .pp = 4 };

    jim_object_begin(&jim);
    jim_member_key(&jim, "task_id");
    jim_string(&jim, options->task_id);
    jim_member_key(&jim, "grade");
    jim_integer(&jim, result->grade);

    if (result->feedback_count > 0 && options->enable_feedback) {
        jim_member_key(&jim, "feedback");
        jim_array_begin(&jim);
        for (int i = 0; i < result->feedback_count && i < LCHK_MAX_FEEDBACK_ITEMS; ++i) {
            jim_string(&jim, result->feedback[i]);
        }
        jim_array_end(&jim);
    }

    jim_object_end(&jim);
    
    char *json = malloc(jim.sink_count + 1);
    if (!json) return NULL;
    
    // Alternative is to make static buffer:
    // static char json[LCHK_MAX_OUTPUT_LENGTH];
    memcpy(json, jim.sink, jim.sink_count);
    json[jim.sink_count] = '\0';
    /* Free allocated memory in serializer */
    free(jim.sink);
    free(jim.scopes);
    /* Caller should free */
    return json;
}

void lchk_print_json(const char *json, FILE *stream) {
    fprintf(stream, "%s\n", json);
}

void lchk_generate_and_print_json(Lchk_Options *options, Lchk_Result *result, FILE *stream) {
    char *json = lchk_generate_json(options, result);
    fprintf(stream, "%s\n", json);
    free(json);
}


#endif /* LCHK_IMPLEMENTATION */
