/***********************  I n c l u d e  -  F i l e  ***********************/
/*!
 *        \file  z146_drv.h
 *
 *      \author  APatil
 *        $Date: 2009/08/03 16:43:29 $
 *    $Revision: 2.4 $
 *
 *       \brief  Header file for Z146 driver containing
 *               Z146 specific status codes and
 *               Z146 function prototypes
 *
 *    \switches  _ONE_NAMESPACE_PER_DRIVER_
 *               _LL_DRV_
 */
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: z17_drv.h,v $
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2004 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#ifndef _Z146_DRV_H
#define _Z146_DRV_H

#ifdef __cplusplus
      extern "C" {
#endif


/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/


/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/** \name Z146 specific Getstat/Setstat standard codes
 *  \anchor getstat_setstat_codes
 */
/**@{*/
#define Z146_DIRECTION           M_DEV_OF+0x01    /**< G,S: Get/Set direction of all ports - 8bit - bit 0 for gpio[0] - value 0 is in - 1 is out  */
#define Z146_SET_SIGNAL          M_DEV_OF+0x02    /**<   S: Set signal sent on IRQ  */
#define Z146_CLR_SIGNAL          M_DEV_OF+0x03    /**<   S: Uninstall signal        */
#define Z146_RX_DATA_LEN         M_DEV_OF+0x04    /**< G  : Get last IRQ request - 8bit - bit 0 for gpio[0] - value 1 is requested - clears the last request  */
#define Z146_RX_RXC_IRQ_STAT     M_DEV_OF+0x05    /**< G  : Get RXC IRQ status. */
#define Z146_RX_RLS_IRQ_STAT     M_DEV_OF+0x06    /**< G  : Get RLS IRQ status. */
#define Z146_RX_SPEED            M_DEV_OF+0x07    /**< G,S: Get/Set RX_LCR RX_SPEED. */
#define Z146_ERR_WE              M_DEV_OF+0x08    /**< G,S: Get/Set RX_LCR RX error write enable. */
#define Z146_PAR_EN              M_DEV_OF+0x09    /**< G,S: Get/Set RX_LCR RX parity enable. */
#define Z146_PAR_TYP             M_DEV_OF+0x0A    /**< G,S: Get/Set RX_LCR RX parity type. */
#define Z146_LAB_EN              M_DEV_OF+0x0B    /**< G,S: Get/Set RX_LCR RX label enable. */
#define Z146_SDI_EN              M_DEV_OF+0x0C    /**< G,S: Get/Set RX_LCR RX source/destination identifier enable. */
#define Z146_SDI                 M_DEV_OF+0x0D    /**< G,S: Get/Set RX_LCR RX source/destination identifier. */
#define Z146_RX_THR_LEV          M_DEV_OF+0x0E    /**< G,S: Get/Set RX_FCR RX threshold level. */
#define Z146_RX_SET_LABEL	     M_DEV_OF+0x0F    /**<   S: Set RX_LA  RX Label set. */
#define Z146_RX_RESET_LABEL      M_DEV_OF+0x10    /**<   S: Set RX_LA  RX Label reset. */
#define Z146_SET_ERROR_SIGNAL    M_DEV_OF+0x11    /**<   S: Set signal sent on error IRQ  */
#define Z146_CLR_ERROR_SIGNAL    M_DEV_OF+0x12    /**<   S: Uninstall error signal        */

/**@}*/


#define _Z146_GLOBNAME(var,name) var##_##name

#ifndef _ONE_NAMESPACE_PER_DRIVER_
  #define Z146_GLOBNAME(var,name)    _Z146_GLOBNAME(var,name)
#else
  #define Z146_GLOBNAME(var,name)    _Z146_GLOBNAME(Z146,name)
#endif

#define __Z146_GetEntry    Z146_GLOBNAME(Z146_VARIANT, GetEntry)

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
#ifdef _LL_DRV_
#ifndef _ONE_NAMESPACE_PER_DRIVER_
	extern void __Z146_GetEntry(LL_ENTRY* drvP);
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

#endif /* _Z146_DRV_H */

