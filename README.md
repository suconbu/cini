# cini

ciniはiniファイルを読み取るためのC/C++インターフェースです。  
設定ファイル/デバッグ用にどうぞ。

# 特徴

* 1つのヘッダーファイルで導入可能
* 配列アクセス用のインターフェース(geta～)を用意

# 仕様

## データ型

ciniは、iniファイル中の値を数値または文字列のどちらかとして認識します。  
データ型ごとの各値取得関数での取得可否を以下の表に示します。

| データ値 | geti, getai | getf, getaf | gets, getas       |
| -------- | ----------- | ----------- | ----------------- |
| 数値     | 可          | 可          | 可 (文字列として) |
| 文字列   | 不可        | 不可        | 可                |

### データ型 - 数値

整数(10進数または16進数)と実数(10進数)

記述例：  
```
;整数(10進数)
dec = 123

;整数(16進数)("0x"または"#"で始める)
hex1 = 0xFF
hex2 = #FF

;実数(10進数)
f1 = 4.56
f2 = .123
f3 = 7.89e2
```

### データ型 - 文字列

値両端の引用符(`"`, `'`)および空白文字(0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x20)を取り除いたものを文字列として認識します。  
文字列の両端以外にある引用符は文字列の一部として認識します。  

記述例：  
```
str1 = One             // One
str2 = " Two, Three "  // ␣Two,␣Three␣
str3 = ' Four, Five '  // ␣Four,␣Five␣
str4 = "Six, "Seven""  // Six,␣"Seven"
```

## 配列

ciniでは、カンマで区切られた値を配列と認識し、インデックス指定で値を取得することができます。

* 配列の要素数は「getcount」関数、各要素の値は「geta～」関数を使用してそれぞれ取得することができます。
* 一つの配列の中で異なるデータ型を混在させることができます
* 文字列内に「,」を含める場合は、文字列の両端を「"」または「'」で囲みます

記述例：  
```c
array1 = 1, 0x2, #3, 4.56, Seven
array2 = One,"Two,Three",'Four,Five'
```

## コメント

「;」または「#」で始まる行はコメント行として無視します。  
コメントは行の途中から始めることはできません。

```
;コメント                      // コメント行
  #コメント                    // コメント行 - 左に空白文字があってもOK
key = 100 ;コメントのつもり    // 「100 ;コメントのつもり」という文字列として認識される
```

# 使いかた例

## C

```c
// 必ず1つのC/C++ファイルではcini.hをインクルードする前にCINI_IMPLEMENTATIONを定義すること(ciniの関数実体定義のため)
#define CINI_IMPLEMENTATION
#include "cini.h"

void example_c()
{
    // iniファイルを参照するのはcini_createコール時のみ
    // 取得した値はハンドル内に保持される
    HCINI hcini = cini_create( "sample.ini" );
    if( hcini == NULL || cini_isfailed( hcini ) != 0 )
    {
        // 'cini_get～'は渡されたハンドルがNULLまたは読み込み失敗している時、デフォルト値または0を返却
    }

    // 基本

    i = cini_geti( hcini, "", "key1", -999 );        // 10
    i = cini_geti( hcini, "", "key2", -999 );        // 20
    i = cini_geti( hcini, "", "key3", -999 );        // 30
    s = cini_gets( hcini, "", "key4", "ERROR" );    // "10   ;コメントのつもり"
    // 'cini_gets'が返却した文字列ポインタはハンドルが解放されるまでの間のみ有効

    i = cini_geti( hcini, "section1", "key1", -999 );        // 100
    i = cini_geti( hcini, "section1", "key2", -999 );        // 256(0x100)
    i = cini_geti( hcini, "section1", "key3", -999 );        // 1122867(0x112233)
    f = cini_getf( hcini, "section1", "key4", -999.0F );    // -12.3400002F
    s = cini_gets( hcini, "section1", "key5", "ERROR" );    // "STRING"
    s = cini_gets( hcini, "section1", "key6", "ERROR" );    // "THIS IS A PEN."
    s = cini_gets( hcini, "section1", "key7", "ERROR" );    // "THIS IS A "PEN"."

    // 配列値

    i = cini_getcount( hcini, "section2", "key1" );                // 4
    i = cini_getai( hcini, "section2", "key1", 0, -999 );        // 1
    i = cini_getai( hcini, "section2", "key1", 1, -999 );        // 2
    i = cini_getai( hcini, "section2", "key1", 2, -999 );        // 3
    i = cini_getai( hcini, "section2", "key1", 3, -999 );        // 4
    i = cini_getai( hcini, "section2", "key1", 4, -999 );        // -999 (インデックス範囲外エラー)

    i = cini_getcount( hcini, "section2", "key2" );                // 5
    i = cini_getai( hcini, "section2", "key2", 0, -999 );        // 1
    i = cini_getai( hcini, "section2", "key2", 1, -999 );        // 2
    i = cini_getai( hcini, "section2", "key2", 2, -999 );        // 3
    f = cini_getaf( hcini, "section2", "key2", 3, -999.0 );        // 4.55999994F
    s = cini_getas( hcini, "section2", "key2", 4, "ERROR" );    // "Seven"

    i = cini_getcount( hcini, "section2", "key3" );                // 3
    s = cini_getas( hcini, "section2", "key3", 0, "ERROR" );    // "One"
    s = cini_getas( hcini, "section2", "key3", 1, "ERROR" );    // "Two,Three"
    s = cini_getas( hcini, "section2", "key3", 2, "ERROR" );    // "Four,Five"

    i = cini_geterrorcount( hcini );    // 1
    for( index = 0; index < i; index++ )
    {
        // Re-defined key, ignore the this line. (15)
        s = cini_geterror( hcini, index );
    }

    cini_free( hcini );
}
```

