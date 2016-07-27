// cini.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#define TEST( cond )	print_test_result( (int)(cond), #cond, __LINE__ )

int G_TEST_COUNT = 0;
int G_OK_COUNT = 0;

void print_test_result( int cond, const char* cond_text, int line )
{
	G_TEST_COUNT++;
	if( cond )
	{
		fprintf_s( stdout, "%4d\t(^-^)\t[%s]\n", G_TEST_COUNT, cond_text );
		G_OK_COUNT++;
	}
	else
	{
		fprintf_s( stdout, "%4d\t(x_x)\t[%s]\n", G_TEST_COUNT, cond_text );
	}
}

void print_test_summary()
{
	fprintf_s( stdout, "----------------------------------------\n");
	fprintf_s( stdout, "count\t(^-^)\t%%\n" );
	fprintf_s( stdout, "%d\t%d\t%.01f\n",
		G_TEST_COUNT,
		G_OK_COUNT,
		G_OK_COUNT * 100.F / G_TEST_COUNT );
	fprintf_s( stdout, "----------------------------------------\n" );
}

void print_errors( Cini& cini )
{
	int num_errors = cini.geterrorcount();
	for( int i = 0; i < num_errors; i++ )
	{
		fprintf_s( stdout, "%4d\t%s\n", i + 1, cini.geterror( i ) );
	}
}

