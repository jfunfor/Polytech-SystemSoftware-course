#include "./lchk.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

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
/*
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
*/

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

static bool check_dummy(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);
    result->grade = 100;
    lchk_add_feedback(result, "Pass: All tests ok");
    return true;
}
LCHK_REGISTER_MODULE(dummy, "dummy", "Dummy module", "0.1", check_dummy)


// ============================================================================
//      ____                           _                 _   _             
//     / ___| ___ _ __   ___ _ __ __ _| |  ___  ___  ___| |_(_) ___  _ __  
//    | |  _ / _ \ '_ \ / _ \ '__/ _` | | / __|/ _ \/ __| __| |/ _ \| '_ |
//    | |_| |  __/ | | |  __/ | | (_| | | \__ \  __/ (__| |_| | (_) | | | |
//     \____|\___|_| |_|\___|_|  \__,_|_| |___/\___|\___|\__|_|\___/|_| |_|
// ============================================================================
#include <mntent.h>

#ifndef HOME
#define HOME "/home/flopp/"
#endif // HOME

#define general__CMD_BUFFER_SIZE 1024
#define general__FILE_CONTENT_SIZE 1024

// Trim string 'str' from '\n' and '\r' chars
void general__trim(char *str) {
    int len = strlen(str);
    while (len > 0 && (str[len-1] == '\n' || str[len-1] == '\r')) {
        str[--len] = '\0';
    }
}

// Execute system command and compare is output with expected one
bool general__check_command(const char *command, const char *expected_output) {
    FILE *f;
    char buffer[general__CMD_BUFFER_SIZE];
    bool found = false;

    f = popen(command, "r");
    if (!f) {
        return false;
    }

    while (fgets(buffer, sizeof(buffer), f)) {
        if (expected_output && strstr(buffer, expected_output)) {
            found = true;
        }
    }

    pclose(f);
    return found;
}

// Execute syetem command 
bool general__run_command(const char *command) {
    char silent_cmd[general__CMD_BUFFER_SIZE];
    snprintf(silent_cmd, sizeof(silent_cmd), "%s >/dev/null 2>&1", command);
    return system(silent_cmd) == 0; 
}

// Execute system command 
#define general__run_command_macro(command) (system(command) == 0)

// Execute system command with no return value
#define general__run_command_macro_noreturn(command) (system(command))

// Check if file exists
#define general__file_exists(filename) (access(filename, F_OK) == 0)

// Check if file exists and is executable
bool general__file_executable(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0 && (st.st_mode & S_IXUSR);
}

// Compare file content with expected one 
bool general__check_file_content(const char *filename, const char *expected_content) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        return false;
    }

    char buffer[general__FILE_CONTENT_SIZE];
    bool found = false;

    while (fgets(buffer, sizeof(buffer), f)) {
        if (strstr(buffer, expected_content)) {
            found = true;
            break;
        }
    }

    fclose(f);
    return found;
} 

// Get partition size in kilobytes
long general__get_partition_size_kb(const char *device) {
    char command[256];
    char buffer[128];
    long size_kb = -1;

    snprintf(command, sizeof(command), "blockdev --getsize64 %s 2>/dev/null", device);
    FILE *f = popen(command, "r");
    if (f) {
        if (fgets(buffer, sizeof(buffer), f)) {
            unsigned long long size_bytes;
            if (sscanf(buffer, "%llu", &size_bytes) == 1) {
                size_kb = size_bytes / 1024;
            }
        }
        pclose(f);
    }
    return size_kb;
}

// Check if filesystem type matches 'expected_fs'
bool general__check_filesystem_type(const char *device, const char *expected_fs) {
    char command[256];
    snprintf(command, sizeof(command), "blkid -s TYPE -o vlaue %s 2>/dev/null", device);
    return general__check_command(command, expected_fs);
}

// Check if 'device' is mounted
bool general__mounted(const char *device, const char *mount_point) {
    FILE *mtab = setmntent("/etc/mtab", "r");
    if (!mtab) return false;

    struct mntent *mnt;
    bool mounted = false;

    while ((mnt = getmntent(mtab)) != NULL) {
        if ((device && strcmp(mnt->mnt_fsname, device) == 0) || 
                (mount_point && strcmp(mnt->mnt_dir, mount_point) == 0)) {
            mounted = true;
            break;
        }
    }

    endmntent(mtab);
    return mounted;
}

// Check if swap is active
bool general__swap_active(const char *device) {
    return general__check_command("swapon --show=name --noheadings 2>/dev/null", device); 
}

// Check if there is entry in /etc/fstab
bool general__check_fstab_entry(const char *device, const char *mount_point, const char *fs_type) {
    FILE *fstab = fopen("/etc/fstab", "r");
    if (!fstab) return false;

    char line[512];
    bool found = false;

    while (fgets(line, sizeof(line), fstab)) {
        if (line[0] == '#' || line[0] == '\n') continue;

        char dev[256], mp[256], type[256], options[256], dump[256], pass[256];

        if (sscanf(line, "%255s %255s %255s %255s %255s %255s", 
                    dev, mp, type, options, dump, pass) >= 3) {
            if ((strcmp(dev, device) == 0) || (mount_point && strcmp(mp, mount_point) == 0)) {
                if (strcmp(type, fs_type) == 0) {
                    found = true;
                    break;
                }
            }
        }
    }
    
    fclose(fstab);
    return found;
}


// ========================================================
//     _            _                             _ 
//    | |_ _____  _| |_ _ __  _ __ ___   ___     / |
//    | __/ _ \ \/ / __| '_ \| '__/ _ \ / __|____| |
//    | ||  __/>  <| |_| |_) | | | (_) | (_|_____| |
//     \__\___/_/\_\\__| .__/|_|  \___/ \___|    |_|
//                     |_|                          
// ========================================================

#define textproc_1__MAX_LINE_LENGTH 1000
#define textproc_1__MAX_ENTRIES 1000

#ifndef textproc_1__PATH
#define textproc_1__PATH HOME "task1.4.1/"
#endif // textproc_1__PATH

typedef struct {
    int number;      
    char time[9];    
    char module[32]; 
    char message[955];
} textproc_1__Log_Entry;

// Parse and process log file
int textproc_1__parse_log(const char *filename, textproc_1__Log_Entry entries[]) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    char line[textproc_1__MAX_LINE_LENGTH];
    int count = 0;

    while (fgets(line, sizeof(line), f) && count < textproc_1__MAX_ENTRIES) {
        if (!strstr(line, "[WARN]") && !strstr(line, "[ERROR]")) continue;

        char level[8], module[32], time[9], message[955];

        if (sscanf(line, "[%7[^]]] [%31[^]]] [%8[^]]] %954[^\n]",
                  level, module, time, message) == 4) {

            // Numbers are set only after sorting
            strncpy(entries[count].time, time, 8);
            entries[count].time[8] = '\0';

            // Delete brackets from module name
            if (module[0] == '[') memmove(module, module+1, strlen(module));
            if (module[strlen(module)-1] == ']') module[strlen(module)-1] = '\0';
            strncpy(entries[count].module, module, 31);
            entries[count].module[31] = '\0';

            general__trim(message);
            strncpy(entries[count].message, message, 954);
            entries[count].message[954] = '\0';

            count++;
        }
    }
    fclose(f);
    return count;
}

