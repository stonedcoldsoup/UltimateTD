#include "atlas.h"

namespace UTD
{
	FUNGUSUTIL_SINGLETON_INSTANCE(atlas)

	atlas::region_info::region_info(extent m_coords,
				                    extent m_extent):
		m_coords(m_coords), m_extent(m_extent),
		m_tile_extent(), n_cols(0), n_tiles(0),
		b_tiled(false)
	{}
	
	atlas::region_info::region_info(extent m_coords,
									extent m_extent,
									extent m_tile_extent,
									size_type n_cols,
									size_type n_tiles):
		m_coords(m_coords), m_extent(m_extent),
		m_tile_extent(m_tile_extent), n_cols(n_cols), n_tiles(n_tiles),
		b_tiled(true)
	{}
	
	bool atlas::region_info::correct_tiling()
	{
		if (b_tiled)
		{
			if (min(m_tile_extent, m_extent) != m_tile_extent) return false;
		
			if (n_cols == 0)
			{
				n_cols = m_extent.x / m_tile_extent.x;
				if (n_cols == 0) return false;
			}
			else if (n_cols * m_tile_extent.x > m_extent.x)
				return false;
			
			if (n_tiles == 0)
			{
				n_tiles = m_extent.y / m_tile_extent.y * n_cols;
				if (n_tiles == 0) return false;
			}
			else if (n_tiles > m_extent.y / m_tile_extent.y * n_cols)
				return false;
		}
		return true;
	}
	
	std::istream &atlas::region_info::read(std::istream &is)
	{
		is >> m_coords >> m_extent;
		if (is >> b_tiled)
		{
			if (b_tiled)
				is >> m_tile_extent >> n_cols >> n_tiles;
		}
		return is;
	}
	
	std::ostream &atlas::region_info::write(std::ostream &os) const
	{
		if (os << m_coords << ' ' << m_extent << ' ' << b_tiled)
		{
			if (b_tiled)
				os << ' ' << m_tile_extent << ' ' << n_cols << ' ' << n_tiles;
		}
		return os;
	}
	
	bool atlas::resolve_texture_region(handle_info &m_info, coord &m_coords, extent &m_extent, tile_index_type i_tile)
	{
		if (m_info.m_region)
		{
			m_coords = m_info.m_region->m_info.m_coords;
			
			if (m_info.m_region->m_info.b_tiled)
			{
				size_type n_tiles = m_info.m_region->m_info.n_tiles;
				if (i_tile < 0 || i_tile >= n_tiles)
					return false; // tile doesn't exist, out of range!
			
				m_extent = m_info.m_region->m_info.m_tile_extent;
				size_type n_cols = m_info.m_region->m_info.n_cols;
				
				m_coords = extent(i_tile % n_cols, i_tile / n_cols) * m_extent + m_coords;
			}
			else
				m_extent = m_info.m_region->m_info.m_extent;
		}
		else
		{
			m_coords = coord();
			m_extent = m_info.m_tex_data->m_extent;
		}
		return true;
	}
	
	void atlas::gen_texcoords(BatchGeometryPtr m_geom, texture_data *m_tex_data, coord m_coords, extent m_extent) const
	{
		float tw = float(m_tex_data->m_extent.x);
		float th = float(m_tex_data->m_extent.y);

		Vector2d originuv(float(m_coords.x)/tw, float(m_coords.y)/th);
		Vector2d spanuv(float(m_extent.x)/tw, float(m_extent.y)/th);
		
		(*m_geom)[3].tcoords = TextureCoords((originuv+spanuv).getX(), originuv.getY());
		(*m_geom)[2].tcoords = TextureCoords((originuv+spanuv).getX(), (originuv+spanuv).getY());
		(*m_geom)[1].tcoords = TextureCoords(originuv.getX(), (originuv+spanuv).getY());
		(*m_geom)[0].tcoords = TextureCoords(originuv.getX(), originuv.getY());
	}
	
