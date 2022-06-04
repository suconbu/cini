#include "cini.h"
#include "test.h"

#include <string.h>

void test_c(const char* path)
{
    TEST_PRINT(stdout, "\n");
    TEST_PRINT(stdout, "---------- test_c ----------\n");
    TEST_PRINT(stdout, "\n");

    TEST_PRINT(stdout, "test data\t%s\n", path);
    TEST_PRINT(stdout, "\n");

    TEST_PRINT(stdout, "#\tresult\ttest\tline\n");
    // unnamed section
    {
        HCINI hcini = cini_create(path);

        TEST(cini_geterrorcount(hcini) == 3);

        TEST(cini_geti(hcini, "", "key01", -999) == 1234);
        TEST(cini_getf(hcini, "", "key01", -999.0f) == 1234.0f);
        TEST(strcmp(cini_gets(hcini, "", "key01", "ERROR"), "1234") == 0);

        TEST(cini_geti(hcini, "", "key02", -999) == -999);
        TEST(cini_getf(hcini, "", "key02", -999.0f) == -999.0f);
        TEST(strcmp(cini_gets(hcini, "", "key02", "ERROR"), "TEST") == 0);

        TEST(cini_geti(hcini, "", "key03", -999) == 1234);
        TEST(cini_getf(hcini, "", "key03", -999.0f) == 1234.0f);
        TEST(strcmp(cini_gets(hcini, "", "key03", "ERROR"), "1234") == 0);

        TEST(cini_geti(hcini, "", "key04", -999) == -999);
        TEST(cini_getf(hcini, "", "key04", -999.0f) == -999.0f);
        TEST(strcmp(cini_gets(hcini, "", "key04", "ERROR"), "TEST") == 0);

        TEST(cini_geti(hcini, "", "key05", -999) == -999);
        TEST(cini_getf(hcini, "", "key05", -999.0f) == -999.0f);
        TEST(strcmp(cini_gets(hcini, "", "key05", "ERROR"), "1234 ; not a comment") == 0);

        TEST(cini_geti(hcini, "", "100", -999) == 100);
        TEST(cini_getf(hcini, "", "100", -999.0f) == 100.0f);
        TEST(strcmp(cini_gets(hcini, "", "100", "ERROR"), "100") == 0);

        TEST(cini_geti(hcini, "", "k e y", -999) == -999);
        TEST(cini_getf(hcini, "", "k e y", -999.0f) == -999.0f);
        TEST(strcmp(cini_gets(hcini, "", "k e y", "ERROR"), "T E S T") == 0);

        TEST(cini_geti(hcini, "", "k\te\ty", -999) == -999);
        TEST(cini_getf(hcini, "", "k\te\ty", -999.0f) == -999.0f);
        TEST(strcmp(cini_gets(hcini, "", "k\te\ty", "ERROR"), "T\tE\tS\tT") == 0);

        TEST(cini_geti(hcini, "", "\\key", -999) == -999);
        TEST(cini_getf(hcini, "", "\\key", -999.0f) == -999.0f);
        TEST(strcmp(cini_gets(hcini, "", "\\key", "ERROR"), "KEY") == 0);

        cini_free(hcini);
    }
    // integer section
    {
        HCINI hcini = cini_create(path);

        TEST(cini_geti(hcini, "integer", "key01", -999) == 0);
        TEST(cini_geti(hcini, "integer", "key02", -999) == 1234);
        TEST(cini_geti(hcini, "integer", "key03", -999) == 1234);
        TEST(cini_geti(hcini, "integer", "key04", -999) == -1234);

        TEST(cini_geti(hcini, "integer", "intmax", -999) == 2147483647LL);
        TEST(cini_geti(hcini, "integer", "intmax+", -999) == -999);
        TEST(cini_geti(hcini, "integer", "intmin", -999) == -2147483648LL);
        TEST(cini_geti(hcini, "integer", "intmin-", -999) == -999);
        TEST(cini_geti(hcini, "integer", "uintmax", -999) == -999);
        TEST(cini_geti(hcini, "integer", "uintmax+", -999) == -999);
        TEST(cini_geti(hcini, "integer", "longmax", -999) == -999);
        TEST(cini_geti(hcini, "integer", "longmax+", -999) == -999);
        TEST(cini_geti(hcini, "integer", "longmin", -999) == -999);
        TEST(cini_geti(hcini, "integer", "longmin-", -999) == -999);
        TEST(cini_geti(hcini, "integer", "ulongmax", -999) == -999);
        TEST(cini_geti(hcini, "integer", "ulongmax+", -999) == -999);

        cini_free(hcini);
    }
    // float section
    {
        HCINI hcini = cini_create(path);

        TEST(cini_getf(hcini, "float", "key01", -999.0f) == 0.0f);
        TEST(cini_getf(hcini, "float", "key02", -999.0f) == 0.5f);
        TEST(cini_getf(hcini, "float", "key03", -999.0f) == 5.0f);
        TEST(cini_getf(hcini, "float", "key04", -999.0f) == 12.34f);
        TEST(cini_getf(hcini, "float", "key05", -999.0f) == -0.125f);
        TEST(cini_getf(hcini, "float", "key06", -999.0f) == 1234.56006f);
        TEST(cini_getf(hcini, "float", "key07", -999.0f) == 1234.56006f);
        TEST(cini_getf(hcini, "float", "key08", -999.0f) == 0.123456001f);

        TEST(cini_getf(hcini, "float", "floatmax", -999.0f) == 3.40282002e+38f);
        TEST(cini_getf(hcini, "float", "floatmax+", -999.0f) == -999.0f);
        TEST(cini_getf(hcini, "float", "floatmin", -999.0f) == -3.40282002e+38f);
        TEST(cini_getf(hcini, "float", "floatmin-", -999.0f) == -999.0f);
        TEST(cini_getf(hcini, "float", "doublemax", -999.0f) == -999.0f);
        TEST(cini_getf(hcini, "float", "doublemax+", -999.0f) == -999.0f);
        TEST(cini_getf(hcini, "float", "doublemin", -999.0f) == -999.0f);
        TEST(cini_getf(hcini, "float", "doublemin-", -999.0f) == -999.0f);

        cini_free(hcini);
    }
    // string section
    {
        HCINI hcini = cini_create(path);

        TEST(strcmp(cini_gets(hcini, "string", "key01", "ERROR"), "") == 0);
        TEST(strcmp(cini_gets(hcini, "string", "key02", "ERROR"), "") == 0);
        TEST(strcmp(cini_gets(hcini, "string", "key03", "ERROR"), "=") == 0);
        TEST(strcmp(cini_gets(hcini, "string", "key04", "ERROR"), "TEST") == 0);
        TEST(strcmp(cini_gets(hcini, "string", "key05", "ERROR"), "\"") == 0);
        TEST(strcmp(cini_gets(hcini, "string", "key06", "ERROR"), "\"\"") == 0);
        TEST(strcmp(cini_gets(hcini, "string", "key07", "ERROR"), "TEST") == 0);
        TEST(strcmp(cini_gets(hcini, "string", "key08", "ERROR"), "\"TEST\"") == 0);
        TEST(strcmp(cini_gets(hcini, "string", "key09", "ERROR"), "\"TEST\" TEST") == 0);
        TEST(strcmp(cini_gets(hcini, "string", "key10", "ERROR"), u8"AA,BB,CC;DD,EE,FF,あ,い,う") == 0);
        TEST(strcmp(cini_gets(hcini, "string", "key11", "ERROR"), "TEST") == 0);
        TEST(strcmp(cini_gets(hcini, "string", "key12", "ERROR"), "'TEST'") == 0);
        TEST(strcmp(cini_gets(hcini, "string", "key13", "ERROR"), "'TEST' TEST") == 0);
        TEST(strcmp(cini_gets(hcini, "string", "key14", "ERROR"), u8"AA,BB,CC;DD,EE,FF,あ,い,う") == 0);
        TEST(strcmp(cini_gets(hcini, "string", "key15", "ERROR"), "\\t\\r\\n") == 0);
        TEST(strcmp(cini_gets(hcini, "string", "key16", "ERROR"), "\"\\\"\\\"\"\\n") == 0);

        cini_free(hcini);
    }
    // array section
    {
        HCINI hcini = cini_create(path);

        TEST(cini_getcount(hcini, "array", "key01") == 3);
        TEST(cini_getai(hcini, "array", "key01", 0, -999) == 1);
        TEST(cini_getai(hcini, "array", "key01", 1, -999) == 2);
        TEST(cini_getai(hcini, "array", "key01", 2, -999) == 3);
        TEST(cini_getai(hcini, "array", "key01", -1, -999) == -999);
        TEST(cini_getai(hcini, "array", "key01", 3, -999) == -999);

        TEST(cini_getcount(hcini, "array", "key02") == 2);
        TEST(strcmp(cini_getas(hcini, "array", "key02", 0, "ERROR"), "1") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key02", 1, "ERROR"), "") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key02", 2, "ERROR"), "ERROR") == 0);

        TEST(cini_getcount(hcini, "array", "key03") == 2);
        TEST(strcmp(cini_getas(hcini, "array", "key03", 0, "ERROR"), "") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key03", 1, "ERROR"), "") == 0);

        TEST(cini_getcount(hcini, "array", "key04") == 3);
        TEST(strcmp(cini_getas(hcini, "array", "key04", 0, "ERROR"), "") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key04", 1, "ERROR"), "") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key04", 2, "ERROR"), "") == 0);

        TEST(cini_getcount(hcini, "array", "key05") == 3);
        TEST(strcmp(cini_getas(hcini, "array", "key05", 0, "ERROR"), "1.23") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key05", 1, "ERROR"), "-0.125") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key05", 2, "ERROR"), "12.3456e2") == 0);

        TEST(cini_getcount(hcini, "array", "key06") == 4);
        TEST(strcmp(cini_getas(hcini, "array", "key06", 0, "ERROR"), "A") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key06", 1, "ERROR"), "BB") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key06", 2, "ERROR"), "CCC,DDDD") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key06", 3, "ERROR"), "EEEEE") == 0);

        TEST(cini_getcount(hcini, "array", "key07") == 6);
        TEST(strcmp(cini_getas(hcini, "array", "key07", 0, "ERROR"), "AA") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key07", 1, "ERROR"), "BB") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key07", 2, "ERROR"), "C\"C\"C , C ") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key07", 3, "ERROR"), "\"D") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key07", 4, "ERROR"), "E E\"E") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key07", 5, "ERROR"), "F") == 0);

        TEST(cini_getcount(hcini, "array", "key08") == 5);
        TEST(strcmp(cini_getas(hcini, "array", "key08", 0, "ERROR"), "AA") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key08", 1, "ERROR"), "BB\" , \"B\"B\"B , B ") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key08", 2, "ERROR"), "C, C C'C'") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key08", 3, "ERROR"), "\"D'") == 0);
        TEST(strcmp(cini_getas(hcini, "array", "key08", 4, "ERROR"), "F") == 0);

        cini_free(hcini);
    }
    {
        HCINI hcini = cini_create_with_section(path, 0);
        TEST(cini_geti(hcini, "", "key01", -999) == 1234);
        TEST(cini_geti(hcini, "integer", "key01", -999) == 0);
        cini_free(hcini);
    }
    {
        HCINI hcini = cini_create_with_section(path, "");
        TEST(cini_geti(hcini, "", "key01", -999) == 1234);
        TEST(cini_geti(hcini, "integer", "key01", -999) == -999);
        cini_free(hcini);
    }
    {
        HCINI hcini = cini_create_with_section(path, "integer");
        TEST(cini_geti(hcini, "", "key01", -999) == -999);
        TEST(cini_geti(hcini, "integer", "key01", -999) == 0);
        cini_free(hcini);
    }
    {
        HCINI hcini = cini_create(path);
        TEST(hcini != NULL);
        TEST(cini_geterrorcount(hcini) == 3);
        cini_free(hcini);
    }
    {
        HCINI hcini = cini_create("alkjgbak4nubiato");
        TEST(hcini != NULL);
        TEST(cini_geterrorcount(hcini) == 1);
        cini_free(hcini);
    }

    TEST_PRINT(stdout, "\n");

    {
        TEST_PRINT(stdout, "ERRORS:\n");
        HCINI hcini = cini_create(path);
        int num_errors = cini_geterrorcount(hcini);
        for (int i = 0; i < num_errors; i++) {
            TEST_PRINT(stdout, "%4d\t%s\n", i + 1, cini_geterror(hcini, i));
        }
        cini_free(hcini);
    }
}
