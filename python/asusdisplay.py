#!/usr/bin/env python
# -*- coding: ascii -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab
#
# asusdisplay.py - Loads an image on the USB TFT display of ASUS A33 DAV
# Copyright (C) 2011  Chris Clark
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
"""Python version of asusdisplay.c

Main differences compared with C version:
  * Requires Python 2.x, PIL, pyusb 1.x (does not work with pyusb 0.1,
    but does work with libusb-0.1 and libusb-1.0. pyusb revision 90
    from svn works fine)
  * Should work with any USB library on any platform
      * Tested with Windows:
        * 32 bit Windows 7 with:
              * libusb-0.1 / libusb-win32
      * Tested with Linux:
          * 32 bit Tiny Core with:
              * libusb-0.1
              * libusb-1.0
              * libusb-1.0 with libusb-0.1 compatibility layer
                (C version hangs/crashes using compatibility layer)
          * Ubuntu 10.10 32 bit:
              * libusb-0.1
              * libusb-1.0
          * Ubuntu 10.10 64 bit:
              * libusb-0.1
              * libusb-1.0
  * Can load ANY image format, with any size supported by PIL
      * NOTE small images end up with black bar(s)
      * this can load the testimg.bmp provided in SVN repo
  * Doesn't really understand the protocol (or packet contents) used,
    see C code and documents for more detail on the protocol
    unless using debug random mode)

NOTE it is important to allow asusdisplay to complete an IO cycle
(e.g. complete image load) before interupting, aborting part way through
can leave the display locked until the next reboot.
I've not yet looked at doing resets to recover from this situation.
"""

import os
import sys
import string
import re
import random
import urllib2
import math
import time
import logging
import threading

try:
    import cStringIO as StringIO
except ImportError:
    import StringIO

try:
    # Windows only
    import wmi  # from http://timgolden.me.uk/python/wmi/index.html
except ImportError:
    wmi = None

try:
    from PIL import Image  # http://www.pythonware.com/products/pil/
    from PIL import ImageFont, ImageDraw, ImageOps
except ImportError:
    try:
        import Image    # http://www.pythonware.com/products/pil/
        import ImageFont
        import ImageDraw
        import ImageOps
    except ImportError:
        raise  # Potential to remove dependency on PIL


DEBUG_USBIO = False  # If set, fake usb IO
DEBUG_DISPLAY = False  # If set, fake screen object
#DEBUG_DISPLAY = 'file'
#DEBUG_DISPLAY = 'tk'

DEBUG_TEMPS = False
#DEBUG_TEMPS = True

#log_format = '%(asctime)s %(levelname)s %(message)s'
log_format = '%(asctime)s %(filename)s:%(lineno)d %(levelname)s %(message)s'
logging.basicConfig(format=log_format)
log = logging.getLogger('asusdisplay')
log.setLevel(logging.NOTSET)  # only logs; WARNING, ERROR, CRITICAL
log.setLevel(logging.DEBUG)


if DEBUG_DISPLAY:
    log.debug('DEBUG_DISPLAY %r', DEBUG_DISPLAY)
    DEBUG_USBIO = True
    if DEBUG_DISPLAY == 'tk':
        import Tkinter
        try:
            from PIL import ImageTk  # http://www.pythonware.com/products/pil/
        except ImportError:
            # Try old style import
            import ImageTk


class AsusDisplayException(Exception):
    '''Base Asus Display exception'''


class SensorsNotFound(AsusDisplayException):
    '''No temperature sensors'''


if DEBUG_USBIO:
    """Horrible mock objects that do NOT emulate usb lib well at all...
    ....but it works well enough for this on machines without the correct
    hardware.
    """
    
    class FakeDev(object):
        iProduct = iManufacturer = idProduct = idVendor = 1
        
        def __init__(self):
            self.__count = 0
        
        def detach_kernel_driver(self, x):
            pass
        
        def set_configuration(self):
            pass
        
        def reset(self):
            pass
        
        def write(self, ep, block, interface, timeout):
            self.__count += 1
            log.debug('Fake WRITE %r', len(block))
            pass
        
        def read(self, ep, size, interface, timeout):
            self.__count += 1
            
            result = range(size)
            log.debug('Fake READ %r', len(result))
            return result
    
    class Fake(object):
        def find(self, idVendor, idProduct):
            return FakeDev()
    usb = Fake()
    usb.core = Fake()
