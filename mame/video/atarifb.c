/*************************************************************************

    Atari Football hardware

*************************************************************************/

#include "emu.h"
#include "includes/atarifb.h"


/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static void get_tile_info_common( running_machine &machine, tile_data &tileinfo, tilemap_memory_index tile_index, UINT8 *alpha_videoram )
{
	int code = alpha_videoram[tile_index] & 0x3f;
	int flip = alpha_videoram[tile_index] & 0x40;
	int disable = alpha_videoram[tile_index] & 0x80;

	if (disable)
		code = 0;   /* I *know* this is a space */

	SET_TILE_INFO(0, code, 0, (flip ? TILE_FLIPX | TILE_FLIPY : 0));
}


TILE_GET_INFO_MEMBER(atarifb_state::alpha1_get_tile_info)
{
	get_tile_info_common(machine(), tileinfo, tile_index, m_alphap1_videoram);
}


TILE_GET_INFO_MEMBER(atarifb_state::alpha2_get_tile_info)
{
	get_tile_info_common(machine(), tileinfo, tile_index, m_alphap2_videoram);
}


TILE_GET_INFO_MEMBER(atarifb_state::field_get_tile_info)
{
	int code = m_field_videoram[tile_index] & 0x3f;
	int flipyx = m_field_videoram[tile_index] >> 6;

	SET_TILE_INFO_MEMBER(1, code, 0, TILE_FLIPYX(flipyx));
}




/*************************************
 *
 *  Video RAM writes
 *
 *************************************/

WRITE8_MEMBER(atarifb_state::atarifb_alpha1_videoram_w)
{

	m_alphap1_videoram[offset] = data;
	m_alpha1_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(atarifb_state::atarifb_alpha2_videoram_w)
{

	m_alphap2_videoram[offset] = data;
	m_alpha2_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(atarifb_state::atarifb_field_videoram_w)
{

	m_field_videoram[offset] = data;
	m_field_tilemap->mark_tile_dirty(offset);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

void atarifb_state::video_start()
{

	m_alpha1_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(atarifb_state::alpha1_get_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 3, 32);
	m_alpha2_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(atarifb_state::alpha2_get_tile_info),this), TILEMAP_SCAN_COLS, 8, 8, 3, 32);
	m_field_tilemap  = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(atarifb_state::field_get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}



static void draw_playfield_and_alpha( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int playfield_x_offset, int playfield_y_offset )
{
	atarifb_state *state = machine.driver_data<atarifb_state>();
	const rectangle bigfield_area(4 * 8, 34 * 8 - 1, 0 * 8, 32 * 8 - 1);

	int scroll_x[1];
	int scroll_y[1];

	scroll_x[0] = - *state->m_scroll_register + 32 + playfield_x_offset;
	scroll_y[0] = 8 + playfield_y_offset;

	copybitmap(bitmap, state->m_alpha1_tilemap->pixmap(), 0, 0, 35*8, 1*8, cliprect);
	copybitmap(bitmap, state->m_alpha2_tilemap->pixmap(), 0, 0,  0*8, 1*8, cliprect);
	copyscrollbitmap(bitmap, state->m_field_tilemap->pixmap(),  1, scroll_x, 1, scroll_y, bigfield_area);
}


static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int gfx, int is_soccer )
{
	atarifb_state *state = machine.driver_data<atarifb_state>();
	const rectangle bigfield_area(4 * 8, 34 * 8 - 1, 0 * 8, 32 * 8 - 1);

	int obj;

	for (obj = 0; obj < 16; obj++)
	{
		int charcode;
		int flipx, flipy;
		int sx, sy;
		int shade = 0;

		sy = 255 - state->m_spriteram[obj * 2 + 1];
		if (sy == 255)
			continue;

		charcode = state->m_spriteram[obj * 2] & 0x3f;
		flipx = (state->m_spriteram[obj * 2] & 0x40);
		flipy = (state->m_spriteram[obj * 2] & 0x80);
		sx = state->m_spriteram[obj * 2 + 0x20] + 8 * 3;

		/* Note on Atari Soccer: */
		/* There are 3 sets of 2 bits each, where the 2 bits represent */
		/* black, dk grey, grey and white. I think the 3 sets determine the */
		/* color of each bit in the sprite, but I haven't implemented it that way. */
		if (is_soccer)
		{
			shade = ((state->m_spriteram[obj * 2 + 1 + 0x20]) & 0x07);

			drawgfx_transpen(bitmap, bigfield_area, machine.gfx[gfx + 1],
				charcode, shade,
				flipx, flipy, sx, sy, 0);

			shade = ((state->m_spriteram[obj * 2 + 1 + 0x20]) & 0x08) >> 3;
		}

		drawgfx_transpen(bitmap, bigfield_area, machine.gfx[gfx],
				charcode, shade,
				flipx, flipy, sx, sy, 0);

		/* If this isn't soccer, handle the multiplexed sprites */
		if (!is_soccer)
		{
			/* The down markers are multiplexed by altering the y location during */
			/* mid-screen. We'll fake it by essentially doing the same thing here. */
			if ((charcode == 0x11) && (sy == 0x07))
			{
				sy = 0xf1; /* When multiplexed, it's 0x10...why? */
				drawgfx_transpen(bitmap, bigfield_area, machine.gfx[gfx],
					charcode, 0,
					flipx, flipy, sx, sy, 0);
			}
		}
	}
}


UINT32 atarifb_state::screen_update_atarifb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_playfield_and_alpha(machine(), bitmap, cliprect, 0, 0);

	draw_sprites(machine(), bitmap, cliprect, 1, 0);

	return 0;
}


UINT32 atarifb_state::screen_update_abaseb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_playfield_and_alpha(machine(), bitmap, cliprect, -8, 0);

	draw_sprites(machine(), bitmap, cliprect, 1, 0);

	return 0;
}


UINT32 atarifb_state::screen_update_soccer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_playfield_and_alpha(machine(), bitmap, cliprect, 0, 8);

	draw_sprites(machine(), bitmap, cliprect, 2, 1);

	return 0;
}
