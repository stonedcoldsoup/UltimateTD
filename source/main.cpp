// TODO: do clipping and window position limiting for map_compositor.

#include <iostream>
#include "common.h"
#include "map2.h"
#include "map_render.h"
#include "editor_widgets.h"
#include "atlas.h"
#include "atlas_image.h"

using namespace UTD;
using namespace phoenix;

static constexpr float GLOBAL_FPS       = 60.0;
static constexpr float UPDATE_FREQUENCY = 1.0/60.0;

// GLOBALS BECAUSE I HATE KITTENS AND HAPPINESS
RenderSystem g_rendersystem(Vector2d(800, 600), false);
Timer 		 g_timer;

#define TILEWIDTH  48
#define TILEHEIGHT 48

#define MAPWIDTH  25
#define MAPHEIGHT 18

//#define MAPWIDTH  3
//#define MAPHEIGHT 3

widget_manager   		 m_editor_widgets;
pattern_part             m_test_pat;

auto_tile_map     m_automap(extent(MAPWIDTH, MAPHEIGHT));
neighbor_renderer m_neighbor_renderer(g_rendersystem.getGraphicsFactory(), Vector2d(TILEWIDTH, TILEHEIGHT));
state_renderer    m_state_renderer(g_rendersystem.getGraphicsFactory(), Vector2d(TILEWIDTH, TILEHEIGHT));

TexturePtr g_tileset_texture;
TexturePtr g_ground_texture;
TexturePtr g_base_texture;
TexturePtr g_spawn_texture;

const uint8_t m_pattern[10*12] = 
{
	0,0,0,0,0,0,0,0,0,0,
	0,1,1,0,0,0,0,1,1,0,
	0,1,1,1,0,0,1,1,1,0,
	0,0,1,1,0,0,1,1,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,0,
	0,0,1,1,0,0,1,1,0,0,
	0,1,1,1,0,0,1,1,1,0,
	0,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,0,0
};

video_tile_buf::bufm m_video_bufm(extent(MAPWIDTH, MAPHEIGHT));
map_compositor *m_map_compositor;
pattern_part_edit_widget *m_pat_edit_test;

void cleanup()
{
	delete m_pat_edit_test;
	delete m_map_compositor;
	builtin_tileset::destroy();
	atlas::destroy();
}

void init()
{
	atlas::create(g_rendersystem);
	builtin_tileset::create();
	
	m_pat_edit_test = new pattern_part_edit_widget(m_test_pat, Vector2d(50, 50), Vector2d(TILEWIDTH,TILEHEIGHT));

	atlas::handle_type id_tiles_rgn = 0, id_big_rgn = 0, id_tiles_tex = atlas::instance()->create_texture("tiles2_0.png", false);
	
	if (id_tiles_tex)
	{
		atlas::tileset_def m_tileset;
		m_tileset.m_coords = coord();
		m_tileset.m_tile_extent = extent(16, 16);
		m_tileset.m_dim_extent = extent(8, 7);
		m_tileset.n_last_row = 2;
		
		id_tiles_rgn = atlas::instance()->create_region("tileset0", id_tiles_tex, m_tileset);
		id_big_rgn   = atlas::instance()->create_region("tileset_all0", id_tiles_tex);
	}
	
	map_metrics m_metrics(Vector2d(TILEWIDTH, TILEHEIGHT), coord(75, 75), extent(800-150,600-150));
	m_map_compositor = new map_compositor(m_metrics);
	m_map_compositor->register_video_layer(&m_video_bufm, id_tiles_rgn, UTS_AUTOTILEEDIT_MISSING_TILE);
	
	m_video_bufm.each_in
	(
		coord(), m_video_bufm.get_extent(),
		[&] (coord m_coord, tile_index_type &i_tile)
		{
			tile_index_type i = m_coord.size() % 10 - 2;
			i_tile = i == -2 ? 2000 : i;
		}
	);

	m_editor_widgets.register_widget(m_pat_edit_test);
	//g_tileset_texture = g_rendersystem.loadTexture("tiles.png", false);

	m_automap.state_buf()->fill(0);
	for (size_type x = 0; x < 10; ++x)
	{
		for (size_type y = 0; y < 12; ++y)
			m_automap.state_buf()->set(x, y, m_pattern[x + y * 10]);
	}
	logic_tile_buf::paste(m_automap.state_buf(), m_automap.state_buf(), coord(10, 0));
	logic_tile_buf::paste(m_automap.state_buf(), m_automap.state_buf(), coord(20, 0));
	logic_tile_buf::paste(m_automap.state_buf(), m_automap.state_buf(), coord(0, 12));
	
	/*g_bufl.set_active_layer(0);
	g_bufl.fill(1);
	g_bufl.set(coord(0,0), 0);
	g_bufl.set(coord(0,2), 0);
	g_bufl.set(coord(1,1), 0);
	g_bufl.set(coord(2,0), 0);
	g_bufl.set(coord(2,2), 0);*/
	
	m_automap.update();
}