else:
    # Import http://sourceforge.net/projects/pyusb/
    import usb.core
    import usb.util


MAX_IMAGE_SIZE = (320, 240)

#clock_font = ImageFont.load_default()
clock_font = None
for font_filename, font_size in [
                                #('orbitron-black.ttf', 60),  # not proportional
                                #('orbitron-bold.ttf', 60),
                                #('orbitron-medium.ttf', 60),
                                ('FreeSansBold.ttf', 72),
                                ('/usr/share/fonts/truetype/freefont/FreeSansBold.ttf', 72),
                                ('DejaVuSans-Bold.ttf', 65),  # related to FreeSansBold
                                ('/usr/share/fonts/luxisr.ttf', 72),  # this leaves some spacce on RHS
                                ]:
    try:
        clock_font = ImageFont.truetype(font_filename, font_size)
        log.debug('font %r %r', font_filename, font_size)
        temp_font = ImageFont.truetype(font_filename, font_size / 2)
        break
    except IOError:
        pass
# FIXME set clock color/colour, this really should be a parameter/config setting
# white matches what c version of asusdisplay
#clock_color = (255, 0, 0)
#clock_color = (255, 255, 255)
clock_color = 255  # NOTE TinyCore 3.7.1, Python 2.6.5 and pil-2.6 screws up image colors when tetx "fill" is specified as a tuple, this works around this problem


def simpleimage_clock(im, include_clock='include_secs'):
    """Apply timestamp to image.
    This is NOT (yet?) the same format as c version of asusdisplay
    NOTE modifies image in place
    include_clock if set to 'no_secs' does not include seconds in time display
    """
    if include_clock == 'no_secs':
        my_text = time.strftime('%H:%M')
    else:
        my_text = time.strftime('%H:%M:%S')
    d = ImageDraw.Draw(im)
    d.text((0, 0), my_text, font=clock_font, fill=clock_color)
    
    return im


def read_temp(filename):
    if isinstance(filename, int):
        # DEBUG
        return filename
    f = open(filename)
    temp = f.read()
    f.close()
    temp = temp.strip()
    temp_c = int(temp) / 1000  # my machine only does whole numbers
    return temp_c


