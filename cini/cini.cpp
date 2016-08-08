//
// cini.cpp - Implement the APIs
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#ifdef CINI_WIN32
#include <windows.h>
#endif //CINI_WIN32

#include "cini.h"

#define CINI_SAFEDELETE( p )	(((p) != NULL) ? (delete (p), (p) = NULL) : NULL)

#define CINI_SECTION_OPEN			"["
#define CINI_SECTION_CLOSE			"]"
#define CINI_COMMENT_CHARS			";"
#define CINI_KEY_PROHIBIT_CHARS		"\""
#define CINI_ASSIGNMENT_OP			"="
#define CINI_ARRAY_SUFFIX			"[]"
#define CINI_ARRAY_SEPARATOR		','
#define CINI_STRING_QUOTE1			'"'
#define CINI_STRING_QUOTE2			'\''

//
// For debug
//

#ifdef CINI_WIN32
#define CINI_TRACE( format, ... ) \
	do { \
		OutputDebugString( Util::MakeString( format " at %s %s:%d\n", __VA_ARGS__, __FUNCTION__, __FILE__, __LINE__ ).c_str() ); \
	} while( 0 )
#define CINI_ASSERT( expr ) \
	do { \
		if( !(expr) ){ \
			CINI_TRACE( "ASSERT!!!" ); \
			DebugBreak(); \
		} \
	} while( 0 )
#else //CINI_WIN32
#define CINI_TRACE( format, ... )
#define CINI_ASSERT( expr )
#endif //CINI_WIN32

//
// Utility methods
//

class Util
{
public:
	static std::string Trim( std::string& s, const char* trim_char = " \t\v\r\n" )
	{
		// http://program.station.ez-net.jp/special/handbook/cpp/std::string/trim.asp
		std::string result;
		std::string::size_type left = s.find_first_not_of( trim_char );
		if( left != std::string::npos )
		{
			std::string::size_type right = s.find_last_not_of( trim_char );
			result = s.substr( left, right - left + 1 );
		}
		return result;
	}
	static bool StartWith( std::string& s, const char* value )
	{
		int len = strlen( value );
		return s.compare( 0, len, value ) == 0;
	}
	static bool EndWith( std::string& s, const char* value )
	{
		int len = strlen( value );
		int offset = s.length() - len;
		return (offset >= 0) ? (s.compare( offset, len, value ) == 0) : false;
	}
	static std::string MakeString( const char* format, ... )
	{
		// https://msdn.microsoft.com/ja-jp/library/xa1a1a6z.aspx
		std::string s;
		va_list args;
		va_start( args, format );
		char buffer[256];
		vsnprintf( buffer, sizeof(buffer) - 1, format, args );
		s = buffer;
		va_end( args );
		return s;
	}
	static std::string ReplaceString( std::string& s, const char* olds, const char* news )
	{
		// http://www.geocities.jp/eneces_jupiter_jp/cpp1/010-055.html
		int oldlen = strlen( olds );
		int newlen = strlen( news );
		std::string r( s );
		std::string::size_type pos( r.find( olds ) );
		while( pos != std::string::npos )
		{
			r.replace( pos, oldlen, news );
			pos = r.find( olds, pos + newlen );
		}
		return r;
	}
};

//
// CiniBody class
//

class CiniBody
{
public:

	enum ValueType
	{
		ValueType_Int,
		ValueType_Float,
		ValueType_String,
	};

	struct Value
	{
		ValueType type;
		int i;
		float f;
		std::string s;

		bool IsNumeric() const
		{
			return type == ValueType_Int || type == ValueType_Float;
		}
	};

	static CiniBody* CreateFromFile( const char* path );

	int GetValueCount( const char* section_name, const char* key_name ) const;
	const Value* GetValue( const char* section_name, const char* key_name ) const;
	const Value* GetValue( const char* section_name, const char* key_name, int index ) const;

	int GetErrorCount() const
	{
		return static_cast<int>(errors_.size());
	}
	const char* GetError( int index ) const
	{
		return (0 <= index && index < static_cast<int>(errors_.size())) ? errors_[index].c_str() : NULL;
	}

private:
	struct Entry
	{
		std::string key_name;
		Value value;
		bool is_array;
		std::vector<Value> array_values;
	};

	struct Section
	{
		std::string name;
		std::map<std::string, Entry> entries;
	};

	class Parser
	{
	public:
		static bool ParseFile( const char* path, std::map<std::string, Section>& sections, std::vector<std::string>& errors );

	private:
		int line_no_;
		Section* current_section_;
		std::map<std::string, Section>* sections_;
		std::vector<std::string>* errors_;
		
		Parser(){}

