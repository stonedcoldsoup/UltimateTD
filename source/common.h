#ifndef UTD_COMMON_H
#define UTD_COMMON_H

#ifndef BOOST_THREAD_USE_LIB
#define BOOST_THREAD_USE_LIB
#endif

#include <Phoenix.h>

#define FUNGUSUTIL_HAVE_gettimeofday
#include <fungus_booster/fungus_booster.h>

#include <functional>

namespace UTD
{
	using namespace phoenix;

	// helpful namespace aliases
	namespace util = fungus_util;
	namespace conc = fungus_concurrency;
	namespace net  = fungus_net;
	
	namespace detail
	{
		template <typename containerT, typename... infoT>
		class list_unroller;
		
		template <typename containerT, typename infoT, typename... infoU>
		class list_unroller<containerT, infoT, infoU...>
		{
		public:
			static inline void __build_list(containerT &m, const infoT &infoV, infoU&&... infoW)
			{
				m.push_back(infoV);
				list_unroller<containerT, infoU...>::__build_list(m, std::forward<infoU>(infoW)...);
			}
		};
		
		template <typename containerT>
		class list_unroller<containerT>
		{
		public:
			static inline void __build_list(containerT &m) {}
		};
		
		template <typename containerT, typename pairT, typename... strT>
		class info_name_unroller;
		
		template <typename containerT, typename pairT, typename strT, typename... strU>
		class info_name_unroller<containerT, pairT, strT, strU...>
		{
		public:
			static inline void __build_list(containerT &m, const strT &strV, strU&&... strW)
			{
				m.push_back(pairT(strV, util::any_type()));
				info_name_unroller<containerT, pairT, strU...>::__build_list(m, std::forward<strU>(strW)...);
			}
		};
		
		template <typename containerT, typename pairT>
		class info_name_unroller<containerT, pairT>
		{
		public:
			static inline void __build_list(containerT &m) {}
		};
		
		template <typename containerT, typename... valueT>
		class info_value_unroller_impl;
		
		template <typename containerT, typename valueT, typename... valueU>
		class info_value_unroller_impl<containerT, valueT, valueU...>
		{
		public:
			static inline containerT &__build_list(size_t i, containerT &m, const valueT &valueV, valueU&&... valueW)
			{
				if (i < m.size())
				{
					m[i].second = valueV;
					(void)info_value_unroller_impl<containerT, valueU...>::__build_list(++i, m, std::forward<valueU>(valueW)...);
				}
				return m;
			}
		};
		
		template <typename containerT>
		class info_value_unroller_impl<containerT>
		{
		public:
			static inline containerT &__build_list(size_t i, containerT &m) {return m;}
		};
		
		template <typename containerT, typename... valueT>
		class info_value_unroller
		{
		public:
			static inline containerT &__build_list(containerT &m, valueT&&... valueV)
			{
				return info_value_unroller_impl<containerT, valueT...>::__build_list(0, m, std::forward<valueT>(valueV)...);
			}
		};
	}
	
	class exception:
		public std::exception
	{
	public:
		typedef
			uint32_t
			id_type;
			
		static constexpr id_type nil_id = id_type(-1);
	
		typedef
			std::pair<std::string, util::any_type>
			info_type;
	private:
		struct data
		{
			std::string m_what;
			std::vector<info_type> m_info;
			id_type m_id;
		};
		
		std::list<data> m_data;
	
		template <typename... infoT>
		struct info_list_unroller:
			detail::list_unroller<std::vector<info_type>, infoT...>
		{};
		
		exception(id_type m_id, const std::string &m_what, const std::vector<info_type> &m_info)
		{
			data m_datum;
			m_datum.m_what = m_what;
			m_datum.m_id   = m_id;
			std::copy(m_info.begin(), m_info.end(), std::back_inserter(m_datum.m_info));
			
			m_data.push_back(std::move(m_datum));
		}
		
		exception(exception &prev, id_type m_id, const std::string &m_what, const std::vector<info_type> &m_info)
		{
			data m_datum;
			m_datum.m_what = m_what;
			m_datum.m_id   = m_id;
			std::copy(m_info.begin(), m_info.end(), std::back_inserter(m_datum.m_info));
			
			m_data.push_front(std::move(m_datum));
			m_data.splice(m_data.end(), prev.m_data);
		}
	public:
		class factory
		{
		private:
			std::string 		           m_what;
			id_type       		           m_id;
			mutable std::vector<info_type> m_info;
			
			template <typename... nameT>
			class name_unroller:
				public detail::info_name_unroller<std::vector<info_type>, info_type, nameT...>
			{};
			
			template <typename... T>
			class value_unroller:
				public detail::info_value_unroller<std::vector<info_type>, T...>
			{};
		public:
			template <typename... nameT>
			factory(id_type m_id, const std::string &m_what, nameT&&... nameV):
				m_what(m_what),
				m_id(m_id)
			{
				name_unroller<nameT...>::__build_list(m_info, std::forward<nameT>(nameV)...);
			}
			
			template <typename... infoT>
			inline exception operator()(exception &_prev, infoT&&... infoV) const throw()
			{
				return exception(_prev, m_id, m_what, value_unroller<infoT...>::__build_list(m_info, std::forward<infoT>(infoV)...));
			}
			
			template <typename... infoT>
			inline exception operator()(infoT&&... infoV) const throw()
			{
				return exception(m_id, m_what, value_unroller<infoT...>::__build_list(m_info, std::forward<infoT>(infoV)...));
			}
		};
	
		exception(const exception &e):
			m_data(e.m_data)
		{}
		
		exception(exception &&e):
			m_data(std::move(e.m_data))
		{}
		
		exception &operator =(const exception &e)
		{
			std::copy(e.m_data.begin(), e.m_data.end(), std::back_inserter(m_data));
			return *this;
		}
		
		exception &operator =(exception &&e)
		{
			m_data.swap(e.m_data);
			return *this;
		}
	
		virtual ~exception() throw() {}
		
		virtual const char *what() const throw()
		{
			if (m_data.empty())
				return "";
			else
				return m_data.front().m_what.c_str();
		}
		
		inline std::vector<info_type> info() const throw()
		{
			if (m_data.empty())
				return std::vector<info_type>();
			else
				return m_data.front().m_info;
		}
		
		inline id_type id() const throw()
		{
			if (m_data.empty())
				return nil_id;
			else
				return m_data.front().m_id;
		}
		
		// returns empty object on failure
		inline util::any_type find_info(const std::string &m_key) const throw()
		{
			if (!m_data.empty())
			{
				for (const info_type &m_pair: m_data.front().m_info)
				{
					if (m_pair.first == m_key)
						return m_pair.second;
				}
			}
			return util::any_type();
		}
		
		inline bool empty() const throw() {return m_data.empty();}
		
		// return false when no data is left.
		inline bool pop() throw()
		{
			if (!m_data.empty())
				m_data.pop_front();

			return !m_data.empty();
		}
		
		friend inline std::ostream &operator <<(std::ostream &m_os, const exception &_e);
	};
	