class TemperatureSensors(object):
    """Handles temperature information
    raise SensorsNotFound if no sensors
    """
    def __init__(self, ignore_missing_sensors=False):
        """TODO add init param, raise error if missing (current behavior)/silent if missing. silent would ONLY warn once as would only need to instantiate object once
        
            @param ignore_missing_sensors - if true does NOT raise an error
                    for missing sensors and will attempt to re-connect
                    to the temp sensors.
                    NOTE if wmi is missing error will still be raised.
        """
        self.cpu_sensor = None
        self.mb_sensor = None
        
        self.temp_cpu_location = None
        self.temp_mb_location = None
        
        self.is_windows = False
        
        if sys.platform.startswith('win'):
            self.is_windows = True
        
        if self.is_windows:
            # Windows
            if wmi is None:
                log.warn('warning: wmi is missing, no temperature support')
                raise SensorsNotFound()
            
            ## Names of sensors (also happens to work on ASUS M2NPV2-DHS motherboards too
            self.temp_cpu_location = '/lpc/it8716f/temperature/0'
            self.temp_mb_location = '/lpc/it8716f/temperature/1'
            
            # Assume Open Hardware Monitor is up and running
            self.w = wmi.WMI(namespace='root\\OpenHardwareMonitor')
            # could perform OHM check here...
            self.update()  # FIXME see note on cpu_sensor attribute no longer needed
        else:
            # Assume Linux
            
            # Just default/assume temperature device location
            self.temp_cpu_location = '/sys/devices/LNXSYSTM:00/LNXSYBUS:00/PNP0A08:00/ATK0110:00/hwmon/hwmon1/temp1_input'
            self.temp_mb_location = '/sys/devices/LNXSYSTM:00/LNXSYBUS:00/PNP0A08:00/ATK0110:00/hwmon/hwmon1/temp2_input'
            if not os.path.exists(self.temp_cpu_location):
                self.temp_cpu_location = None
            else:
                # FIXME this logic is rather roundabout
                # FIXME no need for cpu_sensor attribute, remove
                self.cpu_sensor = self.temp_cpu_location
            if not os.path.exists(self.temp_mb_location):
                self.temp_mb_location = None
            else:
                # FIXME this logic is rather roundabout
                self.mb_sensor = self.temp_mb_location
            if DEBUG_TEMPS:
                self.temp_cpu_location, self.temp_mb_location = 32, 45  # Fake temps for debugging
                self.cpu_sensor, self.mb_sensor = self.temp_cpu_location, self.temp_mb_location  # Fake temps for debugging
        
        if self.cpu_sensor is None:
            # FIXME use logging warning (pyusb seems to hijack so logging support needs time)
            log.warn('warning: CPU temp file missing')
            if self.is_windows:
                # TODO check process list?
                log.warn('\tinstall and start wmi / Open Hardware Monitor')
            else:
                log.warn('\tinstall and configure lm-sensors')
        if self.mb_sensor is None:
            log.warn('warning: Motherboard temp file missing')
            if self.is_windows:
                log.warn('\tinstall and start wmi / Open Hardware Monitor')
            else:
                log.warn('\tinstall and configure lm-sensors')
        
        if not ignore_missing_sensors:
            if self.cpu_sensor is None and self.mb_sensor is None:
                raise SensorsNotFound()
        
        self.update()
    
    def update(self):
        """Pick up new readings
        """
        # Set current temps to None, if temp sensors disappear (e.g.OHM dies)
        # this fails gracefully
        self.sensor_values = {'cpu': None, 'mb': None}
        
        # find sensors
        if self.is_windows:
            # TODO (for Windows) this seems pretty heavy, it iterates ALL sensors!
            # however this is the way OHM / wmi is implemented (i.e. there doesn't seem to be a way around this
            # can not retain sensor as it never updates
            self.cpu_sensor = None
            self.mb_sensor = None

            for sensor in self.w.Sensor():
                if sensor.Identifier == self.temp_cpu_location:
                    self.cpu_sensor = sensor
                    self.sensor_values['cpu'] = self.cpu_sensor.Value
                elif sensor.Identifier == self.temp_mb_location:
                    self.mb_sensor = sensor
                    self.sensor_values['mb'] = self.mb_sensor.Value
        else:
            if self.temp_cpu_location:
                self.sensor_values['cpu'] = read_temp(self.temp_cpu_location)
            if self.temp_mb_location:
                self.sensor_values['mb'] = read_temp(self.temp_mb_location)
    
    # NOTE if more sensors are added, switch to a single lookup function with a name parameter,e.g. s.get('cpu')
    def temp_cpu(self):
        # TODO make this a property.
        return self.sensor_values['cpu']

    def temp_mb(self):
        # TODO make this a property.
        return self.sensor_values['mb']


def simpleimage_temperature(im, sensors=None):
    """Apply temperature to image.
    This is NOT (yet?) the same format as c version of asusdisplay.
    Image format is a PIL image
    NOTE modifies image in place
    """
    d = ImageDraw.Draw(im)
    # FIXME need better method of calculating position
    start_pos = 100  # getsize call?
    off_set = 40  # getsize call?
    temp_cpu = sensors.temp_cpu()
    if temp_cpu:
        degree_sign = u'\u00b0'  # or empty string...
        my_text = 'CPU: %02d%sC' % (temp_cpu, degree_sign)  # optionally add degree sign? Required Unicode font (or at least non-ASCII, e.g. latin1 font is fine)
        d.text((0, start_pos), my_text, font=temp_font, fill=clock_color)
    
    temp_mb = sensors.temp_mb()
    if temp_mb:
        #my_text = 'Motherboard %r C' % read_temp(temp_mb)
        my_text = 'MB: %02d%sC' % (temp_mb, degree_sign)
        d.text((0, start_pos + off_set), my_text, font=temp_font, fill=clock_color)
    
    return im


