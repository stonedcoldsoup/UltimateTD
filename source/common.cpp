#include "common.h"

namespace UTD
{
	/******************\
	|* SHELL LOG BASE *|
	\******************/
	
	/*// we have a concrete intermediate call so that a
	// concrete address can be given for the write
	// subscriber
	void log_stream::__do_write(const std::string &s)
	{
		write(s); // call the virtual writer
	}
	
	log_stream::log_stream():
		m_sink(this, &log_stream::__do_write)
	{}
	
	log_stream::~log_stream() = default;

	log_stream::source::source(log_stream &m_log, const std::string &m_source_name):
		b_moved(false),
		b_writing(false),
		m_source_name(m_source_name),
		m_log(&m_log),
		m_writer(m_log.m_sink.make_writer())
	{
		m_ss = new std::stringstream();
	}
	
	log_stream::source::~source()
	{
		delete m_ss;
	}
	
	log_stream::source::source(source &&m)
	{
		operator =(std::move(m));
	}
	
	log_stream::source &log_stream::source::operator =(source &&m)
	{
		if (!m.b_moved)
		{
			m_writer = std::move(m.m_writer);
			m_source_name = std::move(m.m_source_name);
			
			auto *p = m_ss;
			m_ss = m.m_ss;
			m.m_ss = p;
			
			auto *q = m_log;
			m_log = m.m_log;
			m.m_log = q;
			
			b_writing = m.b_writing;
			m.b_writing = false;
			m.b_moved = true;
		}
		
		return *this;
	}
	
		  log_stream &log_stream::source::log()         {return *m_log;}
	const log_stream &log_stream::source::log() const   {return *m_log;}
	
	const std::string &log_stream::source::name() const {return m_source_name;}
	
	log_stream::source &operator <<(log_stream::source &m_src, log_stream::__marker__start_line __marker_inst)
	{
		if (!m_src.b_moved)
		{
			if (!m_src.b_writing)
			{
				*m_src.m_ss << m_src.m_source_name << ": ";
				m_src.b_writing = true;
			}
		}
		
		return m_src;
	}
	
	log_stream::source &operator <<(log_stream::source &m_src, log_stream::__marker__end_line __marker_inst)
	{
		if (!m_src.b_moved)
		{
			if (m_src.b_writing)
			{
				time_t m_rawt;
				time(&m_rawt);

				std::string s = ctime(&m_rawt);
				s.erase(s.size() - 1);

				*m_src.m_ss << " <" << s << ">\n";
				
				// flush to log and reset stream.
				m_src.m_writer(m_src.m_ss->str());
				m_src.m_ss->str(std::string());
				m_src.b_writing = false;
			}
		}
		
		return m_src;
	}
	
	log_stream::source &operator <<(log_stream::source &m_src, const util::any_type &m_object)
	{
		if (!m_src.b_moved)
		{
			if (m_src.b_writing)
				*m_src.m_ss << m_object;
		}

		return m_src;
	}*/
	
	/************************************\
	|* SHELL LOG OSTREAM IMPLEMENTATION *|
	\************************************/
	
	/*stl_log_stream::stl_log_stream():
		log_stream(),
		p_os(&std::cout)
	{}
	
	stl_log_stream::stl_log_stream(std::ostream &m_os):
		log_stream(),
		p_os(&m_os)
	{}

	inline std::ostream &stl_log_stream::ros()
	{
		return *p_os;
	}
	
	void stl_log_stream::write(const std::string &s)
	{
		ros() << s;
	}
	
	stl_log_stream::stl_log_stream(stl_log_stream &&m)
	{
		operator =(std::move(m));
	}
	
	stl_log_stream &stl_log_stream::operator =(stl_log_stream &&m)
	{
		std::ostream *tmp = p_os;
		p_os = m.p_os;
		m.p_os = tmp;
		
		return *this;
	}*/
}
