/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  z146_drv.c
 *
 *      \author  APatil
 *        $Date: 2016/11/28 12:59:21 $
 *    $Revision: 1.3 $
 *
 *      \brief   Low-level driver for ARINC429 receiver
 *
 *     Required: OSS, DESC, DBG libraries
 *
 *     \switches _ONE_NAMESPACE_PER_DRIVER_
 */
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: z146_drv.c,v $
 * Revision 1.3  2016/11/28 12:59:21  atlbitbucket
 * Stash autocheckin
 *
 * Revision 1.1  2015/10/16 18:07:59  ts
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

#define Z146_MAX_BUFF_SIZE		4096


/* toggle mode defines */
#define TOG_TIME_DEFAULT   1000       /**< default toggle time [ms]  */
#define TOG_TIME_MIN       100        /**< min toggle time (100ms)  */
#define TOG_TIME_MAX       60000      /**< max toggle time (1min)   */
#define TOG_CYCLIC         1          /**< endless timer flag        */

/* descriptor key defines*/
#define RESET_DEFAULT      0          /**< default arwen reset (enabled)     */
#define RESET_OFF          1          /**< disables the arwen reset function */

#define Z146_STAT_REG				0x400			/**< Offset of the status register. */
#define Z146_LSR_REG_OFFSET			0x402		/**< Offset of the LSR status register. */
#define Z146_LSR_RESET_VAL 			0x1E		/**< Reset value of the LSR status register. */
#define Z146_RX_IRQ_MASK    		0x7			/**< Receive IRQ mask. */
#define Z146_RX_RXC_OFFSET			0x03		/**< Status register for received word count. */
#define Z146_RX_RXC_REG_OFFSET  	0x403		/**< Status register for received word count. */

#define Z146_RX_RXA_OFFSET			0x404		/**< Status register for acknowledge word count. */

#define Z146_RX_IER_OFFSET			0x408		/**< Offset of the RX interrupt register */
#define Z146_RX_IER_DEFAULT			0x3		    /**< Default value of the RX interrupt register */

#define Z146_RX_RXCIEN_OFFSET		0x0	    	/**< Offset of the RX and character timout interrupt enable bit. */
#define Z146_RX_RXCIEN_MASK 		0x1	    	/**< Mask of the RX and character timout interrupt enable bit. */
#define Z146_RX_RLSIEN_OFFSET		0x1  		/**< Offset of the receive line status interrupt enable bit. */
#define Z146_RX_RLSIEN_MASK  		0x2  		/**< Mask of the receive line status interrupt enable bit. */

#define Z146_RX_LCR_OFFSET			0x409		/**< Offset of the RX_LCR register */
#define Z146_RX_LCR_DEFAULT			0x17		/**< Default value of the RX_LCR register */


#define Z146_RX_SPEED_OFFSET		0x00		/**< Offset of the register RX_LCR RX_SPEED bit */
#define Z146_RX_SPEED_MASK  		0x01		/**< Mask of the register RX_LCR RX_SPEED bit */
#define Z146_RX_ERR_WE_OFFSET		0x01		/**< Offset of the register RX_LCR RX error write enable bit */
#define Z146_RX_ERR_WE_MASK			0x02		/**< Mask of the register RX_LCR RX error write enable bit */
#define Z146_RX_PAR_EN_OFFSET		0x02		/**< Offset of the register RX_LCR RX parity enable bit */
#define Z146_RX_PAR_EN_MASK 		0x04		/**< Mask of the register RX_LCR RX parity enable bit */
#define Z146_RX_PAR_TYP_OFFSET		0x03		/**< Offset of the register RX_LCR RX parity type bit */
#define Z146_RX_PAR_TYP_MASK		0x08		/**< Mask of the register RX_LCR RX parity type bit */
#define Z146_RX_LAB_EN_OFFSET		0x04		/**< Offset of the register RX_LCR RX label enable bit */
#define Z146_RX_LAB_EN_MASK			0x10		/**< Mask of the register RX_LCR RX label enable bit */
#define Z146_RX_SDI_EN_OFFSET		0x05		/**< Offset of the register RX_LCR RX source/destination identifier enable bit */
#define Z146_RX_SDI_EN_MASK  		0x20		/**< Mask of the register RX_LCR RX source/destination identifier enable bit */
#define Z146_RX_SDI_OFFSET	     	0x06		/**< Offset of the register RX_LCR RX source/destination identifier mask */
#define Z146_RX_SDI_MASK	     	0xC0		/**< Mask of the register RX_LCR RX source/destination identifier mask */
#define Z146_RX_SDI_MAX  	     	0x03		/**< RX_LCR RX source/destination identifier max value */
#define Z146_RX_THR_LEV_MASK		0x07		/**< Offset of the register RX_LCR RX threshold level mask */

#define Z146_RX_FCR_OFFSET			0x40A		/**< Offset of the RX_FCR register */
#define Z146_RX_FCR_DEFAULT			0x5			/**< Default value of the RX_FCR register */

#define Z146_RX_TIMEOUT_OFFSET		0x414		/**< Offset of the RX timeout register */
#define Z146_RX_TIMEOUT_DEFAULT		0x1E		/**< Default value of the RX timeout register */


#define Z146_RX_LA_OFFSET			0x480		/**< Offset of the receive labels register */
#define Z146_RX_LA_SIZE				16			/**< Size of the receive labels register in bytes */
#define Z146_RX_LA_DEFAULT			0		    /**< Default value of the receive label register */

