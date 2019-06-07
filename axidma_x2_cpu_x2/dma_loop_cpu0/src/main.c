#include <stdio.h>

#include "platform.h"
#include "platform_config.h"
#include "xaxidma.h"
#include "xil_exception.h"
#include "dma_defs.h"
#include "xscugic.h"
#include "xil_mmu.h"


int     scugic_initialize(XScuGic *gic_inst_ptr, dma_pkg *dma_pkg_inst_ptr);

int tx_flaq;


void dma_to_cache(XAxiDma dma_inst){
	uint8_t* buffer = (uint8_t*)0xFFFF0000;
	memcpy(buffer, (uint8_t*)&dma_inst, sizeof(XAxiDma));
}



int main()
{
	tx_flaq = 0;
	/**/
	init_platform();

	Xil_L1DCacheDisable();

	xil_printf("CPU0 :: init semaphore\r\n");
	/*Область памяти, инициирующая механизм обмена между процессорами*/
	CPU0_EN_FLAQ = 0;
	CPU1_EN_FLAQ = 0;

	xil_printf("CPU0 :: init memory attr\r\n");
	Xil_SetTlbAttributes(0xFFFF0000,0x14de2);           // S=b1 TEX=b100 AP=b11, Domain=b1111, C=b0, B=b0
	/*Разрешаем работать второму процессору*/
	CPU1_EN_FLAQ = 1;

	xil_printf("CPU0 :: wait cpu1\r\n");
	/*ждем сигнала от второго процессора для разрешения работы*/
	while(CPU0_EN_FLAQ == 0){

	}

	xil_printf("CPU0 :: wait finished\r\n");

	CPU0_EN_FLAQ = 0;

	dbg_print_memspace();

    dma_pkg dma_pkg_0;

    XScuGic scugic_0;
	xil_printf("CPU0 :: gic init\r\n");
    int status = scugic_initialize(&scugic_0, &dma_pkg_0);
	xil_printf("CPU0 :: usb init\r\n");
    dma_usb_init(&dma_pkg_0.dma_usb);
    dma_usb_rx_init(&dma_pkg_0.dma_usb);
    dma_usb_tx_init(&dma_pkg_0.dma_usb);
    dma_to_cache(dma_pkg_0.dma_usb);

    CPU1_EN_FLAQ = 1;
    xil_printf("CPU0 :: wait cpu1\r\n");
    while(CPU0_EN_FLAQ == 0){

    }
    xil_printf("CPU0 :: wait finished\r\n");
    CPU0_EN_FLAQ = 0;
    xil_printf("CPU0 :: pl init\r\n");
    dma_pl_init(&dma_pkg_0.dma_pl);
    dma_pl_rx_init(&dma_pkg_0.dma_pl);
    dma_pl_tx_init(&dma_pkg_0.dma_pl);
    dma_to_cache(dma_pkg_0.dma_pl);

    CPU1_EN_FLAQ = 1;
    xil_printf("CPU0 :: wait cpu1\r\n");
    while(CPU0_EN_FLAQ == 0){

    }
    xil_printf("CPU0 :: wait finished\r\n");
    CPU0_EN_FLAQ = 0;

	while(1){

	}

    cleanup_platform();

    return 0;
}



int     scugic_initialize(XScuGic *gic_inst_ptr, dma_pkg *dma_pkg_inst_ptr){

    int status = 0;

    XScuGic_Config *cfg;

    cfg = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);

    status = XScuGic_CfgInitialize(gic_inst_ptr, cfg, cfg->CpuBaseAddress);

    status = XScuGic_Connect(gic_inst_ptr, USB_RX_INTR, (Xil_InterruptHandler)usb_rx_intr_handler, dma_pkg_inst_ptr);

	XScuGic_Enable(gic_inst_ptr, USB_RX_INTR);
	XScuGic_Enable(gic_inst_ptr, PL_TX_INTR);

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, gic_inst_ptr);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_FIQ_INT, (Xil_ExceptionHandler)pl_tx_intr_handler, dma_pkg_inst_ptr);
	Xil_ExceptionEnableMask(XIL_EXCEPTION_ALL);

	Xil_ExceptionEnable();
    return status ;
}



