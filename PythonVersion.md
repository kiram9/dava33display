

![http://dava33display.googlecode.com/svn/wiki/static/pics/pyv2_display_white.png](http://dava33display.googlecode.com/svn/wiki/static/pics/pyv2_display_white.png)

# Introduction #

The (Pure) Python version is the main release DAV A33 Display and relies on:
  * [PyUSB](http://sourceforge.net/projects/pyusb/) v1.0 (NOTE version 0.x does NOT work)- tested with svn [rev90](http://pyusb.svn.sourceforge.net/viewvc/pyusb?view=revision&revision=90). See the [PyUSB wiki](http://sourceforge.net/apps/trac/pyusb/) more information
    * [PyUSB](http://sourceforge.net/projects/pyusb/) requires [libusb](http://www.libusb.org/)
  * [PIL](http://www.pythonware.com/products/pil/) - Ubuntu includes this as part of the regular installation
    * For Windows the [official PIL binaries](http://pypi.python.org/pypi/PIL), http://www.lfd.uci.edu/~gohlke/pythonlibs/#pil, or [Pillow](http://pypi.python.org/pypi/Pillow) may be used
  * [Open Hardware Monitor 0.3.2 Beta](http://openhardwaremonitor.org/) - Windows only used for temperature information (see Windows section below for more information)

It has been developed against PyUSB 1.0, so older versions of PyUSB (for example the Ubuntu 10.10 package python-usb is too old as it is part of the 0.x series). Because of the new PyUSB library, support for different USB libraries is available.

It has been tested with Python 2.5 and 2.6, but any 2.x version that is 2.4 or later should work (older releases could be supported if needed).

It works on both 32 and 64 bit Linux. It has been tested under Windows 7 (32 bit) with libusb-0.1.

# Linux Notes #

## Linux Install Notes ##

  1. Install python and dependencies, under Ubuntu everything should already be installed, for debugging the optional ImageTK may be installed:
```
sudo apt-get install python-imaging-tk
```

  1. check out code DAV A33 Display
> > `svn checkout http://dava33display.googlecode.com/svn/trunk/python dava33display-python  `
  1. checkout PyUSB from svn
> > ` svn checkout https://pyusb.svn.sourceforge.net/svnroot/pyusb/trunk pyusb`
  1. then install PyUSB, one option is to simply copy the usb directory into the dava33display directory. This option is usually a good idea if the Operating System already has a PyUSB module installed (as the Ubuntu/Debian one is too old).

Running:
```
cd dava33display-python
python asusdisplay some_image.png

# run for ever
python asusdisplay some_image.png--daemon_mode
```

Like the C version, some Linux distributions (like Ubuntu) require root access to access the USB port unless a udev rule is added.

Also note that for temperature sensors to be displayed, lm-sensors should be installed and configured. See SoftwareAndDrivers for information on sensors.


# Windows Notes #

[Windows libusb-0.1](http://www.libusb.org/wiki/windows_backend), aka [libusb-win32](http://sourceforge.net/apps/trac/libusb-win32/wiki) has been tested with Windows 7 (32 bit), other Windows platforms should work fine too. Download from https://sourceforge.net/projects/libusb-win32/files/libusb-win32-releases/ (version 1.2.4.0 has been tested, 1.2.5.0 should work better due to improved reset support). NOTE https://github.com/libusbx/libusbx/wiki/Windows-Backend mentions  Zadig, an Automated Driver Installer GUI application for WinUSB, libusb-win32 and libusbK.

[libusb under Windows](http://www.libusb.org/wiki/libusb-win32) is a slightly different beast compared with Linux. Under Linux the libusb libaray and just be opened and used to access USB devices on the system (with root/sudo permissions). Under Windows, the libusb libraries need to be installed as drivers for the device in question. inf-wizard shipped in the bin directory of libusb performs the task of listing the devices and then creating (and installing) the driver.

Under Windows the TFT display shows up as:

  * "ASeries Device (Interface 1)"
  * "ASeries Device (Interface 0)"

Install drivers for both before using asusdisplay. In theory the driver is available to use immediately after installation, however a reboot may be required (and is recommended) before attempting to use asusdisplay under Windows.

[libusbK aka usb-travis](http://code.google.com/p/usb-travis/) may work but has not been tested.

### Windows Temperature ###

Display of temperatures under Windows 7 is option and depends on:
  * [Open Hardware Monitor 0.3.2 Beta](http://openhardwaremonitor.org/) also available at [Google Code](http://code.google.com/p/open-hardware-monitor/)
    * it also requires the [Python WMI interface](http://timgolden.me.uk/python/wmi/)
      * which in turn requires [pywin32](http://sourceforge.net/projects/pywin32/files/).

OHM must be running when asusdisplay is running, otherwise temp is not displayed. OHM can be started after asusdisplay. OHM requires admin permissions (if UAC is on, user is prompted for permission). One way to avoid the UAC prompt, without disabling UAC, is to use a scheduled task as a workaround. See http://social.technet.microsoft.com/Forums/en/w7itprosecurity/thread/6822b712-8fc3-4253-8856-25e99f144262, http://www.howtogeek.com/howto/windows-vista/create-administrator-mode-shortcuts-without-uac-prompts-in-windows-vista/

TODO consider looking at http://remotehrdwaremonitor.codeplex.com/


# Roadmap / Possible Futures #

  * Improve performance; http://psyco.sourceforge.net/ , [Shed Skin](http://shed-skin.blogspot.com/), Cython, [[nuitka](http://www.nuitka.net/blog/nuitka-a-python-compiler/). Right now most of the time is spent in the Python USB library
  * Other information support (think [Conky](http://conky.sourceforge.net/) / [gtk-desktop-info](https://launchpad.net/~m-buck/+archive/gtk-desktop-info) (gtk-desktop-info is written in Python but uses html output))
  * RFB Protocol support (think VNC)
    * http://en.wikipedia.org/wiki/RFB_protocol
    * http://tigervnc.sourceforge.net/cgi-bin/rfbproto
    * There are a few python libraries for this (client and server), server would be useful for debugging, client for displaying a remote RFB framebuffer
  * MythTV LCD support, mythlcdserver
  * Windows MCE (Media Center Edition) LCD support - "Recording" notices, Show titles, etc.
  * URL support (i.e. load images from a website)
  * Random image support (partially implemented)
  * Windows only, implement support for http://remotehrdwaremonitor.codeplex.com/
  * Record temperatures for later checking, e.g. plot temperature charts over time