#define Z146_RX_LA_NUM_OFFSET		0x40B		/**< Offset of the RX label numbers register */
#define Z146_RX_LA_NUM_DEFAULT		0		    /**< Default value of the RX label numbers register */
#define Z146_RX_LA_NUM_MASK 		0x1F		/**< Bit mask of the RX label numbers register */

#define Z146_RX_FIFO_START_ADDR		0x000		/**< Receive FIFO start address. */
#define Z146_RX_FIFO_LEN			255		    /**< Receive FIFO size. */

#define Z146_RX_LINE_STAT_IRQ		1			/**< Receive line status interrupt offset */
#define Z146_RX_DATA_AVAIL_IRQ		2			/**< Receive data available interrupt offset */
#define Z146_RX_CHAR_TIMEOUT_IRQ	4			/**< Receive character timeout offset */

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

	OSS_SIG_HANDLE          *rxDataSig; /**< signal for port change */
	OSS_SIG_HANDLE          *rxErrorSig; /**< signal for error in reception */

	/* toggle mode */
	OSS_ALARM_HANDLE        *alarmHdl;      /**< alarm handle               */
	OSS_SEM_HANDLE          *devSemHdl;     /**< device semaphore handle    */

	/* Ring buffer parameters */
	u_int32					ringBuffer[Z146_MAX_BUFF_SIZE];
	volatile u_int32	    ringHead;
	volatile u_int32 		ringTail;
	volatile u_int32 		ringDataCnt;

} LL_HANDLE;


/* include files which need LL_HANDLE */
#include <MEN/ll_entry.h>       /* low-level driver jump table */
#include <MEN/z146_drv.h>        /* Z146 driver header file      */

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
static int32 Z146_Init(DESC_SPEC *descSpec, OSS_HANDLE *osHdl,
						MACCESS *ma, OSS_SEM_HANDLE *devSemHdl,
						OSS_IRQ_HANDLE *irqHdl, LL_HANDLE **llHdlP);
static int32 Z146_Exit(LL_HANDLE **llHdlP);
static int32 Z146_Read(LL_HANDLE *llHdl, int32 ch, int32 *value);
static int32 Z146_Write(LL_HANDLE *llHdl, int32 ch, int32 value);
static int32 Z146_SetStat(LL_HANDLE *llHdl,int32 ch, int32 code,
							INT32_OR_64 value32_or_64);
static int32 Z146_GetStat(LL_HANDLE *llHdl, int32 ch, int32 code,
							INT32_OR_64 *value32_or64P);
static int32 Z146_BlockRead(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
							int32 *nbrRdBytesP);
static int32 Z146_BlockWrite(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
								int32 *nbrWrBytesP);
static int32 Z146_Irq(LL_HANDLE *llHdl);

static int32 Z146_Info(int32 infoType, ...);
static char* Ident(void);
static int32 Cleanup(LL_HANDLE *llHdl, int32 retCode);
static void  ConfigureDefault( LL_HANDLE *llHdl );
static void RegStatus(LL_HANDLE *llHdl);
static u_int32 ReadFromBuffer( LL_HANDLE *llHdl, int8 * result);
static int8 StoreInBuffer( LL_HANDLE *llHdl , u_int32 data);


/****************************** Z146_GetEntry ********************************/
/** Initialize driver's jump table
 *
 *  \param drvP     \OUT pointer to the initialized jump table structure
 */
#ifdef _ONE_NAMESPACE_PER_DRIVER_
	extern void LL_GetEntry(
		LL_ENTRY* drvP
	)
#else
	extern void __Z146_GetEntry(
		LL_ENTRY* drvP
	)
#endif
{
	drvP->init        = Z146_Init;
	drvP->exit        = Z146_Exit;
	drvP->read        = Z146_Read;
	drvP->write       = Z146_Write;
	drvP->blockRead   = Z146_BlockRead;
	drvP->blockWrite  = Z146_BlockWrite;
	drvP->setStat     = Z146_SetStat;
	drvP->getStat     = Z146_GetStat;
	drvP->irq         = Z146_Irq;
	drvP->info        = Z146_Info;
}

/******************************** Z146_Init **********************************/
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
static int32 Z146_Init(
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
	/* Receive buffer */
	llHdl->ringHead    = 0;
	llHdl->ringTail    = 0;
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

	DBGWRT_1((DBH, "Z146_Init: base address = %08p\n", (void*)llHdl->ma));

	/* Initialize all elements in one shot */

	/*------------------------------+
	|  init hardware                |
	+------------------------------*/
	ConfigureDefault(llHdl);
	*llHdlP = llHdl;		/* set low-level driver handle */

	return (ERR_SUCCESS);
}

/****************************** Z146_Exit ************************************/
/** De-initialize hardware and clean up memory
 *
 *  The function deinitializes all channels by setting them as inputs.
 *  The interrupt is disabled.
 *
 *  \param llHdlP     \IN  pointer to low-level driver handle
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z146_Exit(
	LL_HANDLE **llHdlP
)
{

	LL_HANDLE *llHdl = *llHdlP;
	int32 error = 0;

	DBGWRT_1((DBH, "Z146_Exit\n"));

	/*------------------------------+
	|  clean up memory              |
	+------------------------------*/
	*llHdlP = NULL;		/* set low-level driver handle to NULL */
	error = Cleanup(llHdl, error);
	/*------------------------------+
	|  free memory                  |
	+------------------------------*/
	/* free my handle */
	OSS_MemFree(llHdl->osHdl, (int8*)llHdl, llHdl->memAlloc);

	return (error);
}

/****************************** Z146_Read ************************************/
/** Read a value from the device
 *
 *  The function is not supported and always returns an ERR_LL_ILL_FUNC error.
 *
 *  \param llHdl      \IN  low-level handle
 *  \param ch         \IN  current channel
 *  \param valueP     \OUT read value
 *
 *  \return           \c 0 on success or error code
 */
