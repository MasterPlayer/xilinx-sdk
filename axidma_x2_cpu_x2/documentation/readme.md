# axidma_x2_cpu_x2

This application demonstrates how to work with two controllers AXIDMA in xilinx Zynq-7000 SoC in two processing cores over interrupts.
The block design for presented in file zturn_bd.pdf
The using address space presented in file Table.xslx

## Interrupts

interrupts has presented as private: 
1) **usb_rx** - private interrupt signal for cpu0 ( USB_S2MM_INTR ). 
    source of signal - axidma_usb channel stream-to-memory-mapped(S2MM). 
    indicates complete receive packets(one or more) from PL to DDR memory or error in channel S2MM
2) **usb_tx** - private interrupt signal for cpu1 ( USB_MM2S_INTR )
    source of signal - axidma_usb channel memory-mapped-to-stream channel(MM2S)
    indicates complete transmission packets(one or more) from DDR memory to PL or error in channel MM2S.
    *This interrupt is hignest for CPU1*. if cpu1 perform processing other interrupt, it will be suspended for processing this interrupt
3) **pl_rx** - fast private interrupt signal for cpu1 ( PL_S2MM_INTR )
    source of signal - axidma_pl channel stream-to-memory-mapped(S2MM). 
    indicates complete receive packets(one or more) from PL to DDR memory or error in channel S2MM
4) **pl_tx** - fast private interrupt signal for cpu0 ( PL_MM2S_INTR )
    source of signal - axidma_usb channel memory-mapped-to-stream channel(MM2S)
    indicates complete transmission packets(one or more) from DDR memory to PL or error in channel MM2S.
    *This interrupt is hignest for CPU0*. if cpu1 perform processing other interrupt, it will be suspended for processing this interrupt
    
## Address space
size of DDR memory on board is 1 GByte. Type - DDR3
Each channel of DMA can access any memory location of DDR. For additional information see document Table.xslx

## Program
after start, cpu0 configure interrupt controller, configure shareable memory, which defines in L2 Cache, and send signal to CPU1.
cpu1 configure shareable memory, interrupt controller and wait signal from cpu0.
cpu0 configure axidma_usb controller and save configuration in shareable memory for transmission to cpu1. After that cpu0 wait for cpu1 
cpu1 receive and apply configuration for axi dma and send signal to cpu0.
cpu0 configure axidma_pl controller and save configuration in shareable memory for transmission to cpu1 and waiting signal from cpu1
cpu1 receive and apply configuration for axi dma and send signal to cpu1

after that, two processing cores go to wait state

This program demonstrate perform two axi dma controllers in two cpu for loopback chain
Route of data is next 
1. Data, which presented one of more packets receive over AXIDMA_USB S2MM channel.
2. AXIDMA_USB S2MM channel raises s2mm interrupt flaq
3. Processing system, CPU0 process the interrupt and initialize transfer over AXIDMA_PL. 
4. CPU0 process interrupt from PL_MM2S channel, 
5. CPU1 process interrupt from PL_S2MM channel, initialize transmission over AXIDMA_USB MM2S channel. 
6. CPU1 process interrupt from USB_MM2S channel

Finally, the route of data can be present as follows (according with current block design, presented in file zturn_bd.pdf
USB_RX -> PL_TX -> PL_RX -> USB_TX
the current implementation of the program also allows you to independently transmit data in the following way:
1) CPU0 : USB_RX -> PL_TX
2) CPU1 : PL_RX -> USB_TX

## Testing
testing execute for board MYIR MYS-7z020-C-S. 
Estimated speed is over 99% of theoretical throughput of axidma channels
