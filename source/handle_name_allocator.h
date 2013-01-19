#ifndef UTD_HANDLE_NAME_ALLOCATOR_H
#define UTD_HANDLE_NAME_ALLOCATOR_H

#include "common.h"
#include "handle_recycler.h"

namespace UTD
{	
	template <typename dataT, typename handleT, handleT _handle_base = handleT(0) >
	class handle_name_allocator
	{
	public:
		typedef
			dataT
			data_type;
		
		typedef
			handleT
			handle_type;
			
		struct data_cont
		{
			std::string m_name;
			handle_type id;
			data_type m_data;
			bool b_alloc;
		};
			
		typedef
			util::hash_map<util::default_hash<std::string, data_cont *, util::hash_entry_ptr_no_delete>>
			data_map;
	private:
		typedef
			std::vector<data_cont>
			data_vector;
			
		data_map    m_data_map;
		data_vector m_data;
		
		handle_recycler<handle_type, _handle_base> m_handles;
		
		static inline void resize_vec(data_vector &m_vec, handle_type id)
		{
			if (size_t(id) >= m_vec.size())
			{
				size_t old_siz = m_vec.size();
				m_vec.resize(util::next_pow2(id+1));
				for (auto it = m_vec.begin()+old_siz; it != m_vec.end(); ++it)
					it->b_alloc = false;
			}
		}
		
		data_cont *_find_impl(const std::string &m_name)
		{
			auto it = m_data_map.find(m_name);
			return it != m_data_map.end() ?
				   it->value : nullptr;
		}
		
		data_cont *_find_impl(handle_type id)
		{
			if (size_t(id) < m_data.size())
				return m_data[id].b_alloc ?
					   m_data.data()+id :
					   nullptr;
			else
				return nullptr;
		}
		
		inline const data_cont *_find_impl(const std::string &m_name) const
		{
			auto it = m_data_map.find(m_name);
			return it != m_data_map.end() ?
				   it->value : nullptr;
		}
		
		inline const data_cont *_find_impl(handle_type id) const
		{
			if (size_t(id) < m_data.size())
				return m_data[id].b_alloc ?
					   m_data.data()+id :
					   nullptr;
			else
				return nullptr;
		}
	public:
		// constant iterators only; screwing with the internal
		// bookkeeping would be a disaster
		auto begin() const -> decltype(m_data_map.begin()) {return m_data_map.begin();}
		auto end()   const -> decltype(m_data_map.end())   {return m_data_map.end();}
	
		template <typename... argT>
		inline handle_type alloc(const std::string &m_name, argT&&... argV)
		{
			handle_type id = m_handles.get();
			resize_vec(m_data, id);
			
			data_cont m_cont = data_cont{m_name, id, data_type(std::forward<argT>(argV)...), true};
			m_data.emplace(m_data.begin()+id, std::move(m_cont));
			m_data_map.insert(typename data_map::entry(m_name, m_data.data() + id));
			return id;
		}
		
		inline data_type *find(const std::string &m_name)
		{
			data_cont *m_cont = _find_impl(m_name);
			return m_cont ? &m_cont->m_data : nullptr;
		}
		
		inline data_type *find(handle_type id)
		{
			data_cont *m_cont = _find_impl(id);
			return m_cont ? &m_cont->m_data : nullptr;
		}
		
		inline const data_type *find(const std::string &m_name) const
		{
			const data_cont *m_cont = _find_impl(m_name);
			return m_cont ? &m_cont->m_data : nullptr;
		}
		
		inline const data_type *find(handle_type id) const
		{
			const data_cont *m_cont = _find_impl(id);
			return m_cont ? &m_cont->m_data : nullptr;
		}
		
		inline handle_type find_id(const std::string &m_name) const
		{
			const data_cont *m_cont = _find_impl(m_name);
			return m_cont ? m_cont->id : 0;
		}
		
		inline const std::string &find_name(handle_type id) const
		{
			static const std::string __dummy = std::string();
			const data_cont *m_cont = _find_impl(id);
			return m_cont ? m_cont->m_name : __dummy;
		}
		
		inline bool free(const std::string &m_name)
		{
			auto it = m_data_map.find(m_name);
			if (it != m_data_map.end())
			{
				it->value->b_alloc = false;
				m_handles.drop(it->value->id);
				m_data_map.erase(it);
				return true;
			}
			else
				return false;
		}
		
		inline bool free(handle_type id)
		{
			if (size_t(id) < m_data.size() && m_data[id].b_alloc)
			{
				m_data[id].b_alloc = false;
				m_handles.drop(id);
				m_data_map.erase(m_data[id].m_name);
				return true;
			}
			else
				return false;
		}
		
		void recycle()
		{
			m_handles.recycle();
		}
	};
}

#endif
