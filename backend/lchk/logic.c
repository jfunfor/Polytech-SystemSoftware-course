#include "./lchk.h"
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <mntent.h>
#include <dirent.h>
#include <signal.h>

// EXAMPLE: Using linker sections.
/*
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
*/

static bool check_dummy(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);
    result->grade = 100;
    lchk_add_feedback(result, "Pass: All tests ok");
    return true;
}
LCHK_REGISTER_MODULE(dummy, "dummy", "0_Dummy/Pass", "0.0", check_dummy)


// ======================================================================================
//            ____                      _                      _   _                   
//           / ___| ___ _ __   ___ _ __(_) ___   ___  ___  ___| |_(_) ___  _ __        
//     _____| |  _ / _ \ '_ \ / _ \ '__| |/ __| / __|/ _ \/ __| __| |/ _ \| '_ \ _____ 
//    |_____| |_| |  __/ | | |  __/ |  | | (__  \__ \  __/ (__| |_| | (_) | | | |_____|
//           \____|\___|_| |_|\___|_|  |_|\___| |___/\___|\___|\__|_|\___/|_| |_|                                                                                       
// ======================================================================================

#ifndef HOME
#define HOME "/home/flopp/"
#endif // HOME

#define generic__CMD_BUFFER_SIZE    4096
#define generic__FILE_CONTENT_SIZE  4096

// Trim string 'str' from '\n' and '\r' chars
void generic__trim(char *str) {
    int len = strlen(str);
    while (len > 0 && (str[len-1] == '\n' || str[len-1] == '\r')) {
        str[--len] = '\0';
    }
}

// Trim string from ALL whitespace (space, tab, newline, etc.)
void generic__remove_all_whitespace(char *str) {
    char *src = str, *dst = str;
    while (*src) {
        if (*src != ' ' && *src != '\t' && *src != '\n' && *src != '\r') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}

/*
// Return minimum of 3 integers
int min3(int a, int b, int c) {
    int m = a;
    if (b < m) m = b;
    if (c < m) m = c;
    return m;
}
// Levenshtein distance calculation
int levenshtein_distance(const char *s1, const char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    
    // Handle edge cases
    if (len1 == 0) return len2;
    if (len2 == 0) return len1;
    if (s1 == s2) return 0;
    if (abs(len1 - len2) > 1000) return abs(len1 - len2) + 1;

    // Allocate matrix (using variable-length arrays for simplicity)
    int matrix[len1 + 1][len2 + 1];
    
    // Initialize first row and column
    for (int i = 0; i <= len1; i++) matrix[i][0] = i;
    for (int j = 0; j <= len2; j++) matrix[0][j] = j;
    
    // Fill the matrix
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            matrix[i][j] = min3(
                matrix[i - 1][j] + 1,       // deletion
                matrix[i][j - 1] + 1,       // insertion
                matrix[i - 1][j - 1] + cost // substitution
            );
        }
    }
    
    return matrix[len1][len2];
}
*/

// Check if 'needle' is a fuzzy substring of 'haystack'
bool generic__contains_fuzzy_substring(const char *haystack, const char *needle, int max_diff) {
    int h_len = strlen(haystack);
    int n_len = strlen(needle);

    if (n_len == 0) return true;
    if (h_len < n_len - max_diff) return false;

    for (int start = 0; start <= h_len - (n_len - max_diff); start++) {
        int errors = 0;
        int n_index = 0;
        int h_index = start;
        
        while (n_index < n_len && h_index < h_len && errors <= max_diff) {
            if (haystack[h_index] == needle[n_index]) {
                n_index++;
                h_index++;
            } else {
                errors++;
                if (h_index + 1 < h_len && haystack[h_index + 1] == needle[n_index]) {
                    h_index++; // Skip one character in haystack (insertion)
                } else if (n_index + 1 < n_len && haystack[h_index] == needle[n_index + 1]) {
                    n_index++; // Skip one character in needle (deletion)
                } else {
                    // Skip in both (substitution)
                    n_index++;
                    h_index++;
                }
            }
        }

        if (n_index == n_len && errors <= max_diff) return true;
    }
    return false;
}

// Execute system command and compare its output with expected one using fuzzy sting matching
bool generic__check_command_fuzzy(const char *command, const char *expected_output, int max_diff) {
    FILE *f;
    char buffer[generic__CMD_BUFFER_SIZE];
    bool found = false;

    f = popen(command, "r");
    if (!f) {
        return false;
    }

    while (fgets(buffer, sizeof(buffer), f)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        if (expected_output && generic__contains_fuzzy_substring(buffer, expected_output, max_diff)) {
            found = true;
            break;
        }
    }
    
    pclose(f);
    return found;
}


// Execute system command and compare its output with expected one
bool generic__check_command(const char *command, const char *expected_output) {
    FILE *f;
    char buffer[generic__CMD_BUFFER_SIZE];
    bool found = false;

    f = popen(command, "r");
    if (!f) {
        return false;
    }

    while (fgets(buffer, sizeof(buffer), f)) {
        if (expected_output && strstr(buffer, expected_output)) {
            found = true;
            break;
        }
    }

    pclose(f);
    return found;
}

// Execute system command silently (with '>/dev/null 2>&1' appended at the end)
bool generic__run_command(const char *command) {
    char silent_cmd[generic__CMD_BUFFER_SIZE];
    snprintf(silent_cmd, sizeof(silent_cmd), "%s >/dev/null 2>&1", command);
    return system(silent_cmd) == 0; 
}

// Execute system command
#define generic__run_command_macro(command) (system(command) == 0)

// Execute system command (skip return value of system() call)
#define generic__run_command_macro_noreturn(command) (system(command))

// Check if file exists
#define generic__file_exists(filename) (access(filename, F_OK) == 0)

// Check if file exists and is executable
bool generic__file_executable(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0 && (st.st_mode & S_IXUSR);
}

// Compare file content with expected one 
bool generic__check_file_content(const char *filename, const char *expected_content) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        return false;
    }

    char buffer[generic__FILE_CONTENT_SIZE];
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