void update_all()
{
	// EVIL!
	static float t = 0.0f;
	
	t+=0.1;
	
	Vector2d m_chg(0,0);
	
	if (EventReceiver::Instance()->getKey(PHK_LEFT))  m_chg += Vector2d(-5, 0);
	if (EventReceiver::Instance()->getKey(PHK_RIGHT)) m_chg += Vector2d( 5, 0);
	
	if (EventReceiver::Instance()->getKey(PHK_UP))    m_chg += Vector2d( 0,-5);
	if (EventReceiver::Instance()->getKey(PHK_DOWN))  m_chg += Vector2d( 0, 5);
	
	m_map_compositor->set_offset(m_map_compositor->get_offset() + m_chg);
	
	/*m_tileset_image->set_position(coord(200.0f+50.0f*cosf(t), 200.0f+50.0f*sinf(t)));
	m_tileset_image->set_rotation(-t);
	m_tileset_image->set_scale(Vector2d(1.0f+0.75f*cosf(t), 1.0f+0.75f*sinf(t)));
	m_tileset_image->update();*/

	m_editor_widgets.update();
}

void draw_all()
{
	m_editor_widgets.draw();
	m_map_compositor->draw();
	//m_video_buf_renderer->draw(&m_video_bufm, Vector2d(0,0), coord(), extent(12, 12));
	
	//m_tileset_factory->create();

	/*// draw grid lines
	for (size_type i = 0; i < MAPWIDTH+1; ++i)
	{
		g_rendersystem.drawLine(Vector2d(i * TILEWIDTH, 0), Vector2d(i * TILEWIDTH, TILEHEIGHT*MAPHEIGHT),
		                        Color(128,128,255,128), Color(128,128,255,128));
	}
	
	for (size_type i = 0; i < MAPHEIGHT+1; ++i)
	{
		g_rendersystem.drawLine(Vector2d(0, i * TILEHEIGHT), Vector2d(TILEWIDTH*MAPWIDTH, i * TILEHEIGHT),
		                        Color(128,128,255,128), Color(128,128,255,128));
	}
	
	m_state_renderer.draw(m_automap.state_buf(), Vector2d(), coord(), m_automap.state_buf()->get_extent());
	m_neighbor_renderer.draw(m_automap.neighbor_buf(), Vector2d(), coord(), m_automap.neighbor_buf()->get_extent());*/

	/*for (int i = 0; i < 4; ++i)
	{
		draw_tile(i, Vector2d(i * TILEWIDTH * SCALEFACTOR, 0));
		draw_tile(i, Vector2d(i * TILEWIDTH * SCALEFACTOR,16 * SCALEFACTOR));
		draw_tile(i, Vector2d(i * TILEWIDTH * SCALEFACTOR,32 * SCALEFACTOR));
		draw_tile(i, Vector2d(i * TILEWIDTH * SCALEFACTOR,48 * SCALEFACTOR));
		
		draw_tile(i, Vector2d(i * TILEWIDTH * SCALEFACTOR + TILEWIDTH * SCALEFACTOR * 4, 0));
		draw_tile(i, Vector2d(i * TILEWIDTH * SCALEFACTOR + TILEWIDTH * SCALEFACTOR * 4,16 * SCALEFACTOR));
		draw_tile(i, Vector2d(i * TILEWIDTH * SCALEFACTOR + TILEWIDTH * SCALEFACTOR * 4,48 * SCALEFACTOR));
		draw_tile(i, Vector2d(i * TILEWIDTH * SCALEFACTOR + TILEWIDTH * SCALEFACTOR * 4,32 * SCALEFACTOR));
	}*/
}

int main()
{
	init();

	bool b_running = true;
	float t_accum = 0.0f;

	g_timer.start();
	while (b_running)
	{
		t_accum += (float)g_timer.getTime();
		g_timer.reset();
		
		bool b_redraw = false;
		if (t_accum >= UPDATE_FREQUENCY)
		{
			do // yeah, this is a do-while. deal with it.
			{
				t_accum -= UPDATE_FREQUENCY;
				update_all();
			}
			while (t_accum >= UPDATE_FREQUENCY);
			b_redraw = true;
		}
		
		if (b_redraw)
		{
			if (!g_rendersystem.run())
				b_running = false;
		
			draw_all();
			b_redraw = false;
		}
	}
	
	cleanup();
	return 0;
}
