#include <gs.h>

// Forward Decls.
gs_result app_update();		// Use to update your application
gs_result app_init(); 		// Use to init application
gs_result app_shutdown(); 	// Use to shutdown application

// Custom struct object for various containers
typedef struct object_t
{
	f32 float_value;
	u32 uint_value;
} object_t;

/*
	Declaring custom hash table type: 
	 * key: u64 					- Key type used to reference stored data
	 * val: object_t				- Value type of stored in table
	 * hash_key_func: gs_hash_u64	- Hash function used to hash the key (stored in gs_util.h)
	 * hash_comp_func: gs_hash_key_comp_std_type	 - Hash compare function (default stored in gs_util.h)
*/
gs_hash_table_decl( u64, object_t, gs_hash_u64, gs_hash_key_comp_std_type );

/*
	Declaring custom slot array type: 
	 * type: object_t				- Value type of stored in table
*/
gs_slot_array_decl( object_t );

// Globals
gs_dyn_array(object_t) 			g_dyn_array;
gs_hash_table(u64, object_t) 	g_hash_table;
gs_slot_array(object_t) 		g_slot_array;

gs_hash_table(u32, u32) g_test_hash;

// Util functions
void print_console_commands();
void print_array( gs_dyn_array( object_t )* );
void print_slot_array( gs_slot_array( object_t )* );
void print_hash_table( gs_hash_table( u64, object_t )* );
void object_to_str( object_t* obj, char* str, usize str_sz );

void print_u32( u32* v ) 
{
	gs_printf( "%zu", *v );
}

#define __print_hash_table( table, __key, __val, key_print, val_print )\
do {\
	/* Hash Table*/\
	gs_printf( "Hash Table: [ " );\
	/* Loop through all elements of hash table data array */\
	u32 cap = gs_dyn_array_capacity( table.data );\
	for ( u32 i = 0; i < cap; ++i ) {\
		b32 found = false;\
		if ( table.data[i].entry_state == hash_table_entry_active ){\
			found = true;\
			__key* k = &table.data[i].key;\
			__val* v = &table.data[i].val;\
			gs_printf( "<key: " );\
			key_print(k);\
			gs_printf( ", val: " );\
			val_print(v);\
			gs_printf( " >" );\
		}\
		if ( found ) {\
			gs_printf( i < cap - 1 ? ", " : "" );\
		}\
	}\
	gs_printf( " ]\n" );\
} while (0)

int main( int argc, char** argv )
{
	// This is our app description. It gives internal hints to our engine for various things like 
	// window size, title, as well as update, init, and shutdown functions to be run. 
	gs_application_desc app = {0};
	app.window_title 		= "Simple Containers";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.update 				= &app_update;
	app.init 				= &app_init;
	app.shutdown 			= &app_shutdown;

	// Construct internal instance of our engine
	gs_engine* engine = gs_engine_construct( app );

	// Run the internal engine loop until completion
	gs_result res = engine->run();

	// Check result of engine after exiting loop
	if ( res != gs_result_success ) 
	{
		gs_println( "Error: Engine did not successfully finish running." );
		return -1;
	}

	gs_println( "Gunslinger exited successfully." );

	return 0;	
}

gs_result app_init()
{
	// Allocate containers
	g_dyn_array = gs_dyn_array_new( object_t );
	g_slot_array = gs_slot_array_new( object_t );
	g_hash_table = gs_hash_table_new( u64, object_t );

	g_test_hash = gs_hash_table_new( u32, u32 );

	// Add elements to containers
	for ( u32 i = 0; i < 10; ++i ) 
	{
		object_t obj = {0};
		obj.float_value = (f32)i;
		obj.uint_value = i;

		// Push into dyn array
		gs_dyn_array_push( g_dyn_array, obj );

		// Push into slot array
		u32 id = gs_slot_array_insert( g_slot_array, obj );

		// Push into hash table
		gs_hash_table_insert( g_hash_table, (u64)i, obj );

		gs_hash_table_insert( g_test_hash, i, i );
	}

	__print_hash_table( g_test_hash, u32, u32, print_u32, print_u32 );

	gs_for_range_i( 10 )
	{
		u32 v = gs_hash_table_get( g_test_hash, i );
		gs_println( "%zu", v );
	}

	print_console_commands();

	return gs_result_success;
}

