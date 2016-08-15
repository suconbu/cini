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

#ifdef CINI_WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <windows.h>
#endif //CINI_WIN32

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <vector>
#include <map>
#include <string>
#include <fstream>

#include "cini.h"

#define CINI_SAFEDELETE( p )	(((p) != NULL) ? (delete (p), (p) = NULL) : NULL)

#define CINI_LINEBUFFER_SIZE		1024
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

template<class Type>
class CiniAllocator : public std::allocator<Type>
{
public:
	CiniAllocator() {}
	CiniAllocator( const CiniAllocator& other ) {}

	template<class OtherType>
	CiniAllocator( const CiniAllocator<OtherType>& other ) {}

	pointer allocate( size_type n, const_pointer hint = 0 )
	{
		size_t size = n * sizeof( Type );
		return (pointer)std::malloc( size );
	}

	void deallocate( pointer ptr, size_type n )
	{
		std::free( ptr );
	}

	template<class OtherType>
	struct rebind
	{
		typedef CiniAllocator<OtherType> other;
	};
};

typedef std::basic_string<char, std::char_traits<char>, CiniAllocator<char>> String;
typedef std::vector<String, CiniAllocator<String>> StringVector;

//
// Utility methods
//

class Util
{
public:
	static void Trim( String& s, const char* trim_char = " \t\v\r\n" )
	{
		String::size_type left = s.find_first_not_of( trim_char );
		if( left != String::npos )
		{
			String::size_type right = s.find_last_not_of( trim_char );
			if( left > 0 )
			{
				s.assign( s.c_str() + left, right - left + 1 );
			}
			else if( (right + 1) < s.size() )
			{
				s.resize( right + 1 );
			}
			else
			{
				// Do nothing
			}
		}
	}
	static bool StartWith( String& s, const char* value )
	{
		int len = strlen( value );
		return s.compare( 0, len, value ) == 0;
	}
	static bool EndWith( String& s, const char* value )
	{
		int len = strlen( value );
		int offset = s.length() - len;
		return (offset >= 0) ? (s.compare( offset, len, value ) == 0) : false;
	}
	static String MakeString( const char* format, ... )
	{
		String s;
		va_list args;
		va_start( args, format );
		char buffer[256];
		vsnprintf( buffer, sizeof( buffer ) - 1, format, args );
		s = buffer;
		va_end( args );
		return s;
	}
	static void ReplaceString( String& s, const char* olds, const char* news )
	{
		int oldlen = strlen( olds );
		int newlen = strlen( news );
		String::size_type pos( s.find( olds ) );
		while( pos != String::npos )
		{
			s.replace( pos, oldlen, news );
			pos = s.find( olds, pos + newlen );
		}
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
		String s;

		bool IsNumeric() const
		{
			return type == ValueType_Int || type == ValueType_Float;
		}
	};
	typedef std::vector<Value, CiniAllocator<Value>> ValueVector;

	static CiniBody* CreateFromFile( const char* path );

	int GetValueCount( const char* section_name, const char* key_name ) const
	{
		int count = 0;
		const Entry* entry = FindEntry( section_name, key_name );
		if( entry != NULL )
		{
			count = entry->is_array ? entry->array_values.size() : 1;
		}
		return count;
	}

	const Value* GetValue( const char* section_name, const char* key_name ) const
	{
		const Value* value = NULL;
		const Entry* entry = FindEntry( section_name, key_name );
		if( entry != NULL )
		{
			value = &entry->value;
		}
		return value;
	}

