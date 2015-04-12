# Display Driver for Asus's DAV Center A33 Media Center PC Plus Amp and MOXI HOME CINEMA HD DMR model HDDMR9000 #

User space device driver for the information display built into the Asus DAV center A33. Internal TFT display.

![http://dava33display.googlecode.com/svn/wiki/static/pics/pyv1_display_white.png](http://dava33display.googlecode.com/svn/wiki/static/pics/pyv1_display_white.png)
![http://dava33display.googlecode.com/svn/wiki/static/pics/pyv2_display_white.png](http://dava33display.googlecode.com/svn/wiki/static/pics/pyv2_display_white.png)

The Asus DAV Center A33 comes with a small built in front panel LCD display which is used in Microsoft Windows to show volume and other information (for example Media Center Edition play/status information). This is a small user space program to use the display under Linux or Windows.

Identification:

Run lsusb and look for the following device:
```
ID 1043:82b2 iCreate Technologies Corp.
```

Under Windows it can show up as a "ASeries Device".

There are currently 2 tools for the display:

  1. Pure Python version, any type of image, any size - works under Linux and Windows, both 32 and 64 bit (yes even Windows 7). Supports temperature display.
  1. C version, requires PNG in raw format - Linux only, only works with libusb-0.1


## Python version ##

This is the current version of the display tool and is cross platform.

See PythonVersion


## C version ##

This is the older, less capable version of the display tool and is Linux only.

See BuildNotes


## More Information ##

There are also resources related to drivers and software as there are no manuals for the MOXI unit.

There is more information in the wiki, for example [Hardware](Hardware.md), ModificationsUpgrades, SoftwareAndDrivers.