// Read student file
int textproc_1__read_student(const char *filename, textproc_1__Log_Entry entries[]) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    char line[textproc_1__MAX_LINE_LENGTH];
    int count = 0;

    while (fgets(line, sizeof(line), f) && count < textproc_1__MAX_ENTRIES) {
        general__trim(line);
        if (sscanf(line, "%d %8s %31s %954[^\n]",
                  &entries[count].number,
                  entries[count].time,
                  entries[count].module,
                  entries[count].message) == 4) {
            count++;
        }
    }
    fclose(f);
    return count;
}

// Comparator to sort modules alphabetically
int textproc_1__compare_modules(const void *a, const void *b) {
    const textproc_1__Log_Entry *entry_a = (const textproc_1__Log_Entry *)a;
    const textproc_1__Log_Entry *entry_b = (const textproc_1__Log_Entry *)b;
    return strcmp(entry_a->module, entry_b->module);
}

// Sort and number entries
void textproc_1__sort_and_number(textproc_1__Log_Entry entries[], int count) {
    qsort(entries, count, sizeof(textproc_1__Log_Entry), textproc_1__compare_modules);
    
    for (int i = 0; i < count; ++i) {
        entries[i].number = i + 1;
    }
}

// Verify student file by comparing entries with expected ones
bool textproc_1__verify(const textproc_1__Log_Entry expected[], 
        const textproc_1__Log_Entry actual[], int count) {
    for (int i = 0; i < count; i++) {
        if (expected[i].number != actual[i].number ||
            strcmp(expected[i].time, actual[i].time) != 0 ||
            strcmp(expected[i].module, actual[i].module) != 0 ||
            strcmp(expected[i].message, actual[i].message) != 0) {
            return false;
        }
    }
    return true;
}


bool textproc_1__main(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);

    // TODO: Implement data corruption checks
    textproc_1__Log_Entry expected[textproc_1__MAX_ENTRIES], student[textproc_1__MAX_ENTRIES];
    
    // Parse initial log file
    int expected_count = textproc_1__parse_log(textproc_1__PATH"black_archive.log", expected);
    if (expected_count < 0) {
        lchk_add_feedback(result, "Fail: Error reading 'black_archive.log'");
        goto textproc_1__fail;
    }

    // Make required transformations to logs 
    textproc_1__sort_and_number(expected, expected_count);

    // Read student file
    int student_count = textproc_1__read_student(textproc_1__PATH"filtered_logs.txt", student);
    if (student_count < 0) {
        lchk_add_feedback(result, "Fail: Error reading 'filtered_logs.txt'");
        goto textproc_1__fail;
    }

    // Verify student file
    if (expected_count != student_count) {
        lchk_add_feedback(result, "Fail: Line count mismatch");
        goto textproc_1__fail;
    }

    if (!textproc_1__verify(expected, student, expected_count)) {
        lchk_add_feedback(result, "Fail: Mismatch in log entries");
        goto textproc_1__fail;
    }

    lchk_add_feedback(result, "Pass: All tests ok");
    lchk_set_grade(result, 100);
    return true;

textproc_1__fail:
    lchk_set_grade(result, 0);
    return false;
}

LCHK_REGISTER_MODULE(textproc_1, "textproc-1", "Linux/textproc-1 (task1.4.1)", "1.0", textproc_1__main)


// ========================================================
//     _            _                            ____  
//    | |_ _____  _| |_ _ __  _ __ ___   ___    |___  | 
//    | __/ _ \ \/ / __| '_ \| '__/ _ \ / __|____ __) |
//    | ||  __/>  <| |_| |_) | | | (_) | (_|_____/ __/ 
//     \__\___/_/\_\\__| .__/|_|  \___/ \___|   |_____|
//                     |_|                             
// ========================================================

#define textproc_2__MAX_ENTRIES 1000
#define textproc_2__MAX_LINE_LENGTH 1024
#define textproc_2__MAX_OUTPUT_SIZE 16384

#ifndef textproc_2__PATH
#define textproc_2__PATH HOME "task1.4.2/"
#endif //textproc_2__PATH

typedef struct {
    char id[50];
    char status[20];
    int priority;
} textproc_2__Entry;

typedef struct {
    char id[50];
    int priority;
} textproc_2__Table_Row;

// Basically it's just trimming whitespaces
void textproc_2__normalize_string(char *str) {
    char *dst = str;
    for (char *src = str; *src; src++) {
        if (!isspace(*src)) {
            *dst++ = *src;
        }
    }
    *dst = '\0';
}

// Determine if 'line' is a markdown table divider
bool textproc_2__is_table_divider(const char *line) {
    char normalized[textproc_2__MAX_LINE_LENGTH];
    strncpy(normalized, line, sizeof(normalized)-1);
    textproc_2__normalize_string(normalized);

    if (normalized[0] != '|' || normalized[strlen(normalized) - 1] != '|') {
        return false;
    }
    // This is ruuuude... But all in compliance with md format :)
    for (int i = 1; i < 4; ++i) {
        if (normalized[i] != '-') {
            return false;
        }
    }
    return true;
}

// Parse markdown table row 
bool textproc_2__parse_table_row1(const char *line, textproc_2__Table_Row *row) {
    char normalized[textproc_2__MAX_LINE_LENGTH];
    strncpy(normalized, line, sizeof(normalized)-1);
    textproc_2__normalize_string(normalized);

    char *id_start = strchr(normalized, '|');
    if (!id_start) return false;
    id_start++;

    char *id_end = strchr(normalized, '|');
    if (!id_end) return false;

    char *priority_start = id_end + 1;
    char *priority_end = strchr(priority_start, '|');
    if (!priority_end) priority_end = normalized + strlen(normalized);

    size_t id_len = id_end - id_start;
    if (id_len >= sizeof(row->id)) id_len = sizeof(row->id) - 1;
    strncpy(row->id, id_start, id_len);
    row->id[id_len] = '\0';

    char priority_str[20];
    size_t priority_len = priority_end - priority_start;
    if (priority_len >= sizeof(priority_str)) priority_len = sizeof(priority_str) - 1;
    strncpy(priority_str, priority_start, priority_len);
    priority_str[priority_len] = '\0';

    row->priority = atoi(priority_str);
    return true;
}

// Does not pretty much differ from '..._row1' idk
bool textproc_2__parse_table_row2(const char *line, textproc_2__Table_Row *row) {
    char normalized[textproc_2__MAX_LINE_LENGTH];
    strncpy(normalized, line, sizeof(normalized)-1);
    textproc_2__normalize_string(normalized);

    if (normalized[0] != '|' || strlen(normalized) < 2) {
        return false;
    }

    // Search for 2nd '|'
    char *second_pipe = strchr(normalized + 1, '|');
    if (!second_pipe) {
        return false;
    }

    // Select ID (betw. 1st and 2nd '|')
    size_t id_len = second_pipe - (normalized + 1);
    if (id_len >= sizeof(row->id)) id_len = sizeof(row->id) - 1;
    strncpy(row->id, normalized + 1, id_len);
    row->id[id_len] = '\0';

    // Search for 3rd '|'
    char *third_pipe = strchr(second_pipe + 1, '|');
    if (!third_pipe) {
        third_pipe = normalized + strlen(normalized);
    }

    // Select priority (betw. 2nd и 3rd '|')
    char priority_str[20];
    size_t priority_len = third_pipe - (second_pipe + 1);
    if (priority_len >= sizeof(priority_str)) priority_len = sizeof(priority_str) - 1;
    strncpy(priority_str, second_pipe + 1, priority_len);
    priority_str[priority_len] = '\0';

    row->priority = atoi(priority_str);
    return true;
}

