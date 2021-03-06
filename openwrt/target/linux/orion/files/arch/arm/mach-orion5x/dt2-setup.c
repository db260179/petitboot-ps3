/*
 * arch/arm/mach-orion5x/dt2-setup.c
 *
 * Freecom DataTank Gateway Setup
 *
 * Copyright (C) 2009 Zintis Petersons <Zintis.Petersons@abcsolutions.lv>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/pci.h>
#include <linux/irq.h>
#include <linux/mtd/physmap.h>
#include <linux/mv643xx_eth.h>
#include <linux/ethtool.h>
#include <linux/if_ether.h>
#include <net/dsa.h>
#include <linux/ata_platform.h>
#include <linux/i2c.h>
#include <linux/reboot.h>
#include <linux/interrupt.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>
#include <asm/leds.h>
#include <asm/mach/arch.h>
#include <asm/mach/pci.h>
#include <mach/orion5x.h>
#include "common.h"
#include "mpp.h"

/*****************************************************************************
 * DT2 local
 ****************************************************************************/
#include <asm/setup.h>
#include "dt2-common.h"

u32 mvUbootVer = 0;
u32 mvTclk = 166666667;
u32 mvSysclk = 200000000;
u32 mvIsUsbHost = 1;
u32 overEthAddr = 0;
u32 gBoardId = -1;
struct DT2_EEPROM_STRUCT dt2_eeprom;

/*****************************************************************************
 * DT2 Info
 ****************************************************************************/
/*
 * PCI
 */

#define DT2_PCI_SLOT0_OFFS	7
#define DT2_PCI_SLOT0_IRQ_A_PIN	3
#define DT2_PCI_SLOT0_IRQ_B_PIN	2

#define DT2_PIN_GPIO_SYNC	25
#define DT2_PIN_GPIO_POWER	24
#define DT2_PIN_GPIO_UNPLUG1	23
#define DT2_PIN_GPIO_UNPLUG2	22
#define DT2_PIN_GPIO_RESET	4

#define DT2_NOR_BOOT_BASE	0xf4000000
#define DT2_NOR_BOOT_SIZE	SZ_512K

#define DT2_LEDS_BASE		0xfa000000
#define DT2_LEDS_SIZE		SZ_1K

/*****************************************************************************
 * 512K NOR Flash on Device bus Boot CS
 ****************************************************************************/

static struct mtd_partition dt2_partitions[] = {
	{
		.name	= "u-boot",
		.size	= 0x00080000,
		.offset	= 0,
	},
};

static struct physmap_flash_data dt2_nor_flash_data = {
	.width		= 1,		/* 8 bit bus width */
	.parts		= dt2_partitions,
	.nr_parts	= ARRAY_SIZE(dt2_partitions)
};

static struct resource dt2_nor_flash_resource = {
	.flags		= IORESOURCE_MEM,
	.start		= DT2_NOR_BOOT_BASE,
	.end		= DT2_NOR_BOOT_BASE + DT2_NOR_BOOT_SIZE - 1,
};

static struct platform_device dt2_nor_flash = {
	.name		= "physmap-flash",
	.id		= 0,
	.dev		= {
		.platform_data	= &dt2_nor_flash_data,
	},
	.resource	= &dt2_nor_flash_resource,
	.num_resources	= 1,
};

/*****************************************************************************
 * PCI
 ****************************************************************************/

void __init dt2_pci_preinit(void)
{
	int pin, irq;

	/*
	 * Configure PCI GPIO IRQ pins
	 */
	pin = DT2_PCI_SLOT0_IRQ_A_PIN;
	if (gpio_request(pin, "PCI IntA") == 0) {
		if (gpio_direction_input(pin) == 0) {
			irq = gpio_to_irq(pin);
			set_irq_type(irq, IRQ_TYPE_LEVEL_LOW);
			printk (KERN_INFO "PCI IntA IRQ: %d\n", irq);
		} else {
			printk(KERN_ERR "dt2_pci_preinit failed to "
					"set_irq_type pin %d\n", pin);
			gpio_free(pin);
		}
	} else {
		printk(KERN_ERR "dt2_pci_preinit failed to request gpio %d\n", pin);
	}

	pin = DT2_PCI_SLOT0_IRQ_B_PIN;
	if (gpio_request(pin, "PCI IntB") == 0) {
		if (gpio_direction_input(pin) == 0) {
			irq = gpio_to_irq(pin);
			set_irq_type(irq, IRQ_TYPE_LEVEL_LOW);
			printk (KERN_INFO "PCI IntB IRQ: %d\n", irq);
		} else {
			printk(KERN_ERR "dt2_pci_preinit failed to "
					"set_irq_type pin %d\n", pin);
			gpio_free(pin);
		}
	} else {
		printk(KERN_ERR "dt2_pci_preinit failed to gpio_request %d\n", pin);
	}
}