void pl_tx_intr_handler(void *callback){
	u32 IRQ;
	dma_pkg *dma_pkg_inst = (dma_pkg*)callback;
	XAxiDma_BdRing* tx_ring = XAxiDma_GetTxRing(&(dma_pkg_inst->dma_pl));

	IRQ = XAxiDma_BdRingGetIrq(tx_ring);

	XAxiDma_BdRingAckIrq(tx_ring, IRQ);

	u32 TimeOut = 10000;

	if (!(IRQ & XAXIDMA_IRQ_ALL_MASK)){
		return;
	}

	if ((IRQ & XAXIDMA_IRQ_ERROR_MASK)){

		XAxiDma_Reset(&(dma_pkg_inst->dma_pl));
		while(TimeOut){

			if (XAxiDma_ResetIsDone(&(dma_pkg_inst->dma_pl))){
				xil_printf("FAILURE IRQ!!!!\r\n");
				break;
			}
			TimeOut -= 1;
		}
	}

	if ((IRQ & (XAXIDMA_IRQ_DELAY_MASK | XAXIDMA_IRQ_IOC_MASK))){

		int proc_bds = 0;

	XAxiDma_Bd* pointer_bd;
	XAxiDma_Bd *current_bd;
	XAxiDma_BdRingIntDisable(tx_ring, XAXIDMA_IRQ_ALL_MASK);

	proc_bds = XAxiDma_BdRingFromHw(tx_ring, XAXIDMA_ALL_BDS, &pointer_bd);

	uint32_t status_reg = 0;

	current_bd = pointer_bd;

	for (int i = 0; i < proc_bds; i++){
	status_reg = XAxiDma_BdGetSts(current_bd);

	if (status_reg & 0x70000000){
	xil_printf("[tx_irq] : %5d : bad descriptor : 0x%08x\r\n", i, current_bd);
	}

	if (!(status_reg & 0x80000000)){
	xil_printf("[tx_irq] : %5d : incomplete descriptor : 0x%08x\r\n", i, current_bd);
	}

	current_bd = (XAxiDma_Bd*)XAxiDma_BdRingNext(tx_ring, current_bd);
	}
	int xstatus = 0;
	xstatus = XAxiDma_BdRingFree(tx_ring, proc_bds, pointer_bd);
		if (xstatus != XST_SUCCESS){
			xil_printf("\r\nFailed to enqueue TxRing to HW processing\r\n");
		}

		XAxiDma_BdRingIntEnable(tx_ring, XAXIDMA_IRQ_ALL_MASK);
	}
	tx_flaq = 1;
	return ;
}




