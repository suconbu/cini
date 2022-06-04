//
// Copyright (c) 2016-2022 D.Miwa.
// This software is released under the MIT License, see LICENSE.
//

#pragma once

//
// API for C
//

#if defined(__cplusplus)
extern "C" {
#endif

typedef void* HCINI;

// Parse ini file and associate it to HCINI handle
// cini_create - Parse the all of ini file
// cini_create_with_section - Parse the specified section in ini file, it is faster than cini_create
HCINI cini_create(const char* path);
HCINI cini_create_with_section(const char* path, const char* section);

// Release resources
void cini_free(HCINI hcini);

// Get the value of indicated section and key
// The function returns default value if could not find the entry or the value type was mismatch
int cini_geti(HCINI hcini, const char* section, const char* key, int idefault);
float cini_getf(HCINI hcini, const char* section, const char* key, float fdefault);
const char* cini_gets(HCINI hcini, const char* section, const char* key, const char* sdefault);

// Array accessors
int cini_getai(HCINI hcini, const char* section, const char* key, int index_, int idefault);
float cini_getaf(HCINI hcini, const char* section, const char* key, int index_, float fdefault);
const char* cini_getas(HCINI hcini, const char* section, const char* key, int index_, const char* sdefault);

// Get number of array elements
int cini_getcount(HCINI hcini, const char* section, const char* key);

// Get error information which recorded when parsing ini file
int cini_geterrorcount(HCINI hcini);
const char* cini_geterror(HCINI hcini, int index_);

#if defined(__cplusplus)
} // extern "C"
#endif

#if defined(__cplusplus)

//
// API for C++
//

class Cini {
public:
    // Parse ini file and associate it to Cini instance
    // If the 'section' is not null, the cini parse specific section only
    Cini(const char* path, const char* section = nullptr) { hcini_ = cini_create_with_section(path, section); }
    ~Cini() { cini_free(hcini_); }

    // Get the value of indicated section and key
    // The function returns default value if could not find the entry or the value type was mismatch
    int geti(const char* section, const char* key, int idefault = 0) const { return cini_geti(hcini_, section, key, idefault); }
    float getf(const char* section, const char* key, float fdefault = 0.0F) const { return cini_getf(hcini_, section, key, fdefault); }
    const char* gets(const char* section, const char* key, const char* sdefault = "") const { return cini_gets(hcini_, section, key, sdefault); }

    // Array accessors
    int getai(const char* section, const char* key, int index_, int idefault = 0) const { return cini_getai(hcini_, section, key, index_, idefault); }
    float getaf(const char* section, const char* key, int index_, float fdefault = 0.0F) const { return cini_getaf(hcini_, section, key, index_, fdefault); }
    const char* getas(const char* section, const char* key, int index_, const char* sdefault = "") const { return cini_getas(hcini_, section, key, index_, sdefault); }

    // Get number of array elements
    int getcount(const char* section, const char* key) const { return cini_getcount(hcini_, section, key); }

    // Get error information which recorded when parsing ini file
    int geterrorcount() const { return cini_geterrorcount(hcini_); }
    const char* geterror(int index_) const { return cini_geterror(hcini_, index_); }

private:
    HCINI hcini_;

    // Non-copiable
    Cini(const Cini&);
    Cini& operator=(const Cini&);
};

#endif // __cplusplus

#if defined(CINI_IMPLEMENTATION)

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef CINI_MEMORY_CHUNK_SIZE
#define CINI_MEMORY_CHUNK_SIZE 2048
#endif

#ifndef CINI_LINE_BUFFER_SIZE
#define CINI_LINE_BUFFER_SIZE 512
#endif

#ifndef CINI_MALLOC
#define CINI_MALLOC(size) malloc(size)
#endif

#ifndef CINI_FREE
#define CINI_FREE(ptr) free(ptr)
#endif

#define CINI_IN_ERROR_BUFFER_SIZE     128
#define CINI_IN_QUOTE_CHARS           "'\""
#define CINI_IN_COMMENT_CHARS         ";#"
#define CINI_IN_ASSIGNMENT_CHARS      "=:"
#define CINI_IN_ARRAY_SEPARATOR       ','
#define CINI_IN_SECTION_BRACKET_OPEN  '['
#define CINI_IN_SECTION_BRACKET_CLOSE ']'