// Update your application here
gs_result app_update()
{
	// Grab global instance of engine
	gs_engine* engine = gs_engine_instance();

	// If we press the escape key, exit the application
	if ( engine->ctx.platform->key_pressed( gs_keycode_esc ) )
	{
		return gs_result_success;
	}

	// Clear containers
	if ( engine->ctx.platform->key_pressed( gs_keycode_c ) )
	{
		gs_dyn_array_clear( g_dyn_array );
		gs_hash_table_clear( g_hash_table );
		gs_slot_array_clear( g_slot_array );
		print_console_commands();		
	}

	// Insert incrementing val into array
	if ( engine->ctx.platform->key_pressed( gs_keycode_a ) )
	{
		static u32 v = 0;
		object_t obj = {};
		obj.float_value = (f32)v;
		obj.uint_value = v;

		gs_dyn_array_push( g_dyn_array, obj );
		gs_slot_array_insert( g_slot_array, obj );
		gs_hash_table_insert( g_hash_table, (u64)v, obj );
		v++;
		print_console_commands();		
	}

	// Print out elements of dynamic array (used a timed action macro so it only prints out at set interval)
	if ( engine->ctx.platform->key_pressed( gs_keycode_p ) )
	{
		// Spacing
		gs_println( "" );

		print_array( &g_dyn_array );
		print_slot_array( &g_slot_array );
		print_hash_table( &g_hash_table );
		print_console_commands();
	}

	// gs_timed_action( 60, 
	// {
	// 	gs_println( "here" );
	// });

	// Otherwise, continue
	return gs_result_in_progress;
}

gs_result app_shutdown()
{
	// Release memory for array
	gs_dyn_array_free( g_dyn_array );

	return gs_result_success;
}

void print_array( gs_dyn_array( object_t )* array )
{
	// Array
	gs_printf( "Array: [ " );

	// Loop through all elements of array
	u32 sz = gs_dyn_array_size( *array );
	for ( u32 i = 0; i < sz; ++i )
	{
	}
	gs_printf( " ]\n" );
}

void print_slot_array( gs_slot_array( object_t )* slot_array )
{
	// Slot Array
	gs_printf( "Slot Array: [ " );

	// Loop through all elements of array
	u32 sz = gs_dyn_array_size( slot_array->data );
	for ( u32 i = 0; i < sz; ++i )
	{
	}
	gs_printf( " ]\n" );
}

void print_hash_table( gs_hash_table( u64, object_t )* hash_table )
{
	// Temp buffer for printing object
	char tmp_buffer[256] = {0};

	// Hash Table
	gs_println( "Hash Table: [ " );

	gs_hash_table_iter( u64, object_t ) iter = gs_hash_table_iter_new( *hash_table );
	for ( ; gs_hash_table_iter_valid( *hash_table, iter ); gs_hash_table_iter_advance( *hash_table, iter ) )
	{
		u64 k = iter.data->key;
		object_t* v = &iter.data->val;
		object_to_str( v, tmp_buffer, 256 );
		gs_println( "\t{ k: %zu, %s } ", k, tmp_buffer );
	}
	gs_println( "]" );
}

void object_to_str( object_t* obj, char* str, usize str_sz )
{
	gs_snprintf( str, str_sz, "{ %.2f, %zu }", obj->float_value, obj->uint_value );
}

void print_console_commands()
{
	// Spacing
	gs_for_range_i( 2 )
	{
		gs_println( "" );
	}

	// Command line options (so fancy)
	gs_println( "Options (press key): " );
	gs_println( "   * A: Add new object into containers" );
	gs_println( "   * P: Print all containers" );
	gs_println( "   * C: Clear all containers" );
	gs_println( "   * Esc: Quit program" );
}
