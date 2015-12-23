#include "usb3300.h"
#include "derivative.h"
#include "hidef.h"
#include "usb_otg_main.h"	// todo: remove this header

#if HIGH_SPEED_DEVICE

void USB3300_Init(){
	// Disable the MPU so that USB can access RAM
	MPU_CESR &= ~MPU_CESR_VLD_MASK;

	// clock init
	SIM_CLKDIV2 |= 
			0 |							// USBHS_FRAC
			SIM_CLKDIV2_USBHSDIV(1);	// Divide reference clock to obtain 60MHz

	// MCGPLLCLK for the USB 60MHz CLKC source 
	SIM_SOPT2 |= SIM_SOPT2_USBHSRC(1);

	// External 60MHz UPLI Clock
	SIM_SOPT2 |= SIM_SOPT2_USBH_CLKSEL_MASK;

	// enable USBHS clock
	SIM_SCGC6 |= SIM_SCGC6_USB2OTG_MASK;

	// select alternate function 2 for ULPI pins
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;
	PORTA_PCR7 = PORT_PCR_MUX(2);	// DIR
	PORTA_PCR8 = PORT_PCR_MUX(2);	// NXT
	PORTA_PCR10 = PORT_PCR_MUX(2);  // Data0
	PORTA_PCR11 = PORT_PCR_MUX(2);  // Data1
	PORTA_PCR24 = PORT_PCR_MUX(2);	// Data2
	PORTA_PCR25 = PORT_PCR_MUX(2);	// Data3
	PORTA_PCR26 = PORT_PCR_MUX(2);	// Data4
	PORTA_PCR27 = PORT_PCR_MUX(2);	// Data5
	PORTA_PCR28 = PORT_PCR_MUX(2);	// Data6
	PORTA_PCR29 = PORT_PCR_MUX(2);	// Data7
	PORTA_PCR6 = PORT_PCR_MUX(2);	// CLK
	PORTA_PCR9 = PORT_PCR_MUX(2);	// STP

	EnableInterrupts;

	while(!(USBHS_ULPI_VIEWPORT & USBHS_ULPI_VIEWPORT_ULPI_SS_MASK));

	USBHS_ULPI_VIEWPORT = 0x40000000;
	while(USBHS_ULPI_VIEWPORT & (0x40000000));

	DisableInterrupts;

#ifdef SERIAL_DEBUG
	printf("module initialized ok\n");
#endif

#define USBHS_IRQ  (112-16)
	NVICICPR3 = (1 << (USBHS_IRQ % 32));	// Clear any pending interrupts on USBHS 
	NVICISER3 = (1 << (USBHS_IRQ % 32));	// Enable interrupts on USBHS
	NVICICPR3 = (1 << (USBHS_IRQ % 32));	// Clear any pending interrupts on USBHS

	//delay(100);

	// only full speed mode
	USBHS_PORTSC1 |= USBHS_PORTSC1_PFSC_MASK;
	
	// reset module
	USBHS_USBCMD |= USBHS_USBCMD_RST_MASK;

	//delay(100);
	/*
	            // check if USBHS module is ok
	        #ifdef SERIAL_DEBUG
	            printf("Initializing ULPI..\t");
	        #endif
	            USBHS_ULPI_VIEWPORT = 0x40000000;
	            while(USBHS_ULPI_VIEWPORT & (0x40000000));

	            printf("OK\n");
	        #warning "Sometimes, ULPI module doesn't initialize correctly	\
	        	(only in debug mode) and the while isn't passed. You need to\
	        	reset the board a couple of times."
	 */ 
}

/*
 * Register can be read
 */
uint_8 USB3300_Read(uint_8 addr){
	
	// only 6 bits are used for address
	addr &= 0x3F;

	USBHS_ULPI_VIEWPORT = 
			USBHS_ULPI_VIEWPORT_ULPI_DATWR(0) |					// data to write
			USBHS_ULPI_VIEWPORT_ULPI_ADDR(addr + USB3300_READ) |	// register address
			USBHS_ULPI_VIEWPORT_ULPI_PORT(0) |
			USBHS_ULPI_VIEWPORT_ULPI_RUN_MASK;
	
	// wait for operation to complete
	while(USBHS_ULPI_VIEWPORT & USBHS_ULPI_VIEWPORT_ULPI_RUN_MASK);
	
	// store register value
	return ((USBHS_ULPI_VIEWPORT & 
			USBHS_ULPI_VIEWPORT_ULPI_DATRD_MASK) >> 
			USBHS_ULPI_VIEWPORT_ULPI_DATRD_SHIFT);
}

