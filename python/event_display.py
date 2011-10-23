#!/usr/bin/env python
# -*- coding: ascii -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab
#
# pyevent_display.py - event based clock for USB TFT display of ASUS A33 DAV
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
"""Uses a simplistic event based dispatch aproach.

An event (observer pattern) implementation of asusdisplay.py:main()

"""

import os
import sys
import math
import time
import logging
import ctypes  # NOTE only used for Windows

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

#import event  # http://code.google.com/p/pyevent/
#import rel as event  # http://code.google.com/p/registeredeventlistener/
import fakeevent as event

import asusdisplay

DEBUG_DISPLAY = asusdisplay.DEBUG_DISPLAY
#asusdisplay.DEBUG_USBIO = True


def doit():
    """
    """
    log = asusdisplay.log  # re-use logging object
    log.setLevel(logging.NOTSET)  # only logs; WARNING, ERROR, CRITICAL
    
    # TODO Command line processing
    image_filename = '320x240_hal256.png'
    include_clock = True
    update_display_period = 1  # number of seconds to wait before updating display
    update_temperature_sensors_period = 10  # number of seconds to wait before re-reading temperator sensors
    update_hdfreespace_period = 5 * 60  # number of seconds to wait before re-reading ammount of free HD space
    
    try:
        sensors = asusdisplay.TemperatureSensors(ignore_missing_sensors=True)
    except asusdisplay.SensorsNotFound, info:
        sensors = None
    
    log.debug('reading %r', image_filename)
    im = Image.open(image_filename)
    im = asusdisplay.simpleimage_resize(im)
    
    if DEBUG_DISPLAY:
        if DEBUG_DISPLAY == 'file':
            display = asusdisplay.DavDisplayDebugFile()
        elif DEBUG_DISPLAY == 'tk':
            display = asusdisplay.DavDisplayDebugTk()
        else:
            raise NotImplemented()
    else:
        display = asusdisplay.DavDisplay()
    display.displayinit()

    ####################
    def human_readable_size(num):
        for x in ['bytes','KB','MB','GB','TB']:
            if num < 1024.0:
                return "%3.1f%s" % (num, x)
            num /= 1024.0
        
    def get_free_space(folder):
        """ Return folder/drive free space (in bytes)"""
        if sys.platform.startswith('win'):
            free_bytes = ctypes.c_ulonglong(0)
            ctypes.windll.kernel32.GetDiskFreeSpaceExW(ctypes.c_wchar_p(folder), None, None, ctypes.pointer(free_bytes))
            return free_bytes.value
        else:
            # Assume Unix
            s = os.statvfs(folder)
            return s.f_bsize * s.f_bavail
    
    def simpleimage_freespace(im, num_bytes, text_color=None):
        """Apply ammount of free hard drive space to image.
        Image format is a PIL image
        NOTE modifies image in place
        """
        d = ImageDraw.Draw(im)
        # FIXME need better method of calculating position
        start_pos = 200  # getsize call?
        my_text = 'Free HD: %s' % human_readable_size(num_bytes)
        text_color = text_color or asusdisplay.clock_color
        d.text((0, start_pos), my_text, font=asusdisplay.temp_font, fill=text_color)
        
        return im

    def simpleimage_date(im, text_color=None):
        """Apply date to image. TODO move into clock routine
        Image format is a PIL image
        NOTE modifies image in place
        """
        d = ImageDraw.Draw(im)
        # FIXME need better method of calculating position
        start_pos = 90  # getsize call?
        off_set = 40  # getsize call?
        #now = time.localtime(100 + 24 * 60 * 60)
        now = time.localtime()
        my_text = time.strftime('%a', now)
        text_color = text_color or asusdisplay.clock_color
        d.text((220, start_pos), my_text, font=asusdisplay.temp_font, fill=text_color)
        
        day = now.tm_mday
        if 4 <= day <= 20 or 24 <= day <= 30:
            suffix = "th"
        else:
            suffix = ["st", "nd", "rd"][day % 10 - 1]
        my_text = '%d%s' % (day, suffix)
        d.text((220, start_pos + off_set), my_text, font=asusdisplay.temp_font, fill=text_color)
        
        return im

    def process_image(image_in, include_clock=True, sensors=None, free_space=None):
        """Params:
            @param image_in - PIL image to write information over the top of
            @param include_clock - Include time information
            @param sensors - optiona temperature sensor information, instance of TemperatureSensors
            @param free_space - optional number of free bytes on harddrive
            
        """
        if sensors or include_clock:
            """this copy step could be avoided if font was printed with a black background
            (would also help with colors selection, e.g. white text on white image,
            if black bar pasted first it would be readable)"""
            image_in = image_in.copy()
        
        image_in = asusdisplay.simpleimage_clock(image_in, include_clock=include_clock)
        image_in = simpleimage_date(image_in)
        if sensors:
            image_in = asusdisplay.simpleimage_temperature(image_in, sensors=sensors)
        if free_space:
            #free_space = 888.8 * 1024* 1024* 1024  # Debug value
            image_in = simpleimage_freespace(image_in, free_space)
        rawimage = asusdisplay.image2raw(image_in)
        return rawimage

    def update_display():
        start = time.time()
        rawimage = process_image(im, include_clock=include_clock, sensors=sensors, free_space=free_space)
        log.debug('render took %3.5f', time.time() - start)
        display.sendimage(rawimage)
        log.debug('    render + send took %3.5f', time.time() - start)

    def temperature_sensors_update_callback(sensors=None):
        # sensors doesn't really need to be a parameter
        if sensors:
            sensors.update()
        return True  # re-start timer - note rough timing as timer start now rather than an accurate poll interval depending on how long sensors.update() took to run
    
    def hdspaces_sensors_update_callback():
        global free_space
        free_space = get_free_space('/')
        return True  # re-start timer

    def signal_ctrlc_cb():
        print 'signal handler called'
        # could still cause a problem if iterupted when in the middle of writing to USB device
        display.close()
        event.abort()  # shutdown event system
    
    global max_frames_to_paint
    max_frames_to_paint = 10
    def display_callback():
        """
        # debug code to only run for a few frames, allows easy profiling.
        global max_frames_to_paint
        max_frames_to_paint -= 1
        if not max_frames_to_paint:
            print 'quit'
            event.abort()
            return
        """
        event.timeout(update_display_period, display_callback)  # re-register the timer immediately to ensure wake up every number_of_secs secs (and do not include render time)
        update_display()
        return None  # causes the timer to NOT be called again
    
    # Paint initial display image
    temperature_sensors_update_callback(sensors)
    hdspaces_sensors_update_callback()
    update_display()
    
    event.timeout(update_display_period, display_callback)
    event.timeout(update_temperature_sensors_period, temperature_sensors_update_callback, sensors)
    event.timeout(update_hdfreespace_period, hdspaces_sensors_update_callback)
    event.signal(2, signal_ctrlc_cb)  # for CTRL-C under Unix and Windows - NOTE under Windows ctrl-break does not fire this
    event.dispatch()

    display.close()  # what happens if called multiple times?


def main(argv=None):
    if argv is None:
        argv = sys.argv
    
    doit()
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
