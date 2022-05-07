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
        TEST(cini_geti(hcini, "", "key1", -999) == 100);
        TEST(strcmp(cini_gets(hcini, "", "key2", "ERROR"), "TEST") == 0);
        TEST(strcmp(cini_gets(hcini, "", "key 3", "ERROR"), "TEST") == 0);
        TEST(cini_geti(hcini, "", "100", -999) == 99);
        TEST(strcmp(cini_gets(hcini, "", "k e\ty 4", "ERROR"), "TE  ST") == 0);
        cini_free(hcini);
    }
    // int section
    {
        HCINI hcini = cini_create(path);
        TEST(cini_geti(hcini, "int section", "key1", -999) == 200);
        TEST(cini_geti(hcini, "int section", "key2", -999) == -50);
        TEST(cini_geti(hcini, "int section", "key3", -999) == 50);
        TEST(cini_geti(hcini, "int section", "key4", -999) == 0xFF);
        TEST(cini_geti(hcini, "int section", "key5", -999) == 0x99FF);

        TEST(cini_getf(hcini, "int section", "key1", -999.0F) == 200.0F);
        TEST(cini_getf(hcini, "int section", "key2", -999.0F) == -50.0F);
        TEST(cini_getf(hcini, "int section", "key3", -999.0F) == 50.0F);
        TEST(cini_getf(hcini, "int section", "key4", -999.0F) == 255.0F);
        TEST(cini_getf(hcini, "int section", "key5", -999.0F) == 39423.0F);

        TEST(strcmp(cini_gets(hcini, "int section", "key1", "ERROR"), "200") == 0);
        TEST(strcmp(cini_gets(hcini, "int section", "key2", "ERROR"), "-50") == 0);
        TEST(strcmp(cini_gets(hcini, "int section", "key3", "ERROR"), "+50") == 0);
        TEST(strcmp(cini_gets(hcini, "int section", "key4", "ERROR"), "0xFF") == 0);
        TEST(strcmp(cini_gets(hcini, "int section", "key5", "ERROR"), "#99FF") == 0);

        TEST(cini_getcount(hcini, "int section", "key1") == 1);
        TEST(cini_getcount(hcini, "int section", "key2") == 1);
        TEST(cini_getcount(hcini, "int section", "key3") == 1);
        TEST(cini_getcount(hcini, "int section", "key4") == 1);
        TEST(cini_getcount(hcini, "int section", "key5") == 1);
        cini_free(hcini);
    }
    // float section
    {
        HCINI hcini = cini_create(path);
        TEST(cini_getf(hcini, "float section", "key1", -999.0F) == 12.34F);
        TEST(cini_getf(hcini, "float section", "key2", -999.0F) == -0.125F);
        TEST(cini_getf(hcini, "float section", "key3", -999.0F) == 1234.56006F);
        TEST(cini_getf(hcini, "float section", "key4", -999.0F) == 0.5F);

        TEST(cini_geti(hcini, "float section", "key1", -999) == 12);
        TEST(cini_geti(hcini, "float section", "key2", -999) == 0);
        TEST(cini_geti(hcini, "float section", "key3", -999) == 1234);
        TEST(cini_geti(hcini, "float section", "key4", -999) == 0);

        TEST(strcmp(cini_gets(hcini, "float section", "key1", "ERROR"), "12.34") == 0);
        TEST(strcmp(cini_gets(hcini, "float section", "key2", "ERROR"), "-0.125") == 0);
        TEST(strcmp(cini_gets(hcini, "float section", "key3", "ERROR"), "12.3456e2") == 0);
        TEST(strcmp(cini_gets(hcini, "float section", "key4", "ERROR"), ".5") == 0);

        TEST(cini_getcount(hcini, "float section", "key1") == 1);
        TEST(cini_getcount(hcini, "float section", "key2") == 1);
        TEST(cini_getcount(hcini, "float section", "key3") == 1);
        TEST(cini_getcount(hcini, "float section", "key4") == 1);
        cini_free(hcini);
    }
    // string section
    {
        HCINI hcini = cini_create(path);
        TEST(strcmp(cini_gets(hcini, "string section", "key1", "ERROR"), "TEXT") == 0);
        TEST(strcmp(cini_gets(hcini, "string section", "key2", "ERROR"), "THIS IS A PEN.") == 0);
        TEST(strcmp(cini_gets(hcini, "string section", "key3", "ERROR"), "=") == 0);
        TEST(strcmp(cini_gets(hcini, "string section", "key4", "ERROR"), "TEXT") == 0);
        TEST(strcmp(cini_gets(hcini, "string section", "key5", "ERROR"), "\"TEXT\"") == 0);
        TEST(strcmp(cini_gets(hcini, "string section", "key6", "ERROR"), "\"TEXT\" TEXT") == 0);
        TEST(strcmp(cini_gets(hcini, "string section", "key7", "ERROR"), u8"AA,BB,CC;DD,EE,FF,あ,い,う") == 0);
        TEST(strcmp(cini_gets(hcini, "string section", "key8", "ERROR"), "\\t\\r\\n") == 0);

        TEST(cini_geti(hcini, "string section", "key1", -999) == -999);
        TEST(cini_geti(hcini, "string section", "key2", -999) == -999);
        TEST(cini_geti(hcini, "string section", "key3", -999) == -999);
        TEST(cini_geti(hcini, "string section", "key4", -999) == -999);
        TEST(cini_geti(hcini, "string section", "key5", -999) == -999);
        TEST(cini_geti(hcini, "string section", "key6", -999) == -999);
        TEST(cini_geti(hcini, "string section", "key7", -999) == -999);
        TEST(cini_geti(hcini, "string section", "key8", -999) == -999);

        TEST(cini_getcount(hcini, "string section", "key1") == 1);
        TEST(cini_getcount(hcini, "string section", "key2") == 1);
        TEST(cini_getcount(hcini, "string section", "key3") == 1);
        TEST(cini_getcount(hcini, "string section", "key4") == 1);
        TEST(cini_getcount(hcini, "string section", "key5") == 1);
        TEST(cini_getcount(hcini, "string section", "key6") == 1);
        TEST(cini_getcount(hcini, "string section", "key7") == 1);
        TEST(cini_getcount(hcini, "string section", "key8") == 1);
        cini_free(hcini);
    }
    // array section
    {
        HCINI hcini = cini_create(path);
        TEST(cini_getcount(hcini, "array section", "key1") == 4);

        TEST(cini_geti(hcini, "array section", "key1", -999) == -999);
        TEST(strcmp(cini_gets(hcini, "array section", "key1", "ERROR"), "1,2,3,") == 0);

        TEST(cini_getai(hcini, "array section", "key1", 0, -999) == 1);
        TEST(cini_getai(hcini, "array section", "key1", 1, -999) == 2);
        TEST(cini_getai(hcini, "array section", "key1", 2, -999) == 3);
        TEST(cini_getai(hcini, "array section", "key1", 3, -999) == -999);
        TEST(strcmp(cini_getas(hcini, "array section", "key1", 3, "ERROR"), "") == 0);
        TEST(cini_getai(hcini, "array section", "key1", 4, -999) == -999);

        TEST(cini_getcount(hcini, "array section", "key10") == 3);
        TEST(cini_getaf(hcini, "array section", "key10", 0, -999.0F) == 1.23F);
        TEST(cini_getaf(hcini, "array section", "key10", 1, -999.0F) == -0.125F);
        TEST(cini_getaf(hcini, "array section", "key10", 2, -999.0F) == 1234.56006F);

        TEST(cini_getcount(hcini, "array section", "key2") == 4);
        TEST(strcmp(cini_getas(hcini, "array section", "key2", 0, "ERROR"), "A") == 0);
        TEST(strcmp(cini_getas(hcini, "array section", "key2", 1, "ERROR"), "BB") == 0);
        TEST(strcmp(cini_getas(hcini, "array section", "key2", 2, "ERROR"), "CCC,DDDD") == 0);
        TEST(strcmp(cini_getas(hcini, "array section", "key2", 3, "ERROR"), "EEEEE") == 0);

        TEST(cini_getcount(hcini, "array section", "key20") == 6);
        TEST(strcmp(cini_getas(hcini, "array section", "key20", 0, "ERROR"), "AA") == 0);
        TEST(strcmp(cini_getas(hcini, "array section", "key20", 1, "ERROR"), "BB") == 0);
        TEST(strcmp(cini_getas(hcini, "array section", "key20", 2, "ERROR"), "C\"C\"C , C ") == 0);
        TEST(strcmp(cini_getas(hcini, "array section", "key20", 3, "ERROR"), "\"D") == 0);
        TEST(strcmp(cini_getas(hcini, "array section", "key20", 4, "ERROR"), "E E\"E") == 0);
        TEST(strcmp(cini_getas(hcini, "array section", "key20", 5, "ERROR"), "F") == 0);

        TEST(cini_getcount(hcini, "array section", "key21") == 5);
        TEST(strcmp(cini_getas(hcini, "array section", "key21", 0, "ERROR"), "AA") == 0);
        TEST(strcmp(cini_getas(hcini, "array section", "key21", 1, "ERROR"), "BB\" , \"B\"B\"B , B ") == 0);
        TEST(strcmp(cini_getas(hcini, "array section", "key21", 2, "ERROR"), "C, C C'C'") == 0);
        TEST(strcmp(cini_getas(hcini, "array section", "key21", 3, "ERROR"), "\"D'") == 0);
        TEST(strcmp(cini_getas(hcini, "array section", "key21", 4, "ERROR"), "F") == 0);

        TEST(cini_geti(hcini, "error", "int over", -999) == -999);
        TEST(cini_geti(hcini, "error", "float over", -999) == -999);
        cini_free(hcini);
    }
    {
        HCINI hcini = cini_create_with_section(path, 0);
        TEST(cini_geti(hcini, "", "key1", -999) == 100);
        TEST(cini_geti(hcini, "int section", "key1", -999) == 200);
        TEST(cini_getf(hcini, "float section", "key1", -999.0F) == 12.34F);
        TEST(strcmp(cini_gets(hcini, "string section", "key1", "ERROR"), "TEXT") == 0);
        TEST(cini_getcount(hcini, "array section", "key1") == 4);
        TEST(cini_getai(hcini, "array section", "key1", 0, -999) == 1);
        TEST(cini_getaf(hcini, "array section", "key10", 0, -999.0F) == 1.23F);
        cini_free(hcini);
    }
    {
        HCINI hcini = cini_create_with_section(path, "");
        TEST(cini_geti(hcini, "", "key1", -999) == 100);
        TEST(cini_geti(hcini, "int section", "key1", -999) == -999);
        TEST(cini_getf(hcini, "float section", "key1", -999.0F) == -999.0F);
        TEST(strcmp(cini_gets(hcini, "string section", "key1", "ERROR"), "TEXT") != 0);
        TEST(cini_getcount(hcini, "array section", "key1") == 0);
        TEST(cini_getai(hcini, "array section", "key1", 0, -999) == -999);
        TEST(cini_getaf(hcini, "array section", "key10", 0, -999.0F) == -999.0F);
        cini_free(hcini);
    }
    {
        HCINI hcini = cini_create_with_section(path, "int section");
        TEST(cini_geti(hcini, "", "key1", -999) == -999);
        TEST(cini_geti(hcini, "int section", "key1", -999) == 200);
        TEST(cini_getf(hcini, "float section", "key1", -999.0F) == -999.0F);
        TEST(strcmp(cini_gets(hcini, "string section", "key1", "ERROR"), "TEXT") != 0);
        TEST(cini_getcount(hcini, "array section", "key1") == 0);
        TEST(cini_getai(hcini, "array section", "key1", 0, -999) == -999);
        TEST(cini_getaf(hcini, "array section", "key10", 0, -999.0F) == -999.0F);
        cini_free(hcini);
    }
    // duplicated section
    {
        HCINI hcini = cini_create(path);
        TEST(cini_geti(hcini, "duplicated", "key1", -999) == 100);
    }
    {
        HCINI hcini = cini_create(path);
        TEST(hcini != NULL);
        TEST(cini_isgood(hcini) != 0);
        cini_free(hcini);
    }
    {
        HCINI hcini = cini_create("alkjgbak4nubiato");
        TEST(hcini != NULL);
        TEST(cini_isgood(hcini) == 0);
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
