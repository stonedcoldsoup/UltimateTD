#ifndef UTD_TILE_H
#define UTD_TILE_H

#include "common.h"
#include "exceptions.h"
#include "vector2i.h"
#include "streamable.h"

namespace UTD
{
	enum: size_type
	{
		nbi_NE = 0,
		nbi_N  = 1,
		nbi_NW = 2,
		nbi_W  = 3,
		nbi_E  = 4,
		nbi_SW = 5,
		nbi_S  = 6,
		nbi_SE = 7,
		
		nbi_max   = 7,
		nbi_none  = 8,
		nbi_count = 8
	};
	
	enum: uint8_t
	{
#ifdef UTD_BIG_ENDIAN
		nbb_NE = 0b10000000,
		nbb_N  = 0b01000000,
		nbb_NW = 0b00100000,
		nbb_W  = 0b00010000,
		nbb_E  = 0b00001000,
		nbb_SW = 0b00000100,
		nbb_S  = 0b00000010,
		nbb_SE = 0b00000001
#else
		nbb_NE = 0b00000001,
		nbb_N  = 0b00000010,
		nbb_NW = 0b00000100,
		nbb_W  = 0b00001000,
		nbb_E  = 0b00010000,
		nbb_SW = 0b00100000,
		nbb_S  = 0b01000000,
		nbb_SE = 0b10000000
#endif
	};
	
	static constexpr uint8_t g_nbbs[nbi_count] =
	{
		nbb_NE,
		nbb_N,
		nbb_NW,
		nbb_W,
		nbb_E,
		nbb_SW,
		nbb_S,
		nbb_SE
	};
	
	// x == row, y == col
	static constexpr size_type g_neighbor_coord_nbis[3][3] =
	{
		{nbi_NW, nbi_W,    nbi_SW}, // x == -1
		{nbi_N,  nbi_none, nbi_S},  // x == 0
		{nbi_NE, nbi_E,    nbi_SE}  // x == 1
	};
	
	static inline constexpr size_type get_neighbor_coord_nbi(coord m_coord)
	{
		return (m_coord.x < -1 || m_coord.x > 1 || m_coord.y < -1 || m_coord.y > 1) ? nbi_none : g_neighbor_coord_nbis[m_coord.x+1][m_coord.y+1];
	}
	
	static constexpr coord g_neighbor_coords[nbi_count] =
	{
		coord( 1, -1), //nbi_NE = 0,
		coord( 0, -1), //nbi_N  = 1,
		coord(-1, -1), //nbi_NW = 2,
		coord(-1,  0), //nbi_W  = 3,
		coord( 1,  0), //nbi_E  = 4,
		coord(-1,  1), //nbi_SW = 5,
		coord( 0,  1), //nbi_S  = 6,
		coord( 1,  1)  //nbi_SE = 7,
	};
	
	namespace neighbor_debug_marker
	{
		static constexpr float marker_size       = 0.1f;
		static constexpr float marker_scatter    = 0.5f-(marker_size/2.0f);
		static constexpr float marker_min_origin = 0.5f-(marker_size/2.0f);
		static constexpr float marker_max_origin = 0.5f+(marker_size/2.0f);
	
		static constexpr float get_x_a(size_type i_neighbor, coord m_coord)
		{
			return float(m_coord.x)+marker_min_origin + float(g_neighbor_coords[i_neighbor].x)*marker_scatter;
		}
		
		static constexpr float get_y_a(size_type i_neighbor, coord m_coord)
		{
			return float(m_coord.y)+marker_min_origin + float(g_neighbor_coords[i_neighbor].y)*marker_scatter;
		}
		
		static constexpr float get_x_b(size_type i_neighbor, coord m_coord)
		{
			return float(m_coord.x)+marker_max_origin + float(g_neighbor_coords[i_neighbor].x)*marker_scatter;
		}
		
		static constexpr float get_y_b(size_type i_neighbor, coord m_coord)
		{
			return float(m_coord.y)+marker_max_origin + float(g_neighbor_coords[i_neighbor].y)*marker_scatter;
		}
		
		static constexpr float get_width(size_type i_neighbor, coord m_coord)
		{
			return get_x_b(i_neighbor, m_coord) - get_x_a(i_neighbor, m_coord);
		}
		