def simpleimage_resize(im):
    if im.size > MAX_IMAGE_SIZE:
        log.debug('resizing')
        """resize - maintain aspect ratio
        NOTE PIL thumbnail method does not increase
        if new size is larger than original
        2 passes gives good speed and quality
        """
        im.thumbnail((MAX_IMAGE_SIZE[0] * 2, MAX_IMAGE_SIZE[1] * 2))
        im.thumbnail(MAX_IMAGE_SIZE, Image.ANTIALIAS)
    
    # image is not too big, but it may be too small
    # _may_ need to add black bar(s)
    if im.size < MAX_IMAGE_SIZE:
        log.debug('too small - add bar')
        im = im.convert('RGB')  # convert to RGB
        bg_col = (0, 0, 0)
        background = Image.new('RGB', MAX_IMAGE_SIZE, bg_col)
        x, y = im.size
        background.paste(im, (0, 0, x, y))  # does not center/centre
        im = background
    
    return im


def image2raw(im):
    """Convert a PIL image into raw format suitable for ASUS A33 DAV screen
    
    returns raw buffer
    """
    
    im = simpleimage_resize(im)  # do not assume image is correct size
    
    """ convert to RGB
    - TODO add check around this, image may already be RGB"""
    im = im.convert('RGB')
    newbuff = im.tostring()
    return newbuff


def process_image(im, include_clock=True, sensors=None):
    im = simpleimage_resize(im)
    
    if sensors or include_clock:
        """this could be avoided if font was printed with a black background
        (would also help with colors, e.g. white text on white image,
        if black bar pasted first it would be readable)"""
        im = im.copy()
    
    if include_clock:
        im = simpleimage_clock(im, include_clock=include_clock)
    if sensors:
        sensors.update()
        im = simpleimage_temperature(im, sensors=sensors)
    rawimage = image2raw(im)
    return rawimage


def raw2png(rawdata):
    """Diagnostic return PIL image from raw image
    """
    im = Image.fromstring('RGB', (320, 240), rawdata)
    x = im.getdata()
    return im


def rawfile2png(raw_filename, image_filename):
    """Diagnostic to dump a viewable (PNG) of raw image
    """
    log.debug('reading %r', raw_filename)
    f = open(raw_filename, 'rb')
    binstr = f.read()
    f.close()
    
    im = raw2png(binstr)
    im.save(image_filename)

    log.debug('wrote %r', image_filename)


def ignore_keyboardinterrupt(callable_object, *args, **kwargs):
    """Run callable_object() and ignore/drop any CTRL-C presses during
    execution. This only works under CPython, it works my spawning a Thread
    that executes callable_object.
    """
    try:
        # CPython threads ignore CTRL-C, (mis)use that to our advantage
        mythread = threading.Thread(target=callable_object, *args, **kwargs)
        mythread.start()
        mythread.join()
    except KeyboardInterrupt:
        # This ignores any ctrl-c that occured whilst waiting for the thread to join
        pass



ASUS_VENDOR_ID = 0x1043
ASUS_PRODUCT_ID = 0x82B2

ImgHeader = [0x02, 0x00, 0x20, 0x00, 0x20, 0x84, 0x03,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x01, 0xF0, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00]
ImgHeader = ''.join(map(chr, ImgHeader))  # convert to "string" (byte buffer)


class DavDisplayModel(object):
    def displayinit(self):
        pass
    
    def close(self):
        pass
    
    def sendimage(self, rawimage):
        pass


class DavDisplayDebugFile(DavDisplayModel):
    def __init__(self):
        self._image_count = 0
    
    def sendimage(self, rawimage):
        self._image_count += 1
        fname = 'fake_display%03d.png' % self._image_count
        im = raw2png(rawimage)
        im.save(fname)
        log.debug('wrote %r', fname)


