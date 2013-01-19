#ifndef UTD_TILE_BUILTIN_SETS_H
#define UTD_TILE_BUILTIN_SETS_H

#include "common.h"
#include "tile.h"
#include "atlas.h"
#include "atlas_image.h"
#include "exceptions.h"

namespace UTD
{
	#define UTD_BUILTIN_TILESET_FILENAME "systiles.png"

	static const atlas::tileset_def  g_builtin_tileset_def =
	{
		coord ( 0, 0), // tileset origin
		extent(16,16), // tile size
		extent( 8, 2), // tileset dim
		5              // count in last row
	};

	enum builtin_tile_index: tile_index_type
	{
		UTS_EMPTY                = -1,
		UTS_DEFAULT_MISSING_TILE = 0,
		
		UTS_PATTERNEDIT_SOLID_TILE = 1,
		UTS_PATTERNEDIT_MAYBE_TILE = 2,
		UTS_PATTERNEDIT_EMPTY_TILE = 3,
		UTS_PATTERNEDIT_BG_TILE    = 5,
		
		UTS_AUTOTILEEDIT_BG_TILE = 4,
		
		UTS_AUTOTILEEDIT_NW_TILE = 6,
		UTS_AUTOTILEEDIT_NE_TILE = 7,
		UTS_AUTOTILEEDIT_SE_TILE = 8,
		UTS_AUTOTILEEDIT_SW_TILE = 9,
		
		UTS_AUTOTILEEDIT_N_TILE = 10,
		UTS_AUTOTILEEDIT_S_TILE = 11,
		UTS_AUTOTILEEDIT_W_TILE = 12,
		UTS_AUTOTILEEDIT_E_TILE = 13,
		
		UTS_AUTOTILEEDIT_MISSING_TILE = 14,
		UTS_AUTOTILEEDIT_MARKER_TILE  = 15
	};
	
	static constexpr tile_index_type g_builtin_tileset_neighbors[nbi_count] =
	{
		UTS_AUTOTILEEDIT_NE_TILE, //nbi_NE,
		UTS_AUTOTILEEDIT_N_TILE,  //nbi_N,
		UTS_AUTOTILEEDIT_NW_TILE, //nbi_NW,
		UTS_AUTOTILEEDIT_W_TILE,  //nbi_W,
		UTS_AUTOTILEEDIT_E_TILE,  //nbi_E,
		UTS_AUTOTILEEDIT_SW_TILE, //nbi_SW,
		UTS_AUTOTILEEDIT_S_TILE,  //nbi_S,
		UTS_AUTOTILEEDIT_SE_TILE  //nbi_SE
	};

	class builtin_tileset:
		public util::singleton<builtin_tileset>
	{
	private:
		atlas *m_atlas;
		atlas::handle_type tex_id, rgn_id;
		atlas::image_factory *m_factory;
		
		builtin_tileset();
		~builtin_tileset();
		
		friend class util::singleton<builtin_tileset>;
	public:
		builtin_tileset(builtin_tileset &&m) = delete;
		builtin_tileset(const builtin_tileset &m) = delete;
		
		builtin_tileset &operator =(builtin_tileset &&m) = delete;
		builtin_tileset &operator =(const builtin_tileset &m) = delete;
	
		      atlas::image_factory *get_tileset_factory()       {return m_factory;}
		const atlas::image_factory *get_tileset_factory() const {return m_factory;}
		
		atlas::handle_type get_tileset_factory_region() const   {return rgn_id;}
	};
}

#endif
