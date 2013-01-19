#ifndef UTD_BUF2_H
#define UTD_BUF2_H

#include "common.h"
#include "exceptions.h"
#include "vector2i.h"

namespace UTD
{
	namespace detail
	{
		template <typename contentT>
		struct buf2_paste_ops
		{
			typedef
				contentT
				content_type;
		
			// copy input buffer to output buffer
			struct copy_op
			{
				inline void operator()(content_type in, content_type &out)
				{
					out = in;
				}
			};
			
			// logical paste functions
			struct or_op
			{
				inline void operator()(content_type in, content_type &out)
				{
					out |= in;
				}
			};
			
			struct xor_op
			{
				inline void operator()(content_type in, content_type &out)
				{
					out ^= in;
				}
			};
			
			struct nor_op
			{
				inline void operator()(content_type in, content_type &out)
				{
					out |= ~in;
				}
			};
			
			struct and_op
			{
				inline void operator()(content_type in, content_type &out)
				{
					out &= in;
				}
			};
			
			struct nand_op
			{
				inline void operator()(content_type in, content_type &out)
				{
					out &= ~in;
				}
			};
			
			typedef
				copy_op
				default_op;
		};
	
		template <typename contentT>
		struct buf2_interface
		{
			typedef
				contentT
				content_type;
		
			virtual ~buf2_interface() {}
		
			virtual bool is_virtual() const = 0;
			
			virtual coord  window_coords() const = 0;
			virtual extent window_extent() const = 0;
			
			virtual void  window_coords(coord m_coords)  = 0;
			virtual void  window_extent(extent m_extent) = 0;
			
			virtual 	  content_type *get_content() = 0;
			virtual const content_type *get_content() const = 0;
			
			virtual const extent get_extent() const = 0;
			virtual const size_type get_pitch() const = 0;
			
			static inline extent compute_window_extent(coord &m_coords, extent m_src_extent, extent m_dst_extent)
			{
				extent m_new_extent = min(m_coords + m_src_extent, m_dst_extent) - m_coords;
				if (min(m_new_extent, m_dst_extent) != m_new_extent)
					throw invalid_buf2_window(m_coords.x, m_coords.y, m_src_extent.x, m_src_extent.y, m_dst_extent.x, m_dst_extent.y);
				
				return m_new_extent;
			}
			
			static inline coord compute_corrected_metrics(coord m_coords, coord &m_wnd_coords, extent &m_wnd_extent)
			{
				if (max(m_wnd_extent + m_coords, coord()) == coord())
					throw invalid_buf2_window(m_coords.x, m_coords.y, m_wnd_extent.x, m_wnd_extent.y, m_wnd_extent.x, m_wnd_extent.y);
			
				coord m_new_coords = max(m_coords, coord());
				m_wnd_coords = -min(m_coords, coord());
				m_wnd_extent = m_wnd_extent - m_wnd_coords;
				
				return m_new_coords;
			}
			
			inline void fill(contentT dat, coord m_coords = coord(), extent m_extent = extent());
			inline void sample(buf2_interface *m_dst, contentT blank, coord m_coords = coord());
			
			inline void set(coord m_coords, contentT v)
			{
				get_content()[m_coords.to_map_index(this)] = v;
			}
			
			inline void set(size_type i, contentT v)
			{
				get_content()[i] = v;
			}
			
			inline void set(coord::integer_type x, coord::integer_type y, contentT v)
			{
				set(coord(x, y), v);
			}
			
			inline contentT get(coord m_coords) const
			{
				return get_content()[m_coords.to_map_index(this)];
			}
			
			inline contentT get(size_type i) const
			{
				return get_content()[i];
			}
			
			inline contentT get(coord::integer_type x, coord::integer_type y) const
			{
				return get(coord(x, y));
			}