// Based algorithm to compare contents of markdown file
bool textproc_2__compare_markdown_files(const char *expected_file, const char *student_file) {
    FILE *expected = fopen(expected_file, "r");
    FILE *student = fopen(student_file, "r");

    if (!expected || !student) {
        if (expected) fclose(expected);
        if (student) fclose(student);
        return false;
    }

    char exp_line[textproc_2__MAX_LINE_LENGTH];
    char stu_line[textproc_2__MAX_LINE_LENGTH];
    int line_num = 0;
    bool equal = true;

    while (fgets(exp_line, sizeof(exp_line), expected) && fgets(stu_line, sizeof(stu_line), student)) {
        line_num++;

        if (textproc_2__is_table_divider(exp_line) && textproc_2__is_table_divider(stu_line)) {
            continue;
        }

        textproc_2__Table_Row exp_row, stu_row;
        bool exp_valid = textproc_2__parse_table_row2(exp_line, &exp_row);
        bool stu_valid = textproc_2__parse_table_row2(stu_line, &stu_row);

        if (exp_valid != stu_valid) {
            equal = false;
            break;
        }

        if (exp_valid && stu_valid) {
            if (strcmp(exp_row.id, stu_row.id) != 0 || exp_row.priority != stu_row.priority) {
                equal = false;
                break;
            }       
        }
    }

    if (equal) {
        // Try to read one more line...
        bool more_in_expected = fgets(exp_line, sizeof(exp_line), expected) != NULL;
        bool more_in_student = fgets(stu_line, sizeof(stu_line), student) != NULL;
        if (more_in_expected != more_in_student) {
            equal = false;
        }
    }
  
    fclose(expected);
    fclose(student);
    return equal;
}

// Simple json parser for the needs of this task
// Note that providing broken file may lead to undefined consequences...
bool textproc_2__parse_json_file(const char *filename, textproc_2__Entry *entries, int *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return false;
    }

    char line[textproc_2__MAX_LINE_LENGTH];
    bool in_entry = false;
    textproc_2__Entry current = {0};
    int braces_level = 0;

    while (fgets(line, sizeof(line), file)) {
        char *pos = line;
        
        while (*pos && isspace(*pos)) pos++;
        if (!*pos) continue;

        if (strstr(pos, "{")) {
            if (!in_entry) {
                memset(&current, 0, sizeof(current));
                in_entry = true;
            }
            braces_level++;
            continue;
        }

        if (strstr(pos, "}")) {
            braces_level--;
            if (braces_level == 0 && in_entry) {
                if (*count < textproc_2__MAX_ENTRIES) {
                    entries[(*count)++] = current;
                }
                in_entry = false;
            }
            continue;
        }

        if (in_entry) {
            char *colon = strchr(pos, ':');
            if (colon) {
                *colon = '\0';
                char *key = pos;
                char *value = colon + 1;
                
                // Trim key
                while (*key && isspace(*key)) key++;
                if (*key == '"') key++;
                char *end = key + strlen(key) - 1;
                while (end > key && (*end == '"' || isspace(*end))) end--;
                *(end + 1) = '\0';

                // Trim value
                while (*value && isspace(*value)) value++;
                if (*value == '"') value++;
                end = value + strlen(value) - 1;
                while (end > value && (*end == '"' || *end == ',' || isspace(*end))) end--;
                *(end + 1) = '\0';

                if (strcmp(key, "id") == 0) {
                    strncpy(current.id, value, sizeof(current.id)-1);
                } else if (strcmp(key, "status") == 0) {
                    strncpy(current.status, value, sizeof(current.status)-1);
                } else if (strcmp(key, "priority") == 0) {
                    // Quite naive approach but works for now
                    current.priority = atoi(value);
                }
            }
        }
    }

    fclose(file);
    return true;
}

// Comparator to sort entries by priority
int textproc_2__compare_entries(const void *a, const void *b) {
    const textproc_2__Entry *entry_a = (const textproc_2__Entry*)a;
    const textproc_2__Entry *entry_b = (const textproc_2__Entry*)b;
    return entry_b->priority - entry_a->priority;
}

// Generate expected ouput in text and mardown files
void textproc_2__generate_expected_output(textproc_2__Entry *entries, int count, 
        char *txt_output, char *md_output) {
    // Filter only active entries
    textproc_2__Entry active_entries[textproc_2__MAX_ENTRIES];
    int active_count = 0;
    
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].status, "active") == 0) {
            active_entries[active_count++] = entries[i];
        }
    }
    
    // Sort by priority
    qsort(active_entries, active_count, sizeof(textproc_2__Entry), textproc_2__compare_entries);
    
    // Generate text ouput
    char *txt_ptr = txt_output;
    for (int i = 0; i < active_count; i++) {
        txt_ptr += snprintf(txt_ptr, textproc_2__MAX_OUTPUT_SIZE - (txt_ptr - txt_output),
                        "%-8s | %d\n", active_entries[i].id, active_entries[i].priority);
    }
    
    // Generate narkdown output
    char *md_ptr = md_output;
    md_ptr += snprintf(md_ptr, textproc_2__MAX_OUTPUT_SIZE - (md_ptr - md_output),
                     "| ID      | PRIORITY |\n|---------|----------|\n");
    
    for (int i = 0; i < active_count; i++) {
        md_ptr += snprintf(md_ptr, textproc_2__MAX_OUTPUT_SIZE - (md_ptr - md_output),
                         "| %-7s | %-8d |\n", active_entries[i].id, active_entries[i].priority);
    }
}

// Compare text files
bool textproc_2__compare_files(const char *file1, const char *file2) {
    FILE *f1 = fopen(file1, "r");
    FILE *f2 = fopen(file2, "r");
    
    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return false;
    }
    
    bool equal = true;
    char ch1, ch2;
    
    do {
        ch1 = fgetc(f1);
        ch2 = fgetc(f2);
        
        while (isspace(ch1)) ch1 = fgetc(f1);
        while (isspace(ch2)) ch2 = fgetc(f2);
        
        if (ch1 != ch2) {
            equal = false;
            break;
        }
    } while (ch1 != EOF && ch2 != EOF);
    
    fclose(f1);
    fclose(f2);
    
    return equal && (ch1 == EOF && ch2 == EOF);
}

