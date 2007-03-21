#ifndef HARDWARE_H
#define HARDWARE_H
//==============================================================================
// Copyright (c) 2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
// File:
//		Hardware.h
//
// Description:
//		Hardware definitions for the Samsung 2460 chip in the Dimension console.
//==============================================================================


//==============================================================================
// Hardware register access functions
//==============================================================================

#define ReadRegisterU8(addr)			((U8)(*((volatile U8 *)addr)))
#define WriteRegisterU8(addr, val)		*((volatile U8 *)addr) = val
#define ClearBitU8(addr,mask)			*((volatile U8 *)addr) &= ~mask
#define SetBitU8(addr,mask)			*((volatile U8 *)addr) |= mask

#define ReadRegisterU16(addr)			((U16)(*((volatile U16 *)addr)))
#define WriteRegisterU16(addr, val)	*((volatile U16 *)addr) = val
#define ClearBitU16(addr,mask)			*((volatile U16 *)addr) &= ~mask
#define SetBitU16(addr,mask)			*((volatile U16 *)addr) |= mask

#define ReadRegisterU32(addr)			((U32)(*((volatile U32 *)addr)))
#define WriteRegisterU32(addr, val)	*((volatile U32 *)addr) = val
#define ClearBitU32(addr,mask)			*((volatile U32 *)addr) &= ~mask
#define SetBitU32(addr,mask)			*((volatile U32 *)addr) |= mask


//---------------------------------------------------------------------------
// General purpose I/O
//---------------------------------------------------------------------------

#define kRegGpACon		0x44800000
#define kRegGpADat		0x44800004
#define kRegGpAPu		0x44800008
#define kRegGpASlpCon	0x4480000C

#define kRegGpBCon		0x44800010
#define kRegGpBDat		0x44800014
#define kRegGpBPu		0x44800018
#define kRegGpBSlpCon	0x4480001C

// Eventually add more register definitions here, but for now what we really need is GpH
#define kRegGpHCon		0x44800070
#define kRegGpHDat		0x44800074
#define kRegGpHPu		0x44800078
#define kRegGpHSlpCon	0x4480007C

#define kSpiNssBit		0x8000

#define kRegGpJCon		0x44800090
#define kRegGpJDat		0x44800094
#define kRegGpJPu		0x44800098


//---------------------------------------------------------------------------
//  Interrupt control
//---------------------------------------------------------------------------

// Vector numbers - need to expand
#define kIntVectorExt03 0
#define kIntVectorDMA	 20
#define kIntVectorSPI	 23

// Top level interrupt registers
#define kRegIrqSourcePending		0x40200000
#define kRegIrqMask				0x40200008
#define kRegIrqInterruptPending	0x40200010

#define kIrqDMABit				(1<<kIntVectorDMA)
#define kIrqExt03Bit			(1<<kIntVectorExt03)		

// Subsource control (per Samsung documentation there is no Subsource0 register)
#define kRegPendIrqSubsource1		0x40200018
#define kRegMaskIrqSubsource1		0x4020001C
#define kRegPendIrqSubsource2		0x40200030
#define kRegMaskIrqSubsource2		0x40200034

#define kRegPendIrqSubsourceDma	kRegPendIrqSubsource1
#define kRegPendIrqSubsourceSpi	kRegPendIrqSubsource1

#define kRegMaskIrqSubsourceDma	kRegMaskIrqSubsource1
#define kRegMaskIrqSubsourceSpi	kRegPendIrqSubsource1

// Subsource bit masks
#define kIrqSubsourceDma3	0x08000000
#define kIrqSubsourceDma2	0x04000000
#define kIrqSubsourceDma1	0x02000000
#define kIrqSubsourceDma0	0x01000000

#define kIrqSubsourceSpi1	0x00000010
#define kIrqSubsourceSpi0	0x00000008

//---------------------------------------------------------------------------
//  External Interrupt control
//---------------------------------------------------------------------------

#define kRegExtIntControl0	0x448000B0
#define kRegExtIntControl1	0x448000B4
#define kRegExtIntFilter0	0x448000B8
#define kRegExtIntFilter1	0x448000BC
#define kRegExtIntMask		0x448000C0
#define kRegExtIntPending	0x448000C4

#define kExtIntTransceiver	0x0000000C

//---------------------------------------------------------------------------
// DMA	Control registers.  All DMAs generate interrupt 20, and the subsource
// register determines which channel(s) have a pending interrupt.
//---------------------------------------------------------------------------

#define kRegDisrc0		0x40400000
#define kRegDisrcc0	0x40400004
#define kRegDidst0		0x40400008
#define kRegDidstc0	0x4040000C
#define kRegDcon0		0x40400010
#define kRegDstat0		0x40400014
#define kRegDcsrc0		0x40400018
#define kRegDcdst0		0x4040001C
#define kRegDmaskTrig0	0x40400020
#define kRegDmaReqSel0	0x40400024

#define kRegDisrc1		0x40500000
#define kRegDisrcc1	0x40500004
#define kRegDidst1		0x40500008
#define kRegDidstc1	0x4050000C
#define kRegDcon1		0x40500010
#define kRegDstat1		0x40500014
#define kRegDcsrc1		0x40500018
#define kRegDcdst1		0x4050001C
#define kRegDmaskTrig1	0x40500020
#define kRegDmaReqSel1	0x40500024

#define kRegDisrc2		0x40600000
#define kRegDisrcc2	0x40600004
#define kRegDidst2		0x40600008
#define kRegDidstc2	0x4060000C
#define kRegDcon2		0x40600010
#define kRegDstat2		0x40600014
#define kRegDcsrc2		0x40600018
#define kRegDcdst2		0x4060001C
#define kRegDmaskTrig2	0x40600020
#define kRegDmaReqSel2	0x40600024

#define kRegDisrc3		0x40700000
#define kRegDisrcc3	0x40700004
#define kRegDidst3		0x40700008
#define kRegDidstc3	0x4070000C
#define kRegDcon3		0x40700010
#define kRegDstat3		0x40700014
#define kRegDcsrc3		0x40700018
#define kRegDcdst3		0x4070001C
#define kRegDmaskTrig3	0x40700020
#define kRegDmaReqSel3	0x40700024


// Values for use in the DmaReqSeln registers
#define kDmaReqHwSelect	1
#define kDmaReqSelSpi0		(0<<1)
#define kDmaReqSelSpi1		(1<<1)
#define kDmaReqSelIssDo	(2<<1)


//---------------------------------------------------------------------------
// SPI registers
//---------------------------------------------------------------------------

#define kRegSpCon0		0x44500000
#define kRegSpSta0		0x44500004
#define kRegSpPin0		0x44500008
#define kRegSpPre0		0x4450000C
#define kRegSpTDat0	0x44500010
#define kRegSpRDat0	0x44500014

#define kRegSpCon1		0x44500020
#define kRegSpSta1		0x44500024
#define kRegSpPin1		0x44500028
#define kRegSpPre1		0x4450002C
#define kRegSpTDat1	0x44500030
#define kRegSpRDat1	0x44500034

// Bits in the SpSta (SpiStatus) register
#define kSpiTransferReady 0x01


#endif // HARDWARE_H

// EOF

