/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  z246_drv.c
 *
 *      \author  APatil
 *        $Date: 2015/11/08 19:47:19 $
 *    $Revision: 1.2 $
 *
 *      \brief   Low-level driver for ARINC429 transmitter
 *
 *     Required: OSS, DESC, DBG libraries
 *
 *     \switches _ONE_NAMESPACE_PER_DRIVER_
 */
/*-------------------------------[ History ]--------------------------------
 *
 * $Log: z246_drv.c,v $
 * Revision 1.2  2015/11/08 19:47:19  atlstash
 * Stash autocheckin
 *
 * Revision 1.1  2015/10/16 18:08:00  ts
 * Initial Revision
 *
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2004 by MEN Mikro Elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#define _NO_LL_HANDLE        /* ll_defs.h: don't define LL_HANDLE struct */

/*-----------------------------------------+
|  INCLUDES                                |
+-----------------------------------------*/
#include <MEN/men_typs.h>    /* system dependent definitions   */
#include <MEN/maccess.h>     /* hw access macros and types     */
#include <MEN/dbg.h>         /* debug functions                */
#include <MEN/oss.h>         /* oss functions                  */
#include <MEN/desc.h>        /* descriptor functions           */
#include <MEN/modcom.h>      /* ID PROM functions              */
#include <MEN/mdis_api.h>    /* MDIS global defs               */
#include <MEN/mdis_com.h>    /* MDIS common defs               */
#include <MEN/mdis_err.h>    /* MDIS error codes               */
#include <MEN/ll_defs.h>     /* low-level driver definitions   */

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/

#define ADDRSPACE_COUNT    1          /**< nbr of required address spaces */
#define ADDRSPACE_SIZE     0x800      /**< ARINC device is 64k address space          */
#define CH_NUMBER          1          /**< number of device channels      */

/* debug defines */
#define DBG_MYLEVEL        llHdl->dbgLevel    /**< debug level  */
#define DBH                llHdl->dbgHdl      /**< debug handle */
#define OSH                llHdl->osHdl       /**< OS handle    */

/* descriptor key defines*/
#define RESET_DEFAULT      0          /**< default arwen reset (enabled)     */
#define RESET_OFF          1          /**< disables the arwen reset function */

#define Z246_TX_IIR_OFFSET		0x400		/**< Offset of the TX_IIR register */
#define Z246_TX_TXC_OFFSET		0x403		/**< Offset of the TX_TXC register */
#define Z246_TX_TXA_OFFSET	    0x404		/**< Offset of the TX_TXA register */
#define Z246_TX_IER_OFFSET	    0x408		/**< Offset of the TX_IER register */
#define Z246_TX_IER_DEFAULT	    0x0         /**< Offset of the TX_LA register */
#define Z246_TX_IRQ_MASK    	0x1			/**< Transmit IRQ mask. */

#define Z246_TX_LCR_OFFSET		0x409		/**< Offset of the TX_LCR register */
#define Z246_TX_SPEED_OFFSET	0			/**< Offset of the TX_LCR SPEED bit */
#define Z246_TX_SPEED_MASK  	0x01		/**< MASK of the TX_LCR SPEED bit */
#define Z246_TX_LOOP_OFFSET		1			/**< Offset of the TX_LCR LOOP bit */
#define Z246_TX_LOOP_MASK  		0x02		/**< MASK of the TX_LCR LOOP bit */
#define Z246_TX_PAR_EN_OFFSET	2			/**< Offset of the TX_LCR PAR_EN bit */
#define Z246_TX_PAR_EN_MASK  	0x04		/**< MASK of the TX_LCR PAR_EN bit */
#define Z246_TX_PAR_TYP_OFFSET	3			/**< Offset of the TX_LCR PAR_TYP bit */
#define Z246_TX_PAR_TYP_MASK  	0x08		/**< MASK of the TX_LCR PAR_TYP bit */
#define Z246_TX_SDI_EN_OFFSET	5			/**< Offset of the TX_LCR SDI_EN bit */
#define Z246_TX_SDI_EN_MASK  	0x20		/**< MASK of the TX_LCR SDI_EN bit */
#define Z246_TX_SDI_OFFSET	    6			/**< Offset of the TX_LCR SDI_EN bit */
#define Z246_TX_SDI_MASK    	0xC0		/**< MASK of the TX_LCR SDI bit */
#define Z146_TX_SDI_MAX    	    0x03		/**< TX_LCR SDI max value */

#define Z246_LCR_PAR_MASK		0x04
#define Z246_LCR_SDI_MASK		0x20

#define Z246_TX_LCR_DEFAULT		0x05		/**< Offset of the TX_LCR register */

#define Z246_TX_FCR_OFFSET		0x40A		/**< Offset of the TX_FCR register */
#define Z246_TX_FCR_DEFAULT 	0x6			/**< Default of the TX_FCR register */
#define Z246_TX_FCR_MASK    	0x7			/**< Mask of the TX_FCR register */

#define Z246_TX_LA_OFFSET		0x40B		/**< Default of the TX_IER register */
#define Z246_TX_LA_DEFAULT		0x0			/**< Default of the TX_LA register */

#define Z246_FIFO_START_ADDR    0x000		/**< Start address of the hardware FIFO of the transmitter. */
#define Z246_TX_FIFO_MAX 		255			/**< Size of the hardware FIFO of the transmitter. */

#define Z246_24_BIT_MASK			0xFFFFFF
#define Z246_23_BIT_MASK			0x7FFFFF
#define Z246_22_BIT_MASK			0x3FFFFF
#define Z246_21_BIT_MASK			0x1FFFFF

