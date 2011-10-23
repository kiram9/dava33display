#!/usr/bin/env python
# -*- coding: ascii -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab
#
# fakeevent.py - implements a partial pyevent like API
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
"""Also see RegisteredEventListener from 
http://code.google.com/p/registeredeventlistener/
And libevent (and pyevent) For more information.

Basically an event (observer pattern) implementation of asusdisplay.py:main()
that only uses batteries included modules.

Consider using PyDispatcher and bolting in timing to it.
"""


import time
#import threading


def signal(signal_number, callback_func, *args):
    """NOTE not actually implemented and unlikely to be added ....
    """
    pass


"""Threading timer (async-like) implementation.
TK tends to freak out/not work with this (presumbly due to no thread locking)
"""


def timeout_threading(number_of_secs, callback_func, *args, **kwargs):
    """NOTE does NOT support re-adding timer if result is not None"""
    temp_thread = threading.Timer(number_of_secs, callback_func, *args, **kwargs)
    temp_thread.start()
    # FIXME TODO add temp_thread to structure for lookup (and removal)


def dispatch_noop():
    pass


timeout = timeout_threading
dispatch = dispatch_noop


"""simplistic sleep (sync) implementation - appears to be less CPU intensive than rel in poll/select emulation mode
"""


_timesleep_queue = []  # FIXME TODO pick a more suitable structure


def timeout_timesleep(number_of_secs, callback_func, *args, **kwargs):
    now = time.time()
    end_time = now + number_of_secs
    _timesleep_queue.append((end_time, number_of_secs, callback_func, args, kwargs))
    _timesleep_queue.sort()  # this is not ideal :-(
    

def dispatch_timesleep():
    while _timesleep_queue:
        end_time, number_of_secs, callback_func, args, kwargs = _timesleep_queue.pop(0)
        sleep_period = end_time - time.time()
        if sleep_period > 0:
            time.sleep(sleep_period)
        result = callback_func(*args, **kwargs)
        if result:
            # re-add timer
            timeout_timesleep(number_of_secs, callback_func, *args, **kwargs)


def abort():
    """# shutdown event system
    """
    global _timesleep_queue
    _timesleep_queue = []

timeout = timeout_timesleep
dispatch = dispatch_timesleep

"""TODO consider implementing:
    
    event.loop(True)  # loop once, return 0 if events are waiting.
    event.loop(False)  # loop until no more events are waiting.

Documentation for pyevent is rather light but the above appear to be useful behaviors to clone.

Also consider implementing using an object instead of as a module.
"""