class DavDisplayDebugTk(DavDisplayModel):
    def __init__(self):
        self._image_count = 0
        import Tkinter
        import Image
        import ImageTk
        width, height = MAX_IMAGE_SIZE
        # create the canvas, size in pixels
        self._canvas = Tkinter.Canvas(width=width, height=height, bg='yellow')

        # pack the canvas into a frame/form
        self._canvas.pack(expand=Tkinter.YES, fill=Tkinter.BOTH)
    
    def sendimage(self, rawimage):
        self._image_count += 1
        log.debug('self._image_count %r', self._image_count)
        im = raw2png(rawimage)
        tk_im = ImageTk.PhotoImage(im)
        self._canvas.create_image(0, 0, image=tk_im, anchor=Tkinter.NW)
        self._canvas.update()
    
    ## FIXME implement clean close


class DavDisplay(DavDisplayModel):
    def __init__(self):
        """
        NOTE this really should be a Highlander object, There Can Be Only One!
        """
        self.packet_count = 0
        self.dev = None
        self._platform = os.name
        
        # packet #4 write - Informs devices the first part of the image data is coming next
        self.sendimage_p1 = 'USBC\xa1i\x00\x00\x00\x00\x01\x00\x00\x00\x0c\xe6\x02\x00\x01\x00\x00\x00\x03\x84 \x00\x00\x00\x00\x00\x00'

        # packet #7 write
        self.sendimage_p2 = 'USBC\xa2i\x00\x00\x00\x00\x01\x00\x00\x00\x0c\xe6\x02\x00\x01\x00\x00\x00\x03\x84 \x01\x00\x00\x00\x00\x00'

        # packet #10 write
        self.sendimage_p3 = 'USBC\xa3i\x00\x00\x00\x00\x01\x00\x00\x00\x0c\xe6\x02\x00\x01\x00\x00\x00\x03\x84 \x02\x00\x00\x00\x00\x00'

        # packet #13 write
        self.sendimage_p4 = 'USBC\xa4i\x00\x00 \x84\x00\x00\x00\x00\x0c\xe6\x02\x00\x00\x84 \x00\x03\x84 \x03\x00\x00\x00\x00\x00'
    
    def displayinit(self):
        """Similar to C version of () and displayinit()
        
        NOTE missing explicit:
           * usb_clear_halt()
           * usb_resetep()
        """
        dev = usb.core.find(idVendor=ASUS_VENDOR_ID, idProduct=ASUS_PRODUCT_ID)

        if dev is None:
            raise ValueError('Device not found')
        
        # NOTE sometimes one of these fails and needs to be re-ran.
        # I've not yet worked out a robust way to deal with this :-(
        try:
            dev.set_configuration()
        except usb.core.USBError, info:
            pass  # assume everything is OK
        try:
            dev.detach_kernel_driver(1)
        except usb.core.USBError, info:
            pass  # assume everything is OK
        except AttributeError, info:
            # Ignore under Windows, missing with libusb-0.1
            # NOTE no backend check below, just platform
            if self._platform != 'nt':
                raise
        
        self.dev = dev
        
        # packet #1 write
        data = 'USBC\x00\x00\x00\x00\t\x00\x00\x00\x00\x00\x0c\xe6\x0b\x00\x00\x00\t\x00\x00\x00\t\x00\x00\x00\x00\x00\x00'
        self.write(0x02, data)

        # packet #2 write
        data = '\x0b\xb2\x08\x00\t\x00\x00\x00\x01'
        self.write(0x02, data)

        # packet #3 read
        result_data = self.read(0x81, 13)
        #assert len(data) == 13, '%d != 13' % (len(data), )   # under Python may see 9? (TODO CHECK newlines?)
    
    def close(self):
        """Similar to C version of close
        """
        if self._platform != 'nt':
            self.dev.reset()
        #del(self.dev)  #? TODO
    
    def sendimage(self, rawimage, interruptible=False):
        """Calls real sendimage in a safe uninterruptible (e.g. from CTRL-C) thread
        
        TODO TEST (under linux and Windows).
        
        If the display protocol of the USB LCD screen is not followed,
        e.g. abort after write of the 2nd chunk of the image, the screen
        no longer responds until powered off/on (i.e. an OS reboot doesn't
        fix it).
        
        TODO consider a flag to this routine, ignore_ctrl_c=True allowing caller to disable.
        """
        if interruptible:
            self._sendimage(rawimage)
        else:
            ignore_keyboardinterrupt(self._sendimage, args=[rawimage])
    
    def _sendimage(self, rawimage):
        """Similar to C version of sendimage()
        
        @param rawimage - the raw image to send
        """
        # packet #4 write - Informs devices the first part of the image data is coming next
        data = self.sendimage_p1
        self.write(0x02, data)

        """
        # send image in 3 chunks (including an image header)
        image p1 32 + 65504  -- where 32 is the length of the image header
        image p2 65536
        image p3 65536
        """

        # packet #5 write - First of Image data, includes Image Header
        data = ImgHeader + rawimage[:65504]
        self.write(0x02, data)

        # packet #6 read
        result_data = self.read(0x81, 13)
        #assert len(data) == 13, '%d != 13' % (len(data), )   # under Python may see 9? (TODO CHECK newlines?)

        # packet #7 write
        data = self.sendimage_p2
        self.write(0x02, data)

        # packet #8 write - Second of Image data
        data = rawimage[65504:65504 + 65536]
        self.write(0x02, data)

        # packet #9 read
        result_data = self.read(0x81, 13)
        #assert len(data) == 13, '%d != 13' % (len(data), )   # under Python may see 9? (TODO CHECK newlines?)

        # packet #10 write
        data = self.sendimage_p3
        self.write(0x02, data)

        # packet #11 write - Third of Image data
        data = rawimage[65504 + 65536:65504 + 65536 + 65536]
        self.write(0x02, data)

        # packet #12 read
        result_data = self.read(0x81, 13)
        #assert len(data) == 13, '%d != 13' % (len(data), )   # under Python may see 9? (TODO CHECK newlines?)

        # packet #13 write
        data = self.sendimage_p4
        self.write(0x02, data)

        # packet #14 write - Fourth/Last of Image data
        data = rawimage[65504 + 65536 + 65536:]
        self.write(0x02, data)

        # packet #15 read
        result_data = self.read(0x81, 13)
        #assert len(data) == 13, '%d != 13' % (len(data), )   # under Python may see 9? (TODO CHECK newlines?)
        
    def write(self, ep, block):
        """Similar to C version of writedata
        
        Writes a block of size data to the device, if an error happens then
        exception is raised.
        
        C: int writedata(int ep, void *block, int size)
        Python dev as param 1, no size needed (uses all of block).
        No return value.
        """
        self.packet_count += 1
        # interface may need to specify bulk....
        self.dev.write(ep, block, interface=None, timeout=1000)
    
    def read(self, ep, size):
        """Similar to C version of readdata
        
        Reads data from the endpoint if an error happens then exception is
        raised.
        
        C: int readdata(int ep, void *block, int size)
        Python dev as param 1, no block passed in (returns data instead),
        size needed. returns data
        """
        self.packet_count += 1
        result = self.dev.read(ep, size, interface=None, timeout=1000)
        return result


