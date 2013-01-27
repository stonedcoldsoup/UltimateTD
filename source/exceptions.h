#ifndef UTD_EXCEPTIONS_H
#define UTD_EXCEPTIONS_H

#include "common.h"

namespace UTD
{
	enum exception_id
	{
		EXCEPTION_PARSE_ERROR_ID,
		EXCEPTION_COORDS_OUT_OF_BOUNDS_ID,
		EXCEPTION_INVALID_BUF2_WINDOW_ID,
		EXCEPTION_BUF2_STREAM_TOO_BIG_ID,
		EXCEPTION_INVALID_AUTOTILE_PATTERN_PERMUTATION_ID,
		EXCEPTION_BUF2_LAYER_OUT_OF_BOUNDS_ID,
		EXCEPTION_INVALID_ATLAS_REGION_TEXTURE_HANDLE_ID,
		EXCEPTION_INVALID_ATLAS_REGION_TEXTURE_NAME_ID,
		EXCEPTION_MALFORMED_ATLAS_FILE_ID,
		EXCEPTION_MALFORMED_OR_MISSING_ATLAS_OBJECT_ID,
		EXCEPTION_NO_SUCH_ATLAS_REGION_ID,
		EXCEPTION_ATLAS_DOES_NOT_EXIST_ID,
		EXCEPTION_DEPTH_SUB_LAYER_OUT_OF_RANGE_ID
	};
	
	enum out_of_bounds_reason
	{
		out_of_bounds_negative_coord = 0,
		out_of_bounds_outside_extent,
		out_of_bounds_reason_count
	};
	
	/*namespace detail
	{
		static const char *__out_of_bounds_reason_desc[out_of_bounds_reason_count] =
		{
			"Cannot map negative coordinates to the map.",
			"Cannot map coordinates outside of extent."
		};
	}*/

	UTD_EXCEPTION_TYPE
	(
		parse_error,
		EXCEPTION_PARSE_ERROR_ID,
		"An error was encountered passing game data.",
		"parser",
		"version",
		"data"
	)
	
	/*UTD_EXCEPTION_TYPE
	(
		coords_out_of_bounds,
		EXCEPTION_COORDS_OUT_OF_BOUNDS_ID,
		"Map coordinates are out of bounds (no valid index exists).",
		"reason_desc",
		"reason"
	)*/
	
	UTD_EXCEPTION_TYPE
	(
		invalid_buf2_window,
		EXCEPTION_INVALID_BUF2_WINDOW_ID,
		"Map window coordinates are outside the source buffer.",
		"source_x", "source_y",
		"source_w", "source_h",
		"target_w", "target_h"
	)
	
	UTD_EXCEPTION_TYPE
	(
		buf2_stream_too_big,
		EXCEPTION_BUF2_STREAM_TOO_BIG_ID,
		"Cannot resize virtual buffer to match stream size.",
		"stream_w", "stream_h",
		"target_w", "target_h"
	)
	
	UTD_EXCEPTION_TYPE
	(
		invalid_autotile_pattern_permutation,
		EXCEPTION_INVALID_AUTOTILE_PATTERN_PERMUTATION_ID,
		"Cannot resize virtual buffer to match stream size.",
		"pattern"
	)
	
	UTD_EXCEPTION_TYPE
	(
		buf2_layer_out_of_bounds,
		EXCEPTION_BUF2_LAYER_OUT_OF_BOUNDS_ID, 
		"The given active layer index is out of bounds.",
		"i", "n"
	)
	
	UTD_EXCEPTION_TYPE
	(
		invalid_atlas_region_texture_handle,
		EXCEPTION_INVALID_ATLAS_REGION_TEXTURE_HANDLE_ID, 
		"The texture handle for the given atlas region is invalid.",
		"region_name", "id"
	)
	
	UTD_EXCEPTION_TYPE
	(
		invalid_atlas_region_texture_name,
		EXCEPTION_INVALID_ATLAS_REGION_TEXTURE_NAME_ID, 
		"The texture name for the given atlas region is invalid.",
		"region_name", "name"
	)
	
	UTD_EXCEPTION_TYPE
	(
		malformed_atlas_file,
		EXCEPTION_MALFORMED_ATLAS_FILE_ID, 
		"The atlas file is malformed.",
		"reason"
	)
	
	UTD_EXCEPTION_TYPE
	(
		malformed_or_missing_atlas_object,
		EXCEPTION_MALFORMED_OR_MISSING_ATLAS_OBJECT_ID,
		"The atlas object is malformed or missing.",
		"id"
	)
	
	UTD_EXCEPTION_TYPE
	(
		no_such_atlas_region,
		EXCEPTION_NO_SUCH_ATLAS_REGION_ID,
		"No such atlas region exists.",
		"id"
	)
	
	UTD_EXCEPTION_TYPE
	(
		atlas_does_not_exist,
		EXCEPTION_ATLAS_DOES_NOT_EXIST_ID,
		"The atlas singleton has not yet been initialized."
	)
	
	UTD_EXCEPTION_TYPE
	(
		depth_sub_layer_out_of_range,
		EXCEPTION_DEPTH_SUB_LAYER_OUT_OF_RANGE_ID,
		"Depth sub layer out of range.",
		"i", "cap"
	)
}

#endif
