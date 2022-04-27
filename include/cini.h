//
// cini.h - Convenient ini parser for C/C++
//
// Copyright (C) 2016-2022 suconbu.
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

#pragma once

#ifndef CINI_H_
#define CINI_H_

//
// API for C
//

typedef void* HCINI;

// Parse ini file and associate it to HCINI handle
// cini_create - Parse the all of ini file
// cini_create_with_section - Parse the specified section in ini file, it is faster than cini_create
HCINI cini_create(const char* path);
HCINI cini_create_with_section(const char* path, const char* section);

// Release resources
void cini_free(HCINI hcini);

// If failed to load the ini file, the function returns non zero
int cini_isfailed(HCINI hcini);

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

#if defined(CINI_IMPLEMENTATION)

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CINI_MEMORY_BLOCK_SIZE 2048
#define CINI_LINE_BUFFER_SIZE  512
#define CINI_ERROR_BUFFER_SIZE 128

#define CINI_IN_QUOTE_CHARS           "'\""
#define CINI_IN_COMMENT_CHARS         ";#"
#define CINI_IN_VALUE_SEPARATOR       ','
#define CINI_IN_ASSIGNMENT            '='
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
    char block[CINI_MEMORY_BLOCK_SIZE];
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
    char error_buffer[CINI_ERROR_BUFFER_SIZE];
    char line_buffer[CINI_LINE_BUFFER_SIZE];
    int line_no;
    int good;
} CINI_IN_HANDLE;

void* cini_in_allocate(CINI_IN_HANDLE* cini, size_t size);

FILE* cini_in_fopen(const char* filename, const char* mode)
{
    FILE* file = NULL;
#if defined(_MSC_VER) && _MSC_VER >= 1400
    (void)fopen_s(&file, filename, mode);
#else
    file = fopen(filename, mode);
#endif
    return file;
}

int cini_in_isspace(char c)
{
    return isspace((unsigned char)c);
}

const char* cini_in_skip_bom(const char* str)
{
    return ((unsigned char)str[0] == 0xEFu && (unsigned char)str[1] == 0xBBu && (unsigned char)str[2] == 0xBFu) ? (str + 3) : str;
}

CINI_IN_STRING cini_in_trim_left(CINI_IN_STRING* str)
{
    CINI_IN_STRING s = {};
    if (str != NULL && str->begin != NULL && str->end != NULL) {
        for (s.begin = str->begin, s.end = str->end; cini_in_isspace(*s.begin); ++s.begin) { }
    }
    return s;
}

CINI_IN_STRING cini_in_trim_right(CINI_IN_STRING* str)
{
    CINI_IN_STRING s = {};
    if (str != NULL && str->begin != NULL && str->end != NULL) {
        for (s.begin = str->begin, s.end = str->end - 1; str->begin <= s.end && cini_in_isspace(*s.end); --s.end) { }
        ++s.end;
    }
    return s;
}

CINI_IN_STRING cini_in_string_make(const char* source)
{
    CINI_IN_STRING str = {};
    if (source != NULL) {
        str.begin = source;
        str.end = source + strlen(source);
    }
    return str;
}

size_t cini_in_string_len(CINI_IN_STRING* str)
{
    return (str != NULL && str->begin != NULL && str->end != NULL && (str->begin <= str->end)) ? (str->end - str->begin) : 0;
}

int cini_in_string_compare(CINI_IN_STRING* str1, const char* str2)
{
    return strncmp(str1->begin, str2, cini_in_string_len(str1));
}

int cini_in_list_count(const CINI_IN_LIST* list)
{
    int count = 0;
    CINI_IN_LIST_NODE* node = (list != NULL) ? list->front : NULL;
    while (node != NULL) {
        node = node->next;
        count += 1;
    }
    return count;
}

CINI_IN_LIST_NODE* cini_in_list_push_back(CINI_IN_HANDLE* cini, CINI_IN_LIST* list, size_t size)
{
    CINI_IN_LIST_NODE* node = (CINI_IN_LIST_NODE*)cini_in_allocate(cini, size);
    if (node != NULL) {
        memset(node, 0, size);
        if (list->front == NULL) {
            list->front = node;
        } else {
            list->back->next = node;
        }
        list->back = node;
    }
    return node;
}

CINI_IN_LIST_NODE* cini_in_list_at(const CINI_IN_LIST* list, int index_)
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

CINI_IN_LIST_NODE* cini_in_list_find(const CINI_IN_LIST* list, int (*match_func)(CINI_IN_LIST_NODE*, const void*), const void* data)
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

