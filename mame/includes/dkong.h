#include "sound/discrete.h"

/*
 * From the schematics:
 *
 * XTAL is 61,44 MHZ. There is some oscillator logic around it. The oscillating circuit
 * transfers the signal with a transformator. Onwards, it is fed through a M(B/C)10136. This
 * is a programmable counter which is used as a divisor by 5.
 * Cascaded 74LS161 further divide the signal. The following signals are generated:
 * 1/2H: 61,44MHZ/5/2 - pixel clock
 * 1H  : 61,44MHZ/5/4 - cpu-clock
 * 2H  : 61,44MHZ/5/8
 * ....
 * 128H: 61,44MHZ/5/512
 * The horizontal circuit counts till 384=256+128, thus 256H only being high for 128H/2
 *
 * Signal 16H,32H,64H and 256H are combined using a LS00, LS04 and a D-Flipflop to produce
 * a signal with Freq 16H/12. This is only possible because a 220pf capacitor with the
 * impedance of the LS-Family of 10K delays the 16H signal by about half a cycle.
 * This signal is divided by two by another D-Flipflop(74LS74) to give:
 * 1VF: 61,44MHZ/5/64/12/2 = 8KHZ
 * 2VF: 1VF/2 - Noise frequency: 4Khz
 * ...
 * The vertical circuit counts from 248 till 512 giving 264 lines.
 * 256VF is not being used, so counting is from 248...255, 0...255, ....
 */

#define MASTER_CLOCK            XTAL_61_44MHz
#define CLOCK_1H                (MASTER_CLOCK / 5 / 4)
#define CLOCK_16H               (CLOCK_1H / 16)
#define CLOCK_1VF               ((CLOCK_16H) / 12 / 2)
#define CLOCK_2VF               ((CLOCK_1VF) / 2)

#define PIXEL_CLOCK             (MASTER_CLOCK/10)
#define HTOTAL                  (384)
#define HBSTART                 (256)
#define HBEND                   (0)
#define VTOTAL                  (264)
#define VBSTART                 (240)
#define VBEND                   (16)

#define I8035_CLOCK             (XTAL_6MHz)

/****************************************************************************
 * CONSTANTS
 ****************************************************************************/

#define HARDWARE_TYPE_TAG       "HARDWARE_TYPE"

enum
{
	HARDWARE_TKG04 = 0,
	HARDWARE_TRS01,
	HARDWARE_TRS02,
	HARDWARE_TKG02
};

enum
{
	DKONG_RADARSCP_CONVERSION = 0,
	DKONG_BOARD = 1
};

enum
{
	DK2650_HERBIEDK = 0,
	DK2650_HUNCHBKD,
	DK2650_EIGHTACT,
	DK2650_SHOOTGAL,
	DK2650_SPCLFORC
};

#define DK2B_PALETTE_LENGTH     (256+256+8+1) /*  (256) */
#define DK4B_PALETTE_LENGTH     (256+256+8+1) /*  (256) */
#define DK3_PALETTE_LENGTH      (256+256+8+1) /*  (256) */
#define RS_PALETTE_LENGTH       (256+256+8+1)

