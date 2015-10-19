/****************************************************************************
 ************                                                    ************
 ************                   Z146_EXAMPLE                     ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file z146_interrupt_test.c
 *       \author Apatil
 *        $Date: 2015/10/16 18:09:13 $
 *    $Revision: 1.1 $
 *
 *       \brief  Simple example program for the Z146 driver
 *
 *               Reads and writes some values from/to GPIO ports,
 *               generating interrupts.
 *               Interrupts will be generated only on inputs. Thatsway
 *               normaly an external loopback from the outputs gpio[0]..[4]
 *               to gpio[5]..[7] is required.
 *
 *     Required: libraries: mdis_api, usr_oss
 *     \switches (none)
 */
/*-------------------------------[ History ]--------------------------------
 *
 * $Log: z146_interrupt_test.c,v $
 * Revision 1.1  2015/10/16 18:09:13  ts
 * Initial Revision
 *
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2003 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

static const char RCSid[]="$Id: z146_interrupt_test.c,v 1.1 2015/10/16 18:09:13 ts Exp $";

#include <stdio.h>
#include <string.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/z146_drv.h>
#include <MEN/z246_drv.h>

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   TYPDEFS                             |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   EXTERNALS                           |
+--------------------------------------*/
/* none */

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
static int G_sigCount = 0;

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void PrintError(char *info);
static void __MAPILIB SignalHandler( u_int32 sig );
static char* bitString( char *s, u_int32 val, int nrBits );

/********************************* main ************************************/
/** Program main function
 *
 *  \param argc       \IN  argument counter
 *  \param argv       \IN  argument vector
 *
 *  \return	          success (0) or error (1)
 */
