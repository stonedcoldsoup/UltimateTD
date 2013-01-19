#include "tile.h"
#include "encoding.h"

namespace UTD
{
	typedef encoding::default_ascii_profile<tile_index_type> profile_type;
	typedef encoding::encode<profile_type> encode_type;
	typedef encoding::decode<profile_type> decode_type;

	pattern_part::pattern_part()
	{
		std::fill(m_buf, m_buf + nbi_count, empty_mark);
	}
	
	pattern_part::pattern_part(const pattern_part &m_pattern)
	{
		operator =(m_pattern);
	}
	
	pattern_part &pattern_part::operator =(const pattern_part &m_pattern)
	{
		std::copy(m_pattern.m_buf, m_pattern.m_buf + nbi_count, m_buf);
		return *this;
	}
	
	pattern_part::pattern_part(pattern_part &&m_pattern)
	{
		operator =(std::move(m_pattern));
	}
	
	pattern_part &pattern_part::operator =(pattern_part &&m_pattern)
	{
		std::copy(m_pattern.m_buf, m_pattern.m_buf + nbi_count, m_buf);
		return *this;
	}

	void pattern_part::gen_patterns(std::vector<pattern> &m_patterns) const
	{
		for (size_type i = 0; i < nbi_count; ++i)
		{
			switch (m_buf[i])
			{
			case unknown_mark:
				{	// do fork
					pattern_part m_p0(*this), m_p1(*this);
					
					m_p0.m_buf[i] = empty_mark;
					m_p1.m_buf[i] = tile_mark;
					
					m_p0.gen_patterns(m_patterns);
					m_p1.gen_patterns(m_patterns);
				}
				// return after forking, the presence of an
				// unknown marker is an ambiguous pattern
				return;
			default:
				break;
			};
		}
		
		// we got this far, no unknowns left in this permutation
		// build the byte
		uint8_t byte = 0;
		for (size_type i = 0; i < nbi_count; ++i)
		{
			switch (m_buf[i])
			{
			case tile_mark:
				byte |= 1 << i;
				break;
			case empty_mark:
				break;
			default:
				throw invalid_autotile_pattern_permutation(*this);
			};
		}
		
		m_patterns.push_back(byte);
	}
	
	std::istream &pattern_part::read(std::istream &is)
	{
		for (size_type i = 0; i < nbi_count; ++i)
		{
			if (!is.get(m_buf[i])) return is;
		}
		
		is >> decode_type(m_index);
		
		return is;
	}
	
	std::ostream &pattern_part::write(std::ostream &os) const
	{
		for (size_type i = 0; i < nbi_count; ++i)
		{
			if (!os.put(m_buf[i])) return os;
		}
		
		os << encode_type(m_index) << ' ';
		
		return os;
	}
	
	auto_tile_pattern_buffer::auto_tile_pattern_buffer()
	{
		clear_sources();
		clear_tiles();
	}
	
	std::istream &auto_tile_pattern_buffer::read(std::istream &is)
	{
		while (is)
		{
			char c = 0;
			if (is.get(c))
			{
				if (c == pattern_part::eof_mark)
					break;
				
				is.unget();
				pattern_part m_part;
				if (!m_part.read(is))
					break;
				
				_src.push_back(m_part);
			}
		}
		
		return is;
	}
	
	std::ostream &auto_tile_pattern_buffer::write(std::ostream &os) const
	{
		for (const pattern_part &m_part: _src)
		{
			if (!m_part.write(os))
				break;
		}
		
		os.put(pattern_part::eof_mark);
		return os;
	}
	
	void auto_tile_pattern_buffer::gen_patterns()
	{
		for (pattern_part &m_part: _src)
		{
			std::vector<pattern> m_pats;
			m_part.gen_patterns(m_pats);
			
			for (pattern m_pat: m_pats)
				_tiles[uint8_t(m_pat)] = m_part.m_index;
		}
	}
}
