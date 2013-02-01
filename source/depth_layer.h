#ifndef UTD_DEPTH_LAYER_H
#define UTD_DEPTH_LAYER_H

#include "common.h"
#include "exceptions.h"

#include <array>

namespace UTD
{
	template <int _min_depth, int _max_depth, size_t... n_segs>
	struct layer_range_def;
	
	template <int _min_depth, int _max_depth, size_t n_segs, size_t... n_seh>
	struct layer_range_def<_min_depth, _max_depth, n_segs, n_seh...>
	{
		static constexpr float  min_depth = _min_depth;
		static constexpr float  max_depth = _max_depth;
		
		static constexpr size_t seg_count = n_segs;
		static constexpr float  seg_size  = (max_depth-min_depth) / float(seg_count);
		
		static constexpr float  range     = max_depth-min_depth;
		static constexpr float  offset    = -range / 2.0f;
		
		static constexpr size_t sub_layer_count = 1 + layer_range_def<_min_depth, _max_depth, n_seh...>::sub_layer_count;
		
		static inline constexpr float get_major_layer_value(size_t i)
		{
			return seg_size*float(i)+min_depth;
		}
		
		static inline constexpr float __append_layer_value(float v, size_t i)
		{
			return v + seg_size*float(i) + offset;
		}
		
		template <typename iteratorT>
		static float __get_layer_value_impl(float v, iteratorT a, iteratorT b)
		{
			if (a != b)
			{
				return layer_range_def<_min_depth, _max_depth, n_seh...>::__get_layer_value_impl
				(
					__append_layer_value(v, *a),
					a+1, b
				);
			}
			else
				return v;
		}
		
		template <typename iteratorT>
		static inline float get_layer_value(iteratorT a, iteratorT b)
		{
			return __get_layer_value_impl(get_major_layer_value(*a), a+1, b);
		}
		
		template <typename layerT>
		static inline float get_layer_value(const layerT &l)
		{
			return get_layer_value(l.begin(), l.end());
		}
		
		template <typename valueT>
		static inline float get_layer_value(const valueT *v, size_t n)
		{
			return get_layer_value(v, v+n);
		}
	};
	
	// end condition stub
	template <int _min_depth, int _max_depth>
	struct layer_range_def<_min_depth, _max_depth>
	{
		static constexpr size_t sub_layer_count = 0;
	
		template <typename iteratorT>
		static inline float __get_layer_value_impl(float v, iteratorT a, iteratorT b)
		{
			return v;
		}
	};
	
	template <int _min_depth, int _max_depth, size_t n_segs, size_t... n_seh> constexpr float  layer_range_def<_min_depth, _max_depth, n_segs, n_seh...>::min_depth;
	template <int _min_depth, int _max_depth, size_t n_segs, size_t... n_seh> constexpr float  layer_range_def<_min_depth, _max_depth, n_segs, n_seh...>::max_depth;
	
	template <int _min_depth, int _max_depth, size_t n_segs, size_t... n_seh> constexpr size_t layer_range_def<_min_depth, _max_depth, n_segs, n_seh...>::seg_count;
	template <int _min_depth, int _max_depth, size_t n_segs, size_t... n_seh> constexpr float  layer_range_def<_min_depth, _max_depth, n_segs, n_seh...>::seg_size;
	
	template <int _min_depth, int _max_depth, size_t n_segs, size_t... n_seh> constexpr float  layer_range_def<_min_depth, _max_depth, n_segs, n_seh...>::range;
	template <int _min_depth, int _max_depth, size_t n_segs, size_t... n_seh> constexpr float  layer_range_def<_min_depth, _max_depth, n_segs, n_seh...>::offset;
	
	template <int _min_depth, int _max_depth, size_t n_segs, size_t... n_seh> constexpr size_t layer_range_def<_min_depth, _max_depth, n_segs, n_seh...>::sub_layer_count;
	
	// performance tweaking for UTD
	typedef
		layer_range_def<-128, 127, // between -128 and 127
		                16,        // 16 major widget layers, giving us increments of 16
						8,         // 8 major map layers, giving us increments of 2
						4>         // 4 minor map layers for overlays, giving us increments of .5
						// floating points should be plenty precise enough for this!
		default_layer_range_def;
	
	template <typename layer_range_defT = default_layer_range_def>
	class depth_layer_stack
	{
	public:
		typedef
			layer_range_defT
			range_def_type;
	    
	    typedef
	        std::array<size_t, range_def_type::sub_layer_count>
            depth_index_array;
	private:
		std::array<depth_index_array, range_def_type::sub_layer_count> m_stack;
		size_t i_stack;
		
	public:
		depth_layer_stack():
			i_stack(0)
		{
			std::fill(m_stack[0].begin(), m_stack[0].end(), 0);
		}
		
		depth_layer_stack(const depth_layer_stack &m):
			m_stack(m.m_stack),
			i_stack(m.i_stack)
		{}
		
		inline depth_layer_stack &operator =(const depth_layer_stack &m)
		{
			m_stack = m.m_stack;
			i_stack = m.i_stack;
			return *this;
		}
		
		~depth_layer_stack() = default;
		
		inline void reset()
		{	
			i_stack = 0;
			std::fill(m_stack[0].begin(), m_stack[0].end(), 0);
		}
		
		inline const depth_index_array &get_depth_indices() const
		{
			return m_stack[i_stack];
		}
		
		inline operator float() const
		{
			return range_def_type::get_layer_value(get_depth_indices());
		}
		
		inline depth_layer_stack &push()
		{
			size_t i_old = i_stack;
			if (++i_stack >= range_def_type::sub_layer_count)
				throw depth_sub_layer_out_of_range(i_stack, range_def_type::sub_layer_count);
			
			m_stack[i_stack] = m_stack[i_old];
			return *this;
		}
		
		inline depth_layer_stack &pop()
		{
			if (--i_stack >= range_def_type::sub_layer_count) reset();
			return *this;
		}
		
		inline depth_layer_stack &increment(int n = 1)
		{
			m_stack[i_stack][i_stack] += n;
			return *this;
		}
		
		inline depth_layer_stack &decrement(int n = 1)
		{
			return increment(-n);
		}
		
		inline depth_layer_stack &operator ++()
		{
			return increment();
		}
		
		inline depth_layer_stack operator ++(int)
		{
			depth_layer_stack m = *this;
			operator ++();
			return m;
		}
		
		inline depth_layer_stack &operator --()
		{
			return decrement();
		}
		
		inline depth_layer_stack operator --(int)
		{
			depth_layer_stack m = *this;
			operator --();
			return m;
		}
	};
	
	extern depth_layer_stack<> g_depth_layer_stack;
}

#endif
