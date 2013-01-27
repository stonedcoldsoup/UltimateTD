#ifndef UTD_MAP_RENDER_H
#define UTD_MAP_RENDER_H

#include "common.h"
#include "map2.h"
#include "atlas.h"
#include "atlas_image.h"
#include "tile_builtin_sets.h"
#include "handle_recycler.h"
#include "depth_layer.h"

namespace UTD
{
	struct metrics
	{
		coord  m_coords;
		extent m_extent;
		bool   b_clip;
		
		metrics():
			b_clip(true)
		{}
		
		metrics(coord  m_coords,
			    extent m_extent):
			m_coords(m_coords),
			m_extent(m_extent),
			b_clip(true)
		{}
		
		metrics(const metrics &m) = default;
		metrics(metrics &&m) = default;
		
		metrics &operator =(const metrics &m) = default;
		metrics &operator =(metrics &&m) = default;
	};

	struct map_metrics: metrics
	{
		Vector2d m_tile_size;
		
		map_metrics(const Vector2d &m_tile_size):
			metrics(),
			m_tile_size(m_tile_size)
		{}
		
		map_metrics(const Vector2d &m_tile_size,
		            coord  m_coords,
					extent m_extent):
			metrics(m_coords, m_extent),
			m_tile_size(m_tile_size)
		{}
		
		map_metrics(const Vector2d &m_tile_size,
					const metrics &m_metrics):
			metrics(m_metrics),
			m_tile_size(m_tile_size)
		{}
		
		map_metrics(const map_metrics &m) = default;
		map_metrics(map_metrics &&m) = default;
		
		map_metrics &operator =(const map_metrics &m) = default;
		map_metrics &operator =(map_metrics &&m) = default;
		
		struct region
		{
			Vector2d m_pos;
			coord    m_coords;
			extent   m_extent;
		};
		
		bool compute_map_region(const Vector2d &m_offset, region &m_rgn);
	};

	class map_renderer2
	{
	private:
		map_metrics m_metrics;
		
		atlas::image_factory m_user_factory;
		atlas::image_factory m_builtin_factory;
		
		struct cell
		{
			map_renderer2 *m_owner;
			atlas::rect_image *m_image;
			
			inline void update(tile_index_type i_tile, coord m_pos, float depth, bool b_sys);
			
			cell(map_renderer2 *m_owner);
			cell(const cell &m);
			inline cell &operator =(const cell &m);
			
			~cell();
		};
		
		std::vector<cell> m_cells;
		size_t __i_update;
		map_metrics::region m_rgn;
		
		tile_index_type i_missing;

		inline void __update_cell(tile_index_type i_tile, coord m_coord, float depth, bool b_sys);
	public:
		void begin_layers(const Vector2d &m_offset, size_type n_layers = 1);
		void end_layers();
		
		inline void push_depth()
		{
			g_depth_layer_stack.push();
		}
		
		inline void pop_depth()
		{
			g_depth_layer_stack.pop();
		}
		
		void draw_background_layer(tile_index_type i_bg = UTS_AUTOTILEEDIT_BG_TILE);
		void draw_video_layer(const video_tile_buf::bufi *m_bufi, tile_index_type i_missing = UTS_DEFAULT_MISSING_TILE);
		void draw_state_layer(const logic_tile_buf::bufi *m_bufi);
		void draw_neighbor_layer(const logic_tile_buf::bufi *m_bufi);
		
		map_renderer2(const map_metrics &m_metrics, atlas::handle_type tileset_id);
		
		const map_metrics &get_metrics() const;
		
		void set_tile_size(const Vector2d &m_tile_size);
		void set_coords(coord m_coords);
		void set_extent(extent m_extent);
		void set_clip(bool b_clip);
	};

