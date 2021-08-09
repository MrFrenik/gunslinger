/*================================================================
	* Copyright: 2020 John Jackson
	* GSAsset: Asset Manager Util for Gunslinger
	* File: gs_asset.h
	All Rights Reserved
=================================================================*/

#ifndef GS_ASSET_H
#define GS_ASSET_H

/*
	USAGE: (IMPORTANT)

	=================================================================================================================

	Before including, define the gunslinger asset manager implementation like this:

	    #define GS_ASSET_IMPL

    in EXACTLY ONE C or C++ file that includes this header, BEFORE the
    include, like this:

		#define GS_ASSET_IMPL
		#include "gs_asset.h"

    All other files should just #include "gs_asset.h" without the #define.

    MUST include "gs.h" and declare GS_IMPL BEFORE this file, since this file relies on that:

    	#define GS_IMPL
    	#include "gs.h"

    	#define GS_ASSET_IMPL
    	#include "gs_asset.h"

	================================================================================================================
*/

/*==== Interface ====*/

/** @defgroup gs_asset_util Asset Util
 *  Gunslinger Asset Util
 *  @{
 */

// Asset handle
typedef struct gs_asset_t
{
	uint64_t type_id;
	uint32_t asset_id;
	uint32_t importer_id;	// 'Unique' id of importer, used for type safety
} gs_asset_t;

GS_API_DECL gs_asset_t __gs_asset_handle_create_impl(uint64_t type_id, uint32_t asset_id, uint32_t importer_id);

#define gs_asset_handle_create(T, ID, IMPID)\
	__gs_asset_handle_create_impl(gs_hash_str64(gs_to_str(T)), ID, IMPID)

typedef void (* gs_asset_load_func)(const char *,void *,...);
typedef gs_asset_t (* gs_asset_default_func)(void *);

typedef struct gs_asset_importer_desc_t {
	void (* load_from_file)(const char* path, void* out, ...);
	gs_asset_t (* default_asset)(void* out);
} gs_asset_importer_desc_t;

typedef struct gs_asset_importer_t 
{
	void* slot_array;
	void* slot_array_data_ptr;
	void* slot_array_indices_ptr;
	void* tmp_ptr;
	uint32_t tmpid;
	size_t data_size;
	gs_asset_importer_desc_t desc;
	uint32_t importer_id;
	gs_asset_t default_asset;
} gs_asset_importer_t;

GS_API_DECL void gs_asset_default_load_from_file( const char* path, void* out );
GS_API_DECL gs_asset_t gs_asset_default_asset();
GS_API_DECL void gs_asset_importer_set_desc(gs_asset_importer_t* imp, gs_asset_importer_desc_t* desc);

#define gs_assets_get_importerp(AM, T)\
	(gs_hash_table_getp((AM)->importers, gs_hash_str64(gs_to_str(T))))

#ifdef __cplusplus
	#define gsa_imsa(IMPORTER, T)\
		(decltype(gs_slot_array(T))(IMPORTER)->slot_array)
#else
	#define gsa_imsa(IMPORTER, T)\
		((gs_slot_array(T))(IMPORTER)->slot_array)
#endif

#define gs_assets_register_importer(AM, T, DESC)\
	do {\
		gs_asset_importer_t ai = gs_default_val();\
		ai.data_size = sizeof(T);\
		ai.importer_id = (AM)->free_importer_id++;\
		gs_asset_importer_set_desc(&ai, (DESC));\
		size_t sz = 2 * sizeof(void*) + sizeof(T);\
		gs_slot_array(T) sa = NULL;\
		gs_slot_array_init((void**)&sa, sizeof(*sa));\
		gs_dyn_array_init((void**)&sa->indices, sizeof(uint32_t));\
		gs_dyn_array_init((void**)&sa->data, sizeof(T));\
		ai.slot_array = (void*)sa;\
		ai.tmp_ptr = (void*)&sa->tmp;\
		ai.slot_array_indices_ptr = (void*)sa->indices;\
		ai.slot_array_data_ptr = (void*)sa->data;\
		if (!ai.desc.load_from_file) {ai.desc.load_from_file = (gs_asset_load_func)&gs_asset_default_load_from_file;}\
		gs_hash_table_insert((AM)->importers, gs_hash_str64(gs_to_str(T)), ai);\
	} while(0)

