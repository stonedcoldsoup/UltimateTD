#include "map2.h"

namespace UTD
{
	std::istream &auto_tile_map::read(std::istream &is)
	{
		is >> b_solid_map_edge;
		
		m_bufl.set_active_layer(0);
		if (logic_tile_buf::streaming::read(is, &m_bufl)) update();

		return is;
	}
	
	std::ostream &auto_tile_map::write(std::ostream &os) const
	{
		return (os << b_solid_map_edge) ?
		       logic_tile_buf::streaming::write(os, m_bufl.get_layer_interface(0)) :
			   os;
	}

	void auto_tile_map::resize(extent m_extent)
	{
		m_bufl.resize(m_extent);
	}

	template <size_type i_edge>
	inline void auto_tile_map::update_tile_pattern(coord m_coords)
	{
		if (!m_bufl.get(m_coords))
		{
			m_bufl.set_active_layer(1);
			m_bufl.set(m_coords, 0);
			m_bufl.set_active_layer(0);
			return; // do not get neighbor info for
				   // empty tiles
		}

		uint8_t byte = b_solid_map_edge ? sample_edge_initialize(i_edge) : 0;
		each_in_mask
		(
			neighbor_sample_mask(i_edge),
			[&] (coord m_nb_coord, uint8_t bit)
			{
				if (m_bufl.get(m_coords+m_nb_coord)) byte |= bit;
			}
		);
		
		m_bufl.set_active_layer(1);
		m_bufl.set(m_coords, byte);
		m_bufl.set_active_layer(0);
	}
	
	void auto_tile_map::update()
	{
		m_bufl.set_active_layer(0);

		if (get_extent().x < 2 && get_extent().y < 2) return;
		
		// do corners
		update_tile_pattern<nbi_NW>(coord());
		update_tile_pattern<nbi_NE>(coord(get_extent().x-1, 0));
		
		update_tile_pattern<nbi_SW>(coord(0, get_extent().y-1));
		update_tile_pattern<nbi_SE>(coord(get_extent().x-1, get_extent().y-1));
		
		// do N/S walls
		for (size_type i = 1; i < get_extent().x-1; ++i)
		{
			update_tile_pattern<nbi_N>(coord(i, 0));
			update_tile_pattern<nbi_S>(coord(i, get_extent().y-1));
		}
		
		// do W/E walls
		for (size_type i = 1; i < get_extent().y-1; ++i)
		{
			update_tile_pattern<nbi_W>(coord(0, i));
			update_tile_pattern<nbi_E>(coord(get_extent().x-1, i));
		}
		
		// avoid a bunch of empty for loops if map is wide enough but not tall enough.
		if (get_extent().y > 2)
		{
			// do body
			for (size_type x = 1; x < get_extent().x-1; ++x)
			{
				for (size_type y = 1; y < get_extent().y-1; ++y)
					update_tile_pattern<nbi_none>(coord(x, y));
			}
		}
	}
	
	      logic_tile_buf::bufi *auto_tile_map::state_buf()			 {return m_bufl.get_layer_interface(0);}
	const logic_tile_buf::bufi *auto_tile_map::state_buf() const     {return m_bufl.get_layer_interface(0);}
	
	const logic_tile_buf::bufi *auto_tile_map::neighbor_buf() const  {return m_bufl.get_layer_interface(1);}
}