void cini_in_error(CINI_IN_HANDLE* cini)
{
    snprintf(cini->error_buffer, sizeof(cini->error_buffer), "at line:%d", cini->line_no);
    size_t len = strlen(cini->error_buffer);
    size_t size = sizeof(CINI_IN_ERROR) + len + 1;
    CINI_IN_ERROR* error = (CINI_IN_ERROR*)cini_in_list_push_back(cini, &cini->error_list, size);
    if (error != NULL) {
        char* s = (char*)(error + 1);
        memcpy(s, cini->error_buffer, len);
        error->message = s;
    }
    return;
}

void* cini_in_allocate(CINI_IN_HANDLE* cini, size_t size)
{
    char* ptr = NULL;
    if (size <= CINI_MEMORY_BLOCK_SIZE) {
        CINI_IN_MEMORY* memory = (CINI_IN_MEMORY*)cini->memory_list.back;
        size_t remain = 0;
        if (memory != NULL) {
            remain = CINI_MEMORY_BLOCK_SIZE - (memory->ptr - memory->block);
        }
        if (remain < size) {
            CINI_IN_LIST_NODE* new_node = (CINI_IN_LIST_NODE*)malloc(sizeof(CINI_IN_MEMORY));
            if (new_node != NULL) {
                memset(new_node, 0, sizeof(CINI_IN_MEMORY));
                if (cini->memory_list.front == NULL) {
                    cini->memory_list.front = new_node;
                } else {
                    cini->memory_list.back->next = new_node;
                }
                cini->memory_list.back = new_node;
                memory = (CINI_IN_MEMORY*)new_node;
                memory->ptr = memory->block;
            } else {
                cini_in_error(cini);
            }
        }

        if (memory != NULL) {
            ptr = memory->ptr;
            memory->ptr += size;
        } else {
            cini_in_error(cini);
        }
    } else {
        cini_in_error(cini);
    }
    return ptr;
}

CINI_IN_VALUE* cini_in_add_value_single(CINI_IN_HANDLE* cini, CINI_IN_LIST* value_list, CINI_IN_STRING* source)
{
    CINI_IN_STRING str = cini_in_trim_left(source);
    str = cini_in_trim_right(&str);
    int has_numeric = 0;
    double numeric = 0.0;
    if (*str.begin != 0 && strchr("+-#0123456789.", *str.begin) != NULL) {
        char* endp = NULL;
        const char* pos = strchr(str.begin, '#');
        if (pos != NULL) {
            numeric = strtol(pos + 1, &endp, 16);
        } else {
            numeric = strtol(str.begin, &endp, 0);
        }

        if (endp == str.end) {
            // Integer
            has_numeric = 1;
        } else {
            // Float
            numeric = strtod(str.begin, &endp);
            if (endp == str.end) {
                has_numeric = 1;
            }
        }
    }

    if (!has_numeric) {
        // Remove the quote mark of both ends
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
        value->f = has_numeric ? numeric : NAN;
    }

    return value;
}

