#include "map_render.h"

namespace UTD
{
	inline bool map_metrics::compute_map_region(const Vector2d &m_offset, region &m_rgn)
	{
		m_rgn.m_pos.setX(-(m_offset.getX() < 0.0f ? m_offset.getX() : std::fmod(m_offset.getX(), m_tile_size.getX())));
		m_rgn.m_pos.setY(-(m_offset.getY() < 0.0f ? m_offset.getY() : std::fmod(m_offset.getY(), m_tile_size.getY())));
		
		Vector2d m_range = Vector2d(m_clip_extent) - m_rgn.m_pos;
		m_rgn.m_pos += Vector2d(m_clip_coords);
		
		m_rgn.m_coords.x = coord::integer_type(m_offset.getX() < 0.0f ? 0 : std::floor(m_offset.getX()/m_tile_size.getX()));
		m_rgn.m_coords.y = coord::integer_type(m_offset.getY() < 0.0f ? 0 : std::floor(m_offset.getY()/m_tile_size.getY()));
						 
		m_rgn.m_extent.x = extent::integer_type(std::ceil(m_range.getX()/m_tile_size.getX()));
		m_rgn.m_extent.y = extent::integer_type(std::ceil(m_range.getY()/m_tile_size.getY()));
		
		return true;
	}

	map_renderer_base::map_renderer_base(const map_metrics &m_metrics):
		m_metrics(m_metrics)
	{}
	
	map_renderer_base::~map_renderer_base() = default;
	
	logic_buf_renderer::logic_buf_renderer(const map_metrics &m_metrics):
		map_renderer_base(m_metrics)
	{}
	
	inline void neighbor_renderer::get_pattern_rects(coord m_coords, uint8_t bits)
	{
		if (bits & nbb_NW) m_rects.push_back(neighbor_debug_marker::get_rect<nbi_NW>(m_coords, get_tile_width(), get_tile_height()));
		if (bits & nbb_N)  m_rects.push_back(neighbor_debug_marker::get_rect<nbi_N> (m_coords, get_tile_width(), get_tile_height()));
		if (bits & nbb_NE) m_rects.push_back(neighbor_debug_marker::get_rect<nbi_NE>(m_coords, get_tile_width(), get_tile_height()));
		if (bits & nbb_W)  m_rects.push_back(neighbor_debug_marker::get_rect<nbi_W> (m_coords, get_tile_width(), get_tile_height()));
		if (bits & nbb_E)  m_rects.push_back(neighbor_debug_marker::get_rect<nbi_E> (m_coords, get_tile_width(), get_tile_height()));
		if (bits & nbb_SW) m_rects.push_back(neighbor_debug_marker::get_rect<nbi_SW>(m_coords, get_tile_width(), get_tile_height()));
		if (bits & nbb_S)  m_rects.push_back(neighbor_debug_marker::get_rect<nbi_S> (m_coords, get_tile_width(), get_tile_height()));
		if (bits & nbb_SE) m_rects.push_back(neighbor_debug_marker::get_rect<nbi_SE>(m_coords, get_tile_width(), get_tile_height()));
	}
	
	inline void neighbor_renderer::draw_rects(const Vector2d &m_pos)
	{
		for (phoenix::Rectangle m_rect: m_rects)
		{
			m_rect.setPosition(m_rect.getPosition() + m_pos);
			m_graphicsfactory.drawRectangle(m_rect,
										 Color(192,192,255), Color(192,192,255),
										 Color(192,192,255), Color(192,192,255));
		}
	}
	
	void state_renderer::draw(const logic_tile_buf::bufi *m_bufi, const map_metrics::region &m_rgn)
	{
		logic_tile_buf::bufw_const m_bufw(m_bufi, m_rgn.m_coords, m_rgn.m_extent);
		m_bufw.each_in
		(
			coord(),
			m_bufw.get_extent(),
			[&] (coord m_coord, bool state)
			{
				if (state)
					m_graphicsfactory.drawRectangle(phoenix::Rectangle(m_rgn.m_pos + Vector2d(get_tile_width()*m_coord.x,get_tile_height()*m_coord.y),
												 get_tile_size()),
												 Color(32, 32, 128), Color(32, 32, 128),
												 Color(32, 32, 128), Color(32, 32, 128));
			}
		);
	}
	
