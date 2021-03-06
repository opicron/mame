#pragma once

#ifndef __C128__
#define __C128__

#include "emu.h"
#include "formats/cbm_snqk.h"
#include "includes/cbm.h"
#include "cpu/m6502/m8502.h"
#include "machine/6526cia.h"
#include "machine/c64exp.h"
#include "machine/c64user.h"
#include "machine/cbmiec.h"
#include "machine/cbmipt.h"
#include "machine/mos6526.h"
#include "machine/mos8722.h"
#include "machine/petcass.h"
#include "machine/pla.h"
#include "machine/ram.h"
#include "machine/vcsctrl.h"
#include "sound/dac.h"
#include "sound/sid6581.h"
#include "video/mos6566.h"

#define Z80A_TAG        "u10"
#define M8502_TAG       "u6"
#define MOS8563_TAG     "u22"
#define MOS8564_TAG     "u21"
#define MOS8566_TAG     "u21"
#define MOS6581_TAG     "u5"
#define MOS6526_1_TAG   "u1"
#define MOS6526_2_TAG   "u4"
#define MOS8721_TAG     "u11"
#define MOS8722_TAG     "u7"
#define SCREEN_VIC_TAG  "screen"
#define SCREEN_VDC_TAG  "screen80"
#define CONTROL1_TAG    "joy1"
#define CONTROL2_TAG    "joy2"