#define Z246_MAX_BUFF_SIZE			4097	/**< 4096 + 1 to avoid buffer overflow. */
#define Z246_RING_SIZE_DEFAULT		0
/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/
/** low-level handle */
typedef struct {
	/* general */
	int32                   memAlloc;       /**< size allocated for the handle */
	OSS_HANDLE              *osHdl;         /**< oss handle        */
	OSS_IRQ_HANDLE          *irqHdl;        /**< irq handle        */
	DESC_HANDLE             *descHdl;       /**< desc handle       */
	MACCESS                 ma;             /**< hw access handle  */
	MDIS_IDENT_FUNCT_TBL    idFuncTbl;      /**< id function table */
	/* debug */
	u_int32                 dbgLevel;       /**< debug level  */
	DBG_HANDLE              *dbgHdl;        /**< debug handle */

	OSS_SIG_HANDLE          *portChangeSig; /**< signal for port change */

	/* toggle mode */
	OSS_ALARM_HANDLE        *alarmHdl;      /**< alarm handle               */
	OSS_SEM_HANDLE          *devSemHdl;     /**< device semaphore handle    */

	/* Ring buffer parameters */
	u_int32					ringBuffer[Z246_MAX_BUFF_SIZE];
	volatile u_int32	    ringHead;
	volatile u_int32 		ringTail;
	volatile u_int32 		ringDataCnt;

} LL_HANDLE;

/* include files which need LL_HANDLE */
#include <MEN/ll_entry.h>       /* low-level driver jump table */
#include <MEN/z246_drv.h>        /* Z246 driver header file      */

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
static int32 Z246_Init(DESC_SPEC *descSpec, OSS_HANDLE *osHdl,
		MACCESS *ma, OSS_SEM_HANDLE *devSemHdl,
		OSS_IRQ_HANDLE *irqHdl, LL_HANDLE **llHdlP);
static int32 Z246_Exit(LL_HANDLE **llHdlP);
static int32 Z246_Read(LL_HANDLE *llHdl, int32 ch, int32 *value);
static int32 Z246_Write(LL_HANDLE *llHdl, int32 ch, int32 value);
static int32 Z246_SetStat(LL_HANDLE *llHdl,int32 ch, int32 code,
		INT32_OR_64 value32_or_64);
static int32 Z246_GetStat(LL_HANDLE *llHdl, int32 ch, int32 code,
		INT32_OR_64 *value32_or64P);
static int32 Z246_BlockRead(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
		int32 *nbrRdBytesP);
static int32 Z246_BlockWrite(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
		int32 *nbrWrBytesP);

static int32 Z246_Irq(LL_HANDLE *llHdl);

static int32 Z246_Info(int32 infoType, ...);
static char* Ident(void);
static int32 Cleanup(LL_HANDLE *llHdl, int32 retCode);
static void  ConfigureDefault( LL_HANDLE *llHdl );
static int HwWrite(LL_HANDLE    *llHdl);
static void RegStatus(LL_HANDLE *llHdl );
static u_int32 ReadFromBuffer( LL_HANDLE *llHdl, int8 * result);
static int8 StoreInBuffer( LL_HANDLE *llHdl , u_int32 data);


/****************************** Z246_GetEntry ********************************/
/** Initialize driver's jump table
 *
 *  \param drvP     \OUT pointer to the initialized jump table structure
 */
#ifdef _ONE_NAMESPACE_PER_DRIVER_
extern void LL_GetEntry(
		LL_ENTRY* drvP
)
#else
extern void __Z246_GetEntry(
		LL_ENTRY* drvP
)
#endif
{
	drvP->init        = Z246_Init;
	drvP->exit        = Z246_Exit;
	drvP->read        = Z246_Read;
	drvP->write       = Z246_Write;
	drvP->blockRead   = Z246_BlockRead;
	drvP->blockWrite  = Z246_BlockWrite;
	drvP->setStat     = Z246_SetStat;
	drvP->getStat     = Z246_GetStat;
	drvP->irq         = Z246_Irq;
	drvP->info        = Z246_Info;
}