// Print difference between markdown files in absolutely crushing way!
// As its name suggests, this function should never be called in release version 
void DEBUG__textproc_2__print_diff(const char *expected_file, const char *student_file) {
    FILE *expected = fopen(expected_file, "r");
    FILE *student = fopen(student_file, "r");
    
    if (!expected || !student) {
        if (expected) fclose(expected);
        if (student) fclose(student);
        return;
    }

    char exp_line[textproc_2__MAX_LINE_LENGTH];
    char stu_line[textproc_2__MAX_LINE_LENGTH];
    int line_num = 0;
    
    printf("\n%-6s | %-30s | %-30s\n", "Line", "Expected", "Student");
    printf("------|--------------------------------|--------------------------------\n");
    
    while (fgets(exp_line, sizeof(exp_line), expected) && 
           fgets(stu_line, sizeof(stu_line), student)) {
        line_num++;
        
        bool is_divider = textproc_2__is_table_divider(exp_line) && textproc_2__is_table_divider(stu_line);
        
        char exp_display[textproc_2__MAX_LINE_LENGTH];
        char stu_display[textproc_2__MAX_LINE_LENGTH];
        
        if (is_divider) {
            snprintf(exp_display, sizeof(exp_display), "[TABLE DIVIDER]");
            snprintf(stu_display, sizeof(stu_display), "[TABLE DIVIDER]");
        } else {
            textproc_2__Table_Row exp_row, stu_row;
            bool exp_valid = textproc_2__parse_table_row2(exp_line, &exp_row);
            bool stu_valid = textproc_2__parse_table_row2(stu_line, &stu_row);
            
            if (exp_valid && stu_valid) {
                snprintf(exp_display, sizeof(exp_display), "|%s|%d|", exp_row.id, exp_row.priority);
                snprintf(stu_display, sizeof(stu_display), "|%s|%d|", stu_row.id, stu_row.priority);
            } else {
                strncpy(exp_display, exp_line, sizeof(exp_display));
                strncpy(stu_display, stu_line, sizeof(stu_display));
                textproc_2__normalize_string(exp_display);
                textproc_2__normalize_string(stu_display);
            }
        }
        
        printf("%-6d | %-30s | %-30s", line_num, exp_display, stu_display);
        
        if (is_divider) {
            printf("  (ignored)");
        } else if (strcmp(exp_display, stu_display) != 0) {
            printf("  <<< DIFFERENCE");
        }
        printf("\n");
    }
    
    fclose(expected);
    fclose(student);

} 


bool textproc_2__main(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);
    // TODO: Implement data corruption checks
    textproc_2__Entry entries[textproc_2__MAX_ENTRIES] = {0};
    int count = 0;
    
    // Parse input file
    if (!textproc_2__parse_json_file(textproc_2__PATH"manifest.json", entries, &count)) {
        lchk_add_feedback(result, "Fail: Error reading 'manifest.json'");
        lchk_set_grade(result, 0);
        return false;
    }
    
    // Generate expected output
    char expected_txt[textproc_2__MAX_OUTPUT_SIZE] = {0};
    char expected_md[textproc_2__MAX_OUTPUT_SIZE] = {0};
    textproc_2__generate_expected_output(entries, count, expected_txt, expected_md);
    
    // Save temp files
    FILE *txt_file = fopen(textproc_2__PATH".expected_result.txt", "w");
    FILE *md_file = fopen(textproc_2__PATH".expected_result.md", "w");
    
    if (txt_file) {
        fputs(expected_txt, txt_file);
        fclose(txt_file);
    }

    if (md_file) {
        fputs(expected_md, md_file);
        fclose(md_file);
    }
    
    // Compare with student files
    bool txt_match = textproc_2__compare_files(textproc_2__PATH".expected_result.txt", textproc_2__PATH"result.txt");
    bool md_match = textproc_2__compare_markdown_files(textproc_2__PATH".expected_result.md", textproc_2__PATH"result.md");
    
    if (!txt_match) {
        lchk_add_feedback(result, "Fail: .txt file contains errors");
        //system("diff -u " textproc_2__PATH".expected_result.txt " textproc_2__PATH"result.txt");
    }
    
    if (!md_match) {
        lchk_add_feedback(result, "Fail: .md file contains errors");
        //DEBUG__textproc_2__print_diff(textproc_2__PATH".expected_result.md", textproc_2__PATH"result.md");
    }
    
    // Delete temp files
    unlink(textproc_2__PATH".expected_result.txt");
    unlink(textproc_2__PATH".expected_result.md");

    // Grade submission
    if (lchk_feedback_empty(result)) {
        lchk_add_feedback(result, "Pass: All tests ok");
        lchk_set_grade(result, 100); 
        return true;       
    } else {
        lchk_set_grade(result, 0);
        return false;
    }
  
    return 0;
}

LCHK_REGISTER_MODULE(textproc_2, "textproc-2", "Linux/textproc-2 (task1.4.2)", "1.0", textproc_2__main)


// ========================================================
//         _                                   _ 
//     ___| |_ ___  _ __ __ _  __ _  ___      / |
//    / __| __/ _ \| '__/ _` |/ _` |/ _ \_____| |
//    \__ \ || (_) | | | (_| | (_| |  __/_____| |
//    |___/\__\___/|_|  \__,_|\__, |\___|     |_|
//                            |___/              
// ========================================================

#define storage_1__BOOT_PARTITION   "/dev/sdb1"
#define storage_1__SWAP_PARTITION   "/dev/sdb2"
#define storage_1__ROOT_PARTITION   "/dev/sdb3"
#define storage_1__BOOT_MOUNT_POINT "mnt/kangtao/boot"
#define storage_1__ROOT_MOUNT_POINT "mnt/kangtao/"
#define storage_1__BOOT_SIZE_MB     500
#define storage_1__SWAP_SIZE_MB     1024
#define storage_1__BOOT_FS_TYPE     "ext2"
#define storage_1__ROOT_FS_TYPE     "ext4"
#define storage_1__SWAP_FS_TYPE     "swap"
#define storage_1__NUM_PARTITIONS   3

typedef struct {
    const char *device;
    const char *mount_point;
    const char *fs_type;
    long expected_size_mb;
    bool is_swap;
} storage_1__Partition_Info;

// Partitions to check
static const storage_1__Partition_Info partitions[] = {
    {storage_1__BOOT_PARTITION, storage_1__BOOT_MOUNT_POINT, storage_1__BOOT_FS_TYPE, storage_1__BOOT_SIZE_MB, false},
    {storage_1__SWAP_PARTITION, NULL, storage_1__SWAP_FS_TYPE, storage_1__SWAP_SIZE_MB, true},
    {storage_1__ROOT_PARTITION, storage_1__ROOT_MOUNT_POINT, storage_1__ROOT_FS_TYPE, -1, false}
};


bool storage_1__main(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);

    for (int i = 0; i < storage_1__NUM_PARTITIONS; ++i) {
        const storage_1__Partition_Info *part = &partitions[i];
      
        // Check for device 
        if (!general__file_exists(part->device)) {
            lchk_add_feedback(result, "Fail: Device '%s' does not exist", part->device);
            continue;
        }
        
        // Check for filesystem type
        if (!general__check_filesystem_type(part->device, part->fs_type)) {
            lchk_add_feedback(result, "Fail: Incorrect filesystem type for '%s'", part->device);
        }
        
        // Check for size
        if (part->expected_size_mb > 0) {
            long actual_size_kb = general__get_partition_size_kb(part->device);
            long expected_size_kb = part->expected_size_mb * 1024;

            if (actual_size_kb > 0) {
                // Assume 10% tolerance is fine
                long tolerance = expected_size_kb * 0.1;
                if (labs(actual_size_kb - expected_size_kb) > tolerance) {
                    lchk_add_feedback(result, "Fail: Size of '%s' is different from expected one", part->device);
                }
            }
        }

        // Check mount / swap
        if (part->is_swap) {
            if (!general__swap_active(part->device)) {
                lchk_add_feedback(result, "Fail: Swap partition '%s' is not active", part->device);
            }
        } else if (part->mount_point) {
            if (!general__mounted(part->device, part->mount_point)) {
                lchk_add_feedback(result, "Fail: Partition '%s' is not mounted to '%s'", part->device, part->mount_point);
            }
            // Additional check of mount point
            if (!general__file_exists(part->mount_point)) {
                lchk_add_feedback(result, "Fail: Mount point '%s' does not exist", part->mount_point);
            }
        }

        // Check fstab entry
        if (!general__check_fstab_entry(part->device, part->mount_point, part->fs_type)) {
            lchk_add_feedback(result, "Fail: /etc/fstab is not configured properly");
        }
    }

    // Grade submission
    if (lchk_feedback_empty(result)) {
        lchk_add_feedback(result, "Pass: All tests ok");
        lchk_set_grade(result, 100); 
        return true;
    } else {
        lchk_set_grade(result, 0);
        return false;
    }
}

