#ifndef UTD_MAP2_H
#define UTD_MAP2_H

#include "common.h"
#include "buf2.h"
#include "tile.h"
#include "exceptions.h"

namespace UTD
{
	typedef
		buf2<tile_index_type>
		video_tile_buf;

	typedef
		buf2<logical_type>
		logic_tile_buf;

	class auto_tile_map:
		public streamable
	{
	private:
		logic_tile_buf::bufl<2> m_bufl;
		bool b_solid_map_edge;
	
		template <size_type i_edge>
		inline void update_tile_pattern(coord m_coords);
	public:
		template <typename... T>
		auto_tile_map(T&&... v):
			m_bufl(std::forward<T>(v)...),
			b_solid_map_edge(true)
		{
			m_bufl.set_active_layer(0);
			m_bufl.fill(0);
			
			m_bufl.set_active_layer(1);
			m_bufl.fill(0);
		}
		
		virtual std::istream &read(std::istream &is);
		virtual std::ostream &write(std::ostream &os) const;

		void resize(extent m_extent);
		inline extent get_extent() const
		{
			return m_bufl.get_extent();
		}
		
		void update();
		
		      logic_tile_buf::bufi *state_buf();
		const logic_tile_buf::bufi *state_buf() const;
		
		const logic_tile_buf::bufi *neighbor_buf() const;
	};
}

#endif