	/*class map_renderer_base
	{
	private:
		map_metrics m_metrics;
	public:
		map_renderer_base(const map_metrics &m_metrics);
		virtual ~map_renderer_base();
		
		inline const Vector2d &get_tile_size() const           {return m_metrics.m_tile_size;}
		inline void get_tile_size(float &tw, float &th) const  {tw = m_metrics.m_tile_size.getX(); th = m_metrics.m_tile_size.getY();}
		inline void set_tile_size(const Vector2d &m_tile_size) {m_metrics.m_tile_size = m_tile_size;}
		inline void set_tile_size(float tw, float th)          {m_metrics.m_tile_size = Vector2d(tw, th);}
		
		inline float get_tile_width()  const {return m_metrics.m_tile_size.getX();}
		inline float get_tile_height() const {return m_metrics.m_tile_size.getY();}
		
		inline void set_clip_rect(coord m_coords, extent m_extent)         {m_metrics.m_coords = m_coords; m_metrics.m_extent = m_extent;}
		inline void get_clip_rect(coord &m_coords, extent &m_extent) const {m_coords = m_metrics.m_coords; m_extent = m_metrics.m_extent;}
		
		inline void   set_clip_coords(coord m_coords)  {m_metrics.m_coords = m_coords;}
		inline coord  get_clip_coords() const          {return m_metrics.m_coords;}
		
		inline void   set_clip_extent(extent m_extent) {m_metrics.m_extent = m_extent;}
		inline extent get_clip_extent() const          {return m_metrics.m_extent;}
		
		inline void set_clip(bool b_clip) {m_metrics.b_clip = b_clip;}
		inline bool get_clip() const      {return m_metrics.b_clip;}
		
		inline void  set_depth(depth_layer depth) {m_metrics.depth = depth;}
		inline depth_layer get_depth() const      {return m_metrics.depth;}
	};
	
	class logic_buf_renderer:
		public map_renderer_base
	{
	public:
		logic_buf_renderer(const map_metrics &m_metrics);

		virtual void draw(const logic_tile_buf::bufi *m_bufi, const map_metrics::region &m_rgn) = 0;
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
		
		tile_index_type i_missing;
		
		friend class std::vector<cell>;
	public:
		video_buf_renderer(atlas::handle_type tileset_id, const map_metrics &m_metrics, tile_index_type i_missing = UTS_DEFAULT_MISSING_TILE);

		void draw(const video_tile_buf::bufi *m_bufi, const map_metrics::region &m_rgn);
	};
	
	class state_renderer:
		public logic_buf_renderer
	{
	private:
		GraphicsFactory2d &m_graphicsfactory;

	public:
		state_renderer(GraphicsFactory2d &m_graphicsfactory, const map_metrics &m_metrics):
			logic_buf_renderer(m_metrics),
			m_graphicsfactory(m_graphicsfactory)
		{}
	
		virtual void draw(const logic_tile_buf::bufi *m_bufi, const map_metrics::region &m_rgn);
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
		neighbor_renderer(GraphicsFactory2d &m_graphicsfactory, const map_metrics &m_metrics):
			logic_buf_renderer(m_metrics),
			m_graphicsfactory(m_graphicsfactory),
			m_prev_bufi(nullptr),
			m_prev_coords(-1,-1),
			m_prev_extent(0,0)
		{}
	
		virtual void draw(const logic_tile_buf::bufi *m_bufi, const map_metrics::region &m_rgn);
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
			
			friend class map_compositor;
		public:
			layer(handle_type id);
			virtual ~layer();
			
			virtual void draw(const map_metrics::region &m_rgn) = 0;
			
			inline void show(bool b_show = true) {this->b_show = b_show;}
			inline void hide() 					 {show(false);}
			inline bool visible() const 		 {return b_show;}
			
			inline handle_type get_id() const  {return id;}
		};
	private:
		template <typename rendererT>
		class logic_layer:
			public layer
		{
		protected:
			rendererT m_renderer;
			const logic_tile_buf::bufi *m_bufi;
			
			friend class map_compositor;
		public:
			template <typename... argT>
			logic_layer(handle_type id, const logic_tile_buf::bufi *m_bufi, argT&&... argV):
				layer(id),
				m_renderer(std::forward<argT>(argV)...),
				m_bufi(m_bufi)
			{}
			
			virtual void draw(const map_metrics::region &m_rgn)
			{
				if (b_show) m_renderer.draw(m_bufi, m_rgn);
			}
		};
		
		class video_layer:
			public layer
		{
		protected:
			video_buf_renderer m_renderer;
			const video_tile_buf::bufi *m_bufi;
			
			friend class map_compositor;
		public:
			video_layer(handle_type id, const video_tile_buf::bufi *m_bufi,
			            atlas::handle_type tileset_id, const map_metrics &m_metrics, tile_index_type i_missing);
			
			virtual void draw(const map_metrics::region &m_rgn);
		};
		
		typedef
			util::hash_map<util::default_hash_no_replace<layer::handle_type, layer *, util::hash_entry_ptr_no_delete>, 11>
			layer_map;
	
		layer_map m_layer_map;
		std::list<layer *> m_layers;
		
		Vector2d    m_offset;
		map_metrics m_metrics;
		static handle_recycler<layer::handle_type> m_handle_alloc;
		
		inline layer::handle_type register_layer(layer *m_layer, bool b_at_back);
	public:
		map_compositor(const map_metrics &m_metrics);
		~map_compositor();
		
		inline void set_offset(const Vector2d &m_offset) {this->m_offset = m_offset;}
		inline const Vector2d &get_offset() const        {return m_offset;}
		
		inline       map_metrics &get_metrics()       {return m_metrics;}
		inline const map_metrics &get_metrics() const {return m_metrics;}
	
		layer::handle_type register_video_layer(const video_tile_buf::bufi *m_bufi, atlas::handle_type tileset_id, tile_index_type i_missing = UTS_DEFAULT_MISSING_TILE, bool b_at_back = false);
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
		atlas::image_factory m_sys_img_factory;
		atlas::rect_image *m_tile_imgs[nbi_count];
		atlas::rect_image *m_center_img;
		
		Vector2d m_prev_pos;

		template <size_type i>
		inline void draw_tile_impl(const Vector2d &m_pos, char dat);
		
		template <size_type i>
		inline void draw_tile(const pattern_part &m_part, const Vector2d &m_pos);
	public:
		pattern_renderer(const Vector2d &m_tile_size, depth_layer depth = {0,0,0});
		virtual ~pattern_renderer();
		
		void draw(const pattern_part &m_part, const Vector2d &m_pos);
	};*/
}

#endif