LCHK_REGISTER_MODULE(storage_1, "storage-1", "Linux/storage-1 (task1.5.1)", "1.0", storage_1__main)


// ========================================================
//         _                                  ____  
//     ___| |_ ___  _ __ __ _  __ _  ___     |___  | 
//    / __| __/ _ \| '__/ _` |/ _` |/ _ \_____ __) |
//    \__ \ || (_) | | | (_| | (_| |  __/_____/ __/ 
//    |___/\__\___/|_|  \__,_|\__, |\___|    |_____|
//                            |___/                 
// ========================================================

#define storage_2__PV1_DEVICE           "/dev/sdc"
#define storage_2__PV2_DEVICE           "/dev/sdd"
#define storage_2__VG_NAME              "biochrome_vg"
#define storage_2__DATA_LV              "/dev/biochrome_vg/data_lv"
#define storage_2__BACKUP_LV            "/dev/biochrome_vg/backup_lv"
#define storage_2__DATA_MOUNT_POINT     "/opt/data"
#define storage_2__BACKUP_MOUNT_POINT   "/mnt/backup"
#define storage_2__DATA_FS_TYPE         "xfs"
#define storage_2__BACKUP_FS_TYPE       "ext4"
#define storage_2__DATA_SIZE_MB         1024
#define storage_2__BACKUP_SIZE_MB       500
#define storage_2__NUM_LVS              2
#define storage_2__NUM_PVS              2

typedef struct {
    const char *lv_device;
    const char *mount_point;
    const char *fs_type;
    long expected_size_mb;
} storage_2__Lv_Info;

// Logic volumes
static const storage_2__Lv_Info logical_volumes[] = {
    {storage_2__DATA_LV, storage_2__DATA_MOUNT_POINT, storage_2__DATA_FS_TYPE, storage_2__DATA_SIZE_MB},
    {storage_2__BACKUP_LV, storage_2__BACKUP_MOUNT_POINT, storage_2__BACKUP_FS_TYPE, storage_2__BACKUP_SIZE_MB}
};

// Physical volumes
static const char *physical_volumes[] = {
    storage_2__PV1_DEVICE,
    storage_2__PV2_DEVICE
};

// Check if 'pv_device' is actually a physical volume
bool storage_2__check_physical_volume(const char *pv_device) {
    if (!general__file_exists(pv_device)) return false;

    char command[256];
    snprintf(command, sizeof(command), "pvs --noheadings -o pv_name %s 2>/dev/null", pv_device);
    return general__check_command(command, pv_device);
}

// Check if 'vg_name' volume group exists
bool storage_2__check_volume_group(const char *vg_name) {
    char command[256];
    snprintf(command, sizeof(command), "vgs --noheadings -o vg_name %s 2>/dev/null", vg_name);
    if (!general__check_command(command, vg_name)) return false;

    // Check if PVS are in 'vg_name'
    for (int i = 0; i < storage_2__NUM_PVS; ++i) {
        snprintf(command, sizeof(command), "vgs --noheadings -o vg_name %s 2>/dev/null", physical_volumes[i]);
        if (!general__check_command(command, vg_name)) return false;
    }

    return true;
}

// Check if logical volume exists and is correct
bool storage_2__check_logical_volume(const storage_2__Lv_Info *lv) {
    // Basic thingy
    if (!general__file_exists(lv->lv_device)) return false;
    if (!general__check_filesystem_type(lv->lv_device, lv->fs_type)) return false;

    // Size thingy
    long actual_size_kb = general__get_partition_size_kb(lv->lv_device);
    long expected_size_kb = lv->expected_size_mb * 1024;

    if (actual_size_kb > 0) {
        long tolerance = expected_size_kb * 0.1;
        if (labs(actual_size_kb - expected_size_kb) > tolerance) {
            return false;
        }
    }
    
    // Mount thingy
    if (!general__mounted(lv->lv_device, lv->mount_point)) return false;
    if (!general__file_exists(lv->mount_point)) return false;

    // Fstab thingy
    if (!general__check_fstab_entry(lv->lv_device, lv->mount_point, lv->fs_type)) return false;

    return true;
}


bool storage_2__main(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);
    
    // Check if PVs are valid
    for (int i = 0; i < storage_2__NUM_PVS; ++i) {
        if (!storage_2__check_physical_volume(physical_volumes[i])) {
            lchk_add_feedback(result, "Fail: PV '%s' does not exist or is not a valid physical volume", physical_volumes[i]);
        }
    }

    // Check if VG exists and is correct
    if (!storage_2__check_volume_group(storage_2__VG_NAME)) {
        lchk_add_feedback(result, "Fail: VG '" storage_2__VG_NAME "' does not exist or not all PVs are present in VG");
    }
    
    // Check is LVs are valid
    for (int i = 0; i < storage_2__NUM_LVS; ++i) {
        if (!storage_2__check_logical_volume(&logical_volumes[i])) {
            lchk_add_feedback(result, "Fail: LV '%s' does not exist, is of wrong size/type or not mounted", logical_volumes[i].lv_device);
        }
    }

    // Grade submission
    if (lchk_feedback_empty(result)) {
        lchk_add_feedback(result, "Pass: All tests ok");
        lchk_set_grade(result, 100); 
        return true;
    } else {
        lchk_set_grade(result, 0);
        return false;
    }
 
}

LCHK_REGISTER_MODULE(storage_2, "storage-2", "Linux/storage-2 (task1.5.2)", "1.0", storage_2__main)


// ========================================================
//         _                                  _____
//     ___| |_ ___  _ __ __ _  __ _  ___     |___ /
//    / __| __/ _ \| '__/ _` |/ _` |/ _ \_____ |_ |
//    \__ \ || (_) | | | (_| | (_| |  __/_____|__) |
//    |___/\__\___/|_|  \__,_|\__, |\___|    |____/
//                            |___/
// ========================================================

bool storage_3__main(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config); UNUSED(result);

    return true;
}

LCHK_REGISTER_MODULE(storage_3, "storage-3", "Linux/storage-3 (task1.5.3)", "1.0", storage_3__main)


// ========================================================
//                                             _
//     _ __  _ __ ___   ___ ___  ___ ___      / |
//    | '_ \| '__/ _ \ / __/ _ \/ __/ __|_____| |
//    | |_) | | | (_) | (_|  __/\__ \__ \_____| |
//    | .__/|_|  \___/ \___\___||___/___/     |_|
//    |_|                                        
// ========================================================

bool process_1__main(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config); UNUSED(result);

    return true;
}