/******************************** Z246_Init **********************************/
/** Allocate and return low-level handle, initialize hardware
 *
 * The function initializes all channels with the definitions made
 * in the descriptor. The interrupt is disabled.
 *
 * The following descriptor keys are used:
 *
 * \code
 * Descriptor key        Default          Range
 * --------------------  ---------------  -------------
 * DEBUG_LEVEL_DESC      OSS_DBG_DEFAULT  see dbg.h
 * DEBUG_LEVEL           OSS_DBG_DEFAULT  see dbg.h
 * ID_CHECK              1                0..1
 * \endcode
 *
 *  \param descP      \IN  pointer to descriptor data
 *  \param osHdl      \IN  oss handle
 *  \param ma         \IN  hw access handle
 *  \param devSemHdl  \IN  device semaphore handle
 *  \param irqHdl     \IN  irq handle
 *  \param llHdlP     \OUT pointer to low-level driver handle
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z246_Init(
		DESC_SPEC       *descP,
		OSS_HANDLE      *osHdl,
		MACCESS         *ma,
		OSS_SEM_HANDLE  *devSemHdl,
		OSS_IRQ_HANDLE  *irqHdl,
		LL_HANDLE       **llHdlP
)
{
	LL_HANDLE *llHdl = NULL;
	u_int32 gotsize;
	int32 error;
	u_int32 value;

	/*------------------------------+
	|  prepare the handle           |
	+------------------------------*/
	*llHdlP = NULL;		/* set low-level driver handle to NULL */

	/* alloc */
	if ((llHdl = (LL_HANDLE*)OSS_MemGet(
			osHdl, sizeof(LL_HANDLE), &gotsize)) == NULL)
		return (ERR_OSS_MEM_ALLOC);

	/* clear */
	OSS_MemFill(osHdl, gotsize, (char*)llHdl, 0x00);

	/* init */
	llHdl->memAlloc    = gotsize;
	llHdl->osHdl       = osHdl;
	llHdl->irqHdl      = irqHdl;
	llHdl->ma          = *ma;
	llHdl->devSemHdl   = devSemHdl;
	llHdl->ringTail    = Z246_RING_SIZE_DEFAULT;
	llHdl->ringHead    = Z246_RING_SIZE_DEFAULT;
	llHdl->ringDataCnt = 0;
	/*------------------------------+
	|  init id function table       |
	+------------------------------*/
	/* driver's ident function */
	llHdl->idFuncTbl.idCall[0].identCall = Ident;
	/* library's ident functions */
	llHdl->idFuncTbl.idCall[1].identCall = DESC_Ident;
	llHdl->idFuncTbl.idCall[2].identCall = OSS_Ident;
	/* terminator */
	llHdl->idFuncTbl.idCall[3].identCall = NULL;

	/*------------------------------+
	|  prepare debugging            |
	+------------------------------*/
	DBG_MYLEVEL = OSS_DBG_DEFAULT;		/* set OS specific debug level */
	DBGINIT((NULL,&DBH));

	/*------------------------------+
	|  scan descriptor              |
	+------------------------------*/
	/* prepare access */
	if ((error = DESC_Init(descP, osHdl, &llHdl->descHdl)))
		return (Cleanup(llHdl, error));

	/* DEBUG_LEVEL_DESC */
	if ((error = DESC_GetUInt32(llHdl->descHdl, OSS_DBG_DEFAULT,
			&value, "DEBUG_LEVEL_DESC")) &&
			error != ERR_DESC_KEY_NOTFOUND)
		return (Cleanup(llHdl, error));

	DESC_DbgLevelSet(llHdl->descHdl, value);	/* set level */

	/* DEBUG_LEVEL */
	if ((error = DESC_GetUInt32(llHdl->descHdl, OSS_DBG_DEFAULT,
			&llHdl->dbgLevel, "DEBUG_LEVEL")) &&
			error != ERR_DESC_KEY_NOTFOUND)
		return (Cleanup(llHdl, error));

	DBGWRT_1((DBH, "Z246_Init: base address = %08p\n", (void*)llHdl->ma));

	/*------------------------------+
	|  init hardware                |
	+------------------------------*/
	ConfigureDefault(llHdl);
	*llHdlP = llHdl;		/* set low-level driver handle */

	return (ERR_SUCCESS);
}

/****************************** Z246_Exit ************************************/
/** De-initialize hardware and clean up memory
 *
 *  The function deinitializes all channels by setting them as inputs.
 *  The interrupt is disabERR_SUCCESSled.
 *
 *  \param llHdlP     \IN  pointer to low-level driver handle
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z246_Exit(
		LL_HANDLE **llHdlP
)
{
	LL_HANDLE *llHdl = *llHdlP;
	int32 error = 0;

	DBGWRT_1((DBH, "Z246_Exit\n"));

	/*------------------------------+
	|  clean up memory              |
	+------------------------------*/
	*llHdlP = NULL;		/* set low-level driver handle to NULL */
	error = Cleanup(llHdl, error);

	return (error);
}

/****************************** Z246_Read ************************************/
/** Read a value from the device.
 *
 *  The function is not supported and always returns an ERR_LL_ILL_FUNC error.
 *
 *  \param llHdl      \IN  low-level handle
 *  \param ch         \IN  current channel
 *  \param valueP     \OUT read value
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z246_Read(
		LL_HANDLE *llHdl,
		int32 ch,
		int32 *valueP
)
{
	DBGWRT_1((DBH, "LL - Z246_Read: ch=%d, valueP=%d\n",ch,*valueP));

	return( ERR_LL_ILL_FUNC );
}

/****************************** Z246_Write ***********************************/
/** Description:  Write a value to the device
 *
 *  The function is not supported and always returns an ERR_LL_ILL_FUNC error.
 *
 *  \param llHdl      \IN  low-level handle
 *  \param ch         \IN  current channel
 *  \param value      \IN  value to write
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z246_Write(
		LL_HANDLE *llHdl,
		int32 ch,
		int32 value
)
{
	DBGWRT_1((DBH, "LL - Z246_Write: ch=%d, valueP=%d\n",ch,value));

	return( ERR_LL_ILL_FUNC );
}

/****************************** Z246_SetStat *********************************/
/** Set the driver status
 *
 *  The driver supports \ref tx_getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *  Note: only inputs are able fire an interrupt
 *
 *  \param llHdl         \IN  low-level handle
 *  \param code          \IN  \ref tx_getstat_setstat_codes "status code"
 *  \param ch            \IN  current channel
 *  \param value32_or_64 \IN  data or pointer to block data structure
 *                            (M_SG_BLOCK) for block status codes
 *  \return              \c 0 on success or error code
 */
