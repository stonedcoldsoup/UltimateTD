#include "atlas_image.h"
#include "atlas_image_geom.h"

namespace UTD
{
	util::block_allocator<     atlas::image, 1024> atlas::image_factory::m_alloc;
	util::block_allocator<atlas::rect_image, 4096> atlas::image_factory::m_alloc_rect;
	
	struct atlas::image_factory::clone_delegate
	{
		const image_factory *_self;
	};

	atlas::image::image(atlas::image_factory &m_factory, BatchGeometryPtr m_geom, coord m_pos, extent m_extent, const image_config &m_config):
		m_factory(&m_factory),
		m_geom(m_geom),
		m_pos(m_pos),
		m_extent(m_extent),
		m_config(m_config),
		b_dirty(true),
		b_geom_attrib_dirty(true)
	{
		update();
	}
	
	atlas::image::~image()
	{
		if (m_geom) m_geom->drop();
	}

	atlas::rect_image::rect_image(atlas::image_factory &m_factory, BatchGeometryPtr m_geom, coord m_pos, extent m_extent, float depth):
		m_factory(&m_factory),
		m_geom(m_geom),
		m_pos(m_pos),
		m_extent(m_extent),
		depth(depth),
		b_dirty(true),
		b_geom_attrib_dirty(true)
	{
		update();
	}
	
	atlas::rect_image::~rect_image()
	{
		if (m_geom) m_geom->drop();
	}
	
	inline void atlas::image_factory::init()
	{
		if (m_atlas == nullptr)
			throw atlas_does_not_exist();
	
		if (!m_atlas->get_handle_info(id, m_info))
			throw malformed_or_missing_atlas_object(id);
		
		size_type n_tiles = m_atlas->get_region_tile_count(m_info);
		if (n_tiles > 0)
		{
			m_coords.resize(n_tiles);
			for (size_type i = 0; i < n_tiles; ++i)
				m_atlas->resolve_texture_region(m_info, m_coords[i], m_extent, i);
		}
		else
		{
			m_coords.resize(1);
			m_atlas->resolve_texture_region(m_info, m_coords[0], m_extent);
		}
		
		m_out_extent = m_extent;
	}

	atlas::image_factory::image_factory(handle_type id, const image_config &m_config):
		m_atlas(atlas::instance()), id(id), m_config(m_config), b_clip(false)
	{
		m_clone_delegate = new clone_delegate{this};
	
		init();
	}
	
	atlas::image_factory::image_factory(const clone_delegate &m):
		m_atlas(atlas::instance()), id(m._self->id), m_config(m._self->m_config), b_clip(m._self->b_clip)
	{
		m_clone_delegate = new clone_delegate{this};
	
		init();
		
		m_out_extent = m._self->m_out_extent;
		
		m_clip_coords = m._self->m_clip_coords;
		m_clip_extent = m._self->m_clip_extent;
	}
	
	atlas::image_factory::~image_factory()
	{
		delete m_clone_delegate;
	}
		
	const atlas::image_factory::clone_delegate &atlas::image_factory::clone() const
	{
		return *m_clone_delegate;
	}
	
#define _impl_UTDEXTT_P      _UTDEXTT_P(m_geom, mp, dat)
#define _impl_UTDQUAD_P      _UTDQUAD_P(m_geom, mp, dat)
#define _impl_UTDEXTT        _UTDEXTT(m_geom, dat)
#define _impl_UTDQUAD        _UTDQUAD(m_geom, dat)
#define _impl_UTDEXTT_INIT   _UTDEXTT_INIT(dat,m_extent.x,m_extent.y)
#define _impl_UTDEXTT_INIT_S _UTDEXTT_INIT_S(dat,m_extent.x,m_extent.y,m_config.m_scl.getX(),m_config.m_scl.getY())
#define _impl_UTDQUAD_INIT   _UTDQUAD_INIT(dat,m_extent.x,m_extent.y)
#define _impl_UTDQUAD_INIT_S _UTDQUAD_INIT_S(dat,m_extent.x,m_extent.y,m_config.m_scl.getX(),m_config.m_scl.getY())
#define _impl_UTDXFFINISH    _UTD_DOXFORM(m_geom,m_config.m_rot,mp)
	
