//HAL_display.h
#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <string.h>
#include <usb.h>
#include <stdint.h>
#include <displayconstants.h> //include the datastructure and strings for the display communication 

int displayinit(); 
int displayclose();
int sendimage( uint8_t * imgbuffer);
int poll();

#endif 