//
// test.h - Function prototypes for test
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

#ifndef _TEST_H_
#define _TEST_H_

#define TEST( cond )					print_test_result( (int)(cond), #cond, __LINE__ )
#define TEST_PRINT( file, format, ... )	fprintf( file, format, __VA_ARGS__ )

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

void test_c( const char* path );
void test_cpp( const char* path );

void print_test_result( int cond, const char* cond_text, int line );
void print_test_summary( long long int elapsed_nanosec );
void append_test_result();
long long int get_nanosec();

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_TEST_H_
