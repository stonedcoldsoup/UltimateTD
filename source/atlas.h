#ifndef UTD_ATLAS_H
#define UTD_ATLAS_H

#include "common.h"
#include "tile.h"
#include "streamable.h"
#include "handle_name_allocator.h"
#include "fismath.h"
#include <cstdint>

#ifndef UINT16_MAX
#define UINT16_MAX 65535
#endif

namespace UTD
{
	class atlas:
		public util::singleton<atlas>,
		public streamable
	{
	public:
		typedef
			uint16_t
			handle_type;
			
		static constexpr uint16_t region_handle_base = UINT16_MAX/2;
	
		struct region_info:
			public streamable
		{
			extent m_coords; // unsigned coordinates for textures
			extent m_extent;
			extent m_tile_extent;
			size_type n_cols, n_tiles;
			bool b_tiled;
			
			region_info(const region_info &m_info) = default;
			region_info(region_info &&m_info) = default;
			
			region_info &operator =(const region_info &m_info) = default;
			region_info &operator =(region_info &&m_info) = default;
			
			region_info(extent m_coords = extent(),
			            extent m_extent = extent());
			
			region_info(extent m_coords,
			            extent m_extent,
						extent m_tile_extent,
						size_type n_cols  = 0,
						size_type n_tiles = 0); // default guesses with maximum space filled
			
			bool correct_tiling();
			
			virtual std::istream &read(std::istream &is);
			virtual std::ostream &write(std::ostream &os) const;
		};
		
		struct texture_data
		{
			TexturePtr m_tex;
			extent     m_extent;
			bool       b_linear_filter;
		};
	
		struct region
		{
			handle_type texture_id;
			region_info m_info;
		};
		
		struct tileset_def
		{
			extent m_coords;
			extent m_tile_extent;
			extent m_dim_extent;
			size_type n_last_row;
		};
		
		struct region_def
		{
			extent m_coords;
			extent m_extent;
		};
		
		enum class handle_class: uint8_t
		{
			nil,
			texture,
			region
		};
		
		inline handle_class get_handle_class(handle_type id)
		{
			return (id == 0) ? handle_class::nil : ((id > region_handle_base) ? handle_class::region : handle_class::texture);
		}
	private:
		RenderSystem &m_rendersystem;
		
		handle_name_allocator<texture_data, handle_type, 0>                   m_tex_alloc;
		handle_name_allocator<region,       handle_type, region_handle_base>  m_rgn_alloc;

		struct handle_info
		{
			handle_type   id;
			handle_class  e_class;
		
			texture_data *m_tex_data;
			region       *m_region;
		};
		
		inline size_type get_region_tile_count(handle_info &m_info)
		{
			return (m_info.m_region && m_info.m_region->m_info.b_tiled) ? m_info.m_region->m_info.n_tiles : 0;
		}
		
		bool resolve_texture_region(handle_info &m_info, coord &m_coords, extent &m_extent, tile_index_type i_tile = -1);
		BatchGeometryPtr gen_geometry(texture_data *m_tex_data, coord m_coords, extent m_extent, float depth = 0.0f);
		void gen_texcoords(BatchGeometryPtr m_geom, texture_data *m_tex_data, coord m_coords, extent m_extent) const;
		
		bool get_handle_info_impl(handle_type id, handle_class e_class, handle_info &m_info);
		
		inline bool get_handle_info(handle_type id, handle_info &m_info)
		{
			m_info.id = id;
			m_info.e_class = get_handle_class(id);
			
			m_info.m_tex_data = nullptr;
			m_info.m_region   = nullptr;
			
			return get_handle_info_impl(id, m_info.e_class, m_info);
		}
	public:
		atlas(RenderSystem &m_rendersystem);
		
		inline       RenderSystem &get_rendersystem()       {return m_rendersystem;}
		inline const RenderSystem &get_rendersystem() const {return m_rendersystem;}
		
		bool gen_region_info(region_info &m_info, handle_type src_id, const tileset_def &m_def);
		bool gen_region_info(region_info &m_info, handle_type src_id, const region_def &m_def);
		bool gen_region_info(region_info &m_info, handle_type src_id);
		
		handle_type create_texture(const std::string &m_name, bool b_linear_filter);
		handle_type create_region(const std::string &m_name, handle_type src_id, region_info m_info);
		
		// convenience region generators
		// creates a region with a tileset definition
		inline handle_type create_region(const std::string &m_name, handle_type src_id, const tileset_def &m_def)
		{
			region_info m_info;
			
			return gen_region_info(m_info, src_id, m_def) ?
			       create_region(m_name, src_id, m_info)  :
				   0;
		}
		
		// creates an autocorrected region for part of the texture
		inline handle_type create_region(const std::string &m_name, handle_type src_id, const region_def &m_def)
		{
			region_info m_info;
			
			return gen_region_info(m_info, src_id, m_def) ?
			       create_region(m_name, src_id, m_info)  :
				   0;
		}
		
		// creates a region for the whole texture
		inline handle_type create_region(const std::string &m_name, handle_type src_id)
		{
			region_info m_info;
			
			return gen_region_info(m_info, src_id)       ?
			       create_region(m_name, src_id, m_info) :
				   0;
		}
		
		handle_type get_texture_handle(const std::string &m_name) const;
		handle_type get_region_handle (const std::string &m_name) const;
		
		const std::string &get_texture_name(handle_type id) const;
		const std::string &get_region_name (handle_type id) const;
		
		const texture_data *get_texture_data(const std::string &m_name) const;
		const texture_data *get_texture_data(handle_type id)            const;
		
		const region       *get_region(const std::string &m_name) const;
		const region       *get_region(handle_type id)            const;
		
		inline bool destroy_object(handle_type id)
		{
			switch (get_handle_class(id))
			{
			case handle_class::texture:
				destroy_texture(id);
				break;
			case handle_class::region:
				destroy_region(id);
				break;
			case handle_class::nil:
			default:
				return false;
				break;
			};
		}
		
		bool destroy_texture(const std::string &m_name);
		bool destroy_texture(handle_type id);
		
		bool destroy_region(const std::string &m_name);
		bool destroy_region(handle_type id);
		
		// we deliberately allow the user to defer handle recycling to avoid
		// accidental references to the wrong texture or region after it is
		// destroyed.
		void recycle_texture_handles();
		void recycle_region_handles();
		
		virtual std::istream &read(std::istream &is);
		virtual std::ostream &write(std::ostream &os) const;
		
		struct image_config
		{
			enum: uint8_t
			{
				simple = 0x0,
				scaled = 0x1,
				rotated = 0x2,
				centered = 0x4,
				all = scaled|rotated|centered,
				
				s = scaled, r = rotated, c = centered,
				sr = s|r, rc = r|c, sc = s|c
			};
		
			uint8_t i_flags;
			
			Vector2d m_scl;
			math::angle m_rot;
			
			float depth;
			
			image_config(uint8_t i_flags = simple, const Vector2d &m_scl = Vector2d(1,1), math::angle m_rot = 0.0f, float depth = 0.0f):
				i_flags(i_flags),
				m_scl(m_scl),
				m_rot(m_rot),
				depth(depth)
			{}
			
			image_config(uint8_t i_flags, math::angle m_rot, float depth = 0.0f):
				i_flags(i_flags),
				m_scl(1,1),
				m_rot(m_rot),
				depth(depth)
			{}
			
			explicit image_config(uint8_t i_flags, float depth, const Vector2d &m_scl = Vector2d(1,1), math::angle m_rot = 0.0f):
				i_flags(i_flags),
				m_scl(1,1),
				m_rot(m_rot),
				depth(depth)
			{}
			
			explicit image_config(uint8_t i_flags, float depth, math::angle m_rot):
				i_flags(i_flags),
				m_scl(1,1),
				m_rot(m_rot),
				depth(depth)
			{}
			
			image_config(const image_config &m) = default;
			image_config(image_config &&m) = default;
			
			image_config &operator =(const image_config &m) = default;
			image_config &operator =(image_config &&m) = default;
		};
		
		enum class factory_switch: uint8_t
		{
			retain_config,
			change_config
		};
	
		class      image;
		class rect_image;
		
		class image_factory;
	};
}

#endif
