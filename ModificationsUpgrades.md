

# Upgrades #

There are multiple reports of increasing memory and changing the CPU.

  * [installed 4Gb RAM + AMD Athlon 64 X2 Dual Core 5400+](http://groups.google.com/group/alt.comp.periphs.mainboard.asus/msg/e9c4c9bd4fec817e)
  * clach04 has installed an extra 2x 2Gb DDR2-800, the existing memory is DDR2-667. According to Memtest86+ 4.10, the 4.5Gb runs at 2699 Mb/s, if the 512Mb is removed the 4Gb of memory runs 3455 MB/s (about 1Mb faster).
  * kieran.levin has installed 2Gb and an AMD Athlon(tm) 64 X2 Dual Core Processor 5600+


## Memory / RAM ##

![http://dava33display.googlecode.com/svn/wiki/static/pics/hddmr9000_x800.jpg](http://dava33display.googlecode.com/svn/wiki/static/pics/hddmr9000_x800.jpg)

The 4 DDR2 memory slots are located underneath the DVD drive (bottom center of image above ). It may be possible to slide the DIMMS in from the side. I ended up unscrewing the 4 hard drive cage (bottom right) screws, removing the cage (unplugging power and data cable first) and then sliding in the DIMMS from the gap this creates.

Update on RAM, I had one of my new (i.e. didn't come with the unit) DIMMs go bad, I was able to remove and replace them by angling them in form the left (if facing from front) side, where the side fan and tuner is located. Rotate the DIMM so it is flat (if pushed against the motherboard) slide in to the right, then rotate into the correct plane down into the DIMM slots.


## PSU / Power Supply ##

The PSU is a [microATX (µATX, mATX, or uATX)](http://en.wikipedia.org/wiki/MicroATX) size which is smaller than a regular ATX supply. The original PSU is rated for 320W.

clach04's power supply fan often doesn't rotate on power up (blowing on the fan causes it to start). It probably needs to be replaced at some point.

kieran.levin has replaced his PSU with an Diablotek PHD Series PHD380M 380W Micro ATX Power Supply. The fan noise is higher than the original one - but the one that came with it was underpowered and probably ran too hot causing its premature demise. It is a very nice supply.

Wilde-Man has replaced his PSU.  Here are the details:
I have replaced the power supply 3 times on an almost yearly basis. I've now past the 4th year. The first time I replaced it with the same PS. The second time I replaced it with a SilenX 400 Watt PS, then turned off my AC while on vacation and it burned up. Finally, I replaced it with a FSP300-GHS from http://UpgradeNation.com. I believe this to be the upgraded replacement for the original PS. I also have a second DAV-A33 whose PS went out in the one year time frame. It was also repaired with the afore mentioned FSP300-GHS, and is currently running for over a year.

# Modifications #

  * Cooling enhancements TODO!
    * The CPU/CPU-Fan seem to run OK (40 degrees C). The tuner cards (top right of image above) seem to run very hot. The PSU (top right) seems to be OK. The hard drive (bottom right) seems to run fairly hot
  * harddrive
    * easy to replace, if using as a DVR, need a fast one that can run 24x7. Seagate DB35 drives are often used for this purpose
    * In theory a 2nd drive could be added to the hard drive cage, BUT this is likely to cause over heating (there is no cooling in that part of the case) and there are no spare power plugs on the factory PSU


# Peripherals #

Regular keyboard/mouse can be plugged into USB ports without the need for special drivers. Logitech Unifying Receiver mice and keyboards work great (over long range). Can be paired under Windows or Linux https://bitbucket.org/clach04/logitech-unifying-receiver-tools/wiki/Home

## Remote controls ##

The A33 appears to ship with a number of IR remote devices:
  * Remote for AMP
  * MCE remote
  * keyboard and trackpad

need documentation and model numbers (and suitable replacements/alternatives).

Under Linux LIRC works fine with the remote receiver, so any remote should be usable under Linux (e.g. an old TV remote). But choosing a Windows Media Center MCE remote is probably the easiest option.

Under [Windows](SoftwareAndDrivers#Microsoft_Windows.md) the ASUS A Series drivers need to be installed. If you do not have an MCE remote but have a universal remote configure the remote to emulate a Philips MCP-9350I/03. The Philips remote is like the RC1974506/00 depicted at http://www.mythtv.org/wiki/MCE_Remote. Also see this thread http://www.avsforum.com/avs-vb/showthread.php?t=1314366 where more advanced remote options are mentioned (Microsoft  MCE-1039 and for Logitech Harmony users see [Microsoft Windows Media Center SE](http://forums.logitech.com/t5/Harmony-Remotes-Programming/Expanded-control-for-Windows-Media-Center-now-available-in-our/td-p/374844?nobounce)).

**NOTE** The way the Windows software is setup, the "Windows Media" green start button must be pressed first (to launch MCE),then things like the volume control buttons work. Volume buttons do NOT work unless MCE is loaded.

# GFX card #

  * [installed a gfx card with a separate hdmi connector, this required minor hacking of the card face plate](http://groups.google.com/group/alt.comp.periphs.mainboard.asus/msg/e9c4c9bd4fec817e)