static int32 Z246_SetStat(
		LL_HANDLE   *llHdl,
		int32       code,
		int32       ch,
		INT32_OR_64 value32_or_64
)
{
	int32 value = (int32)value32_or_64;		/* 32bit value */

	int32 error = ERR_SUCCESS;
	u_int8 regData = 0;

	DBGWRT_1((DBH, "LL - Z246_SetStat: ch=%d code=0x%04x value=0x%x\n",
			ch, code, value));

	switch (code) {
	/*--------------------------+
		|  debug level              |
		+--------------------------*/
	case M_LL_DEBUG_LEVEL:
		llHdl->dbgLevel = value;
		break;
		/*--------------------------+
		|  enable interrupts        |
		+--------------------------*/
	case M_MK_IRQ_ENABLE:
		break;

		/*--------------------------+
		|  channel direction        |
		+--------------------------*/
	case M_LL_CH_DIR:
		if (value != M_CH_INOUT) {
			error = ERR_LL_ILL_DIR;
		}
		break;

		/*--------------------------+
		|  register signal          |
		+--------------------------*/
	case Z246_SET_SIGNAL:
		/* signal already installed ? */
		if (llHdl->portChangeSig) {
			error = ERR_OSS_SIG_SET;
			break;
		}
		error = OSS_SigCreate(OSH, value, &llHdl->portChangeSig);
		break;
		/*--------------------------+
		|  unregister signal        |
		+--------------------------*/
	case Z246_CLR_SIGNAL:
		/* signal already installed ? */
		if (llHdl->portChangeSig == NULL) {
			error = ERR_OSS_SIG_CLR;
			break;
		}
		error = OSS_SigRemove(OSH, &llHdl->portChangeSig);
		break;

		/*--------------------------+
		|  Interrupt enable            |
		+--------------------------*/
	case Z246_TX_TXCIEN_STAT:
		/* Enable interrupt */
		MWRITE_D8(llHdl->ma, Z246_TX_IER_OFFSET, (u_int8)value32_or_64);
		break;

		/*--------------------------+
		|  TX SPEED		            |
		+--------------------------*/
	case Z246_TX_SPEED:
		/* Set loopback mode */
		regData = MREAD_D8(llHdl->ma, Z246_TX_LCR_OFFSET);
		if(value32_or_64 != 0){
			regData = regData | Z246_TX_SPEED_MASK;
		}else{
			regData = regData & (~Z246_TX_SPEED_MASK);
		}
		MWRITE_D8(llHdl->ma, Z246_TX_LCR_OFFSET, regData);
		DBGWRT_1((DBH, "LL - Z246_SetStat: Z246_TX_SPEED: value = %d\n", value32_or_64));
		break;
		/*--------------------------+
		|  Loopback mode            |
		+--------------------------*/
	case Z246_LOOPBACK:
		/* Set loopback mode */
		regData = MREAD_D8(llHdl->ma, Z246_TX_LCR_OFFSET);
		if(value32_or_64 != 0){
			regData = regData | Z246_TX_LOOP_MASK;
		}else{
			regData = regData & (~Z246_TX_LOOP_MASK);
		}
		MWRITE_D8(llHdl->ma, Z246_TX_LCR_OFFSET, regData);
		break;

		/*--------------------------+
		|  Parity enable            |
		+--------------------------*/
	case Z246_PAR_EN:
		/* Set loopback mode */
		regData = MREAD_D8(llHdl->ma, Z246_TX_LCR_OFFSET);
		if(value32_or_64 != 0){
			regData = regData | Z246_TX_PAR_EN_MASK;
		}else{
			regData = regData & (~Z246_TX_PAR_EN_MASK);
		}
		MWRITE_D8(llHdl->ma, Z246_TX_LCR_OFFSET, regData);
		break;

		/*--------------------------+
		|  Parity Type              |
		+--------------------------*/
	case Z246_PAR_TYPE:
		/* Set loopback mode */
		regData = MREAD_D8(llHdl->ma, Z246_TX_LCR_OFFSET);
		if(value32_or_64 != 0){
			regData = regData | Z246_TX_PAR_TYP_MASK;
		}else{
			regData = regData & (~Z246_TX_PAR_TYP_MASK);
		}
		MWRITE_D8(llHdl->ma, Z246_TX_LCR_OFFSET, regData);
		break;

		/*--------------------------+
		|  SDI Enable               |
		+--------------------------*/
	case Z246_SDI_EN:
		/* Set loopback mode */
		regData = MREAD_D8(llHdl->ma, Z246_TX_LCR_OFFSET);
		if(value32_or_64 != 0){
			regData = regData | Z246_TX_SDI_EN_MASK;
		}else{
			regData = regData & (~Z246_TX_SDI_EN_MASK);
		}
		MWRITE_D8(llHdl->ma, Z246_TX_LCR_OFFSET, regData);
		break;

		/*--------------------------+
		|  SDI                      |
		+--------------------------*/
	case Z246_SDI:
		/* Set SDI */
		if(value32_or_64 <= Z146_TX_SDI_MAX){
			regData = MREAD_D8(llHdl->ma, Z246_TX_LCR_OFFSET);
			regData |= (value32_or_64 << Z246_TX_SDI_OFFSET) & Z246_TX_SDI_MASK;
			DBGWRT_1((DBH, "LL - Z246_SetStat: Z246_SDI: LCR = 0x%x.\n", regData));
			MWRITE_D8(llHdl->ma, Z246_TX_LCR_OFFSET, regData);
		}else{
			error = ERR_LL_ILL_PARAM;
		}
		break;


		/*--------------------------------------+
		|  Transmit threshold level status    |
		+---------------------------------------*/
	case Z246_TX_THR_LEV:
		regData = (value32_or_64 & Z246_TX_FCR_MASK);
		MWRITE_D8(llHdl->ma, Z246_TX_FCR_OFFSET, regData);
		break;

		/*-------------------+
		|  Transmit Label    |
		+--------------------*/
	case Z246_TX_LABEL:
		regData = (value32_or_64 & 0xFF);
		MWRITE_D8(llHdl->ma, Z246_TX_LA_OFFSET, regData);
		break;

		/*--------------------------+
		|  (unknown)                |
		+--------------------------*/
	default:
		error = ERR_LL_UNK_CODE;
	}

	return (error);
}

/****************************** Z246_GetStat *********************************/
/** Get the driver status
 *
 *  The driver supports \ref tx_getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *  \param llHdl             \IN  low-level handle
 *  \param code              \IN  \ref tx_getstat_setstat_codes "status code"
 *  \param ch                \IN  current channel
 *  \param value32_or_64P    \IN  pointer to block data structure (M_SG_BLOCK) for
 *                                block status codes
 *  \param value32_or_64P    \OUT data pointer or pointer to block data structure
 *                                (M_SG_BLOCK) for block status codes
 *
 *  \return                  \c 0 on success or error code
 */