	const Value* GetValue( const char* section_name, const char* key_name, int index ) const
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
		String key_name;
		Value value;
		bool is_array;
		ValueVector array_values;
	};
	typedef std::map<String, Entry, std::less<String>, CiniAllocator<std::pair<String, Entry>>> EntryMap;

	struct Section
	{
		String name;
		EntryMap entries;
	};
	typedef std::map<String, Section, std::less<String>, CiniAllocator<std::pair<String, Section>>> SectionMap;

	class Parser
	{
	public:
		static bool ParseFile( const char* path, SectionMap& sections, StringVector& errors );

	private:
		int line_no_;
		Section* current_section_;
		SectionMap* sections_;
		StringVector* errors_;

		Parser() {}

		bool ParseLine( String& text );
		bool ParseSection( String& text );
		bool ParseEntry( String& text );
		bool ParseArray( String& text, Entry& entry );
		bool ParseValue( String& text, Value& value );

		void PushError( const char* message )
		{
			errors_->push_back( Util::MakeString( "%s (%d)", message, line_no_ ) );
		}
	};

	SectionMap sections_;
	StringVector errors_;

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
	Cini* cini = new Cini( path );
	if( cini != 0 && cini->isfailed() )
	{
		CINI_SAFEDELETE( cini );
	}
	return cini;
}

void cini_free( HCINI hcini )
{
	CINI_SAFEDELETE( hcini );
}

int cini_getcount( HCINI hcini, const char* section, const char* key )
{
	return (hcini != 0) ? (static_cast<Cini*>(hcini))->getcount( section, key ) : 0;
}

int cini_geti( HCINI hcini, const char* section, const char* key, int idefault )
{
	return (hcini != 0) ? (static_cast<Cini*>(hcini))->geti( section, key, idefault ) : idefault;
}

float cini_getf( HCINI hcini, const char* section, const char* key, float fdefault )
{
	return (hcini != 0) ? (static_cast<Cini*>(hcini))->getf( section, key, fdefault ) : fdefault;
}

const char*	cini_gets( HCINI hcini, const char* section, const char* key, const char* sdefault )
{
	return (hcini != 0) ? (static_cast<Cini*>(hcini))->gets( section, key, sdefault ) : sdefault;
}

int cini_getai( HCINI hcini, const char* section, const char* key, int index, int idefault )
{
	return (hcini != 0) ? (static_cast<Cini*>(hcini))->getai( section, key, index, idefault ) : idefault;
}

float cini_getaf( HCINI hcini, const char* section, const char* key, int index, float fdefault )
{
	return (hcini != 0) ? (static_cast<Cini*>(hcini))->getaf( section, key, index, fdefault ) : fdefault;
}

const char*	cini_getas( HCINI hcini, const char* section, const char* key, int index, const char* sdefault )
{
	return (hcini != 0) ? (static_cast<Cini*>(hcini))->getas( section, key, index, sdefault ) : sdefault;
}

int cini_geterrorcount( HCINI hcini )
{
	return (hcini != 0) ? (static_cast<Cini*>(hcini))->geterrorcount() : 0;
}

