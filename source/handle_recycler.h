#ifndef UTD_HANDLE_RECYCLER_H
#define UTD_HANDLE_RECYCLER_H

#include "common.h"
#include <stack>

namespace UTD
{
	template <typename idT, idT _handle_base = idT(0) >
	class handle_recycler
	{
	private:
		std::stack<idT> m_recycle;
		std::stack<idT> m_free;
		idT _next;
	
	public:
		handle_recycler():
			_next(0)
		{}
		
		void recycle()
		{
			while (!m_recycle.empty())
			{
				m_free.push(m_recycle.top());
				m_recycle.pop();
			}
		}
		
		idT get()
		{
			idT result;
			if (!m_free.empty())
			{
				result = m_free.top();
				m_free.pop();
			}
			else
				result = ++_next;
			
			return result + _handle_base;
		}
		
		void drop(idT id)
		{
			m_recycle.push(id - _handle_base);
		}
	};
}

#endif