		bool ParseLine( std::string& text );
		bool ParseSection( std::string& text );
		bool ParseEntry( std::string& text );
		bool ParseArray( std::string& text, Entry& entry );
		bool ParseValue( std::string& text, Value& value );

		void PushError( const char* message )
		{
			errors_->push_back( Util::MakeString( "%s (%d)", message, line_no_ ) );
		}
	};

	std::map<std::string, Section> sections_;
	std::vector<std::string> errors_;
	const char** error_messages_;

	CiniBody();
	const Entry* FindEntry( const char* section_name, const char* key_name ) const;
};

//
// Cini class implementation
//

Cini::Cini( const char* path )
{
	body_ = CiniBody::CreateFromFile( path );
}

Cini::~Cini()
{
	CINI_SAFEDELETE( body_ );
}

bool Cini::isfailed() const
{
	return body_ == NULL;
}

int Cini::getcount( const char* section, const char* key ) const
{
	return (body_ != NULL) ? body_->GetValueCount( section, key ) : 0;
}

int Cini::geterrorcount() const
{
	return (body_ != NULL) ? body_->GetErrorCount() : 0;
}

const char* Cini::geterror( int index ) const
{
	return (body_ != NULL) ? body_->GetError( index ) : NULL;
}

int	Cini::geti( const char* section, const char* key, int idefault ) const
{
	int i = idefault;
	if( body_ != NULL )
	{
		const CiniBody::Value* value = body_->GetValue( section, key );
		if( value != NULL && value->IsNumeric() )
		{
			i = value->i;
		}
	}
	return i;
}

float Cini::getf( const char* section, const char* key, float fdefault ) const
{
	float f = fdefault;
	if( body_ != NULL )
	{
		const CiniBody::Value* value = body_->GetValue( section, key );
		if( value != NULL && value->IsNumeric() )
		{
			f = value->f;
		}
	}
	return f;
}

const char*	Cini::gets( const char* section, const char* key, const char* sdefault ) const
{
	const char* s = sdefault;
	if( body_ != NULL )
	{
		const CiniBody::Value* value = body_->GetValue( section, key );
		if( value != NULL )
		{
			s = value->s.c_str();
		}
	}
	return s;
}

int Cini::getai( const char* section, const char* key, int index, int idefault ) const
{
	int i = idefault;
	if( body_ != NULL )
	{
		const CiniBody::Value* value = body_->GetValue( section, key, index );
		if( value != NULL && value->IsNumeric() )
		{
			i = value->i;
		}
	}
	return i;
}

float Cini::getaf( const char* section, const char* key, int index, float fdefault ) const
{
	float f = fdefault;
	if( body_ != NULL )
	{
		const CiniBody::Value* value = body_->GetValue( section, key, index );
		if( value != NULL && value->IsNumeric() )
		{
			f = value->f;
		}
	}
	return f;
}

const char*	Cini::getas( const char* section, const char* key, int index, const char* sdefault ) const
{
	const char* s = sdefault;
	if( body_ != NULL )
	{
		const CiniBody::Value* value = body_->GetValue( section, key, index );
		if( value != NULL )
		{
			s = value->s.c_str();
		}
	}
	return s;
}

//
// C API implementation
//

HCINI cini_create( const char* path )
{
	return new Cini( path );
}

void cini_free( HCINI hcini )
{
	CINI_SAFEDELETE( hcini );
}

int cini_getcount( HCINI hcini, const char* section, const char* key )
{
	return (hcini != 0) ? ((Cini*)(hcini))->getcount( section, key ) : 0;
}

int cini_geti( HCINI hcini, const char* section, const char* key, int idefault )
{
	return (hcini != 0) ? ((Cini*)(hcini))->geti( section, key, idefault ) : idefault;
}

float cini_getf( HCINI hcini, const char* section, const char* key, float fdefault )
{
	return (hcini != 0) ? ((Cini*)(hcini))->getf( section, key, fdefault ) : fdefault;
}

const char*	cini_gets( HCINI hcini, const char* section, const char* key, const char* sdefault )
{
	return (hcini != 0) ? ((Cini*)(hcini))->gets( section, key, sdefault ) : sdefault;
}

int cini_getai( HCINI hcini, const char* section, const char* key, int index, int idefault )
{
	return (hcini != 0) ? ((Cini*)(hcini))->getai( section, key, index, idefault ) : idefault;
}

float cini_getaf( HCINI hcini, const char* section, const char* key, int index, float fdefault )
{
	return (hcini != 0) ? ((Cini*)(hcini))->getaf( section, key, index, fdefault ) : fdefault;
}

const char*	cini_getas( HCINI hcini, const char* section, const char* key, int index, const char* sdefault )
{
	return (hcini != 0) ? ((Cini*)(hcini))->getas( section, key, index, sdefault ) : sdefault;
}

