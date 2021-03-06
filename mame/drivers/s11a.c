/****************************************************************************************

    Pinball
    Williams System 11A

    Status of games:


ToDo:
- Doesn't react to the Advance button very well
- Some LEDs flicker

Note: To start a game, certain switches need to be activated.  You must first press and
      hold one of the trough switches (usually the left) and the ball shooter switch for
      about 1 second.  Then you are able to start a game.
      For Pinbot, you must hold L and V for a second, then press start.
      For Millionaire, you must hold [ and ] for a second, then start.

*****************************************************************************************/


#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "sound/hc55516.h"
#include "sound/2151intf.h"
#include "sound/dac.h"
#include "includes/s11.h"
#include "s11a.lh"

static ADDRESS_MAP_START( s11a_main_map, AS_PROGRAM, 8, s11a_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x2100, 0x2103) AM_MIRROR(0x00fc) AM_DEVREADWRITE("pia21", pia6821_device, read, write) // sound+solenoids
	AM_RANGE(0x2200, 0x2200) AM_MIRROR(0x01ff) AM_WRITE(sol3_w) // solenoids
	AM_RANGE(0x2400, 0x2403) AM_MIRROR(0x03fc) AM_DEVREADWRITE("pia24", pia6821_device, read, write) // lamps
	AM_RANGE(0x2800, 0x2803) AM_MIRROR(0x03fc) AM_DEVREADWRITE("pia28", pia6821_device, read, write) // display
	AM_RANGE(0x2c00, 0x2c03) AM_MIRROR(0x03fc) AM_DEVREADWRITE("pia2c", pia6821_device, read, write) // alphanumeric display
	AM_RANGE(0x3000, 0x3003) AM_MIRROR(0x03fc) AM_DEVREADWRITE("pia30", pia6821_device, read, write) // inputs
	AM_RANGE(0x3400, 0x3403) AM_MIRROR(0x0bfc) AM_DEVREADWRITE("pia34", pia6821_device, read, write) // widget
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( s11a_audio_map, AS_PROGRAM, 8, s11a_state )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x0800) AM_RAM
	AM_RANGE(0x1000, 0x1fff) AM_WRITE(bank_w)
	AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x0ffc) AM_DEVREADWRITE("pias", pia6821_device, read, write)
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank0")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( s11a_bg_map, AS_PROGRAM, 8, s11a_state )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x1800) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_MIRROR(0x1ffe) AM_DEVREADWRITE("ym2151", ym2151_device, read, write)
	AM_RANGE(0x4000, 0x4003) AM_MIRROR(0x1ffc) AM_DEVREADWRITE("pia40", pia6821_device, read, write)
	AM_RANGE(0x7800, 0x7fff) AM_WRITE(bgbank_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bgbank")
ADDRESS_MAP_END

static INPUT_PORTS_START( s11a )
	PORT_START("X0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("X10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)

	PORT_START("X20")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("X40")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("X80")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, s11a_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, s11a_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_9) PORT_TOGGLE
	PORT_CONFNAME( 0x10, 0x10, "Language" )
	PORT_CONFSETTING( 0x00, "German" )
	PORT_CONFSETTING( 0x10, "English" )
INPUT_PORTS_END


MACHINE_RESET_MEMBER( s11a_state, s11a )
{
	MACHINE_RESET_CALL_MEMBER(s11);
	membank("bgbank")->set_entry(0);
}

static const pia6821_interface pia21_intf =
{
	DEVCB_DRIVER_MEMBER(s11_state, dac_r),      /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_NULL,     /* line CA1 in */
	DEVCB_LINE_GND,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11_state, sound_w),        /* port A out */
	DEVCB_DRIVER_MEMBER(s11_state, sol2_w),     /* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia21_ca2_w),       /* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia21_cb2_w),       /* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia_irq),       /* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia_irq)        /* IRQB */
};

static const pia6821_interface pia24_intf =
{
	DEVCB_NULL,     /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_LINE_GND,     /* line CA1 in */
	DEVCB_LINE_GND,     /* line CB1 in */
	DEVCB_LINE_VCC,     /* line CA2 in */
	DEVCB_LINE_VCC,     /* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11_state, lamp0_w),        /* port A out */
	DEVCB_DRIVER_MEMBER(s11_state, lamp1_w),        /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia24_cb2_w),       /* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia_irq),       /* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia_irq)        /* IRQB */
};

