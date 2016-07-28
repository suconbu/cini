//
// cini.h - Comfortable ini parser for C/C++
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

#pragma once

#ifndef _CINI_H_
#define _CINI_H_

#ifdef __cplusplus

#define CINI_VERSION	"1.0.0"

//
// API for C++
//

class Cini
{
public:
	Cini( const char* path );
	~Cini();

	// If failed to load the ini file, the function returns true
	bool isfailed() const;

	// Get the value of indicated section and key
	// The function returns default value if could not find the entry or the value type was mismatch
	int         geti( const char* section, const char* key, int idefault = 0 ) const;
	float       getf( const char* section, const char* key, float fdefault = 0.0F ) const;
	const char* gets( const char* section, const char* key, const char* sdefault = "" ) const;

	// Array accessors
	int         getai( const char* section, const char* key, int index, int idefault = 0 ) const;
	float       getaf( const char* section, const char* key, int index, float fdefault = 0.0F ) const;
	const char* getas( const char* section, const char* key, int index, const char* sdefault = "" ) const;

	// Get number of array elements
	int getcount( const char* section, const char* key ) const;

	// Get error information which recorded when parsing ini file
	//
	// Usage example:
	//
	//   error_count = cini.geterrorcount();
	//   for( i = 0; i < error_count; i++ )
	//   {
	//       printf( "%s\n", cini.geterror( i ) );
	//   }
	//
	int         geterrorcount() const;
	const char* geterror( int index ) const;

private:
	class CiniBody* body_;

	Cini( const Cini& other );
	Cini& operator=( const Cini& other );
};

typedef Cini* HCINI;

extern "C" {

#else //__cplusplus

typedef void* HCINI;

#endif //__cplusplus

//
// API for C
//

// Parse ini file and associate it to HCINI handle
HCINI cini_create( const char* path );

// Release resources
void cini_free( HCINI hcini );

// Get the value of indicated section and key
// The function returns default value if could not find the entry or the value type was mismatch
int         cini_geti( HCINI hcini, const char* section, const char* key, int idefault );
float       cini_getf( HCINI hcini, const char* section, const char* key, float fdefault );
const char* cini_gets( HCINI hcini, const char* section, const char* key, const char* sdefault );

// Array accessors
int         cini_getai( HCINI hcini, const char* section, const char* key, int index, int idefault );
float       cini_getaf( HCINI hcini, const char* section, const char* key, int index, float fdefault );
const char* cini_getas( HCINI hcini, const char* section, const char* key, int index, const char* sdefault );

// Get number of array elements
int cini_getcount( HCINI hcini, const char* section, const char* key );

// Get error information which recorded when parsing ini file
int         cini_geterrorcount( HCINI hcini );
const char* cini_geterror( HCINI hcini, int index );

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_CINI_H_
