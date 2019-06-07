#include "dma_defs.h"

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



int dma_usb_rx_init (XAxiDma *dma_inst){
    int status = 0;
    XAxiDma_BdRing * rx_ring = XAxiDma_GetRxRing(dma_inst);
    int index = 0;
    int count_free_bd = 0;

    uint32_t rx_buffer_ptr;

    XAxiDma_Bd* current_bd      ;
    XAxiDma_Bd* pointer_bd      ;
    XAxiDma_Bd  template_bd     ;

    XAxiDma_BdRingIntDisable(rx_ring, XAXIDMA_IRQ_ALL_MASK);
    int count_bd = DMA_USB_RX_BD_CNT;
    xil_printf("[DMA_USB_RX_INIT] : create ring.....");
    status = XAxiDma_BdRingCreate(rx_ring, DMA_USB_RX_BD_BASE, DMA_USB_RX_BD_BASE, DMA_USB_RX_BDSIZE, count_bd);
    if (status != XST_SUCCESS){
        xil_printf("FAIL\r\n", status);
        return XST_FAILURE;
    }
    else
    {
    	xil_printf("complete\r\n");
    }

    XAxiDma_BdClear(&template_bd);
    xil_printf("[DMA_USB_RX_INIT] : clone bd ring.....");

    status = XAxiDma_BdRingClone(rx_ring, &template_bd);
    if (status != XST_SUCCESS){
        xil_printf("FAIL\r\n", status);
        return XST_FAILURE;
    }
    else{
    	xil_printf("complete\r\n");
    }


    count_free_bd = rx_ring->FreeCnt;

    xil_printf("[DMA_USB_RX_INIT] : alloc bd ring.....");
    status = XAxiDma_BdRingAlloc(rx_ring, count_free_bd, &pointer_bd);
    if (status != XST_SUCCESS){
        xil_printf("FAIL\r\n", status);
        return XST_FAILURE;
    }
    else{
    	xil_printf("complete\r\n");
    }

    current_bd = pointer_bd;

    rx_buffer_ptr = DMA_USB_RX_BUFFER_BASE;

    xil_printf("[DMA_USB_RX_INIT] : bd initialize.....");

    for (index = 0; index < count_free_bd; index++){
        status = XAxiDma_BdSetBufAddr(current_bd, rx_buffer_ptr);
        if (status != XST_SUCCESS){
            xil_printf("SET BUF ADDR FAIL\r\n");
            return XST_FAILURE;
        }
        status = XAxiDma_BdSetLength(current_bd, DMA_USB_RX_BUF_PER_BD, 67108864);
        if (status != XST_SUCCESS){
        	xil_printf("SET LEN FAIL\r\n");
            return XST_FAILURE;
        }
        rx_buffer_ptr += DMA_USB_RX_BUF_PER_BD;
        current_bd = (XAxiDma_Bd*)XAxiDma_BdRingNext(rx_ring, current_bd);
    }

	XAxiDma_BdRingSetCoalesce(rx_ring, 0xFF, 0xFF);

    xil_printf("complete\r\n");

    status = XAxiDma_BdRingToHw(rx_ring, count_free_bd, pointer_bd);
    if (status != XST_SUCCESS){
        xil_printf("failed. iteration : %d\r\n", index);
        return XST_FAILURE;
    }
    xil_printf("complete\r\n");

    XAxiDma_BdRingIntEnable(rx_ring, XAXIDMA_IRQ_ALL_MASK);

    xil_printf("[DMA_USB_RX_INIT] : bd start.....\r\n");

    status = XAxiDma_BdRingStart(rx_ring);
    if (status != XST_SUCCESS){
    	xil_printf("FAIL\r\n");
        return XST_FAILURE;
    }
    return status;
}




