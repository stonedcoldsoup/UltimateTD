#ifndef UTD_MAP_RENDER_H
#define UTD_MAP_RENDER_H

#include "common.h"
#include "map2.h"
#include "atlas.h"
#include "atlas_image.h"
#include "tile_builtin_sets.h"
#include "handle_recycler.h"

namespace UTD
{
	class map_renderer_base
	{
	private:
		float tw, th;
	public:
		map_renderer_base(const Vector2d &m_tile_size);
		virtual ~map_renderer_base();
		
		inline Vector2d get_tile_size() const                  {return Vector2d(tw, th);}
		inline void set_tile_size(const Vector2d &m_tile_size) {tw = m_tile_size.getX(); th = m_tile_size.getY();}
		inline void set_tile_size(float tw, float th)          {this->tw = tw; this->th = th;}
		
		inline float get_tile_width()  const {return tw;}
		inline float get_tile_height() const {return th;}
	};
	
	class logic_buf_renderer:
		public map_renderer_base
	{
	public:
		logic_buf_renderer(const Vector2d &m_tile_size);

		virtual void draw(const logic_tile_buf::bufi *m_bufi, const Vector2d &m_pos, coord m_coords, extent m_extent) = 0;
	};
	
	class video_buf_renderer:
		public map_renderer_base
	{
	private:
		struct cell
		{
			video_buf_renderer *m_owner;
			atlas::rect_image *m_img;
			tile_index_type i_tile;
			
			cell(video_buf_renderer *m_owner);
			cell(const cell &m);
			
			cell &operator =(const cell &m);
			
			inline ~cell()
			{
				if (m_img) atlas::image_factory::destroy(m_img);
			}
			
			inline void update(tile_index_type i_tile, coord m_pos);
		};

		inline void update_tiles(const video_tile_buf::bufi *m_bufi, const Vector2d &m_pos, coord m_coords);
		
		atlas::image_factory m_img_factory;
		atlas::image_factory m_sys_img_factory;
		
		std::vector<cell> m_cells;
		extent m_extent;
		
		friend class std::vector<cell>;
	public:
		video_buf_renderer(atlas::handle_type tileset_id, const Vector2d &m_tile_size);
		
		inline       atlas::image_factory &get_image_factory()       {return m_img_factory;}
		inline const atlas::image_factory &get_image_factory() const {return m_img_factory;}
		
		inline       atlas::image_factory &get_system_image_factory()       {return m_sys_img_factory;}
		inline const atlas::image_factory &get_system_image_factory() const {return m_sys_img_factory;}

		void draw(const video_tile_buf::bufi *m_bufi, const Vector2d &m_pos, coord m_coords, extent m_extent);
	};
	
	class state_renderer:
		public logic_buf_renderer
	{
	private:
		GraphicsFactory2d &m_graphicsfactory;

	public:
		state_renderer(GraphicsFactory2d &m_graphicsfactory, const Vector2d &m_tile_size):
			logic_buf_renderer(m_tile_size),
			m_graphicsfactory(m_graphicsfactory)
		{}
	
		virtual void draw(const logic_tile_buf::bufi *m_bufi, const Vector2d &m_pos, coord m_coords, extent m_extent);
	};
	
	class neighbor_renderer:
		public logic_buf_renderer
	{
	private:
		GraphicsFactory2d &m_graphicsfactory;
		const logic_tile_buf::bufi *m_prev_bufi;
		coord  m_prev_coords;
		extent m_prev_extent;
		std::vector<phoenix::Rectangle> m_rects;
		
		inline void get_pattern_rects(coord m_coords, uint8_t bits);
		inline void draw_rects(const Vector2d &m_pos);
	public:
		neighbor_renderer(GraphicsFactory2d &m_graphicsfactory, const Vector2d &m_tile_size):
			logic_buf_renderer(m_tile_size),
			m_graphicsfactory(m_graphicsfactory),
			m_prev_bufi(nullptr),
			m_prev_coords(-1,-1),
			m_prev_extent(0,0)
		{}
	
		virtual void draw(const logic_tile_buf::bufi *m_bufi, const Vector2d &m_pos, coord m_coords, extent m_extent);
	};
	
	class map_compositor
	{
	public:
		class layer
		{
		public:
			typedef
				logical_type
				handle_type;
		protected:
			handle_type id;
			bool        b_show;
			
			virtual void set_clip_rect(coord m_coords, extent m_extent) = 0;
			
			friend class map_compositor;
		public:
			layer(handle_type id);
			virtual ~layer();
			
			virtual void draw(const Vector2d &m_pos, coord m_coords, extent m_extent) = 0;
			
