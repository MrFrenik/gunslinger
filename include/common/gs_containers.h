#ifndef __GS_CONTAINERS_H__
#define __GS_CONTAINERS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gs_types.h"
#include "common/gs_util.h"

/*===================================
// Dynamic Array
===================================*/

/*
	- HEAVILY lifted from Sean Barret's 'stretchy buffer' implementation

	TODO(john): illustrate basic usage example	
*/

typedef struct
{
	s32 size;
	s32 capacity;
} gs_dyn_array;

#define gs_dyn_array_head( arr )\
	( ( gs_dyn_array* )( ( u8* )arr - sizeof( gs_dyn_array ) ) )

#define gs_dyn_array_size( arr )\
	gs_dyn_array_head( arr )->size

#define gs_dyn_array_capacity( arr )\
	gs_dyn_array_head( arr )->capacity

#define gs_dyn_array_full( arr )\
	( ( gs_dyn_array_size( arr ) == gs_dyn_array_capacity( arr ) ) )	

_inline void* gs_dyn_array_resize_impl( void* arr, usize sz, usize amount ) 
{
	usize capacity;

	if ( arr ) {
		capacity = amount;	
	} else {
		capacity = 0;
	}

	// Create new gs_dyn_array with just the header information
	gs_dyn_array* data = ( gs_dyn_array* )gs_realloc( arr ? gs_dyn_array_head( arr ) : 0, capacity * sz + sizeof( gs_dyn_array ) );
	if ( data )
	{
		// Set size
		if ( !arr )
		{
			data->size = 0;
		}

		data->capacity = capacity;

		// Return actual data position in buffer
		return ( ( s32* )data + 2 );
	}

	return NULL;
}

#define gs_dyn_array_grow( arr )\
	gs_dyn_array_resize_impl( arr, sizeof( *( arr ) ), gs_dyn_array_capacity( arr ) ? gs_dyn_array_capacity( arr ) * 2 : 1 )

#define gs_dyn_array_push( arr, val )\
	do {\
		if ( !( arr ) || ( ( arr ) && gs_dyn_array_full( arr ) ) ) {\
			*( ( void ** )&( arr ) ) = gs_dyn_array_grow( arr ); \
		}\
		( arr )[ gs_dyn_array_size( arr ) ] = ( val );\
		gs_dyn_array_size( arr ) += 1;\
	} while( 0 )

#define gs_dyn_array_reserve( arr, amount )\
	do {\
		if ( ( !arr ) || amount > gs_dyn_array_capacity( arr ) ) {\
			*( ( void ** )&( arr ) ) = gs_dyn_array_resize_impl( arr, sizeof( *arr ), amount );\
		}\
	} while( 0 )

#define gs_dyn_array_empty( arr )\
	( arr && ( gs_dyn_array_size( arr ) == 0 ) )

#define gs_dyn_array_pop( arr )\
	do {\
		if ( arr && !gs_dyn_array_empty( arr ) ) {\
			gs_dyn_array_size( arr ) -= 1;\
		}\
	} while ( 0 )

#define gs_dyn_array_back( arr )\
	( arr + ( gs_dyn_array_size( arr ) ? gs_dyn_array_size( arr ) - 1 : 0 ) )

#define gs_dyn_array_for( arr, type, iter_name )\
	for ( type* iter_name = arr; iter_name != gs_dyn_array_back( arr ); ++iter_name )

#define gs_dyn_array_new( type )\
	( type* )gs_dyn_array_resize_impl( NULL, sizeof( type ), 0 )

#define gs_dyn_array_clear( arr )\
	gs_dyn_array_size( arr ) = 0

#define gs_dyn_array( type )	type*

#define gs_dyn_array_free( arr )\
	do {\
		if ( arr ) {\
			gs_free( gs_dyn_array_head( arr ) );\
			arr = 0;\
		}\
	} while ( 0 )

/*===================================
// Hash Table
===================================*/

// TODO(john): Need to improve the hash table a lot ( round robin should help linear probing )

/*
	NOTE(john): Hash Function References: http://www.cse.yorku.ca/~oz/hash.html

	Want to have syntax like this:
		gs_hash_table table = gs_hash_table( u32, object* );
		gs_hash_table_get( &table, key );
		gs_hash_table_put( &table, key, val );
		gs_hash_table_exists( &table, key );
		gs_hash_table_size( &table );
		gs_hash_table_clear( &table );

	typedef struct
	{
		size_t key_type_size;	
		size_t val_type_size;			// Not sure if this is necessary, but it might be
		void* data_array;
		hash_func_ptr;
		hash_eq_func_ptr;
	} gs_hash_table;

// Will NOT work for const char*, however ( must define new type, I imagine )
#define hash_table( key_type, val_type )\
	{ .key_type_size = sizeof( key_type ), .val_type_size = sizeof( val_type ) };

	// Is this possible? ( I think so, if sizes are used )
*/

