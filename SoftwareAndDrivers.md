

# Operating Systems #

## Linux ##

Booting Ubuntu 11.04 derivatives requires the NOMODESET boot option, when Ubuntu starts booting hit F1 to bring up the boot options. Without the NOMODESET option a black screen is displayed.

Out of box Ubuntu/MythTV ([Mythbuntu](http://www.mythbuntu.org/) 11.04) works fine for everything except the tuner cards. Under Linux the tuner cards do NOT support NTSC (available in the US over cable) in MythTV, getting digital channels to work requires work. Oddly tvtime can show NTSC channels (without audio).

Linux driver support:
  * http://www.linuxtv.org/wiki/index.php/ATSC_PCIe_Cards
  * http://www.linuxtv.org/wiki/index.php/Hauppauge_WinTV-HVR-1800
  * http://www.linuxtv.org/wiki/index.php/Talk:Hauppauge_WinTV-HVR-1800

Sound over HDMI requires enabling digital channel in the Mixer.

Some distros may have a minor mismatch in sound settings; audio output only supports sample rate of 48khz - and pulse defaults to 44.1 so you have to force it using "default-sample-rate = 48000" in /etc/pulse/daemon.conf

### Linux Temperature sensors ###

Out of the box the ACPI temperature sensor **ALWAYS** returns 40 C. A number of BIOS have similar problems. However [lm-sensors](http://www.lm-sensors.org/) works. Notes/extracts below:

```
$ sudo sensors-detect 

......
Now follows a summary of the probes I have just done.
Just press ENTER to continue: 

Driver `it87':
  * ISA bus, address 0x290
    Chip `ITE IT8716F Super IO Sensors' (confidence: 9)

Driver `k8temp':
  * Chip `AMD K8 thermal sensors' (confidence: 9)


Do you want to generate /etc/sysconfig/lm_sensors? (YES/no): YES
Copy prog/init/lm_sensors.init to /etc/init.d/lm_sensors
for initialization at boot time.
You should now start the lm_sensors service to load the required
kernel modules.


tc@box:~$ sensors
k8temp-pci-00c3
Adapter: PCI adapter
temp1:       -49.0 C                                    

atk0110-acpi-0
Adapter: ACPI interface
Vcore Voltage:     +1.23 V  (min =  +1.45 V, max =  +1.75 V)
 +3.3 Voltage:     +3.26 V  (min =  +3.00 V, max =  +3.60 V)
 +5.0 Voltage:     +4.95 V  (min =  +4.50 V, max =  +5.50 V)
+12.0 Voltage:    +12.22 V  (min = +11.20 V, max = +13.20 V)
CPU FAN Speed:    1097 RPM  (min =    0 RPM)
CHASSIS FAN Speed:7031 RPM  (min =    0 RPM)
CPU Temperature:   +24.0 C  (high = +90.0 C, crit = +125.0 C)  
MB Temperature:    +37.0 C  (high = +30.0 C, crit = +90.0 C)  
```

NOTE the AMD CPU temperature is -49C! The real CPU is in the atk0110-acpi-0 section.


## Microsoft Windows ##

### Windows 7 ###

Tested with successfully with Windows 7 x86 (for example the Professional edition).

Ramesh has reported problems with  all editions (for example the Professional edition) of Windows 7 64 bit, it always gets stuck at the "Completing Installation" step.

Out of the box every thing works.
  * gfx over hdmi is fine
  * Sound (over hdmi) is fine have to select the correct output device in sound mixer (and set default in Control Panel), viz. "Digital Audio (S/PDIF) High Definition Audio Device" for sound over HDMI to work. Headphone socket shows as a separate device.
  * Remote control just works, an MCE remote launches MCE correctly.

Only 3 devices shows as unknown in the Control panel, Device Manager:

  * ASeries Device (USB\VID\_1043$PID\_82B2&REV\_0001&MI\_00) - TFT screen - iCreate Technologies Corp
  * Unknown device (ACPI\ATK0110) - See Vista notes on ACPI
  * Unknown device (USB\VID\_04E3&PID\_2009&REV\_0000) - D2 Audio device Unsure, may be to control amp? Volume control knob on unit does NOT work in Media center out of box.

Drivers are installed for the IR port and the Media Center Start button on the remote starts MCE. It looks like an IR keyboard/mouse would work (based on installed drivers list) but I do not have hardware to confirm that.

The HD Tuners show up.

The tuner cards do work with NTSC (available in the US over cable),
I've not been able to get clear QAM to work.
ATSC (over the air seems to work - using the "secondary" port)


#### Windows Temperature sensors ####

Like Linux Out of the box the ACPI temperature sensor **ALWAYS** returns 40 C (i.e. using WMI MSAcpi\_ThermalZoneTemperature, CurrentTemperature always returns 40C). A number of BIOS have similar problems.

[Open Hardware Monitor 0.3.2 Beta](http://openhardwaremonitor.org/) also on [Google Code](http://code.google.com/p/open-hardware-monitor/) (like lm-sensors) reports -49C for the CPU temperature but M2NPV2-DHS / ITE IT8716F reports CPU and Motherboard temperatures. There is also a third temperature (which does change), not sure what that is for.

```
Sensors

|
+- M2NPV2-DHS (/mainboard)
|  |
|  +- ITE IT8716F (/lpc/it8716f)
|  |  +- CPU VCore      :    1.232    1.232    1.232 (/lpc/it8716f/voltage/0)
|  |  +- Voltage #2     :    3.264    3.264    3.264 (/lpc/it8716f/voltage/1)
|  |  +- Voltage #4     :    2.944    2.928    2.944 (/lpc/it8716f/voltage/3)
|  |  +- Voltage #5     :    3.056    3.056    3.056 (/lpc/it8716f/voltage/4)
|  |  +- Voltage #8     :     2.96     2.96     2.96 (/lpc/it8716f/voltage/7)
|  |  +- VBat           :    3.152    3.152    3.152 (/lpc/it8716f/voltage/8)
|  |  +- CPU            :       23       22       25 (/lpc/it8716f/temperature/0)
|  |  +- Motherboard    :       25       25       25 (/lpc/it8716f/temperature/1)
|  |  +- Temperature #3 :       29       27       29 (/lpc/it8716f/temperature/2)
|  |  +- Fan #1         :   1110.2  1108.37  1112.03 (/lpc/it8716f/fan/0)
|
+- AMD Athlon 64 Processor 3800+ (/amdcpu/0)
|  +- Bus Speed      :  200.477  200.477  200.477 (/amdcpu/0/clock/0)
|  +- CPU Core       :  2405.72  2405.72  2405.72 (/amdcpu/0/clock/1)
|  +- Core #1        :      -49      -49      -49 (/amdcpu/0/temperature/0)
|  +- CPU Core       :  1.53783  1.53783      100 (/amdcpu/0/load/1)
```



### Windows Vista ###

Tested with Windows Vista with Service Pack 2 (x86) "Windows Vista Home Premium".

Most things work out of the box. The Windows "find driver now" works for the tuner cards (but it is just as easy to download from http://www.hauppauge.com/site/products/data_hvr1800.html). Out of the box sound only works through headphones.

The TFT display, IR port, and sound need the A33 drivers. Once installed the TFT displays a clock.

MCE drivers appear to link the IR port and the TFT display.

Sound can be through hdmi, headphones, amp.

Video (over hdmi) works out of the box (but with a pink tinge, i.e. white shows a pink, correct drivers correct this) but drivers can be downloaded from nvida.com (this may be required for audio over hdmi to work, the nvidia control panel has an option to disable audio).

Windows install check list:


  1. ACPI driver, Acpi\_1043100.zip
  1. A Series (ASUS) drivers (TFT, IR, sound), ASeries\_V1030\_V10507.zip
  1. Tuner Hauppauge WinTV drivers
  1. GPU/Gfx nVidia, 275.33-desktop-win7-winvista-32bit-english-whql / 275.33-desktop-win7-winvista-64bit-english-whql.exe
  1. MCE drivers, MCE\_EXT\_104.zip / hauppauge\_mcekit\_setup.exe

The soundmax drivers (SoundMAX\_Audio\_V610X6270\_Audio.zip) _may_ be ignored.