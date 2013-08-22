/**********************************************************************

    CMD HD hard drive emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "cmdhd.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "m6502"
#define M6522_1_TAG     "m6522_1"
#define M6522_2_TAG     "m6522_2"
#define I8255A_TAG      "i8255a"
#define RTC72421A_TAG   "rtc"
#define SCSIBUS_TAG     "scsi"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CMD_HD = &device_creator<cmd_hd_device>;


//-------------------------------------------------
//  ROM( cmd_hd )
//-------------------------------------------------

ROM_START( cmd_hd )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "cmd_hd_bootrom_v280.bin", 0x0000, 0x8000, CRC(da68435d) SHA1(defd8bc04a52904b8a3560f11c82126619513a10) )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "pal16l8_1", 0x000, 0x001, NO_DUMP )
	ROM_LOAD( "pal16l8_2", 0x000, 0x001, NO_DUMP )
	ROM_LOAD( "pal16l8_3", 0x000, 0x001, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *cmd_hd_device::device_rom_region() const
{
	return ROM_NAME( cmd_hd );
}


//-------------------------------------------------
//  ADDRESS_MAP( cmd_hd_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( cmd_hd_mem, AS_PROGRAM, 8, cmd_hd_device )
	AM_RANGE(0x0000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION(M6502_TAG, 0)
	AM_RANGE(0x8000, 0x800f) AM_MIRROR(0x1f0) AM_DEVREADWRITE(M6522_1_TAG, via6522_device, read, write)
	AM_RANGE(0x8400, 0x840f) AM_MIRROR(0x1f0) AM_DEVREADWRITE(M6522_2_TAG, via6522_device, read, write)
	AM_RANGE(0x8800, 0x8803) AM_MIRROR(0x1fc) AM_DEVREADWRITE(I8255A_TAG, i8255_device, read, write)
	AM_RANGE(0x8c00, 0x8c0f) AM_MIRROR(0x1f0) //AM_DEVREADWRITE(RTC72421A_TAG, rtc72421a_device, read, write)
	AM_RANGE(0x8f00, 0x8f00) AM_MIRROR(0xff) AM_WRITE(led_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  via6522_interface via1_intf
//-------------------------------------------------

static const via6522_interface via1_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL
};


//-------------------------------------------------
//  via6522_interface via2_intf
//-------------------------------------------------

static const via6522_interface via2_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL
};


//-------------------------------------------------
//  I8255A_INTERFACE( ppi_intf )
//-------------------------------------------------

static I8255A_INTERFACE( ppi_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  MACHINE_DRIVER( cmd_hd )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( cmd_hd )
	MCFG_CPU_ADD(M6502_TAG, M6502, 2000000)
	MCFG_CPU_PROGRAM_MAP(cmd_hd_mem)

	MCFG_VIA6522_ADD(M6522_1_TAG, 2000000, via1_intf)
	MCFG_VIA6522_ADD(M6522_2_TAG, 2000000, via2_intf)
	MCFG_I8255A_ADD(I8255A_TAG, ppi_intf)
	//MCFG_RTC72421A_ADD(RTC72421A_TAG)

	MCFG_SCSIBUS_ADD(SCSIBUS_TAG)
	MCFG_SCSIDEV_ADD(SCSIBUS_TAG ":harddisk0", SCSIHD, SCSI_ID_0)
	MCFG_SCSICB_ADD(SCSIBUS_TAG ":host")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor cmd_hd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cmd_hd );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cmd_hd_device - constructor
//-------------------------------------------------

cmd_hd_device::cmd_hd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CMD_HD, "HD", tag, owner, clock),
		device_cbm_iec_interface(mconfig, *this),
		m_maincpu(*this, M6502_TAG),
		m_scsibus(*this, SCSIBUS_TAG":host")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cmd_hd_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cmd_hd_device::device_reset()
{
}


//-------------------------------------------------
//  cbm_iec_srq -
//-------------------------------------------------

void cmd_hd_device::cbm_iec_srq(int state)
{
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void cmd_hd_device::cbm_iec_atn(int state)
{
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void cmd_hd_device::cbm_iec_data(int state)
{
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void cmd_hd_device::cbm_iec_reset(int state)
{
	if (!state)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  led_w -
//-------------------------------------------------

WRITE8_MEMBER( cmd_hd_device::led_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/
}