class c128_state : public driver_device
{
public:
	c128_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80A_TAG),
			m_subcpu(*this, M8502_TAG),
			m_mmu(*this, MOS8722_TAG),
			m_pla(*this, MOS8721_TAG),
			m_vdc(*this, MOS8563_TAG),
			m_vic(*this, MOS8564_TAG),
			m_sid(*this, MOS6581_TAG),
			m_cia1(*this, MOS6526_1_TAG),
			m_cia2(*this, MOS6526_2_TAG),
			m_iec(*this, CBM_IEC_TAG),
			m_joy1(*this, CONTROL1_TAG),
			m_joy2(*this, CONTROL2_TAG),
			m_exp(*this, C64_EXPANSION_SLOT_TAG),
			m_user(*this, C64_USER_PORT_TAG),
			m_ram(*this, RAM_TAG),
			m_cassette(*this, PET_DATASSETTE_PORT_TAG),
			m_z80en(0),
			m_loram(1),
			m_hiram(1),
			m_charen(1),
			m_game(1),
			m_exrom(1),
			m_rom1(NULL),
			m_rom2(NULL),
			m_rom3(NULL),
			m_rom4(NULL),
			m_from(NULL),
			m_charom(NULL),
			m_color_ram(*this, "color_ram"),
			m_va14(1),
			m_va15(1),
			m_clrbank(0),
			m_cnt1(1),
			m_sp1(1),
			m_iec_data_out(1),
			m_cia1_irq(CLEAR_LINE),
			m_cia2_irq(CLEAR_LINE),
			m_vic_irq(CLEAR_LINE),
			m_exp_irq(CLEAR_LINE),
			m_exp_nmi(CLEAR_LINE),
			m_cass_rd(1),
			m_iec_srq(1),
			m_vic_k(0x07),
			m_caps_lock(1)
	{ }

	required_device<legacy_cpu_device> m_maincpu;
	required_device<m8502_device> m_subcpu;
	required_device<mos8722_device> m_mmu;
	required_device<mos8721_device> m_pla;
	required_device<mos8563_device> m_vdc;
	required_device<mos6566_device> m_vic;
	required_device<sid6581_device> m_sid;
	required_device<mos6526_device> m_cia1;
	required_device<mos6526_device> m_cia2;
	required_device<cbm_iec_device> m_iec;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<c64_expansion_slot_device> m_exp;
	required_device<c64_user_port_device> m_user;
	required_device<ram_device> m_ram;
	required_device<pet_datassette_port_device> m_cassette;

	virtual void machine_start();
	virtual void machine_reset();

	inline void check_interrupts();
	void read_pla(offs_t offset, offs_t ca, offs_t vma, int ba, int rw, int aec, int z80io, int ms3, int ms2, int ms1, int ms0,
		int *sden, int *dir, int *gwe, int *rom1, int *rom2, int *rom3, int *rom4, int *charom, int *colorram, int *vic,
		int *from1, int *romh, int *roml, int *dwe, int *ioacc, int *clrbank, int *iocs, int *casenb);
	UINT8 read_memory(address_space &space, offs_t offset, offs_t vma, int ba, int aec, int z80io);
	void write_memory(address_space &space, offs_t offset, offs_t vma, UINT8 data, int ba, int aec, int z80io);
	inline void update_iec();

	DECLARE_READ8_MEMBER( z80_r );
	DECLARE_WRITE8_MEMBER( z80_w );
	DECLARE_READ8_MEMBER( z80_io_r );
	DECLARE_WRITE8_MEMBER( z80_io_w );
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( vic_videoram_r );
	DECLARE_READ8_MEMBER( vic_colorram_r );

	DECLARE_WRITE_LINE_MEMBER( mmu_z80en_w );
	DECLARE_WRITE_LINE_MEMBER( mmu_fsdir_w );
	DECLARE_READ_LINE_MEMBER( mmu_game_r );
	DECLARE_READ_LINE_MEMBER( mmu_exrom_r );
	DECLARE_READ_LINE_MEMBER( mmu_sense40_r );

	INTERRUPT_GEN_MEMBER( frame_interrupt );
	DECLARE_WRITE_LINE_MEMBER( vic_irq_w );
	DECLARE_WRITE8_MEMBER( vic_k_w );

	DECLARE_READ8_MEMBER( sid_potx_r );
	DECLARE_READ8_MEMBER( sid_poty_r );

	DECLARE_WRITE_LINE_MEMBER( cia1_irq_w );
	DECLARE_WRITE_LINE_MEMBER( cia1_cnt_w );
	DECLARE_WRITE_LINE_MEMBER( cia1_sp_w );
	DECLARE_READ8_MEMBER( cia1_pa_r );
	DECLARE_READ8_MEMBER( cia1_pb_r );
	DECLARE_WRITE8_MEMBER( cia1_pb_w );

	DECLARE_WRITE_LINE_MEMBER( cia2_irq_w );
	DECLARE_READ8_MEMBER( cia2_pa_r );
	DECLARE_WRITE8_MEMBER( cia2_pa_w );

	DECLARE_READ8_MEMBER( cpu_r );
	DECLARE_WRITE8_MEMBER( cpu_w );

	DECLARE_WRITE_LINE_MEMBER( tape_read_w );

	DECLARE_WRITE_LINE_MEMBER( iec_srq_w );
	DECLARE_WRITE_LINE_MEMBER( iec_data_w );

	DECLARE_READ8_MEMBER( exp_dma_r );
	DECLARE_WRITE8_MEMBER( exp_dma_w );
	DECLARE_WRITE_LINE_MEMBER( exp_irq_w );
	DECLARE_WRITE_LINE_MEMBER( exp_nmi_w );
	DECLARE_WRITE_LINE_MEMBER( exp_dma_w );
	DECLARE_WRITE_LINE_MEMBER( exp_reset_w );

	DECLARE_INPUT_CHANGED_MEMBER( restore );
	DECLARE_INPUT_CHANGED_MEMBER( caps_lock );

	// memory state
	int m_z80en;
	int m_loram;
	int m_hiram;
	int m_charen;
	int m_game;
	int m_exrom;
	int m_reset;
	const UINT8 *m_rom1;
	const UINT8 *m_rom2;
	const UINT8 *m_rom3;
	const UINT8 *m_rom4;
	const UINT8 *m_from;
	const UINT8 *m_charom;

	// video state
	optional_shared_ptr<UINT8> m_color_ram;
	int m_va14;
	int m_va15;
	int m_clrbank;

	// fast serial state
	int m_cnt1;
	int m_sp1;
	int m_iec_data_out;

	// interrupt state
	int m_cia1_irq;
	int m_cia2_irq;
	int m_vic_irq;
	int m_exp_irq;
	int m_exp_nmi;
	int m_exp_dma;
	int m_cass_rd;
	int m_iec_srq;

	// keyboard state
	UINT8 m_vic_k;
	int m_caps_lock;
};



#endif
