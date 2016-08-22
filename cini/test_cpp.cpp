//
// test_cpp.cpp - Test code for C++
// 
// Copyright (C) 2016 suconbu.
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

#include "stdafx.h"
#include "test.h"

void test_cpp( const char* path )
{
	TEST_PRINT( stdout, "\n" );
	TEST_PRINT( stdout, "---------- test_cpp ----------\n" );
	TEST_PRINT( stdout, "\n" );

	TEST_PRINT( stdout, "test data\t%s\n", path );
	TEST_PRINT( stdout, "\n" );

	TEST_PRINT( stdout, "#\tresult\ttest\tline\n" );
	// unnamed section
	{
		Cini cini( path );
		TEST( cini.geti( "", "key1", -999 ) == 100 );
		TEST( strcmp( cini.gets( "", "key2", "ERROR" ), "TEST" ) == 0 );
		TEST( strcmp( cini.gets( "", "key 3", "ERROR" ), "TEST" ) == 0 );
		TEST( cini.geti( "", "100", -999 ) == 99 );
		TEST( strcmp( cini.gets( "", "k e\ty 4", "ERROR" ), "TE  ST" ) == 0 );
	}
	// int section
	{
		Cini cini( path );
		TEST( cini.geti( "int section", "key1", -999 ) == 200 );
		TEST( cini.geti( "int section", "key2", -999 ) == -50 );
		TEST( cini.geti( "int section", "key3", -999 ) == 50 );
		TEST( cini.geti( "int section", "key4", -999 ) == 0xFF );
		TEST( cini.geti( "int section", "key5", -999 ) == 0x99FF );

		TEST( cini.getf( "int section", "key1", -999.0F ) == 200.0F );
		TEST( cini.getf( "int section", "key2", -999.0F ) == -50.0F );
		TEST( cini.getf( "int section", "key3", -999.0F ) == 50.0F );
		TEST( cini.getf( "int section", "key4", -999.0F ) == 255.0F );
		TEST( cini.getf( "int section", "key5", -999.0F ) == 39423.0F );

		TEST( strcmp( cini.gets( "int section", "key1", "ERROR" ), "200" ) == 0 );
		TEST( strcmp( cini.gets( "int section", "key2", "ERROR" ), "-50" ) == 0 );
		TEST( strcmp( cini.gets( "int section", "key3", "ERROR" ), "+50" ) == 0 );
		TEST( strcmp( cini.gets( "int section", "key4", "ERROR" ), "0xFF" ) == 0 );
		TEST( strcmp( cini.gets( "int section", "key5", "ERROR" ), "#99FF" ) == 0 );

		TEST( cini.getcount( "int section", "key1" ) == 1 );
		TEST( cini.getcount( "int section", "key2" ) == 1 );
		TEST( cini.getcount( "int section", "key3" ) == 1 );
		TEST( cini.getcount( "int section", "key4" ) == 1 );
		TEST( cini.getcount( "int section", "key5" ) == 1 );
	}
	// float section
	{
		Cini cini( path );
		TEST( cini.getf( "float section", "key1", -999.0F ) == 12.34F );
		TEST( cini.getf( "float section", "key2", -999.0F ) == -0.125F );
		TEST( cini.getf( "float section", "key3", -999.0F ) == 1234.56F );

		TEST( cini.geti( "float section", "key1", -999 ) == 12 );
		TEST( cini.geti( "float section", "key2", -999 ) == 0 );
		TEST( cini.geti( "float section", "key3", -999 ) == 1234 );

		TEST( strcmp( cini.gets( "float section", "key1", "ERROR" ), "12.34" ) == 0 );
		TEST( strcmp( cini.gets( "float section", "key2", "ERROR" ), "-0.125" ) == 0 );
		TEST( strcmp( cini.gets( "float section", "key3", "ERROR" ), "12.3456e2" ) == 0 );

		TEST( cini.getcount( "float section", "key1" ) == 1 );
		TEST( cini.getcount( "float section", "key2" ) == 1 );
		TEST( cini.getcount( "float section", "key3" ) == 1 );
	}
	// string section
	{
		Cini cini( path );
		TEST( strcmp( cini.gets( "string section", "key1", "ERROR" ), "TEXT" ) == 0 );
		TEST( strcmp( cini.gets( "string section", "key2", "ERROR" ), "THIS IS A PEN." ) == 0 );
		TEST( strcmp( cini.gets( "string section", "key3", "ERROR" ), "=" ) == 0 );
		TEST( strcmp( cini.gets( "string section", "key4", "ERROR" ), "TEXT" ) == 0 );
		TEST( strcmp( cini.gets( "string section", "key5", "ERROR" ), "\"TEXT\"" ) == 0 );
		TEST( strcmp( cini.gets( "string section", "key6", "ERROR" ), "\"TEXT\" TEXT" ) == 0 );
		TEST( strcmp( cini.gets( "string section", "key7", "ERROR" ), "AA,BB,CC;DD,EE,FF,‚ ,‚¢,‚¤" ) == 0 );

		TEST( cini.geti( "string section", "key1", -999 ) == -999 );
		TEST( cini.geti( "string section", "key2", -999 ) == -999 );
		TEST( cini.geti( "string section", "key3", -999 ) == -999 );
		TEST( cini.geti( "string section", "key4", -999 ) == -999 );
		TEST( cini.geti( "string section", "key5", -999 ) == -999 );
		TEST( cini.geti( "string section", "key6", -999 ) == -999 );
		TEST( cini.geti( "string section", "key7", -999 ) == -999 );

		TEST( cini.getcount( "string section", "key1" ) == 1 );
		TEST( cini.getcount( "string section", "key2" ) == 1 );
		TEST( cini.getcount( "string section", "key3" ) == 1 );
		TEST( cini.getcount( "string section", "key4" ) == 1 );
		TEST( cini.getcount( "string section", "key5" ) == 1 );
		TEST( cini.getcount( "string section", "key6" ) == 1 );
		TEST( cini.getcount( "string section", "key7" ) == 1 );
	}
	// array section
	{
		Cini cini( path );
		TEST( cini.getcount( "array section", "key1" ) == 4 );

		TEST( cini.geti( "array section", "key1", -999 ) == -999 );
		TEST( strcmp( cini.gets( "array section", "key1", "ERROR" ), "1,2,3," ) == 0 );

		TEST( cini.getai( "array section", "key1", 0, -999 ) == 1 );
		TEST( cini.getai( "array section", "key1", 1, -999 ) == 2 );
		TEST( cini.getai( "array section", "key1", 2, -999 ) == 3 );
		TEST( cini.getai( "array section", "key1", 3, -999 ) == -999 );
		TEST( strcmp( cini.getas( "array section", "key1", 3, "ERROR" ), "" ) == 0 );
		TEST( cini.getai( "array section", "key1", 4, -999 ) == -999 );

		TEST( cini.getcount( "array section", "key10" ) == 3 );
		TEST( cini.getaf( "array section", "key10", 0, -999.0F ) == 1.23F );
		TEST( cini.getaf( "array section", "key10", 1, -999.0F ) == -0.125F );
		TEST( cini.getaf( "array section", "key10", 2, -999.0F ) == 1234.56F );

		TEST( cini.getcount( "array section", "key2" ) == 4 );
		TEST( strcmp( cini.getas( "array section", "key2", 0, "ERROR" ), "A" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key2", 1, "ERROR" ), "BB" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key2", 2, "ERROR" ), "CCC,DDDD" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key2", 3, "ERROR" ), "EEEEE" ) == 0 );

		TEST( cini.getcount( "array section", "key20" ) == 6 );
		TEST( strcmp( cini.getas( "array section", "key20", 0, "ERROR" ), "AA" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key20", 1, "ERROR" ), "BB" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key20", 2, "ERROR" ), "C\"C\"C , C " ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key20", 3, "ERROR" ), "\"D" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key20", 4, "ERROR" ), "E E\"E" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key20", 5, "ERROR" ), "F" ) == 0 );

		TEST( cini.getcount( "array section", "key21" ) == 5 );
		TEST( strcmp( cini.getas( "array section", "key21", 0, "ERROR" ), "AA" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key21", 1, "ERROR" ), "BB\" , \"B\"B\"B , B " ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key21", 2, "ERROR" ), "C, C C'C'" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key21", 3, "ERROR" ), "\"D'" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key21", 4, "ERROR" ), "F" ) == 0 );

		TEST( cini.geti( "error", "int over", -999 ) == -999 );
		TEST( cini.geti( "error", "float over", -999 ) == -999 );
	}
	{
		Cini cini( path, 0 );
		TEST( cini.geti( "", "key1", -999 ) == 100 );
		TEST( cini.geti( "int section", "key1", -999 ) == 200 );
		TEST( cini.getf( "float section", "key1", -999.0F ) == 12.34F );
		TEST( strcmp( cini.gets( "string section", "key1", "ERROR" ), "TEXT" ) == 0 );
		TEST( cini.getcount( "array section", "key1" ) == 4 );
		TEST( cini.getai( "array section", "key1", 0, -999 ) == 1 );
		TEST( cini.getaf( "array section", "key10", 0, -999.0F ) == 1.23F );
	}
	{
		Cini cini( path, "" );
		TEST( cini.geti( "", "key1", -999 ) == 100 );
		TEST( cini.geti( "int section", "key1", -999 ) == -999 );
		TEST( cini.getf( "float section", "key1", -999.0F ) == -999.0F );
		TEST( strcmp( cini.gets( "string section", "key1", "ERROR" ), "TEXT" ) != 0 );
		TEST( cini.getcount( "array section", "key1" ) == 0 );
		TEST( cini.getai( "array section", "key1", 0, -999 ) == -999 );
		TEST( cini.getaf( "array section", "key10", 0, -999.0F ) == -999.0F );
	}
	{
		Cini cini( path, "int section" );
		TEST( cini.geti( "", "key1", -999 ) == -999 );
		TEST( cini.geti( "int section", "key1", -999 ) == 200 );
		TEST( cini.getf( "float section", "key1", -999.0F ) == -999.0F );
		TEST( strcmp( cini.gets( "string section", "key1", "ERROR" ), "TEXT" ) != 0 );
		TEST( cini.getcount( "array section", "key1" ) == 0 );
		TEST( cini.getai( "array section", "key1", 0, -999 ) == -999 );
		TEST( cini.getaf( "array section", "key10", 0, -999.0F ) == -999.0F );
	}
	{
		Cini cini( path );
		TEST( !cini.isfailed() );
	}
	{
		Cini cini( "alkjgbak4nubiato" );
		TEST( cini.isfailed() );
	}

	TEST_PRINT( stdout, "\n" );

	{
		TEST_PRINT( stdout, "ERRORS:\n" );
		Cini cini( path );
		int num_errors = cini.geterrorcount();
		for( int i = 0; i < num_errors; i++ )
		{
			TEST_PRINT( stdout, "%4d\t%s\n", i + 1, cini.geterror( i ) );
		}
	}

	TEST_PRINT( stdout, "\n" );
}