typedef struct {
    const char* begin;
    const char* end;
} CINI_IN_STRING;

typedef struct CINI_IN_LIST_NODE_ {
    struct CINI_IN_LIST_NODE_* next;
} CINI_IN_LIST_NODE;

typedef struct {
    CINI_IN_LIST_NODE node;
    char chunk[CINI_MEMORY_CHUNK_SIZE];
    char* ptr;
} CINI_IN_MEMORY;

typedef struct {
    CINI_IN_LIST_NODE node;
    const char* message;
} CINI_IN_ERROR;

typedef struct {
    CINI_IN_LIST_NODE* front;
    CINI_IN_LIST_NODE* back;
} CINI_IN_LIST;

typedef struct {
    CINI_IN_LIST_NODE node;
    double f;
    const char* s;
} CINI_IN_VALUE;

typedef struct {
    CINI_IN_LIST_NODE node;
    CINI_IN_LIST value_list;
    const char* name;
} CINI_IN_ENTRY;

typedef struct {
    CINI_IN_LIST_NODE node;
    CINI_IN_LIST entry_list;
    const char* name;
} CINI_IN_SECTION;

typedef struct {
    const char* target_section_name;
    CINI_IN_LIST memory_list;
    CINI_IN_LIST error_list;
    CINI_IN_LIST section_list;
    CINI_IN_SECTION* current_section;
    char error_buffer[CINI_IN_ERROR_BUFFER_SIZE];
    char line_buffer[CINI_LINE_BUFFER_SIZE];
    int line_no;
} CINI_IN_HANDLE;

static void* cini_in_allocate(CINI_IN_LIST* memory_list, size_t size);
static void cini_in_error(CINI_IN_HANDLE* cini, const char* message);

static FILE* cini_in_fopen(const char* filename, const char* mode)
{
    FILE* file = NULL;
#if defined(_MSC_VER) && _MSC_VER >= 1400
    (void)fopen_s(&file, filename, mode);
#else
    file = fopen(filename, mode);
#endif
    return file;
}

static int cini_in_isspace(char c)
{
    return isspace((unsigned char)c);
}

static const char* cini_in_skip_bom(const char* str)
{
    return ((unsigned char)str[0] == 0xEFu && (unsigned char)str[1] == 0xBBu && (unsigned char)str[2] == 0xBFu) ? (str + 3) : str;
}

static size_t cini_in_string_len(CINI_IN_STRING* str)
{
    return (str != NULL && str->begin != NULL && str->end != NULL && (str->begin <= str->end)) ? (str->end - str->begin) : 0;
}

static CINI_IN_STRING cini_in_string_trim(CINI_IN_STRING* str)
{
    CINI_IN_STRING s = { NULL, NULL };
    if (str != NULL && str->begin != NULL && str->end != NULL) {
        for (s.begin = str->begin; s.begin < str->end && cini_in_isspace(*s.begin); ++s.begin) { }
        for (s.end = str->end - 1; str->begin <= s.end && cini_in_isspace(*s.end); --s.end) { }
        ++s.end;
    }
    return s;
}

static int cini_in_list_count(const CINI_IN_LIST* list)
{
    int count = 0;
    CINI_IN_LIST_NODE* node = (list != NULL) ? list->front : NULL;
    while (node != NULL) {
        node = node->next;
        count += 1;
    }
    return count;
}

static CINI_IN_LIST_NODE* cini_in_list_push_back(CINI_IN_HANDLE* cini, CINI_IN_LIST* list, size_t size)
{
    CINI_IN_LIST_NODE* node = (CINI_IN_LIST_NODE*)cini_in_allocate(&cini->memory_list, size);
    if (node != NULL) {
        memset(node, 0, size);
        if (list->front == NULL) {
            list->front = node;
        } else {
            list->back->next = node;
        }
        list->back = node;
    } else {
        cini_in_error(cini, "Failed to allocate memory");
    }
    return node;
}