			template <typename functionT>
			inline void each_in(coord m_coords, extent m_extent, functionT m_fn) const
			{
				if (m_coords.x < 0 || m_coords.y < 0)
				{
					coord  m_wnd_coords;
					extent m_wnd_extent;
					
					compute_corrected_metrics(m_coords, m_wnd_coords, m_wnd_extent);
					each_in(m_wnd_coords, m_wnd_extent, m_fn);
					return;
				}
			
				extent m_dst_extent = get_extent();
				if (m_extent == extent())
					m_extent = m_dst_extent;
			
				const content_type *p_dst = get_content() + m_coords.to_map_index(this);
				
				size_type pitch = get_pitch();
				m_extent = compute_window_extent(m_coords, m_extent, m_dst_extent);
				
				for (size_t y = 0; y < m_extent.y; ++y)
				{
					for (size_t x = 0; x < m_extent.x; ++x)
						m_fn(m_coords + coord(x, y), p_dst[x]);
					
					p_dst += pitch;
				}
			}
			
			template <typename functionT>
			inline void each_in(coord m_coords, extent m_extent, functionT m_fn)
			{
				if (m_coords.x < 0 || m_coords.y < 0)
				{
					coord  m_wnd_coords;
					extent m_wnd_extent;
					
					compute_corrected_metrics(m_coords, m_wnd_coords, m_wnd_extent);
					each_in(m_wnd_coords, m_wnd_extent, m_fn);
					return;
				}
			
				extent m_dst_extent = get_extent();
				if (m_extent == extent())
					m_extent = m_dst_extent;
			
				content_type *p_dst = get_content() + m_coords.to_map_index(this);
				
				size_type pitch = get_pitch();
				m_extent = compute_window_extent(m_coords, m_extent, m_dst_extent);
				
				for (size_t y = 0; y < m_extent.y; ++y)
				{
					for (size_t x = 0; x < m_extent.x; ++x)
						m_fn(m_coords + coord(x, y), p_dst[x]);
					
					p_dst += pitch;
				}
			}
		};
		
		template <typename contentT, typename functionT>
		void buf2_paste(buf2_interface<contentT> *m_dst,
					    const buf2_interface<contentT> *m_src,
						coord m_dst_target, functionT m_fn);
		
		template <typename contentT>
		struct buf2_window:
			buf2_interface<contentT>
		{
			typedef
				contentT
				content_type;
			
			buf2_interface<contentT> *m_buf;
			mutable contentT         *p_content;
			coord                     m_coords;
			extent    				  m_extent;
			
			mutable coord     m_true_coords;
			mutable extent    m_true_extent;
			
			mutable size_type m_buf_prev_pitch;
			mutable extent    m_buf_prev_extent;
			
			void recompute() const
			{
				if (m_buf_prev_pitch != m_buf->get_pitch() || m_buf_prev_extent != m_buf->get_extent())
				{
					m_buf_prev_pitch  = m_buf->get_pitch();
					m_buf_prev_extent = m_buf->get_extent();
				
					m_true_extent = min(m_extent, m_buf->get_extent()-m_coords);
					coord m_src_coords = buf2_interface<contentT>::compute_corrected_metrics(m_coords, m_true_coords, m_true_extent);
					m_true_extent = buf2_interface<contentT>::compute_window_extent(m_true_coords, m_true_extent, m_buf->get_extent());
				
					p_content = m_buf->get_content() + m_src_coords.to_map_index(this);
				}
			}
			
			buf2_window(buf2_interface<contentT> *m_buf, coord m_coords, extent m_extent):
				m_buf(m_buf),
				p_content(nullptr),
				m_coords(m_coords),
				m_extent(m_extent),
				m_buf_prev_pitch(0),
				m_buf_prev_extent()
			{}
		
			virtual inline bool is_virtual() const {return true;}
			
			virtual inline coord  window_coords() const {return m_coords;}
			virtual inline extent window_extent() const {return m_extent;}
			
			// m_buf_prev_pitch = 0 marks the current 
			virtual inline void   window_coords(coord m_coords)  {this->m_coords = m_coords; m_buf_prev_pitch = 0;}
			virtual inline void   window_extent(extent m_extent) {this->m_extent = m_extent; m_buf_prev_pitch = 0;}
			
			virtual inline 	     content_type *get_content()  	   {recompute(); return p_content;}
			virtual inline const content_type *get_content() const {recompute(); return p_content;}
			
			virtual inline const extent get_extent() const         {recompute(); return m_true_extent;}
			virtual inline const size_type get_pitch() const 	   {recompute(); return m_buf_prev_pitch;}
		};
		
