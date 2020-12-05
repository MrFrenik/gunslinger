
/*================================================================
	* Copyright: 2020 John Jackson
	* simple_containers

	Simple example demonstrating how to use various included 
	container types core to the gunslinger library.

	Included: 
		Dynamic Array -> gs_dyn_array
		Hash Table    -> gs_hash_table
		Slot Array    -> gs_slot_array

	Press `esc` to exit the application.
=================================================================*/

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
gs_hash_table_decl(u64, object_t, gs_hash_u64, gs_hash_key_comp_std_type);

/*
	Declaring custom slot array type: 
	 * type: object_t				- Value type of stored in table
*/
gs_slot_array_decl(object_t);

// Globals
gs_dyn_array(object_t) 			g_dyn_array;
gs_hash_table(u64, object_t) 	g_hash_table;
gs_slot_array(object_t) 		g_slot_array;
u32 							g_cur_val;

// Util functions
void print_console_commands();
void print_array(gs_dyn_array(object_t)*);
void print_slot_array(gs_slot_array(object_t)*);
void print_hash_table(gs_hash_table(u64, object_t)*);
void object_to_str(object_t* obj, char* str, usize str_sz);

int main(int argc, char** argv)
{
	// This is our app description. It gives internal hints to our engine for various things like 
	// window size, title, as well as update, init, and shutdown functions to be run. 
	gs_application_desc_t app = {0};
	app.window_title 		= "Simple Containers";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.update 				= &app_update;
	app.init 				= &app_init;
	app.shutdown 			= &app_shutdown;

	// Construct internal instance of our engine
	gs_engine_t* engine = gs_engine_construct(app);

	// Run the internal engine loop until completion
	gs_result res = engine->run();

	// Check result of engine after exiting loop
	if (res != gs_result_success) 
	{
		gs_println("Error: Engine did not successfully finish running.");
		return -1;
	}

	gs_println("Gunslinger exited successfully.");

	return 0;	
}

gs_result app_init()
{
	g_cur_val = 0;

	// Allocate containers
	g_dyn_array = gs_dyn_array_new(object_t);
	g_slot_array = gs_slot_array_new(object_t);
	g_hash_table = gs_hash_table_new(u64, object_t);

	// Add elements to containers
	for (g_cur_val = 0; g_cur_val < 10; ++g_cur_val) 
	{
		object_t obj = {0};
		obj.float_value = (f32)g_cur_val;
		obj.uint_value = g_cur_val;

		// Push into dyn array
		gs_dyn_array_push(g_dyn_array, obj);

		// Push into slot array
		u32 id = gs_slot_array_insert(g_slot_array, obj);

		// Push into hash table
		gs_hash_table_insert(g_hash_table, (u64)g_cur_val, obj);
	}

	print_console_commands();

	return gs_result_success;
}

// Update your application here
gs_result app_update()
{
	// Grab global instance of engine
	gs_engine_t* engine = gs_engine_instance();

	// If we press the escape key, exit the application
	if (engine->ctx.platform->key_pressed(gs_keycode_esc))
	{
		return gs_result_success;
	}

	// Clear containers
	if (engine->ctx.platform->key_pressed(gs_keycode_c))
	{
		gs_dyn_array_clear(g_dyn_array);
		gs_hash_table_clear(g_hash_table);
		gs_slot_array_clear(g_slot_array);
		print_console_commands();		
	}

	// Insert incrementing val into array
	if (engine->ctx.platform->key_pressed(gs_keycode_a))
	{
		object_t obj = gs_default_val();
		obj.float_value = (f32)g_cur_val;
		obj.uint_value = g_cur_val;

		gs_dyn_array_push(g_dyn_array, obj);
		gs_slot_array_insert(g_slot_array, obj);
		gs_hash_table_insert(g_hash_table, (u64)g_cur_val, obj);

		g_cur_val++;

		print_console_commands();		
	}

	// Print out elements of dynamic array (used a timed action macro so it only prints out at set interval)
	if (engine->ctx.platform->key_pressed(gs_keycode_p))
	{
		// Spacing
		gs_println("");

		print_array(&g_dyn_array);
		print_slot_array(&g_slot_array);
		print_hash_table(&g_hash_table);
		print_console_commands();
	}

	// Otherwise, continue
	return gs_result_in_progress;
}

gs_result app_shutdown()
{
	// Release memory for array
	gs_dyn_array_free(g_dyn_array);
	gs_hash_table_free(g_hash_table);
	gs_slot_array_free(g_slot_array);

	return gs_result_success;
}

void print_array(gs_dyn_array(object_t)* arr)
{
	char buffer[256] = {0};

	// Array
	gs_println("Array: [ ");

	// Loop through all elements of array
	u32 sz = gs_dyn_array_size(*arr);
	for (u32 i = 0; i < sz; ++i)
	{
		object_to_str(&(*arr)[i], buffer, 256);
		gs_println("\t%s", buffer);
	}
	gs_printf(" ]\n");
}

void print_slot_array(gs_slot_array(object_t)* sa)
{
	// Slot Array
	gs_println("Slot Array: [ ");

	char buffer[256] = {0};

	for (
		gs_slot_array_iter(object_t) it = gs_slot_array_iter_new(*sa);
		gs_slot_array_iter_valid(*sa, it);
		gs_slot_array_iter_advance(*sa, it)
	)
	{
		object_t* obj = it.data;
		object_to_str(obj, buffer, 256);
		gs_println("\t%s, %zu", buffer, it.cur_idx);
	}

	gs_printf(" ]\n");
}

void print_hash_table(gs_hash_table(u64, object_t)* ht)
{
	// Temp buffer for printing object
	char tmp_buffer[256] = {0};

	// Hash Table
	gs_println("Hash Table: [ ");

	// Use iterator to iterate through hash table and print elements
	for (
		gs_hash_table_iter(u64, object_t) it = gs_hash_table_iter_new(*ht); 
		gs_hash_table_iter_valid(*ht, it); 
		gs_hash_table_iter_advance(*ht, it))
	{
		u64 k = it.data->key;
		object_t* v = &it.data->val;
		object_to_str(v, tmp_buffer, 256);
		gs_println("\t{ k: %zu, %s } ", k, tmp_buffer);
	}

	gs_println("]");
}

void object_to_str(object_t* obj, char* str, usize str_sz)
{
	gs_snprintf(str, str_sz, "{ %.2f, %zu }", obj->float_value, obj->uint_value);
}

void print_console_commands()
{
	// Spacing
	gs_for_range_i(2)
	{
		gs_println("");
	}

	// Command line options (so fancy)
	gs_println("Options (press key): ");
	gs_println("   * A: Add new object into containers");
	gs_println("   * P: Print all containers");
	gs_println("   * C: Clear all containers");
	gs_println("   * Esc: Quit program");
}
