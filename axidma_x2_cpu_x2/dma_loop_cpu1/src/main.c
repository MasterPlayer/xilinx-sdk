#include <stdio.h>

#include "platform.h"
#include "platform_config.h"
#include "xaxidma.h"
#include "xil_exception.h"
#include "dma_defs.h"
#include "xscugic.h"
#include "xil_mmu.h"


int     scugic_initialize(XScuGic *gic_inst, dma_pkg *dma_pkg_inst_ptr);

void dma_from_cache(XAxiDma* dma_inst_ptr){
	memcpy(dma_inst_ptr, 0xFFFF0000, sizeof(XAxiDma));
}

int tx_flaq;


int main() {

	init_platform();

    tx_flaq = 0;

	Xil_L1DCacheDisable();

    xil_printf("CPU1 :: init memory attr\r\n");
    Xil_SetTlbAttributes(0xFFFF0000,0x14de2);           // S=b1 TEX=b100 AP=b11, Domain=b1111, C=b0, B=b0

    xil_printf("CPU1 :: wait cpu0\r\n");
	while(CPU1_EN_FLAQ == 0){

	}
	xil_printf("CPU1 :: wait finished\r\n");
	CPU1_EN_FLAQ = 0;

    dma_pkg dma_pkg_0;

    XScuGic scugic_0;

    int status = 0;

    xil_printf("CPU1 :: init gic\r\n");
    status = scugic_initialize(&scugic_0, &dma_pkg_0);
    if (status != XST_SUCCESS){
    	return -1;
    }

    xil_printf("CPU1 :: wait cpu0\r\n");

    CPU0_EN_FLAQ = 1;
    while(CPU1_EN_FLAQ == 0){

    }
    CPU1_EN_FLAQ = 0;

    xil_printf("CPU1 :: wait finished\r\n");

    xil_printf("CPU1 :: restore from cache usb\r\n");

    dma_from_cache(&dma_pkg_0.dma_usb);

    CPU0_EN_FLAQ = 1;

    xil_printf("CPU1 :: wait cpu0\r\n");

    while(CPU1_EN_FLAQ == 0){}

    xil_printf("CPU1 :: wait finished\r\n");
    CPU1_EN_FLAQ = 0;

    xil_printf("CPU1 :: restore from cache pl\r\n");

    dma_from_cache(&dma_pkg_0.dma_pl);

    CPU0_EN_FLAQ = 1;

    while(1) {

	}

    cleanup_platform();

    return 0;
}



int dma_usb_init(XAxiDma *dma_inst){

    int status = 0;

    xil_printf("[DMA_USB_INIT] : lookup cfg.....");
    XAxiDma_Config* cfg_dma_0 = XAxiDma_LookupConfig(DMA_USB_DEV);
    if (!cfg_dma_0){
        xil_printf("FAIL\r\n");
        return -1;
    }
    else{
    	xil_printf("complete\r\n");
    }

    xil_printf("[DMA_USB_INIT] : init cfg.....");
    XAxiDma_CfgInitialize(dma_inst, cfg_dma_0);
    if (status != XST_SUCCESS){
        xil_printf("FAIL\r\n");
        return status;
    }
    else{
    	xil_printf("complete\r\n");
    }

    xil_printf("[DMA_USB_INIT] : sg mode checking cfg.....");
    if (!XAxiDma_HasSg(dma_inst)){
        xil_printf("FAIL\r\n");
        return -2;
    }
    xil_printf("complete\r\n");

    return status;
}



int dma_pl_init(XAxiDma *dma_inst){

    int status = 0;

    xil_printf("[DMA_PL_INIT] : lookup cfg.....");
    XAxiDma_Config* cfg_dma_0 = XAxiDma_LookupConfig(DMA_PL_DEV);
    if (!cfg_dma_0){
        xil_printf("FAIL\r\n");
        return -1;
    }
    else{
    	xil_printf("complete\r\n");
    }

    xil_printf("[DMA_PL_INIT] : init cfg.....");
    XAxiDma_CfgInitialize(dma_inst, cfg_dma_0);
    if (status != XST_SUCCESS){
        xil_printf("FAIL\r\n");
        return status;
    }
    else{
    	xil_printf("complete\r\n");
    }

    xil_printf("[DMA_PL_INIT] : sg mode checking cfg.....");
    if (!XAxiDma_HasSg(dma_inst)){
        xil_printf("FAIL\r\n");
        return -2;
    }
    xil_printf("complete\r\n");

    return status;

}