			inline void show(bool b_show = true) {this->b_show = b_show;}
			inline void hide() 					 {show(false);}
			inline bool visible() const 		 {return b_show;}
			
			inline handle_type get_id() const  {return id;}
		};
		
		struct window_info
		{
			Vector2d m_pos, m_offset, m_extent;
			
			inline void compute_map_region(const Vector2d &m_tile_size, Vector2d &m_dpos, coord &m_dcoords, extent &m_dextent);
		};
	private:
		template <typename rendererT>
		class logic_layer:
			public layer
		{
		protected:
			rendererT m_renderer;
			const logic_tile_buf::bufi *m_bufi;
			
			virtual void set_clip_rect(coord m_coords, extent m_extent) {}
			
			friend class map_compositor;
		public:
			template <typename... argT>
			logic_layer(handle_type id, const logic_tile_buf::bufi *m_bufi, argT&&... argV):
				layer(id),
				m_renderer(std::forward<argT>(argV)...),
				m_bufi(m_bufi)
			{}
			
			virtual void draw(const Vector2d &m_pos, coord m_coords, extent m_extent)
			{
				if (b_show) m_renderer.draw(m_bufi, m_pos, m_coords, m_extent);
			}
		};
		
		class video_layer:
			public layer
		{
		protected:
			video_buf_renderer m_renderer;
			const video_tile_buf::bufi *m_bufi;
			
			virtual void set_clip_rect(coord m_coords, extent m_extent)
			{
				m_renderer.get_image_factory().clip(true);
				m_renderer.get_image_factory().set_clip_rect(m_coords, m_extent);
				
				m_renderer.get_system_image_factory().clip(true);
				m_renderer.get_system_image_factory().set_clip_rect(m_coords, m_extent);
			}
			
			friend class map_compositor;
		public:
			video_layer(handle_type id, const video_tile_buf::bufi *m_bufi,
			            atlas::handle_type tileset_id, const Vector2d &m_tile_size);
					
			inline       atlas::image_factory &get_image_factory()       {return m_renderer.get_image_factory();}
			inline const atlas::image_factory &get_image_factory() const {return m_renderer.get_image_factory();}
			
			virtual void draw(const Vector2d &m_pos, coord m_coords, extent m_extent);
		};
		
		typedef
			util::hash_map<util::default_hash_no_replace<layer::handle_type, layer *, util::hash_entry_ptr_no_delete>, 11>
			layer_map;
	
		layer_map m_layer_map;
		std::list<layer *> m_layers;
		
		window_info m_wnd_info;
		Vector2d m_tile_size;
		static handle_recycler<layer::handle_type> m_handle_alloc;
		
		inline layer::handle_type register_layer(layer *m_layer, bool b_at_back);
	public:
		map_compositor(const window_info &m_wnd_info, const Vector2d &m_tile_size);
		~map_compositor();
		
		inline       window_info &get_window_info()       {return m_wnd_info;}
		inline const window_info &get_window_info() const {return m_wnd_info;}
	
		layer::handle_type register_video_layer(const video_tile_buf::bufi *m_bufi, atlas::handle_type tileset_id, bool b_at_back = false);
		layer::handle_type register_neighbor_layer(const logic_tile_buf::bufi *m_bufi, GraphicsFactory2d &m_graphicsfactory, bool b_at_back = false);
		layer::handle_type register_state_layer(const logic_tile_buf::bufi *m_bufi, GraphicsFactory2d &m_graphicsfactory, bool b_at_back = false);
		
		      layer *get_layer(layer::handle_type i);
		const layer *get_layer(layer::handle_type i) const;
	
		bool send_to_back(layer::handle_type i);
		bool send_to_front(layer::handle_type i);
		
		bool insert_in_front_of(layer::handle_type a, layer::handle_type b); // insert a in front of b
		bool insert_behind(layer::handle_type a, layer::handle_type b);      // insert a behind b
	
		bool unregister_layer(layer::handle_type i);
		bool unregister_layer(layer *m_layer);
		
		void draw();
		
		inline void recycle_handles()
		{
			m_handle_alloc.recycle();
		}
	};
	
	// special renderer that does not support windowing.
	// size is always 3x3
	class pattern_renderer:
		public map_renderer_base
	{
	private:
		GraphicsFactory2d &m_graphicsfactory;

		inline void draw_tile(const Vector2d &m_pos, char dat);
	public:
		pattern_renderer(GraphicsFactory2d &m_graphicsfactory, const Vector2d &m_tile_size);
		
		void draw(const pattern_part &m_part, const Vector2d &m_pos);
	};
}

#endif