		static constexpr float get_height(size_type i_neighbor, coord m_coord)
		{
			return get_y_b(i_neighbor, m_coord) - get_y_a(i_neighbor, m_coord);
		}
		
		template <size_type i_neighbor>
		static inline Vector2d get_extent(coord m_coord, float tw, float th)
		{
			return Vector2d(get_width(i_neighbor, m_coord) * tw, get_height(i_neighbor, m_coord) * th);
		}
		
		template <size_type i_neighbor>
		static inline Vector2d get_position(coord m_coord, float tw, float th)
		{
			return Vector2d(get_x_a(i_neighbor, m_coord) * tw, get_y_a(i_neighbor, m_coord) * th);
		}
		
		template <size_type i_neighbor>
		static inline phoenix::Rectangle get_rect(coord m_coord, float tw, float th, const Vector2d &m_offset = Vector2d(0,0))
		{
			return phoenix::Rectangle(get_position<i_neighbor>(m_coord, tw, th) + m_offset,
									  get_extent  <i_neighbor>(m_coord, tw, th));
		}
	}
	
	static constexpr uint8_t g_edge_lists[nbi_count] =
	{
		nbb_NE|nbb_N|nbb_NW|nbb_E|nbb_SE, 	//nbi_NE = 0,
		nbb_N|nbb_NE|nbb_NW, 				//nbi_N  = 1,
		nbb_NW|nbb_N|nbb_NE|nbb_W|nbb_SW, 	//nbi_NW = 2,
		nbb_W|nbb_NW|nbb_SW, 				//nbi_W  = 3,
		nbb_E|nbb_NE|nbb_SE, 				//nbi_E  = 4,
		nbb_SW|nbb_S|nbb_SE|nbb_W|nbb_NW, 	//nbi_SW = 5,
		nbb_S|nbb_SE|nbb_SW, 				//nbi_S  = 6,
		nbb_SE|nbb_S|nbb_SW|nbb_E|nbb_NE  	//nbi_SE = 7,
	};
	
	static inline constexpr uint8_t neighbor_sample_mask(size_type i_edge)
	{
		return i_edge >= nbi_none ? 0xFF : ~g_edge_lists[i_edge];
	}
	
	static inline constexpr uint8_t sample_edge_initialize(size_type i_edge)
	{
		return i_edge >= nbi_none ? 0 : g_edge_lists[i_edge];
	}
	
	static inline const char *get_pattern_name(uint8_t p)
	{
		switch (p)
		{
		case nbb_SE:	return "SE"; break;
		case nbb_S:		return "S";  break;
		case nbb_SW:	return "SW"; break;
		case nbb_E:		return "E";  break;
		case nbb_W:		return "W";  break;
		case nbb_NW:	return "NW"; break;
		case nbb_N:		return "N";  break;
		case nbb_NE:	return "NE"; break;
		
		case neighbor_sample_mask(nbi_SE):	return "SE sample"; break;
		case neighbor_sample_mask(nbi_S):	return "S sample";  break;
		case neighbor_sample_mask(nbi_SW):	return "SW sample"; break;
		case neighbor_sample_mask(nbi_E):	return "E sample";  break;
		case neighbor_sample_mask(nbi_W):	return "W sample";  break;
		case neighbor_sample_mask(nbi_NW):	return "NW sample"; break;
		case neighbor_sample_mask(nbi_N):	return "N sample";  break;
		case neighbor_sample_mask(nbi_NE):	return "NE sample"; break;
		
		case sample_edge_initialize(nbi_SE):	return "SE edge"; break;
		case sample_edge_initialize(nbi_S):		return "S edge";  break;
		case sample_edge_initialize(nbi_SW):	return "SW edge"; break;
		case sample_edge_initialize(nbi_E):		return "E edge";  break;
		case sample_edge_initialize(nbi_W):		return "W edge";  break;
		case sample_edge_initialize(nbi_NW):	return "NW edge"; break;
		case sample_edge_initialize(nbi_N):		return "N edge";  break;
		case sample_edge_initialize(nbi_NE):	return "NE edge"; break;
		
		default:
			return "<unknown!>";
			break;
		}
	}
	