void usb_tx_intr_handler(void *callback){
	u32 IRQ;
	dma_pkg *dma_pkg_inst = (dma_pkg*)callback;
	XAxiDma_BdRing* tx_ring = XAxiDma_GetTxRing(&(dma_pkg_inst->dma_usb));

	IRQ = XAxiDma_BdRingGetIrq(tx_ring);

	XAxiDma_BdRingAckIrq(tx_ring, IRQ);

	u32 TimeOut = 10000;

	if (!(IRQ & XAXIDMA_IRQ_ALL_MASK)){
		return;
	}

	if ((IRQ & XAXIDMA_IRQ_ERROR_MASK)){

		XAxiDma_Reset(&(dma_pkg_inst->dma_usb));
		while(TimeOut){

			if (XAxiDma_ResetIsDone(&(dma_pkg_inst->dma_usb))){
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
	return 0;
}



void pl_rx_intr_handler(void *callback){

	uint32_t irq;

	int flaq_compl = 0;
    uint32_t buffer_addr_reg = 0;
    uint32_t recv_size = 0;
    uint32_t status_reg = 0;


	dma_pkg *dma_pkg_inst = (dma_pkg*)callback;

	XAxiDma_BdRing *rx_ring = XAxiDma_GetRxRing(&(dma_pkg_inst->dma_pl));

	irq = XAxiDma_BdRingGetIrq(rx_ring);

	if (!(irq & XAXIDMA_IRQ_ALL_MASK)){
		xil_printf("NO IRQ VECTOR\r\n");
		return;
	}

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

	    pkt *rx_pkt_buffer = malloc(DMA_PL_RX_BD_CNT);
	    int pkt_index = 0;

	    pkt rx_pkt;
	    rx_pkt.completed = 0;
	    rx_pkt.size = 0;

	    for (int i = 0; i < processed_bds; i++){

	        buffer_addr_reg = XAxiDma_BdGetBufAddr(current_bd);
	        recv_size = XAxiDma_BdGetActualLength(current_bd, (~XAXIDMA_BD_STS_ALL_MASK));
	        status_reg = XAxiDma_BdGetSts(current_bd);

	        if (status_reg & XAXIDMA_BD_STS_ALL_ERR_MASK){

	        }

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

	        current_bd = (XAxiDma_Bd*)XAxiDma_BdRingNext(rx_ring, current_bd);
	    }

		init_dma_usb_multi_tx(rx_pkt_buffer, pkt_index, &(dma_pkg_inst->dma_usb));

	    while(tx_flaq == 0){

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

	    xstatus = XAxiDma_BdRingToHw(rx_ring, free_cnt_bd, pointer_bd);
	    if (xstatus != XST_SUCCESS){
	        xil_printf("\r\nFailed To Enqueue to Hardware processing BDs\r\n");
	    }

	    XAxiDma_BdRingIntEnable(rx_ring, XAXIDMA_IRQ_ALL_MASK);

	}

	return ;
}




int init_dma_usb_transmission(pkt pkt_inst, XAxiDma *dma_inst_ptr){

//

//	Xil_DCacheInvalidateRange(DMA_USB_TX_BUFFER_ADDRESS, pkt_inst.size);

//    memmove((uint8_t*)DMA_USB_TX_BUFFER_ADDRESS, (uint8_t*)pkt_inst.baseaddr, pkt_inst.size);
    memcpy((uint8_t*)DMA_USB_TX_BUFFER_BASE, (uint8_t*)pkt_inst.baseaddr, pkt_inst.size);
    Xil_DCacheFlushRange(DMA_USB_TX_BUFFER_BASE, pkt_inst.size)     ;

    int status = 0;
    XAxiDma_BdRing* tx_ring = XAxiDma_GetTxRing(dma_inst_ptr);

    XAxiDma_Bd      *pointer_bd;
    XAxiDma_Bd      *current_bd;

    uint32_t buffer_address_inc     = DMA_USB_TX_BUFFER_BASE;
    int needed_bds = pkt_inst.size/DMA_USB_TX_BUF_PER_BD;
    int fully_used_bds = needed_bds * DMA_USB_TX_BUF_PER_BD;
    if (pkt_inst.size > fully_used_bds){
        needed_bds++;
    }
    int tail = pkt_inst.size - ((needed_bds-1) * DMA_USB_TX_BUF_PER_BD);

    uint32_t  control_reg               = 0;
    int index = 0;

    status = XAxiDma_BdRingAlloc(tx_ring, needed_bds, &pointer_bd);
    if (status != XST_SUCCESS){
        return XST_FAILURE;
    }
    current_bd = pointer_bd;
    for (index = 0; index < needed_bds; index++){

        control_reg = 0;
        status = XAxiDma_BdSetBufAddr(current_bd, buffer_address_inc);
        if (status != XST_SUCCESS){
            return XST_FAILURE;
        }

        if (tail == 0){
            if (index == 0){
                control_reg |= XAXIDMA_BD_CTRL_TXSOF_MASK;
            }
            if (index == needed_bds-1){
                control_reg |= XAXIDMA_BD_CTRL_TXEOF_MASK;
            }
            status = XAxiDma_BdSetLength(current_bd, DMA_USB_TX_BUF_PER_BD, tx_ring->MaxTransferLen);
            if (status != XST_SUCCESS){
                    return XST_FAILURE;
            }

        }
        if (tail != 0){
            if (index != needed_bds-1){

                if (index == 0){
                    control_reg |= XAXIDMA_BD_CTRL_TXSOF_MASK;
                }

                status = XAxiDma_BdSetLength(current_bd, DMA_USB_TX_BUF_PER_BD, tx_ring->MaxTransferLen);
                if (status != XST_SUCCESS){

                    return XST_FAILURE;
                }

            }
            if (index == needed_bds-1){
                control_reg |= XAXIDMA_BD_CTRL_TXEOF_MASK;
                control_reg |= XAXIDMA_BD_CTRL_TXSOF_MASK;
                status = XAxiDma_BdSetLength(current_bd, tail, tx_ring->MaxTransferLen);
                if (status != XST_SUCCESS){

                return XST_FAILURE;
                }
            }
        }

        XAxiDma_BdSetCtrl(current_bd, control_reg);

        control_reg = 0;

        buffer_address_inc +=  DMA_USB_TX_BUF_PER_BD;

        current_bd = (XAxiDma_Bd*)XAxiDma_BdRingNext(tx_ring, current_bd);
    }

    status = XAxiDma_BdRingToHw(tx_ring, needed_bds, pointer_bd);
    if (status != XST_SUCCESS){
        return XST_FAILURE;
    }

    return 0;
}



int     scugic_initialize(XScuGic *gic_inst_ptr, dma_pkg *dma_pkg_inst_ptr){

    int status = 0;

    XScuGic_Config *cfg;

    cfg = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);

    status = XScuGic_CfgInitialize(gic_inst_ptr, cfg, cfg->CpuBaseAddress);

    status = XScuGic_Connect(gic_inst_ptr, PL_RX_INTR, (Xil_InterruptHandler)pl_rx_intr_handler, dma_pkg_inst_ptr);

	XScuGic_Enable(gic_inst_ptr, PL_RX_INTR);
	XScuGic_Enable(gic_inst_ptr, USB_TX_INTR);

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, gic_inst_ptr);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_FIQ_INT, (Xil_ExceptionHandler)usb_tx_intr_handler, dma_pkg_inst_ptr);
	Xil_ExceptionEnableMask(XIL_EXCEPTION_ALL);

	Xil_ExceptionEnable();
    return status ;
}





