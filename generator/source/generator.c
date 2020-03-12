#include <stdio.h>
#include <stdarg.h>

#include "common/gs_util.h"
#include "common/gs_containers.h"
#include "lexer/gs_lexer.h"
#include "base/gs_meta_class.h"

/*========================
// Parsing utilities
=========================*/

typedef struct parser
{
	gs_lexer* lex;
	u32 indention;
	u32 status;
} parser;

#define fmt_print_tab( ct )\
	do {\
		for ( u32 __i = 0; __i < ( ct ); ++__i ) { printf( "\t" ); };\
	} while( 0 )
	
_inline void fmt_tabbed_print( u32 indent, const char* fmt, ... )
{
	fmt_print_tab( indent );
	va_list args;
	va_start ( args, fmt );
	vprintf( fmt, args );
	va_end( args );
}

b8 parser_can_parse( parser* p )
{
	return ( p && gs_lexer_can_lex( p->lex ) && p->status );
}

typedef struct meta_attribute
{
	char* tag;
} meta_attribute;

typedef struct meta_field
{
	gs_dyn_array( meta_attribute ) attributes;
	char* type;
	char* identifier;
	char* default_value;
} meta_field;

typedef struct meta_struct
{
	gs_dyn_array( meta_field ) fields;
	char* base;
	char* identifier;
	char* defaults_ctor;
	char* serialize_func;
	char* deserialize_func;
} meta_struct;

meta_struct meta_struct_ctor()
{
	meta_struct ms = ( meta_struct ){};
	ms.fields = gs_dyn_array_new( meta_field );
	return ms;
}