const char* cini_geterror( HCINI hcini, int index )
{
	return (hcini != 0) ? (static_cast<Cini*>(hcini))->geterror( index ) : 0;
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

bool CiniBody::Parser::ParseFile( const char* path, SectionMap& sections, StringVector& errors )
{
	std::ifstream ifs( path );

	if( ifs.fail() )
	{
		CINI_TRACE( "Fail to open file" );
		return false;
	}

	Parser parser;
	parser.line_no_ = 1;
	parser.sections_ = &sections;
	parser.errors_ = &errors;

	parser.current_section_ = &(*parser.sections_)[""];
	parser.current_section_->name = "";

	std::vector<char> buffer( CINI_LINEBUFFER_SIZE );

	String line;
	while( ifs.getline( &buffer[0], buffer.capacity() ) )
	{
		line.assign( &buffer[0] );
		parser.ParseLine( line );
		parser.line_no_++;
	}

	return true;
}

bool CiniBody::Parser::ParseLine( String& text )
{
	CINI_ASSERT( sections_ != NULL );
	CINI_ASSERT( errors_ != NULL );
	CINI_ASSERT( current_section_ != NULL );

	Util::Trim( text );

	if( text.length() == 0 )
	{
		// Blank line
		return true;
	}

	if( Util::StartWith( text, CINI_COMMENT_CHARS ) )
	{
		// Comment line
		return true;
	}

	if( Util::StartWith( text, CINI_SECTION_OPEN ) )
	{
		return ParseSection( text );
	}
	else
	{
		return ParseEntry( text );
	}
}

bool CiniBody::Parser::ParseSection( String& text )
{
	String::size_type close_pos = text.find( CINI_SECTION_CLOSE, 1 );
	if( close_pos == String::npos )
	{
		PushError( Util::MakeString( "'%s' is missing, ignore the this line.", CINI_SECTION_CLOSE ).c_str() );
		return false;
	}

	String section_name( text.c_str() + 1, close_pos - 1 );
	Util::Trim( section_name );
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

bool CiniBody::Parser::ParseEntry( String& text )
{
	String::size_type separator_pos = text.find( CINI_ASSIGNMENT_OP );
	if( separator_pos == String::npos )
	{
		PushError( "'=' is missing, ignore the this line." );
		return false;
	}

	String key_name( text.c_str(), separator_pos );
	Util::Trim( key_name );
	if( key_name.length() == 0 )
	{
		PushError( "Key name is empty, ignore the this line." );
		return false;
	}
	if( key_name.find_first_of( CINI_KEY_PROHIBIT_CHARS ) != String::npos )
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

	String::size_type start_pos = separator_pos + 1;
	String::size_type end_pos = text.length();

	Value value;
	String token( text.c_str() + start_pos, end_pos - start_pos );
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

bool CiniBody::Parser::ParseArray( String& text, Entry& entry )
{
	String::size_type start_pos = 0;
	do
	{
		int openQuote = '\0';
		bool inQuote = false;
		String::size_type pos = start_pos;
		String::size_type end_pos = text.length();
		String::size_type firstSeparatorPos = String::npos;

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
					if( firstSeparatorPos == String::npos )
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

		if( inQuote && firstSeparatorPos != String::npos )
		{
			end_pos = firstSeparatorPos;
		}

		Value value;
		String token( text.c_str() + start_pos, end_pos - start_pos );
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

bool CiniBody::Parser::ParseValue( String& token, Value& value )
{
	Util::Trim( token );

	value.type = ValueType_String;
	value.i = 0;
	value.f = 0.0F;

	if( token.find_first_of( "+-#0123456789" ) == 0 )
	{
		char* endp = NULL;

		// Integer number?
		int i = 0;
		size_t pos = token.find( '#' );
		if( pos != String::npos )
		{
			token[pos] = '0';
			i = strtol( token.c_str(), &endp, 16 );
			token[pos] = '#';
		}
		else
		{
			i = strtol( token.c_str(), &endp, 0 );
		}
		if( *endp == '\0' )
		{
			value.type = ValueType_Int;
			value.i = i;
			value.f = static_cast<float>(i);
			value.s = token;
		}
		else
		{
			// Real number?
			float f = static_cast<float>(strtod( token.c_str(), &endp ));
			if( *endp == '\0' )
			{
				value.type = ValueType_Float;
				value.f = f;
				value.i = static_cast<int>(f);
				value.s = token;
			}
		}
	}

	if( value.type == ValueType_String )
	{
		if( token.length() > 2 &&
			(token[0] == CINI_STRING_QUOTE1 || token[0] == CINI_STRING_QUOTE2) &&
			token[0] == token[token.size() - 1] )
		{
			// Remove the quote mark of both ends
			value.s.assign( token.c_str() + 1, token.length() - 2 );
		}
		else
		{
			value.s = token;
		}
	}

	return true;
}

const CiniBody::Entry* CiniBody::FindEntry( const char * section_name, const char * key_name ) const
{
	const Entry* entry = NULL;
	SectionMap::const_iterator section_itr = sections_.find( section_name );
	if( section_itr != sections_.end() )
	{
		const EntryMap::const_iterator entry_itr = section_itr->second.entries.find( key_name );
		if( entry_itr != section_itr->second.entries.end() )
		{
			entry = &entry_itr->second;
		}
	}
	return entry;
}