	BatchGeometryPtr atlas::gen_geometry(texture_data *m_tex_data, coord m_coords, extent m_extent, float depth)
	{
		if (!m_tex_data)
			return BatchGeometryPtr();
	
		BatchGeometryPtr m_geom = new BatchGeometry(m_rendersystem.getBatchRenderer(),
													phoenix::Rectangle(0, 0, m_extent.x, m_extent.y),
													m_tex_data->m_tex, 0 /*group*/, depth);
		m_geom->setImmediate(false);
		gen_texcoords(m_geom, m_tex_data, m_coords, m_extent);
		
		return m_geom;
	}
	
	bool atlas::get_handle_info_impl(handle_type id, handle_class e_class, handle_info &m_info)
	{
		bool b_success = false;
	
		switch (e_class)
		{
		case handle_class::texture:
			{
				m_info.m_tex_data = m_tex_alloc.find(id);
				if (m_info.m_tex_data) b_success = true;
			} break;
		case handle_class::region:
			{
				m_info.m_region = m_rgn_alloc.find(id);
				if (m_info.m_region)
					b_success = get_handle_info_impl(m_info.m_region->texture_id, handle_class::texture, m_info);
			} break;
		case handle_class::nil:
		default:
			break;
		};
		
		return b_success;
	}
	
	atlas::atlas(RenderSystem &m_rendersystem):
		util::singleton<atlas>(),
		m_rendersystem(m_rendersystem)
	{}
	
	bool atlas::gen_region_info(region_info &m_info, handle_type src_id, const tileset_def &m_def)
	{
		const texture_data *m_tex_data = m_tex_alloc.find(src_id);
		
		if (m_tex_data)
		{
			if (m_def.m_dim_extent.x == 0 || m_def.m_dim_extent.y == 0)
				return false;
			
			size_type n_last_row_out = (m_def.n_last_row == 0 || m_def.n_last_row > m_def.m_dim_extent.x) ?
			                           m_def.m_dim_extent.x                                               :
									   m_def.n_last_row;
		
			extent m_out_extent = m_def.m_dim_extent * m_def.m_tile_extent;
			
			// make sure it's not out of range
			if (max(m_def.m_coords + m_out_extent, m_tex_data->m_extent) != m_tex_data->m_extent)
				return false;
			
			m_info = region_info(m_def.m_coords, m_out_extent, // coords and extent
								 m_def.m_tile_extent,          // tile size
								 m_def.m_dim_extent.x,         // column count
								 m_def.m_dim_extent.size() - m_def.m_dim_extent.x + n_last_row_out);  // tile count
		}
		
		return m_tex_data;
	}
	
	bool atlas::gen_region_info(region_info &m_info, handle_type src_id, const region_def &m_def)
	{
		const texture_data *m_tex_data = m_tex_alloc.find(src_id);
		
		if (m_tex_data)
			m_info = region_info(m_def.m_coords, min(m_def.m_coords + m_def.m_extent, m_tex_data->m_extent) - m_def.m_coords);
		
		return m_tex_data;
	}
	
	bool atlas::gen_region_info(region_info &m_info, handle_type src_id)
	{
		const texture_data *m_tex_data = m_tex_alloc.find(src_id);
		
		if (m_tex_data)
			m_info = region_info(coord(), m_tex_data->m_extent);
		
		return m_tex_data;
	}
	
	atlas::handle_type atlas::create_texture(const std::string &m_name, bool b_linear_filter)
	{
		TexturePtr m_tex = m_rendersystem.loadTexture(m_name, b_linear_filter);
		if (!m_tex) return 0;
		
		return m_tex_alloc.alloc(m_name, texture_data{m_tex, extent(m_tex->getWidth(), m_tex->getHeight()), b_linear_filter});
	}
	
	atlas::handle_type atlas::create_region(const std::string &m_name, handle_type src_id, region_info m_info)
	{
		texture_data *m_tex_data = m_tex_alloc.find(src_id);
		if (!m_tex_data) return 0;
		
		extent tmp = m_info.m_coords + m_info.m_extent;
		if (tmp.x > m_tex_data->m_extent.x || tmp.y > m_tex_data->m_extent.y) return 0;
			
		if (!m_info.correct_tiling()) return 0;
		
		return m_rgn_alloc.alloc(m_name, region{src_id, m_info});
	}
	