## C++

```c++
#define CINI_IMPLEMENTATION
#include "cini.h"

void example_cpp()
{
    Cini cini( "sample.ini" );
    if( cini.isfailed() )
    {
        // 読み込み失敗時、'get～'メソッドはデフォルト値または0を返却
    }

    // 基本

    i = cini.geti( "", "key1", -999 );        // 10
    i = cini.geti( "", "key2", -999 );        // 20
    i = cini.geti( "", "key3", -999 );        // 30
    s = cini.gets( "", "key4", "ERROR" );    // "10   ;コメントのつもり"

    i = cini.geti( "section1", "key1", -999 );        // 100
    i = cini.geti( "section1", "key2", -999 );        // 256(0x100)
    i = cini.geti( "section1", "key3", -999 );        // 1122867(0x112233)
    f = cini.getf( "section1", "key4", -999.0F );    // -12.3400002F
    s = cini.gets( "section1", "key5", "ERROR" );    // "STRING"
    s = cini.gets( "section1", "key6", "ERROR" );    // "THIS IS A PEN."
    s = cini.gets( "section1", "key7", "ERROR" );    // "THIS IS A "PEN"."

    // 配列値

    i = cini.getcount( "section2", "key1" );            // 4
    i = cini.getai( "section2", "key1", 0, -999 );        // 1
    i = cini.getai( "section2", "key1", 1, -999 );        // 2
    i = cini.getai( "section2", "key1", 2, -999 );        // 3
    i = cini.getai( "section2", "key1", 3, -999 );        // 4
    i = cini.getai( "section2", "key1", 4, -999 );        // -999 (インデックス範囲外エラー)

    i = cini.getcount( "section2", "key2" );            // 5
    i = cini.getai( "section2", "key2", 0, -999 );        // 1
    i = cini.getai( "section2", "key2", 1, -999 );        // 2
    i = cini.getai( "section2", "key2", 2, -999 );        // 3
    f = cini.getaf( "section2", "key2", 3, -999.0 );    // 4.55999994F
    s = cini.getas( "section2", "key2", 4, "ERROR" );    // "Seven"

    i = cini.getcount( "section2", "key3" );            // 3
    s = cini.getas( "section2", "key3", 0, "ERROR" );    // "One"
    s = cini.getas( "section2", "key3", 1, "ERROR" );    // "Two,Three"
    s = cini.getas( "section2", "key3", 2, "ERROR" );    // "Four,Five"

    i = cini.geterrorcount();    // 1
    for( index = 0; index < i; index++ )
    {
        // Re-defined key, ignore the this line. (15)
        s = cini.geterror( index );
    }
}
```

#### sample.ini ####
```ini
;「;」で始まる行はコメント

;セクション外にもエントリを記述可能
key1=10
;キーや値の前後の空白文字は除去される
 key2 = 20
        key3         =     30           
;行の途中の「;」はコメント開始として認識されない
;以下は「10   ;コメントのつもり」という文字列と認識
key4=10   ;コメントのつもり

[section1]
key1 = 100
;同一セクション内の重複キーは先勝ちとし2つ目以降を無視
key1 = 200
;「0x」または「#」から始まる場合は16進数と認識
key2 = 0x100
key3 = #112233
;小数点を含む場合は実数と認識
key4 = -12.34
;上記に該当しない場合は文字列と認識
key5 = STRING
;文字列の両端に明示的に付与された「"」または「'」は除去される
key6 = "THIS IS A PEN."
;文字列の途中に含まれる「"」または「'」はそのまま
key7 = THIS IS A "PEN".

[section2]
;値をカンマで区切ることで配列を表現
key1 = 1,2,3,4
;異なるデータ型の混在が可能
key2 = 1, 0x2, #3, 4.56, Seven
;文字列中に「,」を含めたい場合は「"」または「'」で囲む
key3 = One  ,  "Two,Three"  ,  'Four,Five'
```