	void neighbor_renderer::draw(const logic_tile_buf::bufi *m_bufi, const map_metrics::region &m_rgn)
	{
		bool b_recompute = (m_bufi != m_prev_bufi)     ||
						   (m_rgn.m_coords != m_prev_coords) ||
						   (m_rgn.m_extent != m_prev_extent);
		
		m_prev_bufi   = m_bufi;
		m_prev_coords = m_rgn.m_coords;
		m_prev_extent = m_rgn.m_extent;
		
		if (b_recompute)
		{
			std::vector<phoenix::Rectangle> m_all_rects;
			m_all_rects.reserve(m_bufi->get_extent().size() * 8);
			
			logic_tile_buf::bufw_const m_bufw(m_bufi, m_rgn.m_coords, m_rgn.m_extent);
			m_bufw.each_in
			(
				coord(),
				m_bufw.get_extent(),
				[&] (coord m_coord, uint8_t bits)
				{
					m_rects.clear();
					get_pattern_rects(m_coord, bits);
					draw_rects(m_rgn.m_pos);
					std::copy(m_rects.begin(), m_rects.end(), std::back_inserter(m_all_rects));
				}
			);
			m_all_rects.swap(m_rects);
		}
		else
			draw_rects(m_rgn.m_pos);
	}
	
	video_buf_renderer::cell::cell(video_buf_renderer *m_owner):
		m_owner(m_owner),
		m_img(nullptr),
		i_tile(-1)
	{}
	
	video_buf_renderer::cell::cell(const cell &m):
		m_owner(m.m_owner),
		m_img(nullptr),
		i_tile(-1)
	{}
	
	video_buf_renderer::cell &video_buf_renderer::cell::operator =(const cell &m)
	{
		m_owner = m.m_owner;
		m_img = nullptr;
		i_tile = -1;
		
		return *this;
	}
	
	inline void video_buf_renderer::cell::update(tile_index_type i_tile, coord m_pos)
	{
		if (m_img)
		{
			m_img->set_position(m_pos);
			if (this->i_tile != i_tile)
			{
				m_img->set_tile(i_tile,
								m_owner->m_img_factory,
								atlas::factory_switch::change_config);
				this->i_tile = i_tile;
			}
		}
		else
		{
			m_img = m_owner->m_img_factory.create_rect(m_pos, i_tile);
			this->i_tile = i_tile;
		}
		
		if (!m_img->is_valid() && i_tile >= 0)
		{
			m_img->set_tile(m_owner->i_missing,
							m_owner->m_sys_img_factory,
							atlas::factory_switch::retain_config);
		}
		
		m_img->update();
	}

	video_buf_renderer::video_buf_renderer(atlas::handle_type tileset_id, const map_metrics &m_metrics, tile_index_type i_missing):
		map_renderer_base(m_metrics),
		m_img_factory(tileset_id, atlas::image_config(atlas::image_config::simple, get_tile_size(), get_depth())),
		m_sys_img_factory(builtin_tileset::instance()->get_tileset_handle()),
		i_missing(i_missing)
	{
		m_sys_img_factory.set_out_extent(get_tile_size());
		m_sys_img_factory.get_config().depth = get_depth();
		
		if (get_clip())
		{
			    m_img_factory.clip(true);
			m_sys_img_factory.clip(true);
			
			coord  m_clip_coords = get_clip_coords();
			extent m_clip_extent = get_clip_extent();
			
			    m_img_factory.set_clip_rect(m_clip_coords, m_clip_extent);
			m_sys_img_factory.set_clip_rect(m_clip_coords, m_clip_extent);
		}
		else
		{
			    m_img_factory.clip(false);
			m_sys_img_factory.clip(false);
		}
	}

	inline void video_buf_renderer::update_tiles(const video_tile_buf::bufi *m_bufi, const Vector2d &m_pos, coord m_coords)
	{
		video_tile_buf::bufw_const m_bufw(m_bufi, m_coords, m_extent);
	
		size_t i = 0;
		auto update_fn = [&] (coord m_coord, tile_index_type i_tile)
		{
			if (i >= m_cells.size())
				m_cells.emplace_back(this);
			
			m_cells[i].update
			(
				i_tile,
				coord
				(
					m_pos.getX()+get_tile_width()*float(m_coord.x),
					m_pos.getY()+get_tile_height()*float(m_coord.y)
				)
			);
			
			++i;
		};
		
		try
		{
			m_cells.reserve(m_extent.size());
			m_bufw.each_in(coord(), m_extent, update_fn);
			m_cells.erase(m_cells.begin() + i, m_cells.end());
		}
		catch (UTD::exception &e)
		{
			// throw the exception back up if it is not
			// a window problem, a bad window just means
			// that there is nothing to draw.
			if (e.id() != EXCEPTION_INVALID_BUF2_WINDOW_ID)
				throw e;
		}
	}
	
	void video_buf_renderer::draw(const video_tile_buf::bufi *m_bufi, const map_metrics::region &m_rgn)
	{
		extent m_tile_size = get_tile_size();
		if (m_tile_size != m_img_factory.get_out_extent())
			m_img_factory.set_out_extent(m_tile_size);
		
		this->m_extent = m_rgn.m_extent;
		update_tiles(m_bufi, m_rgn.m_pos, m_rgn.m_coords);
	}