// Compare two files
bool generic__compare_files(const char *file1, const char *file2) {
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

// Get file size in bytes
long generic__get_file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

// Get partition size in kilobytes
long generic__get_partition_size_kb(const char *device) {
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
bool generic__check_filesystem_type(const char *device, const char *expected_fs) {
    char command[256];
    snprintf(command, sizeof(command), "blkid -s TYPE -o vlaue %s 2>/dev/null", device);
    return generic__check_command(command, expected_fs);
}

// Check if 'device' is mounted
bool generic__mounted(const char *device, const char *mount_point) {
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

// Check if swap on 'device' is active
bool generic__swap_active(const char *device) {
    return generic__check_command("swapon --show=name --noheadings 2>/dev/null", device); 
}

// Check if there is a specified entry in /etc/fstab
bool generic__check_fstab_entry(const char *device, const char *mount_point, const char *fs_type) {
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

// Find PID of a process by its name (= pidof / pgrep)
pid_t generic__find_pid_by_name(const char *name) {
    char command[256];
    snprintf(command, sizeof(command), "pgrep %s", name);
    
    FILE *f = popen(command, "r");
    if (!f) return -1;

    pid_t pid = -1;
    if (fscanf(f, "%d", &pid) != 1) return -1;

    pclose(f);
    return pid;
}

// Check if docker container is running
bool generic__docker_container_running(const char *container) {
    char command[256];
    snprintf(command, sizeof(command), "docker inspect -f '{{.State.Running}}' %s 2>/dev/null", container);
    return generic__check_command(command, "true");
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
#define textproc_1__MAX_ENTRIES     1000

#ifndef textproc_1__PATH
#define textproc_1__PATH            HOME "textproc-1/"
#endif // textproc_1__PATH

#define textproc_1__ORIGINAL_FILE   textproc_1__PATH "virus_tf.log"
#define textproc_1__RESULT_FILE     textproc_1__PATH "filtered_logs.txt"
#define textproc_1__BACKUP_FILE     "/tmp/bk/.virus_tf.log"
#define textproc_1__BACKUP_SHA1     "/tmp/bk/.sha1"

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

            generic__trim(message);
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
        generic__trim(line);
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
    
    // Data corruption check  
    if (!generic__file_exists(textproc_1__BACKUP_FILE) ||
            !generic__file_exists(textproc_1__BACKUP_SHA1) ||
            !generic__file_exists(textproc_1__ORIGINAL_FILE) ||
            !find_diff_in_files(textproc_1__ORIGINAL_FILE, textproc_1__BACKUP_FILE, textproc_1__BACKUP_SHA1)) {
        lchk_add_feedback(result, "Fail: Aborted, found intentionally corrupted data");
        goto textproc_1__fail;
    }

    textproc_1__Log_Entry expected[textproc_1__MAX_ENTRIES], student[textproc_1__MAX_ENTRIES];
    
    // Parse initial log file
    int expected_count = textproc_1__parse_log(textproc_1__ORIGINAL_FILE, expected);
    if (expected_count < 0) {
        lchk_add_feedback(result, "Fail: Error reading '%s'", textproc_1__ORIGINAL_FILE);
        goto textproc_1__fail;
    }

    // Make required transformations to logs 
    textproc_1__sort_and_number(expected, expected_count);

    // Read student file
    int student_count = textproc_1__read_student(textproc_1__RESULT_FILE, student);
    if (student_count < 0) {
        lchk_add_feedback(result, "Fail: Error reading '%s'", textproc_1__RESULT_FILE);
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

LCHK_REGISTER_MODULE(textproc_1, "textproc-1", "1_Linux_basics/textproc-1", "1.0", textproc_1__main)


// ========================================================
//     _            _                            ____  
//    | |_ _____  _| |_ _ __  _ __ ___   ___    |___  | 
//    | __/ _ \ \/ / __| '_ \| '__/ _ \ / __|____ __) |
//    | ||  __/>  <| |_| |_) | | | (_) | (_|_____/ __/ 
//     \__\___/_/\_\\__| .__/|_|  \___/ \___|   |_____|
//                     |_|                             
// ========================================================

#define textproc_2__MAX_ENTRIES     1000
#define textproc_2__MAX_LINE_LENGTH 1024
#define textproc_2__MAX_OUTPUT_SIZE 16384

#ifndef textproc_2__PATH
#define textproc_2__PATH HOME       "textproc-2/"
#endif // textproc_2__PATH

#define textproc_2__ORIGINAL_JSON   textproc_2__PATH "manifest.json"
#define textproc_2__BACKUP_JSON     "/tmp/bk/.manifest.json"
#define textproc_2__BACKUP_SHA1     "/tmp/bk/.sha1" 

typedef struct {
    char id[50];
    char status[20];
    int priority;
} textproc_2__Entry;

typedef struct {
    char id[50];
    int priority;
} textproc_2__Table_Row;

// TODO: Consider replacing with generic
// Basically it's just trimming whitespace
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
        // Try to read one more line
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

// Generate expected output in text and mardown files
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
    
    // Generate text output
    char *txt_ptr = txt_output;
    for (int i = 0; i < active_count; i++) {
        txt_ptr += snprintf(txt_ptr, textproc_2__MAX_OUTPUT_SIZE - (txt_ptr - txt_output),
                        "%-8s | %d\n", active_entries[i].id, active_entries[i].priority);
    }
    
    // Generate markdown output
    char *md_ptr = md_output;
    md_ptr += snprintf(md_ptr, textproc_2__MAX_OUTPUT_SIZE - (md_ptr - md_output),
                     "| ID      | PRIORITY |\n|---------|----------|\n");
    
    for (int i = 0; i < active_count; i++) {
        md_ptr += snprintf(md_ptr, textproc_2__MAX_OUTPUT_SIZE - (md_ptr - md_output),
                         "| %-7s | %-8d |\n", active_entries[i].id, active_entries[i].priority);
    }
}

// Print difference between markdown files in absolutely crushing way!
// As its name suggests, this function should never be called in release version. 
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
    
    // Data corruption check
    if (!generic__file_exists(textproc_2__BACKUP_JSON) ||
            !generic__file_exists(textproc_2__BACKUP_SHA1) ||
            !generic__file_exists(textproc_2__ORIGINAL_JSON) ||
            !find_diff_in_files(textproc_2__ORIGINAL_JSON, textproc_2__BACKUP_JSON, textproc_2__BACKUP_SHA1)) {
        lchk_add_feedback(result, "Fail: Aborted, found intentionally corrupted data");
        lchk_set_grade(result, 0);
        return false;
    }


    textproc_2__Entry entries[textproc_2__MAX_ENTRIES] = {0};
    int count = 0;
    
    // Parse input file
    if (!textproc_2__parse_json_file(textproc_2__ORIGINAL_JSON, entries, &count)) {
        lchk_add_feedback(result, "Fail: Error reading '%s'", textproc_2__ORIGINAL_JSON);
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
    bool txt_match = generic__compare_files(textproc_2__PATH".expected_result.txt", textproc_2__PATH"result.txt");
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

LCHK_REGISTER_MODULE(textproc_2, "textproc-2", "1_Linux_basics/textproc-2", "1.0", textproc_2__main)


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
#define storage_1__BOOT_MOUNT_POINT "/mnt/vaccine/boot"
#define storage_1__ROOT_MOUNT_POINT "/mnt/vaccine/"
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
        if (!generic__file_exists(part->device)) {
            lchk_add_feedback(result, "Fail: Device '%s' does not exist", part->device);
            continue;
        }
        
        // Check for filesystem type
        if (!generic__check_filesystem_type(part->device, part->fs_type)) {
            lchk_add_feedback(result, "Fail: Incorrect filesystem type for '%s'", part->device);
        }
        
        // Check for size
        if (part->expected_size_mb > 0) {
            long actual_size_kb = generic__get_partition_size_kb(part->device);
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
            if (!generic__swap_active(part->device)) {
                lchk_add_feedback(result, "Fail: Swap partition '%s' is not active", part->device);
            }
        } else if (part->mount_point) {
            if (!generic__mounted(part->device, part->mount_point)) {
                lchk_add_feedback(result, "Fail: Partition '%s' is not mounted to '%s'", part->device, part->mount_point);
            }
            // Additional check of mount point
            if (!generic__file_exists(part->mount_point)) {
                lchk_add_feedback(result, "Fail: Mount point '%s' does not exist", part->mount_point);
            }
        }

        // Check fstab entry
        if (!generic__check_fstab_entry(part->device, part->mount_point, part->fs_type)) {
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

LCHK_REGISTER_MODULE(storage_1, "storage-1", "1_Linux_basics/storage-1", "1.0", storage_1__main)


// ========================================================
//         _                                  ____  
//     ___| |_ ___  _ __ __ _  __ _  ___     |___  | 
//    / __| __/ _ \| '__/ _` |/ _` |/ _ \_____ __) |
//    \__ \ || (_) | | | (_| | (_| |  __/_____/ __/ 
//    |___/\__\___/|_|  \__,_|\__, |\___|    |_____|
//                            |___/                 
// ========================================================

#define storage_2__PV1_DEVICE           "/dev/sdb"
#define storage_2__PV2_DEVICE           "/dev/sdc"
#define storage_2__VG_NAME              "vaccine_vg"
#define storage_2__DATA_LV              "/dev/vaccine_vg/data_lv"
#define storage_2__BACKUP_LV            "/dev/vaccine_vg/backup_lv"
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

// Logical volumes
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
    if (!generic__file_exists(pv_device)) return false;

    char command[256];
    snprintf(command, sizeof(command), "pvs --noheadings -o pv_name %s 2>/dev/null", pv_device);
    return generic__check_command(command, pv_device);
}

// Check if 'vg_name' volume group exists
bool storage_2__check_volume_group(const char *vg_name) {
    char command[256];
    snprintf(command, sizeof(command), "vgs --noheadings -o vg_name %s 2>/dev/null", vg_name);
    if (!generic__check_command(command, vg_name)) return false;

    // Check if PVS are in 'vg_name'
    for (int i = 0; i < storage_2__NUM_PVS; ++i) {
        snprintf(command, sizeof(command), "vgs --noheadings -o vg_name %s 2>/dev/null", physical_volumes[i]);
        if (!generic__check_command(command, vg_name)) return false;
    }

    return true;
}

// Check if logical volume exists and is correct
bool storage_2__check_logical_volume(const storage_2__Lv_Info *lv) {
    // Basic thingy
    if (!generic__file_exists(lv->lv_device)) return false;
    if (!generic__check_filesystem_type(lv->lv_device, lv->fs_type)) return false;

    // Size thingy
    long actual_size_kb = generic__get_partition_size_kb(lv->lv_device);
    long expected_size_kb = lv->expected_size_mb * 1024;

    if (actual_size_kb > 0) {
        long tolerance = expected_size_kb * 0.1;
        if (labs(actual_size_kb - expected_size_kb) > tolerance) {
            return false;
        }
    }
    
    // Mount thingy
    if (!generic__mounted(lv->lv_device, lv->mount_point)) return false;
    if (!generic__file_exists(lv->mount_point)) return false;

    // Fstab thingy
    if (!generic__check_fstab_entry(lv->lv_device, lv->mount_point, lv->fs_type)) return false;

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

LCHK_REGISTER_MODULE(storage_2, "storage-2", "1_Linux_basics/storage-2", "1.0", storage_2__main)


// ========================================================
//         _                                  _____
//     ___| |_ ___  _ __ __ _  __ _  ___     |___ /
//    / __| __/ _ \| '__/ _` |/ _` |/ _ \_____ |_ |
//    \__ \ || (_) | | | (_| | (_| |  __/_____|__) |
//    |___/\__\___/|_|  \__,_|\__, |\___|    |____/
//                            |___/
// ========================================================

#define storage_3__RAID_DEVICE          "/dev/md0"
#define storage_3__RAID_DEVICE_SHORT    "md0"
#define storage_3__DISK1                "/dev/sdb"
#define storage_3__DISK2                "/dev/sdc"
#define storage_3__MOUNT_POINT          "/mnt/ai_storage"
#define storage_3__FS_TYPE              "ext4"
#define storage_3__SWAP_FILE            "/swapfile"
#define storage_3__SWAP_SIZE_MB         1024
#define storage_3__INODE_SIZE           256

// Check RAID array
bool storage_3__check_raid_array() {
    // Check if device exists
    if (!generic__file_exists(storage_3__RAID_DEVICE)) {
        return false;
    }

    // Check RAID condition
    if (!generic__check_command("mdadm --detail "storage_3__RAID_DEVICE" 2>/dev/null | grep 'State'", "clean")) {
        return false;
    }

    // Check RAID level
    if (!generic__check_command("mdadm --detail "storage_3__RAID_DEVICE" 2>/dev/null | grep 'Rail Level'", "raid1")) {
        return false;
    }

    return true;
}

// Check disks in RAID
bool storage_3__check_raid_disks() {
    // Check for disk 1 
    if (!generic__check_command("mdadm --detail "storage_3__RAID_DEVICE" 2>/dev/null", storage_3__DISK1)) {
        return false;
    }
    
    // Check for disk 2
    if (!generic__check_command("mdadm --detail "storage_3__RAID_DEVICE" 2>/dev/null", storage_3__DISK2)) {
        return false;
    }

    return true;
}

// Check settings in 'mdadm.conf'
bool storage_3__check_mdadm_conf() {
    // Check if 'mdadm.conf' is present in either of 2 locations
    if (!generic__file_exists("/etc/mdadm/mdadm.conf") && !generic__file_exists("/etc/mdadm.conf")) {
        return false;
    }

    // Check for short device name in 'mdadm.conf'
    if (!generic__check_file_content("/etc/mdadm/mdadm.conf", storage_3__RAID_DEVICE_SHORT) && 
            !generic__check_file_content("/etc/mdadm.conf", storage_3__RAID_DEVICE_SHORT)) {
        return false;
    }

    return true;
}

// Check inode size
bool storage_3__check_inode_size() {
    char command[256];
    char buffer[128];
    bool inode_correct = false;

    snprintf(command, sizeof(command), "tune2fs -l %s 2>/dev/null | grep 'Inode size'", storage_3__RAID_DEVICE);
    FILE *f = popen(command, "r");
    if (f) {
        if (fgets(buffer, sizeof(buffer), f)) {
            int inode;
            if (sscanf(buffer, "Inode size: %d", &inode) == 1) {
                if (inode == storage_3__INODE_SIZE) {
                    inode_correct = true;
                }
            }
        }
        pclose(f);
    }

    return inode_correct;
}

// Check swap file
bool storage_3__check_swap_file() {
    // Check if swap exists
    if (!generic__file_exists(storage_3__SWAP_FILE)) {
        return false;
    }

    // Check swap size 
    long actual_size = generic__get_file_size(storage_3__SWAP_FILE);
    long expected_size = storage_1__SWAP_SIZE_MB * 1024 * 1024;

    if (actual_size != expected_size) {
        return false;
    }

    // Check if swap is active
    if (!generic__swap_active(storage_3__SWAP_FILE)) {
        return false;
    }

    // Check fstab entry
    if (!generic__check_fstab_entry(storage_3__SWAP_FILE, NULL, "swap")) {
        return false;
    }

    return true;
}

// Check test file
bool storage_3__check_test_file() {
    DIR *dir;
    struct dirent *entry;
    bool found = false;
    
    dir = opendir(storage_3__MOUNT_POINT);
    if (dir == NULL) {
        return false;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Skip system files and directories
        if (entry->d_name[0] == '.' || entry->d_type == DT_DIR) {
            continue;
        }

        if (entry->d_type == DT_REG) {
            found = true;
            break;
        }
    }

    closedir(dir);
    return found;
}


bool storage_3__main(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);
    
    if (!storage_3__check_raid_array()) {
        lchk_add_feedback(result, "Fail: RAID array '%s' is misconfigured or does not exist", storage_3__RAID_DEVICE);
        lchk_set_grade(result, 0);
        return false;
    }

    if (!storage_3__check_raid_disks()) {
        lchk_add_feedback(result, "Fail: Disks in RAID are wrong");
    }

    if (!storage_3__check_mdadm_conf()) {
        lchk_add_feedback(result, "Fail: 'mdadm.conf' is misconfigured");
    }

    if (!storage_3__check_inode_size()) {
        lchk_add_feedback(result, "Fail: Inode is of wrong size");
    }

    if (!generic__check_filesystem_type(storage_3__RAID_DEVICE, storage_3__FS_TYPE)) {
        lchk_add_feedback(result, "Fail: Incorrect filesystem type for '%s'", storage_3__RAID_DEVICE);
    }

    if (!generic__mounted(storage_3__RAID_DEVICE, storage_3__MOUNT_POINT)) {
        lchk_add_feedback(result, "Fail: RAID array '%s' is not mounted to '%s'", storage_3__RAID_DEVICE, storage_3__MOUNT_POINT);
    }
  
    if (!generic__file_exists(storage_3__MOUNT_POINT)) {
        lchk_add_feedback(result, "Fail: Mount point '%s' does not exist", storage_3__MOUNT_POINT);
    }

    if (!generic__check_fstab_entry(storage_3__RAID_DEVICE, storage_3__MOUNT_POINT, storage_3__FS_TYPE)) {
        lchk_add_feedback(result, "Fail: /etc/fstab is misconfigured");
    }

    if (!storage_3__check_swap_file()) {
        lchk_add_feedback(result, "Fail: Swap file is of incorrect size or is not active");
    }
    
    if (!storage_3__check_test_file()) {
        lchk_add_feedback(result, "Fail: Test file (for checking RAID behaviour with faulty disk) is not found");
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

LCHK_REGISTER_MODULE(storage_3, "storage-3", "1_Linux_basics/storage-3", "1.0", storage_3__main)


// ========================================================
//                                             _
//     _ __  _ __ ___   ___ ___  ___ ___      / |
//    | '_ \| '__/ _ \ / __/ _ \/ __/ __|_____| |
//    | |_) | | | (_) | (_|  __/\__ \__ \_____| |
//    | .__/|_|  \___/ \___\___||___/___/     |_|
//    |_|                                        
// ========================================================

#define process_1__LOG_FILE     "/tmp/zombie_army.log"
#define process_1__STATUS_FILE  "/tmp/zombie_army.status"

// Check if 'zombie_general' and its army terminated correctly
bool process_1__check_graceful_exit() {
    // Something is wrong or someone is cheating
    if (!generic__file_exists(process_1__STATUS_FILE)) {
        return false;
    }

    // Check for marker in the status file
    if (!generic__check_file_content(process_1__STATUS_FILE, "graceful_exit")) {
        return false;
    }
    
    return true;
}

// Get list of PIDs from log file
int process_1__get_pids_from_log(pid_t *pids, int max_pids) {
    FILE *log = fopen(process_1__LOG_FILE, "r");
    if (!log) return 0;

    int count = 0;
    char line[256];

    while (fgets(line, sizeof(line), log) && count < max_pids) {
        if (!strstr(line, "pid=")) continue;
        pid_t pid;
        if (sscanf(line, "pid=%d:", &pid) == 1) {
            pids[count++] = pid;
        }
    }

    fclose(log);
    return count;
}

// Check if process is in Z-state
bool process_1__is_zombie(pid_t pid) {
    char stat_path[267];
    snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);
    
    FILE *stat_file = fopen(stat_path, "r");
    if (!stat_file) return false;

    char state;
    fscanf(stat_file, "%*d %*s %c", &state);
    fclose(stat_file);

    return (state == 'Z');
}

// Check if zombie processes from log are currently present
bool process_1__check_zombies_from_log() {
    pid_t pids[100];
    int num_pids = process_1__get_pids_from_log(pids, 100);
    int zombies = 0;

    for (int i = 0; i < num_pids; ++i) {
        if (process_1__is_zombie(pids[i])) {
            zombies++;
        }
    }

    return zombies == 0;
}

// Check if zombie processes are currently present
bool process_1__check_all_zombies() {
    DIR *proc = opendir("/proc");
    if (!proc) return false;

    int zombies = 0;
    struct dirent *entry;

    while ((entry = readdir(proc)) != NULL) {
        if (!isdigit(*entry->d_name)) continue;

        char stat_path[267];
        snprintf(stat_path, sizeof(stat_path), "/proc/%s/stat", entry->d_name);
        FILE *stat_file = fopen(stat_path, "r");
        if (!stat_file) continue;

        char state;
        fscanf(stat_file, "%*d %*s %c", &state);
        fclose(stat_file);

        if (state == 'Z') {
            zombies++;
        }
    }

    closedir(proc);
    return (zombies == 0);
}

// Additional check for probably remaining processes from log file
bool process_1__check_remaining_processes() {
    pid_t pids[100];
    int num_pids = process_1__get_pids_from_log(pids, 100);
    int processes = 0;

    for (int i = 0; i < num_pids; ++i) {
        char path[32];
        snprintf(path, sizeof(path), "/proc/%d", pids[i]);
        if (access(path, F_OK) == 0) {
            if (!process_1__is_zombie(pids[i])) {
                processes++;
            }
        } 
    }

    return processes == 0;
}


bool process_1__main(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config); 
    
    if (!process_1__check_graceful_exit()) {
        lchk_add_feedback(result, "Fail: Processes were not terminated properly");
    }

    if (!process_1__check_zombies_from_log()) {
        lchk_add_feedback(result, "Fail: Found zombie processes");
    }

    if (!process_1__check_remaining_processes()) {
        lchk_add_feedback(result, "Fail: Found remaining processes");
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

LCHK_REGISTER_MODULE(process_1, "process-1", "1_Linux_basics/process-1", "1.0", process_1__main)


// ========================================================
//                      _                          _ 
//     _ __   __ _  ___| | ____ _  __ _  ___      / |
//    | '_ \ / _` |/ __| |/ / _` |/ _` |/ _ \_____| |
//    | |_) | (_| | (__|   < (_| | (_| |  __/_____| |
//    | .__/ \__,_|\___|_|\_\__,_|\__, |\___|     |_|
//    |_|                         |___/              
// ========================================================

#define package_1__PKG_LIST_PATH    "/var/log/pkg-list.log"
#define package_1__REPOS_PATH       "/var/log/repos.log"

// Check if 'package' is installed
bool package_1__package_installed(const char *package_name) {
    char command[256];
    snprintf(command, sizeof(command), "dpkg -l | grep \"^ii\" | grep \" %s \"", package_name);
    return generic__check_command(command, package_name);
}

// Check APT configuration
bool package_1__check_apt_security() {
    // Check if there are files with security config
    if (generic__check_command("grep -r \"AllowUnauthenticated.*false\" /etc/apt/apt.conf.d/", "false") ||
            generic__check_command("grep -r \"Allow-Insecure.*false\" /etc/apt/apt.conf.d/", "false")) {
        return true;
    }

    // Check if there are no 'allow unauthenticated' configs
    if (generic__check_command("grep -r \"AllowUnauthenticated.*true\" /etc/apt/apt.conf.d/", "true") ||
            generic__check_command("grep -r \"Allow-Insecure.*true\" /etc/apt/apt.conf.d/", "true")) {
        return false;
    }

    // By default, installation fron unsigned repos is disabled in Ubuntu.
    // But for sanity-check it's better to consider questioning current behaviour.
    FILE *f = popen("apt-config dump | grep -A1 \"AllowUnauthenticated\" | grep -q \"false\" && echo \"secure\"", "r");
    if (f) {
        char buffer[generic__CMD_BUFFER_SIZE];
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
    if (!generic__check_command("find /var/lib/apt/lists/ -name \"*Packages*\" -mtime -1", "Packages")) {
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
    if (!generic__file_exists(package_1__PKG_LIST_PATH)) {
        lchk_add_feedback(result, "Fail: File '"package_1__PKG_LIST_PATH"' does not exist");
        lchk_set_grade(result, 0);
        return false;
    }

    // Check pkg-list contents
    if (!generic__check_file_content(package_1__PKG_LIST_PATH, "ii ") && 
            !generic__check_file_content(package_1__PKG_LIST_PATH, "ubuntu")) {
        lchk_add_feedback(result, "Fail: File '"package_1__PKG_LIST_PATH"' does not contain package list");
    }

    // Check if telnet is uninstalled
    if (package_1__package_installed("telnet")) {
        lchk_add_feedback(result, "Fail: Package telnet is installed");
    }

    // Check APT config
    if (!package_1__check_apt_security()) {
        lchk_add_feedback(result, "Fail: APT is not configured");
    }

    // Check repos file
    if (!generic__file_exists(package_1__REPOS_PATH)) {
        lchk_add_feedback(result, "Fail: File '"package_1__REPOS_PATH"' does not exist");
        lchk_set_grade(result, 0);
        return false;
    }

    // Check repos contents
    if (!generic__check_file_content(package_1__REPOS_PATH, "http") && 
            !generic__check_file_content(package_1__REPOS_PATH, "ubuntu.com") &&
            !generic__check_file_content(package_1__REPOS_PATH, "deb ")) {
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

LCHK_REGISTER_MODULE(package_1, "package-1", "1_Linux_basics/package-1", "1.0", package_1__main)


// ========================================================
//                   _                     _       _ 
//     ___ _   _ ___| |_ ___ _ __ ___   __| |     / |
//    / __| | | / __| __/ _ \ '_ ` _ \ / _` |_____| |
//    \__ \ |_| \__ \ ||  __/ | | | | | (_| |_____| |
//    |___/\__, |___/\__\___|_| |_| |_|\__,_|     |_|
//         |___/
// ========================================================

#define systemd_1__MARKER_FILE      "/var/cowgreet.done"
#define systemd_1__SERVICE_NAME     "cowgreet.service"
#define systemd_1__SCRIPT_PATH      "/usr/local/bin/cowgreet.sh"
#define systemd_1__HOW_MUCH_LAGGY   10
// It's different on Ubuntu! Make sure to compile with -Dsystemd_1__COWSAY_PATH
#ifndef systemd_1__COWSAY_PATH
#define systemd_1__COWSAY_PATH      "/usr/bin/cowsay"
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
        // If we're here it's either a permisson problem or an invalid cowsay path.
        lchk_add_feedback(result, "Fail: Aborted, details are not provided");
        lchk_set_grade(result, 0);
        return false;
    }
    
    // TODO: Replace system() calls with generic__ wrappers
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

LCHK_REGISTER_MODULE(systemd_1, "systemd-1", "1_Linux_basics/systemd-1", "1.0", systemd_1__main)


// ========================================================
//                   _                     _      ____  
//     ___ _   _ ___| |_ ___ _ __ ___   __| |    |___  | 
//    / __| | | / __| __/ _ \ '_ ` _ \ / _` |_____ __) |
//    \__ \ |_| \__ \ ||  __/ | | | | | (_| |_____/ __/ 
//    |___/\__, |___/\__\___|_| |_| |_|\__,_|    |_____|
//         |___/                                        
// ======================================================== 

#ifndef systemd_2__PKG_PATH
#define systemd_2__PKG_PATH HOME    "systemd-2/cerberus-telemetry.deb"
#endif //systemd_2__PKG_PATH
#define systemd_2__SERVICE_PATH     "/lib/systemd/system/cerberus.service"
#define systemd_2__SCRIPT_PATH      "/usr/bin/cerberus-log.sh"

// Check if 'cerberus-telemetry' package is installed
bool systemd_2__package_installed() {
    return generic__run_command("dpkg -l | grep -q '^ii.*cerberus-telemetry' >/dev/null 2>&1");
}

// Remove installed 'cerberus-telemetry' package
bool systemd_2__remove_package() {
    // Stop service and then remove package
    generic__run_command("sudo systemctl stop cerberus.service 2>/dev/null");
    generic__run_command("sudo systemctl disable cerberus.service 2>/dev/null");
    if (!generic__run_command("sudo dpkg -r cerberus-telemetry")) {
        return false;
    }
    // Reload systemd after deinstallation
    generic__run_command("sudo systemctl daemon-reload");
    return true; // Assume everything's valid
}

// TODO: This function seems kinda crappy
bool systemd_2__install_package() {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "sudo dpkg -i "systemd_2__PKG_PATH" >/dev/null 2>&1");
    if (!generic__run_command(cmd)) {
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
    if (!generic__file_exists(systemd_2__SCRIPT_PATH) || 
            !generic__file_exists(systemd_2__SERVICE_PATH)) {
        lchk_add_feedback(result, "Fail: Package structure is incorrect");
    }
    if (!generic__file_executable(systemd_2__SCRIPT_PATH)) {
        lchk_add_feedback(result, "Fail: Script '/usr/bin/cerberus-log.sh' is not executable");
    }

    // Perform service checks
    if (!generic__run_command("systemctl is-active cerberus.service >/dev/null 2>&1") ||
            !generic__run_command("systemctl is-enabled cerberus.service >/dev/null 2>&1") ||
            !generic__run_command("journalctl -t cerberus --since='1 min ago' | head -5 | grep -q cerberus")) {
        lchk_add_feedback(result, "Fail: Service 'cerberus.service' is misconfigured");
    }

    // Check service file contents
    if (!generic__check_file_content(systemd_2__SERVICE_PATH, "WantedBy=multi-user.target") ||
            !generic__check_file_content(systemd_2__SERVICE_PATH, "SyslogIdentifier=cerberus")) {
        lchk_add_feedback(result, "Fail: Service 'cerberus.service' does not match the given task");
    }

    // Remove package
    if (!systemd_2__remove_package()) {
        lchk_add_feedback(result, "Fail: Unable to remove package");
    }

    // Verify cleanup
    if (generic__file_exists(systemd_2__SCRIPT_PATH) || generic__file_exists(systemd_2__SERVICE_PATH)) {
        lchk_add_feedback(result, "Fail: Files stil present after package was removed");
    }
    if (generic__run_command("systemctl is-active cerberus.service >/dev/null 2>&1")) {
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

LCHK_REGISTER_MODULE(systemd_2, "systemd-2", "1_Linux_basics/systemd-2", "1.0", systemd_2__main)


// ====================================
//     _               _           _
//    | |__   __ _ ___| |__       / |
//    | '_ \ / _` / __| '_ \ _____| |
//    | |_) | (_| \__ \ | | |_____| |
//    |_.__/ \__,_|___/_| |_|     |_|
//
// ====================================

#define bash_1__MAX_ENTRIES     20
#define bash_1__MAX_LINE_LEN    256
#ifndef bash_1__PATH
#define bash_1__PATH HOME       "bash-1/"
#endif // bash_1_PATH

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

    // Check if 'log_analyzer.sh' file exists
    if (!generic__file_exists(bash_1__PATH"log_analyzer.sh")) {
        lchk_add_feedback(result, "Fail: Script '"bash_1__PATH"log_analyzer.sh' does not exist");
        lchk_set_grade(result, 0);
        return false;
    }

    // Check if 'log_analyzer.sh' is executable
    if (!generic__file_executable(bash_1__PATH"log_analyzer.sh")) {
        lchk_add_feedback(result, "Fail: Script '"bash_1__PATH"log_analyzer.sh' is not executable");
        lchk_set_grade(result, 0);
        return false;

    }

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

LCHK_REGISTER_MODULE(bash_1, "bash-1", "1_Linux_basics/bash-1", "1.0", bash_1__main)


// ====================================
//                _        _ 
//     _ __   ___| |_     / |
//    | '_ \ / _ \ __|____| |
//    | | | |  __/ ||_____| |
//    |_| |_|\___|\__|    |_|
// ====================================

#ifndef net_1__PATH
#define net_1__PATH HOME            "net-1/"
#endif // net_1__PATH

#define net_1__REF_DATA net_1__PATH ".ref.json"
#define net_1__STU_DATA net_1__PATH "result.json"


bool net_1__main(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);
    
    // If student file does not exist - what are we doing here?
    if (!generic__file_exists(net_1__STU_DATA)) {
        lchk_add_feedback(result, "Fail: File '%s' does not exist", net_1__STU_DATA);
        lchk_set_grade(result, 0);
        return false;
    }

    // Find 'scdff' process
    pid_t scdff_pid = generic__find_pid_by_name("scdff");
    if (scdff_pid == -1) {
        lchk_add_feedback(result, "Fail: Messing with unknown processes was not your task. Really bad");
        lchk_set_grade(result, 0);
        return false;
    }

    // Send 'SIGUSR1' to 'scdff' 
    if (kill(scdff_pid, SIGUSR1) == -1) {
        lchk_add_feedback(result, "Fail: Aborted, details are not provided");
        lchk_set_grade(result, 0);
        return false;
    }

    // Wait for file creation
    sleep(3);

    // If reference file was not created - something is terribly wrong
    if (!generic__file_exists(net_1__REF_DATA)) {
        lchk_add_feedback(result, "Fail: Aborted, details are not provided");
        lchk_set_grade(result, 0);
        return false;
    }
   
    // Compare student and reference files
    if (!generic__compare_files(net_1__REF_DATA, net_1__STU_DATA)) {
        unlink(net_1__REF_DATA);
        lchk_add_feedback(result, "Fail: Results are incorrect");
        lchk_set_grade(result, 0);
        return false;
    }
 
    unlink(net_1__REF_DATA);
    kill(scdff_pid, SIGTERM);
    lchk_add_feedback(result, "Pass: All tests ok");
    lchk_set_grade(result, 100);
    return true;
} 

LCHK_REGISTER_MODULE(net_1, "net-1", "2_Networks/net-1", "1.0", net_1__main)


// ====================================
//                _       ____
//     _ __   ___| |_    |___  |
//    | '_ \ / _ \ __|____ __) |
//    | | | |  __/ ||_____/ __/
//    |_| |_|\___|\__|   |_____|
//                           
// ====================================  

#ifndef net_2__PATH
#define net_2__PATH                 HOME "net-2/"
#endif // net_2__PATH

#define net_2__CLIENT_SCRIPT        "/root/client/ping_target.sh"
#define net_2__PING_RESPONDER_SRIPT "/root/ping_responder.py"
#define net_2__PING_FROM_DOCKER     net_2__PATH ".ping"
#define net_2__PING_FROM_SCDFF      net_2__PATH "ping_responder.py"


bool net_2__main(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);

    // Check if all containers are running
    if (!generic__docker_container_running("client") ||
            !generic__docker_container_running("router1") ||
            !generic__docker_container_running("router2") ||
            !generic__docker_container_running("router_reserve") ||
            !generic__docker_container_running("target")) {
        lchk_add_feedback(result, "Fail: Not all containers are in running state");
        lchk_set_grade(result, 0);
        return false;
    }

    // Check if 'ping_responder.py' is running on target
    if (!generic__run_command("docker exec target pgrep -f "net_2__PING_RESPONDER_SRIPT)) {
        lchk_add_feedback(result, "Fail: Running state of '"net_2__PING_RESPONDER_SRIPT"' is nessesary to continue");
        lchk_set_grade(result, 0);
        return false;
    }

    // Check if student script ('ping_target.sh') exists
    if (!generic__run_command("docker exec client [ -f "net_2__CLIENT_SCRIPT" ]")) {
        lchk_add_feedback(result, "Fail: Script '"net_2__CLIENT_SCRIPT"' does not exist");
        lchk_set_grade(result, 0);
        return false;
    }

    // Check if 'ping_responder.py' is original.
    // First, copy it from container.
    if (!generic__run_command("docker cp target:"net_2__PING_RESPONDER_SRIPT" "net_2__PING_FROM_DOCKER)) {
        lchk_add_feedback(result, "Fail: Aborted, details are not provided");
        lchk_set_grade(result, 0);
        return false;
    }

    // Find 'scdff' process
    pid_t scdff_pid = generic__find_pid_by_name("scdff");
    if (scdff_pid == -1) {
        lchk_add_feedback(result, "Fail: Messing with unknown processes was not your task. Really bad");
        lchk_set_grade(result, 0);
        return false;
    }

    // Send 'SIGUSR1' to 'scdff' 
    if (kill(scdff_pid, SIGUSR1) == -1) {
        lchk_add_feedback(result, "Fail: Aborted, details are not provided");
        lchk_set_grade(result, 0);
        return false;
    }

    // Wait for file creation
    sleep(3);

    // If reference file was not created - something is terribly wrong
    if (!generic__file_exists(net_2__PING_FROM_SCDFF)) {
        lchk_add_feedback(result, "Fail: Aborted, details are not provided");
        lchk_set_grade(result, 0);
        return false;
    }
   
    // Compare student and reference files
    if (!generic__compare_files(net_2__PING_FROM_DOCKER, net_2__PING_FROM_SCDFF)) {
        unlink(net_2__PING_FROM_DOCKER);
        unlink(net_2__PING_FROM_SCDFF);
        lchk_add_feedback(result, "Fail: Modifying "net_2__PING_RESPONDER_SRIPT" is forbidden");
        lchk_set_grade(result, 0);
        return false;
    }

    unlink(net_2__PING_FROM_DOCKER);
    unlink(net_2__PING_FROM_SCDFF);

    // Check connection between client and target
    if (!generic__run_command("docker exec client ping -c 2 -W 1 10.30.30.10")) {
        lchk_add_feedback(result, "Fail: No connection, target is unreachable from client");
        lchk_set_grade(result, 0);
        return false;
    }
    
    // Check output from the student script
    if (!generic__check_command("docker exec client "net_2__CLIENT_SCRIPT, "1234")) {
        lchk_add_feedback(result, "Fail: Script '"net_2__CLIENT_SCRIPT"' does not produce valid output");
        lchk_set_grade(result, 0);
        return false;
    }
    
    // Stop 'ping_responder.py' to check whether student output is hardcocded.
    // It's actually safe not to check return value here 
    // since we made sure that process 'ping_responder.py' is safe and sound.
    generic__run_command("docker exec target pkill -f "net_2__PING_RESPONDER_SRIPT);

    // Repeat output check but with timeout (in case student script hangs upppp).
    // Note that the logic is inversed.
    if (generic__check_command("docker exec client timeout 5 "net_2__CLIENT_SCRIPT, "1234")) {
        lchk_add_feedback(result, "Fail: Cheating is not a way");
        lchk_set_grade(result, 0);
        // Start 'ping_responder.py' again
        generic__run_command("docker exec target python3 "net_2__PING_RESPONDER_SRIPT" &");
        return false;
    }
    
    // Everything's working, everything's twerking
    generic__run_command("docker exec -d target python3 "net_2__PING_RESPONDER_SRIPT);
    lchk_add_feedback(result, "Pass: All tests ok");
    lchk_set_grade(result, 100);
    return true;
}

LCHK_REGISTER_MODULE(net_2, "net-2", "2_Networks/net-2", "1.0", net_2__main)


// ====================================
//                _       _____ 
//     _ __   ___| |_    |___  / 
//    | '_ \ / _ \ __|____ |_  |
//    | | | |  __/ ||_____|_ )  |
//    |_| |_|\___|\__|   |_____/ 
// ====================================                          

#ifndef net_3__PATH
#define net_3__PATH HOME "net3/"
#endif // net_3__PATH

// Check connection between 'source' (container) and 'dest' (ip)
bool net_3__check_ping(const char *source, const char *dest) {
    char command[256];
    snprintf(command, sizeof(command), "docker exec %s ping -c 2 -W 1 %s", source, dest);
    return generic__run_command(command);
}


bool net_3__main(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);
    
    // Check if all containers are running
    if (!generic__docker_container_running("node-a") ||
            !generic__docker_container_running("node-b") ||
            !generic__docker_container_running("node-c")) {
        lchk_add_feedback(result, "Fail: Not all containers are in running state");
        lchk_set_grade(result, 0);
        return false;
    }

    // Unblock ICMP as it's suggested to do in the task.
    // Probably it will be reasonable to check return value here but who cares.
    generic__run_command("docker exec node-b iptables -F");

    // Check basic connectivity (as well as running state of containers)
    if (!net_3__check_ping("node-a", "10.0.1.1") ||
            !net_3__check_ping("node-b", "10.0.1.10") ||
            !net_3__check_ping("node-b", "10.0.2.20") ||
            !net_3__check_ping("node-c", "10.0.2.1")) {
        
        lchk_add_feedback(result, "Fail: Basic connectivity is broken, not all nodes are reachable from each other");
        lchk_set_grade(result, 0);
        return false;
    }

    // Check IP forwarding
    if (!generic__check_command("docker exec node-b cat /proc/sys/net/ipv4/ip_forward", "1")) {
        lchk_add_feedback(result, "Fail: IP forwarding is disabled");
    }

    // Check routes
    if (!generic__check_command("docker exec node-a ip route", "default via 10.0.1.1") || 
            !generic__check_command("docker exec node-a ip route", "10.0.2.0/24 via 10.0.1.1")) {
        lchk_add_feedback(result, "Fail: Routing is misconfigured");
    }

    // Final connectivity
    if (!net_3__check_ping("node-a", "10.0.2.20")) {
        lchk_add_feedback(result, "Fail: 'node-c' is unreachable from 'node-a'");
    }

    // Routing chain test.
    // Need to block ICMP on 'node-b' for this.
    generic__run_command("docker exec node-b iptables -F");
    generic__run_command("docker exec node-b iptables -A FORWARD -p icmp -j DROP");
    generic__run_command("docker exec node-b iptables -A OUTPUT -p icmp -j DROP");
    if (net_3__check_ping("node-a", "10.0.2.20")) {
        lchk_add_feedback(result, "Fail: Routing chain is not what this task requires");
    }
    // Unblock ICMP anyways
    generic__run_command("docker exec node-b iptables -F");
    
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

LCHK_REGISTER_MODULE(net_3, "net-3", "2_Networks/net-3", "1.0", net_3__main)


// ===================================================
//      __ _                        _ _       _ 
//     / _(_)_ __ _____      ____ _| | |     / |
//    | |_| | '__/ _ \ \ /\ / / _` | | |_____| |
//    |  _| | | |  __/\ V  V / (_| | | |_____| |
//    |_| |_|_|  \___| \_/\_/ \__,_|_|_|     |_|
// ===================================================                                           

#ifndef firewall_1__PATH 
#define firewall_1__PATH            HOME "firewall-1/"
#endif // firewall_1__PATH

#define firewall_1__iptables        "docker exec firewall iptables -S 2>/dev/null"
#define firewall_1__iptables_nat    "docker exec firewall iptables -t nat -S 2>/dev/null"
#define firewall_1__iptables_raw    "docker exec firewall iptables -t raw -S 2>/dev/null"

// This task is really hard to do in docker containers.
// It's probably impossible to configure firewall without doing a mess on host.
// That being said, we should check only iptables rules, and not run actual tests.
// Iptables on 'firewall' container provide quite comprehensive info, whether this task is done correctly.

bool firewall_1__main(Lchk_Options *options, Lchk_Result *result, void *config) {
    UNUSED(options); UNUSED(config);
   
    // Check if all containers are running
    if (!generic__docker_container_running("firewall") ||
            !generic__docker_container_running("attacker") ||
            !generic__docker_container_running("drone")) {
        lchk_add_feedback(result, "Fail: Not all containers are in running state");
        lchk_set_grade(result, 0);
        return false;
    } 

    // Just checking iptables configuration...
    if (!generic__check_command(firewall_1__iptables, "-P INPUT DROP") ||
            !generic__check_command(firewall_1__iptables, "-P FORWARD DROP") ||
            !generic__check_command(firewall_1__iptables, "-P OUTPUT ACCEPT") ||
            !generic__check_command(firewall_1__iptables, "-A INPUT -i lo -j ACCEPT")) {
        lchk_add_feedback(result, "Fail: Common policies for built-in chains are incorrect");
    }
    
    if (!generic__check_command_fuzzy(firewall_1__iptables, "-A INPUT -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT", 25) ||
            !generic__check_command_fuzzy(firewall_1__iptables, "-A FORWARD -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT", 25)) {
        lchk_add_feedback(result, "Fail: Responding to already established connections is not allowed");
    }

    if (!generic__check_command_fuzzy(firewall_1__iptables, "-A INPUT -s 10.66.66.0/24 -j ACCEPT", 3) ||
            !generic__check_command_fuzzy(firewall_1__iptables, "-A INPUT -s 10.10.10.2/32 -d 10.10.10.1/32 -p tcp -m tcp --dport 22 -j DROP", 10)) {
        lchk_add_feedback(result, "Fail: Allowing connections from trusted subnet and/or blocking SSH from 'attacker' is not configured");
    }

    if (!generic__check_command(firewall_1__iptables, "-A INPUT -i eth0 -p icmp -j DROP") &&
            !generic__check_command_fuzzy(firewall_1__iptables_raw, "-A PREROUTING -d 10.10.10.1/32 -p icmp -j DROP", 20)) {
        lchk_add_feedback(result, "Fail: ICMP traffic incoming on 'public_net' is not blocked");
    }

    if (!generic__check_command_fuzzy(firewall_1__iptables, "-A FORWARD -s 192.168.99.0/24 -o eth0 -j ACCEPT", 20) ||
            !generic__check_command_fuzzy(firewall_1__iptables, "-A FORWARD -d 192.168.99.0/24 -m conntrack --ctstate RELATED,ESTABLISHED", 20)) {
        lchk_add_feedback(result, "Fail: Connections from 'private_net' to 'public_net' are not allowed");
    } 

    if (!generic__check_command_fuzzy(firewall_1__iptables, "-A FORWARD -d 192.168.99.10/32 -p tcp -m tcp --dport 22 -j ACCEPT", 10)) {
        lchk_add_feedback(result, "Fail: Forwardind TCP-packets to 'drone':22 is not allowed");
    }
    
    if (!generic__check_command(firewall_1__iptables_nat, "-P PREROUTING ACCEPT") ||
            !generic__check_command(firewall_1__iptables_nat, "-P INPUT ACCEPT") ||
            !generic__check_command(firewall_1__iptables_nat, "-P OUTPUT ACCEPT") ||
            !generic__check_command(firewall_1__iptables_nat, "-P POSTROUTING ACCEPT")) {
        lchk_add_feedback(result, "Fail: Common policies for NAT are incorrect");
    }

    if (!generic__check_command_fuzzy(firewall_1__iptables_nat, "-A POSTROUTING -s 192.168.99.0/24 -o eth0 -j MASQUERADE", 10)) {
        lchk_add_feedback(result, "Fail: SNAT (MASQUERADE) is not configured");
    }

    if (!generic__check_command_fuzzy(firewall_1__iptables_nat, "-A PREROUTING -d 10.10.10.1/32 -i eth0 -p tcp -m tcp --dport 2222 -j DNAT --to-destination 192.168.99.10:22", 10)) {
        lchk_add_feedback(result, "Fail: DNAT is not configured");
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

LCHK_REGISTER_MODULE(firewall_1, "firewall-1", "2_Networks/firewall-1", "1.0", firewall_1__main)

