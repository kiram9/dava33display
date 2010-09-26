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
#include <signal.h>
#include <signal.h>

#include <sys/resource.h> //to set process priority 
#include <HAL_display.h>
#include <gd.h>

#define XSIZE 320
#define YSIZE 240
#define TXTSIZE 128
/* this is kind of inefficient but we have to shove 32bit ints into 24bit pixels 
dropping the transparency byte*/
int copygdtobuf(unsigned char *buf, gdImagePtr in)
{
	int x, y, pixel; 
	int i = 0; 
	for(y = 0; y < YSIZE; y++)
	for(x = 0; x < XSIZE; x++)
	{
		pixel = in->tpixels[y][x];
		buf[i++] = (uint8_t)(pixel >> 16);
		buf[i++] = (uint8_t)(pixel >> 8);
		buf[i++] = (uint8_t)(pixel);
	}
	return (0); 
}

int genereatetime(gdImagePtr in)
{
	time_t curtime;
	struct tm *loctime;
	char txt[TXTSIZE];
	int brect[8];
	char *err;
	//char *font = "Orbitron/TTF/orbitron-medium.ttf"; /* User supplied font */
	char *font = "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf";
	int white;
	
	///get some time from the system to display 
	//SEE http://www.cs.utah.edu/dept/old/texinfo/glibc-manual-0.02/library_19.html#SEC320
	/* Get the current time.  */
	curtime = time (NULL);

	/* Convert it to local time representation.  */
	loctime = localtime (&curtime);
	strftime (txt, TXTSIZE, "%I:%M", loctime);
	white = gdImageColorAllocate(in, 255, 255, 255);  
	err = gdImageStringFT(in,&brect[0],white,font,60,0.0,20,80,txt);

	strftime (txt, TXTSIZE, ":%S%p", loctime);
	err = gdImageStringFT(in,&brect[0],white,font,20,0.0,brect[2],brect[1] - 1,txt);

	
	gdImageSetThickness(in, 2);
	gdImageLine(in, 20, 84, 300, 84, white);
	strftime (txt, TXTSIZE, "%A, %B %d", loctime);
	err = gdImageStringFT(in,&brect[0],white,font,10,0.0,20,95,txt);

	return (0);
}
int genereatesysinfo(gdImagePtr in, int x, int y,int size){
	char txt[20];
	int brect[8];
	char *err;
	FILE *f;
	//char *font = "Orbitron/TTF/orbitron-medium.ttf"; /* User supplied font */
	char *font = "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf";
	char *temppath = "/sys/class/thermal/thermal_zone0/temp";

	int white, grey, temperature, n;
	
	f = fopen(temppath, "r");
	if (f)
	{   

		//fseek(f, 0x35, 0); //we assume the file is an uncompressed bmp RGB, should tidy this up better
		//read in the file to a memory buffer 
		n = fread(txt,10, 1, f);
		fclose(f);
	}
	else
	{
		return (-1);
	}
	temperature = atoi (txt) / 1000;  
	white = gdImageColorAllocate(in, 255, 255, 255);  
	sprintf (txt, "TEMP %i C",temperature);
	err = gdImageStringFT(in,&brect[0],white,font,size,0.0,x,y,txt);
	if(50 > temperature)
		grey = gdImageColorAllocate(in, 0,255,0);
	else 	if(65 > temperature)
		grey = gdImageColorAllocate(in, 255,150,0);
	else grey = gdImageColorAllocate(in, 255,0,0);
	if (98 < temperature) temperature = 98; 
	gdImageSetThickness(in, 1);
	gdImageRectangle(in,brect[4], brect[5], brect[4] + 100,brect[5]+size, white);
	gdImageFilledRectangle(in,brect[4] + 1, brect[5] + 1, brect[4] + (temperature - 20)*2,brect[5] + size - 1, grey);

	return (0);
}



int daemonize;
void 
synch_signal (int sig)
{
	daemonize = 0;
	displayclose();
	
}
int main(int argc, char **argv)
{
	FILE *f;
	unsigned char buffer[XSIZE*YSIZE*3];
	//int n,i,x,y, pixel;
	int black;
	int white;
	gdImagePtr im, imback;

	struct sigaction usr_action;
	sigset_t block_mask;

	daemonize = 0;
	/* Establish the signal handler.  */
	sigfillset (&block_mask);
	usr_action.sa_handler = synch_signal;
	usr_action.sa_mask = block_mask;
	usr_action.sa_flags = 0;
	sigaction (SIGINT, &usr_action, NULL);



	
	if (argc < 2) {
		fprintf(stderr,
				"Specify one image file \n");
		return (0);
	}
	if (argc == 3)
		daemonize = 1; 

	//fprintf(stderr,argv[1]);
	f = fopen(argv[1], "rb");
	if (f)
	{   
		imback = gdImageCreateFromPng(f);
		//fseek(f, 0x35, 0); //we assume the file is an uncompressed bmp RGB, should tidy this up better
		//read in the file to a memory buffer 
		//n = fread(buffer, XSIZE*YSIZE*3, 1, f);
		fclose(f);
	}
	else
	{
		fprintf(stderr,
				"Cannot open the image file \n");
		return (0);
	}

	//Set our process priority to very low so we dont hog resources
	//see http://www.mkssoftware.com/docs/man3/setpriority.3.asp
	setpriority (PRIO_PROCESS, 0, 18);

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



	if(0 <= displayinit())
	{

		while(1){
			
			gdImageCopy(im,imback,0,0,0,0,XSIZE,YSIZE);
			genereatetime(im);
			genereatesysinfo(im,20,120,10);
			copygdtobuf(buffer,im);
			sendimage(buffer);//buffer);
			if (0 == daemonize)
				break; 
			usleep(500);
		}
		displayclose();
	}
	gdImageDestroy(im);
	return (0);
}