static int32 Z246_GetStat(
		LL_HANDLE   *llHdl,
		int32       code,
		int32       ch,
		INT32_OR_64 *value32_or_64P
)
{
	int32 *valueP = (int32*)value32_or_64P;		/* pointer to 32bit value */
	INT32_OR_64 *value64P = value32_or_64P;		/* stores 32/64bit pointer */
	int32 error = ERR_SUCCESS;
	int32 regData = 0;


	DBGWRT_1((DBH, "LL - Z246_GetStat: ch=%d code=0x%04x\n", ch, code));

	switch (code) {
	/*--------------------------+
		|  debug level              |
		+--------------------------*/
	case M_LL_DEBUG_LEVEL:
		*valueP = llHdl->dbgLevel;
		break;

		/*--------------------------+
		|  number of channels       |
		+--------------------------*/
	case M_LL_CH_NUMBER:
		*valueP = CH_NUMBER;
		break;

		/*--------------------------+
		|  channel direction        |
		+--------------------------*/
	case M_LL_CH_DIR:
		*valueP = M_CH_INOUT;
		break;

		/*--------------------------+
		|  channel type info        |
		+--------------------------*/
	case M_LL_CH_TYP:
		*valueP = M_CH_BINARY;
		break;

		/*--------------------------+
		|  ID PROM check enabled    |
		+--------------------------*/
	case M_LL_ID_CHECK:
		*valueP = 0;
		break;
		/*--------------------------+
		|  ident table pointer      |
		|  (treat as non-block!)    |
		+--------------------------*/
	case M_MK_BLK_REV_ID:
		*value64P = (INT32_OR_64)&llHdl->idFuncTbl;
		break;

		/*--------------------------+
		|  Interrupt enable            |
		+--------------------------*/
	case Z246_TX_TXCIEN_STAT:
		*value64P = (INT32_OR_64)(MREAD_D8(llHdl->ma, Z246_TX_IER_OFFSET));
		break;

		/*--------------------------+
		|  TX SPEED		            |
		+--------------------------*/
	case Z246_TX_SPEED:
		*value64P = (INT32_OR_64)(MREAD_D8(llHdl->ma, Z246_TX_LCR_OFFSET) & Z246_TX_SPEED_MASK);
		break;
		/*--------------------------+
		|  Loopback mode            |
		+--------------------------*/
	case Z246_LOOPBACK:
		regData = MREAD_D8(llHdl->ma, Z246_TX_LCR_OFFSET);
		*value64P = (INT32_OR_64)((regData & Z246_TX_LOOP_MASK) >> Z246_TX_LOOP_OFFSET);

		break;

		/*--------------------------+
		|  Parity enable            |
		+--------------------------*/
	case Z246_PAR_EN:
		regData = MREAD_D8(llHdl->ma, Z246_TX_LCR_OFFSET);
		*value64P = (INT32_OR_64)((regData & Z246_TX_PAR_EN_MASK) >> Z246_TX_PAR_EN_OFFSET);
		break;

		/*--------------------------+
		|  Parity Type              |
		+--------------------------*/
	case Z246_PAR_TYPE:
		regData = MREAD_D8(llHdl->ma, Z246_TX_LCR_OFFSET);
		*value64P = (INT32_OR_64)((regData & Z246_TX_PAR_TYP_MASK) >> Z246_TX_PAR_TYP_OFFSET);
		break;

		/*--------------------------+
		|  SDI Enable               |
		+--------------------------*/
	case Z246_SDI_EN:
		regData = MREAD_D8(llHdl->ma, Z246_TX_LCR_OFFSET);
		DBGWRT_1((DBH, "LL - Z246_GetStat: Z246_SDI_EN LCR=0x%04x\n", regData));
		*value64P = (INT32_OR_64)((regData & Z246_TX_SDI_EN_MASK) >> Z246_TX_SDI_EN_OFFSET);
		break;

		/*--------------------------+
		|  SDI                      |
		+--------------------------*/
	case Z246_SDI:
		regData = MREAD_D8(llHdl->ma, Z246_TX_LCR_OFFSET);
		*value64P = (INT32_OR_64)((regData & Z246_TX_SDI_MASK) >> Z246_TX_SDI_OFFSET);
		break;

		/*--------------------------------------+
		|  Transmit threshold level status    |
		+---------------------------------------*/
	case Z246_TX_THR_LEV:
		*value64P = (MREAD_D8(llHdl->ma, Z246_TX_FCR_OFFSET) & Z246_TX_FCR_MASK);
		break;

		/*-------------------+
		|  Transmit Label    |
		+--------------------*/
	case Z246_TX_LABEL:
		*value64P = (MREAD_D8(llHdl->ma, Z246_TX_LA_OFFSET) & 0xFF);
		break;

		/*--------------------------+
		|  (unknown)                |
		+--------------------------*/
	default:
		error = ERR_LL_UNK_CODE;
		break;
	}

	return (error);
}

/******************************* Z246_BlockRead ******************************/
/** Read a data block from the device
 *
 *  The function is not supported and always returns an ERR_LL_ILL_FUNC error.
 * 
 *  \param llHdl       \IN  low-level handle
 *  \param ch          \IN  current channel
 *  \param buf         \IN  data buffer
 *  \param size        \IN  data buffer size
 *  \param nbrRdBytesP \OUT number of read bytes
 *
 *  \return            \c 0 on success or error code
 */
static int32 Z246_BlockRead(
		LL_HANDLE *llHdl,
		int32     ch,
		void      *buf,
		int32     size,
		int32     *nbrRdBytesP
)
{
	DBGWRT_1((DBH, "LL - Z246_BlockRead: ch=%d, size=%d\n",ch,size));

	/* return number of read bytes */
	*nbrRdBytesP = 0;

	return( ERR_LL_ILL_FUNC );
}

/****************************** Z246_BlockWrite *****************************/
/** Write a data block from the device
 *
 *  \param llHdl  	   \IN  low-level handle
 *  \param ch          \IN  current channel
 *  \param buf         \IN  data buffer
 *  \param size        \IN  data buffer size
 *  \param nbrWrBytesP \OUT number of written bytes
 *
 *  \return            \c 0 on success or error code
 */
