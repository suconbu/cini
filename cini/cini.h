//----------------------------------------------------------
// cini.h
//
// (c) 2016 suconbu
//----------------------------------------------------------

#pragma once

#ifdef __cplusplus

// For C++ API's

class Cini
{
public:
	Cini( const char* path );
	~Cini();

	int         geti( const char* section, const char* key, int idefault = 0 );
	float       getf( const char* section, const char* key, float fdefault = 0.0F );
	const char* gets( const char* section, const char* key, const char* sdefault = "" );

	int         getai( const char* section, const char* key, int index, int idefault = 0 );
	float       getaf( const char* section, const char* key, int index, float fdefault = 0.0F );
	const char* getas( const char* section, const char* key, int index, const char* sdefault = "" );

	int getcount( const char* section, const char* key );

	int geterrorcount();
	const char* geterror( int index );

private:
	class CiniBody* body_;
};

typedef Cini* HCINI;

extern "C" {

#else

typedef void* HCINI;

#endif

// For C API's

// Parsing ini file and associate it to HCINI handle.
///	@param[in]	path	
///	@retval		!=NULL	A handle for access to ini file
///	@retval		==NULL	Failure to open or parse ini file
HCINI cini_create( const char* path );

// Release resources.
void cini_free( HCINI hcini );

// The function return the default value if could not find section or key.
// If you use these functions to the array, returns the first data.
int         cini_geti( HCINI hcini, const char* section, const char* key, int idefault );
float       cini_getf( HCINI hcini, const char* section, const char* key, float fdefault );
const char* cini_gets( HCINI hcini, const char* section, const char* key, const char* sdefault );

// Array accessors.
int         cini_getai( HCINI hcini, const char* section, const char* key, int index, int idefault );
float       cini_getaf( HCINI hcini, const char* section, const char* key, int index, float fdefault );
const char* cini_getas( HCINI hcini, const char* section, const char* key, int index, const char* sdefault );

// Non array key     : 1
// Array type key    : Number of elements
// Not found the key : 0
int cini_getcount( HCINI hcini, const char* section, const char* key );

// int num_errors = cini_errorcount();
// for( int i = 0; i < num_errors; i++ )
// {
//     printf( "%s\n", cini_geterror( i ) );
// }
int cini_geterrorcount( HCINI hcini );
const char* cini_geterror( HCINI hcini, int index );

//const char* const* errors = NULL;
//int num_errors = cini_geterrors( hcini, &errors );
//for( i = 0; i < num_errors; i++ )
//{
//	printf( "%s\n", errors[i] );
//}
//int cini_geterrors( HCINI hcini, const char* const** errors );

#ifdef __cplusplus
}
#endif