static CINI_IN_LIST_NODE* cini_in_list_at(const CINI_IN_LIST* list, int index_)
{
    CINI_IN_LIST_NODE* found_node = NULL;
    int i = 0;
    for (CINI_IN_LIST_NODE* node = list->front; node != NULL; node = node->next, ++i) {
        if (i == index_) {
            found_node = node;
            break;
        }
    }
    return found_node;
}

static CINI_IN_LIST_NODE* cini_in_list_find(const CINI_IN_LIST* list, int (*match_func)(CINI_IN_LIST_NODE*, const void*), const void* data)
{
    CINI_IN_LIST_NODE* found_node = NULL;
    for (CINI_IN_LIST_NODE* node = list->front; node != NULL; node = node->next) {
        if (match_func(node, data)) {
            found_node = node;
            break;
        }
    }
    return found_node;
}

static void cini_in_error(CINI_IN_HANDLE* cini, const char* message)
{
    int len = snprintf(cini->error_buffer, sizeof(cini->error_buffer), "%s (line:%d)", message, cini->line_no);
    if (0 < len) {
        size_t size = sizeof(CINI_IN_ERROR) + len + 1;
        CINI_IN_ERROR* error = (CINI_IN_ERROR*)cini_in_list_push_back(cini, &cini->error_list, size);
        if (error != NULL) {
            char* s = (char*)(error + 1);
            memcpy(s, cini->error_buffer, len);
            error->message = s;
        }
    }
}

static void* cini_in_allocate(CINI_IN_LIST* memory_list, size_t size)
{
    char* ptr = NULL;
    if (memory_list != NULL && size <= CINI_MEMORY_CHUNK_SIZE) {
        CINI_IN_MEMORY* memory = (CINI_IN_MEMORY*)memory_list->back;
        size_t remain = 0;
        if (memory != NULL) {
            remain = CINI_MEMORY_CHUNK_SIZE - (memory->ptr - memory->chunk);
        }
        if (remain < size) {
            CINI_IN_LIST_NODE* new_node = (CINI_IN_LIST_NODE*)CINI_MALLOC(sizeof(CINI_IN_MEMORY));
            if (new_node != NULL) {
                memset(new_node, 0, sizeof(CINI_IN_MEMORY));
                if (memory_list->front == NULL) {
                    memory_list->front = new_node;
                } else {
                    memory_list->back->next = new_node;
                }
                memory_list->back = new_node;
                memory = (CINI_IN_MEMORY*)new_node;
                memory->ptr = memory->chunk;
            }
        }

        if (memory != NULL) {
            ptr = memory->ptr;
            memory->ptr += size;
        }
    }
    return ptr;
}

static CINI_IN_VALUE* cini_in_add_value_single(CINI_IN_HANDLE* cini, CINI_IN_LIST* value_list, CINI_IN_STRING* source)
{
    CINI_IN_STRING str = cini_in_string_trim(source);
    const char* str_ptr = str.begin;
    int negative = 0;
    double numeric = NAN;
    if (*str_ptr == '+' || *str_ptr == '-') {
        negative = (*str_ptr == '-') ? 1 : 0;
        ++str_ptr;
    }
    if (str_ptr != str.end) {
        char* endp = NULL;
        double n = NAN;
        errno = 0;
        if (*str_ptr == '#') {
            if ((str_ptr + 1) != str.end) {
                n = (double)strtoll(str_ptr + 1, &endp, 16);
            }
        } else if (*str_ptr == '0' && (*(str_ptr + 1) == 'X' || *(str_ptr + 1) == 'x')) {
            if ((str_ptr + 2) != str.end) {
                n = (double)strtoll(str_ptr + 2, &endp, 16);
            }
        } else {
            n = strtod(str_ptr, &endp);
        }
        if (endp == str.end) {
            if (errno != ERANGE && -FLT_MAX <= n && n <= FLT_MAX) {
                numeric = n;
            } else {
                // Out of range
            }
        }
    }

    if (!isnan(numeric)) {
        // Numeric:
        numeric = negative ? -numeric : numeric;
    } else {
        // String: Remove the quote mark of both ends
        if (2 <= cini_in_string_len(&str) && strchr(CINI_IN_QUOTE_CHARS, *str.begin) != NULL && *str.begin == *(str.end - 1)) {
            str.begin += 1;
            str.end -= 1;
        }
    }

    size_t len = cini_in_string_len(&str);
    size_t size = sizeof(CINI_IN_VALUE) + len + 1;
    CINI_IN_VALUE* value = (CINI_IN_VALUE*)cini_in_list_push_back(cini, value_list, size);
    if (value != NULL) {
        char* s = (char*)(value + 1);
        memcpy(s, str.begin, len);
        value->s = s;
        value->f = numeric;
    }

    return value;
}

