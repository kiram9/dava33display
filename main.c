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
#include <displayconstants.h> //include the datastructure and strings for the display communication 
/* 
*Writes a block of size data to the device, if an error happens then it will exit with error returned
*
*/
int writedata(struct usb_dev_handle *usb_handle, int ep, void *block, int size)
{
  int len;
  int transmitted=0;
  void *data;
  int ret;

  do {
    len = size - transmitted;
    data = (unsigned char *)block + transmitted;

	/* The 1000 is a timeout value */
    ret = usb_bulk_write(usb_handle, ep, data, len, 1000);
    if (ret < 0) {
      printf("usb_bulk: usb_bulk returned error %x\n", ret);
      return -ret;
    } else {
      transmitted += ret;
    }
  } while (ret > 0 && transmitted < size);
  
  return transmitted;
}
/*
*Reads data from the endpoint if an error happens then it will return the error 
*/
int readdata(struct usb_dev_handle *usb_handle, int ep, void *block, int size)
{
  int len;
  int transmitted=0;
  void * data;
  int ret;

  do {
    len = size - transmitted;
    data = (unsigned char *)block + transmitted;

/* The 1000 is a timeout value */
    ret = usb_bulk_read(usb_handle, ep, data, len, 1000);
    if (ret < 0) {
		usb_clear_halt(usb_handle, ep);
		  printf("usb_bulk: usb_bulk returned error %x\n", ret);
		  return -ret;
    } else {
      transmitted += ret;
    }
  } while (ret > 0 && transmitted < size);

  return transmitted;
}
/*
*This sets up some BOT packets and then reads the result back to check certian 
*status fields in the display we really dont do anything with this function it was 
*just used as an initial test to make sure that we can communicate with the display 
*/
void readinitialization(struct usb_dev_handle *usb_handle, uint32_t * taghandle)
{
	int i= 0;
	unsigned char devicename[0xFF]; 
	struct commandblockwrapper command; 
	struct commandstatuswrapper resp;
	uint8_t response[0x026];
	memcpy(command.dCBWSignature,"USBC",4);
	command.dCBWTag = 0x0000FFFF & (uint32_t) time(NULL); //0x0CC4;
	command.dCBWDataTransferLength = 0x26; 
	command.bmCBWFlags = bmCBWFlagsDataIn;
	command.bCBWLUN = 0x00;
	command.bCBWCBLength = 0x0C;
	memcpy(command.CBWCB,CBWCBInit1,sizeof(CBWCBInit1));


	memcpy(devicename,&command,sizeof(command));

		for (i = 0; i < sizeof(response); i++)
		{
			response[i] = 0x00;
		}
	fprintf(stderr, "size of command is %lX \n",sizeof(command));
	
	//send bulk command 
	writedata(usb_handle, 0x02, &command, sizeof(command));
	//Read data from bulk command 
	if(readdata(usb_handle, 0x81, response, sizeof(response)) > -1)
	{
//		fprintf(stderr, response);
	}
	else fprintf(stderr, "did not get response data ");
	
	//get command ack
	if (readdata(usb_handle, 0x81,&resp, 0x0D) > -1)
	{
		
	}
	else fprintf(stderr, "did not get command response ");

	*taghandle = command.dCBWTag;
	
}

