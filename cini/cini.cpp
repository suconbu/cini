//----------------------------------------------------------
// cini.cpp
// (c) 2016 suconbu
//----------------------------------------------------------

#include "stdafx.h"

#define CINI_SAFEDELETE( p )	(((p) != nullptr) ? (delete (p), (p) = nullptr) : nullptr)

#define CINI_INIT_VALUE( v ) \
	do{ \
		v.type = CiniBody::ValueType::Null; \
		v.i = 0; \
		v.f = 0.0F; \
		v.s.clear(); \
	}while(0)

#define CINI_SECTION_OPEN			"["
#define CINI_SECTION_CLOSE			"]"
#define CINI_COMMENT_CHARS			";"
#define CINI_KEY_SEPARATOR			"="
#define CINI_KEY_PROHIBIT_CHARS		"\""
#define CINI_ARRAY_SUFFIX			"[]"
#define CINI_ARRAY_SEPARATOR		","

//----------------------------------------------------------
// For debug
//----------------------------------------------------------

#define CINI_TRACE( format, ... ) \
	do { \
		OutputDebugString( Util::MakeString( format " at %s:%d\n", __VA_ARGS__, __FILE__, __LINE__ ).c_str() ); \
	} while( 0 )
#define CINI_TRACE_ERROR( format, ... ) do { CINI_TRACE( "[ERROR]" format, __VA_ARGS__ ); DebugBreak();	} while( 0 )
#define CINI_TRACE_DEBUG( format, ... ) do { CINI_TRACE( "[DEBUG]" format, __VA_ARGS__ ); } while( 0 )

#if 1
#define ASSERT( cond ) \
		do { \
			if( !(cond) ){ \
				CINI_TRACE_ERROR( _T("ASSERT at [%s] in %s:%d"), __FUNCTION__, __FILE__, __LINE__ ); \
				DebugBreak(); \
			} \
		} while( 0 )
#else
#define ASSERT( cond )
#endif

//----------------------------------------------------------
// Utility methods
//----------------------------------------------------------

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
		int buffer_size = _vscprintf( format, args ) + 1;
		char* buffer = new char[buffer_size];
		if( buffer != nullptr )
		{
			vsprintf_s( buffer, buffer_size, format, args );
			s = buffer;
			delete[] buffer;
		}
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

//----------------------------------------------------------
// CiniBody class
//----------------------------------------------------------

class CiniBody
{
public:

	enum ValueType
	{
		Null,
		Int,
		Float,
		String,
	};

	struct Value
	{
		ValueType type;
		int i;
		float f;
		std::string s;
	};

	static CiniBody* CreateFromFile( const char* path );

	int GetValueCount( const char* section_name, const char* key_name );
	Value* GetValue( const char* section_name, const char* key_name, int index );