void cini_in_add_value_array(CINI_IN_HANDLE* cini, CINI_IN_LIST* value_list, CINI_IN_STRING* source)
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
                    quoteChar = *str_ptr;
                    quoteOpen = 1;
                }
                ++str_ptr;
                break;
            }
        }

        for (; str_ptr < source->end; ++str_ptr) {
            if (cini_in_isspace(*str_ptr)) {
                // Skip
            } else if (*str_ptr == CINI_IN_VALUE_SEPARATOR) {
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

void cini_in_add_value(CINI_IN_HANDLE* cini, CINI_IN_LIST* value_list, CINI_IN_STRING* source)
{
    cini_in_add_value_single(cini, value_list, source);
    cini_in_add_value_array(cini, value_list, source);
    return;
}

CINI_IN_ENTRY* cini_in_add_entry(CINI_IN_HANDLE* cini, CINI_IN_LIST* entry_list, CINI_IN_STRING* name)
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

CINI_IN_SECTION* cini_in_add_section(CINI_IN_HANDLE* cini, CINI_IN_LIST* section_list, CINI_IN_STRING* name)
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

int cini_in_match_entry(CINI_IN_LIST_NODE* node, const void* data)
{
    return cini_in_string_compare((CINI_IN_STRING*)data, ((CINI_IN_ENTRY*)node)->name) == 0;
}

int cini_in_match_section(CINI_IN_LIST_NODE* node, const void* data)
{
    return cini_in_string_compare((CINI_IN_STRING*)data, ((CINI_IN_SECTION*)node)->name) == 0;
}

CINI_IN_ENTRY* cini_in_get_entry(CINI_IN_HANDLE* cini, const char* section_name, const char* key_name)
{
    CINI_IN_STRING section_name_str = cini_in_string_make(section_name);
    CINI_IN_STRING key_name_str = cini_in_string_make(key_name);
    CINI_IN_SECTION* section = (CINI_IN_SECTION*)cini_in_list_find(&cini->section_list, cini_in_match_section, &section_name_str);
    return (section) ? (CINI_IN_ENTRY*)cini_in_list_find(&section->entry_list, cini_in_match_entry, &key_name_str) : NULL;
}

CINI_IN_VALUE* cini_in_get_value(CINI_IN_HANDLE* cini, const char* section_name, const char* key_name, int index_)
{
    CINI_IN_ENTRY* entry = cini_in_get_entry(cini, section_name, key_name);
    return (entry) ? (CINI_IN_VALUE*)cini_in_list_at(&entry->value_list, index_) : NULL;
}

void cini_in_parse(CINI_IN_HANDLE* cini, FILE* file)
{
    // Default section
    CINI_IN_STRING empty_str = cini_in_string_make("");
    cini->current_section = cini_in_add_section(cini, &cini->section_list, &empty_str);

    while (fgets(cini->line_buffer, sizeof(cini->line_buffer), file) != NULL) {
        ++cini->line_no;
        CINI_IN_STRING line = cini_in_string_make(cini->line_buffer);
        size_t line_len = cini_in_string_len(&line);
        if (cini->line_no == 1 && 3 <= line_len) {
            line.begin = cini_in_skip_bom(line.begin);
        }
        line = cini_in_trim_left(&line);
        if (strchr(CINI_IN_COMMENT_CHARS, *line.begin) != NULL) {
            continue;
        }
        line = cini_in_trim_right(&line);
        if (*line.begin == CINI_IN_SECTION_BRACKET_OPEN) {
            ++line.begin;
            CINI_IN_STRING section_name = { line.begin, strchr(line.begin, CINI_IN_SECTION_BRACKET_CLOSE) };
            if (cini_in_string_len(&section_name) == 0) {
                cini_in_error(cini);
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
                CINI_IN_STRING key_name = { line.begin, strchr(line.begin, CINI_IN_ASSIGNMENT) };
                CINI_IN_STRING value_str = { key_name.end + 1, line.end };
                key_name = cini_in_trim_right(&key_name);
                if (cini_in_string_len(&key_name) == 0) {
                    cini_in_error(cini);
                    continue;
                }
                CINI_IN_ENTRY* entry = (CINI_IN_ENTRY*)cini_in_list_find(&cini->current_section->entry_list, cini_in_match_entry, &key_name);
                if (entry == NULL) {
                    entry = cini_in_add_entry(cini, &cini->current_section->entry_list, &key_name);
                    if (entry == NULL) {
                        cini_in_error(cini);
                        continue;
                    }
                    cini_in_add_value(cini, &entry->value_list, &value_str);
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

HCINI cini_create(const char* path)
{
    return cini_create_with_section(path, 0);
}

HCINI cini_create_with_section(const char* path, const char* section)
{
    CINI_IN_HANDLE* cini = (CINI_IN_HANDLE*)malloc(sizeof(CINI_IN_HANDLE));
    if (cini != NULL) {
        memset(cini, 0, sizeof(CINI_IN_HANDLE));
        cini->target_section_name = section;
        FILE* file = cini_in_fopen(path, "r");
        if (file != NULL) {
            cini_in_parse(cini, file);
            fclose(file);
            file = NULL;
            cini->good = 1;
        }
    }
    return (HCINI)cini;
}

void cini_free(HCINI hcini)
{
    CINI_IN_HANDLE* cini = (CINI_IN_HANDLE*)hcini;
    if (cini != NULL) {
        CINI_IN_LIST_NODE* node = (CINI_IN_LIST_NODE*)cini->memory_list.front;
        while (node != NULL) {
            CINI_IN_LIST_NODE* next = node->next;
            free(node);
            node = next;
        }
        free(cini);
    }
}

int cini_isfailed(HCINI hcini)
{
    CINI_IN_HANDLE* cini = (CINI_IN_HANDLE*)hcini;
    return (cini != NULL && cini->good) ? 0 : 1;
}

int cini_geti(HCINI hcini, const char* section, const char* key, int idefault)
{
    const CINI_IN_VALUE* value = cini_in_get_value((CINI_IN_HANDLE*)hcini, section, key, 0);
    return (value && !isnan(value->f)) ? (int)value->f : idefault;
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
    return (value && !isnan(value->f)) ? (int)value->f : idefault;
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

#endif // CINI_H_
