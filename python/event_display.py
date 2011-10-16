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
    update_sensors_period = 10  # number of seconds to wait before re-reading temperator sensors
    
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
    def process_image(image_in, include_clock=True, sensors=None):
        if sensors or include_clock:
            """this copy step could be avoided if font was printed with a black background
            (would also help with colors selection, e.g. white text on white image,
            if black bar pasted first it would be readable)"""
            image_in = image_in.copy()
        
        if include_clock:
            image_in = asusdisplay.simpleimage_clock(image_in, include_clock=include_clock)
        if sensors:
            image_in = asusdisplay.simpleimage_temperature(image_in, sensors=sensors)
        rawimage = asusdisplay.image2raw(image_in)
        return rawimage

    def update_display():
        start = time.time()
        rawimage = process_image(im, include_clock=include_clock, sensors=sensors)
        log.debug('render took %3.5f', time.time() - start)
        display.sendimage(rawimage)
        log.debug('    render + send took %3.5f', time.time() - start)

    def sensors_update_callback(sensors=None):
        # sensors doesn't really need to be a parameter
        if sensors:
            sensors.update()
        return True  # re-start timer - note rough timing as timer start now rather than an accurate poll interval depending on how long sensors.update() took to run

    def signal_ctrlc_cb():
        print 'signal handler called'
        # could still cause a problem if iterupted when in the middle of writing to USB device
        display.close()
        event.abort()  # shutdown event system
    
    def display_callback():
        event.timeout(update_display_period, display_callback)  # re-register the timer immediately to ensure wake up every number_of_secs secs (and do not include render time)
        update_display()
        return None  # causes the timer to NOT be called again
    
    # Paint initial display image
    if sensors:
        sensors.update()
    update_display()
    
    event.timeout(update_display_period, display_callback)
    event.timeout(update_sensors_period, sensors_update_callback, sensors)
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