static int32 Z146_Read(
    LL_HANDLE *llHdl,
    int32 ch,
    int32 *valueP
)
{
    DBGWRT_1((DBH, "LL - Z146_Read: ch=%d, valueP=%d\n",ch,*valueP));

	return( ERR_LL_ILL_FUNC );
}

/****************************** Z146_Write ***********************************/
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
static int32 Z146_Write(
    LL_HANDLE *llHdl,
    int32 ch,
    int32 value
)
{
    DBGWRT_1((DBH, "LL USE_IRQ- Z146_Write: ch=%d, size=%d\n",ch,value));

	return( ERR_LL_ILL_FUNC );
}

/****************************** Z146_SetStat *********************************/
/** Set the driver status
 *
 *  The driver supports \ref rx_getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *  Note: only inputs are able fire an interrupt
 *
 *  \param llHdl         \IN  low-level handle
 *  \param code          \IN  \ref rx_getstat_setstat_codes "status code"
 *  \param ch            \IN  current channel
 *  \param value32_or_64 \IN  data or pointer to block data structure
 *                            (M_SG_BLOCK) for block status codes
 *  \return              \c 0 on success or error code
 */
static int32 Z146_SetStat(
	LL_HANDLE   *llHdl,
	int32       code,
	int32       ch,
	INT32_OR_64 value32_or_64
)
{
	int32 value = (int32)value32_or_64;		/* 32bit value */
	int32 error = ERR_SUCCESS;
	u_int8 regData = 0;
	u_int8 label = 0;
	u_int8 labCnt = 0;
	u_int8 isLab = 0;
	u_int32 i = 0;
	DBGWRT_1((DBH, "LL - Z146_SetStat: ch=%d code=0x%04x value=0x%x\n",
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
		case Z146_SET_SIGNAL:
			/* signal already installed ? */
			if (llHdl->rxDataSig) {
				error = ERR_OSS_SIG_SET;
				break;
			}
			error = OSS_SigCreate(OSH, value, &llHdl->rxDataSig);
			break;
			/*--------------------------+
		|  unregister signal        |
		+--------------------------*/
		case Z146_CLR_SIGNAL:
			/* signal already installed ? */
			if (llHdl->rxDataSig == NULL) {
				error = ERR_OSS_SIG_CLR;
				break;
			}
			error = OSS_SigRemove(OSH, &llHdl->rxDataSig);
			break;

			/*--------------------------+
			|  register signal          |
			+--------------------------*/
		case Z146_SET_ERROR_SIGNAL:
			/* signal already installed ? */
			if (llHdl->rxErrorSig) {
				error = ERR_OSS_SIG_SET;
				break;
			}
			error = OSS_SigCreate(OSH, value, &llHdl->rxErrorSig);
			break;
			/*--------------------------+
			|  unregister signal        |
			+--------------------------*/
		case Z146_CLR_ERROR_SIGNAL:
			/* signal already installed ? */
			if (llHdl->rxErrorSig == NULL) {
				error = ERR_OSS_SIG_CLR;
				break;
			}
			error = OSS_SigRemove(OSH, &llHdl->rxErrorSig);
			break;
			/*------------------------------------------------+
			|  RX and Character timout interrupt status       |
			+-------------------------------------------------*/
		case Z146_RX_RXC_IRQ_STAT:
			/* Set IRQ mode */
			regData = MREAD_D8(llHdl->ma, Z146_RX_IER_OFFSET);
			if(value != 0){
				regData = regData | Z146_RX_RXCIEN_MASK;
			}else{
				DBGWRT_1((DBH, "LL - Z146_SetStat:Z146_RX_RXC_IRQ_STAT regData 0x%04x, 0x%04x\n", regData, (~Z146_RX_RXCIEN_OFFSET)));
				regData = regData & (~Z146_RX_RXCIEN_MASK);
			}
			MWRITE_D8(llHdl->ma, Z146_RX_IER_OFFSET, regData);
			DBGWRT_1((DBH, "LL - Z146_SetStat: Z146_RX_RXC_IRQ_STAT 0x%04x\n", MREAD_D8(llHdl->ma, Z146_RX_IER_OFFSET)));
			break;

			/*--------------------------------------------+
			|  Receive line status interrupt status       |
			+---------------------------------------------*/
		case Z146_RX_RLS_IRQ_STAT:
			/* Set IRQ mode */
			regData = MREAD_D8(llHdl->ma, Z146_RX_IER_OFFSET);
			if(value32_or_64 != 0){
				regData = regData | Z146_RX_RLSIEN_MASK;
			}else{
				regData = regData & (~Z146_RX_RLSIEN_MASK);
			}
			MWRITE_D8(llHdl->ma, Z146_RX_IER_OFFSET, regData);
			DBGWRT_1((DBH, "LL - Z146_SetStat:Z146_RX_RLS_IRQ_STAT 0x%04x\n", MREAD_D8(llHdl->ma, Z146_RX_IER_OFFSET)));
			break;

		/*-----------------------+
		|  Set Receive Label     |
		+------------------------*/
		case Z146_RX_SET_LABEL:
			regData = (value32_or_64 & 0xFF);
			labCnt = (MREAD_D8(llHdl->ma, Z146_RX_LA_NUM_OFFSET) & Z146_RX_LA_NUM_MASK);
			/* Check for the max limit of the count. */
			if(labCnt < Z146_RX_LA_NUM_MASK){
				for(i=0;i<labCnt;i++){
					label = MREAD_D8(llHdl->ma, Z146_RX_LA_OFFSET + i);
					/* Check whether the label exists. */
					if(label == regData){
						DBGWRT_1((DBH, "LL - Z146_SetStat:Z146_RX_SET_LABEL = 0x%04x already exists in the list.\n", regData));
						isLab = 1;
						break;
					}
				}
				if(isLab == 0){
					/* Write the label. */
					MWRITE_D8(llHdl->ma, Z146_RX_LA_OFFSET + i, regData);
					/* Increment the count */
					labCnt = labCnt + 1;
					MWRITE_D8(llHdl->ma, Z146_RX_LA_NUM_OFFSET, (labCnt &  Z146_RX_LA_NUM_MASK));
					DBGWRT_1((DBH, "LL - Z146_SetStat:Z146_RX_SET_LABEL = 0x%04x set @ %d position\n", regData, i));
				}
			}else{
				error = ERR_LL_WRITE;
				DBGWRT_1((DBH, "LL - Z146_SetStat:Z146_RX_SET_LABEL = 0x%04x; Error- label queue is full\n", regData));
			}
			break;


		/*-------------------------+
		|  Reset Receive Label     |
		+--------------------------*/
		case Z146_RX_RESET_LABEL:
			regData = (value32_or_64 & 0xFF);
			labCnt = (MREAD_D8(llHdl->ma, Z146_RX_LA_NUM_OFFSET) & Z146_RX_LA_NUM_MASK);
			/* Check for the max limit of the count. */
			if(labCnt != 0){
				for(i=0;i<labCnt;i++){
					label = MREAD_D8(llHdl->ma, Z146_RX_LA_OFFSET + i);
					/* Check whether the label exists. */
					if((label == regData) && (isLab == 0)){
						/* The last label count shall be treated differently. */
						if((i+1) == labCnt){
							/* Reset the label. */
							MWRITE_D8(llHdl->ma, Z146_RX_LA_OFFSET + i, 0);
							/* Decrement the count */
							labCnt = (labCnt - 1);
							MWRITE_D8(llHdl->ma, Z146_RX_LA_NUM_OFFSET, (labCnt &  Z146_RX_LA_NUM_MASK));
							DBGWRT_1((DBH, "LL - Z146_SetStat:Z146_RX_RESET_LABEL : New label count set to = %d\n", labCnt &  Z146_RX_LA_NUM_MASK));
						}else{
							/* Else set the flag for the next loop. */
							isLab = 1;
						}
					}else if(isLab == 1){
						/* Decrement the label position.*/
						MWRITE_D8(llHdl->ma, Z146_RX_LA_OFFSET + (i - 1), label);
						/* Decrement the count */
						regData = (labCnt - 1);
						MWRITE_D8(llHdl->ma, Z146_RX_LA_NUM_OFFSET, (regData &  Z146_RX_LA_NUM_MASK));
						DBGWRT_1((DBH, "LL - Z146_SetStat:Z146_RX_RESET_LABEL : label = %d old position = %d, new position = %d\n", label, i, i-1));
					} /* Else continue the loop. */
				}

			}else{
				error = ERR_LL_WRITE;
				DBGWRT_1((DBH, "LL - Z146_SetStat:Z146_RX_RESET_LABEL = 0x%04x; Error- label queue is empty\n", regData));
			}
			break;
		/*---------------------------+
		|  Receive speed status      |
		+----------------------------*/
		case Z146_RX_SPEED:
			regData = MREAD_D8(llHdl->ma, Z146_RX_LCR_OFFSET);
			if(value32_or_64 != 0){
				regData = regData | Z146_RX_SPEED_MASK;
			}else{
				regData = regData & (~Z146_RX_SPEED_MASK);
			}
			MWRITE_D8(llHdl->ma, Z146_RX_LCR_OFFSET, regData);
			DBGWRT_1((DBH, "LL - Z146_SetStat: Z146_RX_SPEED: value = %d", value32_or_64));
			break;

		/*----------------------------------------+
		|  Receive error write enable status      |
		+-----------------------------------------*/
		case Z146_ERR_WE:
			regData = MREAD_D8(llHdl->ma, Z146_RX_LCR_OFFSET);
			if(value32_or_64 != 0){
				regData = regData | Z146_RX_ERR_WE_MASK;
			}else{
				regData = regData & (~Z146_RX_ERR_WE_MASK);
			}
			MWRITE_D8(llHdl->ma, Z146_RX_LCR_OFFSET, regData);
			break;

		/*---------------------------------------------+
		|  Receive receive parity enable enable status |
		+----------------------------------------------*/
		case Z146_PAR_EN:
			regData = MREAD_D8(llHdl->ma, Z146_RX_LCR_OFFSET);
			if(value32_or_64 != 0){
				regData = regData | Z146_RX_PAR_EN_MASK;
			}else{
				regData = regData & (~Z146_RX_PAR_EN_MASK);
			}
			MWRITE_D8(llHdl->ma, Z146_RX_LCR_OFFSET, regData);
			break;

		/*----------------------------------------+
		|  Receive receive parity type status      |
		+-----------------------------------------*/
		case Z146_PAR_TYP:
			regData = MREAD_D8(llHdl->ma, Z146_RX_LCR_OFFSET);
			if(value32_or_64 != 0){
				regData = regData | Z146_RX_PAR_TYP_MASK;
			}else{
				regData = regData & (~Z146_RX_PAR_TYP_MASK);
			}
			MWRITE_D8(llHdl->ma, Z146_RX_LCR_OFFSET, regData);
			break;

		/*----------------------------------------+
		|  ReceiRegStatusve receive label enable status    |
		+-----------------------------------------*/
		case Z146_LAB_EN:
			regData = MREAD_D8(llHdl->ma, Z146_RX_LCR_OFFSET);
			if(value32_or_64 != 0){
				regData = regData | Z146_RX_LAB_EN_MASK;
			}else{
				regData = regData & (~Z146_RX_LAB_EN_MASK);
			}
			MWRITE_D8(llHdl->ma, Z146_RX_LCR_OFFSET, regData);
			break;
		/*----------------------------------------+
		|  Receive src/dst identifier enable status    |
		+-----------------------------------------*/
		case Z146_SDI_EN:
			regData = MREAD_D8(llHdl->ma, Z146_RX_LCR_OFFSET);
			if(value32_or_64 != 0){
				regData = regData | Z146_RX_SDI_EN_MASK;
			}else{
				regData = regData & (~Z146_RX_SDI_EN_MASK);
			}
			MWRITE_D8(llHdl->ma, Z146_RX_LCR_OFFSET, regData);
			break;

		/*--------------------------------------+
		|  Receive src/dst identifier status    |
		+---------------------------------------*/
		case Z146_SDI:
			if(value32_or_64 <= Z146_RX_SDI_MAX){
				regData = MREAD_D8(llHdl->ma, Z146_RX_LCR_OFFSET);
				regData = regData & (~Z146_RX_SDI_MASK);
				regData |= (value32_or_64 << Z146_RX_SDI_OFFSET);
				MWRITE_D8(llHdl->ma, Z146_RX_LCR_OFFSET, regData);
			}else{
				error = ERR_LL_ILL_PARAM;
			}
			break;

		/*--------------------------------------+
		|  Receive threshold level status    |
		+---------------------------------------*/
		case Z146_RX_THR_LEV:
			regData = MREAD_D8(llHdl->ma, Z146_RX_FCR_OFFSET);
			regData = regData & (~Z146_RX_THR_LEV_MASK);
			regData |= (value32_or_64 & Z146_RX_THR_LEV_MASK);
			MWRITE_D8(llHdl->ma, Z146_RX_FCR_OFFSET, regData);
			break;

		/*--------------------------+
		|  (unknown)                |
		+--------------------------*/
		default:
			error = ERR_LL_UNK_CODE;
	}

	return (error);
}

/****************************** Z146_GetStat *********************************/
/** Get the driver status
 *
 *  The driver supports \ref rx_getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *  and clear it than.
 *  \param llHdl             \IN  low-level handle
 *  \param code              \IN  \ref rx_getstat_setstat_codes "status code"
 *  \param ch                \IN  current channel
 *  \param value32_or_64P    \IN  pointer to block data structure (M_SG_BLOCK) for
 *                                block status codes
 *  \param value32_or_64P    \OUT data pointer or pointer to block data structure
 *                                (M_SG_BLOCK) for block status codes
 *
 *  \return                  \c 0 on success or error code
 */
static int32 Z146_GetStat(
	LL_HANDLE   *llHdl,
	int32       code,
	int32       ch,
	INT32_OR_64 *value32_or_64P
)
{
	int32 *valueP = (int32*)value32_or_64P;		/* pointer to 32bit value */
	INT32_OR_64 *value64P = value32_or_64P;		/* stores 32/64bit pointer */
	int32 error = ERR_SUCCESS;
	u_int8 regData = 0;

	DBGWRT_1((DBH, "LL - Z146_GetStat: ch=%d code=0x%04x\n", ch, code));

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
		|  RX data length           |
		+--------------------------*/
		case Z146_RX_DATA_LEN:
			*value64P = (INT32_OR_64)llHdl->ringDataCnt;
			break;

		/*------------------------------------------------+
		|  RX and Character timout interrupt status       |
		+-------------------------------------------------*/
		case Z146_RX_RXC_IRQ_STAT:
			*value64P = (INT32_OR_64)(MREAD_D8(llHdl->ma, Z146_RX_IER_OFFSET) & Z146_RX_RXCIEN_OFFSET);
			break;

		/*--------------------------------------------+
		|  Receive line status interrupt status       |
		+---------------------------------------------*/
		case Z146_RX_RLS_IRQ_STAT:
			regData = MREAD_D8(llHdl->ma, Z146_RX_IER_OFFSET);
			*value64P = (INT32_OR_64)((regData & Z146_RX_RLSIEN_MASK) >> Z146_RX_RLSIEN_OFFSET );
			break;

		/*---------------------------+
		|  Receive speed status      |
		+----------------------------*/
		case Z146_RX_SPEED:
			regData = MREAD_D8(llHdl->ma, Z146_RX_LCR_OFFSET);
			*value64P = (INT32_OR_64)(regData & Z146_RX_SPEED_MASK);
			break;

		/*----------------------------------------+
		|  Receive error write enable status      |
		+-----------------------------------------*/
		case Z146_ERR_WE:
			regData = MREAD_D8(llHdl->ma, Z146_RX_LCR_OFFSET);
			*value64P = (INT32_OR_64)((regData & Z146_RX_ERR_WE_MASK) >> Z146_RX_ERR_WE_OFFSET );
			break;

		/*----------------------------------------+
		|  Receive receive parity enable enable status      |
		+-----------------------------------------*/
		case Z146_PAR_EN:
			regData = MREAD_D8(llHdl->ma, Z146_RX_LCR_OFFSET);
			*value64P = (INT32_OR_64)((regData & Z146_RX_PAR_EN_MASK) >> Z146_RX_PAR_EN_OFFSET);
			break;

		/*----------------------------------------+
		|  Receive receive parity type status      |
		+-----------------------------------------*/
		case Z146_PAR_TYP:
			regData = MREAD_D8(llHdl->ma, Z146_RX_LCR_OFFSET);
			*value64P = (INT32_OR_64)((regData & Z146_RX_PAR_TYP_MASK) >> Z146_RX_PAR_TYP_OFFSET);
			break;

		/*----------------------------------------+
		|  ReceiRegStatusve receive label enable status    |
		+-----------------------------------------*/
		case Z146_LAB_EN:
			regData = MREAD_D8(llHdl->ma, Z146_RX_LCR_OFFSET);
			*value64P = (INT32_OR_64)((regData & Z146_RX_LAB_EN_MASK) >> Z146_RX_LAB_EN_OFFSET);
			break;

		/*----------------------------------------+
		|  Receive src/dst identifier enable status    |
		+-----------------------------------------*/
		case Z146_SDI_EN:
			regData = MREAD_D8(llHdl->ma, Z146_RX_LCR_OFFSET);
			*value64P = (INT32_OR_64)((regData & Z146_RX_SDI_EN_MASK) >> Z146_RX_SDI_EN_OFFSET);
			break;

		/*--------------------------------------+
		|  Receive src/dst identifier status    |
		+---------------------------------------*/
		case Z146_SDI:
			regData = MREAD_D8(llHdl->ma, Z146_RX_LCR_OFFSET);
			*value64P = (INT32_OR_64)((regData & Z146_RX_SDI_MASK) >> Z146_RX_SDI_OFFSET);
			break;

		/*--------------------------------------+
		|  Receive threshold level status    |
		+---------------------------------------*/
		case Z146_RX_THR_LEV:
			regData = MREAD_D8(llHdl->ma, Z146_RX_FCR_OFFSET);
			*value64P = (INT32_OR_64)((regData & Z146_RX_THR_LEV_MASK));
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

/******************************* Z146_BlockRead ******************************/
/** Read a data block from the driver buffer when interrupt is enabled. If the
 *  itnerrupt is disabled then reads the data block from the FPGA FIFO.
 *
 *  \param llHdl       \IN  low-level handle
 *  \param ch          \IN  current channel
 *  \param buf         \IN  data buffer
 *  \param size        \IN  data buffer size
 *  \param nbrRdBytesP \OUT number of read bytes
 *
 *  \return            \c 0 on success or error code
 */
static int32 Z146_BlockRead(
	LL_HANDLE *llHdl,
	int32     ch,
	void      *buf,
	int32     size,
	int32     *nbrRdBytesP
)
{
	int32 result = 0;
	u_int32 i = 0;
	u_int32 * userBuf = (u_int32*)buf;
	u_int32 llRxLen = 0;
	u_int32 statReg = 0;
	u_int32 dataLenByte = (llHdl->ringDataCnt * 4);
	u_int32 dataLenWord = llHdl->ringDataCnt;
	int8 readRes = 0;

	DBGWRT_1((DBH, ">>> LL - Z146_BlockRead: ch=%d, size=%d\n",ch,size));

    statReg = MREAD_D32(llHdl->ma, Z146_STAT_REG);

	if((nbrRdBytesP != NULL) && (buf != NULL)){

		IDBGWRT_1((DBH, ">>> LL - Z146_BlockRead: status register = %08x\n", statReg));
		IDBGWRT_1((DBH, ">>> LL - Z146_BlockRead: LSR = 0x%x\n", MREAD_D32(llHdl->ma, Z146_LSR_REG_OFFSET)));

		MWRITE_D32(llHdl->ma, 0x00, 0xFFFFFFFF);

		if((MREAD_D8(llHdl->ma, Z146_RX_IER_OFFSET) & 1) == 0){
			/* The interrupt is disabled therefore read directly from FIFO. */

			/* Read the RXC to get the received data length. */
			llRxLen = MREAD_D8(llHdl->ma, Z146_RX_RXC_REG_OFFSET);
			IDBGWRT_1((DBH, ">>> LL - Z146_BlockrRead: RXC Data length = %d\n", llRxLen));

			if(size >= (int32)llRxLen){
				for(i=0; i<llRxLen; i++){
					*userBuf++ = MREAD_D32(llHdl->ma, (Z146_RX_FIFO_START_ADDR + (i * 4)));
					IDBGWRT_1((DBH, ">>> LL - Z146_BlockRead: Data[%d] = 0x%x\n",i, *userBuf));
				}
				MWRITE_D8(llHdl->ma, Z146_RX_RXA_OFFSET, llRxLen);
				/* return number of read bytes */
				*nbrRdBytesP = (llRxLen * 4);
			}else{
				IDBGWRT_1((DBH, ">>> LL - Z146_BlockRead: (size >= llHdl->rxDataLen)\n"));
				result = ERR_MBUF_USERBUF;
				/* return number of read bytes */
				*nbrRdBytesP = 0;
			}

		}else{
			/* The interrupt is enabled therefore read from ring buffer. */
		    if(dataLenWord != 0 ){
		    	/* Check user buffer length */
		    	if(size >= (int32)dataLenByte){
		    		/* Do not use the ringDataCnt because it can be influenced by IRQ during the data copy. */
		    		for(i=0; i<dataLenWord; i++){
		    			*userBuf++ = ReadFromBuffer(llHdl, &readRes);
		    			if(readRes != 0){
		    				*nbrRdBytesP = 0;
		    				result = ERR_MBUF_OVERFLOW;
		    				IDBGWRT_1((DBH, ">>> LL - Z146_BlockRead: ring buffer problem \n"));
		    			}
		    		}
		    		*nbrRdBytesP = dataLenByte;
		    		IDBGWRT_1((DBH, ">>> LL - Z146_BlockRead: Data length byte = %d\n", dataLenByte));
		    	}else{
		    		IDBGWRT_1((DBH, ">>> LL - Z146_BlockRead: (size >= llHdl->rxDataLen)\n"));
		    		result = ERR_MBUF_USERBUF;
		    		/* return number of read bytes */
		    		*nbrRdBytesP = 0;
		    	}
		    }else{
		    	IDBGWRT_1((DBH, ">>> LL - Z146_BlockRead: llHdl->ringDataCnt = %d \n", llHdl->ringDataCnt));
		    	/* return number of read bytes */
		    	*nbrRdBytesP = 0;
		    }
		}
	}else{
		IDBGWRT_1((DBH, ">>> LL - Z146_BlockRead: ERR_MBUF_ILL_SIZE buf ptr = 0x%x, nbrRdBytesP ptr = 0x%x\n",buf, nbrRdBytesP));
		result = ERR_MBUF_ILL_SIZE;
		/* return number of read bytes */
		*nbrRdBytesP = 0;
	}
	RegStatus(llHdl);

	return result;
}

/****************************** Z146_BlockWrite *****************************/
/** Write a data block from the device.
 *
 *  The function is not supported and always returns an ERR_LL_ILL_FUNC error. 
 *
 *  \param llHdl  	   \IN  low-level handle
 *  \param ch          \IN  current channel
 *  \param buf         \IN  data buffer
 *  \param size        \IN  data buffer size
 *  \param nbrWrBytesP \OUT number of written bytes
 *
 *  \return            \c 0 on success or error code
 */
static int32 Z146_BlockWrite(
     LL_HANDLE *llHdl,
     int32     ch,
     void      *buf,
     int32     size,
     int32     *nbrWrBytesP
)
{
    DBGWRT_1((DBH, ">>> LL - LL - Z146_BlockWrite: ch=%d, size=%d\n",ch,size));

	/* return number of written bytes */
	*nbrWrBytesP = 0;

	return( ERR_LL_ILL_FUNC );
}

/****************************** Z146_Irq ************************************/
/** Interrupt service routine
 *
 *  The interrupt is when data is available in the FPGA FIFO or error detected
 *  on the receive line.
 *
 *  If the driver can detect the interrupt's cause it returns
 *  LL_IRQ_DEVICE or LL_IRQ_DEV_NOT, otherwise LL_IRQ_UNKNOWN.
 *  
 *
 *  \param llHdl       \IN  low-level handle
 *  \return LL_IRQ_DEVICE   irq caused by device
 *          LL_IRQ_DEV_NOT  irq not caused by device
 */
static int32 Z146_Irq(LL_HANDLE *llHdl){

	int32 result = LL_IRQ_DEV_NOT;
	u_int32 statReg = 0;

    statReg = MREAD_D32(llHdl->ma, Z146_STAT_REG);

    /* Check whether the interrupt caused by RX ? */
	if(statReg & Z146_RX_IRQ_MASK){

		u_int32 data = 0;
		u_int32 dataLen = 0;
	    u_int16 i = 0;
		IDBGWRT_3((DBH, ">>> LL - Z146_Irq: status register = %08x\n", statReg));
		IDBGWRT_3((DBH, ">>> LL - Z146_Irq: LSR = %08x\n", MREAD_D32(llHdl->ma, Z146_LSR_REG_OFFSET)));
		MWRITE_D32(llHdl->ma, 0x00, 0xFFFFFFFF);

		/* disable all IRQs */
		MWRITE_D8(llHdl->ma, Z146_RX_IER_OFFSET, 0);

		/* Call routine according to the interrupt. */
		if((statReg & Z146_RX_LINE_STAT_IRQ) != 0){
			/* An error is detected so discard all the received data. */

			/* Read the RXC to get the received data length. */
			dataLen = (statReg >> (Z146_RX_RXC_OFFSET * 8));

			IDBGWRT_2((DBH, ">>> LL - Z146_Irq with RX line error: dataLen %08x\n", dataLen));

			/* Acknowledge the received data, which will lead to discard. */
			MWRITE_D8(llHdl->ma, Z146_RX_RXA_OFFSET, dataLen);

			/* Clear the errors */
			MWRITE_D8(llHdl->ma, Z146_LSR_REG_OFFSET, Z146_LSR_RESET_VAL);
			/* if requested send signal to application */
			if (llHdl->rxErrorSig){
				OSS_SigSend(OSH, llHdl->rxErrorSig);
			}
			result = LL_IRQ_DEVICE ;
		}else{
			/* Read the RXC to get the received data length. */
			dataLen = (statReg >> (Z146_RX_RXC_OFFSET * 8)) &  0xFF;
			IDBGWRT_1((DBH, ">>> LL - Z146_Irq: Data length = %d\n", dataLen));

			/* Create buffer for the user data. */
			for(i=0; i<dataLen; i++){

				data = MREAD_D32(llHdl->ma, (Z146_RX_FIFO_START_ADDR + (i * 4)));
				StoreInBuffer(llHdl, data);
				IDBGWRT_1((DBH, ">>> LL - Z146_Irq: Rx Data word-%d = 0x%x\n",i, data));

			}

			/* Acknowledge the data . */
			MWRITE_D8(llHdl->ma, Z146_RX_RXA_OFFSET, dataLen);

			/* FIFO is empty now send signal to the application. */
			/* if requested send signal to application */
			if (llHdl->rxDataSig){
				OSS_SigSend(OSH, llHdl->rxDataSig);
			}

			result = LL_IRQ_DEVICE ;

		}
		/* Enable the default interrupts */
		MWRITE_D8(llHdl->ma, Z146_RX_IER_OFFSET, Z146_RX_IER_DEFAULT);

	}

	return result;
}

/****************************** Z146_Info ***********************************/
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
 *  The LL_INFO_ADDRSPACE code returns information about one
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
static int32 Z146_Info(
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
	return ("Z146 - Z146 low level driver: $Id: z146_drv.c,v 1.3 2016/11/28 12:59:21 atlbitbucket Exp $"
			#ifdef Z146_MODEL_Z127
				" Z127 model"
			#else
				" Z34/Z37 model"
			#endif
			#ifdef Z127_NOIRQ
				" - no IRQs"
			#endif
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

    /* reset the default interrupts */
    MWRITE_D8(llHdl->ma, Z146_RX_IER_OFFSET, 0);

    /* Doesn't need to clear the rest of the configuration c */

	/*return error code */
	return (retCode);
}

/**********************************************************************/
/** Configure default values to registers.
 *
 *  Sets the controller registers to default values:
 *  - all interrupt enabled
 *  - default timeout and receive settings
 *
 *
 *  \param llHdl      \IN  low-level handle
 */

static void
ConfigureDefault( LL_HANDLE *llHdl )
{
    int i =0;

    /* Configure RX LCR */
    MWRITE_D8(llHdl->ma, Z146_RX_LCR_OFFSET, Z146_RX_LCR_DEFAULT);

    /* Configure RX FCR */
    MWRITE_D8(llHdl->ma, Z146_RX_FCR_OFFSET, Z146_RX_FCR_DEFAULT);

    /* Configure RX timeout */
    MWRITE_D8(llHdl->ma, Z146_RX_TIMEOUT_OFFSET, Z146_RX_TIMEOUT_DEFAULT);

    /* Enable the default interrupts */
    MWRITE_D8(llHdl->ma, Z146_RX_IER_OFFSET, Z146_RX_IER_DEFAULT);

    /* Reset the receive label. */
    for(i=0;i<Z146_RX_LA_SIZE;i++ ){
    	MWRITE_D8(llHdl->ma, Z146_RX_LA_OFFSET, Z146_RX_LA_DEFAULT);
    }
    /* Reset the receive label numbers. */
    MWRITE_D8(llHdl->ma, Z146_RX_LA_NUM_OFFSET, (Z146_RX_LA_NUM_DEFAULT &  Z146_RX_LA_NUM_MASK));

    /* Clear the error register. */
    MWRITE_D8(llHdl->ma, Z146_LSR_REG_OFFSET, 0xFF);

    DBGWRT_1((DBH, " >>  LL - Z146_drv cfg after Init:"));
    RegStatus(llHdl);
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
		DBGWRT_1((DBH, " >>  LL - Z146: ReadFromBuffer either llHdl->ringHead(0x%lx) == llHdl->ringTail(0x%lx), or llHdl->ringDataCnt[%d] == 0\n", llHdl->ringHead == llHdl->ringTail, llHdl->ringDataCnt));
	} else {
		data = llHdl->ringBuffer[llHdl->ringTail];
		llHdl->ringTail = (unsigned int)(llHdl->ringTail + 1) % Z146_MAX_BUFF_SIZE;
		llHdl->ringDataCnt--;
		*result = 0;
	}
	return data;
}

/**********************************************************************/
/** Store data in the ring buffer.
 *
 *  \param llHdl      \IN low-level handle
 *  \param data       \IN uint32 data
 *  \return           \OUT result of the operation; 0 on success and -1 on error.
 */
int8 StoreInBuffer( LL_HANDLE *llHdl , u_int32 data){

	int8 result = 0;
	unsigned int next = (unsigned int)(llHdl->ringHead + 1) % Z146_MAX_BUFF_SIZE;
	if (next != llHdl->ringTail)
	{
		llHdl->ringBuffer[llHdl->ringHead] = data;
		llHdl->ringHead = next;
		llHdl->ringDataCnt++;
	}else{
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

void RegStatus(LL_HANDLE *llHdl){
	DBGWRT_1((DBH, " \n"));
	DBGWRT_1((DBH, " >>  LL - Z146_drv status: stat reg 0  = 0x%x\n", MREAD_D32(llHdl->ma, Z146_STAT_REG)));
	DBGWRT_1((DBH, " >>  LL - Z146_drv status: RXA  = 0x%x\n", MREAD_D8(llHdl->ma, Z146_RX_RXA_OFFSET)));
	DBGWRT_1((DBH, " >>  LL - Z146_drv status: RXC  = 0x%x\n", MREAD_D8(llHdl->ma, Z146_RX_RXC_REG_OFFSET)));
	DBGWRT_1((DBH, " >>  LL - Z146_drv status: IER  = 0x%x\n", MREAD_D8(llHdl->ma, Z146_RX_IER_OFFSET)));
	DBGWRT_1((DBH, " >>  LL - Z146_drv status: LCR  = 0x%x\n", MREAD_D8(llHdl->ma, Z146_RX_LCR_OFFSET)));
	DBGWRT_1((DBH, " >>  LL - Z146_drv status: FCR  = 0x%x\n", MREAD_D8(llHdl->ma, Z146_RX_FCR_OFFSET)));
	DBGWRT_1((DBH, " >>  LL - Z146_drv status: LA_NUM  = 0x%x\n", MREAD_D8(llHdl->ma, Z146_RX_LA_NUM_OFFSET)));
	DBGWRT_1((DBH, " >>  LL - Z146_drv status: LA  = 0x%x\n", MREAD_D8(llHdl->ma, Z146_RX_LA_OFFSET)));
	DBGWRT_1((DBH, " \n"));
}
 
 