		// source buffer interface is constant
		template <typename contentT>
		struct buf2_const_window:
			buf2_window<contentT>
		{
			typedef
				contentT
				content_type;
			
			buf2_const_window(const buf2_interface<contentT> *m_buf, coord m_coords, extent m_extent):
				buf2_window<contentT>(const_cast<buf2_interface<contentT> *>(m_buf), m_coords, m_extent)
			{}
		
			//virtual inline       content_type *get_content()       {return nullptr;}
		};
		
		template <typename contentT>
		struct buf2_resizable:
			buf2_interface<contentT>
		{
			virtual void resize(extent m_extent) = 0;
		};
		
		template <typename contentT>
		struct buf2_mem:
			buf2_resizable<contentT>
		{
			typedef
				contentT
				content_type;
		
			util::optional<std::vector<contentT>> m_content;
			
			extent    m_extent;
			size_type m_pitch;
			
			virtual inline void resize(extent m_extent)
			{
				if (m_content)
				{
					if (m_extent.x > m_pitch || m_extent.y * m_pitch > size_type(m_content->size()))
					{
						buf2_mem m_new_map(m_extent);
						buf2_paste(&m_new_map, this, coord(), typename buf2_paste_ops<contentT>::copy_op());
						swap(&m_new_map);
					}
					else
						this->m_extent = m_extent;
				}
				else
				{
					m_content.create();
					m_content->resize(m_extent.size());
					m_pitch = m_extent.x;
					this->m_extent = m_extent;
				}
			}
			
			buf2_mem():
				m_extent(),
				m_pitch(0)
			{}
			
			buf2_mem(extent m_extent):
				m_extent(),
				m_pitch(0)
			{
				resize(m_extent);
			}
			
			inline void swap(buf2_mem *m_map)
			{
				if (!m_content)
				{
					if (!m_map->m_content)
						return;
					
					m_content.create(std::move(*(m_map->m_content)));
					m_map->m_content.destroy();
				}
				else if (!m_map->m_content)
				{
					m_map->m_content.create(std::move(*m_content));
					m_content.destroy();
				}
				else
					m_map->m_content->swap(*(this->m_content));
				
				extent m_t0 = m_extent;
				m_extent = m_map->m_extent;
				m_map->m_extent = m_t0;
				
				size_type m_t1 = m_pitch;
				m_pitch = m_map->m_pitch;
				m_map->m_pitch = m_t1;
			}
		
			virtual inline bool  is_virtual() const                {return false;}
			
			virtual inline coord  window_coords() const {return coord();}
			virtual inline extent window_extent() const {return extent();}
			
			virtual inline void   window_coords(coord m_coords)  {}
			virtual inline void   window_extent(extent m_extent) {}
		
			virtual inline 	     content_type *get_content() 	   {return m_content ? m_content->data() : nullptr;}
			virtual inline const content_type *get_content() const {return m_content ? m_content->data() : nullptr;}
			
			virtual inline const extent        get_extent()      const {return m_extent;}
			virtual inline const size_type     get_pitch()       const {return m_pitch;}
		};
		