	static inline std::ostream &write_pattern_text(std::ostream &os, uint8_t bits)
	{
		os.put((bits & nbb_NW) ? '1' : '0');
		os.put((bits & nbb_N)  ? '1' : '0');
		os.put((bits & nbb_NE) ? '1' : '0');
		os << std::endl;
		
		os.put((bits & nbb_W)  ? '1' : '0');
		os.put(' ');
		os.put((bits & nbb_E)  ? '1' : '0');
		os << std::endl;
		
		os.put((bits & nbb_SW) ? '1' : '0');
		os.put((bits & nbb_S)  ? '1' : '0');
		os.put((bits & nbb_SE) ? '1' : '0');
		
		return os;
	}

	template <typename functorT>
	static inline void each_in_mask(const uint8_t mask, functorT m_fn)
	{
		// manually unrolled
		if (g_nbbs[nbi_NE] & mask) m_fn(g_neighbor_coords[nbi_NE], g_nbbs[nbi_NE]);
		if (g_nbbs[nbi_N]  & mask) m_fn(g_neighbor_coords[nbi_N],  g_nbbs[nbi_N]);
		if (g_nbbs[nbi_NW] & mask) m_fn(g_neighbor_coords[nbi_NW], g_nbbs[nbi_NW]);
		if (g_nbbs[nbi_W]  & mask) m_fn(g_neighbor_coords[nbi_W],  g_nbbs[nbi_W]);
		if (g_nbbs[nbi_E]  & mask) m_fn(g_neighbor_coords[nbi_E],  g_nbbs[nbi_E]);
		if (g_nbbs[nbi_SW] & mask) m_fn(g_neighbor_coords[nbi_SW], g_nbbs[nbi_SW]);
		if (g_nbbs[nbi_S]  & mask) m_fn(g_neighbor_coords[nbi_S],  g_nbbs[nbi_S]);
		if (g_nbbs[nbi_SE] & mask) m_fn(g_neighbor_coords[nbi_SE], g_nbbs[nbi_SE]);
	}
	
	typedef
		int_type
		tile_index_type;
		
	static constexpr tile_index_type empty_tile_index = -1;
		
	struct pattern
	{
		uint8_t m_byte;
		
		pattern(uint8_t m_byte): m_byte(m_byte) {}
		pattern &operator =(uint8_t m_byte) {this->m_byte = m_byte; return *this;}
		
		operator uint8_t() const {return m_byte;}
	};
	
	// this is a partial pattern, representing every
	// permutation possible in the unknown bits,
	// and as a result resolves to multiple concrete
	// patterns.
	struct pattern_part:
		public streamable
	{
		enum: char
		{
			tile_mark    = '*',
			unknown_mark = '?',
			empty_mark   = '-',
			eof_mark     = '\\'
		};
	
		char m_buf[nbi_count];
		tile_index_type m_index;
		
		pattern_part();
		pattern_part(const pattern_part &m_pattern);
		pattern_part &operator =(const pattern_part &m_pattern);
		
		pattern_part(pattern_part &&m_pattern);
		pattern_part &operator =(pattern_part &&m_pattern);
		
		// generate the concrete binary patterns
		void gen_patterns(std::vector<pattern> &m_patterns) const;
		
		virtual std::istream &read(std::istream &is);
		virtual std::ostream &write(std::ostream &os) const;
	};
	
	class auto_tile_pattern_buffer:
		public streamable
	{
	private:
		std::vector<pattern_part> _src;
		tile_index_type _tiles[256];
	public:
		auto_tile_pattern_buffer();
		
		virtual std::istream &read(std::istream &is);
		virtual std::ostream &write(std::ostream &os) const;
		
		void gen_patterns();
		
		// delete source patterns (save memory)
		inline void clear_sources()
		{
			_src.clear();
		}
		
		inline void clear_tiles()
		{
			std::fill(_tiles, _tiles + 256, empty_tile_index);
		}
		
		inline       std::vector<pattern_part> &get_source_patterns()       {return _src;}
		inline const std::vector<pattern_part> &get_source_patterns() const {return _src;}
		
		inline tile_index_type get_tile(pattern m_pattern) const
		{
			return _tiles[uint8_t(m_pattern)];
		}
	};
}

#endif