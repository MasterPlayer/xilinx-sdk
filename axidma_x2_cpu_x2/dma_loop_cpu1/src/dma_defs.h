/*
 * dma_defs.h
 *
 *  Created on: 17 окт. 2018 г.
 *      Author: Maxim
 */
#include <xaxidma.h>
#ifndef SRC_DMA_DEFS_H_
#define SRC_DMA_DEFS_H_

typedef struct pkt{
	uint32_t baseaddr;
	uint32_t size;
	uint32_t completed;
}pkt;

typedef struct dma_pkg{
	XAxiDma dma_usb;
	XAxiDma dma_pl;
}dma_pkg;


#define 	DMA_USB_DEV					XPAR_AXI_DMA_USB_DEVICE_ID
#define 	DMA_PL_DEV 					XPAR_AXI_DMA_PL_DEVICE_ID


#define CPU0_EN_FLAQ  (*(uint8_t*)(0xFFFF0200))
#define CPU1_EN_FLAQ  (*(uint8_t*)(0xFFFF0300))



/**************************************** PL Interrupts ****************************************/
#define 	USB_TX_INTR 				XPAR_FABRIC_AXI_DMA_USB_MM2S_INTROUT_INTR
#define 	USB_RX_INTR 				XPAR_FABRIC_AXI_DMA_USB_S2MM_INTROUT_INTR
#define 	PL_TX_INTR 					XPAR_FABRIC_AXI_DMA_PL_MM2S_INTROUT_INTR
#define 	PL_RX_INTR 					XPAR_FABRIC_AXI_DMA_PL_S2MM_INTROUT_INTR




/**************************************** DMA BUFFER PARAMETERS ****************************************/
#define 	DMA_USB_TX_BUF_PER_BD 		65536 										// buffer size per bd
#define 	DMA_USB_TX_BD_CNT 			512 										// bd count in bd ring
#define 	DMA_USB_TX_BUFFER_BASE 		0x01000000 									// TX BUFFER BASE
#define 	DMA_USB_TX_BUFFER_HIGH 		((DMA_USB_TX_BUFFER_BASE) + (DMA_USB_TX_BUF_PER_BD * DMA_USB_TX_BD_CNT))
#define 	DMA_USB_TX_BDSIZE 			128
#define 	DMA_USB_TX_BD_BASE 			0x03100000
#define 	DMA_USB_TX_BD_HIGH 			(DMA_USB_TX_BD_BASE + (DMA_USB_TX_BD_CNT * DMA_USB_TX_BDSIZE))


#define 	DMA_USB_RX_BUF_PER_BD 		65536 										// buffer size per bd
#define 	DMA_USB_RX_BD_CNT 			512 										// bd count in bd ring
#define 	DMA_USB_RX_BUFFER_BASE 		0x04000000 									// TX BUFFER BASE
#define 	DMA_USB_RX_BUFFER_HIGH 		((DMA_USB_RX_BUFFER_BASE) + (DMA_USB_RX_BUF_PER_BD * DMA_USB_RX_BD_CNT))
#define 	DMA_USB_RX_BDSIZE 			128
#define 	DMA_USB_RX_BD_BASE 			0x06100000
#define 	DMA_USB_RX_BD_HIGH 			(DMA_USB_RX_BD_BASE + (DMA_USB_RX_BD_CNT * DMA_USB_RX_BDSIZE))

#define 	DMA_PL_TX_BUF_PER_BD 		65536 										// buffer size per bd
#define 	DMA_PL_TX_BD_CNT 			512 										// bd count in bd ring
#define 	DMA_PL_TX_BUFFER_BASE 		0x07000000 									// TX BUFFER BASE
#define 	DMA_PL_TX_BUFFER_HIGH 		((DMA_PL_TX_BUFFER_BASE) + (DMA_PL_TX_BUF_PER_BD * DMA_PL_TX_BD_CNT))
#define 	DMA_PL_TX_BDSIZE 			128
#define 	DMA_PL_TX_BD_BASE 			0x09100000
#define 	DMA_PL_TX_BD_HIGH 			(DMA_PL_TX_BD_BASE + (DMA_PL_TX_BD_CNT * DMA_PL_TX_BDSIZE))

#define 	DMA_PL_RX_BUF_PER_BD 		65536 										// buffer size per bd
#define 	DMA_PL_RX_BD_CNT 			512 										// bd count in bd ring
#define 	DMA_PL_RX_BUFFER_BASE 		0x0A000000 									// TX BUFFER BASE
#define 	DMA_PL_RX_BUFFER_HIGH 		((DMA_PL_RX_BUFFER_BASE) + (DMA_PL_RX_BUF_PER_BD * DMA_PL_RX_BD_CNT))
#define 	DMA_PL_RX_BDSIZE 			128
#define 	DMA_PL_RX_BD_BASE 			0x0C100000
#define 	DMA_PL_RX_BD_HIGH 			(DMA_PL_RX_BD_BASE + (DMA_PL_RX_BD_CNT * DMA_PL_RX_BDSIZE))


int 	dma_usb_init 	(XAxiDma *dma_inst);
int 	dma_usb_rx_init (XAxiDma *dma_inst);
int 	dma_usb_tx_init (XAxiDma *dma_inst);
int     init_dma_usb_transmission(pkt pkt_inst, XAxiDma *dma_inst_ptr)      ;

/**************************************** DMA USB BUFFER PARAMETERS ****************************************/


int 	dma_pl_init 	(XAxiDma *dma_inst);
int 	dma_pl_rx_init (XAxiDma *dma_inst);
int 	dma_pl_tx_init (XAxiDma *dma_inst);
int     init_dma_pl_transmission(pkt pkt_inst, XAxiDma *dma_inst_ptr)      ;
int     init_dma_usb_multi_tx(pkt *pkt_inst, int cnt ,XAxiDma *dma_inst_ptr);


/**************************************** INTERRUPT HANDLERS ****************************************/
void usb_rx_intr_handler(void *callback);
void usb_tx_intr_handler(void *callback);
void pl_rx_intr_handler(void *callback);
void pl_tx_intr_handler(void *callback);



#endif /* SRC_DMA_DEFS_H_ */
