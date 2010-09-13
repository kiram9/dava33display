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
#define ASUS_VENDOR_ID   0x1043
#define ASUS_PRODUCT_ID  0x82B2

typedef struct commandblockwrapper {
	uint8_t dCBWSignature[4]; /* = {'U','S','B','C'}; Signature that helps identify this data packet as a CBW. The signature field shall contain the value
																			43425355h (little endian), indicating a CBW.	*/

	
	uint32_t dCBWTag;				/*A Command Block Tag sent by the host. The device shall echo the contents of this field back to the
												host in the dCSWTag field of these associated CSW. The dCSWTag positively associates a CSW with the
												corresponding CBW.*/

	
	uint32_t dCBWDataTransferLength;	/*The number of bytes of data that the host expects to transfer on the Bulk-In or Bulk-Out endpoint (as
																indicated by the Direction bit) during the execution of this command. If this field is zero, the device and
																the host shall transfer no data between the CBW and the associated CSW, and the device shall ignore
																the value of the Direction bit in bmCBWFlags.*/

	
	uint8_t bmCBWFlags;			/*The bits of this field are defined as follows:
												Bit 7
													Direction - the device shall ignore this bit if the dCBWDataTransferLength field is
													zero, otherwise:
													0 = Data-Out from host to the device,
													1 = Data-In from the device to the host.
												Bit 6
													Obsolete. The host shall set this bit to zero.
												Bits 5..0 Reserved - the host shall set these bits to zero.*/

	
	uint8_t bCBWLUN;				/*The device Logical Unit Number (LUN) to which the command block is being sent. For devices that
												support multiple LUNs, the host shall place into this field the LUN to which this command block is
												addressed. Otherwise, the host shall set this field to zero.*/

	
	uint8_t bCBWCBLength;	   /*The valid length of the CBWCB in bytes. This defines the valid length of the command block. The
												only legal values are 1 through 16 (01h through 10h). All other values are reserved.*/

	uint8_t CBWCB[16];			/*The command block to be executed by the device. The device shall interpret the first bCBWCBLength
												bytes in this field as a command block as defined by the command set identified by bInterfaceSubClass.
												If the command set supported by the device uses command blocks of fewer than 16 (10h) bytes in
												length, the significant bytes shall be transferred first, beginning with the byte at offset 15 (Fh). The
												device shall ignore the content of the CBWCB field past the byte at offset (15 + bCBWCBLength - 1).*/

}__attribute__((__packed__));
#define bmCBWFlagsDataIn	0x80

typedef struct commandstatuswrapper { 
		/*Signature that helps identify this data packet as a CSW. The signature field shall contain the value 53425355h (little endian), indicating CSW.*/
	uint8_t dCSWSignature[4];
	
		/*The device shall set this field to the value received in the dCBWTag of the associated CBW.*/
	uint32_t dCSWTag;
	
		/*For Data-Out the device shall report in the dCSWDataResidue the difference between the amount of
		data expected as stated in the dCBWDataTransferLength, and the actual amount of data processed by
		the device. For Data-In the device shall report in the dCSWDataResidue the difference between the
		amount of data expected as stated in the dCBWDataTransferLength and the actual amount of relevant
		data sent by the device. The dCSWDataResidue shall not exceed the value sent in the
		dCBWDataTransferLength.*/
	uint32_t dCSWDataResidue;
	
	uint8_t bCSWStatus;
}__attribute__((__packed__));
/*
bCSWStatus indicates the success or failure of the command. The device shall set this byte to zero if
the command completed successfully. A non-zero value shall indicate a failure during command
execution according to the following table:
*/
#define bCSWStatusPassed		0x00		//Command Passed ("good status")
#define bCSWStatusFailed			0x01		//Command Failed
#define bCSWStatusPhaseErr		0x02		//Phase Error

int writedata(struct usb_dev_handle *usb_handle, int ep, void *block, int size)
{
  int len;
  int transmitted=0;
  void *data;
  int ret;

  do {
    len = size - transmitted;
    data = (unsigned char *)block + transmitted;

/* The 5000 is a timeout value */
    ret = usb_bulk_write(usb_handle, ep, data, len, 1000);
    //fprintf(stderr, "wrote %i bytes to device\n ",ret);
    if (ret < 0) {
      printf("usb_bulk: usb_bulk returned error %x\n", ret);
      return -ret;
    } else {
      transmitted += ret;
    }
  } while (ret > 0 && transmitted < size);

  return transmitted;
}

