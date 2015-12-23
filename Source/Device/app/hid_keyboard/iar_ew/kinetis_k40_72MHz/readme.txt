########################################################################
#
#                           USB_HIDKBD.eww
#
# $Revision: #1 $
#
########################################################################

DESCRIPTION
===========

  Freescale's general purpose USB stack with personal health care device class 
 (PHDC) support currently offers device and host functionality for a wide number 
 of Freescale microcontrollers. This stack is also provided with complete source 
 code, sample applications, a fully featured user guide and a reference manual 
 to accelerate the development process. The stack currently supports USB.org 
 standard USB classes such as human interface device (HID), mass storage 
 device (MSD), communications device class (CDC), audio and PHDC. The stack is 
 provided free of charge while using supported Freescale products. 
  USB PHDC enables USB connectivity among medical devices. The stack is also 
 compatible with the Freescale Medical Connectivity Library, which provides 
 IEEE® 11073 support and is available for the USB transport layer. The stack was 
 also designed to support the Continua Health Alliance (an organization which 
 defines standards for medical device connectivity) guidelines, so the software is 
 ready for certification. 
 For more information see <installation-root>\arm\examples\Freescale\USB\Documentation\
 
COMPATIBILITY
=============

  The example project is compatible with Freescale TWR-K40D72M board.
  By default, the project is configured to use the P&E OS-JTAG interface.
 
CONFIGURATION
=============
  The Workspace contains USB HID keyboard example with the following configuration:
  
  Flash Debug K40D7: The example is flashed to embedded flash memory.

GETTING STARTED
===============

  1) Start the IAR Embedded Workbench for ARM.

  2) Select File->Open->Workspace...
     Open the following workspace:

      <installation-root>\arm\examples\Freescale\USB\
      Source\Device\app\hid_keyboard\iar_ew\kinetis_k40d7\USB_HIDKBD.eww
	  
  3) Connect the P&E OS-JTAG interface
  
  4) Power on the board.
  
  5) Press Ctrl+D or use Download and Debug button to start a debug session