	map_compositor::layer::layer(handle_type id):
		id(id),
		b_show(true)
	{}
	
	map_compositor::layer::~layer() = default;
	
	map_compositor::video_layer::video_layer(handle_type id, const video_tile_buf::bufi *m_bufi,
	                                         atlas::handle_type tileset_id,
											 const map_metrics &m_metrics, tile_index_type i_missing):
		layer(id),
		m_renderer(tileset_id, m_metrics, i_missing),
		m_bufi(m_bufi)
	{}
	
	void map_compositor::video_layer::draw(const map_metrics::region &m_rgn)
	{
		if (b_show) m_renderer.draw(m_bufi, m_rgn);
	}
	
	handle_recycler<map_compositor::layer::handle_type> map_compositor::m_handle_alloc;
	
	map_compositor::map_compositor(const map_metrics &m_metrics):
		m_offset(),
		m_metrics(m_metrics)
	{}
	
	map_compositor::~map_compositor()
	{
		for (layer *l: m_layers) delete l;
	}
	
	map_compositor::layer *map_compositor::get_layer(layer::handle_type i)
	{
		auto it = m_layer_map.find(i);
		return it != m_layer_map.end() ?
			   it->value               :
			   nullptr;
	}
	
	const map_compositor::layer *map_compositor::get_layer(layer::handle_type i) const
	{
		auto it = m_layer_map.find(i);
		return it != m_layer_map.end() ?
			   it->value               :
			   nullptr;
	}
	
	inline map_compositor::layer::handle_type map_compositor::register_layer(layer *m_layer, bool b_at_back)
	{
		layer::handle_type i = m_layer->get_id();
		auto it = m_layer_map.find(i);
		if (it != m_layer_map.end())
			return 0;
		
		m_layer_map.insert(layer_map::entry(i, m_layer));
		if (b_at_back)
			m_layers.push_front(m_layer);
		else
			m_layers.push_back(m_layer);
		
		return i;
	}
	
	map_compositor::layer::handle_type
	map_compositor::register_video_layer(const video_tile_buf::bufi *m_bufi, atlas::handle_type tileset_id, tile_index_type i_missing, bool b_at_back)
	{
		layer *m_layer = new video_layer(m_handle_alloc.get(), m_bufi, tileset_id, m_metrics, i_missing);
		
		layer::handle_type id = register_layer(m_layer, b_at_back);
		if (id == 0) delete m_layer;
		
		return id;
	}
	
	map_compositor::layer::handle_type
	map_compositor::register_neighbor_layer(const logic_tile_buf::bufi *m_bufi, GraphicsFactory2d &m_graphicsfactory, bool b_at_back)
	{
		layer *m_layer = new logic_layer<neighbor_renderer>(m_handle_alloc.get(), m_bufi, m_graphicsfactory, m_metrics);
		
		layer::handle_type id = register_layer(m_layer, b_at_back);
		if (id == 0) delete m_layer;
		
		return id;
	}
	
	map_compositor::layer::handle_type
	map_compositor::register_state_layer(const logic_tile_buf::bufi *m_bufi, GraphicsFactory2d &m_graphicsfactory, bool b_at_back)
	{
		layer *m_layer = new logic_layer<state_renderer>(m_handle_alloc.get(), m_bufi, m_graphicsfactory, m_metrics);
		
		layer::handle_type id = register_layer(m_layer, b_at_back);
		if (id == 0) delete m_layer;
		
		return id;
	}

	bool map_compositor::send_to_back(layer::handle_type i)
	{
		layer *m_layer = get_layer(i);
		if (m_layer)
		{
			m_layers.remove(m_layer);
			m_layers.push_front(m_layer);
		}
		return m_layer;
	}
	
	bool map_compositor::send_to_front(layer::handle_type i)
	{
		layer *m_layer = get_layer(i);
		if (m_layer)
		{
			m_layers.remove(m_layer);
			m_layers.push_back(m_layer);
		}
		return m_layer;
	}
	
	bool map_compositor::insert_in_front_of(layer::handle_type a, layer::handle_type b)
	{
		layer *la = get_layer(a), *lb = get_layer(b);

		bool b_success = la && lb && (la != lb);
		if (b_success)
		{
			b_success = false;
			m_layers.remove(la);
			for (auto it = m_layers.begin(); it != m_layers.end(); ++it)
			{
				if (*it == lb)
				{
					m_layers.insert(++it, la);
					b_success = true;
					break;
				}
			}
		}
		
		return b_success;
	}
	
	bool map_compositor::insert_behind(layer::handle_type a, layer::handle_type b)
	{
		layer *la = get_layer(a), *lb = get_layer(b);

		bool b_success = la && lb && (la != lb);
		if (b_success)
		{
			b_success = false;
			m_layers.remove(la);
			for (auto it = m_layers.begin(); it != m_layers.end(); ++it)
			{
				if (*it == lb)
				{
					m_layers.insert(it, la);
					b_success = true;
					break;
				}
			}
		}
		
		return b_success;
	}

