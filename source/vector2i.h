#ifndef UTD_VECTOR2I_H
#define UTD_VECTOR2I_H

#include "common.h"

namespace UTD
{
	template <typename T>
	struct vector2i
	{
		typedef T        integer_type;
		typedef int_type diff_type;
	
		integer_type x;
		integer_type y;
		
		constexpr vector2i(integer_type x = 0, integer_type y = 0):
			x(x), y(y)
		{}
		
		constexpr vector2i(const vector2i<T> &m):
			x(integer_type(m.x)), y(integer_type(m.y))
		{}
		
		vector2i(vector2i<T> &&m):
			x(integer_type(m.x)), y(integer_type(m.y))
		{}
		
		vector2i(const Vector2d &m):
			x(m.getX()),
			y(m.getY())
		{}
		
		vector2i &operator =(const vector2i<T> &m)
		{
			x = integer_type(m.x);
			y = integer_type(m.y);
			return *this;
		}
		
		vector2i &operator =(vector2i<T> &&m)
		{
			x = integer_type(m.x);
			y = integer_type(m.y);
			return *this;
		}
		
		template <typename U>
		operator vector2i<U>() const
		{
			return vector2i<U>(U(x), U(y));
		}
		
		explicit operator Vector2d() const // conversion to phoenix::Vector2d
		{
			return Vector2d(x, y);
		}
		
		vector2i<diff_type> operator -() const
		{
			return vector2i<diff_type>(-x, -y);
		}
		
		inline size_type size() const {return size_type(std::abs(x) * std::abs(y));}
		inline vector2i abs()   const {return vector2i(std::abs(x), std::abs(y));}
		
		template <typename buf2T>
		inline size_type to_map_index(const buf2T *m_buf) const
		{
			/*if (x < 0 || y < 0)
				throw coords_out_of_bounds(detail::__out_of_bounds_reason_desc[out_of_bounds_negative_coord],
										   out_of_bounds_negative_coord);
			
			if (x >= m_buf->get_extent().x || y >= m_buf->get_extent().y)
				throw coords_out_of_bounds(detail::__out_of_bounds_reason_desc[out_of_bounds_outside_extent],
										   out_of_bounds_outside_extent);*/
		
			return size_type(x) + size_type(y) * m_buf->get_pitch();
		}
		
		template <typename buf2T>
		static inline vector2i from_map_index(size_type i, const buf2T *m_buf)
		{
			vector2i m_v(i % m_buf->get_pitch(), i / m_buf->get_pitch());
			
			/*if (m_v.x >= m_buf->get_extent().x || m_v.y >= m_buf->get_extent().y)
				throw coords_out_of_bounds(detail::__out_of_bounds_reason_desc[out_of_bounds_outside_extent],
										   out_of_bounds_outside_extent);*/
			return m_v;
		}
	};
	
	namespace detail
	{
		template <typename scalarT, typename scalarU>
		struct vector2i_result_type_selector;
		
		template <>
		struct vector2i_result_type_selector<int_type, int_type>
		{
			typedef int_type type;
			typedef int_type diff_type;
		};
		
		template <>
		struct vector2i_result_type_selector<int_type, size_type>
		{
			typedef int_type type;
			typedef int_type diff_type;
		};
		
		template <>
		struct vector2i_result_type_selector<size_type, int_type>
		{
			typedef int_type type;
			typedef int_type diff_type;
		};
		
		template <>
		struct vector2i_result_type_selector<size_type, size_type>
		{
			typedef size_type type;
			typedef int_type  diff_type;
		};
		
		template <typename scalarT, typename scalarU>
		struct _slct
		{
			typedef
				vector2i<typename detail::vector2i_result_type_selector<scalarT, scalarU>::type>
				vec_type;
			
			typedef
				vector2i<typename detail::vector2i_result_type_selector<scalarT, scalarU>::diff_type>
				diff_vec_type;
			
			typedef
				typename vec_type::integer_type
				scalar_type;
			
