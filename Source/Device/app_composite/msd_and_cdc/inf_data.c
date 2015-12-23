/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
* ALL RIGHTS RESERVED.
*
**************************************************************************//*!
*
* @file inf_data.c
*
* @author
*
* @version
*
* @date
*
* @brief The file contains a usb CDC device driver data
*****************************************************************************/
#include "types.h"

const unsigned char inf_data[] =
{"[Version]\r\n\
Signature=\"$Windows NT$\"\r\n\
Class=Ports\r\n\
ClassGuid={4D36E978-E325-11CE-BFC1-08002BE10318}\r\n\
Provider=%MFGNAME%\r\n\
LayoutFile=layout.inf\r\n\
CatalogFile=\%MFGFILENAME\%.cat\r\n\
DriverVer=02/16/2011,1.0\r\n\
\r\n\
[Manufacturer]\r\n\
%MFGNAME%=DeviceList, NTamd64\r\n\
\r\n\
[DestinationDirs]\r\n\
DefaultDestDir=12\r\n\
\r\n\
\r\n\
;------------------------------------------------------------------------------\r\n\
; Windows 2000/XP/Vista-32bit Sections\r\n\
;------------------------------------------------------------------------------\r\n\
\r\n\
[DriverInstall.nt]\r\n\
include=mdmcpq.inf\r\n\
CopyFiles=DriverCopyFiles.nt\r\n\
AddReg=DriverInstall.nt.AddReg\r\n\
\r\n\
[DriverCopyFiles.nt]\r\n\
usbser.sys,,,0x20\r\n\
\r\n\
[DriverInstall.nt.AddReg]\r\n\
HKR,,DevLoader,,*ntkern\r\n\
HKR,,NTMPDriver,,%DRIVERFILENAME%.sys\r\n\
HKR,,EnumPropPages32,,\"MsPorts.dll,SerialPortPropPageProvider\"\r\n\
\r\n\
[DriverInstall.nt.Services]\r\n\
AddService=usbser, 0x00000002, DriverService.nt\r\n\
\r\n\
[DriverService.nt]\r\n\
DisplayName=%SERVICE%\r\n\
ServiceType=1\r\n\
StartType=3\r\n\
ErrorControl=1\r\n\
ServiceBinary=%12%\\%DRIVERFILENAME%.sys\r\n\
\r\n\
;------------------------------------------------------------------------------\r\n\
; Vista-64bit Sections\r\n\
;------------------------------------------------------------------------------\r\n\
\r\n\
[DriverInstall.NTamd64]\r\n\
include=mdmcpq.inf\r\n\
CopyFiles=DriverCopyFiles.NTamd64\r\n\
AddReg=DriverInstall.NTamd64.AddReg\r\n\
\r\n\
[DriverCopyFiles.NTamd64]\r\n\
%DRIVERFILENAME%.sys,,,0x20\r\n\
\r\n\
[DriverInstall.NTamd64.AddReg]\r\n\
HKR,,DevLoader,,*ntkern\r\n\
HKR,,NTMPDriver,,%DRIVERFILENAME%.sys\r\n\
HKR,,EnumPropPages32,,\"MsPorts.dll,SerialPortPropPageProvider\"\r\n\
\r\n\
[DriverInstall.NTamd64.Services]\r\n\
AddService=usbser, 0x00000002, DriverService.NTamd64\r\n\
\r\n\
[DriverService.NTamd64]\r\n\
DisplayName=%SERVICE%\r\n\
ServiceType=1\r\n\
StartType=3\r\n\
ErrorControl=1\r\n\
ServiceBinary=%12%\\%DRIVERFILENAME%.sys\r\n\
\r\n\
\r\n\
;------------------------------------------------------------------------------\r\n\
; Vendor and Product ID Definitions\r\n\
;------------------------------------------------------------------------------\r\n\
; When developing your USB device, the VID and PID used in the PC side\r\n\
; application program and the firmware on the microcontroller must match.\r\n\
; Modify the below line to use your VID and PID. Use the format as shown below.\r\n\
; Note: One INF file can be used for multiple devices with different VID and PIDs.\r\n\
; For each supported device, append \",USB\\VID_xxxx&PID_yyyy\" to the end of the line.\r\n\
;------------------------------------------------------------------------------\r\n\
[SourceDisksFiles]\r\n\
[SourceDisksNames]\r\n\
[DeviceList]\r\n\
%DESCRIPTION%=DriverInstall, USB\\VID_15A2&PID_0800&MI_00\r\n\
\r\n\
[DeviceList.NTamd64]\r\n\
%DESCRIPTION%=DriverInstall, USB\\VID_15A2&PID_0800\r\n\
\r\n\
\r\n\
;------------------------------------------------------------------------------\r\n\
; String Definitions\r\n\
;------------------------------------------------------------------------------\r\n\
;Modify these strings to customize your device\r\n\
;------------------------------------------------------------------------------\r\n\
[Strings]\r\n\
MFGFILENAME=\"CDC\"\r\n\
DRIVERFILENAME =\"usbser\"\r\n\
MFGNAME=\"Freescale\"\r\n\
INSTDISK=\"Freescale CDC Driver Installer\"\r\n\
DESCRIPTION=\"Virtual Com Port\"\r\n\
SERVICE=\"FSL Virtual COM Driver\"\r\n\
\r\n\
"
};

const uint_32 inf_data_size = sizeof(inf_data)/sizeof(inf_data[0]);
const uint_32 inf_size = sizeof(inf_data[0]);
const uint_32 data_size = sizeof(inf_data);
/* EOF */
