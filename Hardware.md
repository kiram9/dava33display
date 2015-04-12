

# Introduction #

The display driver is for the USB TFT that comes as standard on the [ASUS DAV A33 HTPC](http://www.asus.com/Desktop/Home_Theater_PC/DAV_Center_A33/#overview). The display also comes with a not documented Moxi htpc, which comes in the same case as the A33.

# ASUS DAV A33 #

  * AMD Athlon 64 3800+ AM2 (2.4Ghz)
  * 512Mb ram
  * TFT display (usb)
  * 2x Hauppauge card (TV/composite/S-VHS inputs)
  * IR port (usb)  - for 2 remotes (one for AMP, another for MCE) and keyboard+trackpad
  * ATI gfx built in



# MOXI HOME CINEMA HD DMR model HDDMR9000 #

![http://dava33display.googlecode.com/svn/wiki/static/pics/hddmr9000_x800.jpg](http://dava33display.googlecode.com/svn/wiki/static/pics/hddmr9000_x800.jpg)

Seen on ebay, usually described as a "ASUS HTPC with HDMI". It looks like an A33 unless the front drop down panel and rear is examined closely. Most units ship without Operating Systems and remotes/keyboard/mice. See SoftwareAndDrivers for information on OS and drivers.

This unit is essentially a cut down A33; missing some ports (and WiFi) and with NVIDA/nForce chipset instead of ATI. Because of the differences to an A33, only some of the A33 drivers can be used.

  * AMD Athlon 64 3800+
  * 512Mb ram (2x 256Mb DDR2 667)

## Mainboard/Chipset ##

  * ASUS M2NPV2-DHS motherboard with nVidia nForce 4 chipset
  * 4x DDR2 slots in total (2 are usually used)
  * NVIDIA GPU GeForce 6510LE built in HDMI (video and audio), also Component, S-Video, and composite
  * SoundMax / Analog Devices AD1988B HD Audio with D2 Audio 5.1 channel amplifier
  * RCA Audio
  * SPDIF Optical Audio IN
  * SPDIF Optical Audio OUT
  * 1/4" Jack head phone socket (in front drop down panel )
  * 3.5mm microphone socket (in front drop down panel ) (possible to use as line in?)
  * E-SATA
  * 3x USB 2.0 (2 in rear, 1 in front drop down panel)
  * IEEE1394/FireWire
  * Gigabit Ethernet (10/100/1000 network port)


## Other ##

  * 320GB 7200RPM SATA (Seagate 35.3) hard disk
  * Slim slot DVD drive
  * TFT display (usb) - what http://code.google.com/p/dava33display/ is all about. Under Windows with drivers this has the time and under Windows Media Center may list the current video name
```
# lsusb under Linux shows:
ID 1043:82b2 iCreate Technologies Corp.
```
  * 2x Hauppauge 1800 PCI-Express dual tuner (HD+Analog) tuners with hardware MPEG encoder (TV ONLY inputs, i.e. no FM or RCA inputs) - NOTE some hardware ID software identifies these as Hauppauge HDPVR-1250 model 1196. One card is PCI-E 16x, the other is PCI-E 1x. NOTE there are 2 cable inputs, one marked **Primary**, the other **Secondary**. NOTE when using OTA (Over The Air) HD ATSC Antenna (for example, Radio Shack 15-1874 is an excellent un-powered indoor antenna), this **must** be in the **Secondary** port, attaching an antenna in the Primary does **not** work. Cable for NTSC works in the Primary.
  * IR port (usb)
```
# lsusb under Linux shows:
ID 1784:0006 TopSeed Technology Corp. eHome Infrared Transceiver
```
  * Rotary volume control (with push butto action) - Under Windows with drivers this controls Windows volume control
  * Front panel buttons: Menu, Left, Right, Up, Down, OK - Unsure how to use these. Menu button controls "effects" (Normal, Jazz, Rock, etc.)

## BIOS information ##

  * Phoenix - AwardBIOS v6.00PG
  * ASUS M2NPV2-DHS ACPI BIOS [Revision 0110](https://code.google.com/p/dava33display/source/detail?r=0110)
  * BIOS: 06/28/2007-C51PV-MCP51-M2NPV2DH-00
  * Keys:
    * DEL - setup
    * F8 - boot menu
    * Esc - skip memory test (probably only visible with non quick boot option set in bios)
  * GFX setting in BIOS appears to be set to PCI-E (rather than internal VGA)


The motherboard of the this unit is a M2NPV2-DHS, this appears to be very similar to the [M2NPV-MX](RelatedHardware.md) motherboard (minus a few pieces of hardware). The manual for the M2NPV-MX is useful for this machine even if the motherboard is not identical.

For information on upgrading the hardware see ModificationsUpgrades.


Some of the hardware was manafactured by iCreate Technologies Corporation. http://wikimapia.org/1787927/cn/iCreate-Technologies-Corporation-%E6%88%91%E6%83%B3%E7%A7%91%E6%8A%80%E8%82%A1%E4%BB%BD%E6%9C%89%E9%99%90%E5%85%AC%E5%8F%B8 they do not appear to be in business any more.