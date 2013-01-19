#ifndef UTD_TILE_EDITOR_H
#define UTD_TILE_EDITOR_H

#include "common.h"
#include "map2.h"

namespace UTD
{
	class tiled_widget_impl
	{
	public:
		virtual ~tiled_widget_impl();
		
		virtual inline void on_focus(bool b_focus)                                  {}
		virtual inline void on_lmb(bool b_down, coord m_coords)                     {}
		virtual inline void on_mmb(bool b_down, coord m_coords)                     {}
		virtual inline void on_rmb(bool b_down, coord m_coords)                     {}
		virtual inline void on_move(coord m_coords)								    {}
		virtual inline void on_key(phoenix::Key m_key, bool b_down, coord m_coords) {}
		virtual inline void on_char(char c)                                         {}
		virtual inline void on_scroll(int_type d)									{}
		
		virtual inline const Vector2d &get_position()     const {static const Vector2d _dummy = Vector2d(); return _dummy;}
		virtual inline const Vector2d &get_offset()       const {static const Vector2d _dummy = Vector2d(); return _dummy;}
		virtual inline const Vector2d &get_extent()       const {static const Vector2d _dummy = Vector2d(); return _dummy;}
		virtual inline const Vector2d &get_tile_size()    const {static const Vector2d _dummy = Vector2d(); return _dummy;}
		virtual inline          float  get_tile_width()   const {return get_tile_size().getX();}
		virtual inline          float  get_tile_height()  const {return get_tile_size().getY();}
		virtual inline         extent  get_tiles_extent() const {return extent();}
		
		virtual inline void update() {}
		virtual inline void draw()   {}
	};
	
	class widget_base
	{
	protected:
		inline bool get_corrected_pos(const Vector2d &m_in, Vector2d &m_out)
		{
			m_out = m_in - get_position();
			
			if (m_out.getX() < 0.0f || m_out.getY() < 0.0f ||
			    m_out.getX() >= get_extent().getX() ||
				m_out.getY() >= get_extent().getY())
				return false;
			
			m_out += get_offset();
			return true;
		}
		
		friend class widget_manager;
	public:
		virtual ~widget_base();
		
		virtual void on_focus(bool b_focus);
		virtual void on_lmb(bool b_down, const Vector2d &m_pos);
		virtual void on_mmb(bool b_down, const Vector2d &m_pos);
		virtual void on_rmb(bool b_down, const Vector2d &m_pos);
		virtual void on_move(const Vector2d &m_pos);
		virtual void on_key(phoenix::Key m_key, bool b_down, const Vector2d &m_pos);
		virtual void on_char(char c);
		virtual void on_scroll(int_type d);
		
		virtual const Vector2d &get_position() const = 0;
		virtual const Vector2d &get_offset()   const = 0;
		virtual const Vector2d &get_extent()   const = 0;
		
		virtual void update() = 0;
		virtual void draw()   = 0;
	};
	
	class widget_dummy:
		public widget_base
	{
	public:
		virtual const Vector2d &get_position() const {static const Vector2d _dummy = Vector2d(); return _dummy;}
		virtual const Vector2d &get_offset()   const {static const Vector2d _dummy = Vector2d(); return _dummy;}
		virtual const Vector2d &get_extent()   const {static const Vector2d _dummy = Vector2d(); return _dummy;}
		
		virtual void update() {}
		virtual void draw()   {}
	};
	
	template <typename tiled_widget_implT>
	class tiled_widget:
		public widget_base
	{
	public:
		typedef tiled_widget_implT tiled_widget_type;
	private:
		tiled_widget_type m_widget;
		coord m_prev_coords;
		
		inline coord to_tile_coords(const Vector2d &m_pos)
		{
			return coord(m_pos.getX() / get_tile_width(), m_pos.getY() / get_tile_height());
		}
		
		inline void to_mouse_coords(coord m_coord, Vector2d &m_pos)
		{
			m_pos.setX(m_coord.x * int_type(get_tile_width()));
			m_pos.setY(m_coord.y * int_type(get_tile_height()));
		}
	public:
		template <typename... T>
		tiled_widget(T&&... v):
			m_widget(std::forward<T>(v)...),
			m_prev_coords(0,0)
		{}
		
		virtual void on_focus(bool b_focus)
		{
			m_widget.on_focus(b_focus);
		}
		
		virtual void on_lmb(bool b_down, const Vector2d &m_pos)
		{
			m_widget.on_lmb(b_down, to_tile_coords(m_pos));
		}
		
		virtual void on_mmb(bool b_down, const Vector2d &m_pos)
		{
			m_widget.on_mmb(b_down, to_tile_coords(m_pos));
		}
		
		virtual void on_rmb(bool b_down, const Vector2d &m_pos)
		{
			m_widget.on_rmb(b_down, to_tile_coords(m_pos));
		}
		
		virtual void on_move(const Vector2d &m_pos)
		{
			coord m_coords = to_tile_coords(m_pos);
			if (m_coords != m_prev_coords)
			{
				m_widget.on_move(m_coords);
				m_prev_coords = m_coords;
			}
		}
		
		virtual void on_key(phoenix::Key m_key, bool b_down, const Vector2d &m_pos)
		{
			m_widget.on_key(m_key, b_down, to_tile_coords(m_pos));
		}
		
		virtual void on_char(char c)
		{
			m_widget.on_char(c);
		}
		
		virtual void on_scroll(int_type d)
		{
			m_widget.on_scroll(d);
		}
		
		virtual         const Vector2d &get_position()     const {return m_widget.get_position();}
		virtual         const Vector2d &get_offset()       const {return m_widget.get_offset();}
		virtual         const Vector2d &get_extent()       const {return m_widget.get_extent();}
		virtual inline  const Vector2d &get_tile_size()    const {return m_widget.get_tile_size();}
		virtual inline           float  get_tile_width()   const {return m_widget.get_tile_width();}
		virtual inline           float  get_tile_height()  const {return m_widget.get_tile_height();}
		virtual inline          extent  get_tiles_extent() const {return m_widget.get_tiles_extent();}
		
		virtual void update() {m_widget.update();}
		virtual void draw()   {m_widget.draw();}
	};
	
	class widget_manager
	{
	private:
		std::list<widget_base *> m_widgets;
		
		boost::signals2::connection event_connection;
		
		Vector2d m_mouse_pos;
		int_type scroll_pos;
		
		inline widget_base *get_target_widget(Vector2d &m_corrected_mouse_pos, bool b_grab_focus = false);
		
		void on_event(const WindowEvent& e);
	public:
		widget_manager();
		~widget_manager();
		
		void   register_widget(widget_base *m_widget, bool b_grab_focus = true);
		void unregister_widget(widget_base *m_widget);
		
		void update();
		void draw();
	};
}

#endif