static void cini_in_add_value_array(CINI_IN_HANDLE* cini, CINI_IN_LIST* value_list, CINI_IN_STRING* source)
{
    const char* str_ptr = source->begin;
    while (str_ptr <= source->end) {
        // It is not "str_ptr < source_end" because pick up the empty string on end of line.
        // ex. key = 1,2,3, <<< number of elements is 4. (1, 2, 3 and "")
        int quoteChar = 0;
        int quoteOpen = 0;
        const char* separator = NULL;
        CINI_IN_STRING value_str = { str_ptr, source->end };
        for (; str_ptr < source->end; ++str_ptr) {
            if (!cini_in_isspace(*str_ptr)) {
                if (strchr(CINI_IN_QUOTE_CHARS, *str_ptr) != NULL) {
                    quoteChar = *str_ptr++;
                    quoteOpen = 1;
                }
                break;
            }
        }

        for (; str_ptr < source->end; ++str_ptr) {
            if (cini_in_isspace(*str_ptr)) {
                // Skip
            } else if (*str_ptr == CINI_IN_ARRAY_SEPARATOR) {
                if (quoteOpen) {
                    if (separator == NULL) {
                        separator = str_ptr;
                    }
                } else {
                    value_str.end = str_ptr;
                    break;
                }
            } else {
                if (quoteChar) {
                    quoteOpen = (*str_ptr != quoteChar);
                }
            }
        }

        if (quoteOpen && separator != NULL) {
            value_str.end = separator;
        }

        cini_in_add_value_single(cini, value_list, &value_str);

        str_ptr = value_str.end + 1;
    }
    return;
}

static void cini_in_add_value(CINI_IN_HANDLE* cini, CINI_IN_LIST* value_list, CINI_IN_STRING* source)
{
    cini_in_add_value_single(cini, value_list, source);
    cini_in_add_value_array(cini, value_list, source);
    return;
}

static CINI_IN_ENTRY* cini_in_add_entry(CINI_IN_HANDLE* cini, CINI_IN_LIST* entry_list, CINI_IN_STRING* name)
{
    size_t len = cini_in_string_len(name);
    size_t size = sizeof(CINI_IN_ENTRY) + len + 1;
    CINI_IN_ENTRY* entry = (CINI_IN_ENTRY*)cini_in_list_push_back(cini, entry_list, size);
    if (entry != NULL) {
        char* s = (char*)(entry + 1);
        memcpy(s, name->begin, len);
        entry->name = s;
    }
    return entry;
}

static CINI_IN_SECTION* cini_in_add_section(CINI_IN_HANDLE* cini, CINI_IN_LIST* section_list, CINI_IN_STRING* name)
{
    size_t len = cini_in_string_len(name);
    size_t size = sizeof(CINI_IN_SECTION) + len + 1;
    CINI_IN_SECTION* section = (CINI_IN_SECTION*)cini_in_list_push_back(cini, section_list, size);
    if (section != NULL) {
        char* s = (char*)(section + 1);
        memcpy(s, name->begin, len);
        section->name = s;
    }
    return section;
}

static int cini_in_match_entry(CINI_IN_LIST_NODE* node, const void* data)
{
    return strncmp(((CINI_IN_STRING*)data)->begin, ((CINI_IN_ENTRY*)node)->name, cini_in_string_len((CINI_IN_STRING*)data)) == 0;
}

static int cini_in_match_section(CINI_IN_LIST_NODE* node, const void* data)
{
    return strncmp(((CINI_IN_STRING*)data)->begin, ((CINI_IN_SECTION*)node)->name, cini_in_string_len((CINI_IN_STRING*)data)) == 0;
}