def gen_images_urls(search_term=None, random_offset=True):
    """Quick-n-dirty random image URL generator.
    Uses google, and google's image cache and thus tends to only have small images
    Also google images tend to be "safe"
    """
    RE_IMAGEURL = re.compile('"(http.+?gstatic.com/images.+?)"')  # filter googles copies of the static images
    word_list = []
    charset = string.ascii_lowercase * 2 + string.digits
    
    if not search_term:
        # generate complete garbage - not a real (English) word
        for i in range(random.randint(2, 7)):
            word_list.append(random.choice(charset))
        search_term = ''.join(word_list)
    
    #limit_size = '&tbs=isz:ex,iszw:320,iszh:240'  # 320x240 - note only useful if getting original image
    limit_size = "&tbs=isz:lt,islt:2mp"  # Larger than 2MP
    if random_offset:
        start_offset = (random.randint(0, 50) * 10)  # Randomize the start page
    else:
        start_offset = 0
    request_url = 'http://images.google.com/images?q=%s&start=%d' % (search_term, start_offset)
    request_url += limit_size
    request_hdr = {'User-Agent': 'random/1.0'}
    
    request = urllib2.Request(request_url, None, request_hdr)
    html = urllib2.urlopen(request).read()
    """TODO? detect "did not match any documents."? will need "&hl=en" adding
    NOTE at this point have links to "small" google static cache in html,
    also have embedded data:image (base64) slightly larger than "small"
    images that could be extracted and ran through mime"""
    
    matches = RE_IMAGEURL.findall(html)
    for hit in matches:
        if hit.startswith('http') and len(hit) < 128 and 'gstatic.com/images':
            # this complex if basically skips the first regexp hit (could just ignore matches[0] I supposed, really need to fix regx
            yield hit


