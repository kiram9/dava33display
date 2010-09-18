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
#include <sys/resource.h> //to set process priority 
#include <HAL_display.h>
#include <gd.h>

#define XSIZE 320
#define YSIZE 240
#define TXTSIZE 128
int main(int argc, char **argv)
{
	FILE *f;
	unsigned char buffer[XSIZE*YSIZE*3];
	int n,i,x,y, pixel;
	int black;
	int white;
	gdImagePtr im;
	time_t curtime;
	struct tm *loctime;
	char txt[TXTSIZE];
	char *s = "Hello.";
	int brect[8];
	char *err;
	char *font = "Orbitron/TTF/orbitron-medium.ttf"; /* User supplied font */

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
		n = fread(buffer, XSIZE*YSIZE*3, 1, f);
	}
	else
	{
		fprintf(stderr,
				"Cannot open the image file \n");
		return (0);
	}

	//Set our process priority to very low so we dont hog resources
	//see http://www.mkssoftware.com/docs/man3/setpriority.3.asp
	setpriority (PRIO_PROCESS, 0, 15);


	///get some time from the system to display 
	//SEE http://www.cs.utah.edu/dept/old/texinfo/glibc-manual-0.02/library_19.html#SEC320
	/* Get the current time.  */
	curtime = time (NULL);

	/* Convert it to local time representation.  */
	loctime = localtime (&curtime);
	strftime (txt, TXTSIZE, "%I:%M", loctime);



	//Generate some images
	im = gdImageCreateTrueColor(XSIZE, YSIZE);
	black = gdImageColorAllocate(im, 0, 0, 0);  
	white = gdImageColorAllocate(im, 255, 255, 255);  
	gdImageSetAntiAliased(im, black);
	gdImageSetAntiAliased(im, white);
	gdImageSetThickness(im, 4);
	//gdImageRectangle(im, 5, 5, XSIZE - 5, YSIZE - 5, white);
	//gdImageLine(im, 0, 0, 319, 239, white);
	//gdImageLine(im, 10, 10, 310, 10, white);
	err = gdImageStringFT(im,&brect[0],white,font,50,0.0,20,70,txt);

	
	//copy gd image into the buffer 
	i = 0; 
	for(y = 0; y < YSIZE; y++)
	for(x = 0; x < XSIZE; x++)
	{
		pixel = im->tpixels[y][x];
		buffer[i++] = (uint8_t)(pixel >> 16);
		buffer[i++] = (uint8_t)(pixel >> 8);
		buffer[i++] = (uint8_t)(pixel);
	}

	if(0 <= displayinit())
	{
		
		sendimage(buffer);//buffer);

		displayclose();
	}
	gdImageDestroy(im);
	return (0);
}