/*
 * Pattern on the data bus will be written over all bits of the register
 */
void USB3300_Write(uint_8 addr, uint_8 val){
	
	// only 6 bits are used for address
	addr &= 0x3F;

	USBHS_ULPI_VIEWPORT = 
			USBHS_ULPI_VIEWPORT_ULPI_DATWR(val) |				// data to write
			USBHS_ULPI_VIEWPORT_ULPI_ADDR(addr + USB3300_WRITE) |	// register address
			USBHS_ULPI_VIEWPORT_ULPI_PORT(0) |
			USBHS_ULPI_VIEWPORT_ULPI_RW_MASK |					// direction: write
			USBHS_ULPI_VIEWPORT_ULPI_RUN_MASK;
	
	// wait for operation to complete
	while(USBHS_ULPI_VIEWPORT & USBHS_ULPI_VIEWPORT_ULPI_RUN_MASK);
}

/*
 *  Pattern on the data bus will be OR'd with and written into the register
 */
void USB3300_Set(uint_8 addr, uint_8 val){
	
	// only 6 bits are used for address
	addr &= 0x3F;

	USBHS_ULPI_VIEWPORT = 
			USBHS_ULPI_VIEWPORT_ULPI_DATWR(val) |				// data to write
			USBHS_ULPI_VIEWPORT_ULPI_ADDR(addr + USB3300_SET) |	// register address
			USBHS_ULPI_VIEWPORT_ULPI_PORT(0) |
			USBHS_ULPI_VIEWPORT_ULPI_RW_MASK |					// direction: write
			USBHS_ULPI_VIEWPORT_ULPI_RUN_MASK;
	
	// wait for operation to complete
	while(USBHS_ULPI_VIEWPORT & USBHS_ULPI_VIEWPORT_ULPI_RUN_MASK);
}

/* 
 * Pattern on the data bus is a mask. If a bit in the mask is set, then
 * the corresponding register bit will be set to zero
 */
void USB3300_Clear(uint_8 addr){
	
	// only 6 bits are used for address
	addr &= 0x3F;

	USBHS_ULPI_VIEWPORT = 
			USBHS_ULPI_VIEWPORT_ULPI_DATWR(0) |					// data to write
			USBHS_ULPI_VIEWPORT_ULPI_ADDR(addr + USB3300_CLEAR) |	// register address
			USBHS_ULPI_VIEWPORT_ULPI_PORT(0) |
			USBHS_ULPI_VIEWPORT_ULPI_RW_MASK |					// direction: write
			USBHS_ULPI_VIEWPORT_ULPI_RUN_MASK;
	
	// wait for operation to complete
	while(USBHS_ULPI_VIEWPORT & USBHS_ULPI_VIEWPORT_ULPI_RUN_MASK);
}

uint_8 USB3300_GetStatus(){
	return USB3300_Read(USB3300_INTERRUPT_STATUS);
}

uint_8 USB3300_GetInterrupts(){
	return USB3300_Read(USB3300_INTERRUPT_LATCH);
}

void USB3300_SetVBUS(boolean enable){
	if(enable){
		//USB3300_Write(USB3300_OTG_CONTROL, USB3300_OTG_CONTROL_DRV_VBUS_MASK);
		USBHS_PORTSC1 |= USBHS_PORTSC1_PP_MASK;
	}else{
		//USB3300_Write(USB3300_OTG_CONTROL, USB3300_OTG_CONTROL_DISCHARGE_VBUS_MASK);
		USBHS_PORTSC1 &= ~USBHS_PORTSC1_PP_MASK;
	}
}

void USB3300_SetPdowns(uint_8 bitfield){
	if(bitfield & OTG_CTRL_PDOWN_DP)
		USB3300_Write(USB3300_OTG_CONTROL, USB3300_OTG_CONTROL_PDOWN_DP_MASK);
	if(bitfield & OTG_CTRL_PDOWN_DM)
		USB3300_Write(USB3300_OTG_CONTROL, USB3300_OTG_CONTROL_PDOWN_DM_MASK);
}

#endif // HIGH_SPEED_DEVICE