void append_test_result()
{
	std::ofstream ofs( "testresult.txt", std::ifstream::app );

	if( !ofs.fail() )
	{
		TCHAR date[100] = { 0 };
		TCHAR time[100] = { 0 };
		::GetDateFormat( LOCALE_SYSTEM_DEFAULT, 0, NULL, "yyyy/MM/dd", date, sizeof( date ) );
		::GetTimeFormat( LOCALE_SYSTEM_DEFAULT, 0, NULL, "HH:mm:ss", time, sizeof( time ) );
		ofs << date << " " << time << "\t"
			<< (G_OK_COUNT * 100 / G_TEST_COUNT) << "%" << "\t"
			<< G_OK_COUNT << "/" << G_TEST_COUNT << std::endl;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	const char* path = "test.ini";
	if( argc > 1 )
	{
		path = argv[1];
	}

	fprintf_s( stdout, "test data\t%s\n", path);
	fprintf_s( stdout, "\n" );

	fprintf_s( stdout, "#\tresult\ttest\tline\n" );
	{
		Cini cini( path );
		TEST( cini.geti( "", "key1", -999 ) == 100 );
		TEST( strcmp( cini.gets( "", "key2", "ERROR" ), "TEST" ) == 0 );
		TEST( strcmp( cini.gets( "", "key 3", "ERROR" ), "TEST" ) == 0 );
		TEST( cini.geti( "", "100", -999 ) == 99 );
		TEST( strcmp( cini.gets( "", "k e\ty 4", "ERROR" ), "TE  ST" ) == 0 );
	}
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
	{
		Cini cini( path );
		TEST( cini.getf( "float section", "key1", -999.0F ) == 12.34F );
		TEST( cini.getf( "float section", "key2", -999.0F ) == -0.125F );

		TEST( cini.geti( "float section", "key1", -999 ) == 12 );
		TEST( cini.geti( "float section", "key2", -999 ) == 0 );

		TEST( strcmp( cini.gets( "float section", "key1", "ERROR" ), "12.34" ) == 0 );
		TEST( strcmp( cini.gets( "float section", "key2", "ERROR" ), "-0.125" ) == 0 );

		TEST( cini.getcount( "float section", "key1" ) == 1 );
		TEST( cini.getcount( "float section", "key2" ) == 1 );
	}
	{
		Cini cini( path );
		TEST( strcmp( cini.gets( "string section", "key1", "ERROR" ), "TEXT" ) == 0 );
		TEST( strcmp( cini.gets( "string section", "key2", "ERROR" ), "THIS IS A PEN." ) == 0 );
		TEST( strcmp( cini.gets( "string section", "key3", "ERROR" ), "=" ) == 0 );
		TEST( strcmp( cini.gets( "string section", "key4", "ERROR" ), "TEXT" ) == 0 );
		TEST( strcmp( cini.gets( "string section", "key5", "ERROR" ), "\"TEXT\"" ) == 0 );
		TEST( strcmp( cini.gets( "string section", "key6", "ERROR" ), "\"TEXT\" TEXT" ) == 0 );
		TEST( strcmp( cini.gets( "string section", "key7", "ERROR" ), "AA,BB,CC;DD,EE,FF" ) == 0 );

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
	{
		Cini cini( path );
		TEST( cini.getcount( "array section", "key1[]" ) == 4 );

		TEST( cini.geti( "array section", "key1[]", -999 ) == -999 );
		TEST( strcmp( cini.gets( "array section", "key1[]", "ERROR" ), "1,2,3," ) == 0 );

		TEST( cini.getai( "array section", "key1[]", 0, -999 ) == 1 );
		TEST( cini.getai( "array section", "key1[]", 1, -999 ) == 2 );
		TEST( cini.getai( "array section", "key1[]", 2, -999 ) == 3 );
		TEST( cini.getai( "array section", "key1[]", 3, -999 ) == -999 );
		TEST( strcmp( cini.getas( "array section", "key1[]", 3, "ERROR" ), "" ) == 0 );
		TEST( cini.getai( "array section", "key1[]", 4, -999 ) == -999 );

		TEST( cini.getcount( "array section", "key10" ) == 1 );
		TEST( strcmp( cini.gets( "array section", "key10", "ERROR" ), "1,2,3," ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key10", 0, "ERROR" ), "1,2,3," ) != 0 );

		TEST( cini.getcount( "array section", "key2[]" ) == 4 );
		TEST( strcmp( cini.getas( "array section", "key2[]", 0, "ERROR" ), "A" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key2[]", 1, "ERROR" ), "BB" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key2[]", 2, "ERROR" ), "CCC,DDDD" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key2[]", 3, "ERROR" ), "EEEEE" ) == 0 );

		TEST( cini.getcount( "array section", "key20[]" ) == 6 );
		TEST( strcmp( cini.getas( "array section", "key20[]", 0, "ERROR" ), "AA" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key20[]", 1, "ERROR" ), "BB" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key20[]", 2, "ERROR" ), "C\"C\"C , C " ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key20[]", 3, "ERROR" ), "\"D" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key20[]", 4, "ERROR" ), "E E\"E" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key20[]", 5, "ERROR" ), "F" ) == 0 );

		TEST( cini.getcount( "array section", "key21[]" ) == 5 );
		TEST( strcmp( cini.getas( "array section", "key21[]", 0, "ERROR" ), "AA" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key21[]", 1, "ERROR" ), "BB\" , \"B\"B\"B , B " ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key21[]", 2, "ERROR" ), "C, C C'C'" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key21[]", 3, "ERROR" ), "\"D'" ) == 0 );
		TEST( strcmp( cini.getas( "array section", "key21[]", 4, "ERROR" ), "F" ) == 0 );

		TEST( cini.getcount( "array section", "key3[]" ) == 1 );
		TEST( cini.geti( "array section", "key3[]", -999 ) == 1 );

		TEST( cini.getcount( "array section", "key4[100]" ) == 1 );
		TEST( strcmp( cini.gets( "array section", "key4[100]", "ERROR" ), "1,2,3" ) == 0 );
		TEST( cini.getai( "array section", "key4[100]", 0, -999 ) == -999 );

		TEST( cini.getcount( "array section", "key5[ ]" ) == 1 );
		TEST( strcmp( cini.gets( "array section", "key5[ ]", "ERROR" ), "1,2,3" ) == 0 );
		TEST( cini.getai( "array section", "key5[ ]", 0, -999 ) == -999 );

		TEST( cini.getcount( "array section", "key[]6" ) == 1 );
		TEST( strcmp( cini.gets( "array section", "key[]6", "ERROR" ), "1,2,3" ) == 0 );
		TEST( cini.getai( "array section", "key[]6", 0, -999 ) == -999 );

		TEST( cini.geti( "error", "int over", -999 ) == -999 );
		TEST( cini.geti( "error", "float over", -999 ) == -999 );
	}
	{
		Cini cini( path );
		TEST( !cini.isfailed() );
	}
	{
		Cini cini( "alkjgbak4nubiato" );
		TEST( cini.isfailed() );
	}
	{
		HCINI hcini = cini_create( path );
		TEST( cini_geti( hcini, "int section", "key1", -999 ) == 200 );
		TEST( cini_getf( hcini, "float section", "key1", -999 ) == 12.34F );
		TEST( strcmp( cini_gets( hcini, "string section", "key1", "ERROR" ), "TEXT" ) == 0 );
		TEST( cini_getcount( hcini, "array section", "key1[]" ) == 4 );
		TEST( cini_getai( hcini, "array section", "key1[]", 0, -999 ) == 1 );
		TEST( cini_getaf( hcini, "array section", "key1[]", 1, -999 ) == 2.0F );
		TEST( strcmp( cini_getas( hcini, "array section", "key1[]", 2, "ERROR" ), "3" ) == 0 );
		cini_free( hcini );
	}

	fprintf_s( stdout, "\n" );

	{
		fprintf_s( stdout, "ERRORS:\n" );
		Cini cini( path );
		print_errors( cini );
	}

	fprintf_s( stdout, "\n" );

	print_test_summary();

	append_test_result();

	::_gettchar();

	return 0;
}

