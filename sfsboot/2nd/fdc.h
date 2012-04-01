#ifndef __FDC_H__
#define __FDC_H__

#define FDC_CMD		0x3F0
#define FDC_STAT	0x3f4
#define FDC_DATA	0x3f5
#define FDC_EXTERNAL	0xBE	/* 外部レジスタへのアドレス */
#define FDC_WCNTL	0x3f2


#define STATUS_BUSYMASK	0x0F	/* drive busy mask */
#define FD0_BUSY	0x01
#define FD1_BUSY	0x02
#define FD2_BUSY	0x04
#define FD3_BUSY	0x08
#define FDC_BUSY	0x10
#define FDC_DIN		0x40
#define FDC_DOUT	0x00
#define FDC_DCR		0x3f7	/* Diskette Control Register (write)*/
/*#define FDC_NDM		0x20 */
#define FDC_MASTER	0x80

/* FDC COMMANDS */
#define FDC_SPECIFY	0x03
#define FDC_READ	0xE6	/* <MT><MF><SK> 0 0 1 1 0 */
#define FDC_SEEK	0x0f
#define FDC_RECALIBRATE	0x07
#define FDC_DEVSTAT	0x04
#define FDC_SENSE	0x08
#define FDC_DEVICE_STAT	0x040


#define FD0		0x00
#define FD1		0x01

#define HEAD0		0x0
#define HEAD1		0x1

#define MOTOR_SHIFT	4


/* SUPPORT 1.44MByte floppy disk */

#define HD_TYPE		0
#define HD_HEAD		2
#define HD_EXTERN	3
#define HD_SECTOR	18
#define HD_LENGTH	512             
#define HD_LENCODE	2               /* 512bytes/sector */
#define HD_GAP		0x1B		/* HD の場合の GAP3 の値 */
#define HD_DTL		0xff

#define DD_TYPE		1

/* Write control registers bit. IO port address 0x94 */
#define FDC_RESET	0x80
#define FDC_FRY		0x40
#define FDC_DMAE	0x10
#define FDC_MTON	0x08

/* External status bit. IO port address 0x94 */
#define FDC_FINT1	0x80
#define FDC_FINT0	0x40
#define FDC_DMACH	0x20
#define FDC_PSTB	0x10

#define FD_DMA_BUFF     0x8000

#endif