/*Sends an image to the display as put in imgbuffer assumes an image buffer of 320*240*3 bytes */
void sendimage(struct usb_dev_handle *usb_handle,uint32_t * taghandle, uint8_t * imgbuffer)
{
	int i= 0;
	uint8_t imgdata[(0x10000*3 + 0x8420)];
	struct commandblockwrapper command; 
	struct commandstatuswrapper resp;


	memcpy(command.dCBWSignature,"USBC",4);
	command.dCBWTag = *taghandle; //0x0CC4;
	command.dCBWDataTransferLength = 0x09; 
	command.bmCBWFlags = 0x00;
	command.bCBWLUN = 0x00;
	command.bCBWCBLength = 0x0C;
	memcpy(command.CBWCB,CBWCBInitSend,sizeof(CBWCBInit1));
	
	//We copy the image data passed in on imgbuffer into our own buffer and then 
	//flip it horiz and vertically and swap the RGB to RBG since this is the format 
	//of the display (terrible format) 
	for (i = 32; i < sizeof(imgdata);){
		imgdata[i] = imgbuffer[320*240*3 - (((i-32)/960)*960 + 960 -(i - 32)%960)]; 
		i++;
		imgdata[i] = imgbuffer[320*240*3 - (((i + 1-32)/960)*960 + 960 -(i+ 1 - 32)%960)]; //swap G and B
		i++;
		imgdata[i] = imgbuffer[320*240*3 - (((i-1-32)/960)*960 + 960 -(i-1 - 32)%960)];  //swap G and B
		i++;

	}
	//start image transfer by sending an init command (we really only need to do this 
	//one time and then we can just loop and send the data serially 
	writedata(usb_handle, 0x02, &command, sizeof(command));
	writedata(usb_handle, 0x02, InitSendData, sizeof(InitSendData));
	readdata(usb_handle, 0x81, &resp, 0x0d);
	
	//send first bunch of image data 
	command.dCBWTag++;
	command.dCBWDataTransferLength = 0x10000; 
	memcpy(command.CBWCB,SendFirstPart,sizeof(CBWCBInit1));
	//copy in image header information 
	memcpy(imgdata,ImgHeader,sizeof(ImgHeader));
	writedata(usb_handle, 0x02, &command, sizeof(command));
	writedata(usb_handle, 0x02, imgdata, 0x10000);
	readdata(usb_handle, 0x81, &resp, 0x0d);


	
	//send second bunch of data 
	command.dCBWTag++;
	command.dCBWDataTransferLength = 0x10000; 
	(command.CBWCB)[10]++;
	writedata(usb_handle, 0x02, &command, sizeof(command));
	writedata(usb_handle, 0x02, imgdata + 0x10000, 0x10000);
	readdata(usb_handle, 0x81, &resp, 0x0d);
	
	//send third bunch of data 
	command.dCBWTag++;
	command.dCBWDataTransferLength = 0x10000; 
	(command.CBWCB)[10]++;
	writedata(usb_handle, 0x02, &command, sizeof(command));
	writedata(usb_handle, 0x02, imgdata + 0x20000, 0x10000);
	readdata(usb_handle, 0x81, &resp, 0x0d);
	
	//send forth bunch of data 
	command.dCBWTag++;
	command.dCBWDataTransferLength = 0x8420; 
	(command.CBWCB)[10]++;
	//This time we seem to be writing to a non sequential memory location to cause 
	//the display to update so we kind of mess with the communication protocal 
	//to update what looks like a very different memory location 
	(command.CBWCB)[3] = 0;
	(command.CBWCB)[4] = 0x84;
	(command.CBWCB)[5] = 0x20;
	writedata(usb_handle, 0x02, &command, sizeof(command));
	writedata(usb_handle, 0x02, imgdata + 0x30000, 0x8420);
	readdata(usb_handle, 0x81, &resp, 0x0d);

	//we want to return the tag handle so we can keep on updating the message tag 
	//sequentially, (although i dont think the display cares too much it might mess 
	//with its state machine if it doesnt get sequential tags. 
	*taghandle = command.dCBWTag;
}

/* poll  
* this is a function that polls some other memory location and seems to get 
* a response, I think it is used to read the raw front panel button press values or 
* something however since they are enumerated on a different endpoint it makes 
* little sense to poll the display, it might also be used to poll the serial port header
* that is unpopulated internally? however since i dont know how it is used it 
* just generates crap on the bus and I dont use it for now, for experemental purposes 
* only 
*/
void poll(struct usb_dev_handle *usb_handle, uint32_t * taghandle)
{
	char devicename[0xFF]; 
	struct commandblockwrapper command; 
	struct commandstatuswrapper resp;


	memcpy(command.dCBWSignature,"USBC",4);
	command.dCBWTag = *taghandle; //0x0CC4;
	command.dCBWDataTransferLength = 0x0100; 
	command.bmCBWFlags = bmCBWFlagsDataIn;
	command.bCBWLUN = 0x00;
	command.bCBWCBLength = 0x0C;
	memcpy(command.CBWCB,pollcommand,sizeof(pollcommand));
	//poll
	writedata(usb_handle, 0x02, &command, sizeof(command));
	while (0 < usb_bulk_read(usb_handle, 0x81, devicename,0x26,100));
	/*Whenever the device was polled with this command in windows it would cause a 
	bus halt so we would have to clear the halt, this would happen at random times 
	somewhere in the middle of reading back the data the device is trying to send, 
	I guess whoever wrote the firmware did something strange here, so I just assume
	there is a halt after the first read, clear it, and then read again*/
	usb_clear_halt(usb_handle, 0x81);
	readdata(usb_handle, 0x81, &resp, 0x0d);

	*taghandle = command.dCBWTag;
}	
/*Init the device, this was pulled from the libusb project examples, it reads the 
devices on the bus and returns a handle to my device. 
*/
static struct usb_device *device_init(void)
{
    struct usb_bus *usb_bus;
    struct usb_device *dev;