	atlas::handle_type atlas::get_texture_handle(const std::string &m_name) const
	{
		return m_tex_alloc.find_id(m_name);
	}
	
	atlas::handle_type atlas::get_region_handle(const std::string &m_name) const
	{
		return m_rgn_alloc.find_id(m_name);
	}
	
	const std::string &atlas::get_texture_name(handle_type id) const
	{
		return m_tex_alloc.find_name(id);
	}
	
	const std::string &atlas::get_region_name(handle_type id) const
	{
		return m_rgn_alloc.find_name(id);
	}
	
	const atlas::texture_data *atlas::get_texture_data(const std::string &m_name) const {return m_tex_alloc.find(m_name);}
	const atlas::texture_data *atlas::get_texture_data(handle_type id)            const {return m_tex_alloc.find(id);}
	
	const atlas::region       *atlas::get_region(const std::string &m_name) const {return m_rgn_alloc.find(m_name);}
	const atlas::region       *atlas::get_region(handle_type id)            const {return m_rgn_alloc.find(id);}
	
	bool atlas::destroy_texture(const std::string &m_name) {return m_tex_alloc.free(m_name);}
	bool atlas::destroy_texture(handle_type id)            {return m_tex_alloc.free(id);}
	bool atlas::destroy_region(const std::string &m_name)  {return m_rgn_alloc.free(m_name);}
	bool atlas::destroy_region(handle_type id)             {return m_rgn_alloc.free(id);}
	
	void atlas::recycle_texture_handles() {m_tex_alloc.recycle();}
	void atlas::recycle_region_handles()  {m_rgn_alloc.recycle();}
	
	std::istream &atlas::read(std::istream &is)
	{
		std::string m_guard;
		
		is >> m_guard;
		if (m_guard == "TEX")
		{
			// read textures
			std::string m_tex_name;
			bool b_linear_filter;
			
			while (is)
			{
				if (is >> m_tex_name)
				{
					if (m_tex_name == "END")
						break;
					
					if (is >> b_linear_filter)
						create_texture(m_tex_name, b_linear_filter);
				}
			}
			
			if (!is) return is;
		}
		else
			throw malformed_atlas_file("Missing textures list at beginning of file.");
		
		is >> m_guard;
		if (m_guard == "RGN")
		{
			// read regions
			std::string m_rgn_name;
			std::string m_tex_name;
			
			while (is)
			{
				if (is >> m_rgn_name)
				{
					if (m_rgn_name == "END")
						break;
					
					if (is >> m_tex_name)
					{
						handle_type texture_id = m_tex_alloc.find_id(m_tex_name);
						if (texture_id == 0)
							throw invalid_atlas_region_texture_name(m_rgn_name, m_tex_name);
						
						region_info m_info;
						if (!m_info.read(is))
							return is;
						
						create_region(m_rgn_name, texture_id, m_info);
					}
				}
			}
		}
		else
			throw malformed_atlas_file("Missing regions list after textures list.");
		
		return is;
	}
	
	std::ostream &atlas::write(std::ostream &os) const
	{
		// write textures
		os << "TEX";
		for (const auto &m_entry: m_tex_alloc)
		{
			const auto *m_tex_data = m_entry.value;
			if (!(os << ' ' << m_tex_data->m_name << ' ' << m_tex_data->m_data.b_linear_filter))
				return os;
		}
		os << " END";
		
		// write regions
		os << " RGN";
		for (const auto &m_entry: m_rgn_alloc)
		{
			const auto *m_rgn_data = m_entry.value;
			std::string m_tex_name = m_tex_alloc.find_name(m_rgn_data->m_data.texture_id);
			if (m_tex_name.empty())
				throw invalid_atlas_region_texture_handle(m_rgn_data->m_name, m_rgn_data->m_data.texture_id);
			
			os << ' ' << m_rgn_data->m_name << ' ' << m_tex_name << ' ';
			if (!m_rgn_data->m_data.m_info.write(os))
				return os;
		}
		os << " END";
		
		return os;
	}
}