int cini_geterrorcount( HCINI hcini )
{
	return (hcini != 0) ? ((Cini*)(hcini))->geterrorcount() : 0;
}

const char* cini_geterror( HCINI hcini, int index )
{
	return (hcini != 0) ? ((Cini*)(hcini))->geterror( index ) : 0;
}

//
// CiniBody class implementation
//

CiniBody::CiniBody()
{
	;
}

CiniBody* CiniBody::CreateFromFile( const char* path )
{
	CiniBody* body = new CiniBody();
	if( body != NULL )
	{
		bool result = Parser::ParseFile( path, body->sections_, body->errors_ );
		if( !result )
		{
			CINI_TRACE( "Fail to parse file" );
			CINI_SAFEDELETE( body );
		}
	}
	return body;
}

int CiniBody::GetValueCount( const char* section_name, const char* key_name ) const
{
	int count = 0;
	const Entry* entry = FindEntry( section_name, key_name );
	if( entry != NULL )
	{
		count = entry->is_array ? entry->array_values.size() : 1;
	}
	return count;
}

const CiniBody::Value* CiniBody::GetValue( const char* section_name, const char* key_name ) const
{
	const Value* value = NULL;
	const Entry* entry = FindEntry( section_name, key_name );
	if( entry != NULL )
	{
		value = &entry->value;
	}
	return value;
}

const CiniBody::Value* CiniBody::GetValue( const char* section_name, const char* key_name, int index ) const
{
	const Value* value = NULL;
	if( index >= 0 )
	{
		const Entry* entry = FindEntry( section_name, key_name );
		if( entry != NULL && entry->is_array && index < static_cast<int>(entry->array_values.size()) )
		{
			value = &entry->array_values[index];
		}
	}
	return value;
}

bool CiniBody::Parser::ParseFile( const char* path, std::map<std::string, Section>& sections, std::vector<std::string>& errors )
{
	bool result = false;
	std::ifstream ifs( path );

	if( !ifs.fail() )
	{
		Parser parser;
		parser.line_no_ = 1;
		parser.sections_ = &sections;
		parser.errors_ = &errors;

		parser.current_section_ = &(*parser.sections_)[""];
		parser.current_section_->name = "";

		std::string line;
		while( std::getline( ifs, line ) )
		{
			parser.ParseLine( line );
			parser.line_no_++;
		}

		result = true;
	}
	else
	{
		CINI_TRACE( "Fail to open file" );
	}
	return result;
}

bool CiniBody::Parser::ParseLine( std::string& text )
{
	CINI_ASSERT( sections_ != NULL );
	CINI_ASSERT( errors_ != NULL );
	CINI_ASSERT( current_section_ != NULL );

	std::string t = Util::Trim( text );
	if( t.length() == 0 )
	{
		// Blank line
		return true;
	}

	if( Util::StartWith( t, CINI_COMMENT_CHARS ) )
	{
		// Comment line
		return true;
	}

	if( Util::StartWith( t, CINI_SECTION_OPEN ) )
	{
		return ParseSection( t );
	}
	else
	{
		return ParseEntry( t );
	}
}

bool CiniBody::Parser::ParseSection( std::string& text )
{
	std::string::size_type close_pos = text.find_first_of( CINI_SECTION_CLOSE, 1 );
	if( close_pos == std::string::npos )
	{
		PushError( "']' is missing, ignore the this line." );
		return false;
	}

	std::string section_name = text.substr( 1, close_pos - 1 );
	section_name = Util::Trim( section_name );
	if( section_name.length() == 0 )
	{
		PushError( "Section name is empty, ignore the this line." );
		return false;
	}

	// Considering that already exist same section in the sections_.
	current_section_ = &(*sections_)[section_name];
	current_section_->name = section_name;

	return true;
}

bool CiniBody::Parser::ParseEntry( std::string& text )
{
	std::string::size_type separator_pos = text.find( CINI_ASSIGNMENT_OP );
	if( separator_pos == std::string::npos )
	{
		PushError( "'=' is missing, ignore the this line." );
		return false;
	}

	std::string key_name = text.substr( 0, separator_pos );
	key_name = Util::Trim( key_name );
	if( key_name.length() == 0 )
	{
		PushError( "Key name is empty, ignore the this line." );
		return false;
	}
	if( key_name.find_first_of( CINI_KEY_PROHIBIT_CHARS ) != std::string::npos )
	{
		PushError( "Key name includes prohibited character(" CINI_KEY_PROHIBIT_CHARS "), ignore the this line." );
		return false;
	}

	if( current_section_->entries.count( key_name ) > 0 )
	{
		PushError( "Re-defined key, ignore the this line." );
		return false;
	}

	Entry* current_entry = &current_section_->entries[key_name];
	current_entry->key_name = key_name;
	current_entry->is_array = Util::EndWith( key_name, CINI_ARRAY_SUFFIX );

	std::string::size_type start_pos = separator_pos + 1;
	std::string::size_type end_pos = text.length();

	Value value;
	std::string token = text.substr( start_pos, end_pos - start_pos );
	bool parse_result = ParseValue( token, value );
	if( parse_result )
	{
		current_entry->value = value;
	}
	else
	{
		PushError( Util::MakeString( "Could not parsed '%s'.", token.c_str() ).c_str() );
	}

	if( current_entry->is_array )
	{
		ParseArray( token, *current_entry );
	}

	return true;
}