def main(argv=None):
    if argv is None:
        argv = sys.argv
    
    # Future option, only update display every minute (and do not display secs)
    update_display_period = 1  # number of seconds to wait before updating display
    floor = math.floor  # minor optimization

    # Super horrible command line checking
    do_random = False
    daemon_mode = False
    include_clock = True
    if '--no_clock' in argv:
        include_clock = False
    if '--no_secs' in argv:
        update_display_period = 60 / 3  # cpu temp to be monitored 3x a minute
        include_clock = 'no_secs'
        #import pdb ; pdb.set_trace()
    if '--random' in argv:
        do_random = True
    if '--daemon_mode' in argv:
        daemon_mode = True
    
    try:
        sensors = TemperatureSensors(ignore_missing_sensors=True)
    except SensorsNotFound, info:
        sensors = None
    
    if do_random:
        # DEBUG mode, useful for soak testing to make sure LOTS of updates work
        log.debug('getting random images....')
        image_list = []
        for url in gen_images_urls(search_term='maps', random_offset=False):
            log.debug(url)
            fobj = urllib2.urlopen(url)
            image_data = fobj.read()
            fileptr = StringIO.StringIO(image_data)  # fobj is missing seek() required by PIL
            im = Image.open(fileptr)
            rawimage = image2raw(im)
            image_list.append(rawimage)
    else:
        image_filename = argv[1]
        log.debug('reading %r', image_filename)
        im = Image.open(image_filename)
        im = simpleimage_resize(im)
        rawimage = process_image(im, include_clock=include_clock, sensors=sensors)

    ## TODO move this earlier?
    if DEBUG_DISPLAY:
        if DEBUG_DISPLAY == 'file':
            display = DavDisplayDebugFile()
        elif DEBUG_DISPLAY == 'tk':
            display = DavDisplayDebugTk()
        else:
            raise NotImplemented()
    else:
        display = DavDisplay()
    display.displayinit()
    
    if do_random:
        while 1:
            for rawimage in image_list:
                # TODO need fps
                display.sendimage(rawimage)
    else:
        display.sendimage(rawimage)
        while daemon_mode:
            # FIXME add interupt handler than would unset daemon_mode
            """NOTE with "standard" CPU AMD Athlon(tm) 64 Processor 3800+
            
            Old occasionally jumped 2 secs code performance figures:
                daemon mode takes 12-17% of CPU time (according to top in TinyCore)
                Windows 7 task manager shows about 18-20%
                
                process_image(clock+2*temps) takes 0.21 secs on the same hardware! TinyCore Py2.6
                process_image(clock+2*temps) + sendimage() takes 0.5 secs on the same hardware! TinyCore Py2.6
            
            With more accurate timing code (datetime), Linux (TinyCore) top shows CPU usage at 22%
            With more accurate timing code (time), Linux (TinyCore) top shows CPU usage at 21%
            
            '--no_secs' option reduces CPU time to 0-4% at the expense of less
            updates of cpu/mb temperature monitoring and no seconds in the
            time display
            
            Hopefully some scope for improvement as there are no optimizations in place at the moment.
            """
            
            # time module more accurate timer code
            start_time = time.time()
            #future_time = int(start_time) + update_display_period  # int conversion and float math
            future_time = floor(start_time) + update_display_period  # pure float math
            sleep_time = future_time - start_time
            time.sleep(sleep_time)
            
            """NOTE because this sleep should awaken dead on the next whole
            second, the time display lag will be say 0.5 secs (or however
            long it takes to process and send the image).
            """
            
            #start = time.time()
            
            rawimage = process_image(im, include_clock=include_clock, sensors=sensors)
            #print 'render took %3.5f' % (time.time() - start,)
            display.sendimage(rawimage)
            #print '    render + send took %3.5f' % (time.time() - start,)
    display.close()
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