LCHK_REGISTER_MODULE(process_1, "process-1", "Linux/process-1 (task1.6.1)", "1.0", process_1__main)


// ========================================================
//                      _                          _ 
//     _ __   __ _  ___| | ____ _  __ _  ___      / |
//    | '_ \ / _` |/ __| |/ / _` |/ _` |/ _ \_____| |
//    | |_) | (_| | (__|   < (_| | (_| |  __/_____| |
//    | .__/ \__,_|\___|_|\_\__,_|\__, |\___|     |_|
//    |_|                         |___/              
// ========================================================

#define package_1__PKG_LIST_PATH "/var/log/pkg-list.log"
#define package_1__REPOS_PATH "/var/log/repos.log"

// Check if 'package' is installed
bool package_1__package_installed(const char *package_name) {
    char command[256];
    snprintf(command, sizeof(command), "dpkg -l | grep \"^ii\" | grep \" %s \"", package_name);
    return general__check_command(command, package_name);
}

// Check APT configuration
bool package_1__check_apt_security() {
    // Check if there are files with security config
    if (general__check_command("grep -r \"AllowUnauthenticated.*false\" /etc/apt/apt.conf.d/", "false") ||
            general__check_command("grep -r \"Allow-Insecure.*false\" /etc/apt/apt.conf.d/", "false")) {
        return true;
    }

    // Check if there are no allow unauthenticated configs
    if (general__check_command("grep -r \"AllowUnauthenticated.*true\" /etc/apt/apt.conf.d/", "true") ||
            general__check_command("grep -r \"Allow-Insecure.*true\" /etc/apt/apt.conf.d/", "true")) {
        return false;
    }

    // By default, installation fron unsigned repos is disabled in Ubuntu
    // But for sanity-check it's better to check current behaviour
    FILE *f = popen("apt-config dump | grep -A1 \"AllowUnauthenticated\" | grep -q \"false\" && echo \"secure\"", "r");
    if (f) {
        char buffer[general__CMD_BUFFER_SIZE];
        if (fgets(buffer, sizeof(buffer), f) && strstr(buffer, "secure")) {
            pclose(f);
            return true;
        }
        pclose(f);
    }

    return false;
}


bool package_1__main(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);
    
    // Check recent system updates
    if (!general__check_command("find /var/lib/apt/lists/ -name \"*Packages*\" -mtime -1", "Packages")) {
        lchk_add_feedback(result, "Fail: Package lists are outdated");
    }

    // Check for installed unitilies
    const char *packages[] = {"curl", "ufw", "fail2ban"};
    for (int i = 0; i < 3; ++i) {
        if (!package_1__package_installed(packages[i])) {
            lchk_add_feedback(result, "Fail: Package '%s' is not installed", packages[i]);
        }
    }

    // Check pkg-list file 
    if (!general__file_exists(package_1__PKG_LIST_PATH)) {
        lchk_add_feedback(result, "Fail: File " package_1__PKG_LIST_PATH " does not exist");
        lchk_set_grade(result, 0);
        return false;
    }

    // Check pkg-list contents
    if (!general__check_file_content(package_1__PKG_LIST_PATH, "ii ") && 
            !general__check_file_content(package_1__PKG_LIST_PATH, "ubuntu")) {
        lchk_add_feedback(result, "Fail: File " package_1__PKG_LIST_PATH " does not contain package list");
    }

    // Check telnet is uninstalled
    if (package_1__package_installed("telnet")) {
        lchk_add_feedback(result, "Fail: Package telnet is installed");
    }

    // Check APT config
    if (!package_1__check_apt_security()) {
        lchk_add_feedback(result, "Fail: APT is not configured");
    }

    // Check repos file
    if (!general__file_exists(package_1__REPOS_PATH)) {
        lchk_add_feedback(result, "Fail: File '" package_1__REPOS_PATH "' does not exist");
        lchk_set_grade(result, 0);
        return false;
    }

    // Check repos contents
    if (!general__check_file_content(package_1__REPOS_PATH, "http") && 
            !general__check_file_content(package_1__REPOS_PATH, "ubuntu.com") &&
            !general__check_file_content(package_1__REPOS_PATH, "deb ")) {
        lchk_add_feedback(result, "Fail: File '" package_1__REPOS_PATH "' does not contain repo info");
    }

    // Grade submission
    if (lchk_feedback_empty(result)) {
        lchk_add_feedback(result, "Pass: All tests ok");
        lchk_set_grade(result, 100); 
        return true;
    } else {
        lchk_set_grade(result, 0);
        return false;
    }
}

LCHK_REGISTER_MODULE(package_1, "package-1", "Linux/package-1 (task1.6.1)", "1.0", package_1__main)


// ========================================================
//                   _                     _       _ 
//     ___ _   _ ___| |_ ___ _ __ ___   __| |     / |
//    / __| | | / __| __/ _ \ '_ ` _ \ / _` |_____| |
//    \__ \ |_| \__ \ ||  __/ | | | | | (_| |_____| |
//    |___/\__, |___/\__\___|_| |_| |_|\__,_|     |_|
//         |___/
// ========================================================

#define systemd_1__MARKER_FILE "/var/cowgreet.done"
#define systemd_1__SERVICE_NAME "cowgreet.service"
#define systemd_1__SCRIPT_PATH "/usr/local/bin/cowgreet.sh"
#define systemd_1__HOW_MUCH_LAGGY 10
// TODO: It's different on Ubuntu! Make sure to compile with -Dsystemd_1__COWSAY_PATH
// #define systemd_1__COWSAY_PATH "/usr/games/cowsay"
#ifndef systemd_1__COWSAY_PATH
#define systemd_1__COWSAY_PATH "/usr/bin/cowsay"
#endif //systemd_1__COWSAY_PATH

// Temporarily disable cowsay by renaming executable
bool systemd_1__disable_cowsay() {
    // Rename cowsay to prevent making shit in terminals
    if (access(systemd_1__COWSAY_PATH, F_OK) == 0) {
        rename(systemd_1__COWSAY_PATH, systemd_1__COWSAY_PATH".bak");
        
        // Make a dummy executable
        FILE *fake = fopen(systemd_1__COWSAY_PATH, "w");
        if (!fake) return false;
        fprintf(fake, "\n");
        fclose(fake);
        chmod(systemd_1__COWSAY_PATH, 0755);
        return true;
    }
    return false;
}

// Restore cowsay
void systemd_1__restore_cowsay() {
    if (access(systemd_1__COWSAY_PATH".bak", F_OK) == 0) {
        remove(systemd_1__COWSAY_PATH);
        rename(systemd_1__COWSAY_PATH".bak", systemd_1__COWSAY_PATH);
    }
}

// Wait until new marker is created (or timeout is reached)
bool systemd_1__wait_for_marker(int timeout_seconds) {
    time_t start = time(NULL);
    while (time(NULL) - start < timeout_seconds) {
        if (access(systemd_1__MARKER_FILE, F_OK) == 0) {
            return true; // File created
        }
        usleep(100000); // 100 ms delay
    }
    return false; // Timeout
}

// Can be skipped probably
bool systemd_1__is_marker_fresh(time_t restart_time) {
    struct stat st;
    if (stat(systemd_1__MARKER_FILE, &st) != 0) return false;
    return st.st_mtime >= restart_time;
}

