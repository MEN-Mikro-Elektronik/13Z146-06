/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  z17_doc.c
 *
 *      \author  ulrich.bogensperger@men.de
 *        $Date: 2009/07/10 13:37:24 $
 *    $Revision: 1.3 $
 *
 *      \brief   User documentation for MDIS5 Z17 driver
 *
 *     Required: -
 *
 *     \switches -
 */
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: z17_doc.c,v $
 * Revision 1.3  2009/07/10 13:37:24  CRuff
 * R: Porting to MDIS5
 * M: Changed description to MDIS5; added compatibility hint
 *
 * Revision 1.2  2006/08/02 08:31:50  DPfeuffer
 * z17_io tool added, cosmetics
 *
 * Revision 1.1  2004/06/18 14:29:51  ub
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2004 by MEN Mikro Elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

/*! \mainpage
    This is the documentation of the MDIS low-level driver for the 
    16Z034 GPIO controller.

    16Z034 is a GPIO controller implemented in the Chameleon FPGA. It supports
    the following features:

    - 8 bit general purpose I/O
    - data direction programmable for every I/O port
    - state of output port pins readable
    - ability to generate interrupts on rising, falling or both edges on port
    - every port can work in open drain mode (Arwen_2 only)
    - hardware debouncing possible for every port (Arwen_2 only)

    This driver supports the 16Z017 (Arwen_2 and Arwen_3) GPIO controller. It uses call 
    locking.
    
    Note: MDIS5 32bit drivers are compatible to the MDIS4 drivers, but must not
          be mixed with MDIS4 drivers at one target system. 

    \n
    \section Variants Variants
    (no variants)

    \n \section FuncDesc Functional Description

    \n \subsection General General

    The most functions return/take an 8 bit value. If not described
    differently every bit in the set/read values correspond to a single IO
    port pin. Bit 0 (LSB) to port 0, bit 7 (MSB) to port 7.


    \n \subsection readwrite Reading and writing
    The M_read() function call reads the current state of all ports (input
    and output) at once. M_write() forces all ports which are programmed as
    outputs to the given level.  
    
    - Bit=0: low level on port pin
    - Bit=1: high level on port pin
    
    Additionally ports can be set/ cleared using SetStat Z16_SET_PORTS and
    Z16_CLEAR_PORTS. These functions affect only the ports whose corresponding
    bits are set in the given value. 
    
    Data direction can be programmed using SetStat Z17_DIRECTION.
    - Bit=0: port is input
    - Bit=1: port is output


    \n \subsection interrupts Interrupt handling and signals
    
    Using SetStat Z17_IRQ_SENSE it is possible to determine if an interrupt is
    generated on rising edge, falling edge, both edges or not at all. If an
    interrupt happens the driver will send the signal which was assigned using
    SetStat Z17_SET_SIGNAL to the application. Uninstall signal using 
    Z17_CLR_SIGNAL.

    The current status can be read back using GetStat Z17_IRQ_SENSE.

    Z17_IRQ_SENSE takes a 16 bit value. Two bits correspond to each port: Bit1/
    0 to port 0, bit 3/2 to port 1,..., bit15/14 to port7.

    Bit encoding:
    - 00	No interrupt generated for port
    - 01	Interrupt generated at rising edge on port
    - 10	Interrupt generated at falling edge on port
    - 11	Interrupt generated at rising and falling edge on port


	\n \subsection opendrain Open Drain feature
	
    The open drain feature of Arwen_2 can be enabled using SetStat
    Z17_OPEN_DRAIN. This SetStat code is applicable for Arwen_3 too, but no
    action is performed. If the corresponding bit is set, the output pin is
    only driven if its value is set to low (similar to an open collector/open
    drain driver). By default, all ports are driven to both high and low, if
    they are programmed as output. The current status can be read back using
    GetStat Z17_OPEN_DRAIN.

    - Bit=0: port in normal mode
    - Bit=1: port open drain


    \n \subsection debounce Debouncing
    
    Debouncing is enabled for Arwen_2 using SetStat Z17_DEBOUNCE. On Arwen_3
    no action is performed. This enables the hardware debouncer for all ports
    whose corresponding bit is set. It disables the debouncer for all ports
    whose corresponding bit is cleared. The current status can be read back
    using GetStat Z17_DEBOUNCE
    
    - Bit=0: debouncing off
    - Bit=1: debouncing on

    
    \n \subsection default Default values
    M_open() and M_close() set all ports to default values: 
    
    - Ports programmed as inputs 
    - Interrupts disabled 
    - Open drain and debouncing disabled


    \n \subsection sa15  SA15 issues
    SA15 is an IO adapter implementing digital input/outputs.
    It works together with a special version of the 16Z034
    GPIO controller: Arwen_03.

    Ports supported:
    - OUT 0..4: 16Z034 port 0..4 when configured as output
    - IN  0..4: 16Z034 port 0..4, always readable
    - IO  0..3: 16Z034 port 5..7, data direction programmable

    The out ports on SA15 are driven by an open collector driver,
    push-pull operation is not possible. Setstats Z17_OPEN_DRAIN
    and Z17_DEBOUNCE do not work. Also it is not possible to read back
    the current state of OUT 0..4 since the corresponging input circuity
    is internally wired to IN 0..4.


    \n \section api_functions Supported API Functions

    <table border="0">
    <tr>
        <td><b>API function</b></td>
        <td><b>Functionality</b></td>
        <td><b>Corresponding low level function</b></td></tr>

    <tr><td>M_open()</td><td>Open device</td><td>Z17_Init()</td></tr>

    <tr><td>M_close()     </td><td>Close device             </td>
    <td>Z17_Exit())</td></tr>
    <tr><td>M_read()      </td><td>Read from device         </td>
    <td>Z17_Read()</td></tr>
    <tr><td>M_write()     </td><td>Write to device          </td>
    <td>Z17_Write()</td></tr>
    <tr><td>M_setstat()   </td><td>Set device parameter     </td>
    <td>Z17_SetStat()</td></tr>
    <tr><td>M_getstat()   </td><td>Get device parameter     </td>
    <td>Z17_GetStat()</td></tr>
    <tr><td>M_getblock()  </td><td>Block read from device   </td>
    <td>Z17_BlockRead()</td></tr>
    <tr><td>M_setblock()  </td><td>Block write from device  </td>
    <td>Z17_BlockWrite()</td></tr>
    <tr><td>M_errstringTs() </td><td>Generate error message </td>
    <td>-</td></tr>
    </table>

    \n \section descriptor_entries Descriptor Entries
    (no descriptor entries supported)    
    
    \n \section codes Z17 specific Getstat/Setstat codes
    see \ref getstat_setstat_codes "section about Getstat/Setstat codes"

    \n \section Documents Overview of all Documents

    \subsection z17_simp  Simple example for using the driver
    z17_simp.c (see example section)

    \subsection z17_io  Tool to access the 16Z034 I/Os
    z17_io.c (see example section)

    \subsection z17_min   Minimum descriptor
    z17_min.dsc
    demonstrates the minimum set of options necessary for using the drver.

    \subsection z17_max   Maximum descriptor
    z17_max.dsc
    shows all possible configuration options for this driver.
*/

/** \example z17_simp.c
Simple example for driver usage
*/

/** \example z17_io.c */

/*! \page dummy
  \menimages
*/

