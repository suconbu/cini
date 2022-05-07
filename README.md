# cini - single header ini file parser for C/C++

Licensed under MIT License.

# Version

1.2.0

# Overview

* Single header
* No third-party dependency
* Supports array value

# Sample

```c
#define CINI_IMPLEMENTATION // This definition is required in only one source file
#include "cini.h"

void sample_for_c()
{
    HCINI hcini = cini_create("sample.ini");
    if (cini_isgood(hcini) == 0) {
        fprintf(stderr, "...");
    }
    int i = cini_geti(hcini, "section-name", "key-name", 0 /* default_value */);
    float f = cini_getf(hcini, "section-name", "key-name", 0.0f);
    const char* s = cini_gets(hcini, "section-name", "key-name", "default");
    // The pointer returned by cini_gets/cini_getas is valid until the handle is released.
    cini_free(hcini);
}

void sample_for_cpp()
{
    Cini cini("sample.ini");
    if (!cini) {
        std::cerr << "..." << std::endl;
    }
    int i = cini.geti("section-name", "key-name", 0 /* default_value */);
    float f = cini.getf("section-name", "key-name", 0.0);
    const char* s = cini.gets("section-name", "key-name", "default");
}
```

# Supported ini file format

* The new-line character is LF (0x0A) or CRLF (0x0D, 0x0A).
* Allow blank line.
* Lines beginning with "`;`" or "`#`" are considered comment lines.
* Case sensitive in key-name and section-name.
* Ignores space-character (0x09, 0x0A, 0x0B, 0x0C, 0x0D and 0x20) around key-name, section-name and value.
* Allow to write the entry on out of the section.
* If duplicate the key-name, first one will be use.
* If duplicate the section-name, merge the entries in each sections.

# Data types

The cini can read value as string or numeric.

## Data types - String

* Quotation marks ("`"`" or "`'`") are optional, but their use allows you to include whitespace at around of a string, or to include the array element delimiter "`,`" in a string.
* Numerical values can also be obtained as string.
* "`\`" is not recognized as an escape character.

ini file:
```
[String example]
str1 = One
str2 = 123
str3 = " Two, Three "
str4 = ' Four \t Five \n'
str5 = "Six, "Seven""
```

Code:
```
cini_gets(hcini, "String example", "str1", "default");      // "One"
cini_gets(hcini, "String example", "str2", "default");      // "123"
cini_gets(hcini, "String example", "str3", "default");      // " Two, Three "
cini_gets(hcini, "String example", "str4", "default");      // " Four \t Five \n"
cini_gets(hcini, "String example", "str5", "default");      // "Six, "Seven""
```

## Data types - Numeric

* The cini can use decimal and hexadecimal numbers.
* If an attempt is made to retrieve a value that cannot be interpreted as a number using a function for numeric (such as "cini_geti"), the attempt will fail and the default value specified in the argument will be returned.  

ini file:
```
;sample.ini

[Decimal example]
dec1 = 123
dec2 = -4.56
dec3 = .123
dec4 = 7.89e2
dec5 = One

[Hexadecimal example]
hex1 = 0xFF
hex2 = #FF
```

Code:
```c
cini_geti(hcini, "Decimal example", "dec1", -1);        // 123
cini_getf(hcini, "Decimal example", "dec1", -1.0f);     // 123.0
cini_geti(hcini, "Decimal example", "dec2", -1);        // -4
cini_getf(hcini, "Decimal example", "dec2", -1.0f);     // -4.55999994
cini_geti(hcini, "Decimal example", "dec3", -1);        // 0
cini_getf(hcini, "Decimal example", "dec3", -1.0f);     // 0.123000003
cini_geti(hcini, "Decimal example", "dec4", -1);        // 789
cini_getf(hcini, "Decimal example", "dec4", -1.0f);     // 789.0
cini_geti(hcini, "Decimal example", "dec5", -1);        // -1
cini_getf(hcini, "Decimal example", "dec5", -1.0f);     // -1.0
cini_geti(hcini, "Hexadecimal example", "hex1", -1);    // 255
cini_getf(hcini, "Hexadecimal example", "hex1", -1.0f); // 255.0
cini_geti(hcini, "Hexadecimal example", "hex2", -1);    // 255
cini_getf(hcini, "Hexadecimal example", "hex2", -1.0f); // 255.0
```

# Array

The cini recognizes comma-separated values as an array, and values can be retrieved by specifying an index.

* You can mix different data types in a single array
* To include "`,`" in a string, enclose both ends of the string with "`"`" or "`'`".

ini file:
```
[Array example]
array1 = 1, 0x2, #3, 4.56, Seven
array2 = One,"Two,Three",'Four,Five'
```

Code:
```c
cini_getcount(hcini, "Array example", "array1");            // 5
cini_getai(hcini, "Array example", "array1", 0, -1)         // 1
cini_getai(hcini, "Array example", "array1", 4, -1)         // -1
cini_gets(hcini, "Array example", "array1", "default")      // "1, 0x2, #3, 4.56, Seven"
cini_getcount(hcini, "Array example", "array2");            // 3
cini_getas(hcini, "Array example", "array2", 1, "default")  // "Two,Three"
cini_gets(hcini, "Array example", "array2", "default")      // "One,"Two,Three",'Four,Five'"
```