class dkong_state : public driver_device
{
public:
	dkong_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_video_ram(*this,"video_ram"),
		m_sprite_ram(*this,"sprite_ram"),
		m_vidhw(DKONG_BOARD),
		m_discrete(*this, "discrete")
	{ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_video_ram;
	required_shared_ptr<UINT8> m_sprite_ram;

	/* devices */
	device_t *m_dev_n2a03a;
	device_t *m_dev_n2a03b;
	device_t *m_dev_vp2;        /* virtual port 2 */
	device_t *m_dev_6h;

	/* machine states */
	UINT8               m_hardware_type;
	UINT8               m_nmi_mask;

	/* sound state */
	const UINT8       *m_snd_rom;

	/* video state */
	tilemap_t           *m_bg_tilemap;

	bitmap_ind16  m_bg_bits;
	const UINT8 *     m_color_codes;
	emu_timer *       m_scanline_timer;
	INT8              m_vidhw;          /* Selected video hardware RS Conversion / TKG04 */

	optional_device<discrete_device> m_discrete;
	/* radar scope */

	UINT8 *           m_gfx4;
	UINT8 *           m_gfx3;
	int               m_gfx3_len;

	UINT8             m_sig30Hz;
	UINT8             m_lfsr_5I;
	UINT8             m_grid_sig;
	UINT8             m_rflip_sig;
	UINT8             m_star_ff;
	UINT8             m_blue_level;
	double            m_cd4049_a;
	double            m_cd4049_b;

	/* Specific states */
	INT8              m_decrypt_counter;

	/* 2650 protection */
	UINT8             m_protect_type;
	UINT8             m_hunchloopback;
	UINT8             m_prot_cnt;
	UINT8             m_main_fo;

	/* Save state relevant */
	UINT8             m_gfx_bank;
	UINT8             m_palette_bank;
	UINT8             m_grid_on;
	UINT16        m_grid_col;
	UINT8             m_sprite_bank;
	UINT8             m_dma_latch;
	UINT8             m_flip;

	/* radarscp_step */
	double m_cv1;
	double m_cv2;
	double m_vg1;
	double m_vg2;
	double m_vg3;
	double m_cv3;
	double m_cv4;
	double m_vc17;
	int m_pixelcnt;

	/* radarscp_scanline */
	int m_counter;

	/* reverse address lookup map - hunchbkd */
	INT16             m_rev_map[0x200];
	DECLARE_READ8_MEMBER(hb_dma_read_byte);
	DECLARE_WRITE8_MEMBER(hb_dma_write_byte);
	DECLARE_WRITE8_MEMBER(dkong3_coin_counter_w);
	DECLARE_READ8_MEMBER(dkong_in2_r);
	DECLARE_READ8_MEMBER(dkongjr_in2_r);
	DECLARE_READ8_MEMBER(s2650_mirror_r);
	DECLARE_WRITE8_MEMBER(s2650_mirror_w);
	DECLARE_READ8_MEMBER(epos_decrypt_rom);
	DECLARE_WRITE8_MEMBER(s2650_data_w);
	DECLARE_WRITE8_MEMBER(s2650_fo_w);
	DECLARE_READ8_MEMBER(s2650_port0_r);
	DECLARE_READ8_MEMBER(s2650_port1_r);
	DECLARE_WRITE8_MEMBER(dkong3_2a03_reset_w);
	DECLARE_READ8_MEMBER(strtheat_inputport_0_r);
	DECLARE_READ8_MEMBER(strtheat_inputport_1_r);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(braze_a15_w);
	DECLARE_WRITE8_MEMBER(dkong_videoram_w);
	DECLARE_WRITE8_MEMBER(dkongjr_gfxbank_w);
	DECLARE_WRITE8_MEMBER(dkong3_gfxbank_w);
	DECLARE_WRITE8_MEMBER(dkong_palettebank_w);
	DECLARE_WRITE8_MEMBER(radarscp_grid_enable_w);
	DECLARE_WRITE8_MEMBER(radarscp_grid_color_w);
	DECLARE_WRITE8_MEMBER(dkong_flipscreen_w);
	DECLARE_WRITE8_MEMBER(dkong_spritebank_w);
	DECLARE_WRITE8_MEMBER(dkong_voice_w);
	DECLARE_WRITE8_MEMBER(dkong_audio_irq_w);
	DECLARE_READ8_MEMBER(p8257_ctl_r);
	DECLARE_WRITE8_MEMBER(p8257_ctl_w);
	DECLARE_WRITE8_MEMBER(p8257_drq_w);
	DECLARE_WRITE8_MEMBER(dkong_z80dma_rdy_w);
	DECLARE_READ8_MEMBER(braze_eeprom_r);
	DECLARE_WRITE8_MEMBER(braze_eeprom_w);
	DECLARE_DRIVER_INIT(strtheat);
	DECLARE_DRIVER_INIT(herodk);
	DECLARE_DRIVER_INIT(dkingjr);
	DECLARE_DRIVER_INIT(drakton);
	DECLARE_DRIVER_INIT(dkongx);
	TILE_GET_INFO_MEMBER(dkong_bg_tile_info);
	TILE_GET_INFO_MEMBER(radarscp1_bg_tile_info);
	DECLARE_MACHINE_START(dkong2b);
	DECLARE_MACHINE_RESET(dkong);
	DECLARE_VIDEO_START(dkong);
	DECLARE_VIDEO_START(dkong_base);
	DECLARE_PALETTE_INIT(dkong2b);
	DECLARE_MACHINE_START(dkong3);
	DECLARE_PALETTE_INIT(dkong3);
	DECLARE_MACHINE_START(radarscp);
	DECLARE_PALETTE_INIT(radarscp);
	DECLARE_MACHINE_START(radarscp1);
	DECLARE_PALETTE_INIT(radarscp1);
	DECLARE_MACHINE_START(s2650);
	DECLARE_MACHINE_RESET(strtheat);
	DECLARE_MACHINE_RESET(drakton);
	UINT32 screen_update_dkong(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_pestplce(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_spclforc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(s2650_interrupt);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	TIMER_CALLBACK_MEMBER(scanline_callback);
	DECLARE_WRITE8_MEMBER(M58817_command_w);
	DECLARE_READ8_MEMBER(dkong_voice_status_r);
	DECLARE_READ8_MEMBER(dkong_tune_r);
	DECLARE_WRITE8_MEMBER(dkong_p1_w);
};

/*----------- defined in audio/dkong.c -----------*/

MACHINE_CONFIG_EXTERN( radarscp_audio );
MACHINE_CONFIG_EXTERN( dkong2b_audio );
MACHINE_CONFIG_EXTERN( dkongjr_audio );
MACHINE_CONFIG_EXTERN( dkong3_audio );
MACHINE_CONFIG_EXTERN( radarscp1_audio );