		// layered map with interleaved rows
		template <typename contentT, size_type n_layers>
		struct buf2_layered_mem:
			buf2_resizable<contentT>
		{
		public:
			static constexpr size_type layer_count = n_layers;
			
			typedef
				contentT
				content_type;
			
		private:
			struct M_
			{
				buf2_mem<contentT> m_bufm;
				util::optional<buf2_window<contentT>> m_bufws[n_layers];
				
				extent m_layer_extent;
				
				M_(extent m_layer_extent):
					m_bufm(m_layer_extent * extent(n_layers, 1)),
					m_layer_extent(m_layer_extent)
				{
					for (size_type i = 0; i < n_layers; ++i)
						m_bufws[i].create(&m_bufm, coord(i * m_layer_extent.x, 0), m_layer_extent);
				}
				
				inline M_ *copy_to_new_size(extent _ex)
				{
					M_ *m_ = new M_(_ex);
					for (size_type i = 0; i < n_layers; ++i)
						buf2_paste((buf2_interface<contentT> *)m_->m_bufws[i],
						           (const buf2_interface<contentT> *)m_bufws[i], coord(), typename buf2_paste_ops<contentT>::copy_op());
					
					return m_;
				}
				
				inline       contentT *get_layer_content(size_type i)       {return m_bufws[i]->get_content();}
				inline const contentT *get_layer_content(size_type i) const {return m_bufws[i]->get_content();}
				
				inline       buf2_window<contentT> *get_layer_interface(size_type i)       {return m_bufws[i];}
				inline const buf2_window<contentT> *get_layer_interface(size_type i) const {return m_bufws[i];}
			};
			
			M_ *m_;
			size_type _layer_i;
		public:
			virtual inline void resize(extent m_layer_extent)
			{
				if (m_)
				{
					M_ *m_new_ = m_->copy_to_new_size(m_layer_extent);
					
					delete m_;
					m_ = m_new_;
				}
				else
					m_ = new M_(m_layer_extent);
			}
		
			buf2_layered_mem():
				m_(nullptr),
				_layer_i(0)
			{}
			
			buf2_layered_mem(extent m_layer_extent):
				m_(nullptr),
				_layer_i(0)
			{
				resize(m_layer_extent);
			}
			
			virtual inline ~buf2_layered_mem()
			{
				if (m_) delete m_;
			}
			
			inline void swap(buf2_layered_mem *m_map)
			{
				M_ *_tmp = m_map->m_;
				m_map->m_ = m_;
				m_ = _tmp;
			}
			
			inline void set_active_layer(size_type i)
			{
				if (i > n_layers)
					throw buf2_layer_out_of_bounds(i, n_layers);
				
				_layer_i = i;
			}
			
			inline size_type get_active_layer() const
			{
				return _layer_i;
			}
			
			inline buf2_interface<contentT> *get_layer_interface(size_type i)
			{
				if (i > n_layers)
					throw buf2_layer_out_of_bounds(i, n_layers);
			
				return m_ ? m_->get_layer_interface(i) : nullptr;
			}
			
			inline const buf2_interface<contentT> *get_layer_interface(size_type i) const
			{
				if (i > n_layers)
					throw buf2_layer_out_of_bounds(i, n_layers);
			
				return m_ ? m_->get_layer_interface(i) : nullptr;
			}
		
			virtual inline bool  is_virtual() const     {return false;}
			
			virtual inline coord  window_coords() const {return coord();}
			virtual inline extent window_extent() const {return extent();}
			
			virtual inline void   window_coords(coord m_coords)  {}
			virtual inline void   window_extent(extent m_extent) {}
		
			virtual inline 	     content_type *get_content() 	   {return m_ ? m_->get_layer_content(_layer_i) : nullptr;}
			virtual inline const content_type *get_content() const {return m_ ? m_->get_layer_content(_layer_i) : nullptr;}
			
			virtual inline const extent        get_extent()  const {return m_ ? m_->m_layer_extent : extent();}
			virtual inline const size_type     get_pitch()   const {return m_ ? m_->m_bufm.get_pitch() : 0;}
		};
		
		template <typename contentT, typename functionT>
		void buf2_paste(buf2_interface<contentT> *m_dst,
					    const buf2_interface<contentT> *m_src,
						coord m_dst_target, functionT m_fn)
		{
			if (m_dst_target.x < 0 || m_dst_target.y < 0)
			{
				coord  m_wnd_coords;
				extent m_wnd_extent = m_src->get_extent();

				m_dst_target = buf2_interface<contentT>::compute_corrected_metrics(m_dst_target, m_wnd_coords, m_wnd_extent);
				buf2_const_window<contentT> m_wnd(m_src, m_wnd_coords, m_wnd_extent);
				
				buf2_paste(m_dst, &m_wnd, m_dst_target, m_fn);
				return;
			}
			
			contentT *p_dst = m_dst->get_content() + m_dst_target.to_map_index(m_dst);
			const contentT *p_src = m_src->get_content();
			
			size_type dst_pitch = m_dst->get_pitch();
			size_type src_pitch = m_src->get_pitch();
			
			extent m_dst_extent  = m_dst->get_extent();
			extent m_src_extent  = buf2_interface<contentT>::compute_window_extent(m_dst_target, m_src->get_extent(), m_dst_extent);
			
			for (size_t y = 0; y < m_src_extent.y; ++y)
			{
				const contentT *p_src_end = p_src + m_src_extent.x;
				const contentT *p_src_it = p_src;
				contentT *p_dst_it = p_dst;
				
				for (; p_src_it < p_src_end; (++p_src_it, ++p_dst_it))
					m_fn(*p_src_it, *p_dst_it);
			
				p_dst += dst_pitch;
				p_src += src_pitch;
			}
		}
		