// Need a way to be able to print upon assert
#define gs_assets_load_from_file(AM, T, PATH, ...)\
	(\
		/*gs_assert(gs_hash_table_key_exists((AM)->importers, gs_hash_str64(gs_to_str(T)))),*/\
		(AM)->tmpi = gs_hash_table_getp((AM)->importers, gs_hash_str64(gs_to_str(T))),\
		(AM)->tmpi->desc.load_from_file(PATH, (AM)->tmpi->tmp_ptr, ## __VA_ARGS__),\
		(AM)->tmpi->tmpid = gs_slot_array_insert_func(&(AM)->tmpi->slot_array_indices_ptr, &(AM)->tmpi->slot_array_data_ptr, (AM)->tmpi->tmp_ptr, (AM)->tmpi->data_size, NULL),\
		gs_asset_handle_create(T, (AM)->tmpi->tmpid, (AM)->tmpi->importer_id)\
	)

#define gs_assets_create_asset(AM, T, DATA)\
	(\
		/*gs_assert(gs_hash_table_key_exists((AM)->importers, gs_hash_str64(gs_to_str(T)))),*/\
		(AM)->tmpi = gs_hash_table_getp((AM)->importers, gs_hash_str64(gs_to_str(T))),\
		(AM)->tmpi->tmp_ptr = (DATA),\
		(AM)->tmpi->tmpid = gs_slot_array_insert_func(&(AM)->tmpi->slot_array_indices_ptr, &(AM)->tmpi->slot_array_data_ptr, (AM)->tmpi->tmp_ptr, (AM)->tmpi->data_size, NULL),\
		gs_asset_handle_create(T, (AM)->tmpi->tmpid, (AM)->tmpi->importer_id)\
	)

typedef struct gs_asset_manager_t
{
	gs_hash_table(uint64_t, gs_asset_importer_t) importers;	// Maps hashed types to importer
	gs_asset_importer_t* tmpi;								// Temporary importer for caching 
	uint32_t free_importer_id;
} gs_asset_manager_t;

GS_API_DECL gs_asset_manager_t gs_asset_manager_new();
GS_API_DECL void gs_asset_manager_free(gs_asset_manager_t* am);
GS_API_DECL void* __gs_assets_getp_impl(gs_asset_manager_t* am, uint64_t type_id, gs_asset_t hndl);

#define gs_assets_getp(AM, T, HNDL)\
	(T*)(__gs_assets_getp_impl(AM, gs_hash_str64(gs_to_str(T)), HNDL))

#define gs_assets_get(AM, T, HNDL)\
	*(gs_assets_getp(AM, T, HNDL));

/** @} */ // end of gs_asset_util

/*==== Implementation ====*/

#ifdef GS_ASSET_IMPL

gs_asset_t __gs_asset_handle_create_impl(uint64_t type_id, uint32_t asset_id, uint32_t importer_id)
{
	gs_asset_t asset = gs_default_val();
	asset.type_id = type_id;
	asset.asset_id = asset_id;
	asset.importer_id = importer_id;
	return asset;
}

gs_asset_manager_t gs_asset_manager_new()
{
	gs_asset_manager_t assets = gs_default_val();

	// Register default asset importers
	gs_asset_importer_desc_t tex_desc   = gs_default_val();
	gs_asset_importer_desc_t font_desc  = gs_default_val();
	gs_asset_importer_desc_t audio_desc = gs_default_val();
	gs_asset_importer_desc_t mesh_desc  = gs_default_val();
	gs_asset_importer_desc_t asset_desc  = gs_default_val();

	tex_desc.load_from_file = (gs_asset_load_func)&gs_asset_texture_load_from_file;
	font_desc.load_from_file = (gs_asset_load_func)&gs_asset_font_load_from_file;
	audio_desc.load_from_file = (gs_asset_load_func)&gs_asset_audio_load_from_file;
	mesh_desc.load_from_file = (gs_asset_load_func)&gs_asset_mesh_load_from_file;

	gs_assets_register_importer(&assets, gs_asset_t, &asset_desc);
	gs_assets_register_importer(&assets, gs_asset_texture_t, &tex_desc);
	gs_assets_register_importer(&assets, gs_asset_font_t, &font_desc);
	gs_assets_register_importer(&assets, gs_asset_audio_t, &audio_desc);
	gs_assets_register_importer(&assets, gs_asset_mesh_t, &mesh_desc);

	return assets;
}

void gs_asset_manager_free(gs_asset_manager_t* am)
{
	// Free all data	
}

void* __gs_assets_getp_impl(gs_asset_manager_t* am, uint64_t type_id, gs_asset_t hndl)
{
	if (type_id != hndl.type_id) { 
		gs_println("Warning: Type id: %zu doesn't match handle type id: %zu.", type_id, hndl.type_id);
		gs_assert(false);
		return NULL;
	}

	// Need to grab the appropriate importer based on type	
	if (!gs_hash_table_key_exists(am->importers, type_id)) {
		gs_println("Warning: Importer type %zu does not exist.", type_id);
		gs_assert(false);
		return NULL;
	}

	gs_asset_importer_t* imp = gs_hash_table_getp(am->importers, type_id);

	// Vertify that importer id and handle importer id align
	if (imp->importer_id != hndl.importer_id) {
		gs_println("Warning: Importer id: %zu does not match handle importer id: %zu.", 
			imp->importer_id, hndl.importer_id);
		gs_assert(false);
		return NULL;
	}

	// Need to get data index from slot array using hndl asset id
	size_t offset = (((sizeof(uint32_t) * hndl.asset_id) + 3) & (~3));
	uint32_t idx = *(uint32_t*)((char*)(imp->slot_array_indices_ptr) + offset);
	// Then need to return pointer to data at index
	size_t data_sz = imp->data_size;
	size_t s = data_sz == 8 ? 7 : 3;
	offset = (((data_sz * idx) + s) & (~s));
	return ((char*)(imp->slot_array_data_ptr) + offset);
}

void gs_asset_importer_set_desc(gs_asset_importer_t* imp, gs_asset_importer_desc_t* desc)
{
	imp->desc = desc ? *desc : imp->desc;
	imp->desc.load_from_file = imp->desc.load_from_file ? (gs_asset_load_func)imp->desc.load_from_file 
					: (gs_asset_load_func)&gs_asset_default_load_from_file; 
	imp->desc.default_asset = imp->desc.default_asset ? (gs_asset_default_func)imp->desc.default_asset 
					: (gs_asset_default_func)&gs_asset_default_asset; 
}

gs_asset_t gs_asset_default_asset()
{
	gs_asset_t a = gs_default_val();
	return a;
}

void gs_asset_default_load_from_file(const char* path, void* out) 
{
	// Nothing...
}

#undef GS_ASSET_IMPL
#endif // GS_ASSET_IMPL 

#endif // GS_ASSET_H




