void usb_rx_intr_handler(void *callback){

	uint32_t irq;

	dma_pkg *dma_pkg_inst = (dma_pkg*)callback;

	XAxiDma_BdRing *rx_ring = XAxiDma_GetRxRing(&(dma_pkg_inst->dma_usb));

	irq = XAxiDma_BdRingGetIrq(rx_ring);

	if (!(irq & XAXIDMA_IRQ_ALL_MASK)){
		xil_printf("NO IRQ VECTOR\r\n");
		return;
	}

    uint32_t buffer_addr_reg = 0;
    uint32_t recv_size = 0;
    uint32_t status_reg = 0;


	XAxiDma_BdRingAckIrq(rx_ring, irq);

	if ((irq & XAXIDMA_IRQ_ERROR_MASK)){
		xil_printf("[err] : irq event for error event\r\n");
		return ;
	}

	if ((irq & (XAXIDMA_IRQ_DELAY_MASK | XAXIDMA_IRQ_IOC_MASK))){

		/**** PROCESSING PKT ****/
	    int processed_bds = 0;
	    int free_cnt_bd = 0;


	    XAxiDma_Bd *pointer_bd;
	    XAxiDma_Bd *current_bd;

	    processed_bds = XAxiDma_BdRingFromHw(rx_ring, XAXIDMA_ALL_BDS, &pointer_bd);
	    if (processed_bds == 0){
	        return;
	    }

	    XAxiDma_BdRingIntDisable(rx_ring, XAXIDMA_IRQ_ALL_MASK);

	    current_bd = pointer_bd;

	    int xstatus = 0;

	    pkt *rx_pkt_buffer = malloc(DMA_USB_RX_BD_CNT);
	    int pkt_index = 0;


	    pkt rx_pkt;

	    rx_pkt.completed = 0;
	    rx_pkt.size = 0;


	    for (int i = 0; i < processed_bds; i++){
	        buffer_addr_reg = XAxiDma_BdGetBufAddr(current_bd);
	        recv_size = XAxiDma_BdGetActualLength(current_bd, (~XAXIDMA_BD_STS_ALL_MASK));
	        status_reg = XAxiDma_BdGetSts(current_bd);

	        if (status_reg & XAXIDMA_BD_STS_COMPLETE_MASK){
	        	if (status_reg & XAXIDMA_BD_STS_RXSOF_MASK){
	        		rx_pkt.baseaddr = buffer_addr_reg;
	    			rx_pkt.size = recv_size;
	        	}
	        	else{
	        		rx_pkt.size += recv_size;
	        	}

	        	if (status_reg & XAXIDMA_BD_STS_RXEOF_MASK){
					rx_pkt_buffer[pkt_index].baseaddr = rx_pkt.baseaddr;
					rx_pkt_buffer[pkt_index].size = rx_pkt.size;
					rx_pkt_buffer[pkt_index].completed = 1;
					pkt_index++;
	    		}
	        }

	        if (status_reg & XAXIDMA_BD_STS_ALL_ERR_MASK){

	        }

	        current_bd = (XAxiDma_Bd*)XAxiDma_BdRingNext(rx_ring, current_bd);
	    }


		init_dma_pl_multi_tx(rx_pkt_buffer, pkt_index, &(dma_pkg_inst->dma_pl));

	    while (tx_flaq == 0){

	    }

	    xstatus = XAxiDma_BdRingFree(rx_ring, processed_bds, pointer_bd);
	    if (xstatus != XST_SUCCESS){
	        xil_printf("\r\nFailed to Free RxRing\r\n");
	    }

	    free_cnt_bd = rx_ring->FreeCnt;

	    xstatus = XAxiDma_BdRingAlloc(rx_ring, free_cnt_bd, &pointer_bd);
	    if (xstatus != XST_SUCCESS){
	        xil_printf("\r\nFailed to Allocate RxRing's BDs_ARCV\r\n");
	    }


	    tx_flaq = 0;

	    free(rx_pkt_buffer);

	    xstatus = XAxiDma_BdRingToHw(rx_ring, free_cnt_bd, pointer_bd);
	    if (xstatus != XST_SUCCESS){
	        xil_printf("\r\nFailed To Enqueue to Hardware processing BDs\r\n");
	    }

	    XAxiDma_BdRingIntEnable(rx_ring, XAXIDMA_IRQ_ALL_MASK);

	}

	return ;
}