static int __init dt2_pci_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	int irq;

	/*
	 * Check for devices with hard-wired IRQs.
	 */
	irq = orion5x_pci_map_irq(dev, slot, pin);
	if (irq != -1){
		printk(KERN_INFO "orion5x_pci_map_irq: %d\n", irq);
		return irq;
	}

	/*
	 * PCI IRQs are connected via GPIOs
	 */
	switch (slot - DT2_PCI_SLOT0_OFFS) {
	case 0:
		if (pin == 1){
			irq = gpio_to_irq(DT2_PCI_SLOT0_IRQ_A_PIN);
			printk(KERN_INFO "dt2_pci_map_irq DT2_PCI_SLOT0_IRQ_A_PIN: %d\n", irq);
		}
		else {
			irq = gpio_to_irq(DT2_PCI_SLOT0_IRQ_B_PIN);
			printk(KERN_INFO "dt2_pci_map_irq DT2_PCI_SLOT0_IRQ_B_PIN: %d\n", irq);
		}
	default:
		irq = -1;
			printk(KERN_INFO "dt2_pci_map_irq IRQ: %d\n", irq);
	}

	return irq;
}

static struct hw_pci dt2_pci __initdata = {
	.nr_controllers	= 2,
	.preinit	= dt2_pci_preinit,
	.swizzle	= pci_std_swizzle,
	.setup		= orion5x_pci_sys_setup,
	.scan		= orion5x_pci_sys_scan_bus,
	.map_irq	= dt2_pci_map_irq,
};

static int __init dt2_pci_init(void)
{
	if (machine_is_dt2())
		pci_common_init(&dt2_pci);

	return 0;
}

subsys_initcall(dt2_pci_init);

/*****************************************************************************
 * Ethernet
 ****************************************************************************/

static struct mv643xx_eth_platform_data dt2_eth_data = {
	.phy_addr	= MV643XX_ETH_PHY_NONE,
	.speed		= SPEED_1000,
	.duplex		= DUPLEX_FULL,
};

static struct dsa_chip_data dt2_switch_chip_data = {
	.port_names[0] = "wan",
	.port_names[1] = "lan1",
	.port_names[2] = "lan2",
	.port_names[3] = "cpu",
	.port_names[4] = "lan3",
	.port_names[5] = "lan4",
};

static struct dsa_platform_data dt2_switch_plat_data = {
	.nr_chips	= 1,
	.chip		= &dt2_switch_chip_data,
};

/*****************************************************************************
 * RTC ISL1208 on I2C bus
 ****************************************************************************/
static struct i2c_board_info __initdata dt2_i2c_rtc = {
	I2C_BOARD_INFO("isl1208", 0x6F),
};

/*****************************************************************************
 * Sata
 ****************************************************************************/
static struct mv_sata_platform_data dt2_sata_data = {
	.n_ports	= 2,
};

/*****************************************************************************
 * General Setup
 ****************************************************************************/
