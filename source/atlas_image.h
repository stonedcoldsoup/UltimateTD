#ifndef UTD_ATLAS_IMAGE_H
#define UTD_ATLAS_IMAGE_H

#include "common.h"
#include "atlas.h"

// TODO: add clipping
//       clipping is done per image factory, clipping rect is retrieved from
//       image factory via m_factory reference.  clipping is done via direct
//       modification to geometry in rect_image, or in the case that the
//       image is not rotated.  If it is, PhoenixCore's geometry clipping 
//       is used.  for axis aligned cases, obviously we can make a lot of
//       simplifying assumptions.  Tilemaps and such can be clipped extremely
//       fast as a result, compared to rotating sprites.

namespace UTD
{
	class atlas::image
	{
	private:
		atlas::image_factory *m_factory;
		BatchGeometryPtr m_geom;
		
		coord m_pos;
		
		extent m_extent;
		image_config m_config;
		
		bool b_dirty, b_geom_attrib_dirty;
		
		image(atlas::image_factory &m_factory, BatchGeometryPtr m_geom, coord m_pos, extent m_extent, const image_config &m_config);
		~image();
		
		friend class image_factory;
		friend class util::block_allocator<image, 1024>;
	public:
		inline uint8_t flags() const {return m_config.i_flags;}
		
		inline uint8_t set_flags(uint8_t i_flags = image_config::simple) {b_dirty = true; return m_config.i_flags = i_flags;}
		
		inline uint8_t append_flags(uint8_t i_flags) {b_dirty = true; return m_config.i_flags |= i_flags;}
		inline uint8_t remove_flags(uint8_t i_flags) {b_dirty = true; return m_config.i_flags &= ~i_flags;}
		
		inline uint8_t toggle_flags(uint8_t i_flags = image_config::all) {b_dirty = true; return m_config.i_flags ^= i_flags;}
		
		inline extent get_extent() const {return m_extent;}
		
		inline void set_position(coord m_pos) {this->m_pos = m_pos; b_dirty = true;}
		inline coord get_position() const     {return m_pos;}
		
		inline void set_scale(const Vector2d &s) {m_config.m_scl = s; b_dirty = true;}
		inline const Vector2d &get_scale() const {return m_config.m_scl;}
		
		inline void set_rotation(math::angle rot) {m_config.m_rot = rot; b_dirty = true;}
		inline math::angle get_rotation() const   {return m_config.m_rot;}
		
		inline void  set_depth(float depth) {m_config.depth = depth; b_geom_attrib_dirty = true;}
		inline float get_depth() const      {return m_config.depth;}
	
		bool set_tile(tile_index_type i_tile);
		bool set_tile(tile_index_type i_tile, atlas::image_factory &m_factory,
		                     factory_switch e_mode = factory_switch::retain_config); // switch factories
		
		inline bool is_valid() const {return m_geom;}
		
		void reset_to_config();
		
		void update();
	};
	
	class atlas::rect_image
	{
	private:
		atlas::image_factory *m_factory;
		BatchGeometryPtr m_geom;
		
		coord m_pos;
		extent m_extent;
		
		float depth;
		
		bool b_dirty;
		bool b_geom_attrib_dirty;
		
		rect_image(atlas::image_factory &m_factory, BatchGeometryPtr m_geom, coord m_pos, extent m_extent, float depth);
		~rect_image();
		
		friend class image_factory;
		friend class util::block_allocator<rect_image, 4096>;
	public:
		inline void set_position(coord m_pos) {this->m_pos = m_pos; b_dirty = true;}
		inline coord get_position() const     {return m_pos;}
		
		inline void  set_depth(float depth) {this->depth = depth; b_geom_attrib_dirty = true;}
		inline float get_depth() const      {return depth;}
		
		bool set_tile(tile_index_type i_tile);
		bool set_tile(tile_index_type i_tile, atlas::image_factory &m_factory,
		                     factory_switch e_mode = factory_switch::retain_config); // switch factories
		
		inline bool is_valid() const {return m_geom;}
		
		void reset_to_config();
	
		void update();
	};

	class atlas::image_factory
	{
	public:
		struct clone_delegate;
		
	private:
		static util::block_allocator<     image, 1024> m_alloc;
		static util::block_allocator<rect_image, 4096> m_alloc_rect;
	
		clone_delegate *m_clone_delegate;
	
		atlas *m_atlas;
		handle_type id;
		handle_info m_info;
		
		std::vector<coord> m_coords;
		extent m_extent;
		extent m_out_extent;
		
		coord  m_clip_coords;
		extent m_clip_extent;
		
		image_config m_config;
		bool         b_clip;
		
		BatchGeometryPtr gen_geometry(tile_index_type i_tile);
		BatchGeometryPtr gen_texcoords(BatchGeometryPtr m_geom, tile_index_type i_tile) const;
		void clip_geometry(BatchGeometryPtr m_geom) const;
		
		inline void init();
		
		friend class image;
		friend class rect_image;
	public:
		image_factory(handle_type id, const image_config &m_config = atlas::image_config());
		image_factory(const clone_delegate &m_clone_delegate);
		
		~image_factory();
		
		const clone_delegate &clone() const;
		
		inline void set_clip_rect(coord m_coords, extent m_extent)
		{
			m_clip_coords = m_coords;
			m_clip_extent = m_extent;
		}
		
		inline void set_clip_coords(coord m_coords)  {m_clip_coords = m_coords;}
		inline void set_clip_extent(extent m_extent) {m_clip_extent = m_extent;}
		
		inline coord  get_clip_coords() const {return m_clip_coords;}
		inline extent get_clip_extent() const {return m_clip_extent;}
		
		inline void clip(bool b_enable) {b_clip = b_enable;}
		inline bool clip() const        {return b_clip;}
		
		inline handle_type get_handle() const {return id;}
		
		inline extent      get_extent() const {return m_extent;}
		inline size_type   tile_count() const {return m_coords.size();}
		
		inline image_config  get_config() const {return m_config;}
		inline image_config &get_config()       {return m_config;}

		inline void set_out_scale(const Vector2d &m_out_scale = Vector2d(1,1))
		{
			m_out_extent.x = m_out_scale.getX()*float(m_extent.x);
			m_out_extent.y = m_out_scale.getY()*float(m_extent.y);
		}
		
		inline void get_out_scale(Vector2d &m_out_scale) const
		{
			m_out_scale.setX(float(m_out_extent.x)/float(m_extent.x));
			m_out_scale.setY(float(m_out_extent.y)/float(m_extent.y));
		}
		
		inline void set_out_extent(extent m_out_extent) {this->m_out_extent = m_out_extent;}
		inline extent get_out_extent() const            {return m_out_extent;}
		
		inline image *create(coord m_pos, tile_index_type i_tile = 0)
		{
			return m_alloc.create(*this, gen_geometry(i_tile), m_pos, m_out_extent, m_config);
		}
		
		inline rect_image *create_rect(coord m_pos, tile_index_type i_tile = 0)
		{
			return m_alloc_rect.create(*this, gen_geometry(i_tile), m_pos, m_out_extent, m_config.depth);
		}
		
		static inline void destroy(image *m_image)
		{
			m_alloc.destroy(m_image);
		}
		
		static inline void destroy(rect_image *m_rect_image)
		{
			m_alloc_rect.destroy(m_rect_image);
		}
	};
}

#endif
