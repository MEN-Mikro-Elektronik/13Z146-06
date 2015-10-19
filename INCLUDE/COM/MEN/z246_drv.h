/***********************  I n c l u d e  -  F i l e  ***********************/
/*!
 *        \file  z246_drv.h
 *
 *      \author  APatil
 *        $Date: 2015/10/17 15:09:13 $
 *    $Revision: 3.1 $
 *
 *       \brief  Header file for Z246 driver containing
 *               Z246 specific status codes and
 *               Z246 function prototypes
 *
 *    \switches  _ONE_NAMESPACE_PER_DRIVER_
 *               _LL_DRV_
 */
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: z246_drv.h,v $
 * Revision 3.1  2015/10/17 15:09:13  ts
 * Initial Revision
 *
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2004 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#ifndef _Z246_DRV_H
#define _Z246_DRV_H

#ifdef __cplusplus
      extern "C" {
#endif


/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/
/* structure for the Z246_BLK_IRQLAT_X setstat/getstat */
typedef struct {
	/* in */
	u_int32 outPort;    /* output port (0..31) */
	u_int32 inPort;     /* input port (0..31) */
	u_int32 irqs2fire;  /* number of interrupts to fire */
	/* out */
	u_int32 tickRate;   /* tick rate from OSS_TickRateGet */
	u_int32 startTick;  /* start tick count */
	u_int32 stopTick;   /* stop tick count */	
	u_int32 irqsRcved;  /* received IRQs */
} Z246_BLK_IRQLAT;

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/** \name Z246 specific Getstat/Setstat standard codes
 *  \anchor getstat_setstat_codes
 */
/**@{*/
#define Z246_SET_SIGNAL          M_DEV_OF+0x01    /**<   S: Set signal sent on IRQ  */
#define Z246_CLR_SIGNAL          M_DEV_OF+0x02    /**<   S: Uninstall signal        */
#define Z246_TX_TXCIEN_STAT      M_DEV_OF+0x03    /**< G  : Get TX IER IRQ status. */
#define Z246_LOOPBACK		     M_DEV_OF+0x04    /**< G,S: Get/Set Loop back mode. */
#define Z246_TX_SPEED            M_DEV_OF+0x05    /**< G,S: Get/Set TX_LCR TX_SPEED. */
#define Z246_PAR_EN              M_DEV_OF+0x06    /**< G,S: Get/Set TX_LCR TX parity enable. */
#define Z246_PAR_TYPE            M_DEV_OF+0x07    /**< G,S: Get/Set TX_LCR TX parity type. */
#define Z246_SDI_EN              M_DEV_OF+0x08    /**< G,S: Get/Set TX_LCR TX source/destination identifier enable. */
#define Z246_SDI                 M_DEV_OF+0x09    /**< G,S: Get/Set TX_LCR TX source/destination identifier. */
#define Z246_TX_THR_LEV          M_DEV_OF+0x0A    /**< G,S: Get/Set TX_FCR TX threshold level. */
#define Z246_TX_LABEL            M_DEV_OF+0x0B    /**< G,S: Get/Set TX_LA TX label. */

/**@}*/


#define _Z246_GLOBNAME(var,name) var##_##name

#ifndef _ONE_NAMESPACE_PER_DRIVER_
  #define Z246_GLOBNAME(var,name)    _Z246_GLOBNAME(var,name)
#else
  #define Z246_GLOBNAME(var,name)    _Z246_GLOBNAME(Z246,name)
#endif

#define __Z246_GetEntry    Z246_GLOBNAME(Z246_VARIANT, GetEntry)

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
#ifdef _LL_DRV_
#ifndef _ONE_NAMESPACE_PER_DRIVER_
	extern void __Z246_GetEntry(LL_ENTRY* drvP);
#endif
#endif /* _LL_DRV_ */

/*-----------------------------------------+
|  BACKWARD COMPATIBILITY TO MDIS4         |
+-----------------------------------------*/
#ifndef U_INT32_OR_64
  /* we have an MDIS4 men_types.h and mdis_api.h included */
  /* only 32bit compatibility needed!                     */
  #define INT32_OR_64    int32
  #define U_INT32_OR_64  u_int32
  typedef INT32_OR_64    MDIS_PATH;
#endif /* U_INT32_OR_64 */

#ifdef __cplusplus
      }
#endif

#endif /* _Z246_DRV_H */