		template <typename contentT>
		inline void buf2_interface<contentT>::fill(contentT dat, coord m_coords, extent m_extent)
		{
			each_in
			(
				m_coords,
				m_extent,
				[&] (coord c, contentT &m) {m = dat;}
			);
		}
		
		template <typename contentT>
		inline void buf2_interface<contentT>::sample(buf2_interface<contentT> *m_dst, contentT blank, coord m_coords)
		{
			m_dst->fill(blank);
			buf2_paste(m_dst, this, -m_coords, typename buf2_paste_ops<contentT>::copy_op());
		}
		
		// concrete implementations for smallest ints
		template <typename T>
		struct buf2_streaming;
		
#define __UTD_BUF2_STREAMING_DEF(T)                                        \
template <>						                                           \
struct buf2_streaming<T>		                                           \
{                                                                          \
	static std::ostream &write(std::ostream &os, const buf2_interface<T> *m_buf); \
	static std::istream &read(std::istream &is, buf2_interface<T> *m_buf);        \
};

		__UTD_BUF2_STREAMING_DEF(int8_t)
		__UTD_BUF2_STREAMING_DEF(uint8_t)
		__UTD_BUF2_STREAMING_DEF(int16_t)
		__UTD_BUF2_STREAMING_DEF(uint16_t)
		__UTD_BUF2_STREAMING_DEF(int32_t)
		__UTD_BUF2_STREAMING_DEF(uint32_t)
		__UTD_BUF2_STREAMING_DEF(int64_t)
		__UTD_BUF2_STREAMING_DEF(uint64_t)

#undef __UTD_BUF2_STREAMING_DEF
	}
	
	template <typename T>
	struct buf2
	{
		typedef T				                content_type;  
		typedef detail::buf2_paste_ops<T>       paste_ops;
		typedef detail::buf2_interface<T>       bufi;		   // buffer interface, homogenous for windows and concrete buffers.
		typedef detail::buf2_window<T>          bufw;		   // buffer window, a window in to a buffer interface.
		typedef detail::buf2_resizable<T>       bufr;          // resizable buffer interface, concrete buffers only (standard or layered)
		typedef detail::buf2_const_window<T>    bufw_const;    // const buffer window, a window in to a const buffer interface.
		typedef detail::buf2_mem<T>             bufm;          // a concrete buffer in memory.
		typedef detail::buf2_streaming<T>       streaming;     // the streaming op container
		
		template <size_type n_layers>
		struct bufl:
			detail::buf2_layered_mem<T, n_layers>
		{
			bufl():
				detail::buf2_layered_mem<T, n_layers>()
			{}
			
			bufl(extent m_layer_extent):
				detail::buf2_layered_mem<T, n_layers>(m_layer_extent)
			{}
		};

		template <typename functionT = typename paste_ops::copy_op>
		static inline void paste(bufi *m_dst,
				    		     const bufi *m_src,
						         coord m_dst_target = coord(),
								 functionT m_fn = functionT())
		{
			buf2_paste(m_dst, m_src, m_dst_target, m_fn);
		}
	};
	
	typedef buf2<logical_type> logic_buf2;
	typedef buf2<size_type>    tile_buf2;
	typedef buf2<char_type>    char_buf2;
	typedef buf2<int_type>     int_buf2;
}

#endif
