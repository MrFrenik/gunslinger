#ifndef __GS_LEXER_H__
#define __GS_LEXER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gs_types.h"
#include "common/gs_util.h"

#define parser_debug_enabled 0

/*==============================
// GS_Token
==============================*/

typedef struct
{
	const char* text;
	const char* type;				// Would prefer this as an enum
	u32 len;
} gs_token;

_inline gs_token gs_token_invalid_token()
{
	gs_token t;
	t.text = "";
	t.type = "_invalid_";
	return t;
}

_inline b8 gs_token_compare_type( gs_token t, const char* match_type )
{
	return ( gs_string_compare_equal( t.type, match_type ) );
}

_inline b8 gs_token_compare_text( gs_token t, const char* match_text )
{
	return ( gs_string_compare_equal_n( t.text, match_text, t.len ) );
}

_inline void gs_token_print_text( gs_token t )
{
	gs_printf( "%.*s\n", t.len, t.text );
}

_inline void gs_token_debug_print( gs_token t )
{
	gs_printf( "%s: %.*s\n", t.type, t.len, t.text );
}

_inline b8 gs_token_is_end_of_line( char c )
{
	return ( c == '\n' || c == '\r' );
}

_inline b8 gs_token_char_is_white_space( char c )
{
	return (
		c == '\t' 	||
		c == ' '	||
		gs_token_is_end_of_line( c )
	);
}

_inline b8 gs_token_char_is_alpha( char c )
{
	return ( 
		( c >= 'a' && c <= 'z' ) ||
		( c >= 'A' && c <= 'Z' ) 
	);
}

_inline b8 
gs_token_char_is_numeric( char c )
{
	return ( c >= '0' && c <= '9' );
} 

/*==============================
// GS_Lexer
==============================*/

typedef struct gs_lexer
{
	const char* at;
	const char* contents;
	gs_token current_token;

	gs_token ( * next_token )( struct gs_lexer* );
} gs_lexer;

typedef struct
{
	gs_lexer _base;
} gs_lexer_c;

_inline b8 gs_lexer_can_lex( gs_lexer* lex )
{
	char c = *lex->at;
	return ( lex->at != NULL && *( lex->at ) != '\0' );
}

// Assumes that container and ignore list are set
_inline void gs_lexer_set_contents( gs_lexer* lex, const char* contents )
{
	lex->at = contents;
	lex->current_token = gs_token_invalid_token();
}

_inline void gs_lexer_advance_position( gs_lexer* lex, usize advance )
{
	lex->at += advance;
}

_inline gs_token gs_lexer_next_token( gs_lexer* lex )
{
	gs_token t = lex->next_token( lex );
	lex->current_token = t;
	return t;
}

_inline void gs_lexer_eat_whitespace( gs_lexer* lex )
{
	for ( ;; )
	{
		if ( gs_token_char_is_white_space( lex->at[ 0 ] ) )
		{
			lex->at++;
		}

		// Single line comment
		else if ( ( lex->at[ 0 ] == '/' ) && ( lex->at[ 1 ] ) && ( lex->at[ 1 ] == '/' ) )
		{
			lex->at += 2;
			while ( lex->at[ 0 ] && !gs_token_is_end_of_line( lex->at[ 0 ] ) )
			{
				lex->at++;
			}
		}

		// Multi line comment
		else if ( ( lex->at[ 0 ] == '/' ) && ( lex->at[ 1 ] ) && ( lex->at[ 1 ] == '*' ) )
		{
			lex->at += 2;
			while ( lex->at[ 0 ] && lex->at[ 1 ] && !( lex->at[ 0 ] == '*' && lex->at[ 1 ] == '/' ) )
			{
				lex->at++;
			}
			if ( lex->at[ 0 ] == '*' )
			{
				lex->at++;
			}
		}

		else
		{
			break;
		}
	}
}