static int32 Z246_BlockWrite(
		LL_HANDLE *llHdl,
		int32     ch,
		void      *buf,
		int32     size,
		int32     *nbrWrBytesP
)
{
	int32 result = ERR_SUCCESS;
	int8  ringResult = 0;
	u_int32 i = 0;
	u_int32 gotsize = 0;
	u_int32 llDataLen = size/4;
	u_int32 * userBuf = (u_int32*)buf;

	DBGWRT_1((DBH, " >>> LL - Z246_BlockWrite: size=%d\n",size));

	/* Check for user buffer size */
	if((size != 0) && (buf != NULL)){
		if(llDataLen <= (Z246_MAX_BUFF_SIZE - 1)){
			/* Copy data from user space to kernel space (ring buffer). */
			for(i=0;i<llDataLen;i++){
				ringResult = StoreInBuffer(llHdl, userBuf[i]);
				if(ringResult != 0){
					result = ERR_MBUF_OVERFLOW;
					IDBGWRT_1((DBH, ">>> LL - Z246_BlockWrite: ring buffer problem \n"));
				}
			}
			if(result == ERR_SUCCESS){
				if(HwWrite(llHdl) == 0){
					/* Write operation successful */
					result = ERR_SUCCESS;
				} /* Else return the result */
			}
		}else{
			result = ERR_MBUF_OVERFLOW;
		}
	}else{
		result = ERR_MBUF_ILL_SIZE;
	}

	/* Return number of written bytes as per the result status. */
	if(result == ERR_SUCCESS){
		*nbrWrBytesP = size;
	}else{
		*nbrWrBytesP = 0;
	}
	RegStatus(llHdl);
	return result;
}


/****************************** Z246_Irq ************************************/
/** Interrupt service routine
 *
 *  The interrupt is triggered when transmit FIFO has space to accept more data.
 *  Then the remaining data is transmitted. If data size is more than the FIFO size
 *  then the interrupt will be set again. If the data fits in the fifo then the
 *  interrupt will be disabled.
 *  \param llHdl       \IN  low-level handle
 *  \return LL_IRQ_DEVICE   irq caused by device
 *          LL_IRQ_DEV_NOT  irq not caused by device
 *          LL_IRQ_UNKNOWN  unknown
 */
static int32 Z246_Irq(
		LL_HANDLE *llHdl
)
{
	u_int32 irqReq;
	u_int32 ier1, ier2;

	/* interrupt caused by TX ? */
	irqReq = MREAD_D8(llHdl->ma, Z246_TX_IIR_OFFSET);

	if (irqReq & Z246_TX_IRQ_MASK) {

		IDBGWRT_1((DBH, ">>> LL - Z246_Irq: request %08x\n", irqReq));

		/* Else disable the queue space interrupt. */
		MWRITE_D8(llHdl->ma, Z246_TX_IER_OFFSET, 0);
		/* interrupt is cleared by disabling it.  */

		/* Call the tx routine to send remaining data. */
		HwWrite(llHdl);

		/* if requested send signal to application */
		if (llHdl->portChangeSig){
			OSS_SigSend(OSH, llHdl->portChangeSig);
		}

		return (LL_IRQ_DEVICE);
	}

	return (LL_IRQ_DEV_NOT);
}

/****************************** Z246_Info ***********************************/
/** Get information about hardware and driver requirements
 *
 *  The following info codes are supported:
 *
 * \code
 *  Code                      Description
 *  ------------------------  -----------------------------
 *  LL_INFO_HW_CHARACTER      hardware characteristics
 *  LL_INFO_ADDRSPACE_COUNT   nr of required address spaces
 *  LL_INFO_ADDRSPACE         address space information
 *  LL_INFO_IRQ               interrupt required
 *  LL_INFO_LOCKMODE          process lock mode required
 * \endcode
 *
 *  The LL_INFO_HW_CHARACTER code returns all address and
 *  data modes (ORed) which are supported by the hardware
 *  (MDIS_MAxx, MDIS_MDxx).
 *
 *  The LL_INFO_ADDRSPACE_COUNT code returns the number
 *  of address spaces used by the driver.
 *
 *  The LL_INFO_ADDRSPACE sizecode returns information about one
 *  specific address space (MDIS_MAxx, MDIS_MDxx). The returned
 *  data mode represents the widest hardware access used by
 *  the driver.
 *
 *  The LL_INFO_IRQ code returns whether the driver supports an
 *  interrupt routine (TRUE or FALSE).
 *
 *  The LL_INFO_LOCKMODE code returns which process locking
 *  mode the driver needs (LL_LOCK_xxx).
 *
 *  \param infoType    \IN  info code
 *  \param ...         \IN  argument(s)
 *
 *  \return            \c 0 on success or error code
 */