    usb_init();
    usb_find_busses();
    usb_find_devices();

    for (usb_bus = usb_busses;
         usb_bus;
         usb_bus = usb_bus->next) {
        for (dev = usb_bus->devices;
             dev;
             dev = dev->next) {
            if ((dev->descriptor.idVendor
                  == ASUS_VENDOR_ID) && //VID 
                (dev->descriptor.idProduct
                  == ASUS_PRODUCT_ID)) //PID
                return dev;
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
		struct usb_device *usb_dev;
		struct usb_dev_handle *usb_handle;
		int retval = 1;
		int i;
		uint32_t starttag = 0x0000FFFF & (uint32_t) time(NULL);
		char devicename[0xFF]; 

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
			fseek(f, 0x35, 0); //we assume the file is an uncompressed bmp, should tidy this up better
			//read in the file to a memory buffer 
			n = fread(buffer, 320*240*3, 1, f);
		}
		else
		{
			fprintf(stderr,
				    "Cannot open the image file \n");
			return (0);
		}
		//a small test to see what the screen shows us 
		//for (i = 0; i < sizeof(buffer);){
		//	buffer[i++] =  0x00; //green
		//	buffer[i++] =  0x00; //blue
		//	buffer[i++] =  0x00; //red
		//}
		for (i = 0; i < sizeof(devicename); i++)
		{
			devicename[i] = 0x00;
		}
		
		
		usb_dev = device_init();
		if (usb_dev == NULL) {
			fprintf(stderr, "Device not found\n");
			return (0);
		}

		usb_handle = usb_open(usb_dev);
		if (usb_handle == NULL) {
			fprintf(stderr,
				 "Not able to claim the USB device\n");
			usb_reset(usb_handle);
			usb_close(usb_handle);
			return (0);
		}
		//Prints some status info about the device 
		printf("Device Name: ");
		usb_get_string_simple(usb_handle, 16, devicename, sizeof(devicename));
		printf( devicename);
		printf("\nProduct: ");
		usb_get_string_simple(usb_handle, 32, devicename, sizeof(devicename));
		printf("%s\n",devicename);

		usb_get_string_simple(usb_handle, 96, devicename, sizeof(devicename));
		printf("Serial: %s \n", devicename);
		sleep(1);
		//usb_reset(usb_handle);
		//	goto exit;
		//set device to active configuration 1 (only configuration ) 
		usb_set_configuration(usb_handle,1);
		
		if(0 > usb_claim_interface(usb_handle,0))
		{
			fprintf(stderr,
				 "Not able to claim interface 0\n");
			usb_reset(usb_handle);
			usb_close(usb_handle);
			return (0);
		}
		//clear any halts we may have on the devices endpoints 
		usb_clear_halt(usb_handle,0x81);
		usb_clear_halt(usb_handle,0x02);

		readinitialization(usb_handle,&starttag);
		usb_resetep(usb_handle,0x81);
		usb_resetep(usb_handle,0x02);	
		//poll(usb_handle,&starttag);
		sendimage(usb_handle,&starttag,buffer);//buffer);
		//sleep(1);
		//shutdown the usb interface 
		usb_release_interface(usb_handle, 0);
		//maybe get rid of this..... TODO 
		usb_reset(usb_handle);
		usb_close(usb_handle);
		return (0);
}