#define gs_hash_key_comp_str( a, b ) ( gs_string_compare_equal( a, b ) )

#define gs_hash_key_comp_std_type( a, b ) ( a == b )

#define gs_hash_table_valid_idx( idx )\
	( idx < ( UINT_MAX - 1 ) )

typedef enum
{
	hash_table_entry_inactive 	= 0,
	hash_table_entry_active 	= 1
} gs_hash_table_entry_state;

#define gs_hash_table_decl( key_type, val_type, _hash_func, _hash_comp_key_func )\
	typedef struct\
	{\
		key_type key;\
		val_type val;\
		gs_hash_table_entry_state entry_state;\
	} gs_ht_entry_##key_type##_##val_type;\
\
	typedef struct\
	{\
		gs_ht_entry_##key_type##_##val_type *data;\
		u32 ( * hash_func )();\
		u32 ( * hash_key_idx_func )( void*, key_type );\
		val_type ( * hash_get_val_func )( void*, key_type );\
		val_type* ( * hash_get_val_ptr_func )( void*, key_type );\
		b8 ( * hash_comp_key_func )( key_type, key_type );\
		void ( * hash_grow_func )( void*, u32 );\
	} gs_ht_##key_type##_##val_type;\
\
	_inline u32 gs_ht_##key_type##_##val_type##_key_idx_func( void* tbl_ptr, key_type key )\
	{\
		gs_ht_##key_type##_##val_type* tbl = ( gs_ht_##key_type##_##val_type* )tbl_ptr;\
		u32 capacity = gs_hash_table_capacity( ( *tbl ) );\
		u32 idx = tbl->hash_func( key ) % capacity;\
		for ( u32 i = idx, c = 0; c < capacity; ++c, i = ( ( i + 1 ) % capacity ) )\
		{\
			if ( _hash_comp_key_func( tbl->data[ i ].key, key ) )\
			{\
				return i;\
			}\
		}\
		return UINT_MAX - 1;\
	}\
	_inline val_type gs_ht_##key_type##_##val_type##_get_func( void* tbl_ptr, key_type key )\
	{\
		val_type out = { 0 };\
		gs_ht_##key_type##_##val_type* tbl = ( gs_ht_##key_type##_##val_type* )tbl_ptr;\
		if ( gs_dyn_array_empty( tbl->data ) ) \
		{\
			return out;\
		}\
		u32 capacity = gs_hash_table_capacity( ( *tbl ) );\
		u32 val = UINT_MAX - 1;\
		u32 idx = tbl->hash_func( key ) % capacity;\
		gs_ht_entry_##key_type##_##val_type* data = tbl->data;\
		for ( u32 i = idx, c = 0; c < capacity; ++c, i = ( ( i + 1 ) % capacity ) )\
		{\
			if ( _hash_comp_key_func( data[ i ].key, key ) )\
			{\
				val = i;\
				break;\
			}\
		}\
		if ( gs_hash_table_valid_idx( val ) )\
		{\
			out = data[ val ].val;\
		}\
		return out;\
	}\
	_inline val_type* gs_ht_##key_type##_##val_type##_get_ptr_func( void* tbl_ptr, key_type key )\
	{\
		val_type* out = NULL;\
		gs_ht_##key_type##_##val_type* tbl = ( gs_ht_##key_type##_##val_type* )tbl_ptr;\
		if ( gs_dyn_array_empty( tbl->data ) ) \
		{\
			return out;\
		}\
		u32 capacity = gs_hash_table_capacity( ( *tbl ) );\
		u32 val = UINT_MAX - 1;\
		u32 idx = tbl->hash_func( key ) % capacity;\
		gs_ht_entry_##key_type##_##val_type* data = tbl->data;\
		for ( u32 i = idx, c = 0; c < capacity; ++c, i = ( ( i + 1 ) % capacity ) )\
		{\
			if ( _hash_comp_key_func( data[ i ].key, key ) )\
			{\
				val = i;\
				break;\
			}\
		}\
		if ( gs_hash_table_valid_idx( val ) )\
		{\
			out = &data[ val ].val;\
		}\
		return out;\
	}\
	_inline b8 gs_ht_##key_type##_##val_type##_comp_key_func( key_type k0, key_type k1 )\
	{\
		if ( _hash_comp_key_func( k0, k1 ) )\
		{\
			return true;\
		}\
		return false;\
	}\
\
	_inline void gs_ht_##key_type##_##val_type##_grow_func( void* tbl_ptr, u32 new_sz );\
