/****************************************************************************
 ************                                                    ************
 ************                   Z146_12KHz_test                   ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file z146_12KHz_test.c
 *       \author Apatil
 *        $Date: 2009/07/10 13:40:12 $
 *    $Revision: 1.5 $
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
 * $Log: z17_simp.c,v $
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2003 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

static const char RCSid[]="$Id: z146_12KHz_test.c,v 1.5 2009/07/10 13:40:12 CRuff Exp $";

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
	u_int32 label = 12;
	u_int32 dataLen = 0;
	u_int32 txSpeed = 0;
	u_int32 rxSpeed = 0;
	u_int32 txDataArray[4096];
	u_int32 rxDataArray[4096];
	printf("argc = %ld\n",argc );
	if (argc < 2 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: z146_12KHz_test <rx_rxDevice> <txDevice> >\n");
		printf("Function: Test program for 12KHz speed configuration test.\n");
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

	/*--------------------+
    |  config             |
    +--------------------*/
	/* install signal which will be received at change of input ports */
	UOS_SigInit( SignalHandler );
	UOS_SigInstall( UOS_SIG_USR1 );
//	M_setstat(rxPath, Z146_SET_SIGNAL, UOS_SIG_USR1);

	M_setstat(txPath, Z246_TX_LABEL, label);
	M_setstat(rxPath, Z146_RX_SET_LABEL, label);

	txSpeed = 0;
	M_setstat(txPath, Z246_TX_SPEED, txSpeed);
	if(result != 0){
		printf("TxSetStat failed with result %ld\n", result);
		errors++;
	}

	rxSpeed = 0;
	result = M_setstat(rxPath, Z146_RX_SPEED, rxSpeed);
	if(result != 0){
		printf("RxSetStat failed with result %ld\n", result);
		errors++;
	}

	result = M_getstat(txPath, Z246_TX_SPEED, &txSpeed);
	if(result != 0){
		printf("TxSetStat failed with result %ld\n", result);
		errors++;
	}
	if(txSpeed != 0){
		printf("TxSpeed is not 12KHz.\n");
		errors++;
	}

	result = M_getstat(rxPath, Z146_RX_SPEED, &rxSpeed);
	if(result != 0){
		printf("RxSetStat failed with result %ld\n", result);
		errors++;
	}
	if(rxSpeed != 0){
		printf("RxSpeed is not 12KHz.\n");
		errors++;
	}
	printf("\n");
	printf("\n");
	printf("############# TESTING 12KHz SPEED CFG #############\n");
	printf("----------- Transmit -----------\n");
	dataLen   = 100;
	printf("User data length = %ld and dataptr = 0x%lx\n", dataLen *4, txDataArray);
	for(i=0;i<dataLen;i++){
		txDataArray[i] = i+ 7;
		printf(" 0x%lx", txDataArray[i]);
	}
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
	UOS_Delay(2000);

	result = M_getblock( rxPath, (u_int8*)rxDataArray, (dataLen * 4));

	/* program lower five ports as outputs, others as inputs */
	if(result > 0){
		printf("----------- Receive -----------\n");
		printf("Receiving = %ld bytes data\n", result);
		for(i=0;i<result/4;i++){
			printf(" 0x%lx", rxDataArray[i]);
			if(txDataArray[i] != ((rxDataArray[i] >> 8) & 0x7FFFFF)){
				printf("\nExpected txDataArray[%d] = 0x%lx but received rxDataArray[%d] = 0x%lx\n",i, txDataArray[i], i, ((rxDataArray[i] >> 8) & 0x7FFFFF));
				errors++;
			}
		}
		printf("\n");
		printf("Received %ld bytes successfully\n", result);
		printf("--------------------------------\n");
	}else{
		printf("Read failed with the result = %ld\n", result);
		errors++;
	}

	/* Test 100KHz speed configuration.  */

	txSpeed = 1;
	result = M_setstat(txPath, Z246_TX_SPEED, txSpeed);
	if(result != 0){
		printf("TxSetStat failed with result %ld\n", result);
		errors++;
	}

	rxSpeed = 1;
	result = M_setstat(rxPath, Z146_RX_SPEED, rxSpeed);
	if(result != 0){
		printf("RxSetStat failed with result %ld\n", result);
		errors++;
	}

	result = M_getstat(txPath, Z246_TX_SPEED, &txSpeed);
	if(result != 0){
		printf("TxSetStat failed with result %ld\n", result);
		errors++;
	}
	if(txSpeed != 1){
		printf("TxSpeed is not 100KHz, returned value = %ld.\n", txSpeed);
		errors++;
	}

	result = M_getstat(rxPath, Z146_RX_SPEED, &rxSpeed);
	if(result != 0){
		printf("RxSetStat failed with result %ld\n", result);
		errors++;
	}
	if(rxSpeed != 1){
		printf("RxSpeed is not 100KHz, returned value = %ld.\n", rxSpeed);
		errors++;
	}
	printf("\n");
	printf("\n");
	printf("############# TESTING 100KHz SPEED CFG #############\n");
	printf("----------- Transmit -----------\n");
	dataLen   = 100;
	printf("User data length = %ld and dataptr = 0x%lx\n", dataLen *4, txDataArray);
	for(i=0;i<dataLen;i++){
		txDataArray[i] = i+ 7;
		printf(" 0x%lx", txDataArray[i]);
	}
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
	UOS_Delay(2000);

	result = M_getblock( rxPath, (u_int8*)rxDataArray, (dataLen * 4));

	/* program lower five ports as outputs, others as inputs */
	if(result > 0){
		printf("----------- Receive -----------\n");
		printf("Receiving = %ld bytes data\n", result);
		for(i=0;i<result/4;i++){
			printf(" 0x%lx", rxDataArray[i]);
			if(txDataArray[i] != ((rxDataArray[i] >> 8) & 0x7FFFFF)){
				printf("\nExpected txDataArray[%d] = 0x%lx but received rxDataArray[%d] = 0x%lx\n",i, txDataArray[i], i, ((rxDataArray[i] >> 8) & 0x7FFFFF));
				errors++;
			}
		}
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
 