static struct orion5x_mpp_mode dt2_mpp_modes[] __initdata = {
	{  0, MPP_GPIO },		// RTC interrupt
	{  1, MPP_GPIO },		// 88e6131 interrupt
	{  2, MPP_GPIO },		// PCI_intB
	{  3, MPP_GPIO },		// PCI_intA
	{  4, MPP_GPIO },		// reset button switch
	{  5, MPP_GPIO },
	{  6, MPP_GPIO },
	{  7, MPP_GPIO },
	{  8, MPP_GPIO },
	{  9, MPP_GIGE },		/* GE_RXERR */
	{ 10, MPP_GPIO },		// usb
	{ 11, MPP_GPIO },		// usb
	{ 12, MPP_GIGE },		// GE_TXD[4]
	{ 13, MPP_GIGE },		// GE_TXD[5]
	{ 14, MPP_GIGE },		// GE_TXD[6]
	{ 15, MPP_GIGE },		// GE_TXD[7]
	{ 16, MPP_GIGE },		// GE_RXD[4]
	{ 17, MPP_GIGE },		// GE_RXD[5]
	{ 18, MPP_GIGE },		// GE_RXD[6]
	{ 19, MPP_GIGE },		// GE_RXD[7]
	{ -1 },
};

/*****************************************************************************
 * LEDS
 ****************************************************************************/
static struct platform_device dt2_leds = {
	.name		= "dt2-led",
	.id		= -1,
};

/****************************************************************************
 * GPIO key
 ****************************************************************************/
static irqreturn_t dt2_reset_handler(int irq, void *dev_id)
{
	/* This is the paper-clip reset which does an emergency reboot. */
	printk(KERN_INFO "Restarting system.\n");
	machine_restart(NULL);

	/* This should never be reached. */
	return IRQ_HANDLED;
}

static irqreturn_t dt2_power_handler(int irq, void *dev_id)
{
	printk(KERN_INFO "Shutting down system.\n");
	machine_power_off();
	return IRQ_HANDLED;
}

static void __init dt2_init(void)
{
	/*
	 * Setup basic Orion functions. Need to be called early.
	 */
	orion5x_init();

	orion5x_mpp_conf(dt2_mpp_modes);

	/*
	 * Configure peripherals.
	 */

	orion5x_uart0_init();
	orion5x_ehci0_init();
	orion5x_ehci1_init();
	orion5x_i2c_init();
	orion5x_sata_init(&dt2_sata_data);
	orion5x_xor_init();

	printk(KERN_INFO "U-Boot parameters:\n");
	printk(KERN_INFO "Sys Clk = %d, Tclk = %d, BoardID = 0x%02x\n", mvSysclk, mvTclk, gBoardId);

	printk(KERN_INFO "Serial: %s\n", dt2_eeprom.fc.dt2_serial_number);
	printk(KERN_INFO "Revision: %016x\n", dt2_eeprom.fc.dt2_revision);
	printk(KERN_INFO "DT2: Using MAC address %pM for port 0\n",
	       dt2_eeprom.gw.mac_addr[0]);
	printk(KERN_INFO "DT2: Using MAC address %pM for port 1\n",
	       dt2_eeprom.gw.mac_addr[1]);

	orion5x_eth_init(&dt2_eth_data);
	memcpy(dt2_eth_data.mac_addr, dt2_eeprom.gw.mac_addr[0], 6);
	orion5x_eth_switch_init(&dt2_switch_plat_data, NO_IRQ);

	i2c_register_board_info(0, &dt2_i2c_rtc, 1);

	orion5x_setup_dev_boot_win(DT2_NOR_BOOT_BASE, DT2_NOR_BOOT_SIZE);
	platform_device_register(&dt2_nor_flash);

	orion5x_setup_dev0_win(DT2_LEDS_BASE, DT2_LEDS_SIZE);
	platform_device_register(&dt2_leds);

	if (request_irq(gpio_to_irq(DT2_PIN_GPIO_RESET), &dt2_reset_handler,
			IRQF_DISABLED | IRQF_TRIGGER_LOW,
			"DT2: Reset button", NULL) < 0) {

		printk("DT2: Reset Button IRQ %d not available\n",
			gpio_to_irq(DT2_PIN_GPIO_RESET));
	}

	if (request_irq(gpio_to_irq(DT2_PIN_GPIO_POWER), &dt2_power_handler,
			IRQF_DISABLED | IRQF_TRIGGER_LOW,
			"DT2: Power button", NULL) < 0) {

		printk(KERN_DEBUG "DT2: Power Button IRQ %d not available\n",
			gpio_to_irq(DT2_PIN_GPIO_POWER));
	}
}