\
	_inline gs_ht_##key_type##_##val_type gs_ht_##key_type##_##val_type##_new()\
	{\
		gs_ht_##key_type##_##val_type ht = {\
			.data 					= gs_dyn_array_new( gs_ht_entry_##key_type##_##val_type ),\
			.hash_func 				= &_hash_func,\
			.hash_key_idx_func 		= gs_ht_##key_type##_##val_type##_key_idx_func,\
			.hash_get_val_func 		= gs_ht_##key_type##_##val_type##_get_func,\
			.hash_get_val_ptr_func 	= gs_ht_##key_type##_##val_type##_get_ptr_func,\
			.hash_comp_key_func 	= gs_ht_##key_type##_##val_type##_comp_key_func,\
			.hash_grow_func 		= gs_ht_##key_type##_##val_type##_grow_func\
		};\
		return ht;\
	}\
\
	_inline void gs_ht_##key_type##_##val_type##_grow_func( void* tbl_ptr, u32 new_sz )\
	{\
		gs_ht_##key_type##_##val_type* tbl = ( gs_ht_##key_type##_##val_type* )tbl_ptr;\
		gs_ht_##key_type##_##val_type new_tbl = gs_ht_##key_type##_##val_type##_new();\
		gs_dyn_array_reserve( new_tbl.data, new_sz );\
		memset( new_tbl.data, 0, new_sz * sizeof( gs_ht_entry_##key_type##_##val_type ) );\
		for ( u32 i = 0; i < gs_dyn_array_capacity( tbl->data ); ++i ) {\
			if ( tbl->data[ i ].entry_state == hash_table_entry_active ) {\
				gs_hash_table_insert( new_tbl, tbl->data[ i ].key, tbl->data[ i ].val );\
			}\
		}\
		gs_dyn_array_free( tbl->data );\
		tbl->data = new_tbl.data;\
	}

#define gs_hash_table_key_idx_func( tbl, key, val, _hash_comp_key_func )\
	do {\
		u32 capacity = gs_hash_table_capacity( ( *tbl ) );\
		u32 idx = tbl->hash_func( key ) % capacity;\
		val =  UINT_MAX - 1;\
		for ( u32 i = idx, c = 0; c < capacity; ++c, i = ( ( i + 1 ) % capacity ) )\
		{\
			gs_println( "i: %d, capacity: %d", i, capacity );\
			if ( _hash_comp_key_func( tbl->data[ i ].key, key ) )\
			{\
				val = i;\
				break;\
			}\
		}\
	} while( 0 )


#define gs_hash_table_exists( tbl, k )\
	gs_hash_table_valid_idx( tbl.hash_key_idx_func( &tbl, k ) )

#define gs_hash_table_key_idx( tbl, k )\
	tbl.hash_key_idx_func( &tbl, k )

#define gs_hash_table_reserve( tbl, sz )\
	__gs_hash_table_grow( tbl, sz )

#define gs_hash_table_size( tbl )\
	gs_dyn_array_size( tbl.data )

#define gs_hash_table_empty( tbl )\
	!gs_dyn_array_size( tbl.data )

#define gs_hash_table_capacity( tbl )\
	gs_dyn_array_capacity( tbl.data )

#define gs_hash_table_load_factor( tbl )\
	gs_hash_table_capacity( tbl ) ? (f32)gs_hash_table_size( tbl ) / (f32)gs_hash_table_capacity( tbl ) : 0.0f

// Should this grow to be sizes equal to primes?
#define __gs_hash_table_grow( tbl, sz )\
	do {\
		tbl.hash_grow_func( &tbl, sz );\
	} while ( 0 )

// Need to check load factor for this
// If the load factor grows beyond max, then need to grow data array and rehash
// If shrinks beyond min, then shrink data array and rehash
// Need to handle collisions between keys ( preferrably robin hood hash with linear prob )
// Need a way to determine whether or not a key exists
// TODO(john): Fix bug with load factor ( for some reason, anything higher than 0.5 causes incorrect placement of values in slots )
// TODO(john): Implement Robin Hood hashing for collision resolution
#define gs_hash_table_insert( tbl, k, v )\
	do {\
		u32 capacity = gs_hash_table_capacity( tbl );\
		f32 load_factor = gs_hash_table_load_factor( tbl );\
		if ( load_factor >= 0.5f || !capacity )\
		{\
			__gs_hash_table_grow( tbl, capacity ? capacity * 2 : 1 );\
			capacity = gs_hash_table_capacity( tbl );\
		}\
		u32 hash_idx = tbl.hash_func( k ) % gs_hash_table_capacity( tbl );\
		u32 c = 0;\
		while ( c < capacity && !tbl.hash_comp_key_func( tbl.data[ hash_idx ].key, k ) && tbl.data[ hash_idx ].entry_state == hash_table_entry_active ) {\
			hash_idx = ( ( hash_idx + 1 ) % capacity );\
			c++;\
		}\
		tbl.data[ hash_idx ].key = k;\
		tbl.data[ hash_idx ].val = v;\
		tbl.data[ hash_idx ].entry_state = hash_table_entry_active;\
		gs_dyn_array_size( tbl.data )++;\
	} while ( 0 )

#define gs_hash_table_get( tbl, k )\
	tbl.hash_get_val_func( ( void* )&( tbl ), ( k ) )

#define gs_hash_table_get_ptr( tbl, k )\
	tbl.hash_get_val_ptr_func( ( void* )&( tbl ), ( k ) )

#define gs_hash_table( key_type, val_type )\
	gs_ht_##key_type##_##val_type

#define gs_hash_table_entry( key_type, val_type )\
	gs_ht_entry_##key_type##_##val_type

#define gs_hash_table_new( key_type, val_type )\
	gs_ht_##key_type##_##val_type##_new()

#define gs_hash_table_clear( tbl )\
	gs_dyn_array_clear( tbl.data )


// Some typical hash tables

// Hash table := key: u32, val: u32
gs_hash_table_decl( u32, u32, gs_hash_u32, gs_hash_key_comp_std_type );


/*===================================
// Slot Array
===================================*/

#define gs_slot_array_invalid_handle 	u32_max

typedef struct 
gs_slot_array_base
{
	gs_dyn_array( u32 )	handle_indices;
	gs_dyn_array( u32 )	reverse_indirection_indices;
	gs_dyn_array( u32 )	index_free_list;
} gs_slot_array_base;

#define gs_slot_array_decl( T )\
	typedef struct gs_sa_##T\
	{\
		gs_slot_array_base _base;\
		gs_dyn_array( T ) data;\
		u32 ( * insert_func )( struct gs_sa_##T*, T );\
	} gs_sa_##T;\
\
	_force_inline u32\
	gs_sa_##T##_insert_func( struct gs_sa_##T* s, T v )\
	{\
		u32 free_idx = gs_slot_array_find_next_available_index( ( gs_slot_array_base* )s );\
		gs_dyn_array_push( s->data, v );\
		gs_dyn_array_push( s->_base.reverse_indirection_indices, free_idx );\
		s->_base.handle_indices[ free_idx ] = gs_dyn_array_size( s->data ) - 1;\
		return free_idx;\
	}\
\
	_inline\
	gs_sa_##T __gs_sa_##T##_new()\
	{\
		gs_sa_##T sa = {\
			.data 								= gs_dyn_array_new( T ),\
			._base.handle_indices 				= gs_dyn_array_new( u32 ),\
			._base.reverse_indirection_indices 	= gs_dyn_array_new( u32 ),\
			._base.index_free_list 				= gs_dyn_array_new( u32 ),\
			.insert_func 						= &gs_sa_##T##_insert_func,\
		};\
		return sa;\
	}

_force_inline u32
gs_slot_array_find_next_available_index( gs_slot_array_base* sa )
{
	if ( gs_dyn_array_empty( sa->index_free_list ) ) {
		gs_dyn_array_push( sa->handle_indices, 0 );
		return ( gs_dyn_array_size( sa->handle_indices ) - 1 );
	}
	u32 idx = sa->index_free_list[ 0 ];
	if ( gs_dyn_array_size( sa->index_free_list ) > 1 ) {
		u32 sz = gs_dyn_array_size( sa->index_free_list );
		u32 tmp = sa->index_free_list[ sz - 1 ];
		sa->index_free_list[ 0 ] = tmp;
		gs_dyn_array_pop( sa->index_free_list );
	} else {
		gs_dyn_array_clear( sa->index_free_list );	
	}

	return idx;
}

#define gs_slot_array_clear( s )\
	do {\
		gs_dyn_array_clear( s._base.handle_indices );\
		gs_dyn_array_clear( s._base.index_free_list );\
		gs_dyn_array_clear( s._base.reverse_indirection_indices );\
		gs_dyn_array_clear( s.data );\
	} while ( 0 )

#define gs_slot_array_size( s )\
	gs_dyn_array_size( (s).data )

#define gs_slot_array_new( type )\
	__gs_sa_##type##_new()

#define gs_slot_array_insert( s, v )\
	s.insert_func( &( s ), ( v ) )

#define gs_slot_array_get( s, handle )\
	( s.data[ s._base.handle_indices[ handle ] ] )

#define gs_slot_array_get_unsafe( s, handle )\
	( &s.data[ s._base.handle_indices[ handle ] ] )

#define gs_slot_array_get_ptr( s, handle )\
	( &s.data[ s._base.handle_indices[ handle ] ] )

#define gs_slot_array_handle_valid( s, handle )\
	( handle < gs_slot_array_size( s ) )

#define gs_slot_array( T )\
	gs_sa_##T

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_CONTAINERS_H__
