	inline std::ostream &operator <<(std::ostream &m_os, const UTD::exception &_e)
	{
		UTD::exception e(_e);
		m_os << "EXCEPTION" << std::endl;
		while (!e.empty())
		{
			m_os << " ... caused by ..." << std::endl
				 << "    what=" << e.what() << std::endl;
			
			for (const auto &m_info: e.info())
				m_os << "    " << m_info.first << " = " << m_info.second << std::endl;
			
			e.pop();
		}
		
		return m_os;
	}
	
#define UTD_EXCEPTION_TYPE(FACTORY_, ID_, WHAT_, NAMES_...) \
	static const ::UTD::exception::factory FACTORY_ = ::UTD::exception::factory(ID_, WHAT_, ##NAMES_);
	
	class log_stream
	{
	private:
		typedef
			util::sink_subscriber<log_stream, const std::string &>
			sink_type;
	
		sink_type m_sink;
		
		void __do_write(const std::string &s);

	protected:
		virtual void write(const std::string &s) = 0;
		
	public:
		// control markers
		struct __marker__start_line {};
		struct __marker__end_line   {};
		
		static constexpr __marker__start_line start_line = __marker__start_line();
		static constexpr __marker__end_line   end_line   = __marker__end_line();
	
		class source
		{
		private:
			bool 					b_moved;
			bool 					b_writing;
			std::stringstream       *m_ss;
			std::string             m_source_name;
			log_stream              *m_log;
			sink_type::writer_type  m_writer;
		public:
			source(log_stream &m_log, const std::string &m_source_name = "Log");
			~source();
			
			source(const source &m) = delete;
			source(source &&m);
			
			source &operator =(const source &m) = delete;
			source &operator =(source &&m);
			
			      log_stream &log();
			const log_stream &log() const;
			
			const std::string &name() const;
			
			// stream markers
			friend source &operator <<(source &m_src, __marker__start_line __marker_inst);
			friend source &operator <<(source &m_src, __marker__end_line   __marker_inst);
			
			// default fungus_booster any_type object writer
			friend source &operator <<(source &m_src, const util::any_type &m_object);
		};
	
		log_stream();
		virtual ~log_stream();
		
		log_stream(log_stream &&m) = delete;
		log_stream(const log_stream &m) = delete;
		
		log_stream &operator =(log_stream &&m) = delete;
		log_stream &operator =(const log_stream &m) = delete;
	};
	
	class stl_log_stream:
		public log_stream
	{
	private:
		std::ostream *p_os;
		
		inline std::ostream &ros();
	protected:
		virtual void write(const std::string &s);
	public:
		stl_log_stream(); // default constructor 
		stl_log_stream(std::ostream &m_os);
		
		stl_log_stream(stl_log_stream &&m);
		stl_log_stream &operator =(stl_log_stream &&m);
		
		stl_log_stream(const stl_log_stream &m) = delete;
		stl_log_stream &operator =(const stl_log_stream &m) = delete;
	};
	
	typedef
		int16_t
		int_type;
	
	typedef
		uint16_t
		size_type;
	
	typedef
		int32_t
		size_diff_type;
	
	typedef
		uint8_t
		logical_type;
	
	typedef
		int8_t
		char_type;
}

#endif
