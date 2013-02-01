// TODO: refactor! ... widget system
//       on_focus returns true to bring to front
//       global depth layering stack is appropriately
//       incremented when drawing widgets.
//       widgets which implement drawing tilemaps
//       should push and pop depth priority before and after
//       tilemap renderer use.
//
//       improve! ... auto_tile_map, pattern_part
//       the code inside auto_tile_map does not need
//       refactoring, but the scheme does: get neighbors
//       for ALL tiles indiscriminantly.
//       store incrementally updated video map
//       given a tileset and pattern buffer.
//       pattern buffer should now also store a neighbor
//       map vector for empty tiles as well to suit
//       the new autotiling scheme expansion.
//       map2 class: store layers and batch each autotile
//       layer to a single tilemap renderer.  each raw
//       video layer is batched to it's own renderer, minus
//       the information overlay layers.  layers can
//       be user sorted and hidden and shown.  an active layer
//       will take editor input.
//       
//       optimize! ... atlas::image_factory
//       in the case of tilemaps, we are NO LONGER taking the
//       approach of minimum geometry construction.
//       Enough deep copying happens by design that the overhead
//       may actually be a bottleneck.  Add workaround for
//       atlas::image as a persistent geometry object by
//       adding static function atlas::image_factory::immediate(),
//       which destroys an image object, but not before making the
//       geometry immediate and then swapping it out, preventing
//       atlas::image from dropping it.  Should speed things up,
//       prevents any and all redundant list iteration client side
//       of PhoenixCore; letting the geometry batcher sort it all out.
//
//       socialize! ... Let Jon and Sven know about the critical bug in the
//       BMFont loader.  If they still haven't gotten around to helping in
//       a couple of months, implement a fix in the stonedcoldsoup fork.

#include <iostream>
#include "common.h"
#include "map2.h"
#include "map_render.h"
#include "editor_widgets.h"
#include "atlas.h"
#include "atlas_image.h"
#include "depth_layer.h"
#include "fismath.h"

using namespace UTD;
using namespace phoenix;

static constexpr float GLOBAL_FPS       = 60.0;
static constexpr float UPDATE_FREQUENCY = 1.0/60.0;

// GLOBALS BECAUSE I HATE KITTENS AND HAPPINESS
RenderSystem  g_rendersystem(Vector2d(800, 600), false);
Timer 		  g_timer;
//BitmapFontPtr g_font;

#define TILEWIDTH  48
#define TILEHEIGHT 48

#define MAPWIDTH  25
#define MAPHEIGHT 18

//#define MAPWIDTH  3
//#define MAPHEIGHT 3

widget_manager   		 m_editor_widgets;
pattern_part             m_test_pat;

auto_tile_map        m_automap(extent(MAPWIDTH, MAPHEIGHT));
video_tile_buf::bufm m_video_bufm(extent(MAPWIDTH, MAPHEIGHT));
map_renderer2        *m_map_renderer;

atlas::handle_type id_tiles_rgn = 0, id_tiles_tex = 0;

void cleanup()
{
	//g_font->drop();

	delete m_map_renderer;

	builtin_tileset::destroy();
	atlas::destroy();
}

void init()
{
	atlas::create(g_rendersystem);
	builtin_tileset::create();
	
	id_tiles_tex = atlas::instance()->create_texture("tiles2_0.png", false);
	if (id_tiles_tex)
	{
		atlas::tileset_def m_tileset;
		m_tileset.m_coords = coord();
		m_tileset.m_tile_extent = extent(16, 16);
		m_tileset.m_dim_extent = extent(8, 7);
		m_tileset.n_last_row = 2;
		
		id_tiles_rgn = atlas::instance()->create_region("tileset0", id_tiles_tex, m_tileset);
		//id_big_rgn   = atlas::instance()->create_region("tileset_all0", id_tiles_tex);
	}
	
	static constexpr uint8_t pat[11][11] =
	{
		{0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,1,1,1,0,0,0,0},
		{0,0,1,1,1,1,1,1,1,0,0},
		{0,0,1,1,0,0,0,1,1,0,0},
		{0,1,1,0,0,0,0,0,1,1,0},
		{0,1,1,0,0,0,0,0,1,1,0},
		{0,1,1,0,0,0,0,0,1,1,0},
		{0,0,1,1,0,0,0,1,1,0,0},
		{0,0,1,1,1,1,1,1,1,0,0},
		{0,0,0,0,1,1,1,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0}
	};
	
	m_automap.state_buf()->each_in
	(
		coord(),
		m_automap.get_extent(),
		[&] (coord m_coord, uint8_t &b)
		{
			b = pat[m_coord.x%11][m_coord.y%11];
		}
	);
	
	m_automap.update();
	
	m_video_bufm.each_in
	(
		coord(),
		m_video_bufm.get_extent(),
		[&] (coord m_coord, tile_index_type &i_tile)
		{
			i_tile = m_coord.size() % 250;
		}
	);
	
	map_metrics m_metrics(Vector2d(TILEWIDTH,TILEHEIGHT), coord(75,75), extent(800-150,600-150));
	m_map_renderer = new map_renderer2(m_metrics, id_tiles_rgn);
	
	//g_font = new BitmapFont(g_rendersystem);
	//g_font->load(g_rendersystem, "mfonts.fnt");
	//g_rendersystem.setFont();
}

static Vector2d m_map_offset = Vector2d(0,0);

void update_all()
{
	if (EventReceiver::Instance()->getKey(PHK_LEFT))  m_map_offset += Vector2d(-5, 0);
	if (EventReceiver::Instance()->getKey(PHK_RIGHT)) m_map_offset += Vector2d( 5, 0);
	
	if (EventReceiver::Instance()->getKey(PHK_UP))    m_map_offset += Vector2d( 0,-5);
	if (EventReceiver::Instance()->getKey(PHK_DOWN))  m_map_offset += Vector2d( 0, 5);
}

void draw_all()
{
	g_depth_layer_stack.reset();
	m_map_renderer->begin_layers(m_map_offset);
		//m_map_renderer->draw_video_layer(&m_video_bufm);
		m_map_renderer->push_depth();
			m_map_renderer->draw_background_layer(UTS_AUTOTILEEDIT_BG_TILE);
			m_map_renderer->push_depth();
				m_map_renderer->draw_state_layer(m_automap.state_buf());
				m_map_renderer->draw_neighbor_layer(m_automap.neighbor_buf());
			m_map_renderer->pop_depth();
		m_map_renderer->pop_depth();
	m_map_renderer->end_layers();
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