static int32 Z246_Info(
		int32 infoType,
		...
)
{
	int32   error = ERR_SUCCESS;
	va_list argptr;

	va_start(argptr, infoType);

	switch (infoType) {
	/*-------------------------------+
		|  hardware characteristics      |
		|  (all addr/data modes ORed)    |
		+-------------------------------*/
	case LL_INFO_HW_CHARACTER:
	{
		u_int32 *addrModeP = va_arg(argptr, u_int32*);
		u_int32 *dataModeP = va_arg(argptr, u_int32*);

		*addrModeP = MDIS_MA08;
		*dataModeP = MDIS_MD08 | MDIS_MD16;
		break;
	}

	/*-------------------------------+
		|  nr of required address spaces |
		|  (total spaces used)           |
		+-------------------------------*/
	case LL_INFO_ADDRSPACE_COUNT:
	{
		u_int32 *nbrOfAddrSpaceP = va_arg(argptr, u_int32*);

		*nbrOfAddrSpaceP = ADDRSPACE_COUNT;
		break;
	}
	/*-------------------------------+
		|  address space type            |
		|  (widest used data mode)       |
		+-------------------------------*/
	case LL_INFO_ADDRSPACE:
	{
		u_int32 addrSpaceIndex = va_arg(argptr, u_int32);
		u_int32 *addrModeP = va_arg(argptr, u_int32*);
		u_int32 *dataModeP = va_arg(argptr, u_int32*);
		u_int32 *addrSizeP = va_arg(argptr, u_int32*);

		if (addrSpaceIndex >= ADDRSPACE_COUNT) {
			error = ERR_LL_ILL_PARAM;
		} else {
			*addrModeP = MDIS_MA08;
			*dataModeP = MDIS_MD16;
			*addrSizeP = ADDRSPACE_SIZE;
		}
		break;
	}
	/*-------------------------------+
		|  interrupt required            |
		+-------------------------------*/
	case LL_INFO_IRQ:
	{
		u_int32 *useIrqP = va_arg(argptr, u_int32*);

		*useIrqP = 1;
		break;
	}
	/*-------------------------------+
		|  process lock mode             |
		+-------------------------------*/
	case LL_INFO_LOCKMODE:
	{
		u_int32 *lockModeP = va_arg(argptr, u_int32*);

		*lockModeP = LL_LOCK_NONE;
		break;
	}
	/*-------------------------------+
		|  (unknown)                     |
		+-------------------------------*/
	default:
		error = ERR_LL_ILL_PARAM;
	}

	va_end(argptr);

	return (error);
}

/******************************** Ident ************************************/
/** Return ident string
 *
 *  \return            pointer to ident string
 */
static char* Ident(void)
{
	return ("Z246 - Z246 low level driver: $Id: z246_drv.c,v 1.2 2015/11/08 19:47:19 atlstash Exp $"
	);
}

/********************************* Cleanup *********************************/
/** Close all handles, free memory and return error code
 *
 *  \warning The low-level handle is invalid after this function is called.
 *
 *  \param llHdl      \IN  low-level handle
 *  \param retCode    \IN  return value
 *
 *  \return           \IN  retCode
 */
static int32 Cleanup(
		LL_HANDLE *llHdl,
		int32     retCode
)
{
	/*------------------------------+
	|  close handles                |
	+------------------------------*/
	/* clean up desc */
	if (llHdl->descHdl)
		DESC_Exit(&llHdl->descHdl);

	/* clean up debug */
	DBGEXIT((&DBH));


	/* Disable the interrupt */
	MWRITE_D8(llHdl->ma, Z246_TX_IER_OFFSET, 0);

	/*------------------------------+
	|  free memory                  |
	+------------------------------*/
	/* free my handle */
	OSS_MemFree(llHdl->osHdl, (int8*)llHdl, llHdl->memAlloc);

	/*return error code */
	return (retCode);
}

/**********************************************************************/
/** Configure default values to registers.
 *
 *  Sets the controller registers to default values:
 *
 *  \param llHdl      \IN  low-level handle
 */

static void
ConfigureDefault( LL_HANDLE *llHdl )
{
	int i =0;

	/* Configure TX LCR */
	MWRITE_D8(llHdl->ma, Z246_TX_LCR_OFFSET, Z246_TX_LCR_DEFAULT);

	/* Configure TX LA */
	MWRITE_D8(llHdl->ma, Z246_TX_LA_OFFSET, Z246_TX_LA_DEFAULT);

	/* Configure TX FCR */
	MWRITE_D8(llHdl->ma, Z246_TX_FCR_OFFSET, Z246_TX_FCR_DEFAULT);

	/* Disable the interrupt */
	MWRITE_D8(llHdl->ma, Z246_TX_IER_OFFSET, Z246_TX_IER_DEFAULT);

}

/**********************************************************************/
/** Write data from the ring buffer to the FPGA FIFO.
 *
 *  Writes data according to the configuration and space available.
 *
 *  \param llHdl      \IN  low-level handle
 */