// Explicit tokenizing ( not using regex )
_inline gs_token gs_lexer_c_next_token( gs_lexer* lex )
{
	// Eat all white space
	// gs_lexer_eat_whitespace( lex );

	gs_token t = gs_token_invalid_token();
	t.text = lex->at;
	t.len = 1;

	if ( gs_lexer_can_lex( lex ) )
	{
		char c = lex->at[ 0 ];
		switch( c )
		{
			case '(': { t.type = "lparen"; lex->at++; } 		break;
			case ')': { t.type = "rparen"; lex->at++; } 		break;
			case '<': { t.type = "lthan"; lex->at++; } 			break;
			case '>': { t.type = "gthan"; lex->at++; } 			break;
			case ';': { t.type = "semi_colon"; lex->at++; } 	break;
			case ':': { t.type = "colon"; lex->at++; }			break;
			case ',': { t.type = "comma"; lex->at++; }			break;
			case '=': { t.type = "equal"; lex->at++; }			break;
			case '!': { t.type = "not"; lex->at++; } 			break;
			case '#': { t.type = "hash"; lex->at++; }			break;
			case '|': { t.type = "pipe"; lex->at++; }			break;
			case '&': { t.type = "ampersand"; lex->at++; }		break;
			case '{': { t.type = "lbrace"; lex->at++; } 		break;
			case '}': { t.type = "rbrace"; lex->at++; } 		break;
			case '[': { t.type = "lbracket"; lex->at++; } 		break;
			case ']': { t.type = "rbracket"; lex->at++; } 		break;
			case '-': { t.type = "minus"; lex->at++; } 			break;
			case '+': { t.type = "plus"; lex->at++; } 			break;
			case '*': { t.type = "asterisk"; lex->at++; } 		break;
			case '\\': { t.type = "bslash"; lex->at++; } 		break;
			case '?': { t.type = "qmark"; lex->at++; } 			break;
			case ' ': { t.type = "space"; lex->at++; }			break;
			case '\n': { t.type = "newline"; lex->at++; }		break;
			case '\r': { t.type = "newline"; lex->at++; }		break;
			case '\t': { t.type = "tab"; lex->at++; } 			break;

			case '/': 
			{
				// Single line comment
				if ( ( lex->at[ 0 ] == '/' ) && ( lex->at[ 1 ] ) && ( lex->at[ 1 ] == '/' ) )
				{
					lex->at += 2;
					while ( lex->at[ 0 ] && !gs_token_is_end_of_line( lex->at[ 0 ] ) )
					{
						lex->at++;
					}
					t.len = lex->at - t.text;
					t.type = "single_line_comment";
				}

				// Multi line comment
				else if ( ( lex->at[ 0 ] == '/' ) && ( lex->at[ 1 ] ) && ( lex->at[ 1 ] == '*' ) )
				{
					lex->at += 2;
					while ( lex->at[ 0 ] && lex->at[ 1 ] && !( lex->at[ 0 ] == '*' && lex->at[ 1 ] == '/' ) )
					{
						lex->at++;
					}
					if ( lex->at[ 0 ] == '*' )
					{
						lex->at++;
					}
					t.len = lex->at - t.text;
					t.type = "multi_line_comment";
				}

			} break;

			case '"':
			{
				// Move forward after finding first quotation
				lex->at++;

				while
				(
					lex->at[ 0 ] &&
					lex->at[ 0 ] != '"'
				)
				{
					if 
					( 
						lex->at[ 0 ] == '\\' && 
						lex->at[ 1 ] 
					)
					{
						lex->at++;
					}

					lex->at++;				
				}

				// Move past the quotation
				lex->at++;

				t.len = lex->at - t.text;
				t.type = "string";
			} break;

			default:
			{
				if ( ( gs_token_char_is_alpha( c ) || c == '_' ) && c != '-' )
				{
					while 
					( 
						gs_token_char_is_alpha( lex->at[ 0 ] ) || 
						gs_token_char_is_numeric( lex->at[ 0 ] ) || 
						lex->at[ 0 ] == '_'
					)
					{
						lex->at++;
					}

					t.len = lex->at - t.text;
					t.type = "identifier";
				}
				else if ( gs_token_char_is_numeric( c ) || c == '-' )
				{
					u32 num_decimals = 0;
					while 
					( 
						gs_token_char_is_numeric( lex->at[ 0 ] ) || 
						( lex->at[ 0 ] == '.' && num_decimals == 0 ) || 
						lex->at[ 0 ] == 'f' 
					)
					{
						// Grab decimal
						num_decimals = lex->at[ 0 ] == '.' ? num_decimals++ : num_decimals;

						// Increment
						lex->at++;
					}

					t.len = lex->at - t.text;
					t.type = "number";
				}
				else
				{
					t.type = "unknown";
					lex->at++;
				}

			} break;
		}
	}

	return t;
}

_inline gs_token gs_lexer_current_token( gs_lexer* lex )
{
	return lex->current_token;
}