static CINI_IN_ENTRY* cini_in_get_entry(CINI_IN_HANDLE* cini, const char* section_name, const char* key_name)
{
    CINI_IN_STRING section_name_str = { section_name, section_name + strlen(section_name) };
    CINI_IN_STRING key_name_str = { key_name, key_name + strlen(key_name) };
    CINI_IN_SECTION* section = (CINI_IN_SECTION*)cini_in_list_find(&cini->section_list, cini_in_match_section, &section_name_str);
    return (section) ? (CINI_IN_ENTRY*)cini_in_list_find(&section->entry_list, cini_in_match_entry, &key_name_str) : NULL;
}

static CINI_IN_VALUE* cini_in_get_value(CINI_IN_HANDLE* cini, const char* section_name, const char* key_name, int index_)
{
    CINI_IN_ENTRY* entry = cini_in_get_entry(cini, section_name, key_name);
    return (entry) ? (CINI_IN_VALUE*)cini_in_list_at(&entry->value_list, index_) : NULL;
}

static void cini_in_parse(CINI_IN_HANDLE* cini, FILE* file)
{
    // Default section
    const char* default_name = "";
    CINI_IN_STRING default_name_str = { default_name, default_name + strlen(default_name) };
    cini->current_section = cini_in_add_section(cini, &cini->section_list, &default_name_str);

    while (fgets(cini->line_buffer, sizeof(cini->line_buffer), file) != NULL) {
        ++cini->line_no;
        size_t line_len = strlen(cini->line_buffer);
        CINI_IN_STRING line = { cini->line_buffer, cini->line_buffer + line_len };
        if (cini->line_no == 1 && 3 <= line_len) {
            line.begin = cini_in_skip_bom(line.begin);
        }
        line = cini_in_string_trim(&line);
        if (strchr(CINI_IN_COMMENT_CHARS, *line.begin) != NULL) {
            continue;
        }
        if (*line.begin == CINI_IN_SECTION_BRACKET_OPEN) {
            ++line.begin;
            CINI_IN_STRING section_name = { line.begin, strchr(line.begin, CINI_IN_SECTION_BRACKET_CLOSE) };
            if (cini_in_string_len(&section_name) == 0) {
                cini_in_error(cini, "Invalid section name");
                continue;
            }
            CINI_IN_SECTION* existing_section = (CINI_IN_SECTION*)cini_in_list_find(&cini->section_list, cini_in_match_section, &section_name);
            if (existing_section == NULL) {
                cini->current_section = cini_in_add_section(cini, &cini->section_list, &section_name);
            } else {
                cini->current_section = existing_section;
            }
        } else {
            if (cini->current_section != NULL && (cini->target_section_name == NULL || strcmp(cini->current_section->name, cini->target_section_name) == 0)) {
                CINI_IN_STRING key_name = { line.begin, line.begin };
                for (; *key_name.end != 0; ++key_name.end) {
                    if (strchr(CINI_IN_ASSIGNMENT_CHARS, *key_name.end) != NULL) {
                        break;
                    }
                }
                CINI_IN_STRING value_str = { key_name.end + 1, line.end };
                key_name = cini_in_string_trim(&key_name);
                if (cini_in_string_len(&key_name) == 0) {
                    cini_in_error(cini, "Invalid key name");
                    continue;
                }
                CINI_IN_ENTRY* entry = (CINI_IN_ENTRY*)cini_in_list_find(&cini->current_section->entry_list, cini_in_match_entry, &key_name);
                if (entry == NULL) {
                    entry = cini_in_add_entry(cini, &cini->current_section->entry_list, &key_name);
                    if (entry == NULL) {
                        continue;
                    }
                    cini_in_add_value(cini, &entry->value_list, &value_str);
                }
            }
        }
    }
}

HCINI cini_in_create_handle(const char* path, const char* section)
{
    CINI_IN_LIST memory_list = { NULL, NULL };
    CINI_IN_HANDLE* cini = (CINI_IN_HANDLE*)cini_in_allocate(&memory_list, sizeof(CINI_IN_HANDLE));
    if (cini != NULL) {
        cini->memory_list = memory_list;
        cini->target_section_name = section;
        FILE* file = cini_in_fopen(path, "r");
        if (file != NULL) {
            cini_in_parse(cini, file);
            fclose(file);
            file = NULL;
        } else {
            cini_in_error(cini, "Cannot open file");
        }
    }
    return (HCINI)cini;
}

