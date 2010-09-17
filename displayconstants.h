/*displayconstants.h 
contains constants used to get data to the display since i dont really want to 
completely regenerate the entire protocol 
*/

//Define the hardware ID's for the device. 
#define ASUS_VENDOR_ID   0x1043
#define ASUS_PRODUCT_ID  0x82B2

typedef struct commandblockwrapper {
	uint8_t dCBWSignature[4]; 
	    /* = {'U','S','B','C'}; Signature that helps identify this data packet as a CBW. 
	    The signature field shall contain the value 43425355h (little endian), indicating a CBW.	*/

	
	uint32_t dCBWTag;				
	/*A Command Block Tag sent by the host. The device shall echo the contents of this field back to the
	host in the dCSWTag field of these associated CSW. The dCSWTag positively associates a CSW with the
	corresponding CBW.*/

	
	uint32_t dCBWDataTransferLength;	
	/*The number of bytes of data that the host expects to transfer on the Bulk-In or Bulk-Out endpoint (as
	indicated by the Direction bit) during the execution of this command. If this field is zero, the device and
	the host shall transfer no data between the CBW and the associated CSW, and the device shall ignore
	the value of the Direction bit in bmCBWFlags.*/

	
	uint8_t bmCBWFlags;			
	/*The bits of this field are defined as follows:
    Bit 7
	    Direction - the device shall ignore this bit if the dCBWDataTransferLength field is
	    zero, otherwise:
	    0 = Data-Out from host to the device,
	    1 = Data-In from the device to the host.
    Bit 6
	    Obsolete. The host shall set this bit to zero.
    Bits 5..0 Reserved - the host shall set these bits to zero.*/

	
	uint8_t bCBWLUN;
	    /*The device Logical Unit Number (LUN) to which the command block is being sent. For devices that
	    support multiple LUNs, the host shall place into this field the LUN to which this command block is
		addressed. Otherwise, the host shall set this field to zero.*/

	
	uint8_t bCBWCBLength;	   
	/*The valid length of the CBWCB in bytes. This defines the valid length of the command block. The
	only legal values are 1 through 16 (01h through 10h). All other values are reserved.*/

	
	uint8_t CBWCB[16];			
	/*The command block to be executed by the device. The device shall interpret the first bCBWCBLength
	bytes in this field as a command block as defined by the command set identified by bInterfaceSubClass.
	If the command set supported by the device uses command blocks of fewer than 16 (10h) bytes in
	length, the significant bytes shall be transferred first, beginning with the byte at offset 15 (Fh). The
	device shall ignore the content of the CBWCB field past the byte at offset (15 + bCBWCBLength - 1).*/

}__attribute__((__packed__));
#define bmCBWFlagsDataIn	0x80 //flag defined for when we are sending data to the host 

typedef struct commandstatuswrapper { 
		/*Signature that helps identify this data packet as a CSW. The signature field 
		shall contain the value 53425355h (little endian), indicating CSW.*/
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

/*****************************************************************************/
//initialization CBWCBs
static uint8_t CBWCBInit1[16] = {0x12, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, \
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
										

static uint8_t CBWCBInitSend[16] = {0xE6, 0x0B, 0x00, 0x00, 0x00, 0x09, 0x00, \
				0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static uint8_t InitSendData [0x09] = {0x0B, 0xB2, 0x08, 0x00, 0x09, 0x00, 0x00, 0x00, 0x01};
//we dont need this actually afaik (works fine w/o it) 
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

/******************************************************************************/
//The CBWCB command data for polling the display 
static uint8_t pollcommand[16] = {0xE7, 0x0A, 0x00, 0x00, 0x01, 0x00, 0x00, \
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
				