// Check if script file contains 'cowsay'
bool systemd_1__script_contains_cowsay(FILE *script) {
    char line[512];
    bool found = false;
    while (fgets(line, sizeof(line), script)) {
        if (strstr(line, "cowsay")) {
            found = true;
        }
    }
    return found;
}


bool systemd_1__main(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);
    // Save time before restarting service
    time_t before_restart = time(NULL);

    // Remove existing marker
    remove(systemd_1__MARKER_FILE);

    // Check if student script exists
    FILE *script = fopen(systemd_1__SCRIPT_PATH, "r");
    if (!script) {
        lchk_add_feedback(result, "Fail: Script '%s' does not exist", systemd_1__SCRIPT_PATH);
        lchk_set_grade(result, 0);
        return false;
    }
    
    // Check if script file contains 'cowsay'
    if (!systemd_1__script_contains_cowsay(script)) {
        lchk_add_feedback(result, "Fail: Script '%s' does not match the given task", systemd_1__SCRIPT_PATH);
        lchk_set_grade(result, 0);
        return false;
    }
    fclose(script);

    // Temporarily disable cowsay to prevent a mess
    if (!systemd_1__disable_cowsay()) {
        // TODO: Maybe log this? 
        // If we're here it's either a permisson problem or an invalid cowsay path 
        lchk_add_feedback(result, "Fail: Aborted, details are not provided");
        lchk_set_grade(result, 0);
        return false;
    }

    // Restart service
    if (system("systemctl restart " systemd_1__SERVICE_NAME " >/dev/null 2>&1") != 0) {
        lchk_add_feedback(result, "Fail: Could not restart service '%s'", systemd_1__SERVICE_NAME);
    }

    // Wait for marker for 10 s
    if (!systemd_1__wait_for_marker(systemd_1__HOW_MUCH_LAGGY)) {
        lchk_add_feedback(result, "Fail: Marker was not created within %d seconds", systemd_1__HOW_MUCH_LAGGY);
    }

    // Sanity check - marker freshness
    if (!systemd_1__is_marker_fresh(before_restart)) {
        lchk_add_feedback(result, "Fail: Marker is too old");
    }

    // Check if service is enabled
    if (system("systemctl is-enabled " systemd_1__SERVICE_NAME " >/dev/null") != 0) {
        lchk_add_feedback(result, "Fail: Service '%s' is inactive", systemd_1__SERVICE_NAME);
    } 

    // Restore cowsay
    systemd_1__restore_cowsay();

    // Grade submission
    if (lchk_feedback_empty(result)) {
        lchk_add_feedback(result, "Pass: All tests ok");
        lchk_set_grade(result, 100); 
        return true;       
    } else {
        lchk_set_grade(result, 0);
        return false;
    }
}

LCHK_REGISTER_MODULE(systemd_1, "systemd-1", "Linux/systemd-1 (task1.9.1)", "1.0", systemd_1__main)


// ========================================================
//                   _                     _      ____  
//     ___ _   _ ___| |_ ___ _ __ ___   __| |    |___  | 
//    / __| | | / __| __/ _ \ '_ ` _ \ / _` |_____ __) |
//    \__ \ |_| \__ \ ||  __/ | | | | | (_| |_____/ __/ 
//    |___/\__, |___/\__\___|_| |_| |_|\__,_|    |_____|
//         |___/                                        
// ======================================================== 

#ifndef systemd_2__PKG_PATH
#define systemd_2__PKG_PATH HOME "task1.9.2/cerberus-telemetry.deb"
#endif //systemd_2__PKG_PATH

#define systemd_2__SERVICE_PATH "/lib/systemd/system/cerberus.service"
#define systemd_2__SCRIPT_PATH "/usr/bin/cerberus-log.sh"

// Check if cerberus-telemetry package is installed
bool systemd_2__package_installed() {
    return general__run_command("dpkg -l | grep -q '^ii.*cerberus-telemetry'");
}

// Remove installed cerberus-telemetry package
bool systemd_2__remove_package() {
    // Stop service ant then remove package
    general__run_command("sudo systemctl stop cerberus.service 2>/dev/null");
    general__run_command("sudo systemctl disable cerberus.service 2>/dev/null");
    if (!general__run_command("sudo dpkg -r cerberus-telemetry")) {
        return false;
    }
    // Reload systemd after deinstallation
    general__run_command("sudo systemctl daemon-reload");
    return true; // Assume everything's valid
}

bool systemd_2__install_package() {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "sudo dpkg -i "systemd_2__PKG_PATH);
    if (!general__run_command(cmd)) {
        return false;
    }
    sleep(3);
    return true;
}

bool systemd_2__main(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);
    
    // Check if package is installed. If so, remove it
    if (systemd_2__package_installed()) {
        systemd_2__remove_package();
        sleep(2);
    }

    // Install package
    if (!systemd_2__install_package()) {
        lchk_add_feedback(result, "Fail: Unable to install package");
        lchk_set_grade(result, 0);
        return false;
    }

    // Perform file checks
    if (!general__file_exists(systemd_2__SCRIPT_PATH) || 
            !general__file_exists(systemd_2__SERVICE_PATH)) {
        lchk_add_feedback(result, "Fail: Package structure is incorrect");
    }
    if (!general__file_executable(systemd_2__SCRIPT_PATH)) {
        lchk_add_feedback(result, "Fail: Script '/usr/bin/cerberus-log.sh' is not executable");
    }

    // Proform service checks
    if (!general__run_command("systemctl is-active cerberus.service >/dev/null 2>&1") ||
            !general__run_command("systemctl is-enabled cerberus.service >/dev/null 2>&1") ||
            !general__run_command("journalctl -t cerberus --since='1 min ago' | head -5 | grep -q cerberus")) {
        lchk_add_feedback(result, "Fail: Service 'cerberus.service' is misconfigured");
    }

    // Check service file contents
    if (!general__check_file_content(systemd_2__SERVICE_PATH, "WantedBy=multi-user.target") ||
            !general__check_file_content(systemd_2__SERVICE_PATH, "SyslogIdentifier=cerberus")) {
        lchk_add_feedback(result, "Fail: Service 'cerberus.service' does not match the given task");
    }

    // Remove package
    if (!systemd_2__remove_package()) {
        lchk_add_feedback(result, "Fail: Unable to remove package");
    }

    // Verify cleanup
    if (general__file_exists(systemd_2__SCRIPT_PATH) || general__file_exists(systemd_2__SERVICE_PATH)) {
        lchk_add_feedback(result, "Fail: Files stil present after package was removed");
    }
    if (general__run_command("systemctl is-active cerberus.service >/dev/null 2>&1")) {
        lchk_add_feedback(result, "Fail: Service still active after package was removed");
    }
    if (systemd_2__package_installed()) {
        lchk_add_feedback(result, "Fail: Package still installed after it was removed");
    }

    // Grade submission
    if (lchk_feedback_empty(result)) {
        lchk_add_feedback(result, "Pass: All tests ok");
        lchk_set_grade(result, 100); 
        return true;       
    } else {
        lchk_set_grade(result, 0);
        return false;
    }
}

LCHK_REGISTER_MODULE(systemd_2, "systemd-2", "Linux/systemd-2 (task1.9.2)", "1.0", systemd_2__main)


// ====================================
//     _               _           _
//    | |__   __ _ ___| |__       / |
//    | '_ \ / _` / __| '_ \ _____| |
//    | |_) | (_| \__ \ | | |_____| |
//    |_.__/ \__,_|___/_| |_|     |_|
//
// ====================================

