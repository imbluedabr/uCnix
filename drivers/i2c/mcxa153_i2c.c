#include <uapi/sys/i2c.h>
#include <uapi/sys/errno.h>
#include <kernel/device.h>
#include <drivers/i2c.h>
#include <kernel/majors.h>
#include <board/mcxa153/board.h>
#include <lib/kmalloc.h>
#include <stdint.h>


struct bus_driver i2c_driver = {
	.base.probe = i2c_probe,
	.base.name = "i2c",
	.base.bus_accept = BUS_MMIO,
	.base.bus_type = BUS_I2C
};

const struct dev_ops i2c_ops = {
	.ioctl = i2c_ioctl
};

const struct bus_ops i2c_bus_ops = {
	.probe = i2c_bus_probe,
	.i2c_rdwr = i2c_rdwr
};

void i2c_init()
{
	register_driver(I2C_MAJOR, &i2c_driver.base);
}

struct device* i2c_probe(struct bus_device* parent, const void* descriptor)
{
	struct i2c_bus_device* dev = (struct i2c_bus_device*) kzalloc(sizeof(struct i2c_bus_device));
	if (!dev) return NULL;
	
	const struct mmio_bus_desc* desc = descriptor;
	LPI2C_Type* i2c = (LPI2C_Type*) desc->base;
	dev->base.base.ops = (struct dev_ops*) &i2c_ops;
	dev->base.bus_ops = (struct bus_ops*) &i2c_bus_ops;
	dev->i2c = i2c;
	switch ((uint32_t) desc->base) {
		case LPI2C0_BASE: //dogshit mcu lol, only has one i2c controller
			MRCC0->MRCC_LPI2C0_CLKSEL = MRCC_MRCC_LPI2C0_CLKSEL_MUX(2);
			MRCC0->MRCC_LPI2C0_CLKDIV = 0;
			MRCC0->MRCC_GLB_CC0 |= MRCC_MRCC_GLB_CC0_LPI2C0(1);
			MRCC0->MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_PORT3(1);
			MRCC0->MRCC_GLB_RST0 |= MRCC_MRCC_GLB_RST0_LPI2C0(1);
			MRCC0->MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_PORT3(1);
			
			PORT3->PCR[28] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_PE(1) | PORT_PCR_PS(1) | PORT_PCR_ODE(1) | PORT_PCR_IBE(1); // LPI2C0_SDA
			PORT3->PCR[27] = PORT_PCR_LK(1) | PORT_PCR_MUX(2) | PORT_PCR_PE(1) | PORT_PCR_PS(1) | PORT_PCR_ODE(1) | PORT_PCR_IBE(1); // LPI2C0_SCL
			break;
	}

	i2c->MCFGR1 = LPI2C_MCFGR1_PRESCALE(0);
	i2c->MCFGR2 = LPI2C_MCFGR2_FILTSDA(1) | LPI2C_MCFGR2_FILTSCL(1);
	i2c->MCCR0 = LPI2C_MCCR0_DATAVD(0x0F) | LPI2C_MCCR0_SETHOLD(0x1D) | LPI2C_MCCR0_CLKHI(0x35) | LPI2C_MCCR0_CLKLO(0x3E);
	i2c->MCR |= LPI2C_MCR_MEN(1);
	return &dev->base.base;

}

struct device* i2c_bus_probe(struct bus_device* bus, const void* descriptor)
{
	struct i2c_bus_device* dev = (struct i2c_bus_device*) bus;
	const struct i2c_bus_desc* i2c_desc = descriptor;
	
	//simply match the descriptor against registered drivers
	for (int i = 0; i < i2c_driver.registered_drivers_count; i++) {
		if (i2c_driver.registered_drivers[i] == i2c_desc->major) {
			return device_probe(i2c_desc->major, bus, descriptor);
		}
	}

	return NULL;
}

int i2c_ioctl(struct file* f, int cmd, void* arg)
{
	struct i2c_bus_device* i2c = (struct i2c_bus_device*) f->i->devfs.dev;
	switch(cmd) {
		case I2C_RDWR:
			return i2c_rdwr(&i2c->base, arg);
		default:
			return -ENOTTY;
	}
}


static int i2c_rx_ready(volatile LPI2C_Type* i2c) {
	volatile int timeout = 100000;
	while(!(i2c->MSR & LPI2C_MSR_RDF(1)) && timeout-- > 0);
	if (timeout == 0) return -1;
	return 0;
}

static int i2c_tx_ready(volatile LPI2C_Type* i2c) {
	volatile int timeout = 100000;
	while(!(i2c->MSR & LPI2C_MSR_TDF(1)) && timeout-- > 0);
	if (timeout == 0) return -1;
	return 0;
}

int i2c_rdwr(struct bus_device* bus, struct i2c_rdwr_ioctl_data* data)
{
	struct i2c_bus_device* dev = (struct i2c_bus_device*) bus;
	volatile LPI2C_Type* i2c = dev->i2c;

	while(i2c->MSR & LPI2C_MSR_BBF(1));

	// Clear all status flags
	i2c->MSR = LPI2C_MSR_STF_MASK | LPI2C_MSR_DMF_MASK |
		LPI2C_MSR_PLTF_MASK | LPI2C_MSR_FEF_MASK | LPI2C_MSR_ALF_MASK |
		LPI2C_MSR_NDF_MASK | LPI2C_MSR_SDF_MASK | LPI2C_MSR_EPF_MASK;
	
	for (uint32_t i = 0; i < data->nmsgs; i++) {
		struct i2c_msg msg = data->msgs[i];
			
		if (!(msg.flags & I2C_M_NOSTART) || i == 0) { //dont send start condition + address when I2C_M_NOSTART is set, unless were at message 0 since then we must send that
			//start + address
			i2c->MTDR = LPI2C_MTDR_CMD(0b100) | LPI2C_MTDR_DATA((msg.addr << 1) | (msg.flags & I2C_M_RD));
		}
		
		if (msg.flags & I2C_M_RD) {
			i2c->MTDR = LPI2C_MTDR_CMD(0b001) | LPI2C_MTDR_DATA(msg.len - 1);
			for (uint32_t k = 0; k < msg.len; k++) {
				if (i2c->MSR & LPI2C_MSR_NDF(1)) return -ENXIO;
				if (i2c_rx_ready(i2c) == -1) return -EIO; //wait for data in the rx fifo
				msg.buf[k] = (uint8_t) i2c->MRDR;
			}
		} else {
			for (uint32_t k = 0; k < msg.len; k++) {
				if (i2c->MSR & LPI2C_MSR_NDF(1)) return -ENXIO;
				if (i2c_tx_ready(i2c) == -1) return -EIO; //wait for space in transmit fifo
				//data
				i2c->MTDR = LPI2C_MTDR_CMD(0b000) | LPI2C_MTDR_DATA(msg.buf[k]);
			}
		}
	}

	//stop
	if (i2c->MSR & LPI2C_MSR_NDF(1)) return -ENXIO;
	if (i2c_tx_ready(i2c) == -1) return -EIO;
	i2c->MTDR = LPI2C_MTDR_CMD(0b010);

	while(i2c->MSR & LPI2C_MSR_BBF(1));
	if (i2c->MSR & LPI2C_MSR_NDF(1)) return -ENXIO;

	return 0;
}