bool CiniBody::Parser::ParseArray( std::string& text, Entry& entry )
{
	std::string::size_type start_pos = 0;
	do
	{
		int openQuote = '\0';
		bool inQuote = false;
		std::string::size_type pos = start_pos;
		std::string::size_type end_pos = text.length();
		std::string::size_type firstSeparatorPos = std::string::npos;

		while( pos < text.length() )
		{
			int c = text[pos];
			if( isspace( c ) == 0 )
			{
				if( c == CINI_STRING_QUOTE1 || c == CINI_STRING_QUOTE2 )
				{
					openQuote = text[pos];
					inQuote = true;
				}
				pos++;
				break;
			}
			pos++;
		}
		while( pos < text.length() )
		{
			int c = text[pos];
			if( isspace( c ) != 0 )
			{
				;
			}
			else if( text[pos] == CINI_ARRAY_SEPARATOR )
			{
				if( !inQuote )
				{
					end_pos = pos;
					break;
				}
				else
				{
					if( firstSeparatorPos == std::string::npos )
					{
						// Save the separator position for if could not found the closing quote.
						firstSeparatorPos = pos;
					}
				}
			}
			else
			{
				if( openQuote != '\0' )
				{
					if( c == openQuote )
					{
						inQuote = false;
					}
					else
					{
						inQuote = true;
					}
				}
			}
			pos++;
		}

		if( inQuote && firstSeparatorPos != std::string::npos )
		{
			end_pos = firstSeparatorPos;
		}

		Value value;
		std::string token = text.substr( start_pos, end_pos - start_pos );
		bool parse_result = ParseValue( token, value );
		if( parse_result )
		{
			entry.array_values.push_back( value );
		}
		else
		{
			PushError( Util::MakeString( "Could not parsed '%s'.", text.c_str() ).c_str() );
		}

		start_pos = end_pos + 1;

		// Pick up the empty string on end of line.
		// ex. key = 1,2,3, <<< number of elements is 4.
	} while( start_pos <= text.length() );

	return true;
}

bool CiniBody::Parser::ParseValue( std::string& token, Value& value )
{
	std::string s = Util::Trim( token );

	value.type = ValueType_String;
	value.i = 0;
	value.f = 0.0F;
	value.s = s;

	if( s.find_first_of( "+-#0123456789" ) == 0 )
	{
		char* endp = NULL;

		// Integer number?
		int i = strtol( Util::ReplaceString( s, "#", "0x" ).c_str(), &endp, 0 );
		if( *endp == '\0' )
		{
			value.type = ValueType_Int;
			value.i = i;
			value.f = static_cast<float>(i);
		}
		else
		{
			// Real number?
			float f = strtof( s.c_str(), &endp );
			if( *endp == '\0' )
			{
				value.type = ValueType_Float;
				value.f = f;
				value.i = static_cast<int>(f);
			}
		}
	}

	if( value.type == ValueType_String )
	{
		if( s.length() > 2 )
		{
			if( (s[0] == CINI_STRING_QUOTE1 && s[s.length() - 1] == CINI_STRING_QUOTE1) ||
				(s[0] == CINI_STRING_QUOTE2 && s[s.length() - 1] == CINI_STRING_QUOTE2) )
			{
				// Remove the quote mark of both ends
				value.s = s.substr( 1, s.length() - 2 );
			}
		}
	}

	return true;
}

const CiniBody::Entry* CiniBody::FindEntry( const char * section_name, const char * key_name ) const
{
	const Entry* entry = NULL;
	std::map<std::string, Section>::const_iterator section_itr = sections_.find( section_name );
	if( section_itr != sections_.end() )
	{
		const std::map<std::string, Entry>::const_iterator entry_itr = section_itr->second.entries.find( key_name );
		if( entry_itr != section_itr->second.entries.end() )
		{
			entry = &entry_itr->second;
		}
	}
	return entry;
}
