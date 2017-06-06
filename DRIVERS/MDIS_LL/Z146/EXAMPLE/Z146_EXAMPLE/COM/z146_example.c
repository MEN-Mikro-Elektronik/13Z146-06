/****************************************************************************
 ************                                                    ************
 ************                   Z146_EXAMPLE                     ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file z146_example.c
 *       \author Apatil
 *        $Date: 2015/10/16 18:08:18 $
 *    $Revision: 1.1 $
 *
 *       \brief  Simple example program for the Z146 driver
 *
 *     Required: libraries: mdis_api, usr_oss
 *     \switches (none)
 */
/*-------------------------------[ History ]--------------------------------
 *
 * $Log: z146_example.c,v $
 * Revision 1.1  2015/10/16 18:08:18  ts
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2003 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

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
|   PROTOTYPES                          |
+--------------------------------------*/
static void PrintError(char *info);

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
	u_int32 i = 0;
	u_int32 label = 0;
	u_int32 dataLen = 0;
	u_int32 txDataArray[MAX_DATA_LEN];
	u_int32 rxDataArray[MAX_DATA_LEN];

	if (argc < 4 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: z146_example <rxDevice> <txDevice> <dataLength> [<label>]\n");
		printf("Function: Example program for using the Z146 driver\n");
		printf("Options:\n");
		printf("    rxDevice       rx device name\n");
		printf("    txDevice       tx device name\n");
		printf("    dataLength     data length\n");
		printf("    label          label\n");
		printf("\n");
		return(1);
	}

	rxDevice = argv[1];
	txDevice = argv[2];
	/*--------------------+
    |  open rxPath        |
    +--------------------*/
	if ((rxPath = M_open(rxDevice)) < 0) {
		PrintError("rx open");
		return(1);
	}
	if ((txPath = M_open(txDevice)) < 0) {
		PrintError("tx open");
		return(1);
	}

	/*--------------------+
    |  config             |
    +--------------------*/
	if(argc == 5){
		label   = atoi(argv[4]);
		M_setstat(txPath, Z246_TX_LABEL, label);
		M_setstat(rxPath, Z146_RX_SET_LABEL, label);
	}

	printf("############# Transmit ###############\n");
	dataLen   = atoi(argv[3]);
	printf("User data length = %ld and dataptr = 0x%lx\n", dataLen *4, txDataArray);
	for(i=0;i<dataLen;i++){
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
	UOS_Delay(3000);

	result = M_getblock( rxPath, (u_int8*)rxDataArray, (dataLen * 4));

	/* program lower five ports as outputs, others as inputs */
	if(result > 0){
		printf("############# Receive ###############\n");
		printf("Receiving = %ld bytes data\n", result);
		for(i=0;i<(u_int32)(result/4);i++){
			printf(" 0x%lx", rxDataArray[i]);
			if(txDataArray[i] != ((rxDataArray[i] >> 8) & 0x7FFFFF)){
				printf("\nExpected txDataArray[%d] = 0x%lx but received rxDataArray[%d] = 0x%lx\n",
					i, txDataArray[i], i, ((rxDataArray[i] >> 8) & 0x7FFFFF));
			}
		}
		printf("\n");
		printf("\n");
		printf("Received %ld bytes successfully\n", result);
		printf("--------------------------------\n");
	}else{
		printf("Read failed with the result = %ld\n", result);
	}

	if (M_close(rxPath) < 0){
		PrintError("rx close");
	}
	if (M_close(txPath) < 0){
		PrintError("tx close");
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
