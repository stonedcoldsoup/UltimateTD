#include "widgets.h"

namespace UTD
{
	tiled_widget_impl::~tiled_widget_impl() = default;
	
	widget_base::~widget_base() = default;

	void widget_base::on_focus(bool b_focus) {}
	void widget_base::on_lmb(bool b_down, const Vector2d &m_pos) {}
	void widget_base::on_mmb(bool b_down, const Vector2d &m_pos) {}
	void widget_base::on_rmb(bool b_down, const Vector2d &m_pos) {}
	void widget_base::on_move(const Vector2d &m_pos) {}
	void widget_base::on_key(phoenix::Key m_key, bool b_down, const Vector2d &m_pos) {}
	void widget_base::on_char(char c) {}
	void widget_base::on_scroll(int_type d) {}
	
	inline widget_base *widget_manager::get_target_widget(Vector2d &m_corrected_mouse_pos, bool b_grab_focus)
	{
		static widget_dummy __dummy_wgt;
		widget_base *m_wgt_result = &__dummy_wgt;
		
		if (!m_widgets.empty())
		{
			for (widget_base *m_wgt: m_widgets)
			{
				// get corrected pos returns true if the mouse is inside,
				// fills the corrected position with information (including
				// adding the offset).
				if (m_wgt->get_corrected_pos(m_mouse_pos, m_corrected_mouse_pos))
				{
					m_wgt_result = m_wgt;
					break;
				}
			}
			
			// switch focus!
			if (b_grab_focus && (m_wgt_result != &__dummy_wgt) && (m_wgt_result != m_widgets.front()))
			{
				m_widgets.front()->on_focus(false); // take focus from current
				m_widgets.remove(m_wgt_result);     // pull the result to the front of the list (making it top priority in focus search)
				m_widgets.push_front(m_wgt_result);
				m_wgt_result->on_focus(true); // give focus to the result
			}
		}
		
		return m_wgt_result;
	}
	
	void widget_manager::on_event(const WindowEvent& e)
	{
		if (!m_widgets.empty())
		{
			Vector2d m_corrected_mouse_pos;
			switch (e.type)
			{
			case WET_KEY:
				if (e.int_data > 15)
					// always sent to current focused widget
					m_widgets.front()->on_key(phoenix::Key(e.int_data), e.bool_data, m_corrected_mouse_pos);
				else
				{
					// find target widget and grab focus
					switch (e.int_data)
					{
					case PHK_MB_LEFT:
						get_target_widget(m_corrected_mouse_pos, true)->on_lmb(e.bool_data, m_corrected_mouse_pos);
						break;
					case PHK_MB_RIGHT:
						get_target_widget(m_corrected_mouse_pos, true)->on_rmb(e.bool_data, m_corrected_mouse_pos);
						break;
					case PHK_MB_MIDDLE:
						get_target_widget(m_corrected_mouse_pos, true)->on_mmb(e.bool_data, m_corrected_mouse_pos);
						break;
					};
				}
				
				break;
			case WET_CHAR:
				// always sent to current focused widget
				m_widgets.front()->on_char(e.int_data);
				break;
			case WET_MOUSE_POSITION:
				m_mouse_pos = e.vector_data;
				// find target widget and do not grab focus
				get_target_widget(m_corrected_mouse_pos)->on_move(m_corrected_mouse_pos);
				break;
			case WET_MOUSE_WHEEL:
				scroll_pos = e.int_data;
				// find target widget and grab focus
				get_target_widget(m_corrected_mouse_pos, true)->on_scroll(scroll_pos);
				break;
			default:
				// other events are not of our concern here.
				break;
			};
		}
	}
	
	widget_manager::widget_manager():
		m_mouse_pos(),
		scroll_pos(EventReceiver::Instance()->getMouseWheelPosition())
	{
		event_connection = WindowManager::Instance()->listen(boost::bind(&widget_manager::on_event, this, _1));
	}
	
	widget_manager::~widget_manager()
	{
		event_connection.disconnect();
	}
	
	void widget_manager::register_widget(widget_base *m_widget, bool b_grab_focus)
	{
		if (m_widgets.empty())
		{
			m_widgets.push_front(m_widget);
			m_widget->on_focus(true);
		}
		else if (b_grab_focus)
		{
			// switch focus
			m_widgets.front()->on_focus(false);
			m_widgets.push_front(m_widget);
			m_widget->on_focus(true);
		}
		else
			m_widgets.push_back(m_widget);
		
		m_widget->on_move(m_mouse_pos);
		m_widget->on_scroll(scroll_pos);
	}
	
	void widget_manager::unregister_widget(widget_base *m_widget)
	{
		if (m_widgets.front() == m_widget)
		{
			m_widget->on_focus(false);
			m_widgets.pop_front();
			if (!m_widgets.empty())
				m_widgets.front()->on_focus(true);
		}
		else
			m_widgets.remove(m_widget);
	}
	
	void widget_manager::update()
	{
		for (widget_base *m_widget: m_widgets)
			m_widget->update();
	}
	
	void widget_manager::draw()
	{
		for (auto it = m_widgets.rbegin(); it != m_widgets.rend(); ++it)
			(*it)->draw();
	}
}
