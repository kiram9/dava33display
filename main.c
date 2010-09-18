/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) kieran levin 2010 <kieranlevin@kieranlevin.com>
 * 
 * asusdisplay is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * asusdisplay is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <usb.h>
#include <stdint.h>
#include <time.h>
#include <HAL_display.h>

int main(int argc, char **argv)
{
		FILE *f;
		unsigned char buffer[320*240*3];
		int n;
		if (argc != 2) {
			fprintf(stderr,
				    "Specify one image file \n");
			return (0);
		}
		
		//fprintf(stderr,argv[1]);
		f = fopen(argv[1], "rb");
		if (f)
		{   
			fseek(f, 0x35, 0); //we assume the file is an uncompressed bmp RGB, should tidy this up better
			//read in the file to a memory buffer 
			n = fread(buffer, 320*240*3, 1, f);
		}
		else
		{
			fprintf(stderr,
				    "Cannot open the image file \n");
			return (0);
		}

	
		if(0 <= displayinit())
		{
		
			sendimage(buffer);//buffer);
		
			displayclose();
		}

		return (0);
}