int readdata(struct usb_dev_handle *usb_handle, int ep, void *block, int size)
{
  int len;
  int transmitted=0;
  void * data;
  int ret;

  do {
    len = size - transmitted;
    data = (unsigned char *)block + transmitted;

/* The 5000 is a timeout value */
    ret = usb_bulk_read(usb_handle, ep, data, len, 1000);
    //fprintf(stderr, "read %i bytes from device\n",ret);
    if (ret < 0) {
		usb_clear_halt(usb_handle, ep);
		//if(ret == -110){
		//	usb_resetep(usb_handle, ep);
		//	printf("usb_bulk: reset endpoint, : error %x\n", ret);
		//}
		//else{
		  printf("usb_bulk: usb_bulk returned error %x\n", ret);
		  return -ret;
		//}
    } else {
      transmitted += ret;
    }
  } while (ret > 0 && transmitted < size);

  return transmitted;
}
//initialization CBWCBs
static uint8_t CBWCBInit1[16] = {0x12, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, \
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
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
	
	if(readdata(usb_handle, 0x81, response, sizeof(response)) > -1)
	{
//		fprintf(stderr, response);
	}
	else fprintf(stderr, "did not get response data ");
	
	//get command response
	if (readdata(usb_handle, 0x81,&resp, 0x0D) > -1)
	{
		
	}
	else fprintf(stderr, "did not get command response ");

	*taghandle = command.dCBWTag;
	
}
static uint8_t CBWCBInitSend[16] = {0xE6, 0x0B, 0x00, 0x00, 0x00, 0x09, 0x00, \
						0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static uint8_t InitSendData [0x09] = {0x0B, 0xB2, 0x08, 0x00, 0x09, 0x00, 0x00, 0x00, 0x01};

/*static uint8_t InitFinish [16] = {0xE6, 0x09, 0x00, 0x00, 0x00, 0x0B, 0x00, \
						0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t InitFinishData[0x0B] = {0x09, 0xB2, 0x08, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0xB8, 0x0B};
*/
static uint8_t SendFirstPart[16] = {0xE6, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, \
						0x03, 0x84, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t ImgHeader[32] = {0x02, 0x00, 0x20, 0x00, 0x20, 0x84, 0x03, \
			0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x01, 0xF0, 0x00, 0x00, 0x00, \
			0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
			0x00, 0x00, 0x00};
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
	
	
	
	//memcpy(imgdata + 32,imgbuffer,sizeof(imgdata - 32));
	for (i = 32; i < sizeof(imgdata);){
		imgdata[i] = imgbuffer[320*240*3 - (((i-32)/960)*960 + 960 -(i - 32)%960)]; 
		i++;
		imgdata[i] = imgbuffer[320*240*3 - (((i + 1-32)/960)*960 + 960 -(i+ 1 - 32)%960)]; //swap G and B
		i++;
		imgdata[i] = imgbuffer[320*240*3 - (((i-1-32)/960)*960 + 960 -(i-1 - 32)%960)];  //swap G and B
		i++;

	}
	//start image transfer
	writedata(usb_handle, 0x02, &command, sizeof(command));
	writedata(usb_handle, 0x02, InitSendData, sizeof(InitSendData));
	readdata(usb_handle, 0x81, &resp, 0x0d);

	//send first bunch of data 
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
	//memcpy(command.CBWCB,SendFirstPart,sizeof(CBWCBInit1));
	(command.CBWCB)[10]++;
	writedata(usb_handle, 0x02, &command, sizeof(command));
	writedata(usb_handle, 0x02, imgdata + 0x10000, 0x10000);
	readdata(usb_handle, 0x81, &resp, 0x0d);
	
	//send third bunch of data 
	command.dCBWTag++;
	command.dCBWDataTransferLength = 0x10000; 
	//memcpy(command.CBWCB,SendFirstPart,sizeof(CBWCBInit1));
	(command.CBWCB)[10]++;
	writedata(usb_handle, 0x02, &command, sizeof(command));
	writedata(usb_handle, 0x02, imgdata + 0x20000, 0x10000);
	readdata(usb_handle, 0x81, &resp, 0x0d);
	
	//send forth bunch of data 
	command.dCBWTag++;
	command.dCBWDataTransferLength = 0x8420; 
	//memcpy(command.CBWCB,SendFirstPart,sizeof(CBWCBInit1));
	(command.CBWCB)[10]++;
	(command.CBWCB)[3] = 0;
	(command.CBWCB)[4] = 0x84;
	(command.CBWCB)[5] = 0x20;
	writedata(usb_handle, 0x02, &command, sizeof(command));
	writedata(usb_handle, 0x02, imgdata + 0x30000, 0x8420);
	readdata(usb_handle, 0x81, &resp, 0x0d);

	//send end tag
/*	command.dCBWTag++;
	command.dCBWDataTransferLength = sizeof(InitFinishData); 
	memcpy(command.CBWCB,InitFinish,sizeof(InitFinish));
	writedata(usb_handle, 0x02, &command, sizeof(command));
	writedata(usb_handle, 0x02, InitFinishData, sizeof(InitFinishData));
	readdata(usb_handle, 0x81, &resp, 0x0d);
*/
	//send some other tag perhaps a poll? 
	*taghandle = command.dCBWTag;
}
static uint8_t pollcommand[16] = {0xE7, 0x0A, 0x00, 0x00, 0x01, 0x00, 0x00, \
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
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
	usb_clear_halt(usb_handle, 0x81);
	readdata(usb_handle, 0x81, &resp, 0x0d);

	*taghandle = command.dCBWTag;
}	
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
                  == ASUS_VENDOR_ID) &&
                (dev->descriptor.idProduct
                  == ASUS_PRODUCT_ID))
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
			fseek(f, 0x35, 0);

			n = fread(buffer, 320*240*3, 1, f);
		}
		else
		{
			fprintf(stderr,
				    "Cannot open the image file \n");
			return (0);
		}

		//for (i = 0; i < sizeof(buffer);){
		//	buffer[i++] =  0x00; //green
		//	buffer[i++] =  0x00; //blue
		//	buffer[i++] =  0x00; //red
		//}
	
		printf("USB device test\n");
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
			goto exit;
		}