int main(int argc, char *argv[])
{
	MDIS_PATH rxPath;
	MDIS_PATH txPath;
	char	*rxDevice;
	char	*txDevice;
	int32 result = 0;
	int i = 0;
	int errors = 0;
	u_int32 label = 0;
	u_int32 dataLen = 0;
	u_int32 txDataArray[4096];
	u_int32 rxDataArray[4096];
	printf("argc = %d\n",argc );
	if (argc < 3 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: z146_interrupt_test <rx_rxDevice> <txDevice>\n");
		printf("Function: Test for interrupt enable and disable.\n");
		printf("Options:\n");
		printf("rxDevice       rxDevice name    txDevice       TxDevice name\n");
		printf("\n");
		printf("%s \n",RCSid );
		return(1);
	}

	printf("%s \n",RCSid );

	rxDevice = argv[1];
	txDevice = argv[2];
	/*--------------------+
    |  open rxPath          |
    +--------------------*/
	if ((rxPath = M_open(rxDevice)) < 0) {
		PrintError("open");
		return(1);
	}
	if ((txPath = M_open(txDevice)) < 0) {
		PrintError("open");
		return(1);
	}

	label   = 5;
	result = M_setstat(txPath, Z246_TX_LABEL, label);
	if(result != 0){
		printf("TxSetStat failed with result %ld\n", result);
		errors++;
	}
	result = M_setstat(rxPath, Z146_RX_SET_LABEL, label);
	if(result != 0){
		printf("RxSetStat failed with result %ld\n", result);
		errors++;
	}


	/*--------------------+
    |  config             |
    +--------------------*/
	/* install signal which will be received at change of input ports */
	UOS_SigInit( SignalHandler );
	UOS_SigInstall( UOS_SIG_USR1 );
//	M_setstat(rxPath, Z146_SET_SIGNAL, UOS_SIG_USR1);

	printf("\n");
	printf("\n");
	printf("############# TESTING INTERRUPT (Enable) #############\n");
	printf("############# Transmit ###############\n");
	dataLen   = 100;
	printf("User data length = %ld and dataptr = 0x%lx\n", dataLen *4, txDataArray);
	for(i=0;i<(int)dataLen;i++){
		txDataArray[i] = i+ 7;
		printf(" 0x%lx", txDataArray[i]);
	}
	printf("\n");
	printf("\n");
	result = M_setblock(txPath, (u_int8*)txDataArray, (dataLen * 4));
	if(result > 0){
		printf("Transmitted %ld bytes successfully\n", dataLen *4);
	}else{
		printf("Write failed with result %ld\n", result);
		errors++;
	}
	printf("--------------------------------\n");
	printf("\n");
	printf("\n");
	UOS_Delay(3000);

	result = M_getblock( rxPath, (u_int8*)rxDataArray, (dataLen * 4));

	/* program lower five ports as outputs, others as inputs */
	if(result > 0){
		printf("############# Receive ###############\n");
		printf("Receiving = %ld bytes data\n", result);
		for(i=0;i<(int)(result/4);i++){
			printf(" 0x%lx", rxDataArray[i]);
			if(txDataArray[i] != ((rxDataArray[i] >> 8) & 0x7FFFFF)){
				printf("\nExpected txDataArray[%d] = 0x%lx but received rxDataArray[%d] = 0x%lx\n",i, txDataArray[i], i, ((rxDataArray[i] >> 8) & 0x7FFFFF));
				errors++;
			}
		}
		printf("\n");
		printf("\n");
		printf("Received %ld bytes successfully\n", result);
		printf("--------------------------------\n");
	}else{
		printf("Read failed with the result = %ld\n", result);
		errors++;
	}

	printf("\n");
	printf("\n");
	printf("############# TESTING INTERRUPT (Disable) #############\n");
	printf("############# Transmit ###############\n");
	/* Disable interrupt */
	result = M_setstat(rxPath, 0, 0);
	if(result != 0){
		printf("RxSetStat failed with result %ld\n", result);
		errors++;
	}
	dataLen   = 100;
	for(i=0;i<(int)dataLen;i++){
		txDataArray[i] = i+ 7;
		printf(" 0x%lx", txDataArray[i]);
	}
	printf("\n");
	printf("\n");
	result = M_setblock(txPath, (u_int8*)txDataArray, (dataLen * 4));
	if(result > 0){
		printf("Transmitted %ld bytes successfully\n", dataLen *4);
	}else{
		printf("Write failed with result %ld\n", result);
	}
	printf("--------------------------------\n");
	printf("\n");
	printf("\n");
	UOS_Delay(2000);

	result = M_getblock( rxPath, (u_int8*)rxDataArray, (dataLen * 4));

	/* program lower five ports as outputs, others as inputs */
	if(result > 0){
		printf("############# Receive ###############\n");
		printf("Receiving = %ld bytes data\n", result);
		for(i=0;i<(int)(result/4);i++){
			printf(" 0x%lx", rxDataArray[i]);
			if(txDataArray[i] != ((rxDataArray[i] >> 8) & 0x7FFFFF)){
				printf("\nExpected txDataArray[%d] = 0x%lx but received rxDataArray[%d] = 0x%lx\n",i, txDataArray[i], i, ((rxDataArray[i] >> 8) & 0x7FFFFF));
				errors++;
			}
		}
		printf("\n");
		printf("\n");
		printf("Received %ld bytes successfully\n", result);
		printf("--------------------------------\n");
	}else{
		printf("Read failed with the result = %ld\n", result);
		errors++;
	}


	printf("-------------------------------------------\n");
	printf("-------------------------------------------\n\n");
	printf("Test Result : ");
	if(errors != 0){
		printf("FAILED\n");
	}else{
		printf("PASSED\n");
	}
	printf("\n-------------------------------------------\n");
	printf("-------------------------------------------\n");

	if (M_close(rxPath) < 0){
		PrintError("close");
	}
	if (M_close(txPath) < 0){
		PrintError("close");
	}

	return(0);
}

/********************************* PrintError ******************************/
/** Print MDIS error message
 *
 *  \param info       \IN  info string
 */
static void PrintError(char *info)
{
	printf("*** can't %s: %s\n", info, M_errstring(UOS_ErrnoGet()));
}

/**********************************************************************/
/** Signal handler
 *
 *  \param  sig    \IN   received signal
 */
static void __MAPILIB SignalHandler( u_int32 sig )
{
	if( sig == UOS_SIG_USR1 ) {
		++G_sigCount;
	}
}

/**********************************************************************/
/** Convert an integer into its binary string representation
 *
 *  e.g. 0x4f is converted to "1 0 0 1 1 1 1 "
 *
 *  \param  s      \IN   pointer to where result is written
 *  \param  val    \IN   value to convert
 *  \param  nrBits \IN   number of bits to convert (max 32)
 *
 *  \return pointer to resulting string
 */

static char* bitString( char *s, u_int32 val, int nrBits )
{
	u_int32 bit;
	int i;

	*s = '\0';

	bit = 1 << (nrBits-1);

	for( i=0; i<nrBits; ++i, bit>>=1 ) {
		strcat( s, val & bit ? "1 " : "0 " );
	}

	return( s );
}