void dbg_print_memspace(){
	xil_printf("\r\n\r\n");
	xil_printf("********** MEMORY SPACE DEBUG INFO **********");
	xil_printf("\r\n");

	xil_printf("-- USB TX CHANNEL --\r\n");
	xil_printf("USB MM2S BUFFERBASE : 0x%08x\r\n", DMA_USB_TX_BUFFER_BASE);
	xil_printf("USB MM2S BUFFERHIGH : 0x%08x\r\n", DMA_USB_TX_BUFFER_HIGH);
	xil_printf("USB MM2S BD COUNT   : %d\r\n", DMA_USB_TX_BD_CNT);
	xil_printf("USB MM2S BUF PER BD : %d\r\n", DMA_USB_TX_BUF_PER_BD);
	xil_printf("USB MM2S BD SIZE    : %d\r\n", DMA_USB_TX_BDSIZE);
	xil_printf("USB MM2S BDBASE     : 0x%08x\r\n", DMA_USB_TX_BD_BASE);
	xil_printf("USB MM2S BDHIGH     : 0x%08x\r\n", DMA_USB_TX_BD_HIGH);
	xil_printf("USB MM2S BUFFERSIZE : %d\r\n", DMA_USB_TX_BUFFER_HIGH - DMA_USB_TX_BUFFER_BASE);
	xil_printf("USB MM2S BDRINGSIZE : %d\r\n", DMA_USB_TX_BD_HIGH - DMA_USB_TX_BD_BASE);

	xil_printf("-- USB RX CHANNEL --\r\n");
	xil_printf("USB S2MM BUFFERBASE : 0x%08x\r\n", DMA_USB_RX_BUFFER_BASE);
	xil_printf("USB S2MM BUFFERHIGH : 0x%08x\r\n", DMA_USB_RX_BUFFER_HIGH);
	xil_printf("USB S2MM BD COUNT   : %d\r\n", DMA_USB_RX_BD_CNT);
	xil_printf("USB S2MM BUF PER BD : %d\r\n", DMA_USB_RX_BUF_PER_BD);
	xil_printf("USB S2MM BD SIZE    : %d\r\n", DMA_USB_RX_BDSIZE);
	xil_printf("USB S2MM BDBASE     : 0x%08x\r\n", DMA_USB_RX_BD_BASE);
	xil_printf("USB S2MM BDHIGH     : 0x%08x\r\n", DMA_USB_RX_BD_HIGH);
	xil_printf("USB S2MM BUFFERSIZE : %d\r\n", DMA_USB_RX_BUFFER_HIGH - DMA_USB_RX_BUFFER_BASE);
	xil_printf("USB S2MM BDRINGSIZE : %d\r\n", DMA_USB_RX_BD_HIGH - DMA_USB_RX_BD_BASE);

	xil_printf("-- PL TX CHANNEL --\r\n");
	xil_printf("PL MM2S BUFFERBASE  : 0x%08x\r\n", DMA_PL_TX_BUFFER_BASE);
	xil_printf("PL MM2S BUFFERHIGH  : 0x%08x\r\n", DMA_PL_TX_BUFFER_HIGH);
	xil_printf("PL MM2S BD COUNT    : %d\r\n", DMA_PL_TX_BD_CNT);
	xil_printf("PL MM2S BUF PER BD  : %d\r\n", DMA_PL_TX_BUF_PER_BD);
	xil_printf("PL MM2S BD SIZE     : %d\r\n", DMA_PL_TX_BDSIZE);
	xil_printf("PL MM2S BDBASE      : 0x%08x\r\n", DMA_PL_TX_BD_BASE);
	xil_printf("PL MM2S BDHIGH      : 0x%08x\r\n", DMA_PL_TX_BD_HIGH);
	xil_printf("PL MM2S BUFFERSIZE  : %d\r\n", DMA_PL_TX_BUFFER_HIGH - DMA_PL_TX_BUFFER_BASE);
	xil_printf("PL MM2S BDRINGSIZE  : %d\r\n", DMA_PL_TX_BD_HIGH - DMA_PL_TX_BD_BASE);

	xil_printf("-- PL RX CHANNEL --\r\n");
	xil_printf("PL S2MM BUFFERBASE  : 0x%08x\r\n", DMA_PL_RX_BUFFER_BASE);
	xil_printf("PL S2MM BUFFERHIGH  : 0x%08x\r\n", DMA_PL_RX_BUFFER_HIGH);
	xil_printf("PL S2MM BD COUNT    : %d\r\n", DMA_PL_RX_BD_CNT);
	xil_printf("PL S2MM BUF PER BD  : %d\r\n", DMA_PL_RX_BUF_PER_BD);
	xil_printf("PL S2MM BD SIZE     : %d\r\n", DMA_PL_RX_BDSIZE);
	xil_printf("PL S2MM BDBASE      : 0x%08x\r\n", DMA_PL_RX_BD_BASE);
	xil_printf("PL S2MM BDHIGH      : 0x%08x\r\n", DMA_PL_RX_BD_HIGH);
	xil_printf("PL S2MM BUFFERSIZE  : %d\r\n", DMA_PL_RX_BUFFER_HIGH - DMA_PL_RX_BUFFER_BASE);
	xil_printf("PL S2MM BDRINGSIZE  : %d\r\n", DMA_PL_RX_BD_HIGH - DMA_PL_RX_BD_BASE);

}



