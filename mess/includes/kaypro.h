#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "machine/com8116.h"
#include "machine/ctronics.h"
#include "machine/wd17xx.h"
#include "formats/basicdsk.h"
#include "imagedev/snapquik.h"
#include "imagedev/flopdrv.h"
#include "sound/beep.h"
#include "video/mc6845.h"

struct kay_kbd_t;

class kaypro_state : public driver_device
{
public:
	kaypro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_pio_g(*this, "z80pio_g"),
	m_pio_s(*this, "z80pio_s"),
	m_sio(*this, "z80sio"),
	m_sio2x(*this, "z80sio_2x"),
	m_centronics(*this, "centronics"),
	m_fdc(*this, "wd1793"),
	m_crtc(*this, "crtc"),
	m_beep(*this, BEEPER_TAG),
	m_p_videoram(*this, "p_videoram"){ }

	required_device<cpu_device> m_maincpu;
	optional_device<z80pio_device> m_pio_g;
	optional_device<z80pio_device> m_pio_s;
	required_device<z80sio_device> m_sio;
	optional_device<z80sio_device> m_sio2x;
	required_device<centronics_device> m_centronics;
	required_device<fd1793_device> m_fdc;
	optional_device<mc6845_device> m_crtc;
	required_device<beep_device> m_beep;
	DECLARE_READ8_MEMBER(kaypro2x_87_r);
	DECLARE_READ8_MEMBER(kaypro2x_system_port_r);
	DECLARE_READ8_MEMBER(kaypro2x_status_r);
	DECLARE_READ8_MEMBER(kaypro2x_videoram_r);
	DECLARE_WRITE8_MEMBER(kaypro2x_system_port_w);
	DECLARE_WRITE8_MEMBER(kaypro2x_index_w);
	DECLARE_WRITE8_MEMBER(kaypro2x_register_w);
	DECLARE_WRITE8_MEMBER(kaypro2x_videoram_w);
	DECLARE_READ8_MEMBER(pio_system_r);
	DECLARE_WRITE8_MEMBER(common_pio_system_w);
	DECLARE_WRITE8_MEMBER(kayproii_pio_system_w);
	DECLARE_WRITE8_MEMBER(kaypro4_pio_system_w);
	DECLARE_WRITE_LINE_MEMBER(kaypro_fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(kaypro_fdc_drq_w);
	DECLARE_READ8_MEMBER(kaypro_videoram_r);
	DECLARE_WRITE8_MEMBER(kaypro_videoram_w);
	const UINT8 *m_p_chargen;
	required_shared_ptr<UINT8> m_p_videoram;
	UINT8 m_system_port;
	UINT8 m_mc6845_cursor[16];
	UINT8 m_mc6845_reg[32];
	UINT8 m_mc6845_ind;
	UINT8 m_speed;
	UINT8 m_flash;
	UINT8 m_framecnt;
	UINT16 m_cursor;
	UINT16 m_mc6845_video_address;
	kay_kbd_t *m_kbd;
	void mc6845_cursor_configure();
	void mc6845_screen_configure();
	DECLARE_MACHINE_START(kayproii);
	DECLARE_MACHINE_RESET(kayproii);
	DECLARE_VIDEO_START(kaypro);
	DECLARE_MACHINE_RESET(kaypro2x);
	DECLARE_PALETTE_INIT(kaypro);
	DECLARE_MACHINE_RESET(kay_kbd);
	UINT32 screen_update_kayproii(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_kaypro2x(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_omni2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(kay_kbd_interrupt);
	TIMER_CALLBACK_MEMBER(kaypro_timer_callback);
	DECLARE_WRITE_LINE_MEMBER(kaypro_interrupt);
	DECLARE_READ8_MEMBER(kaypro_sio_r);
	DECLARE_WRITE8_MEMBER(kaypro_sio_w);
};


/*----------- defined in machine/kay_kbd.c -----------*/

UINT8 kay_kbd_c_r( running_machine &machine );
UINT8 kay_kbd_d_r( running_machine &machine );
void kay_kbd_d_w( running_machine &machine, UINT8 data );

INPUT_PORTS_EXTERN( kay_kbd );


/*----------- defined in machine/kaypro.c -----------*/

extern const z80pio_interface kayproii_pio_g_intf;
extern const z80pio_interface kayproii_pio_s_intf;
extern const z80pio_interface kaypro4_pio_s_intf;
extern const z80sio_interface kaypro_sio_intf;
extern const wd17xx_interface kaypro_wd1793_interface;


QUICKLOAD_LOAD( kayproii );
QUICKLOAD_LOAD( kaypro2x );

/*----------- defined in video/kaypro.c -----------*/

MC6845_UPDATE_ROW( kaypro2x_update_row );