int     init_dma_usb_multi_tx(pkt *pkt_inst, int cnt ,XAxiDma *dma_inst_ptr){

	int status = 0;

	XAxiDma_BdRing* tx_ring = XAxiDma_GetTxRing(dma_inst_ptr);

	XAxiDma_Bd      *pointer_bd;
	XAxiDma_Bd      *current_bd;

	uint32_t  control_reg = 0;
	int index = 0;

	status = XAxiDma_BdRingAlloc(tx_ring, cnt, &pointer_bd);
	if (status != XST_SUCCESS){
		return XST_FAILURE;
	}

	current_bd = pointer_bd;
//
//	xil_printf("***** OUT FOR ALL PKTS WHICH NEED TO TRANSMIT *****\r\n");
//	for (index = 0; index < cnt; index++){
//		xil_printf("[%3d] : 0x%08x::%5d::%d\r\n", index, pkt_inst[index].baseaddr, pkt_inst[index].size, pkt_inst[index].completed);
//	}

	for ( index = 0 ; index < cnt; index++ ){
		control_reg = 0;
		XAxiDma_BdSetBufAddr(current_bd, pkt_inst[index].baseaddr);
		XAxiDma_BdSetLength(current_bd, pkt_inst[index].size, tx_ring->MaxTransferLen);
		control_reg |= XAXIDMA_BD_CTRL_TXSOF_MASK;
		control_reg |= XAXIDMA_BD_CTRL_TXEOF_MASK;
		XAxiDma_BdSetCtrl(current_bd, control_reg);
		current_bd = (XAxiDma_Bd*)XAxiDma_BdRingNext(tx_ring, current_bd);
	}

	status = XAxiDma_BdRingToHw(tx_ring, cnt, pointer_bd);

	return 0;
}



