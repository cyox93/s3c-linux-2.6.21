/*
 * Xilinx ML300 evaluation board initialization
 *
 * Author: MontaVista Software, Inc.
 *         source@mvista.com
 *
 * 2002-2004 (c) MontaVista Software, Inc.  This file is licensed under the
 * terms of the GNU General Public License version 2.  This program is licensed
 * "as is" without any warranty of any kind, whether express or implied.
 */

#include <linux/init.h>
#include <linux/irq.h>
#include <linux/tty.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>
#include <linux/serialP.h>
#include <linux/kgdb.h>

#include <asm/io.h>
#include <asm/machdep.h>
#include <asm/ppc_sys.h>

#include <syslib/gen550.h>
#include <platforms/4xx/xparameters/xparameters.h>

/*
 * As an overview of how the following functions (platform_init,
 * ml300_map_io, ml300_setup_arch and ml300_init_IRQ) fit into the
 * kernel startup procedure, here's a call tree:
 *
 * start_here					arch/ppc/kernel/head_4xx.S
 *  early_init					arch/ppc/kernel/setup.c
 *  machine_init				arch/ppc/kernel/setup.c
 *    platform_init				this file
 *      ppc4xx_init				arch/ppc/syslib/ppc4xx_setup.c
 *        parse_bootinfo
 *          find_bootinfo
 *        "setup some default ppc_md pointers"
 *  MMU_init					arch/ppc/mm/init.c
 *    *ppc_md.setup_io_mappings == ml300_map_io	this file
 *      ppc4xx_map_io				arch/ppc/syslib/ppc4xx_setup.c
 *  start_kernel				init/main.c
 *    setup_arch				arch/ppc/kernel/setup.c
 *      *ppc_md.setup_arch == ml300_setup_arch	this file
 *        ppc4xx_setup_arch			arch/ppc/syslib/ppc4xx_setup.c
 *          ppc4xx_find_bridges			arch/ppc/syslib/ppc405_pci.c
 *    init_IRQ					arch/ppc/kernel/irq.c
 *      *ppc_md.init_IRQ == ml300_init_IRQ	this file
 *        ppc4xx_init_IRQ			arch/ppc/syslib/ppc4xx_setup.c
 *          ppc4xx_pic_init			arch/ppc/syslib/xilinx_pic.c
 */

/* Board specifications structures */
struct ppc_sys_spec *cur_ppc_sys_spec;
struct ppc_sys_spec ppc_sys_specs[] = {
	{
		/* Only one entry, always assume the same design */
		.ppc_sys_name	= "Xilinx ML300 Reference Design",
		.mask 		= 0x00000000,
		.value 		= 0x00000000,
		.num_devices	= 1,
		.device_list	= (enum ppc_sys_devices[])
		{
			VIRTEX_UART,
		},
	},
};

#if defined(XPAR_POWER_0_POWERDOWN_BASEADDR)

static volatile unsigned *powerdown_base =
    (volatile unsigned *) XPAR_POWER_0_POWERDOWN_BASEADDR;

static void
xilinx_power_off(void)
{
	local_irq_disable();
	out_be32(powerdown_base, XPAR_POWER_0_POWERDOWN_VALUE);
	while (1) ;
}
#endif

void __init
ml300_map_io(void)
{
	ppc4xx_map_io();

#if defined(XPAR_POWER_0_POWERDOWN_BASEADDR)
	powerdown_base = ioremap((unsigned long) powerdown_base,
				 XPAR_POWER_0_POWERDOWN_HIGHADDR -
				 XPAR_POWER_0_POWERDOWN_BASEADDR + 1);
#endif
}

/* Early serial support functions */
static void __init
ml300_early_serial_init(int num, struct plat_serial8250_port *pdata)
{
#if defined(CONFIG_SERIAL_TEXT_DEBUG) || defined(CONFIG_KGDB_8250)
	struct uart_port serial_req;

	memset(&serial_req, 0, sizeof(serial_req));
	serial_req.mapbase	= pdata->mapbase;
	serial_req.membase	= pdata->membase;
	serial_req.irq		= pdata->irq;
	serial_req.uartclk	= pdata->uartclk;
	serial_req.regshift	= pdata->regshift;
	serial_req.iotype	= pdata->iotype;
	serial_req.flags	= pdata->flags;
#ifdef CONFIG_SERIAL_TEXT_DEBUG
	gen550_init(num, &serial_req);
#endif
#ifdef CONFIG_KGDB_8250
	kgdb8250_add_port(num, &serial_req);
#endif
#endif
}

void __init
ml300_early_serial_map(void)
{
#if defined(CONFIG_SERIAL_8250) || defined(CONFIG_KGDB_8250)
	struct plat_serial8250_port *pdata;
	int i = 0;

	pdata = (struct plat_serial8250_port *) ppc_sys_get_pdata(VIRTEX_UART);
	while(pdata && pdata->flags)
	{
		pdata->membase = ioremap(pdata->mapbase, 0x100);
		ml300_early_serial_init(i, pdata);
		pdata++;
		i++;
	}
#endif /* defined(CONFIG_SERIAL_8250) || defined(CONFIG_KGDB_8250) */
}

void __init
ml300_setup_arch(void)
{
	ml300_early_serial_map();
	ppc4xx_setup_arch();	/* calls ppc4xx_find_bridges() */

	/* Identify the system */
	printk(KERN_INFO "Xilinx Virtex-II Pro port\n");
	printk(KERN_INFO "Port by MontaVista Software, Inc. (source@mvista.com)\n");
}

/* Called after board_setup_irq from ppc4xx_init_IRQ(). */
void __init
ml300_init_irq(void)
{
	ppc4xx_init_IRQ();
}

void __init
platform_init(unsigned long r3, unsigned long r4, unsigned long r5,
	      unsigned long r6, unsigned long r7)
{
	ppc4xx_init(r3, r4, r5, r6, r7);

	identify_ppc_sys_by_id(mfspr(SPRN_PVR));

	ppc_md.setup_arch = ml300_setup_arch;
	ppc_md.setup_io_mappings = ml300_map_io;
	ppc_md.init_IRQ = ml300_init_irq;

#if defined(XPAR_POWER_0_POWERDOWN_BASEADDR)
	ppc_md.power_off = xilinx_power_off;
#endif
}