static int __init parse_tag_dt2_uboot(const struct tag *t)
{
	struct tag_mv_uboot *mv_uboot;
		
	// Get pointer to our block
	mv_uboot = (struct tag_mv_uboot*)&t->u;
	mvTclk = mv_uboot->tclk;
	mvSysclk = mv_uboot->sysclk;
	mvUbootVer = mv_uboot->uboot_version;
	mvIsUsbHost = mv_uboot->isUsbHost;

	// Some clock fixups
	if(mvTclk == 166000000) mvTclk = 166666667;
	else if(mvTclk == 133000000) mvTclk = 133333333;
	else if(mvSysclk == 166000000) mvSysclk = 166666667;

	gBoardId =  (mvUbootVer & 0xff);
			
	//DT2 specific data
	memcpy(&dt2_eeprom, mv_uboot->dt2_eeprom, sizeof(struct DT2_EEPROM_STRUCT));

	return 0;
}
__tagtable(ATAG_MV_UBOOT, parse_tag_dt2_uboot);

/*
 * This is OpenWrt specific fixup. It includes code from original "tag_fixup_mem32" to
 * fixup bogus memory tags and also fixes kernel cmdline by adding " init=/etc/preinit"
 * at the end. It is important to flash OpenWrt image from original Freecom firmware.
 *
 * Vanilla kernel should use "tag_fixup_mem32" function.
 */
void __init openwrt_fixup(struct machine_desc *mdesc, struct tag *t,
			    char **from, struct meminfo *meminfo)
{
	char *p = NULL;
	static char openwrt_init_tag[] __initdata = " init=/etc/preinit";

	for (; t->hdr.size; t = tag_next(t)){
		/* Locate the Freecom cmdline */
		if (t->hdr.tag == ATAG_CMDLINE) {
			p = t->u.cmdline.cmdline;
			printk("%s(%d): Found cmdline '%s' at 0x%0lx\n",
			       __FUNCTION__, __LINE__, p, (unsigned long)p);
		}
		/*
		 * Many orion-based systems have buggy bootloader implementations.
		 * This is a common fixup for bogus memory tags.
		 */
		if (t->hdr.tag == ATAG_MEM &&
		    (!t->u.mem.size || t->u.mem.size & ~PAGE_MASK ||
		     t->u.mem.start & ~PAGE_MASK)) {
			printk(KERN_WARNING
			       "Clearing invalid memory bank %dKB@0x%08x\n",
			       t->u.mem.size / 1024, t->u.mem.start);
			t->hdr.tag = 0;
		}
	}

	printk("%s(%d): End of table at 0x%0lx\n", __FUNCTION__, __LINE__, (unsigned long)t);

	/* Overwrite the end of the table with a new cmdline tag. */ 
	t->hdr.tag = ATAG_CMDLINE;
	t->hdr.size =
		(sizeof (struct tag_header) +
		 strlen(p) + strlen(openwrt_init_tag) + 1 + 4) >> 2;

	strlcpy(t->u.cmdline.cmdline, p, COMMAND_LINE_SIZE);
	strlcpy(t->u.cmdline.cmdline + strlen(p), openwrt_init_tag,
		COMMAND_LINE_SIZE - strlen(p));

	printk("%s(%d): New cmdline '%s' at 0x%0lx\n",
	       __FUNCTION__, __LINE__,
	       t->u.cmdline.cmdline, (unsigned long)t->u.cmdline.cmdline);

	t = tag_next(t);

	printk("%s(%d): New end of table at 0x%0lx\n", __FUNCTION__, __LINE__, (unsigned long)t);

	t->hdr.tag = ATAG_NONE;
	t->hdr.size = 0;
}

/* Warning: Freecom uses their own custom bootloader with mach-type (=1500) */
MACHINE_START(DT2, "Freecom DataTank Gateway")
	/* Maintainer: Zintis Petersons <Zintis.Petersons@abcsolutions.lv> */
	.boot_params	= 0x00000100,
	.init_machine	= dt2_init,
	.map_io		= orion5x_map_io,
	.init_irq	= orion5x_init_irq,
	.timer		= &orion5x_timer,
	.fixup		= openwrt_fixup, //tag_fixup_mem32,
MACHINE_END