#define meta_set_str( meta, prop, token )\
do {\
	if ( meta->prop != NULL )\
	{\
		printf( "prop: %s\n", #prop );\
		gs_free( meta->prop );\
	}\
	meta->prop = gs_malloc( token.len + 1 );\
	memset( meta->prop, 0, token.len + 1 );\
	memcpy( meta->prop, token.text, token.len );\
} while( 0 );

void meta_struct_dtor( meta_struct* ms )
{
	gs_dyn_array_free( ms->fields );
}

meta_field meta_field_ctor()
{
	meta_field field = { 0 };
	field.attributes = gs_dyn_array_new( meta_attribute );
	field.type = ( char* )NULL;
	field.identifier = ( char* )NULL;
	field.default_value = ( char* )NULL;
	return field;
}

meta_attribute meta_attribute_ctor()
{
	meta_attribute attr = { 0 };
	attr.tag = ( char* )NULL;
	return attr;
}

void meta_struct_debug_print( meta_struct* ms )
{
	gs_printf( "struct:\n" );

	// Print struct identifier
	gs_printf( "\tidentifier: %s\n", ms->identifier );

	// Print struct base
	gs_printf( "\tbase: %s\n", ms->base );

	// Print struct fields
	gs_printf( "\tfields:\n");
	gs_for_range_i( gs_dyn_array_size( ms->fields ) )
	{
		gs_printf( "\t\tfield:\n");

		meta_field* mf = &ms->fields[ i ];
		gs_printf( "\t\t\ttype: %s\n", mf->type );
		gs_printf( "\t\t\tidentifier: %s\n", mf->identifier );

		gs_printf( "\t\t\tattributes:\n");
		gs_for_range_j( gs_dyn_array_size( mf->attributes ) )
		{
			gs_printf( "\t\t\t\t%s\n", mf->attributes[ j ].tag );
		}
	}

	gs_printf( "\n" );
}

#define parse_print_current_token( p, label, indent )\
	do {\
		fmt_print_tab( indent );\
		printf( "%s: ", label );\
		gs_token_print_text( gs_lexer_current_token( p->lex ) );\
	} while( 0 )


#define parse_func( p, ... )\
{\
	const char* __at = p->lex->at;\
	u32 __cur_idention = p->indention;\
	__VA_ARGS__\
	p->lex->at = __at;\
	p->indention = __cur_idention;\
	return false;\
}

#define parser_capture( p, func )\
	p->status &= ( func )

b8 parse_identifier( parser* p, meta_struct* ms )
{
	parse_func( p, 
	{
		if ( gs_lexer_require_token_type( p->lex, "identifier" ) )
		{
			return true;		
		}
	});
}

// Would rather not have to update this every time a new attribute tag is added.
const char* gs_field_tags[] = 
{
	"_non_serializable",
	"_immutable",
	"_ignore",
	"_default",
	"_attributes"
};

b8 parse_is_token_a_field_tag( gs_token t )
{
	gs_for_range_i( gs_array_size( gs_field_tags ) )
	{
		if ( gs_token_compare_text( t, gs_field_tags[ i ] ) )
		{
			return true;
		}
	}
	return false;
}

b8 parse_struct_field_attribute_default_value( parser* p, meta_field* mf )
{
	// expression = term
	// term = identifier | number
	// expression = ( expression ) | expression
	// default_value_tag := _default = expression
	parse_func( p, 
	{
		if ( gs_token_compare_text( gs_lexer_current_token( p->lex ), "_default" ) )
		{
		  	parser_capture( p, gs_lexer_require_token_type( p->lex, "lparen" ) );

		  	// These have to match to be done with parsing this expression
		  	u32 num_lparen = 1;
		  	u32 num_rparen = 0;

		  	// Eat all whitespace up to first valid character
		  	gs_lexer_eat_whitespace( p->lex );

		  	// Capture the current position of the lexer
		  	const char* at = p->lex->at;
		  	const char* to = at;

		  	// Now need to go until a few specific conditions are met
		  	while ( num_lparen != num_rparen )
		  	{
		  		gs_token t = gs_lexer_next_token( p->lex );

		  		if ( gs_token_compare_type( t, "lparen" ) )
		  		{
		  			num_lparen++;
		  		}
		  		else if ( gs_token_compare_type( t, "rparen" ) )
		  		{
		  			num_rparen++;
		  		}

	  			// Store the to if still parsing statement
		  		if ( num_lparen != num_rparen )
		  		{
		  			to = p->lex->at;
		  		}
		  	}

		  	// Calculate size of string
		  	usize len = ( to - at );

		  	// Have the default value, so store in the meta field
		  	mf->default_value = NULL;
		  	mf->default_value = gs_malloc( len + 1 );
		  	gs_assert( mf->default_value );
		  	memcpy( mf->default_value, at, len );
		  	mf->default_value[ len ] = '\0';

			return true;
		}
	});
}

b8 parse_struct_field_attribute_tag( parser* p, meta_field* mf )
{
	parse_func( p, 
	{
		// Push back new attribute and set tag
		if ( gs_lexer_optional_token_type( p->lex, "identifier" ) )
		{
			if ( parse_struct_field_attribute_default_value( p, mf ) )
			{
				return true;	
			}
			else
			{
				gs_dyn_array_push( mf->attributes, meta_attribute_ctor() );
				meta_set_str( gs_dyn_array_back( mf->attributes ), tag, gs_lexer_current_token( p->lex ) );
				return true;
			}
		}
	});
}

b8 parse_struct_field_attribute_tags( parser* p, meta_struct* ms )
{
	// default_value_assignment := _default = expression
	// attribute_tag := identifier | empty
	// attribute_tag := default_value_assignment	
	// attribute_tags := identifier { ,attribute_tags }

	parse_func( p, 
	{
		// Grab last field
		meta_field* mf = gs_dyn_array_back( ms->fields );

		// Optional tags for group
		if ( parse_struct_field_attribute_tag( p, mf ) )
		{
			// Look for tag groups ( { , attribute_tag }* )
			while ( gs_token_compare_type( gs_lexer_peek_next_token( p->lex ), "comma" ) )
			{
				gs_lexer_require_token_type( p->lex, "comma" );

				parse_struct_field_attribute_tag( p, mf );
			}

			return true;
		}
	});
}

b8 parse_struct_field_attribute_tag_group( parser* p, meta_struct* ms )
{
	// attribute_tag_group := _attributes( { attribute_tags } )

	parse_func( p, 
	{
		// Found
		if ( gs_lexer_optional_token_text( p->lex, "_attributes" ) )
		{
			// Push on new meta field
			gs_dyn_array_push( ms->fields, meta_field_ctor() );

			// Parse attribute tags
			// Have found the _attributes tag, so now require paren
			parser_capture( p, gs_lexer_require_token_type( p->lex, "lparen" ) );

			parser_capture( p, parse_struct_field_attribute_tags( p, ms ) );

			parser_capture( p, gs_lexer_require_token_type( p->lex, "rparen") );

			return true;
		} 
	});
}

b8 parse_struct_field_ignore_tag( parser* p, meta_struct* ms )
{
	parse_func( p, 
	{
		if ( gs_lexer_optional_token_text( p->lex, "_ignore" ) )
		{
			return true;
		}
	});
}

b8 parse_struct_field_defaults_tag( parser* p, meta_struct* ms )
{
	// defaults_tag := _defaults( args )
	parse_func( p, 
	{
		b32 success = true;
		u32 lparen_count = 0, rparen_count = 0;
		if ( gs_lexer_optional_token_text( p->lex, "_defaults" ) )
		{
			success &= gs_lexer_require_token_type( p->lex, "lparen" );
			lparen_count++;
			const char* at = p->lex->at;
			const char* stop = NULL;
			gs_token token = gs_lexer_current_token( p->lex );
			gs_token start_token = gs_lexer_peek_next_token( p->lex );
			while ( lparen_count != rparen_count )
			{
				if ( gs_token_compare_type( gs_lexer_peek_next_token( p->lex ), "rparen" ) )
				{
					rparen_count++;
					stop = p->lex->at;
				}
				else if ( gs_token_compare_type( gs_lexer_peek_next_token( p->lex ), "lparen" ) )
				{
					lparen_count++;	
				}
				token = gs_lexer_next_token( p->lex );
			}
			u32 len = ( stop - start_token.text );
			ms->defaults_ctor = malloc( len + 1 );
			memcpy( ms->defaults_ctor, start_token.text, len );
			ms->defaults_ctor[ len ] = '\0';
			success &= gs_token_compare_type( gs_lexer_current_token( p->lex ), "rparen" );
			if ( success )
			{
				return true;
			}
		}
	});
}

b8 parse_struct_field_serialize_tag( parser* p, meta_struct* ms )
{
	parse_func( p, 
	{
		b32 success = true;
		u32 lparen_count = 0, rparen_count = 0;
		if ( gs_lexer_optional_token_text( p->lex, "_serialize" ) )
		{
			success &= gs_lexer_require_token_type( p->lex, "lparen" );
			lparen_count++;
			const char* at = p->lex->at;
			const char* stop = NULL;
			gs_token token = gs_lexer_current_token( p->lex );
			gs_token start_token = gs_lexer_peek_next_token( p->lex );
			while ( lparen_count != rparen_count )
			{
				if ( gs_token_compare_type( gs_lexer_peek_next_token( p->lex ), "rparen" ) )
				{
					rparen_count++;
					stop = p->lex->at;
				}
				else if ( gs_token_compare_type( gs_lexer_peek_next_token( p->lex ), "lparen" ) )
				{
					lparen_count++;	
				}
				token = gs_lexer_next_token( p->lex );
			}
			u32 len = ( stop - start_token.text );
			ms->serialize_func = malloc( len + 1 );
			memcpy( ms->serialize_func, start_token.text, len );
			ms->serialize_func[ len ] = '\0';
			success &= gs_token_compare_type( gs_lexer_current_token( p->lex ), "rparen" );
			gs_assert( success );
			return true;
		}
	});
}

b8 parse_struct_field_deserialize_tag( parser* p, meta_struct* ms )
{
	parse_func( p, 
	{
		b32 success = true;
		u32 lparen_count = 0, rparen_count = 0;
		if ( gs_lexer_optional_token_text( p->lex, "_deserialize" ) )
		{
			success &= gs_lexer_require_token_type( p->lex, "lparen" );
			lparen_count++;
			const char* at = p->lex->at;
			const char* stop = NULL;
			gs_token token = gs_lexer_current_token( p->lex );
			gs_token start_token = gs_lexer_peek_next_token( p->lex );
			while ( lparen_count != rparen_count )
			{
				if ( gs_token_compare_type( gs_lexer_peek_next_token( p->lex ), "rparen" ) )
				{
					rparen_count++;
					stop = p->lex->at;
				}
				else if ( gs_token_compare_type( gs_lexer_peek_next_token( p->lex ), "lparen" ) )
				{
					lparen_count++;	
				}
				token = gs_lexer_next_token( p->lex );
			}
			u32 len = ( stop - start_token.text );
			ms->deserialize_func = malloc( len + 1 );
			memcpy( ms->deserialize_func, start_token.text, len );
			ms->deserialize_func[ len ] = '\0';
			success &= gs_token_compare_type( gs_lexer_current_token( p->lex ), "rparen" );
			if ( success )
			{
				return true;
			}
		}
	});
}

b8 parse_struct_field( parser* p, meta_struct* ms )
{
	// field_type := identifier
	// field_identifier := identifier
	// field := _attributes( tags ) field_type field_identifier
	// field := field_type identifier | _attributes( tags ) field_type field_identifier | empty

	parse_func( p, 
	{
		// Ignore tag
		if ( parse_struct_field_ignore_tag( p, ms ) )
		{
			// Continue to semi colon
			gs_lexer_advance_before_next_token_type_occurence( p->lex, "semi_colon" );
			return true;
		}
		else if ( parse_struct_field_defaults_tag( p, ms ) )
		{
			return true;
		}
		else if ( parse_struct_field_serialize_tag( p, ms ) )
		{
			return true;
		} 
		else if ( parse_struct_field_deserialize_tag( p, ms ) )
		{
			return true;
		}
		// Optional attribute tag group ( but if found, field cannot be empty )
		else if ( parse_struct_field_attribute_tag_group( p, ms ) )
		{
			b8 success = true;

			// Pushed back a previous field - so grab it
			meta_field* mf = gs_dyn_array_back( ms->fields );

			success &= gs_lexer_require_token_type( p->lex, "identifier" ); 	// field_type
			meta_set_str( mf, type, gs_lexer_current_token( p->lex ) );

			success &= gs_lexer_require_token_type( p->lex, "identifier" ); 	// field_identifier
			meta_set_str( mf, identifier, gs_lexer_current_token( p->lex ) );

			if ( success )
			{
				return true;
			}
		}
		// No attribute tag, so attempt to parse struct field ( optional )
		else
		{
			if ( gs_lexer_optional_token_type( p->lex, "identifier") )
			{
				b8 success = true;

				// Push on new meta field
				gs_dyn_array_push( ms->fields, meta_field_ctor() );
				meta_field* mf = gs_dyn_array_back( ms->fields );

				gs_token t_field_type = gs_lexer_current_token( p->lex ); 			// field_type
				meta_set_str( mf, type, t_field_type );

				success &= gs_lexer_require_token_type( p->lex, "identifier" ); 	// field_identifier
				meta_set_str( mf, identifier, gs_lexer_current_token( p->lex ) );

				if ( success )
				{
					return true;
				}
			}
			else
			{
				return true;
			}
		}
	});
}

b8 parse_struct_fields( parser* p, meta_struct* ms )
{
	// Parsing individual field
	parse_func( p, 
	{
		// If found one, search for fields
		if ( parse_struct_field( p, ms ) )
		{
			while ( gs_token_compare_type( gs_lexer_peek_next_token( p->lex ), "semi_colon" ) )
			{
				gs_lexer_require_token_type( p->lex, "semi_colon" );	// semi_colon required
				parse_struct_field( p, ms );							// attempt to parse another struct field
			}
		} 

		return true;
	});
}

b8 parse_struct_base_tag( parser* p, meta_struct* ms )
{
	// base_tag := _base( field_type )

	parse_func( p, 
	{
		gs_lexer_require_token_text( p->lex, "_base" );
		gs_lexer_require_token_type( p->lex, "lparen" ); 

		// Base
		gs_lexer_require_token_type( p->lex, "identifier" );
		gs_token derive_t = gs_lexer_current_token( p->lex );

		// Store struct base token
		meta_set_str( ms, base, derive_t );

		gs_lexer_require_token_type( p->lex, "rparen" );

		if ( gs_token_compare_type( gs_lexer_current_token( p->lex ), "rparen" ) )
		{
			return true;
		}
	});
}

b8 parse_struct_body( parser* p, meta_struct* ms )
{
	// struct_body := derive_tag fields

	parse_func( p, 
	{
		if ( parse_struct_base_tag( p, ms ) && 
			 parse_struct_fields( p, ms ) )
		{
			return true;
		}
	});
}

b8 parse_struct_block( parser* p, meta_struct* ms )
{
	// struct_block := { struct_body };

	parse_func( p, 
	{
		if ( gs_lexer_require_token_type( p->lex, "lbrace" ) && 
			 parse_struct_body( p, ms ) && 
			 gs_lexer_require_token_type( p->lex, "rbrace" ) )
		{
			return true;
		}
	})
}

b8 parse_typedef_struct( parser* p, meta_struct* ms )
{
	// typedef_struct := typedef struct { identifier } struct_block identifier;

	parse_func( p, 
	{
		parser_capture( p, gs_lexer_require_token_text( p->lex, "typedef" ) );
		parser_capture( p, gs_lexer_require_token_text( p->lex, "struct" ) );
		gs_lexer_optional_token_type( p->lex, "identifier" );

		parser_capture( p, parse_struct_block( p, ms ) );

		// Struct identifier
		parser_capture( p, gs_lexer_require_token_type( p->lex, "identifier" ) );
		gs_token t_identifier = gs_lexer_current_token( p->lex );

		// Store identifier
		meta_set_str( ms, identifier, t_identifier );

		parser_capture( p, gs_lexer_require_token_type( p->lex, "semi_colon" ) );

		// NOTE(john): Really don't like this... just doesn't make sense, really
		// No errors occurred
		if ( p->status )
		{
			return true;
		}
	});
}

b8 parse_struct( parser* p, meta_struct* ms )
{
	// struct := typedef_struct
	// struct := struct_def; 

	parse_func( p, 
	{
		/*
			// Possible struct definition variations
			1). typedef struct [ name ] { _base( parent ) fields; } name_t;
			2). struct name_t { _base( parent ) fields; };
		*/
		// Version 1). 
		gs_token t = gs_lexer_peek_next_token( p->lex );
		if ( gs_token_compare_text( gs_lexer_peek_next_token( p->lex ), "typedef"  ) )
		{
			if ( parse_typedef_struct( p, ms ) )
			{
				return true;
			}
		}
		// Version 2). 
		// else if ( parse_standard_struct( p, indent + 1 ) )
		// {
		// 	return true;
		// }
	});
}

b8 parse_introspection( parser* p, gs_dyn_array( meta_struct )* meta_structs )
{
	// introspected_struct := _introspection struct

	parse_func( p, 
	{
		meta_struct ms = meta_struct_ctor();
		if ( parse_struct( p, &ms ) )
		{
			gs_dyn_array_push( *meta_structs, ms );
			return true;
		}
	});
}

typedef struct
{
	gs_dyn_array( meta_struct ) meta_struct_arr;
	gs_dyn_array( const char* ) file_arr;
	const char* root_dir;
	const char* proj_name;
} introspection_ctx;


/*========================
// Function Decls.
=========================*/

void compile_generated_reflection( introspection_ctx* ctx );

/*========================
// Main Entry Point
=========================*/

int main( int argc, char** argv )
{
	gs_println( "Starting reflection generation..." );

	// Context to use for parsing introspection data
	introspection_ctx ctx = ( introspection_ctx ){};
	ctx.file_arr = gs_dyn_array_new( const char* );
	ctx.meta_struct_arr= gs_dyn_array_new( meta_struct );

	// Collect all files for reflection
	gs_for_range_i( argc )
	{
		if ( gs_string_compare_equal( argv[ i ], "--files" ) )
		{
			for ( u32 j = i + 1; j < argc; ++j )
			{
				gs_dyn_array_push( ctx.file_arr, argv[ j ] );
			}
		} 
		else if ( gs_string_compare_equal( argv[ i ], "--proj_name" ) )
		{
			ctx.proj_name = argv[ i + 1 ];
		}
		else if ( gs_string_compare_equal( argv[ i ], "--root_dir" ) )
		{
			ctx.root_dir = argv[ i + 1 ];
		}
	}

	// Construct lexer with null contents
	gs_lexer_c lex = gs_lexer_c_ctor( NULL );

	// Parse all files
	gs_for_range_i( gs_dyn_array_size( ctx.file_arr ) )
	{
		// Read in source file to parse ( first elem )
		usize sz = 0;
		const char* src = gs_read_file_contents_into_string_null_term( ctx.file_arr[ i ], "r", &sz );

		// If file does not exist or allocation fails, skip
		if ( !src )
		{
			continue;
		}

		gs_println( "Generating reflection for: %s", ctx.file_arr[ i ] );	

		gs_lexer_set_contents( gs_cast( gs_lexer, &lex ), src );

		// Construct new parser
		parser parse = { gs_cast( gs_lexer, &lex ), 0, true };

		// Parse file
		while ( parser_can_parse( &parse ) )
		{
			// Simple debug ( just output next valid token from stream )
			gs_token t = gs_lexer_next_token( gs_cast( gs_lexer, &lex ) );

			if ( gs_token_compare_text( t, "_introspect" ) )
			{
				parse_introspection( &parse, &ctx.meta_struct_arr );
			}
		}
	}

	// Debug print out all information
	compile_generated_reflection( &ctx );

	gs_println( "Reflection generation complete.\n" );

	return 0;	
}

void compile_generated_reflection( introspection_ctx* ctx )
{
	// Temporary stack buffer to use 
	char temp_stack_buffer_header_file[ 1024 ];
	char temp_stack_buffer_source_file[ 1024 ];

	// Output header file name
	gs_snprintf( temp_stack_buffer_header_file, 1024, "%s/include/generated/%s_generated.h", ctx->root_dir, ctx->proj_name );
	gs_snprintf( temp_stack_buffer_source_file, 1024, "%s/source/generated/%s_generated.c", ctx->root_dir, ctx->proj_name );

	// Open header file for writing
	FILE* fp_header = fopen( temp_stack_buffer_header_file, "w" );

	gs_assert( fp_header );

	// Cache off meta struct array
	gs_dyn_array( meta_struct ) meta_struct_arr = ctx->meta_struct_arr;
	// Cache off header file array
	gs_dyn_array( const char* ) header_file_arr = ctx->file_arr;

	/*==============================================
	// Reflection Generation
	==============================================*/

	/*==============================================
	// gs_generated.h
	==============================================*/

	gs_fprintln( fp_header, "// This file has been generated. Any and all modifications will be lost." );
	gs_fprintln( fp_header, "" );

	gs_fprintln( fp_header, "#ifndef %s_GENERATED_H", ctx->proj_name );
	gs_fprintln( fp_header, "#define %s_GENERATED_H", ctx->proj_name );
	gs_fprintln( fp_header, "" );

	gs_fprintln( fp_header, "#include \"base/gs_meta_class.h\"" );
	gs_fprintln( fp_header, "" );

	/*==============================================
	// static property type hashes
	==============================================*/

	gs_fprintln( fp_header, "// Generated static hashes of string version of base types" );
	gs_for_range_i( gs_meta_property_type_count )
	{
		const char* meta_prop_str = gs_meta_property_to_str( ( gs_meta_property_type )i );
		gs_fprintln( fp_header, "#define gs_meta_property_type_%s_hash\t\t%zu", meta_prop_str, gs_hash_str( meta_prop_str ) );	
	}
	gs_fprintln( fp_header, "" );

	// gs_fprintln( fp_header, "// Generated static hashes of string version of base types" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_s8_hash 			5863760" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_u8_hash 			5863826" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_s16_hash 			193503903" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_u16_hash 			193506081" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_s32_hash 			193503965" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_u32_hash 			193506143" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_s64_hash 			193504066" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_u64_hash 			193506244" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_f32_hash 			193489808" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_f64_hash 			193489909" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_gs_vec2_hash 		262723214" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_gs_vec3_hash 		262723215" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_gs_vec4_hash 		262723216" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_gs_mat4_hash 		262395988" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_gs_quat_hash 		262560953" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_gs_vqs_hash 		7961720" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_gs_uuid_hash 		262704949" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_gs_entity_hash 	1983784891" );
	// gs_fprintln( fp_header, "#define gs_meta_property_type_gs_enum_hash 		262122739" );
	// gs_fprintln( fp_header, "" );

	/*==============================================
	// gs_meta_class_type enum decl
	==============================================*/

	const u32 col_alignment = 57;

	// Output generated 'gs_meta_class_type' enum
	gs_fprintln( fp_header, "/*==============================================");
	gs_fprintln( fp_header, "// Meta Class Id" );
	gs_fprintln( fp_header, "==============================================*/");
	gs_fprintln( fp_header, "" );

	gs_fprintln( fp_header, "typedef enum\n{" );
	gs_for_range_i( gs_dyn_array_size( meta_struct_arr ) )
	{
		usize len = gs_string_length( meta_struct_arr[ i ].identifier );
		u32 cur_col_num = 5 + gs_string_length( "gs_meta_class_id_" ) + len; 
		gs_fprintf( fp_header, "\tgs_meta_class_id_%s", meta_struct_arr[ i ].identifier );
		gs_for_range_i( ( col_alignment - cur_col_num ) ) { gs_fprintf( fp_header, " "); };
		gs_fprintln( fp_header, "= %d,", i );

	}
	gs_fprintln( fp_header, "\tgs_meta_class_id_count");
	gs_fprintln( fp_header, "} gs_meta_class_type;" );
	gs_fprintln( fp_header, "" );

	/*============================================================
	// Function decls
	============================================================*/

	gs_for_range_i( gs_dyn_array_size( meta_struct_arr ) )
	{
		gs_fprintln( fp_header, "/*==============================================");
		gs_fprintln( fp_header, "// %s", meta_struct_arr[ i ].identifier );
		gs_fprintln( fp_header, "==============================================*/");
		gs_fprintln( fp_header, "" );

		gs_fprintln( fp_header, "_inline u32 gs_type_id_%s()", meta_struct_arr[ i ].identifier ); 
		gs_fprintln( fp_header, "{" );
		gs_fprintln( fp_header, "\treturn ( u32 )gs_meta_class_id_%s;", meta_struct_arr[ i ].identifier );
		gs_fprintln( fp_header, "}" );

		gs_fprintln( fp_header, "" );
		gs_fprintln( fp_header, "const struct gs_object* __gs_default_object_%s();", meta_struct_arr[ i ].identifier );
		gs_fprintln( fp_header, "" );

		gs_fprintln( fp_header, "" );
		gs_fprintln( fp_header, "struct gs_object* __gs_default_object_%s_heap();", meta_struct_arr[ i ].identifier );
		gs_fprintln( fp_header, "" );
	}

	// End
	gs_fprintln( fp_header, "#endif" );

	// Close file
	fclose( fp_header );
	fp_header = NULL;

	/*==============================================
	// gs_generated.c 
	==============================================*/

	// Open source file for writing
	FILE* fp_src = fopen( temp_stack_buffer_source_file, "w" );

	gs_assert( fp_src );

	gs_fprintln( fp_src, "// This file has been generated. Any and all modifications will be lost." );
	gs_fprintln( fp_src, "" );

	gs_fprintln( fp_src, "#include \"base/gs_meta_class.h\"" );
	gs_fprintln( fp_src, "#include \"generated/gs_generated.h\"" );
	gs_fprintln( fp_src, "" );

	gs_fprintln( fp_src, "// Included introspected files" );
	gs_for_range_i( gs_dyn_array_size( header_file_arr ) )
	{
		gs_fprintln( fp_src, "#include \"%s\"", header_file_arr[ i ] );
	}
	gs_fprintln( fp_src, "" );

	gs_fprintln( fp_src, "#include <stdlib.h>" );
	gs_fprintln( fp_src, "#include <stdio.h>" );
	gs_fprintln( fp_src, "" );

	/*==============================================
	// Meta class labels 
	==============================================*/

	gs_fprintln( fp_src, "// Labels for all introspected types" );
	gs_fprintln( fp_src, "const char* gs_meta_class_labels[] = {" );
	gs_for_range_i( gs_dyn_array_size( meta_struct_arr ) )
	{
		u8 delim = i < gs_dyn_array_size( meta_struct_arr ) - 1 ? ',' : ' ';
		gs_fprintln( fp_src, "\t\"%s\"%c", meta_struct_arr[ i ].identifier, delim );
	}
	gs_fprintln( fp_src, "};" );
	gs_fprintln( fp_src, "" );

	/*==============================================
	// Default struct instances 
	==============================================*/

	gs_fprintln( fp_src, "/*==============================================");
	gs_fprintln( fp_src, "// Default Structures" );
	gs_fprintln( fp_src, "==============================================*/");
	gs_fprintln( fp_src, "" );

	gs_for_range_i( gs_dyn_array_size( meta_struct_arr ) )
	{
		const char* id = meta_struct_arr[ i ].identifier;
		// gs_fprintln( fp_src, "// %s", id );
		gs_fprintln( fp_src, "%s %s_default;", id, id, id );
	}
	gs_fprintln( fp_src, "" );

	/*==============================================
	// Default struct init function 
	==============================================*/

	gs_fprintln( fp_src, "void gs_init_default_struct_instances()" );
	gs_fprintln( fp_src, "{" );

	gs_for_range_i( gs_dyn_array_size( meta_struct_arr ) )
	{
		const char* id = meta_struct_arr[ i ].identifier;
		gs_fprintln( fp_src, "\t// %s", id );
		gs_fprintf( fp_src, "\t%s_default =", id );

		b32 empty = gs_dyn_array_empty( meta_struct_arr[ i ].fields );
		b32 has_defaults_ctor = meta_struct_arr[ i ].defaults_ctor != NULL;
		b32 has_any_default_values = empty;
		if ( !empty )
		{
			gs_for_range_j( gs_dyn_array_size( meta_struct_arr[ i ].fields ) )
			{
				meta_field* field = &meta_struct_arr[ i ].fields[ j ];
				has_any_default_values |= ( field->default_value != NULL );
			}
		}

		if ( ( empty || !has_any_default_values ) && !has_defaults_ctor ) 
		{
			gs_fprintf( fp_src, " ( %s ){};", id );
		} 
		else 
		{
			gs_fprintf( fp_src, "\n\t( %s )\n\t{\n ", id );

			// Output default values for fields
			b8 first_default = true;
			gs_for_range_j( gs_dyn_array_size( meta_struct_arr[ i ].fields ) )
			{
				meta_field* field = &meta_struct_arr[ i ].fields[ j ];
				if ( field->default_value )
				{
					if ( first_default ) 
					{
						first_default = false;
						gs_fprintf( fp_src, "\t\t.%s = %s", field->identifier, field->default_value );
					} 
					else 
					{
						gs_fprintf( fp_src, ",\n\t\t.%s = %s", field->identifier, field->default_value );
					}
				}
			}
			// Default ctor
			if ( meta_struct_arr[ i ].defaults_ctor )
			{
				if ( gs_dyn_array_size( meta_struct_arr[ i ].fields ) )
				{
					gs_fprintf( fp_src, ",\n\t\t%s", meta_struct_arr[ i ].defaults_ctor );
				}
				else
				{
					gs_fprintf( fp_src, "\t\t%s", meta_struct_arr[ i ].defaults_ctor );
				}
			}
			gs_fprintf( fp_src, "\n\t};" );
		}
		gs_fprintln( fp_src, "\n" );
	}

	gs_for_range_i( gs_dyn_array_size( meta_struct_arr ) )
	{
		const char* id = meta_struct_arr[ i ].identifier;
		gs_fprintln( fp_src, "\tgs_cast( gs_object, &%s_default )->type_id = &gs_type_id_%s;", id, id );
	}

	gs_fprintln( fp_src, "}" );
	gs_fprintln( fp_src, "" );

	/*==============================================
	// Registry init function
	==============================================*/

	gs_fprintln( fp_src, "void gs_meta_class_registry_init_meta_properties( gs_meta_class_registry* restrict registry )" );
	gs_fprintln( fp_src, "{" );
	gs_fprintln( fp_src, "\t// Construct registry" );
	gs_fprintln( fp_src, "\tregistry->classes = ( gs_meta_class* )gs_malloc( sizeof( gs_meta_class ) * gs_meta_class_id_count );" );
	gs_fprintln( fp_src, "" );

		gs_fprintln( fp_src, "\t// Set the ids for all the meta class information" );
		gs_fprintln( fp_src, "\tfor( usize i = 0; i < (usize)gs_meta_class_id_count; ++i )" );
		gs_fprintln( fp_src, "\t{" );
			gs_fprintln( fp_src, "\t\tregistry->classes[ i ].id = ( gs_meta_class_id )i;" );
		gs_fprintln( fp_src, "\t};" );
		gs_fprintln( fp_src, "" );

		gs_fprintln( fp_src, "\t// Meta class variable to use for these ops" );
		gs_fprintln( fp_src, "\tgs_meta_class* cls = NULL;" );
		gs_fprintln( fp_src, "" );

		gs_fprintln( fp_src, "\t// Meta class initialization" );
		gs_fprintln( fp_src, "" );

		// Print out all type information for introspected structs
		gs_for_range_i( gs_dyn_array_size( meta_struct_arr ) )
		{
			const char* cls_id = meta_struct_arr[ i ].identifier;
			gs_dyn_array( meta_field ) fields = meta_struct_arr[ i ].fields;
			u32 field_count = gs_dyn_array_size( fields );
			gs_fprintln( fp_src, "\t// %s meta class initialization", cls_id );
			gs_fprintln( fp_src, "\tcls = &registry->classes[ (usize)gs_meta_class_id_%s ];", cls_id ); 
			gs_fprintln( fp_src, "\tcls->label = \"%s\";", cls_id );
			gs_fprintln( fp_src, "\tcls->id = %d;", i );
			gs_fprintln( fp_src, "\tcls->property_count = %d;", field_count );
			gs_fprintln( fp_src, "\tcls->properties = ( gs_meta_property* )gs_malloc( sizeof( gs_meta_property ) * cls->property_count );" );
			gs_fprintln( fp_src, "\tcls->serialize_func = &%s;",
						meta_struct_arr[ i ].serialize_func ? 
						meta_struct_arr[ i ].serialize_func : 
						"__gs_object_serialization_base" );
			gs_fprintln( fp_src, "\tcls->deserialize_func = &%s;",
						meta_struct_arr[ i ].deserialize_func ? 
						meta_struct_arr[ i ].deserialize_func : 
						"__gs_object_deserialization_base" );

			// Output field information
			gs_for_range_j( field_count )
			{
				const char* field_id = fields[ j ].identifier;
				const char* field_type = fields[ j ].type;
				gs_fprintln( fp_src, "\tcls->properties[ %d ] = gs_meta_property_ctor( \"%s\", gs_type_to_meta_property_type( %s ), gs_offset( %s, %s ) );",
								j, field_id, field_type, cls_id, field_id );

			}

			gs_fprintln( fp_src, "\tcls->property_name_to_index_table = gs_hash_table_new( u32, gs_meta_property_ptr );" );

			gs_for_range_j( field_count )
			{
				const char* field_id = fields[ j ].identifier;
				gs_fprintln( fp_src, "\tgs_hash_table_insert( cls->property_name_to_index_table, gs_hash_str( \"%s\" ), &cls->properties[ %d ] );", 
								field_id, j );
			}

			gs_fprintln( fp_src, "" );
		}

		gs_fprintln( fp_src, "\tregistry->count = %d;", gs_dyn_array_size( meta_struct_arr ) );
		gs_fprintln( fp_src, "" );

		gs_fprintln( fp_src, "\tgs_init_default_struct_instances();" );
	gs_fprintln( fp_src, "}" );
	gs_fprintln( fp_src, "" );

	/*==============================================
	// Utility functions for meta classes
	==============================================*/

	gs_fprintln( fp_src, "const char* gs_meta_class_get_label( gs_meta_class* restrict cls )" );
	gs_fprintln( fp_src, "{" );
	gs_fprintln( fp_src, "\treturn gs_meta_class_labels[ cls->id ];" );
	gs_fprintln( fp_src, "}" );
	gs_fprintln( fp_src, "" );

	gs_fprintln( fp_src, "const char* gs_type_name_obj( gs_object* restrict obj )" );
	gs_fprintln( fp_src, "{" );
	gs_fprintln( fp_src, "\treturn ( gs_meta_class_labels[ gs_type_id( obj ) ] );" );
	gs_fprintln( fp_src, "}" );
	gs_fprintln( fp_src, "" );

	gs_fprintln( fp_src, "const char* __gs_type_name_cls( u32 id )" );
	gs_fprintln( fp_src, "{" );
	gs_fprintln( fp_src, "\treturn gs_meta_class_labels[ id ];" );
	gs_fprintln( fp_src, "}" );
	gs_fprintln( fp_src, "" );

	/*==============================================
	// Default object accessors
	==============================================*/

	gs_for_range_i( gs_dyn_array_size( meta_struct_arr ) )
	{
		const char* cls_id = meta_struct_arr[ i ].identifier;

		gs_fprintln( fp_src, "// %s", cls_id );
		gs_fprintln( fp_src, "const struct gs_object* __gs_default_object_%s()", cls_id ); 
		gs_fprintln( fp_src, "{" );
		gs_fprintln( fp_src, "\treturn ( gs_cast( gs_object, &%s_default ) );", cls_id );
		gs_fprintln( fp_src, "}" );
		gs_fprintln( fp_src, "" );

		gs_fprintln( fp_src, "struct gs_object* __gs_default_object_%s_heap()", cls_id );
		gs_fprintln( fp_src, "{" );
		gs_fprintln( fp_src, "\t%s* obj = gs_malloc( sizeof( %s ) );", cls_id, cls_id );
		gs_fprintln( fp_src, "\t*obj = %s_default;", cls_id, cls_id );
		gs_fprintln( fp_src, "\treturn ( gs_cast( gs_object, obj ) );" );
		gs_fprintln( fp_src, "}" );
		gs_fprintln( fp_src, "" );
	}
	gs_fprintln( fp_src, "" );

	/*==============================================
	// Finished Reflection Generation
	==============================================*/
}



















