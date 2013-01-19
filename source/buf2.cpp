#include "buf2.h"
#include "encoding.h"

namespace UTD
{
	namespace detail
	{
		template <typename T, typename profileT = encoding::default_ascii_profile<T> >
		struct _buf2_streaming_impl
		{
			typedef buf2_interface<T> bufi;
			typedef buf2_resizable<T> bufm;
		
			static inline std::ostream &write(std::ostream &os, const bufi &m_buf)
			{
				typedef profileT prof_t;
				typedef encoding::encode<prof_t> encode_t;
				typedef encoding::decode<prof_t> decode_t;
			
				extent m_extent = m_buf.get_extent();
			
				os << m_buf.get_extent();
				for (size_type y = 0; y < m_extent.y; ++y)
				{
					for (size_type x = 0; x < m_extent.x; ++x)
						os << ' ' << encode_t(m_buf.get(x, y));
				}
				
				return os;
			}
			
			static inline std::istream &read(std::istream &is, bufi &m_buf)
			{
				typedef profileT prof_t;
				typedef encoding::encode<prof_t> encode_t;
				typedef encoding::decode<prof_t> decode_t;
			
				extent m_extent;
				is >> m_extent;
				
				if (min(m_extent, m_buf.get_extent()) != m_extent)
				{
					// see if the stream is concrete (can be resized)
					bufm *m_bufm = dynamic_cast<bufm *>(&m_buf);
					if (!m_bufm) throw buf2_stream_too_big(m_extent.x, m_extent.y, m_buf.get_extent().y, m_buf.get_extent().y);
					
					// resize to fit
					m_bufm->resize(m_extent);
				}
				
				for (size_type y = 0; y < m_extent.y; ++y)
				{
					for (size_type x = 0; x < m_extent.x; ++x)
					{
						T dat = T();
						is >> decode_t(dat);
						m_buf.set(x, y, dat);
					}
				}
				
				return is;
			}
		};
	
#define __UTD_BUF2_STREAMING_IMPL(T)                                                         \
	std::ostream &buf2_streaming<T>::write(std::ostream &os, const buf2_interface<T> *m_buf) \
		{ return m_buf ? _buf2_streaming_impl<T>::write(os, *m_buf) : os; }		             \
	std::istream &buf2_streaming<T>::read(std::istream &is, buf2_interface<T> *m_buf)        \
		{ return m_buf ? _buf2_streaming_impl<T>::read(is, *m_buf) : is; }

		__UTD_BUF2_STREAMING_IMPL(int8_t)
		__UTD_BUF2_STREAMING_IMPL(uint8_t)
		__UTD_BUF2_STREAMING_IMPL(int16_t)
		__UTD_BUF2_STREAMING_IMPL(uint16_t)
		__UTD_BUF2_STREAMING_IMPL(int32_t)
		__UTD_BUF2_STREAMING_IMPL(uint32_t)
		__UTD_BUF2_STREAMING_IMPL(int64_t)
		__UTD_BUF2_STREAMING_IMPL(uint64_t)
		
#undef __UTD_BUF2_STREAMING_IMPL
	}
}