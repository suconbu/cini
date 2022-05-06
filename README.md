# cini

ciniはC/C++向けのiniファイル読み取りライブラリです。

# 特徴

* シングルヘッダー
* 配列表現をサポート

# 使用例

```c
#define CINI_IMPLEMENTATION
#include "cini.h"

void sample_for_c()
{
    HCINI hcini = cini_create("sample.ini");
    if (cini_isgood(hcini)) {
        int i = cini_geti(hcini, "section-name", "key-name", 0 /* default_value */);
        float f = cini_getf(hcini, "section-name", "key-name", 0.0f);
        // cini_gets(cini_getas)が返す文字列ポインタはciniハンドルが解放されるまでの間有効
        const char* s = cini_gets(hcini, "section-name", "key-name", "default");
        cini_free(hcini);
    }
}

void sample_for_cpp()
{
    Cini cini("sample.ini");
    if (cini) {
        int i = cini.geti("section-name", "key-name", 0 /* default_value */);
        float f = cini.getf("section-name", "key-name", 0.0);
        const char* s = cini.gets("section-name", "key-name", "default");
    }
}
```

# 対象とするiniファイル形式

* 改行文字は LF (0x0A) または CRLF (0x0D, 0x0A) とします。
* 改行文字のみの行 (空行) を許容します。
* 「;」または「#」で始まる行をコメント行とみなします。
* キー名、セクション名の英大文字と小文字を区別します。
* キー名、セクション名、値の前後にある空白文字 (0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x20) は無視します。
* キー名の重複があった場合は先に記述されているものを優先します。
* セクション外でのエントリ (キーと値) の記述を許容します。
* セクション名の重複があった場合はそれぞれのセクションに含まれるエントリをマージします。キー名の重複があった場合は先に記述されているものを優先します。

# データ型

ciniでは、キーに設定された値を文字列または数値として取得できます。

## データ型 - 文字列

* 引用符 (「"」または「'」) は任意ですが、使用することで文字列の先頭または末尾に空白文字を含めたり、文字列中に配列要素の区切り文字である「,」を含めたりすることができます。
* 数値も文字列として取得することができます。
* 「\」で始まるエスケープ文字は認識しません。

iniファイル例：
```
[String example]
str1 = One
str2 = 123
str3 = " Two, Three "
str4 = ' Four \t Five \n'
str5 = "Six, "Seven""
```

取得例：
```
cini_gets(hcini, "String example", "str1", "default");      // "One"
cini_gets(hcini, "String example", "str2", "default");      // "123"
cini_gets(hcini, "String example", "str3", "default");      // " Two, Three "
cini_gets(hcini, "String example", "str4", "default");      // " Four \t Five \n"
cini_gets(hcini, "String example", "str5", "default");      // "Six, "Seven""
```

## データ型 - 数値

* 10進数と16進数を認識します。  
* 数値として解釈できない値に対して数値用の関数 (「cini_geti」等) で値の取得を試みた場合は失敗となり、引数で指定された既定値が返されます。  

iniファイル例：
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

取得例：
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

# 配列

ciniでは、カンマで区切られた値を配列と認識し、インデックス指定で値を取得することができます。

* 配列の要素数は「getcount」関数、各要素の値は「geta～」関数を使用してそれぞれ取得することができます。
* 一つの配列の中で異なるデータ型を混在させることができます
* 文字列内に「,」を含める場合は、文字列の両端を「"」または「'」で囲みます

iniファイル例：
```
[Array example]
array1 = 1, 0x2, #3, 4.56, Seven
array2 = One,"Two,Three",'Four,Five'
```

取得例：
```c
HCINI hcini = cini_create("sample.ini");
cini_getcount(hcini, "Array example", "array1");            // 5
cini_getai(hcini, "Array example", "array1", 0, -1)         // 1
cini_getai(hcini, "Array example", "array1", 4, -1)         // -1
cini_gets(hcini, "Array example", "array1", "default")      // "1, 0x2, #3, 4.56, Seven"
cini_getcount(hcini, "Array example", "array2");            // 3
cini_getas(hcini, "Array example", "array2", 1, "default")  // "Two,Three"
cini_gets(hcini, "Array example", "array2", "default")      // "One,"Two,Three",'Four,Five'"
cini_free(hcini);
```

# ライセンス

MIT License.