int HwWrite(LL_HANDLE    *llHdl){
	int32 result = ERR_SUCCESS;
	int8 ringResult = 0;
	u_int32 dataBitMask = 0;
	u_int16 lcrRegData = 0;
	u_int32 dataCount = 0;
	u_int32 data = 0;
	u_int8 isSdiEn = 0;
	u_int8 isParityEn = 0;
	u_int16 i = 0;
	u_int8 txcStatus = 0;

	DBGWRT_2((DBH, "LL - Z246_Write: \n"));

	lcrRegData = MREAD_D8(llHdl->ma, Z246_TX_LCR_OFFSET);
	isParityEn = (lcrRegData & Z246_LCR_PAR_MASK);
	isSdiEn = (lcrRegData & Z246_LCR_SDI_MASK);

	/* Check TXC register for remaining space in the TX queue. */
	txcStatus = MREAD_D8(llHdl->ma, Z246_TX_TXC_OFFSET);

	DBGWRT_2((DBH, "LL - Z246_Write: txcStatus = %d\n", txcStatus));

	if(llHdl->ringDataCnt > (u_int32)(Z246_TX_FIFO_MAX - txcStatus)){
		/* If data length is greater than the queue space then transmit that much data only. */
		dataCount = (Z246_TX_FIFO_MAX - txcStatus);
	}else{
		/* If data length is smaller than the queue space then transmit all the data. */
		dataCount = llHdl->ringDataCnt;
	}
	DBGWRT_2((DBH, "LL - Z246_Write: writing %d bytes\n", dataCount));

	/* If enough space then write the data to the queue */
	if(dataCount != 0){
		/* Check if the parity and SDI is disabled.	If disabled then 24 bit space. */
		if((isParityEn == 0) && (isSdiEn == 0)){
			dataBitMask = Z246_24_BIT_MASK;
		}
		/* Else if parity is enabled and SDI is disabled then 23 bit space */
		else if((isParityEn != 0) && (isSdiEn == 0)){
			dataBitMask = Z246_23_BIT_MASK;
		}
		/* Else if parity is disabled and SDI is enabled then 22 bit space */
		else if((isParityEn == 0) && (isSdiEn != 0)){
			dataBitMask = Z246_22_BIT_MASK;
		}
		/* Else if parity is enabled and SDI is enabled then 21 bit space */
		else{
			dataBitMask = Z246_21_BIT_MASK;
		}
		/* Need to add support for user buffer tx */
		for(i=0;i<dataCount;i++){
			data = ReadFromBuffer(llHdl, &ringResult);
			if(ringResult != 0){
				return ERR_MBUF_UNDERRUN;
			}
			MWRITE_D32(llHdl->ma, Z246_FIFO_START_ADDR + (i*4), (dataBitMask & data));
			DBGWRT_2((DBH, "LL - Z246_Write: Tx Data[%d] = 0x%x\n",i, (dataBitMask & data)));
		}
	} /* Else dataCount < len so it will land in if(dataCount < len) condition */

	/* If data is remaining then enable the queue space interrupt. */
	if(llHdl->ringDataCnt != 0){
		DBGWRT_2((DBH, ">>> Z246_Write: TXA data len %d\n", dataCount));
		/* Acknowledge the the data before enabling the queue space interrupt. */
		MWRITE_D8(llHdl->ma, Z246_TX_TXA_OFFSET, dataCount);
		/* Enable the queue space interrupt. */
		MWRITE_D8(llHdl->ma, Z246_TX_IER_OFFSET, 1);
	}else{
		/* Else disable the queue space interrupt. */
		MWRITE_D8(llHdl->ma, Z246_TX_IER_OFFSET, 0);

		DBGWRT_2((DBH, ">>> Z246_Write: TXA data len %d\n", dataCount));
		/* Acknowledge the the data after disabling the queue space interrupt. */
		MWRITE_D8(llHdl->ma, Z246_TX_TXA_OFFSET, dataCount);
		DBGWRT_2((DBH, ">>> Z246_Write: TXC %d\n", MREAD_D8(llHdl->ma, Z246_TX_TXC_OFFSET)));

	}
	return result;
}


/**********************************************************************/
/** Read data from the buffer.
 *
 *  Read one word from the ring buffer.
 *
 *  \param llHdl      \IN  low-level handle
 *  \param result     \OUT result of the operation; 0 on success and -1 on error
 *  \return           \OUT uint32 data
 */
u_int32 ReadFromBuffer( LL_HANDLE *llHdl, int8 * result){

	u_int32 data = 0;
	/* if the head isn't ahead of the tail, we don't have any characters */
	if ((llHdl->ringHead == llHdl->ringTail) || (llHdl->ringDataCnt == 0)) {
		data = 0;        /* quit with an error */
		*result = -1;
	} else {
		data = llHdl->ringBuffer[llHdl->ringTail];
		llHdl->ringTail = (unsigned int)(llHdl->ringTail + 1);
		llHdl->ringDataCnt--;
		if (llHdl->ringDataCnt == 0) {
			llHdl->ringTail = Z246_RING_SIZE_DEFAULT;
			llHdl->ringHead = Z246_RING_SIZE_DEFAULT;
		}
		*result = 0;

	}
	return data;
}

/**********************************************************************/
/** Store data in the buffer.
 *
 *
 *  \param llHdl      \IN low-level handle
 *  \param data       \IN uint32 data
 *  \return           \OUT result of the operation; 0 on success and -1 on error.
 */
int8 StoreInBuffer( LL_HANDLE *llHdl , u_int32 data){

	int8 result = 0;
	unsigned int next = (unsigned int)(llHdl->ringHead + 1);
	if (next != llHdl->ringTail)
	{
		llHdl->ringBuffer[llHdl->ringHead] = data;
		llHdl->ringHead = next;
		llHdl->ringDataCnt++;
	}else{
		DBGWRT_1((DBH, ">>> Z246: StoreInBuffer next[0x%lx] == llHdl->ringTail[0x%lx]\n", next, llHdl->ringTail));
		result = -1;
	}
	return result;
}


/**********************************************************************/
/** Print register configuration.
 *
 *  Print the status of the registers.
 *
 *  \param llHdl      \IN low-level handle
 */


void RegStatus(LL_HANDLE *llHdl ){
	DBGWRT_1((DBH, " \n"));
	DBGWRT_1((DBH, " >>  LL - Z246_drv status: stat reg 0 = 0x%x\n", MREAD_D32(llHdl->ma, Z246_TX_IIR_OFFSET)));
	DBGWRT_1((DBH, " >>  LL - Z246_drv status: txa = 0x%x\n", MREAD_D8(llHdl->ma, Z246_TX_TXA_OFFSET)));
	DBGWRT_1((DBH, " >>  LL - Z246_drv status: txc = 0x%x\n", MREAD_D8(llHdl->ma, Z246_TX_TXC_OFFSET)));
	DBGWRT_1((DBH, " >>  LL - Z246_drv status: IER = 0x%x\n", MREAD_D8(llHdl->ma, Z246_TX_IER_OFFSET)));
	DBGWRT_1((DBH, " >>  LL - Z246_drv status: LCR = 0x%x\n", MREAD_D8(llHdl->ma, Z246_TX_LCR_OFFSET)));
	DBGWRT_1((DBH, " >>  LL - Z246_drv status: FCR = 0x%x\n", MREAD_D8(llHdl->ma, Z246_TX_FCR_OFFSET)));
	DBGWRT_1((DBH, " >>  LL - Z246_drv status: LA = 0x%x\n", MREAD_D8(llHdl->ma, Z246_TX_LA_OFFSET)));
	DBGWRT_1((DBH, " \n"));
}
 