	void atlas::image::update()
	{
		if (m_geom)
		{
			if (b_dirty)
			{
				const Vector2d mp(m_pos.x, m_pos.y);
				float dat[4]; // precomputation

				switch (m_config.i_flags)
				{
				case 0:												_impl_UTDEXTT_INIT   _impl_UTDEXTT_P                 break;
				case image_config::centered: 						_impl_UTDQUAD_INIT   _impl_UTDQUAD_P                 break;
				case image_config::scaled: 							_impl_UTDEXTT_INIT_S _impl_UTDEXTT_P                 break;
				case image_config::centered|image_config::scaled: 	_impl_UTDQUAD_INIT_S _impl_UTDQUAD_P                 break;
				case image_config::scaled|image_config::rotated: 	_impl_UTDEXTT_INIT_S _impl_UTDEXTT _impl_UTDXFFINISH break;
				case image_config::centered|image_config::scaled|image_config::rotated:
																	_impl_UTDQUAD_INIT_S _impl_UTDQUAD _impl_UTDXFFINISH break;
				case image_config::rotated: 						_impl_UTDEXTT_INIT   _impl_UTDEXTT _impl_UTDXFFINISH break;
				case image_config::centered|image_config::rotated: 	_impl_UTDQUAD_INIT   _impl_UTDQUAD _impl_UTDXFFINISH break;
				default:
					break;
				};
				
				b_dirty = false;
			}
			
			if (b_geom_attrib_dirty)
			{
				m_geom->setDepth(m_config.depth);
				b_geom_attrib_dirty = false;
			}
			
			m_factory->clip_geometry(m_geom);
		}
	}
	
	void atlas::rect_image::update()
	{
		if (m_geom)
		{
			if (b_dirty)
			{
				Vector2d mp(m_pos.x, m_pos.y);
				float dat[4];
				
				_impl_UTDEXTT_INIT
				_impl_UTDEXTT_P
				
				b_dirty = false;
			}
			
			if (b_geom_attrib_dirty)
			{
				m_geom->setDepth(depth);
				b_geom_attrib_dirty = false;
			}
			
			m_factory->clip_geometry(m_geom);
		}
	}
	
	void atlas::image::reset_to_config()
	{
		m_extent = m_factory->get_out_extent();
		m_config = m_factory->get_config();
		
		b_dirty = true;
	}
	
	void atlas::rect_image::reset_to_config()
	{
		m_extent = m_factory->get_out_extent();
		depth    = m_factory->get_config().depth;
		
		b_dirty = true;
	}
	
	bool atlas::image::set_tile(tile_index_type i_tile)
	{
		if (m_geom)
		{
			BatchGeometryPtr m_new_geom = m_factory->gen_texcoords(m_geom, i_tile);
			if (m_new_geom != m_geom) m_geom->drop();
			return (m_geom = m_new_geom) != BatchGeometryPtr();
		}
		else
			return (m_geom = m_factory->gen_geometry(i_tile)) != BatchGeometryPtr();
	}
	
	bool atlas::rect_image::set_tile(tile_index_type i_tile)
	{
		if (m_geom)
		{
			BatchGeometryPtr m_new_geom = m_factory->gen_texcoords(m_geom, i_tile);
			if (m_new_geom != m_geom) m_geom->drop();
			return (m_geom = m_new_geom) != BatchGeometryPtr();
		}
		else
			return (m_geom = m_factory->gen_geometry(i_tile)) != BatchGeometryPtr();
	}
	
	bool atlas::image::set_tile(tile_index_type i_tile, atlas::image_factory &m_factory, factory_switch e_mode)
	{
		if ((this->m_factory != &m_factory) && m_geom)
		{
			m_geom->drop();
			m_geom = BatchGeometryPtr();
		}
	
		this->m_factory = &m_factory;
		switch (e_mode)
		{
		case factory_switch::change_config:
			reset_to_config();
			break;
		case factory_switch::retain_config:
		default:
			break;
		};
		
		return set_tile(i_tile);
	}
	
	bool atlas::rect_image::set_tile(tile_index_type i_tile, atlas::image_factory &m_factory, factory_switch e_mode)
	{
		if ((this->m_factory != &m_factory) && m_geom)
		{
			m_geom->drop();
			m_geom = BatchGeometryPtr();
		}
	
		this->m_factory = &m_factory;
		switch (e_mode)
		{
		case factory_switch::change_config:
			reset_to_config();
			break;
		case factory_switch::retain_config:
		default:
			break;
		};

		return set_tile(i_tile);
	}
}