void cini_in_free_handle(HCINI hcini)
{
    CINI_IN_HANDLE* cini = (CINI_IN_HANDLE*)hcini;
    if (cini != NULL) {
        CINI_IN_LIST memory_list = cini->memory_list;
        CINI_IN_LIST_NODE* node = (CINI_IN_LIST_NODE*)memory_list.front;
        while (node != NULL) {
            CINI_IN_LIST_NODE* next = node->next;
            memset(node, 0, sizeof(CINI_IN_MEMORY));
            CINI_FREE(node);
            node = next;
        }
    }
}

int cini_in_check_int(double f, int idefault)
{
    return (INT32_MIN <= f && f <= INT32_MAX) ? (int)f : idefault;
}

////////////////////////////////////////////////////////////////////////////////

HCINI cini_create(const char* path)
{
    return cini_in_create_handle(path, NULL);
}

HCINI cini_create_with_section(const char* path, const char* section)
{
    return cini_in_create_handle(path, section);
}

void cini_free(HCINI hcini)
{
    cini_in_free_handle(hcini);
}

int cini_geti(HCINI hcini, const char* section, const char* key, int idefault)
{
    const CINI_IN_VALUE* value = cini_in_get_value((CINI_IN_HANDLE*)hcini, section, key, 0);
    return (value && !isnan(value->f)) ? cini_in_check_int(value->f, idefault) : idefault;
}

float cini_getf(HCINI hcini, const char* section, const char* key, float fdefault)
{
    const CINI_IN_VALUE* value = cini_in_get_value((CINI_IN_HANDLE*)hcini, section, key, 0);
    return (value && !isnan(value->f)) ? (float)value->f : fdefault;
}

const char* cini_gets(HCINI hcini, const char* section, const char* key, const char* sdefault)
{
    const CINI_IN_VALUE* value = cini_in_get_value((CINI_IN_HANDLE*)hcini, section, key, 0);
    return (value && value->s) ? value->s : sdefault;
}

int cini_getai(HCINI hcini, const char* section, const char* key, int index_, int idefault)
{
    const CINI_IN_VALUE* value = (0 <= index_) ? cini_in_get_value((CINI_IN_HANDLE*)hcini, section, key, index_ + 1) : NULL;
    return (value && !isnan(value->f)) ? cini_in_check_int(value->f, idefault) : idefault;
}
float cini_getaf(HCINI hcini, const char* section, const char* key, int index_, float fdefault)
{
    const CINI_IN_VALUE* value = (0 <= index_) ? cini_in_get_value((CINI_IN_HANDLE*)hcini, section, key, index_ + 1) : NULL;
    return (value && !isnan(value->f)) ? (float)value->f : fdefault;
}
const char* cini_getas(HCINI hcini, const char* section, const char* key, int index_, const char* sdefault)
{
    const CINI_IN_VALUE* value = (0 <= index_) ? cini_in_get_value((CINI_IN_HANDLE*)hcini, section, key, index_ + 1) : NULL;
    return (value && value->s) ? value->s : sdefault;
}

int cini_getcount(HCINI hcini, const char* section, const char* key)
{
    const CINI_IN_ENTRY* entry = cini_in_get_entry((CINI_IN_HANDLE*)hcini, section, key);
    return (entry) ? (cini_in_list_count(&entry->value_list) - 1) : 0;
}

int cini_geterrorcount(HCINI hcini)
{
    const CINI_IN_HANDLE* cini = (CINI_IN_HANDLE*)hcini;
    return (cini != NULL) ? cini_in_list_count(&cini->error_list) : 0;
}

const char* cini_geterror(HCINI hcini, int index_)
{
    const CINI_IN_HANDLE* cini = (CINI_IN_HANDLE*)hcini;
    const char* message = NULL;
    if (cini != NULL) {
        const CINI_IN_ERROR* error = (CINI_IN_ERROR*)cini_in_list_at(&cini->error_list, index_);
        if (error) {
            message = error->message;
        }
    }
    return message;
}

#endif // CINI_IMPLEMENTATION