#define bash_1__MAX_ENTRIES 20
#define bash_1__MAX_LINE_LEN 256
#define bash_1__PATH HOME "task1.10.2/"

typedef struct {
    char time[25];
    char level[10];
    char module[10];
    char message[100];
} bash_1__Log_Entry;

typedef struct {
    const char *levels[4];
    const char *modules[4];
    const char *messages[6];
} bash_1__Log_Data;

typedef struct {
    bash_1__Log_Entry entries[bash_1__MAX_ENTRIES];
    int count;
} bash_1__Log_Collection;

// Initialize log data
bash_1__Log_Data bash_1__create_log_data() {
    bash_1__Log_Data data = {
        .levels = {"ERROR", "WARN", "INFO", "DEBUG"},
        .modules = {"AUTH", "DB", "NETWORK", "API"},
        .messages = {
            "Failed login", "Connection timeout", "Query executed",
            "Invalid request", "Access denied", "Transaction completed"
        }
    };
    return data;
}

// Generate random log to be written into the test file
bash_1__Log_Collection bash_1__generate_random_log(bash_1__Log_Data *data, int log_count) {
    bash_1__Log_Collection logs;
    logs.count = log_count;
    // Really random stuff going on!
    for (int i = 0; i < log_count; ++i) {
        snprintf(logs.entries[i].time, 25, "2025-%02d-%02d %02d:%02d:%02d",
                1 + rand() % 12, 1 + rand() % 28,
                rand() % 24, rand() % 60, rand() % 60);

        strcpy(logs.entries[i].level, data->levels[rand() % 4]);
        strcpy(logs.entries[i].module, data->modules[rand() % 4]);
        strcpy(logs.entries[i].message, data->messages[rand() % 6]);
    }
    return logs;
}

// Create test log file
void bash_1__create_log_file(bash_1__Log_Collection *logs, const char *filename) {
    FILE *f = fopen(filename, "w");
    for (int i = 0; i < logs->count; ++i) {
        fprintf(f, "[%s] [%s] [%s] %s\n",
                logs->entries[i].time, logs->entries[i].level,
                logs->entries[i].module, logs->entries[i].message);
    }
    fclose(f);
}

// Calculate stats based on produced log
void bash_1__calculate_stats(bash_1__Log_Collection *logs, bash_1__Log_Data *data,
        int *level_counts, int *module_counts) {
    memset(level_counts, 0, 4 * sizeof(int));
    memset(module_counts, 0, 4 * sizeof(int));

    for (int i = 0; i < logs->count; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (strcmp(logs->entries[i].level, data->levels[j]) == 0) level_counts[j]++;
            if (strcmp(logs->entries[i].module, data->modules[j]) == 0) module_counts[j]++;
        }
    }
}

// Run script and pipe its output
char *bash_1__run_script(const char *args) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), bash_1__PATH"log_analyzer.sh --file " bash_1__PATH".test.log %s", args);

    FILE *pipe = popen(cmd, "r");
    if (!pipe) return NULL;

    // Too lazy to do it properly. RAM is cheap
    char *output = malloc(4096);
    output[0] = '\0';
    char buffer[bash_1__MAX_LINE_LEN];

    while (fgets(buffer, sizeof(buffer), pipe)) {
        strcat(output, buffer);
    }

    pclose(pipe);
    return output;
}

// Test filters
bool bash_1__test_filter(bash_1__Log_Collection *logs, const char *filter_type, const char *filter_value) {
    int expected_count = 0;
    bool success = false;
    for (int i = 0; i < logs->count; ++i) {
        if ((strcmp(filter_type, "--level") == 0 && strcmp(logs->entries[i].level, filter_value) == 0)) {
            expected_count++;
        } else if (strcmp(filter_type, "--module") == 0 && strcmp(logs->entries[i].module, filter_value) == 0) {
            expected_count++;
        }
    }

    char args[64];
    snprintf(args, sizeof(args), "%s %s", filter_type, filter_value);
    char *output = bash_1__run_script(args);

    if (!output) {
        return false;
    }

    char expected_header[64];
    snprintf(expected_header, sizeof(expected_header), "== FILTERED LOGS ==\n%d entries found", expected_count);

    if (strstr(output, expected_header) == NULL) {
        success = false;
    } else {
        success = true;
    }

    free(output);
    return success;
}

// Test stats
bool bash_1__test_stats(bash_1__Log_Collection *logs, bash_1__Log_Data *data) {
    int level_counts[4], module_counts[4];
    bool success = false;

    bash_1__calculate_stats(logs, data, level_counts, module_counts);

    char *output = bash_1__run_script("--stats");
    if (!output) {
        return false;
    }

    char expected_stats[512];
    snprintf(expected_stats, sizeof(expected_stats),
        "== STATISTICS ==\n"
        "ERROR: %d\nWARN: %d\nINFO: %d\nDEBUG: %d\n"
        "---\n"
        "AUTH: %d\nDB: %d\nNETWORK: %d\nAPI: %d",
        level_counts[0], level_counts[1], level_counts[2], level_counts[3],
        module_counts[0], module_counts[1], module_counts[2], module_counts[3]);

    if (strstr(output, expected_stats) == NULL) {
        success = false;
    } else {
        success = true;
    }

    free(output);
    return success;
}


bool bash_1__main(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);
    srand(time(NULL));

    // Create log data and generate random logs
    bash_1__Log_Data log_data = bash_1__create_log_data();
    // Generating from 5 to MAX_ENTRIES entries
    int log_count = 5 + rand() % (bash_1__MAX_ENTRIES - 5 + 1);
    bash_1__Log_Collection logs = bash_1__generate_random_log(&log_data, log_count);

    // Write logs to file
    bash_1__create_log_file(&logs, bash_1__PATH".test.log");

    // Test filters in different modes
    for (int i = 0; i < 4; ++i) {
        if (!bash_1__test_filter(&logs, "--level", log_data.levels[i])) {
            lchk_add_feedback(result, "Fail: <./log_analyzer.sh ... --level %s> produces wrong output", log_data.levels[i]);
            break;
        }
        if (!bash_1__test_filter(&logs, "--module", log_data.modules[i])) {
            lchk_add_feedback(result, "Fail: <./log_analyzer.sh ... --module %s> produces wrong output", log_data.modules[i]);
            break;
        }
    }

    // Test stats
    if (!bash_1__test_stats(&logs, &log_data)) {
        lchk_add_feedback(result, "Fail: <./log_analyzer.sh ... --stats> produces wrong output");
    }

    // Test error handling
    char *output = bash_1__run_script("--file missing.log");
    if (!(output && strstr(output, "== ERROR =="))) {
        lchk_add_feedback(result, "Fail: Error handling is incorrect");   
    }

    // Perform cleanup
    free(output);
    unlink(bash_1__PATH".test.log");

    // Grade submission
    if (lchk_feedback_empty(result)) {
        lchk_add_feedback(result, "Pass: All tests ok");
        lchk_set_grade(result, 100); 
        return true;       
    } else {
        lchk_set_grade(result, 0);
        return false;
    }
}

LCHK_REGISTER_MODULE(bash_1, "bash-1", "Linux/bash-1 (task1.10.2)", "1.0", bash_1__main)


