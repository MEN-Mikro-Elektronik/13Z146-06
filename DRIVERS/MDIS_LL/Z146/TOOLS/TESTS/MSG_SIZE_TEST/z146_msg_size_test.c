/****************************************************************************
 ************                                                    ************
 ************                   z146_msg_size_test                  ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file z146_msg_size_test.c
 *       \author Apatil
 *        $Date: 2009/07/10 13:40:12 $
 *    $Revision: 1.5 $
 *
 *       \brief  Test tool for message size test
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

static const char RCSid[]="$Id: z146_msg_size_test.c,v 1.5 2009/07/10 13:40:12 CRuff Exp $";

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
#define MAX_DATA_LEN 	4096

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
	u_int32 errors = 0;
	int i = 0;
	int j = 0;
	int k = 0;
	int timeout = 0;
	u_int32 label = 0;
	u_int32 dataLen = 0;
	u_int32 msgCnt = 50;
	u_int32 txDataArray[MAX_DATA_LEN];
	u_int32 rxDataArray[MAX_DATA_LEN];
	printf("argc = %ld\n",argc );
	if (argc < 2 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: z146_example <rx_rxDevice> <txDevice> \n");
		printf("Function: Test label configurations.\n");
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
	label   = 10;
	M_setstat(txPath, Z246_TX_LABEL, label);
	M_setstat(rxPath, Z146_RX_SET_LABEL, label);

	dataLen   = MAX_DATA_LEN;
	for(i=0;i<dataLen;i++){
		txDataArray[i] = i+ 7;
	}
	printf("\n");
	printf("\n");

	for(j=0;j<13;j++){
		dataLen = (1<<j);
		printf("############# TESTING Message Size %ld(word) #############\n\n", dataLen);

		printf("############# Transmitting %d Messages of Size %ld(word) #############\n\n",msgCnt, dataLen);
		for(k=0;k<msgCnt;k++){
			printf("----------- Transmit-%d-----------\n", k+1);
			result = M_setblock(txPath, (u_int8*)txDataArray, (dataLen * 4));
			if(result > 0){
				printf("Transmitted %ld bytes successfully\n", dataLen *4);
			}else{
				printf("Write failed with result %ld\n", result);
				errors++;
			}
			printf("--------------------------------\n");
			printf("\n");
			UOS_Delay((j* 200) + 500);

			result = M_getblock( rxPath, (u_int8*)rxDataArray, (dataLen * 4));

			/* program lower five ports as outputs, others as inputs */
			if(result > 0){
				printf("----------- Receive-%d -----------\n", k+1);
				printf("Receiving = %ld bytes data\n", result);
				for(i=0;i<result/4;i++){
					//					printf(" 0x%lx", rxDataArray[i]);
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
//			timeout = 500;
//			while(1){
//				result = M_getblock( rxPath, (u_int8*)rxDataArray, (dataLen * 4));
//
//				/* program lower five ports as outputs, others as inputs */
//				if(result > 0){
//					printf("----------- Receive-%d -----------\n", k+1);
//					for(i=0;i<result/4;i++){
//						//					printf(" 0x%lx", rxDataArray[i]);
//						if(txDataArray[i] != ((rxDataArray[i] >> 8) & 0x7FFFFF)){
//							printf("\nExpected txDataArray[%d] = 0x%lx but received rxDataArray[%d] = 0x%lx\n",i, txDataArray[i], i, ((rxDataArray[i] >> 8) & 0x7FFFFF));
//							errors++;
//						}
//					}
//					printf("Received %ld bytes successfully\n", result);
//					printf("--------------------------------\n");
//					printf("\n");
//					break;
//				}else{
//					UOS_Delay(100);
//
//					if(timeout != 0){
//						timeout--;
//					}else{
//						break;
//						printf("Read failed with the result = %ld\n", result);
//						errors++;
//					}
//				}
//			}
		}
		printf("\n");
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
 
 
 