	int GetErrorCount()
	{
		return static_cast<int>(errors_.size());
	}
	const char* GetError( int index )
	{
		return (0 <= index && index < static_cast<int>(errors_.size())) ? errors_[index].c_str() : nullptr;
	}

private:
	struct Entry
	{
		std::string key_name;
		Value value;
		bool is_array;
		std::vector<Value> values;
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
	Entry* FindEntry( const char* section_name, const char* key_name );
};

//----------------------------------------------------------
// Cini class implementation
//----------------------------------------------------------

Cini::Cini( const char* path )
{
	body_ = CiniBody::CreateFromFile( path );
}

Cini::~Cini()
{
	CINI_SAFEDELETE( body_ );
}

bool Cini::isfailed()
{
	return body_ == nullptr;
}

int Cini::getcount( const char* section, const char* key )
{
	return (body_ != nullptr) ? body_->GetValueCount( section, key ) : 0;
}

int Cini::geterrorcount()
{
	return (body_ != nullptr) ? body_->GetErrorCount() : 0;
}

const char* Cini::geterror( int index )
{
	return (body_ != nullptr) ? body_->GetError( index ) : nullptr;
}

int	Cini::geti( const char* section, const char* key, int idefault )
{
	int i = idefault;
	if( body_ != nullptr )
	{
		CiniBody::Value* value = body_->GetValue( section, key, 0 );
		if( value != nullptr )
		{
			i = value->i;
		}
	}
	return i;
}

float Cini::getf( const char* section, const char* key, float fdefault )
{
	float f = fdefault;
	if( body_ != nullptr )
	{
		CiniBody::Value* value = body_->GetValue( section, key, 0 );
		if( value != nullptr )
		{
			f = value->f;
		}
	}
	return f;
}

const char*	Cini::gets( const char* section, const char* key, const char* sdefault )
{
	const char* s = sdefault;
	if( body_ != nullptr )
	{
		CiniBody::Value* value = body_->GetValue( section, key, 0 );
		if( value != nullptr )
		{
			s = value->s.c_str();
		}
	}
	return s;
}

int Cini::getai( const char* section, const char* key, int index, int idefault )
{
	int i = idefault;
	if( body_ != nullptr )
	{
		CiniBody::Value* value = body_->GetValue( section, key, index );
		if( value != nullptr )
		{
			i = value->i;
		}
	}
	return i;
}

float Cini::getaf( const char* section, const char* key, int index, float fdefault )
{
	float f = fdefault;
	if( body_ != nullptr )
	{
		CiniBody::Value* value = body_->GetValue( section, key, index );
		if( value != nullptr )
		{
			f = value->f;
		}
	}
	return f;
}

const char*	Cini::getas( const char* section, const char* key, int index, const char* sdefault )
{
	const char* s = sdefault;
	if( body_ != nullptr )
	{
		CiniBody::Value* value = body_->GetValue( section, key, index );
		if( value != nullptr )
		{
			s = value->s.c_str();
		}
	}
	return s;
}

//----------------------------------------------------------
// C interfaces implementation
//----------------------------------------------------------

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

//----------------------------------------------------------
// CiniBody class implementation
//----------------------------------------------------------

CiniBody::CiniBody()
{
	;
}

CiniBody* CiniBody::CreateFromFile( const char* path )
{
	CiniBody* body = new CiniBody();
	if( body != nullptr )
	{
		bool result = Parser::ParseFile( path, body->sections_, body->errors_ );
		if( !result )
		{
			CINI_TRACE_DEBUG( "" );
			CINI_SAFEDELETE( body );
		}
	}
	return body;
}

int CiniBody::GetValueCount( const char* section_name, const char* key_name )
{
	int count = 0;
	Entry* entry = FindEntry( section_name, key_name );
	if( entry != nullptr )
	{
		count = entry->values.size();
	}
	return count;
}

CiniBody::Value* CiniBody::GetValue( const char* section_name, const char* key_name, int index )
{
	Value* value = nullptr;
	if( index >= 0 )
	{
		Entry* entry = FindEntry( section_name, key_name );
		if( entry != nullptr && index < static_cast<int>(entry->values.size()) )
		{
			value = &entry->values[index];
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
		CINI_TRACE_DEBUG( "" );
	}
	return result;
}

bool CiniBody::Parser::ParseLine( std::string& line )
{
	Entry* current_entry = nullptr;
	std::string token;

	ASSERT( sections_ != nullptr );
	ASSERT( errors_ != nullptr );
	ASSERT( current_section_ != nullptr );

	line = Util::Trim( line );
	if( line.length() == 0 )
	{
		// Blank line
		return true;
	}

	if( Util::StartWith( line, CINI_COMMENT_CHARS ) )
	{
		// Comment line
		return true;
	}

	if( Util::StartWith( line, CINI_SECTION_OPEN ) )
	{
		// Section
		std::string::size_type close_pos = line.find_first_of( CINI_SECTION_CLOSE, 1 );
		if( close_pos != std::string::npos )
		{
			std::string section_name( Util::Trim( line.substr( 1, close_pos - 1 ) ) );
			if( section_name.length() > 0 )
			{
				// Considering that already exist same section in the sections_.
				current_section_ = &(*sections_)[section_name];
				current_section_->name = section_name;
			}
			else
			{
				PushError( "Section name is empty, ignore the this line." );
			}
		}
		else
		{
			PushError( "']' is missing, ignore the this line." );
		}
	}
	else
	{
		// Key
		std::string::size_type separator_pos = line.find_first_of( CINI_KEY_SEPARATOR );
		if( separator_pos != std::string::npos )
		{
			std::string key_name( Util::Trim( line.substr( 0, separator_pos ) ) );
			if( key_name.length() == 0 )
			{
				PushError( "Key name is empty, ignore the this line." );
			}
			else if( key_name.find_first_of( CINI_KEY_PROHIBIT_CHARS ) != std::string::npos )
			{
				PushError( "Key name includes prohibited character(" CINI_KEY_PROHIBIT_CHARS "), ignore the this line." );
			}
			else
			{
				current_entry = &current_section_->entries[key_name];
				current_entry->key_name = key_name;
				current_entry->is_array = Util::EndWith( key_name, CINI_ARRAY_SUFFIX );

				std::string::size_type start_pos = separator_pos + 1;
				std::string::size_type end_pos = std::string::npos;
				do
				{
					end_pos = line.find( CINI_ARRAY_SEPARATOR, start_pos );
					if( end_pos == std::string::npos )
					{
						end_pos = line.length();
					}
					Value value;
					bool parse_result = ParseValue( line.substr( start_pos, end_pos - start_pos ), value );
					if( parse_result )
					{
						current_entry->values.push_back( value );
					}
					else
					{
						PushError( "Could not parsed the value." );
						break;
					}
					start_pos = end_pos + 1;
				} while( start_pos < (int)line.length() );
			}
		}
		else
		{
			PushError( "'=' is missing, ignore the this line." );
		}
	}

	return true;
}

bool CiniBody::Parser::ParseValue( std::string& token, Value& value )
{
	value.type = ValueType::Null;
	value.i = 0;
	value.f = 0.0F;

	token = Util::Trim( token );
	if( token.length() > 0 )
	{
		value.type = ValueType::String;

		char* endp = NULL;
		std::string::size_type pos = token.find_first_of( '.' );
		if( pos != std::string::npos )
		{
			// Real value
			float f = strtof( token.c_str(), &endp );
			if( *endp == '\0' )
			{
				value.type = ValueType::Float;
				value.f = f;
				value.i = (int)f;
			}
		}
		else if( token.find_first_of( "+-#0123456789" ) == 0 )
		{
			// Integer value
			int i = strtol( Util::ReplaceString( token, "#", "0x" ).c_str(), &endp, 0 );
			if( *endp == '\0' )
			{
				value.type = ValueType::Int;
				value.i = i;
				value.f = (float)i;
			}
		}
		else
		{
			;
		}
		value.s = token;
	}

	return true;
}

CiniBody::Entry * CiniBody::FindEntry( const char * section_name, const char * key_name )
{
	Entry* entry = nullptr;
	std::map<std::string, Section>::iterator section_itr = sections_.find( section_name );
	if( section_itr != sections_.end() )
	{
		std::map<std::string, Entry>& entries = section_itr->second.entries;
		std::map<std::string, Entry>::iterator key_itr = entries.find( key_name );
		if( key_itr != entries.end() )
		{
			entry = &key_itr->second;
		}
	}
	return entry;
}