int dma_usb_tx_init(XAxiDma *dma_inst){
    int status = 0;

   XAxiDma_BdRing* tx_ring = XAxiDma_GetTxRing(dma_inst);

   XAxiDma_Bd              template_bd;

   XAxiDma_BdRingIntDisable(tx_ring, XAXIDMA_IRQ_ALL_MASK);

   int count_bd = DMA_USB_TX_BD_CNT;

   status = XAxiDma_BdRingCreate(tx_ring, DMA_USB_TX_BD_BASE, DMA_USB_TX_BD_BASE, DMA_USB_TX_BDSIZE, count_bd);


   XAxiDma_BdClear(&template_bd);


   status = XAxiDma_BdRingClone(tx_ring, &template_bd);

   XAxiDma_BdRingSetCoalesce(tx_ring, 0xFF, 0xFF);

   XAxiDma_BdRingIntEnable(tx_ring, XAXIDMA_IRQ_ALL_MASK);


   status = XAxiDma_BdRingStart(tx_ring);

   return 0;
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



int dma_pl_rx_init (XAxiDma *dma_inst){
    int status = 0;
    XAxiDma_BdRing * rx_ring = XAxiDma_GetRxRing(dma_inst);
    int index = 0;
    int count_free_bd = 0;

    uint32_t rx_buffer_ptr;

    XAxiDma_Bd* current_bd      ;
    XAxiDma_Bd* pointer_bd      ;
    XAxiDma_Bd  template_bd     ;

    XAxiDma_BdRingIntDisable(rx_ring, XAXIDMA_IRQ_ALL_MASK);
    int count_bd = DMA_PL_RX_BD_CNT;
    xil_printf("[DMA_PL_RX_INIT] : create ring.....");
    status = XAxiDma_BdRingCreate(rx_ring, DMA_PL_RX_BD_BASE, DMA_PL_RX_BD_BASE, DMA_PL_RX_BDSIZE, count_bd);
    if (status != XST_SUCCESS){
        xil_printf("FAIL\r\n", status);
        return XST_FAILURE;
    }
    else
    {
    	xil_printf("complete\r\n");
    }

    XAxiDma_BdClear(&template_bd);
    xil_printf("[DMA_PL_RX_INIT] : clone bd ring.....");

    status = XAxiDma_BdRingClone(rx_ring, &template_bd);
    if (status != XST_SUCCESS){
        xil_printf("FAIL\r\n", status);
        return XST_FAILURE;
    }
    else{
    	xil_printf("complete\r\n");
    }


    count_free_bd = rx_ring->FreeCnt;

    xil_printf("[DMA_USB_PL_INIT] : alloc bd ring.....");
    status = XAxiDma_BdRingAlloc(rx_ring, count_free_bd, &pointer_bd);
    if (status != XST_SUCCESS){
        xil_printf("FAIL\r\n", status);
        return XST_FAILURE;
    }
    else{
    	xil_printf("complete\r\n");
    }

    current_bd = pointer_bd;

    rx_buffer_ptr = DMA_PL_RX_BUFFER_BASE;

    xil_printf("[DMA_USB_PL_INIT] : bd initialize.....");

    for (index = 0; index < count_free_bd; index++){
        status = XAxiDma_BdSetBufAddr(current_bd, rx_buffer_ptr);
        if (status != XST_SUCCESS){
            xil_printf("SET BUF ADDR FAIL\r\n");
            return XST_FAILURE;
        }
        status = XAxiDma_BdSetLength(current_bd, DMA_PL_RX_BUF_PER_BD, 67108864);
        if (status != XST_SUCCESS){
        	xil_printf("SET LEN FAIL\r\n");
            return XST_FAILURE;
        }
        rx_buffer_ptr += DMA_PL_RX_BUF_PER_BD;
        current_bd = (XAxiDma_Bd*)XAxiDma_BdRingNext(rx_ring, current_bd);
    }

    xil_printf("complete\r\n");

    status = XAxiDma_BdRingToHw(rx_ring, count_free_bd, pointer_bd);
    if (status != XST_SUCCESS){
        xil_printf("failed. iteration : %d\r\n", index);
        return XST_FAILURE;
    }
    xil_printf("complete\r\n");

    XAxiDma_BdRingIntEnable(rx_ring, XAXIDMA_IRQ_ALL_MASK);

    xil_printf("[DMA_PL_RX_INIT] : bd start.....\r\n");

    status = XAxiDma_BdRingStart(rx_ring);
    if (status != XST_SUCCESS){
    	xil_printf("FAIL\r\n");
        return XST_FAILURE;
    }

    return status;
}




int dma_pl_tx_init(XAxiDma *dma_inst){
    int status = 0;

   XAxiDma_BdRing* tx_ring = XAxiDma_GetTxRing(dma_inst);

   XAxiDma_Bd              template_bd;

   XAxiDma_BdRingIntDisable(tx_ring, XAXIDMA_IRQ_ALL_MASK);

   int count_bd = DMA_PL_TX_BD_CNT;

   status = XAxiDma_BdRingCreate(tx_ring, DMA_PL_TX_BD_BASE, DMA_PL_TX_BD_BASE, DMA_PL_TX_BDSIZE, count_bd);

   XAxiDma_BdClear(&template_bd);

   status = XAxiDma_BdRingClone(tx_ring, &template_bd);

   XAxiDma_BdRingIntEnable(tx_ring, XAXIDMA_IRQ_ALL_MASK);

   status = XAxiDma_BdRingStart(tx_ring);

   return 0;
}


int     init_dma_pl_transmission(pkt pkt_inst, XAxiDma *dma_inst_ptr){

//	Xil_DCacheInvalidateRange(DMA_USB_TX_BUFFER_ADDRESS, pkt_inst.size);
	memcpy((uint8_t*)DMA_PL_TX_BUFFER_BASE, (uint8_t*)pkt_inst.baseaddr, pkt_inst.size);
//	Xil_DCacheFlushRange(DMA_PL_TX_BUFFER_BASE, pkt_inst.size)     ; // now

	int status = 0;
	XAxiDma_BdRing* tx_ring = XAxiDma_GetTxRing(dma_inst_ptr);

	XAxiDma_Bd      *pointer_bd;
	XAxiDma_Bd      *current_bd;

	uint32_t buffer_address_inc     = DMA_PL_TX_BUFFER_BASE;
	int needed_bds = pkt_inst.size/DMA_PL_TX_BUF_PER_BD;
	int fully_used_bds = needed_bds * DMA_PL_TX_BUF_PER_BD;
	if (pkt_inst.size > fully_used_bds){
		needed_bds++;
	}
	int tail = pkt_inst.size - ((needed_bds-1) * DMA_PL_TX_BUF_PER_BD);

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
			status = XAxiDma_BdSetLength(current_bd, DMA_PL_TX_BUF_PER_BD, tx_ring->MaxTransferLen);
			if (status != XST_SUCCESS){
					return XST_FAILURE;
			}

		}
		if (tail != 0){
			if (index != needed_bds-1){

				if (index == 0){
					control_reg |= XAXIDMA_BD_CTRL_TXSOF_MASK;
				}

				status = XAxiDma_BdSetLength(current_bd, DMA_PL_TX_BUF_PER_BD, tx_ring->MaxTransferLen);
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

		buffer_address_inc +=  DMA_PL_TX_BUF_PER_BD;

		current_bd = (XAxiDma_Bd*)XAxiDma_BdRingNext(tx_ring, current_bd);
	}

	status = XAxiDma_BdRingToHw(tx_ring, needed_bds, pointer_bd);
	if (status != XST_SUCCESS){
		return XST_FAILURE;
	}

	return 0;

}



int     init_dma_pl_multi_tx(pkt *pkt_inst, int cnt ,XAxiDma *dma_inst_ptr){

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
	if (status != XST_SUCCESS){
		return XST_FAILURE;
	}

	return 0;

}

