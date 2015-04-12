# Ubuntu #

## udev setup ##
To use this device from user space without being root we have to set up a udev rule so that when it loads we have read and write permissions.

create the file `/etc/udev/rules.d/51-asus.rules`

put the following in the file:
`SUBSYSTEM=="usb", SYSFS{idVendor}=="1043", MODE="0666"`

now give the file the correct permissions for udev:
`sudo chmod a+r /etc/udev/rules.d/51-asus.rules`

Now download the source.

## Actual build ##

First option  is automake. The configure script requires automake to be installed. The automake scripts in the source directory are actual soft links to automake-1.11. Other versions may work but the soft links will need to be updated.

Build requirements are documented on the front page but to build without Ajunta:


```
sudo apt-get install automake  build-essential libusb-dev libgd2-xpm-dev
```

NOTE libusb is version 0.1 (not the newer 1.0).



The other option is the Ajunta IDE.

In ubuntu, you will need libusb and libgd2 to build it, the easiest way is to install anjuta, libusb-dev, and build-essential, i built the project in the anjuta ide, so you might want to also build it using this, otherwise you will also need to install automake.

```
sudo apt-get install anjuta build-essential libusb-dev libgd2-xpm-dev
```


# Running #

If using pre-built binaries, need libusb (usually already installed) and libgd:

```
sudo apt-get install libgd2-xpm
```
Or
```
sudo apt-get install libgd2-noxpm
```

## Running the program ##
Generate an uncompressed RGB 24bit png image with no compression in GIMP, put your image in the same directory, then run:

```
asusdisplay imagename.png
```

if all goes well you should see the image displayed on the external display

to daemonize it run

```
asusdisplay background.png -d
```


## TinyCore / MicroCore Linux ##

asusdisplay can work with [Tiny Core](http://tinycorelinux.com) however as of July 2011 the libusb provided with TC 3.7.1 doesn't work with dava33display - see http://forum.tinycorelinux.net/index.php?topic=10818.0 for more details and the work around (don't install libusb.tcz and copy the shared library from Ubuntu).

Information about how the C version was created is at http://kieranlevin.com/index.php?option=com_content&view=article&id=54:reverseengdisplay