			typedef
				typename diff_vec_type::integer_type
				diff_type;
		};
	}
	
#define _UTD_VEC2I_OP_HELPER(scalarT, scalarU, _lt, _rt)                  \
	-> typename detail::_slct<scalarT, scalarU>::vec_type                          \
	{																	  \
		typedef typename detail::_slct<scalarT, scalarU>::vec_type vec_type;       \
		typedef typename detail::_slct<scalarT, scalarU>::scalar_type scalar_type; \
																		  \
		vec_type a(_lt);												  \
		vec_type b(_rt);												  

#define _UTD_VEC2I_DIFFOP_HELPER(scalarT, scalarU, _lt, _rt)              \
	-> typename detail::_slct<scalarT, scalarU>::diff_vec_type                     \
	{																	  \
		typedef typename detail::_slct<scalarT, scalarU>::diff_vec_type vec_type;  \
		typedef typename detail::_slct<scalarT, scalarU>::diff_type scalar_type;   \
																		  \
		vec_type a(_lt);												  \
		vec_type b(_rt);		

#define _UTD_VEC2I_BOOLOP_HELPER(scalarT, scalarU, _lt, _rt)              \
	-> bool																  \
	{																	  \
		typedef typename detail::_slct<scalarT, scalarU>::vec_type vec_type;       \
		typedef typename detail::_slct<scalarT, scalarU>::scalar_type scalar_type; \
																		  \
		vec_type a(_lt);												  \
		vec_type b(_rt);												  		

	template <typename scalarT, typename scalarU>
	inline auto operator +(vector2i<scalarT> _a, vector2i<scalarU> _b)
	_UTD_VEC2I_OP_HELPER(scalarT, scalarU, _a, _b)
	
		return vec_type(a.x + b.x, a.y + b.y);
	}

	template <typename scalarT, typename scalarU>
	inline auto operator *(vector2i<scalarT> _a, vector2i<scalarU> _b)
	_UTD_VEC2I_OP_HELPER(scalarT, scalarU, _a, _b)
	
		return vec_type(a.x * b.x, a.y * b.y);
	}
	
	template <typename scalarT, typename scalarU>
	inline auto operator -(vector2i<scalarT> _a, vector2i<scalarU> _b)
	_UTD_VEC2I_DIFFOP_HELPER(scalarT, scalarU, _a, _b)
	
		return vec_type(a.x - b.x, a.y - b.y);
	}
	
	template <typename scalarT, typename scalarU>
	inline auto max(vector2i<scalarT> _a, vector2i<scalarU> _b)
	_UTD_VEC2I_OP_HELPER(scalarT, scalarU, _a, _b)
	
		return vec_type(a.x < b.x ? b.x : a.x, a.y < b.y ? b.y : a.y);
	}
	
	template <typename scalarT, typename scalarU>
	inline auto min(vector2i<scalarT> _a, vector2i<scalarU> _b)
	_UTD_VEC2I_OP_HELPER(scalarT, scalarU, _a, _b)
	
		return vec_type(a.x > b.x ? b.x : a.x, a.y > b.y ? b.y : a.y);
	}
	
	template <typename scalarT, typename scalarU>
	inline auto operator ==(vector2i<scalarT> _a, vector2i<scalarU> _b)
	_UTD_VEC2I_BOOLOP_HELPER(scalarT, scalarU, _a, _b)
	
		return a.x == b.x && a.y == b.y;
	}
	
	template <typename scalarT, typename scalarU>
	inline auto operator !=(vector2i<scalarT> _a, vector2i<scalarU> _b)
	_UTD_VEC2I_BOOLOP_HELPER(scalarT, scalarU, _a, _b)
	
		return a.x != b.x || a.y != b.y;
	}

	template <typename scalarT, typename scalarU>
	inline vector2i<size_type> distance_v(vector2i<scalarT> _a, vector2i<scalarU> _b)
	{
		vector2i<int_type> a(int_type(_a.x), int_type(_a.y));
		vector2i<int_type> b(int_type(_b.x), int_type(_b.y));
		
		return vector2i<size_type>(size_type(a.x > b.x ? a.x - b.x : b.x - a.x),
								   size_type(a.y > b.y ? a.y - b.y : b.y - a.y));
	}
	
	template <typename scalarT, typename scalarU>
	inline vector2i<size_type> distance_s(vector2i<scalarT> _a, vector2i<scalarU> _b)
	{
		vector2i<int_type> a(int_type(_a.x), int_type(_a.y));
		vector2i<int_type> b(int_type(_b.x), int_type(_b.y));
		
		return size_type(a.x > b.x ? a.x - b.x : b.x - a.x) + size_type(a.y > b.y ? a.y - b.y : b.y - a.y);
	}
	
	template <typename scalarT>
	static inline std::ostream &operator <<(std::ostream &os, const vector2i<scalarT> &v)
	{
		return os << v.x << ',' << v.y;
	}
	
	template <typename scalarT>
	static inline std::istream &operator >>(std::istream &is, vector2i<scalarT> &v)
	{
		std::string s;
		if (!(is >> s))
			return is;
		
		std::string::size_type n = s.find(',');
	
		std::stringstream ex(s.substr(0,   n));
		std::stringstream wy(s.substr(n+1, std::string::npos));
		ex >> v.x; wy >> v.y;
		
		return is;
	}
	
	typedef vector2i<int_type>  coord;
	typedef vector2i<size_type> extent;
}

#endif
