#***************************  M a k e f i l e  *******************************
#
#         Author: ap
#          $Date: 2015/07/22 14:30:02 $
#      $Revision: 1.1 $
#
#    Description: Makefile definitions for the Z146 example program
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#
#-----------------------------------------------------------------------------
#   (c) Copyright 2000 by MEN mikro elektronik GmbH, Nuernberg, Germany
#*****************************************************************************

MAK_NAME=z146_msg_size_test

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)	\
			$(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)	\

MAK_INCL=$(MEN_INC_DIR)/z146_drv.h	\
         $(MEN_INC_DIR)/men_typs.h	\
         $(MEN_INC_DIR)/mdis_api.h	\
         $(MEN_INC_DIR)/usr_oss.h	\

MAK_INP1=z146_msg_size_test$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
 