WRITE8_MEMBER( s11a_state::dig0_w )
{
	data &= 0x7f;
	set_strobe(data & 15);
	set_diag((data & 0x70) >> 4);
	output_set_digit_value(60, 0);  // +5VDC (always on)
	output_set_digit_value(61, get_diag() & 0x01);  // connected to PA4
	output_set_digit_value(62, 0);  // Blanking (pretty much always on)
	set_segment1(0);
	set_segment2(0);
}

static const pia6821_interface pia28_intf =
{
	DEVCB_DRIVER_MEMBER(s11_state, pia28_w7_r),     /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_NULL,     /* line CA1 in */
	DEVCB_NULL,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11a_state, dig0_w),        /* port A out */
	DEVCB_DRIVER_MEMBER(s11_state, dig1_w),     /* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia28_ca2_w),       /* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia28_cb2_w),       /* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia_irq),       /* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia_irq)        /* IRQB */
};

static const pia6821_interface pia2c_intf =
{
	DEVCB_NULL,     /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_NULL,     /* line CA1 in */
	DEVCB_NULL,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11_state, pia2c_pa_w),     /* port A out */
	DEVCB_DRIVER_MEMBER(s11_state, pia2c_pb_w),     /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_NULL,     /* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia_irq),       /* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia_irq)        /* IRQB */
};

static const pia6821_interface pia30_intf =
{
	DEVCB_DRIVER_MEMBER(s11_state, switch_r),       /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_LINE_GND,     /* line CA1 in */
	DEVCB_LINE_GND,     /* line CB1 in */
	DEVCB_LINE_VCC,     /* line CA2 in */
	DEVCB_LINE_VCC,     /* line CB2 in */
	DEVCB_NULL,     /* port A out */
	DEVCB_DRIVER_MEMBER(s11_state, switch_w),       /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia30_cb2_w),       /* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia_irq),       /* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia_irq)        /* IRQB */
};

static const pia6821_interface pia34_intf =
{
	DEVCB_NULL,     /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_NULL,     /* line CA1 in */
	DEVCB_NULL,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11_state, pia34_pa_w),     /* port A out */
	DEVCB_DRIVER_MEMBER(s11_state, pia34_pb_w),     /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia34_cb2_w),       /* line CB2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia_irq),       /* IRQA */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia_irq)        /* IRQB */
};

WRITE8_MEMBER( s11a_state::bgbank_w )
{
	membank("bgbank")->set_entry(data & 0x03);
}

static const pia6821_interface pias_intf =
{
	DEVCB_DRIVER_MEMBER(s11_state, dac_r),      /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pias_ca1_r),        /* line CA1 in */
	DEVCB_NULL,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11_state, sound_w),        /* port A out */
	DEVCB_DRIVER_MEMBER(s11_state, dac_w),      /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pia40_cb2_w),       /* line CB2 out */
	DEVCB_CPU_INPUT_LINE("audiocpu", M6800_IRQ_LINE),       /* IRQA */
	DEVCB_CPU_INPUT_LINE("audiocpu", M6800_IRQ_LINE)        /* IRQB */
};

static const pia6821_interface pia40_intf =
{
	DEVCB_NULL,     /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pias_ca1_r),        /* line CA1 in */
	DEVCB_NULL,     /* line CB1 in */
	DEVCB_LINE_VCC,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_DRIVER_MEMBER(s11_state, pia40_pa_w),     /* port A out */
	DEVCB_DRIVER_MEMBER(s11_state, pia40_pb_w),     /* port B out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pias_ca2_w),        /* line CA2 out */
	DEVCB_DRIVER_LINE_MEMBER(s11_state, pias_cb2_w),        /* line CB2 out */
	DEVCB_CPU_INPUT_LINE("bgcpu", M6809_FIRQ_LINE),     /* IRQA */
	DEVCB_CPU_INPUT_LINE("bgcpu", INPUT_LINE_NMI)       /* IRQB */
};

