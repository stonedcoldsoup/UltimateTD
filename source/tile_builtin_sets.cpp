#include "tile_builtin_sets.h"

namespace UTD
{
	FUNGUSUTIL_SINGLETON_INSTANCE(builtin_tileset)
	
	builtin_tileset::builtin_tileset():
		util::singleton<builtin_tileset>(),
		m_atlas(atlas::instance()),
		tex_id(0), rgn_id(0),
		m_factory(nullptr)
	{
		if (m_atlas == nullptr)
			throw atlas_does_not_exist();
	
		tex_id = m_atlas->create_texture(UTD_BUILTIN_TILESET_FILENAME, false);
		if (tex_id)
			rgn_id = m_atlas->create_region("_builtin_tileset", tex_id, g_builtin_tileset_def);
			
		m_factory = new atlas::image_factory(rgn_id);
	}
	
	builtin_tileset::~builtin_tileset()
	{
		delete m_factory;
	}
}