/*		if (argc == 1) {
			fprintf(stderr,
				    "specify at least 1 color\n");
			goto exit;
		}*/

/*		for (i = 1; i < argc; ++i) {
			if (strcasecmp(argv[i], "red") == 0)
				color |= RED;
			if (strcasecmp(argv[i], "blue") == 0)
				color |= BLUE;
			if (strcasecmp(argv[i], "green") == 0)
				color |= GREEN;
			if (strcasecmp(argv[i], "none") == 0)
				color = NONE;
		}*/
		fprintf(stderr, "Device Name: ");
		usb_get_string_simple(usb_handle, 16, devicename, sizeof(devicename));
		fprintf(stderr, devicename);
		fprintf(stderr, "\nProduct: ");
		usb_get_string_simple(usb_handle, 32, devicename, sizeof(devicename));
		fprintf(stderr, devicename);
		fprintf(stderr, "\nSerial: ");
		usb_get_string_simple(usb_handle, 96, devicename, sizeof(devicename));
		fprintf(stderr, devicename);
		fprintf(stderr, "\n");
		sleep(1);
		//usb_reset(usb_handle);
		//	goto exit;
		usb_set_configuration(usb_handle,1);
		if(0 > usb_claim_interface(usb_handle,0))
		{
			fprintf(stderr,
				 "Not able to claim interface 0\n");
			goto exit;
		}
		usb_clear_halt(usb_handle,0x81);
		usb_clear_halt(usb_handle,0x02);
		
		//usb_bulk_write(usb_handle, 0x02,initialdata , sizeof(initialdata),0);
		
		//writedata(usb_handle, 0x02, initialdata, sizeof(initialdata));
		//usb_bulk_read(usb_handle, 0x81, devicename,0x26,0);
		//usb_bulk_read(usb_handle, 0x81, devicename+ 26 ,0x0d,0);
		readinitialization(usb_handle,&starttag);
		usb_resetep(usb_handle,0x81);
		usb_resetep(usb_handle,0x02);	
		for(i = 0; i < 2; i++) {
			//poll(usb_handle,&starttag);
			sendimage(usb_handle,&starttag,buffer);//buffer);
			sleep(1);
		}
		usb_release_interface(usb_handle, 0);
		
		
		retval = 0;
		//
	exit:
		sleep(1);
		usb_reset(usb_handle);
		usb_close(usb_handle);
		return (0);
}