DRIVER_INIT_MEMBER( s11a_state, s11a )
{
	UINT8 *BGROM = memregion("bgcpu")->base();
	membank("bgbank")->configure_entries(0, 4, &BGROM[0x10000], 0x8000);
	membank("bgbank")->set_entry(0);
	s11_state::init_s11();
}

static MACHINE_CONFIG_START( s11a, s11a_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6808, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(s11a_main_map)
	MCFG_MACHINE_RESET_OVERRIDE(s11a_state, s11a)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_s11a)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_PIA6821_ADD("pia21", pia21_intf)
	MCFG_PIA6821_ADD("pia24", pia24_intf)
	MCFG_PIA6821_ADD("pia28", pia28_intf)
	MCFG_PIA6821_ADD("pia2c", pia2c_intf)
	MCFG_PIA6821_ADD("pia30", pia30_intf)
	MCFG_PIA6821_ADD("pia34", pia34_intf)
	MCFG_NVRAM_ADD_1FILL("nvram")

	/* Add the soundcard */
	MCFG_CPU_ADD("audiocpu", M6802, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(s11a_audio_map)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SPEAKER_STANDARD_MONO("speech")
	MCFG_SOUND_ADD("hc55516", HC55516, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speech", 0.50)

	MCFG_PIA6821_ADD("pias", pias_intf)

	/* Add the background music card */
	MCFG_CPU_ADD("bgcpu", M6809E, 8000000) // MC68B09E
	MCFG_CPU_PROGRAM_MAP(s11a_bg_map)

	MCFG_SPEAKER_STANDARD_MONO("bg")
	MCFG_YM2151_ADD("ym2151", 3580000)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(s11a_state, ym2151_irq_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)

	MCFG_PIA6821_ADD("pia40", pia40_intf)
MACHINE_CONFIG_END

/*------------------------
/ F14 Tomcat 5/87 (#554)
/-------------------------*/

ROM_START(f14_p3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("f14_l3.u26", 0x4000, 0x4000, CRC(cd607556) SHA1(2ec95085784370a071cbf5df7ae5c6b4749605e2))
	ROM_LOAD("f14_l3.u27", 0x8000, 0x8000, CRC(72951fd1) SHA1(b5f3fe1859e0abf9ab558b4b4f6754134d528c23))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u21.l1", 0x18000, 0x8000, CRC(e412300c) SHA1(382d0cfa47abea295f0c7501bc0a010473e9d73b))
	ROM_LOAD("f14_u22.l1", 0x10000, 0x8000, CRC(c9dd7496) SHA1(de3cb855d87033274cc912578b02d1593d2d69f9))

	ROM_REGION(0x30000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u4.l1", 0x10000, 0x8000, CRC(43ecaabf) SHA1(64b50dbff03cd556130d0cff47b951fdf37d397d))
	ROM_LOAD("f14_u19.l1", 0x18000, 0x8000, CRC(d0de4a7c) SHA1(46ecd5786653add47751cc56b38d9db7c4622377))
ROM_END

ROM_START(f14_p4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26_l4.128", 0x4000, 0x4000, CRC(7b39706a) SHA1(0dc0b1a1dfd12bc73e6fd8b825fe72ddc8fc1497))
	ROM_LOAD("u27_l4.256", 0x8000, 0x8000, CRC(189f9488) SHA1(7536d56cb83bf29f8d8b03b226a5f60200776095))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u21.l1", 0x18000, 0x8000, CRC(e412300c) SHA1(382d0cfa47abea295f0c7501bc0a010473e9d73b))
	ROM_LOAD("f14_u22.l1", 0x10000, 0x8000, CRC(c9dd7496) SHA1(de3cb855d87033274cc912578b02d1593d2d69f9))

	ROM_REGION(0x30000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u4.l1", 0x10000, 0x8000, CRC(43ecaabf) SHA1(64b50dbff03cd556130d0cff47b951fdf37d397d))
	ROM_LOAD("f14_u19.l1", 0x18000, 0x8000, CRC(d0de4a7c) SHA1(46ecd5786653add47751cc56b38d9db7c4622377))
ROM_END

ROM_START(f14_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("f14_u26.l1", 0x4000, 0x4000, CRC(62c2e615) SHA1(456ce0d1f74fa5e619c272880ba8ac6819848ddc))
	ROM_LOAD("f14_u27.l1", 0x8000, 0x8000, CRC(da1740f7) SHA1(1395a4f3891a043cfedc5426ec88af35eab8d4ea))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u21.l1", 0x18000, 0x8000, CRC(e412300c) SHA1(382d0cfa47abea295f0c7501bc0a010473e9d73b))
	ROM_LOAD("f14_u22.l1", 0x10000, 0x8000, CRC(c9dd7496) SHA1(de3cb855d87033274cc912578b02d1593d2d69f9))

	ROM_REGION(0x30000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("f14_u4.l1", 0x10000, 0x8000, CRC(43ecaabf) SHA1(64b50dbff03cd556130d0cff47b951fdf37d397d))
	ROM_LOAD("f14_u19.l1", 0x18000, 0x8000, CRC(d0de4a7c) SHA1(46ecd5786653add47751cc56b38d9db7c4622377))
ROM_END

/*--------------------
/ Fire! 8/87 (#556)
/--------------------*/
ROM_START(fire_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("fire_u26.l3", 0x4000, 0x4000, CRC(48abae33) SHA1(00ce24316aa007eec090ae74818003e11a141214))
	ROM_LOAD("fire_u27.l3", 0x8000, 0x8000, CRC(4ebf4888) SHA1(45dc0231404ed70be2ab5d599a673aac6271550e))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("fire_u21.l2", 0x18000, 0x8000, CRC(2edde0a4) SHA1(de292a340a3a06b0b996fc69fee73eb7bbfbbe64))
	ROM_LOAD("fire_u22.l2", 0x10000, 0x8000, CRC(16145c97) SHA1(523e99df3907a2c843c6e27df4d16799c4136a46))

	ROM_REGION(0x30000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("fire_u4.l1", 0x10000, 0x8000, CRC(0e058918) SHA1(4d6bf2290141119174787f8dd653c47ea4c73693))
ROM_END

/*--------------------------------------
/ Fire! Champagne Edition 9/87 (#556SE)
/---------------------------------------*/

/*-------------------------
/ Millionaire 1/87 (#555)
/--------------------------*/
ROM_START(milln_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mill_u26.l3", 0x4000, 0x4000, CRC(07bc9fff) SHA1(b16082fb51df3e4d2fb786cb8894b1c232521ef3))
	ROM_LOAD("mill_u27.l3", 0x8000, 0x8000, CRC(ba789c43) SHA1(c066a304882bea4cba1e215642416fcb22585aa4))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("mill_u21.l1", 0x18000, 0x8000, CRC(4cd1ee90) SHA1(4e24b96138ced16eff9036303ca6347e3423dbfc))
	ROM_LOAD("mill_u22.l1", 0x10000, 0x8000, CRC(73735cfc) SHA1(f74c873a20990263e0d6b35609fc51c08c9f8e31))

	ROM_REGION(0x30000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("mill_u4.l1", 0x10000, 0x8000, CRC(cf766506) SHA1(a6e4df19a513102abbce2653d4f72245f54407b1))
	ROM_LOAD("mill_u19.l1", 0x18000, 0x8000, CRC(e073245a) SHA1(cbaddde6bb19292ace574a8329e18c97c2ee9763))
ROM_END

/*--------------------
/ Pinbot 10/86 (#549)
/--------------------*/
ROM_START(pb_l5)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("pbot_u26.l5", 0x4000, 0x4000, CRC(daa0c8e4) SHA1(47289b350eb0d84aa0d37e53383e18625451bbe8))
	ROM_LOAD("pbot_u27.l5", 0x8000, 0x8000, CRC(e625d6ce) SHA1(1858dc2183954342b8e2e5eb9a14edcaa8dad5ae))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u21.l1", 0x18000, 0x8000, CRC(3eab88d9) SHA1(667e3b675e2ae8fec6a6faddb9b0dd5531d64f8f))
	ROM_LOAD("pbot_u22.l1", 0x10000, 0x8000, CRC(a2d2c9cb) SHA1(46437dc54538f1626caf41a2818ddcf8000c44e4))

	ROM_REGION(0x30000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u4.l1", 0x10000, 0x8000, CRC(de5926bd) SHA1(3d111e27c5f0c8c0afc5fe5cc45bf77c12b69228))
	ROM_LOAD("pbot_u19.l1", 0x18000, 0x8000, CRC(40eb4e9f) SHA1(07b0557b35599a2dd5aa66a306fbbe8f50eed998))
ROM_END

ROM_START(pb_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-l2.rom", 0x4000, 0x4000, CRC(e3b94ca4) SHA1(1db2acb025941cc165cc7ec70a160e07ab1eeb2e))
	ROM_LOAD("u27-l2.rom", 0x8000, 0x8000, CRC(0a334fc5) SHA1(d08afe6ddc141e37f97ea588d184a316ff7f6db7))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u21.l1", 0x18000, 0x8000, CRC(3eab88d9) SHA1(667e3b675e2ae8fec6a6faddb9b0dd5531d64f8f))
	ROM_LOAD("pbot_u22.l1", 0x10000, 0x8000, CRC(a2d2c9cb) SHA1(46437dc54538f1626caf41a2818ddcf8000c44e4))

	ROM_REGION(0x30000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u4.l1", 0x10000, 0x8000, CRC(de5926bd) SHA1(3d111e27c5f0c8c0afc5fe5cc45bf77c12b69228))
	ROM_LOAD("pbot_u19.l1", 0x18000, 0x8000, CRC(40eb4e9f) SHA1(07b0557b35599a2dd5aa66a306fbbe8f50eed998))
ROM_END

ROM_START(pb_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-l2.rom", 0x4000, 0x4000, CRC(e3b94ca4) SHA1(1db2acb025941cc165cc7ec70a160e07ab1eeb2e))
	ROM_LOAD("u27-l3.rom", 0x8000, 0x8000, CRC(6f40ee84) SHA1(85453137e3fdb1e422e3903dd053e04c9f2b9607))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u21.l1", 0x18000, 0x8000, CRC(3eab88d9) SHA1(667e3b675e2ae8fec6a6faddb9b0dd5531d64f8f))
	ROM_LOAD("pbot_u22.l1", 0x10000, 0x8000, CRC(a2d2c9cb) SHA1(46437dc54538f1626caf41a2818ddcf8000c44e4))

	ROM_REGION(0x30000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("pbot_u4.l1", 0x10000, 0x8000, CRC(de5926bd) SHA1(3d111e27c5f0c8c0afc5fe5cc45bf77c12b69228))
	ROM_LOAD("pbot_u19.l1", 0x18000, 0x8000, CRC(40eb4e9f) SHA1(07b0557b35599a2dd5aa66a306fbbe8f50eed998))
ROM_END

GAME(1987, f14_l1,   0,      s11a, s11a, s11a_state, s11a, ROT0, "Williams", "F14 Tomcat (L-1)", GAME_IS_SKELETON_MECHANICAL)
GAME(1987, f14_p3,   f14_l1, s11a, s11a, s11a_state, s11a, ROT0, "Williams", "F14 Tomcat (P-3)", GAME_IS_SKELETON_MECHANICAL)
GAME(1987, f14_p4,   f14_l1, s11a, s11a, s11a_state, s11a, ROT0, "Williams", "F14 Tomcat (P-4)", GAME_IS_SKELETON_MECHANICAL)
GAME(1987, fire_l3,  0,      s11a, s11a, s11a_state, s11a, ROT0, "Williams", "Fire! (L-3)", GAME_IS_SKELETON_MECHANICAL)
GAME(1987, milln_l3, 0,      s11a, s11a, s11a_state, s11a, ROT0, "Williams", "Millionaire (L-3)", GAME_IS_SKELETON_MECHANICAL)
GAME(1986, pb_l5,    0,      s11a, s11a, s11a_state, s11a, ROT0, "Williams", "Pin-Bot (L-5)", GAME_IS_SKELETON_MECHANICAL)
GAME(1986, pb_l2,    pb_l5,  s11a, s11a, s11a_state, s11a, ROT0, "Williams", "Pin-Bot (L-2)", GAME_IS_SKELETON_MECHANICAL)
GAME(1986, pb_l3,    pb_l5,  s11a, s11a, s11a_state, s11a, ROT0, "Williams", "Pin-Bot (L-3)", GAME_IS_SKELETON_MECHANICAL)
