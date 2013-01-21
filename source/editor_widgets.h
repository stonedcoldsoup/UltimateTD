#ifndef UTD_EDITOR_WIDGETS_H
#define UTD_EDITOR_WIDGETS_H

#include "common.h"
#include "map2.h"
#include "map_render.h"
#include "widgets.h"

namespace UTD
{
	class pattern_part_edit_widget_impl:
		public tiled_widget_impl
	{
	private:
		pattern_part &m_pat;
		Vector2d m_pos;
		const Vector2d m_tile_size;
		const Vector2d m_offset, m_extent;
		
		bool b_lmb_down;
		
		pattern_renderer m_renderer;
	public:
		pattern_part_edit_widget_impl(pattern_part &m_pat, const Vector2d &m_pos, const Vector2d &m_tile_size):
			m_pat(m_pat), m_pos(m_pos),
			m_tile_size(m_tile_size),
			m_offset(0,0), m_extent(m_tile_size * 3.0f),
			b_lmb_down(false),
			m_renderer(m_tile_size, 1.0f)
		{
		}
		
		virtual inline void on_focus(bool b_focus)                                  {}
		virtual inline void on_lmb(bool b_down, coord m_coords)
		{
			if (!b_down && b_lmb_down)
			{
				const size_type i_buf = get_neighbor_coord_nbi(m_coords + coord(-1,-1));
				switch (m_pat.m_buf[i_buf])
				{
				case pattern_part::tile_mark:
					m_pat.m_buf[i_buf] = pattern_part::unknown_mark;
					break;
				case pattern_part::unknown_mark:
					m_pat.m_buf[i_buf] = pattern_part::empty_mark;
					break;
				case pattern_part::empty_mark:
					m_pat.m_buf[i_buf] = pattern_part::tile_mark;
					break;
				};
			}
			b_lmb_down = b_down;
		}
		
		/*virtual inline void on_mmb(bool b_down, coord m_coords)                     {}
		virtual inline void on_rmb(bool b_down, coord m_coords)                     {}
		virtual inline void on_move(coord m_coords)								    {}
		virtual inline void on_key(phoenix::Key m_key, bool b_down, coord m_coords) {}
		virtual inline void on_char(char c)                                         {}
		virtual inline void on_scroll(int_type d)									{}*/
		
		virtual inline const Vector2d &get_position()     const {return m_pos;}
		virtual inline const Vector2d &get_offset()       const {return m_offset;}
		virtual inline const Vector2d &get_extent()       const {return m_extent;}
		virtual inline const Vector2d &get_tile_size()    const {return m_tile_size;}
		virtual inline         extent  get_tiles_extent() const {return extent(3,3);}
		
		virtual inline void update() {}
		virtual inline void draw()   {m_renderer.draw(m_pat, m_pos);}
	};
	
	typedef
		tiled_widget<pattern_part_edit_widget_impl>
		pattern_part_edit_widget;
}

#endif