	bool map_compositor::unregister_layer(layer::handle_type i)
	{
		auto it = m_layer_map.find(i);
		if (it != m_layer_map.end())
		{
			m_layers.remove(it->value);
			m_layer_map.erase(it);
			return true;
		}
		else
			return false;
	}
	
	bool map_compositor::unregister_layer(layer *m_layer)
	{
		for (auto it = m_layers.begin(); it != m_layers.end(); ++it)
		{
			if (*it == m_layer)
			{
				m_layer_map.erase(m_layer->get_id());
				m_layers.erase(it);
				return true;
			}
		}
		return false;
	}
	
	void map_compositor::draw()
	{
		map_metrics::region m_rgn;
		if (m_metrics.compute_map_region(m_offset, m_rgn))
		{
			for (layer *l: m_layers) l->draw(m_rgn);
		}
	}
	
	pattern_renderer::pattern_renderer(const Vector2d &m_tile_size, float depth):
		map_renderer_base(map_metrics(m_tile_size, depth)),
		m_sys_img_factory(builtin_tileset::instance()->get_tileset_handle())
	{
		m_sys_img_factory.set_out_extent(get_tile_size());
		m_sys_img_factory.get_config().depth = get_depth();
		
		if (get_clip())
		{
			m_sys_img_factory.clip(true);
			m_sys_img_factory.set_clip_rect(get_clip_coords(), get_clip_extent());
		}
		else
			m_sys_img_factory.clip(false);
		
		for (size_type i = 0; i < nbi_count; ++i)
			m_tile_imgs[i] = m_sys_img_factory.create_rect(coord(), UTS_EMPTY);
		
		m_center_img = m_sys_img_factory.create_rect(coord(), UTS_PATTERNEDIT_CENTER_TILE);
	}
	
	pattern_renderer::~pattern_renderer()
	{
		for (size_type i = 0; i < nbi_count; ++i)
			atlas::image_factory::destroy(m_tile_imgs[i]);
	}
	
	template <size_type i>
	void pattern_renderer::draw_tile_impl(const Vector2d &m_pos, char dat)
	{
		switch (dat)
		{
		case pattern_part::tile_mark:
			m_tile_imgs[i]->set_tile(UTS_PATTERNEDIT_SOLID_TILE);
			break;
		case pattern_part::unknown_mark:
			m_tile_imgs[i]->set_tile(UTS_PATTERNEDIT_MAYBE_TILE);
			break;
		case pattern_part::empty_mark:
			m_tile_imgs[i]->set_tile(UTS_PATTERNEDIT_EMPTY_TILE);
			break;
		};
		
		m_tile_imgs[i]->set_position(m_pos);
		m_tile_imgs[i]->update();
	}
	
	static constexpr float __offset_nbc_x[nbi_count*2] =
	{
		g_neighbor_coords[0].x+1,
		g_neighbor_coords[1].x+1,
		g_neighbor_coords[2].x+1,
		g_neighbor_coords[3].x+1,
		g_neighbor_coords[4].x+1,
		g_neighbor_coords[5].x+1,
		g_neighbor_coords[6].x+1,
		g_neighbor_coords[7].x+1
	};
	
	static constexpr float __offset_nbc_y[nbi_count*2] =
	{
		g_neighbor_coords[0].y+1,
		g_neighbor_coords[1].y+1,
		g_neighbor_coords[2].y+1,
		g_neighbor_coords[3].y+1,
		g_neighbor_coords[4].y+1,
		g_neighbor_coords[5].y+1,
		g_neighbor_coords[6].y+1,
		g_neighbor_coords[7].y+1
	};
	
	template <size_type i>
	inline void pattern_renderer::draw_tile(const pattern_part &m_part, const Vector2d &m_pos)
	{
		draw_tile_impl<i>
		(
			m_pos + Vector2d(__offset_nbc_x[i] * get_tile_width(),
							 __offset_nbc_y[i] * get_tile_height()),
			m_part.m_buf[i]
		);
	}
	
	void pattern_renderer::draw(const pattern_part &m_part, const Vector2d &m_pos)
	{
		draw_tile<nbi_NE>(m_part, m_pos);
		draw_tile<nbi_N>(m_part, m_pos);
		draw_tile<nbi_NW>(m_part, m_pos);
		draw_tile<nbi_W>(m_part, m_pos);
		draw_tile<nbi_E>(m_part, m_pos);
		draw_tile<nbi_SW>(m_part, m_pos);
		draw_tile<nbi_S>(m_part, m_pos);
		draw_tile<nbi_SE>(m_part, m_pos);
		
		m_center_img->set_position(m_pos + get_tile_size());
		m_center_img->update();
	}
}