_inline b8 gs_lexer_current_token_type_eq( gs_lexer* lex, const char* match_type )
{
	return ( gs_string_compare_equal( gs_lexer_current_token( lex ).type, match_type ) );
}

_inline gs_token gs_lexer_peek_next_token( gs_lexer* lex )
{
	// Store the at
	const char* at = lex->at;
	gs_token cur_t = gs_lexer_current_token( lex );
	gs_token next_t = lex->next_token( lex );
	lex->current_token = cur_t;
	lex->at = at;
	return next_t;
}

// Checks to see if the token type of the next valid token matches the match_type argument
// Will restore pointer of lex if not a match
_inline b8 gs_lexer_require_token_text( gs_lexer* lex, const char* match_text )
{
	// Store current position
	const char* at = lex->at;

	gs_token t =  gs_lexer_next_token( lex );

#if parser_debug_enabled
		gs_printf( "require_expect_text: %s, found: ", match_text );
		gs_token_debug_print( t );
#endif

	if ( gs_string_compare_equal_n( t.text, match_text, t.len ) )
	{
		return true;
	}

	// Hit invalid token, print error
	gs_printf( "\nLex Error: Unexpected token text: %.*s, Expected: %s\n", t.len, t.text, match_text );

	// Restore position
	lex->at = at;

	return false;
}

// Checks to see if the token type of the next valid token matches the match_type argument
// Will restore pointer of lex if not a match
_inline b8 gs_lexer_require_token_type( gs_lexer* lex, const char* match_type )
{
	// Store current position
	const char* at = lex->at;

	gs_token t = gs_lexer_next_token( lex );

#if parser_debug_enabled
	gs_printf( "require_expect_type: %s, found: ", match_type );
	gs_token_debug_print( t );
#endif

	if ( gs_string_compare_equal( t.type, match_type ) )
	{
		return true;
	}

	// Hit invalid token, print error
	gs_printf( "\nLex Error: Unexpected token type: %.s, Expected: %s\n\n", t.type, match_type );

	// Restore position
	lex->at = at;

	return false;
}

_inline b8 gs_lexer_optional_token_type( gs_lexer* lex, const char* match_type )
{
	const char* at = lex->at;

	gs_token t = gs_lexer_next_token( lex );

#if parser_debug_enabled
	gs_printf( "optional_expected: %s, found: ", match_type );
	gs_token_debug_print( t );
#endif

	if ( gs_token_compare_type( t, match_type ) )
	{
		return true;	
	}

	// Restore previous position
	lex->at = at;

	return false;
}

_inline b8 gs_lexer_optional_token_text( gs_lexer* lex, const char* match_text )
{
	const char* at = lex->at;

	gs_token t = gs_lexer_next_token( lex );

#if parser_debug_enabled
	gs_printf( "optional_expect: %s, found: ", match_text );
	gs_token_debug_print( t );
#endif

	if ( gs_token_compare_text( t, match_text ) )
	{
		return true;
	}

	// Restore previous position
	lex->at = at;

	return false;
}

// Advances position until lexer can no longer lex or token of type is found
// Returns true if found, false if end of stream is found
_inline b8 gs_lexer_find_token_type( gs_lexer* lex, const char* match_type )
{
	gs_token t = gs_lexer_current_token( lex );
	while ( gs_lexer_can_lex( lex ) )
	{
		if ( gs_token_compare_type( t, match_type ) )
		{
			return true;
		}
		t = gs_lexer_next_token( lex );
	}

	return false;
}

_inline gs_token gs_lexer_advance_before_next_token_type_occurence( gs_lexer* lex, const char* match_type )
{
	gs_token t = gs_lexer_current_token( lex );
	gs_token peek_t = gs_lexer_peek_next_token( lex );

	// Continue right up the token before the required type
	while ( !gs_token_compare_type( peek_t, match_type ) )
	{
		t = gs_lexer_next_token( lex );
		peek_t = gs_lexer_peek_next_token( lex );
	}

	return t;
}

_inline gs_lexer_c gs_lexer_c_ctor( const char* contents )
{
	gs_lexer_c lex;
	lex._base.at = contents;
	lex._base.contents = contents;
	lex._base.current_token = gs_token_invalid_token();
	lex._base.next_token = &gs_lexer_c_next_token;

	return lex;
}

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_LEXER_H__


























