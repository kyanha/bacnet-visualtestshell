/* -------------------------------------------------------------------- */
/*              BACnet Protocol Interpreter Version 15                  */
/* -------------------------------------------------------------------- */
/*                           */
/* INITPI.C will be set up to branch to this routine when 802.3 packets */
/* are found with a DSAP of 0x82, 0x03(PTP), 0x04(MSTP); or ARCNET      */
/* packets are found with a Sytem Code of 0xCD.                         */
/*                   Steven T. Bushby, NIST                             */
/*                   H. Michael Newman, Cornell                         */
/*                   Joel Bender, Cornell                               */
/* -------------------------------------------------------------------- */
/* Application layer summary line added to show APDU and service type.  */
/* Corrected show_bac_unsigned to allow for 3-byte integers.            */
/* Added display of Device IDs to summary line for Who-Is and I-Am.     */
/* Added display of Invoke ID and, conditionally, SEG, MOR, SA, NAK,    */
/* SRV, and SEQ to the various PDU summary lines.                       */
/* -------------------------------------------------------------------- */

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pi.h"

namespace NetworkSniffer {

#include "bacproto.h"

#define max_confirmed_services 26
#define max_unconfirmed_services 9
#define max_property_id 123
#define FW "-27"
#define ARC 0

static char outstr[80]; /* buffer for output string manipulation */
static unsigned int lenexp; /* expected length of data following context tag */
static unsigned int lenact; /* actual length of data following context tag */

extern struct pi_data *pi_data_bacnet_IP;       /* Pointer to PI data structure */
extern struct pi_data *pi_data_bacnet_ETHERNET; /* Pointer to PI data structure */
extern struct pi_data *pi_data_bacnet_ARCNET;   /* Pointer to PI data structure */
extern struct pi_data *pi_data_bacnet_MSTP;     /* Pointer to PI data structure */
extern struct pi_data *pi_data_bacnet_PTP;      /* Pointer to PI data structure */
extern struct pi_data *pi_data_bacnet_BVLL;     /* Pointer to PI data structure */
extern struct pi_data *pi_data_bacnet_NL;       /* Pointer to PI data structure */
extern struct pi_data *pi_data_bacnet_AL;       /* Pointer to PI data structure */
extern struct pi_data *pi_data_Message;         /* Pointer to PI data structure */

struct pi_data *pi_data_current;     /* Current ptr for BAC_SHOW functions */

#include "bacfuncs.h"

/**************************************************************************/
int interp_bacnet_IP( char *header, int length)  /* IP interpreter */
/**************************************************************************/
{
   /* Summary line? */
   if (pi_data_bacnet_IP->do_sum)
	  interp_bacnet_BVLL( header + 6, length - 6 );

   /* Detail line? */
   if (pi_data_bacnet_IP->do_int) {
      pif_init (pi_data_bacnet_IP, header, length);
      pif_header (length, "IP Frame Detail");
      bac_show_bipaddr( "Source/Destination" );
	  pif_show_space();

	  interp_bacnet_BVLL( header + 6, length - 6 );
   }

   return length;
}

/**************************************************************************/
int interp_bacnet_ETHERNET( char *header, int length)  /* Ethernet interpreter */
/**************************************************************************/
{
   pif_init( pi_data_bacnet_ETHERNET, header, length );
   int len = (pif_get_byte(12) << 8) + pif_get_byte(13) - 3;

   /* Summary line? */
   if (pi_data_bacnet_ETHERNET->do_sum) {
	  if (pif_get_byte(17) == 0x01)
		  interp_bacnet_NL( header + 17, len );
	  else
		  interp_bacnet_BVLL( header + 17, len );
   } 

   /* Detail line? */
   if (pi_data_bacnet_ETHERNET->do_int) {
      pif_header( length, "Ethernet Frame Detail" );
      bac_show_enetaddr( "Destination" );
      bac_show_enetaddr( "Source" );

      bac_show_word_hl( "Length", "%u" );

	  bac_show_byte( "DSAP", "X'%02X'" );
	  bac_show_byte( "SSAP", "X'%02X'" );
	  bac_show_byte( "LLC Control", "X'%02X'" );

	  pif_show_space();

	  if (pif_get_byte(0) == 0x01)
		  interp_bacnet_NL( header + 17, len );
	  else
		  interp_bacnet_BVLL( header + 17, len );
   }

   return length;
}

/**************************************************************************/
int interp_bacnet_ARCNET( char *header, int length)  /* ARCNET interpreter */
/**************************************************************************/
{
   /* Summary line? */
   if (pi_data_bacnet_ARCNET->do_sum)
      sprintf( get_sum_line( pi_data_bacnet_ARCNET ), "ARCNET Frame" );

   /* Detail line? */
   if (pi_data_bacnet_ARCNET->do_int) {
      pif_init( pi_data_bacnet_ARCNET, header, length );
      pif_header( length, "ARCNET Frame Detail" );
      bac_show_byte( "Source", "%u" );
      bac_show_byte( "Destination", "%u" );

	  bac_show_byte( "BACnet SC", "X'%02X'" );
	  bac_show_byte( "DSAP", "X'%02X'" );
	  bac_show_byte( "SSAP", "X'%02X'" );

	  bac_show_byte( "LLC Control", "X'%02X'" );

	  pif_show_space();

	  if (pif_get_byte(0) == 0x01)
		  interp_bacnet_NL( header + 6, length - 6 );
	  else
		  interp_bacnet_BVLL( header + 6, length - 6 );
   }

   return length;
}

/**************************************************************************/
int interp_bacnet_MSTP( char *header, int length)  /* MS/TP interpreter */
/**************************************************************************/
{
   unsigned int wbuff;        /* 2 octet buffer */
   unsigned char *ptr;        /* general purpose pointer */
   int data_length;           /* length of data field not including CRC */
   unsigned char frame_type;  /* PTP frame type */
   boolean crc_test;          /* TRUE if CRC is correct */
   int i;
   unsigned char dataValue;   /* used in data CRC check */
   unsigned char headerCRC;   /* used in header CRC check */
   unsigned int crc;          /* used in header CRC check */
   unsigned int crcValue;     /* used in data CRC check */
   unsigned int crcLow;       /* used in data CRC check */
   char *mstp_header;         /* points to beginning of MS/TP preamble */

   /* Summary line? */
   if (pi_data_bacnet_MSTP->do_sum) {
      sprintf (get_sum_line (pi_data_bacnet_MSTP),
         "BACnet MS/TP Frame");
   }   /* End of Summary Line */

   /* Detail line? */
   if (pi_data_bacnet_MSTP->do_int) {
      pif_init (pi_data_bacnet_MSTP, header, length);
      pif_header (length, "BACnet MS/TP Frame Detail");
      mstp_header = header;

      /* ----- interpret MS/TP frame ----- */
      wbuff = pif_get_word_hl(0);
      if(wbuff == 0x55FF)
         pif_show_word_hl("Preamble                      = X'%04X'");
      else
         pif_show_word_hl("Error: Preamble (X'55FF') Expected. Found  X'%04X'");

      /* frame type field */
      frame_type = pif_get_byte(0);
      switch(frame_type){
         case 0x00: pif_show_byte("Token Frame                   = X'%02X'");
                    break;
         case 0x01: pif_show_byte("Poll For Master Frame         = X'%02X'");
                    break;
         case 0x02: pif_show_byte("Reply To Poll For Master Frame= X'%02X'");
                    break;
         case 0x03: pif_show_byte("Test Request Frame            = X'%02X'");
                    break;
         case 0x04: pif_show_byte("Test Response Frame           = X'%02X'");
                    break;
         case 0x05: pif_show_byte("Data Expecting Reply Frame    = X'%02X'");
                    break;
         case 0x06: pif_show_byte("Data Not Expecting Reply Frame= X'%02X'");
                    break;
         case 0x07: pif_show_byte("Reply Postponed Frame         = X'%02X'");
                    break;
         default:   pif_show_byte("Error: Unknown Frame Type     = X'%02X'");
         };

      /* address fields */
      pif_show_byte("Destination Address           = X'%02X'");
      pif_show_byte("Source Address                = X'%02X'");


      /* length field */
      data_length = pif_get_word_hl(0);

      switch(frame_type){
         /* frames with no data */
         case 0x00: case 0x01: case 0x02: case 0x07:
                    if(data_length == 0)
                       bac_show_word_hl("Data Length","%u");
                     else
                       bac_show_word_hl("Error: Invalid Data Length","%u");
                    break;
         /* frames with data */
         case 0x03: case 0x04: case 0x05: case 0x06:
                    if( (((data_length + 10) == length) || ((data_length + 11) == length))
                       && (data_length <= 510) )
                       /* allow for optional pad octet but restrict to 510 octets of data */
                       bac_show_word_hl("Data Length","%u");
                    else
                       bac_show_word_hl("Error: Invalid Data Length","%u");
                    break;
         default:   /* unknown frame case */
                    if(data_length == 0){
                       bac_show_word_hl("Data Length","%u");
                       break;
                       };
                    if( (((data_length + 10) == length) || ((data_length + 11) == length))
                       && (data_length <= 510) )
                       /* allow for optional pad octet but restrict to 510 octets of data */
                       bac_show_word_hl("Data Length","%u");
                    else
                       bac_show_word_hl("Error: Invalid Data Length","%u");
                    break;

         };  /* end of frame_type switch */

      /* header CRC field */
      /* -- check the CRC -- */
      headerCRC = 0xff;
      ptr = (unsigned char *)(mstp_header + 2);  /* point to beginning frame type field */
      for(i=0; i<6; i++){  /* process frame type, address, length, and CRC */
         dataValue = *ptr++;
         crc = headerCRC ^ dataValue;
         crc = crc ^ (crc<<1) ^ (crc<<2) ^ (crc<<3)
                   ^ (crc <<4) ^ (crc<<5) ^ (crc<<6) ^ (crc <<7);
         headerCRC = (crc & 0xfe) ^ ((crc>>8) & 1);
      }; /* end of for loop */
      crc_test = (headerCRC == 0x55);
      if(crc_test)
         bac_show_byte("Header CRC Verified","%02X");
      else
         bac_show_byte("Error: Header CRC Failure","%02X");


      /* data field */
      if(data_length > 0){
         switch(frame_type){
            /* frames with no data */
            case 0x00: case 0x01: case 0x02: case 0x07:
                       bac_show_nbytes(data_length,"Error: No data expected for this frame type.");
                       break;
            case 0x03: case 0x04: /* Test Request or Test Response Frame */
                       bac_show_nbytes(data_length,"Test Data");
                       break;
            case 0x05: case 0x06: /* call network layer interpreter */
                       pif_show_space();
                       interp_bacnet_NL( header+=8, length - 10 );
                       break;
            };  /* end of frame_type switch */

         /* data CRC field */
         /* -- check the CRC -- */
         crcValue = 0xffff;
         ptr = (unsigned char *)(mstp_header + 8);  /* point to beginning data field */
         for(i=0; i< (data_length + 2); i++){  /* process all data incl CRC */
            dataValue = *ptr++;

            crcLow = (crcValue & 0xff) ^ dataValue;   /* XOR C7..C0 with D7..D0 */
            /* Exclusive OR the terms in the table (top down) */
            crcValue = (crcValue >> 8) ^ (crcLow << 8)   ^ (crcLow << 3)
                                   ^ (crcLow << 12)  ^ (crcLow >> 4)
                                   ^ (crcLow & 0x0f) ^((crcLow & 0x0f) << 7);
            }; /* end of for loop */
         crc_test = (crcValue == 0xf0b8);
         if(crc_test)
            bac_show_word_hl("Data CRC Verified","%04X");
         else
            bac_show_word_hl("Error: Data CRC Failure","%04X");
         }; /* end of if(data_length>0) */
      if(pif_offset < pif_end_offset) bac_show_byte("Padding Octet","%02X");
      };   /* End of Detail Lines */
   return length;
}

/**************************************************************************/
int interp_bacnet_PTP( char *header, int length)  /* PTP interpreter */
/**************************************************************************/
{
   unsigned char buff;        /* 1 octet buffer for manipulation */
   unsigned int wbuff;        /* 2 octet buffer */
   unsigned char *ptr;        /* general purpose pointer */
   unsigned char *ptr1;       /* general purpose pointer */
   char trigger[8];           /* BACnet trigger string */
   char result_str[8];        /* data string */
   int data_length;           /* length of data field not including CRC */
   int len;                   /* data length field size */
   int len2;                  /* stuffed data field (or data CRC) size */
   unsigned char frame_type;  /* PTP frame type */
   boolean crc_test;          /* TRUE if CRC is correct */
   boolean byte1_stuffed;     /* TRUE if X'10' code found */
   boolean byte2_stuffed;     /* TRUE if X'10' code found */
   int i,j;                   /* loop index */
   unsigned char dataValue;   /* used in data CRC check */
   unsigned char headerCRC;   /* used in header CRC check */
   unsigned int crc;          /* used in header CRC check */
   unsigned int crcValue;     /* used in data CRC check */
   unsigned int crcLow;       /* used in data CRC check */
   char *ptp_header;          /* points to beginning of PTP preamble */

   strcpy(trigger, "BACnet");  /* initialize trigger string */
   /* Summary line? */
   if (pi_data_bacnet_PTP->do_sum) {
      sprintf (get_sum_line (pi_data_bacnet_PTP),
         "BACnet PTP Frame");
   }   /* End of Summary Line */

   /* Detail line? */
   if (pi_data_bacnet_PTP->do_int) {
      pif_init (pi_data_bacnet_PTP, header, length);
      pif_header (length, "BACnet PTP Frame Detail");
      ptp_header = header;

      /* ----- check for BACnet trigger sequence ----- */
      if((length == 7) && (pif_get_byte(6) == 0x0d)){
         pif_get_ascii(0, 6, result_str);
         if(strcmp(result_str, trigger) == 0)  /* trigger found */
            bac_show_nbytes(7,"`BACnet<CR>' Trigger Sequence Found");
         }
      else{ /* not a trigger sequence */
         /* ----- interpret PTP frame ----- */
         wbuff = pif_get_word_hl(0);
         if(wbuff == 0x55FF)
            pif_show_word_hl("Preamble                        = X'%04X'");
         else
            pif_show_word_hl("Error: PTP Preamble (X'55FF') Expected.  X'%04X'");

         /* frame type field */
         frame_type = pif_get_byte(0);
         switch(frame_type){
            case 0x00: pif_show_byte("Heartbeat (XOFF) Frame          = X'%02X'");
                       break;
            case 0x01: pif_show_byte("Heartbeat (XON) Frame           = X'%02X'");
                       break;
            case 0x02: pif_show_byte("Data Request 0 Frame            = X'%02X'");
                       break;
            case 0x03: pif_show_byte("Data Request 1 Frame            = X'%02X'");
                       break;
            case 0x04: pif_show_byte("Data Response 0 (XOFF) Frame    = X'%02X'");
                       break;
            case 0x05: pif_show_byte("Data Response 1 (XOFF) Frame    = X'%02X'");
                       break;
            case 0x06: pif_show_byte("Data Response 0 (XON) Frame     = X'%02X'");
                       break;
            case 0x07: pif_show_byte("Data Response 1 (XON) Frame     = X'%02X'");
                       break;
            case 0x08: pif_show_byte("Data Reject 0 Frame             = X'%02X'");
                       break;
            case 0x09: pif_show_byte("Data Reject 1 Frame             = X'%02X'");
                       break;
            case 0x0A: pif_show_byte("Connect Request Frame           = X'%02X'");
                       break;
            case 0x0B: pif_show_byte("Connect Response Frame          = X'%02X'");
                       break;
            case 0x0C: pif_show_byte("Disconnect Request Frame        = X'%02X'");
                       break;
            case 0x0D: pif_show_byte("Disconnect Response Frame       = X'%02X'");
                       break;
            case 0x0E: pif_show_byte("Test Request Frame              = X'%02X'");
                       break;
            case 0x0F: pif_show_byte("Test Response Frame             = X'%02X'");
                       break;
            default:   pif_show_byte("Error: Invalid Frame Type       = X'%02X'");
            };

         /* length field -- check for stuffed bytes */
         if(pif_get_byte(0) == 0x10){
            byte1_stuffed = TRUE;
            if(pif_get_byte(2) == 0x10)
               byte2_stuffed = TRUE;
            else
               byte2_stuffed = FALSE;
            }
         else{
            byte1_stuffed = FALSE;
            if(pif_get_byte(1) == 0x10)
               byte2_stuffed = TRUE;
            else
               byte2_stuffed = FALSE;
            };

         if(byte1_stuffed && byte2_stuffed) {
            data_length = ((pif_get_byte(1) & 0x7f) *16) +
                          (pif_get_byte(3) & 0x7f);
            len = 4;
            };

         if(byte1_stuffed && (!byte2_stuffed)){
            data_length = ((pif_get_byte(1) & 0x7f) * 16) +
                          pif_get_byte(2);
            len = 3;
            };

         if((!byte1_stuffed) && byte2_stuffed){
            data_length = pif_get_byte(0) * 16 +
                          (pif_get_byte(2) & 0x7f);
            len = 3;
            };
         if((!byte1_stuffed) && (!byte2_stuffed)){
            len = 2;
            data_length = pif_get_word_hl(0);
            };

         switch(frame_type){
            /* frames with no data */
            case 0x00: case 0x01: case 0x04: case 0x05: case 0x06: case 0x07:
            case 0x08: case 0x09: case 0x0a: case 0x0d:
                       if(data_length == 0){
                          sprintf(outstr, "Data Length                 = %u", data_length);
                          bac_show_nbytes(len, outstr);
                          }
                        else{
                          sprintf(outstr, "Error: Invalid Data Length  = %u", data_length);
                          bac_show_nbytes(len, outstr);
                          };
                       break;
            /* frames with data */
            case 0x02: case 0x03: case 0x0B: 
            case 0x0C: case 0x0E: case 0x0F: 
                       if((data_length + 8) <= length){ /* must allow byte stuffing in data */
                          sprintf(outstr, "Data Length                 = %u", data_length);
                          bac_show_nbytes(len, outstr);
                          }
                        else{
                          sprintf(outstr, "Error: Invalid Data Length  = %u", data_length);
                          bac_show_nbytes(len, outstr);
                          };
                       break;
            };  /* end of frame_type switch */

         /* header CRC field */
         /* -- check the CRC -- */
         headerCRC = 0xff;
         ptr = (unsigned char *)(ptp_header + 2);  /* point to beginning frame type field */
         for(i=0; i<4; i++){  /* process frame type, length, and CRC */
            dataValue = *ptr++;
            if(dataValue == 0x10){  /* decode next octct */
               dataValue = *ptr++;
               dataValue = dataValue & 0x7f;
               };
            crc = headerCRC ^ dataValue;
            crc = crc ^ (crc<<1) ^ (crc<<2) ^ (crc<<3)
                      ^ (crc <<4) ^ (crc<<5) ^ (crc<<6) ^ (crc <<7);
            headerCRC = (crc & 0xfe) ^ ((crc>>8) & 1);
         }; /* end of for loop */
         crc_test = (headerCRC == 0x55);
         if(crc_test){
            if(pif_get_byte(0) == 0x10)
               bac_show_word_hl("Header CRC Verified", "%04X");
            else
               bac_show_byte("Header CRC Verified","%02X");
            }
         else{
            if(pif_get_byte(0) == 0x10)
               bac_show_word_hl("Header CRC Failure", "%04X");
            else
               bac_show_byte("Error: Header CRC Failure","%02X");
            };


            /*  check the data CRC before decoding higher layers */
            crcValue = 0xffff;
            ptr = (unsigned char *)(ptp_header + len + 4);  /* point to beginning data field */
            for(i=0; i< (data_length + 2); i++){  /* process all data incl CRC */
               dataValue = *ptr++;
               if(dataValue == 0x10){  /* decode next octct */
                  dataValue = *ptr++;
                  dataValue = dataValue & 0x7f;
                  };

               crcLow = (crcValue & 0xff) ^ dataValue;   /* XOR C7..C0 with D7..D0 */
               /* Exclusive OR the terms in the table (top down) */
               crcValue = (crcValue >> 8) ^ (crcLow << 8)   ^ (crcLow << 3)
                                      ^ (crcLow << 12)  ^ (crcLow >> 4)
                                      ^ (crcLow & 0x0f) ^((crcLow & 0x0f) << 7);
               }; /* end of for loop */
            crc_test = (crcValue == 0xf0b8);


         /* data field */
         if(data_length > 0){
            switch(frame_type){
               case 0x00: case 0x01: case 0x04: case 0x05: case 0x06: case 0x07:
               case 0x08: case 0x09: case 0x0a: case 0x0d: 
                          bac_show_nbytes(data_length,"Error: No data expected for this frame type.");
                          break;
               case 0x02: case 0x03: /* call network layer interpreter */
                          /* decode AL message before passing it on */
                          ptr = (unsigned char *)(ptp_header + len + 4);  /* point to beginning data field */
                          ptr1 = ptr;
                          len2 = 0;
                          for(i=0; i<data_length; i++){
                            if(*ptr1 == 0x10){
                              len2++;
                              ptr1++;
                              *ptr = *ptr1 & 0x7f;
                              ptr++;
                              ptr1++;
                              }
                            else
                              *ptr++ = *ptr1++;
                            };
                          pif_show_space();
                          interp_bacnet_NL (ptp_header+len+4, data_length);
                          if(len2 > 0){
                            sprintf(outstr, "%d Encoded data octet(s) were found", len2);
                            bac_show_nbytes(len2,outstr);
                            };
                          break;
               case 0x0B: /* Connect Response Frame */
                          len2 = data_length;
                          j = 0;
                          for(i=0; i<data_length; i++){
                            /* account for stuffed data bytes */
                            if(pif_get_byte(j) == 0x10){
                               j++; /* increment to pass a stuffed X'10' */
                               len2++;
                               };
                            j++;
                            };
                          bac_show_nbytes(len2,"Password");
                          break;
               case 0x0C: /* Disconnect Request Frame  */
                          buff = pif_get_byte(0);
                          pif_show_space();
                          pif_show_ascii(0,"Reason for Disconnect");
                          switch(buff){
                           case 0x00: bac_show_byte("No more data to be transmitted","%u");
                                      break;
                           case 0x01: bac_show_byte("The peer process is being preempted","%u");
                                      break;
                           case 0x02: bac_show_byte("The received password is invalid","%u");
                                      break;
                           case 0x03: bac_show_byte("Other","%u");
                                      break;
                           default:   bac_show_byte("Error: Invalid Disconnect Reason.", "%u");
                           };
                          break;
               case 0x0E: case 0x0F: /* Test Request or Test Response Frame */
                          len2 = data_length;
                          j = 0;
                          for(i=0; i<data_length; i++){
                            /* account for stuffed data bytes */
                            if(pif_get_byte(j) == 0x10){
                               j++; /* increment to pass a stuffed X'10' */
                               len2++;
                               };
                            j++;
                            };
                          bac_show_nbytes(len2,"Test Data");
                          break;
               };  /* end of frame_type switch */

            /* data CRC field (CRC was tested before decoding data) */
            if(crc_test){ /* CRC is good */
               if((pif_get_byte(0) == 0x10) && (pif_get_byte(2) == 0x10)){
                  crcValue = ((pif_get_byte(1) & 0x7f)*16) +
                           (pif_get_byte(3) & 0x7f);
                  sprintf(outstr,  "Data CRC Verified           = %04X", crcValue);
                  len2 = 4;
                  };
               if((pif_get_byte(0) == 0x10) && (pif_get_byte(2) != 0x10)){
                  crcValue = ((pif_get_byte(1) & 0x7f)*16) +
                           pif_get_byte(2);
                  sprintf(outstr,  "Data CRC Verified           = %04X",crcValue);
                  len2 = 3;
                  };
               if((pif_get_byte(0) != 0x10) && (pif_get_byte(1) == 0x10)){
                  crcValue = (pif_get_byte(0) * 16) +
                           (pif_get_byte(2) & 0x7f);
                  sprintf(outstr,  "Data CRC Verified           = %04X",crcValue);
                  len2 = 3;
                  };
               if((pif_get_byte(0) != 0x10) && (pif_get_byte(1) != 0x10)){
                  sprintf(outstr,  "Data CRC Verified           = %04X",
                          pif_get_word_hl(0));
                  len2 = 2;
                  };
               bac_show_nbytes(len2, outstr);
               }
            else{ /* CRC is bad */
               if((pif_get_byte(0) == 0x10) && (pif_get_byte(2) == 0x10)){
                  crcValue = ((pif_get_byte(1) & 0x7f)*16) +
                           (pif_get_byte(3) & 0x7f);
                  sprintf(outstr,  "Error: Data CRC Failure     = %04X",crcValue);
                  len2 = 4;
                  };
               if((pif_get_byte(0) == 0x10) && (pif_get_byte(2) != 0x10)){
                  crcValue = ((pif_get_byte(1) & 0x7f)*16) +
                           pif_get_byte(2);
                  sprintf(outstr,  "Error: Data CRC Failure     = %04X",crcValue);
                  len2 = 3;
                  };
               if((pif_get_byte(0) != 0x10) && (pif_get_byte(2) == 0x10)){
                  crcValue = (pif_get_byte(0) * 16) +
                           (pif_get_byte(2) & 0x7f);
                  sprintf(outstr,  "Error: Data CRC Failure     = %04X",crcValue);
                  len2 = 3;
                  };
               if((pif_get_byte(0) != 0x10) && (pif_get_byte(2) != 0x10)){
                  sprintf(outstr,  "Error: Data CRC Failure     = %04X",
                          pif_get_word_hl(0));
                  len2 = 2;
                  };
               bac_show_nbytes(len2, outstr);
               };
            }; /* end of if(data_length>0) */
         };  /* end of else not a trigger sequence */
      };   /* End of Detail Lines */
   return length;
}

/**************************************************************************/
int interp_bacnet_BVLL( char *header, int length)  /* BVLL interpreter */
/**************************************************************************/
{
   pi_data_current = pi_data_bacnet_BVLL;
   
   if (pi_data_bacnet_BVLL->do_sum) {
      pif_init( pi_data_bacnet_BVLL, header, length );
      
      switch (pif_get_byte(0)) {
         case 0x01:
            interp_bacnet_NL( header, length );
            break;
         case 0x81:
            switch (pif_get_byte(1)) {
               case 0x04:		// Forwarded-NPDU
                  interp_bacnet_NL( header + 10, length - 10 );
                  break;
               case 0x0A:		// Original-Unicast-NPDU
               case 0x0B:		// Original-Broadcast-NPDU
                  interp_bacnet_NL( header + 4, length - 4 );
                  break;
               default:
                  strcpy( get_sum_line(pi_data_bacnet_BVLL), BVLL_Function[pif_get_byte(1)] );
			}
            break;
      }
   }
   
   if (pi_data_bacnet_BVLL->do_int) {
      pif_init( pi_data_bacnet_BVLL, header, length );
      pif_header( length, "BACnet Virtual Link Layer Detail" );

	  switch (pif_get_byte(0)) {
         case 0x01:
            pif_show_ascii( 0, "Empty BVLL" );
            pif_show_space();
            return interp_bacnet_NL( header, length );
            break;
         case 0x81:
            show_str_eq_str("BVLC Type","BACnet/IP",1);
            pif_offset += 1;
            switch (pif_get_byte(0)) {
               case 0x00:
                  pif_show_byte("BVLC Function               = %u  BVLC-Result");
                  bac_show_word_hl("BVLC Length","%u");
                  bac_show_word_hl("Result Code","%u");
                  pif_show_space();
                  break;
               case 0x01:
                  pif_show_byte("BVLC Function               = %u  Write-Broadcast-Distribution-Table");
                  bac_show_word_hl("BVLC Length","%u");
                  while (pif_offset < pif_end_offset) {
                     pif_show_space();
                     bac_show_bipaddr( "BDT Entry" );
                     bac_show_long_hl( "Distribution Mask", "X'%08X'" );
                  }
                  pif_show_space();
                  break;
               case 0x02:
                  pif_show_byte("BVLC Function               = %u  Read-Broadcast-Distribution-Table");
                  bac_show_word_hl("BVLC Length","%u");
                  pif_show_space();
                  break;
               case 0x03:
                  pif_show_byte("BVLC Function               = %u  Read-Broadcast-Distribution-Table-Ack");
                  bac_show_word_hl("BVLC Length","%u");
                  while (pif_offset < pif_end_offset) {
                     pif_show_space();
                     bac_show_bipaddr( "BDT Entry" );
                     bac_show_long_hl( "Distribution Mask", "X'%08X'" );
                  }
                  pif_show_space();
                  break;
               case 0x04:
                  pif_show_byte("BVLC Function               = %u  Forwarded-NPDU");
                  bac_show_word_hl("BVLC Length","%u");
                  pif_show_space();
                  bac_show_bipaddr("Originating Device");
                  pif_show_space();
                  interp_bacnet_NL( header + pif_offset, length - pif_offset );
                  break;
               case 0x05:
                  pif_show_byte("BVLC Function               = %u  Register-Foreign-Device");
                  bac_show_word_hl("BVLC Length","%u");
                  pif_show_space();
                  bac_show_word_hl("Time-To-Live","%u");
                  pif_show_space();
                  break;
               case 0x06:
                  pif_show_byte("BVLC Function               = %u  Read-Foreign-Device-Table");
                  bac_show_word_hl("BVLC Length","%u");
                  pif_show_space();
                  break;
               case 0x07:
                  pif_show_byte("BVLC Function               = %u  Read-Foreign-Device-Table-Ack");
                  bac_show_word_hl("BVLC Length","%u");
                  while (pif_offset < pif_end_offset) {
                     pif_show_space();
                     bac_show_bipaddr( "FDT Entry" );
                     bac_show_word_hl( "Time-To-Live", "%u" );
                     bac_show_word_hl( "Time-Remaining", "%u" );
                  }
                  pif_show_space();
                  break;
               case 0x08:
                  pif_show_byte("BVLC Function               = %u  Delete-Foreign-Device-Table-Entry");
                  bac_show_word_hl("BVLC Length","%u");
                  pif_show_space();
                  bac_show_bipaddr( "FDT Entry" );
                  pif_show_space();
                  break;
               case 0x09:
                  pif_show_byte("BVLC Function               = %u  Distribute-Broadcast-To-Network");
                  bac_show_word_hl("BVLC Length","%u");
                  pif_show_space();
                  interp_bacnet_NL( header + pif_offset, length - pif_offset );
                  break;
               case 0x0A:
                  pif_show_byte("BVLC Function               = %u  Original-Unicast-NPDU");
                  bac_show_word_hl("BVLC Length","%u");
                  pif_show_space();
                  interp_bacnet_NL( header + pif_offset, length - pif_offset );
                  break;
               case 0x0B:
                  pif_show_byte("BVLC Function               = %u  Original-Broadcast-NPDU");
                  bac_show_word_hl("BVLC Length","%u");
                  pif_show_space();
                  interp_bacnet_NL( header + pif_offset, length - pif_offset );
                  break;
            }
            break;
      }
   }
   
   return length;
}

/**************************************************************************/
int interp_bacnet_NL( char *header, int length)  /* Network Layer interpreter */
/**************************************************************************/
{
   char *control;           /* pointer to the control octet */
   unsigned char buff;      /* 1 octet buffer for manipulation */
   unsigned short wbuff;    /* 2 octet buffer */
   unsigned char APDUmask;  /* APDU vs Network Layer flag mask */
   unsigned char RESV1mask; /* Reserved bit mask */
   unsigned char DESTmask;  /* DNET, DLEN, DADR, and Hop Count flag mask */
   unsigned char RESV2mask; /* Reserved bit mask */
   unsigned char SOURCEmask;/* SNET, SLEN, AND SADR flag mask */
   unsigned char DERmask;   /* Data Expecting Reply mask */
   int mac_length;          /* MAC address length in octets */
   int npdu_length;         /* NPDU length in octets */
   int port_id;             /* port ID field */
   int port_info_length;    /* port information length in octets */
   const char *name;		/* translated name */
   char namebuff[80];		/* buffer to print translated name */

   pi_data_current = pi_data_bacnet_NL;
   npdu_length = 2;
   control = header + 1;  /* initialize pointer to control octet */
   /* Summary line? */
   if (pi_data_bacnet_NL->do_sum) {
       /* Figure out length of NPCI */
       pif_init (pi_data_bacnet_NL, header, length);
       pif_skip(1); /* Point to control octet */
       buff = pif_get_byte(0);
       DESTmask = (buff & 0x20 )>>5;   /* mask for DNET, DLEN, DADR, & Hop Count present switch */
       SOURCEmask = (buff & 0x08)>>3;  /* mask for SNET, SLEN, and SADR present switch */
       if (DESTmask == 1) {
          npdu_length += 4; /* DNET, DLEN, DADR, & Hop Count ARE present */  
          pif_skip(3);      /* Point to DLEN */      
          npdu_length += pif_get_byte(0); /* Add length of DADR and skip over */
          pif_skip(pif_get_byte(0));
       }
       if (SOURCEmask == 1) {
          npdu_length += 3; /* SNET, SLEN, and SADR ARE present */
          pif_skip(3);      /* Point to SLEN */
          npdu_length += pif_get_byte(0);
          pif_skip(pif_get_byte(0));
       }
      if (*control&0x80) {
         if (DESTmask == 1) /* Must skip over hop count too! */
            pif_skip(2);
         else
            pif_skip(1);

		 int	nlMsgID = pif_get_byte(0);
		 if ((nlMsgID < 0) || (nlMsgID > (sizeof(NL_msgs)/sizeof(char*))))
		    strcpy( get_sum_line(pi_data_bacnet_NL), "Proprietary Network Layer Message" );
	     else
		    strcpy( get_sum_line(pi_data_bacnet_NL), NL_msgs[pif_get_byte(0)] );
/*
         sprintf (get_sum_line (pi_data_bacnet_NL),
            "BACnet NL (%s)",NL_msgs[pif_get_byte(0)]);
*/
      }
      else {
/*
         sprintf (get_sum_line (pi_data_bacnet_NL),
            "BACnet NL (APDU)");
*/
         return npdu_length + interp_bacnet_AL( header + npdu_length, length - npdu_length );
      }
   }   /* End of Summary Line */

   /* Detail line? */
   if (pi_data_bacnet_NL->do_int) {
      pif_init (pi_data_bacnet_NL, header, length);
      pif_header (length, "BACnet Network Layer Detail");

      pif_show_byte("Protocol Version Number = %d");

      /* --- display the network control octet detail --- */
      buff = pif_get_byte(0);
      APDUmask = (buff & 0x80);       /* mask  APDU/Network control bit (7) */
      RESV1mask = (buff & 0x40)>>6;       /* reserved mask */
      DESTmask = (buff & 0x20 )>>5;   /* mask for DNET, DLEN, DADR, & Hop Count present switch */
      RESV2mask = (buff & 0x10)>>4;   /* reserved mask */
      SOURCEmask = (buff & 0x08)>>3;  /* mask for SNET, SLEN, and SADR present switch */
      DERmask = (buff & 0x04)>>2;     /* data expecting reply mask */

      /*  ----- show control octet  ------  */
      sprintf(outstr,"%"FW"s = X'%%02X'","Network Control Octet");
      bac_show_flag(outstr,0xFF);
      pif_show_flagbit(0x80,"BACnet Network Control Message", "BACnet APDU");
      pif_show_flagbit(0x40,"Reserved","Reserved");
      pif_show_flagbit(0x20,"DNET,DLEN,DADR & Hop Count Present","DNET,DLEN,DADR & Hop Count Omitted");
      pif_show_flagbit(0x10,"Reserved","Reserved");
      pif_show_flagbit(0x08,"SNET,SLEN, & SADR Present","SNET,SLEN, & SADR Omitted");
      pif_show_flagbit(0x04,"Data Expecting Reply","No Reply Expected");
      pif_show_flagmask(0x03, 0x03, "Life Safety Message");
      pif_show_flagmask(0x03, 0x02, "Critical Equipment Message");
      pif_show_flagmask(0x03, 0x01, "Urgent Message");
      pif_show_flagmask(0x03, 0x00, "Normal Message");

      /*  ----- show DNET, DLEN and DADR if present ----- */
      if (DESTmask == 1) {
         wbuff = pif_get_word_hl(0);
         if(wbuff == 65535) {
		   if ((name = LookupName( 65535, 0, 0 )) != 0) {
			  sprintf( namebuff, "Global Broadcast DNET          = X'%%04X', %s", name );
              pif_show_word_hl(namebuff);
		   } else
              pif_show_word_hl("Global Broadcast DNET          = X'%04X'");
         } else
           pif_show_word_hl("Destination Network Number     = %u");

         mac_length = pif_get_byte(0);
         pif_show_byte("Destination MAC address length = %u");

         if (wbuff != 65535)
            name = LookupName( wbuff, (const unsigned char *)msg_origin + pif_offset, mac_length );
         else
            name = 0;

         if(mac_length > 0){
             pif_show_nbytes_hex
             ("Destination MAC address        = X'%s'",mac_length);
             }
         else {  /* Broadcast MAC address (not shown in PDU) */
             pif_show_ascii(0,"Broadcast MAC address implied");
             }

		 if (name) pif_append_ascii( ", %s", name );
         }

      /*  ----- show SNET, SLEN and SADR if present ----- */
      if (SOURCEmask == 1) {
         wbuff = pif_get_word_hl(0);
         pif_show_word_hl("Source Network Number          = %u");

         mac_length = pif_get_byte(0);
         pif_show_byte("Source MAC address length      = %u");

         name = LookupName( wbuff, (const unsigned char *)msg_origin + pif_offset, mac_length );

         pif_show_nbytes_hex
               ("Source MAC address             = X'%s'", mac_length);
         if(mac_length <= 0)
            pif_show_ascii(0,"Invalid MAC address length!");

		 if (name) pif_append_ascii( ", %s", name );
         }

      /*  ----- show Hop Count if DNET present ----- */
      if (DESTmask == 1) pif_show_byte("Hop Count                   = %d");

      /*  -----  APDU or network layer message ? ----  */
      if(APDUmask == 0) {  /* BACnet APDU in message, call AL interpreter */
         pif_show_space();
         npdu_length = pif_offset;
         return npdu_length
            + interp_bacnet_AL( header + npdu_length, length - npdu_length );
         }  /* end of APDU present case */

      else {   /* BACnet network control message */

        /*  ----- show message type ----- */

        buff = pif_get_byte(0);
        if(buff >= 0x80){
          pif_show_byte("Proprietary Message Type    = %u");
          pif_show_word_hl("Vendor ID                   = %d");
          }
        else{
          switch (buff) {   /* show appropriate NL interpretation */
            case 0x00:  /* Who-Is-Router-To-Network */
                        pif_show_byte("Message Type                = %u  Who-Is-Router-To-Network");
                        if(pif_offset < pif_end_offset) {   /* network address present */
                           sprintf(outstr,"%"FW"s = %%u","Network Number");
                           pif_show_word_hl(outstr);
                           }
                        break;
            case 0x01:  /*  I-Am-Router-To-Network  */
                        pif_show_byte("Message Type                = %u  I-Am-Router-To-Network");
                        while(pif_offset < pif_end_offset) { /* display network numbers */
                           sprintf(outstr,"%"FW"s = %%u","Network Number");
                           pif_show_word_hl(outstr);
                           }
                        break;
            case 0x02:  /*  I-Could-Be-Router-To-Network  */
                        pif_show_byte("Message Type                = %u  I-Could-Be-Router-To-Network");
                          sprintf(outstr,"%"FW"s = %%u","Network Number");
                        pif_show_word_hl(outstr);
                        pif_show_byte("Performance Index           = %u");
                        break;
            case 0x03:  /* Reject-Message-To-Network */
                        pif_show_byte("Message Type                = %u  Reject-Message-To-Network");
                        sprintf(outstr,"%"FW"s = X'%%02X'","Reject Reason");
                        bac_show_flag(outstr,0xFF);
                        pif_show_flagmask(0x07, 0x00, "Other");
                        pif_show_flagmask(0x07, 0x01, "Unknown DNET");
                        pif_show_flagmask(0x07, 0x02, "Router Busy");
                        pif_show_flagmask(0x07, 0x03, "Unknown Message Type");
                        pif_show_flagmask(0x07, 0x04, "Msg too long for this DNET");
                        pif_show_word_hl("Network Number              = %u");
                        break;
            case 0x04:  /* Router-Busy-To-Network */
                        pif_show_byte("Message Type                = %u  Router-Busy-To-Network");
                        if (length > 3) {   /* DNET included */
                          while(pif_offset < pif_end_offset) { /* display net numbers */
                             sprintf(outstr,"%"FW"s = %%u","Network Number");
                             pif_show_word_hl(outstr);
                             }
                          }
                        else
                           sprintf(pif_line(0),"Busy to all networks");
                        break;
            case 0x05:  /* Router-Available-To-Network */
                        pif_show_byte("Message Type                = %u  Router-Available-To-Network");
                        if (length > 3) {   /* DNET included */
                          while(pif_offset < pif_end_offset) { /* display net numbers */
                             sprintf(outstr,"%"FW"s = %%u","Network Number");
                             pif_show_word_hl(outstr);
                             }
                           }
                        else
                           sprintf(pif_line(0), "Available to all networks");
                        break;
            case 0x06:  /* Initialize-Routing-Table */
                        pif_show_byte("Message Type                = %u  Initialize-Routing-Table");
                          pif_show_byte("Number of Ports             = %u");
                        if (pif_offset == pif_end_offset) {
                           pif_show_space();
                           pif_show_ascii(0, "Initialize-Routing-Table Query");
                           }
                        else
                        while(pif_offset < pif_end_offset) { /* process data */
                           sprintf(outstr,"%"FW"s = %%u","Connected DNET");
                           pif_show_word_hl(outstr);
                           port_id = pif_get_byte(0);
                           if(port_id == 0)
                              pif_show_byte("Port ID Number              = %u (remove entry)");
                           else
                              pif_show_byte("Port ID Number              = %u");
                           port_info_length = pif_get_byte(0);
                           pif_show_byte("Port Info Length            = %u");
                           bac_show_nbytes(port_info_length,"Port Information");
                           }
                        break;
            case 0x07:  /* Initialize-Routing-Table-Ack */
                        pif_show_byte("Message Type                = %u  Initialize-Routing-Table-Ack");
                        if (pif_offset == pif_end_offset) {
                           pif_show_space();
                           pif_show_ascii(0, "Simple Initialize-Routing-Table-Ack without data");
                           }
                        else {
                           pif_show_byte("Number of Ports             = %u");
                           while(pif_offset < pif_end_offset) { /* process data */
                              sprintf(outstr,"%"FW"s = %%u","Connected DNET");
                              pif_show_word_hl(outstr);
                              pif_show_byte("Port ID Number              = %u");
                              port_info_length = pif_get_byte(0);
                              pif_show_byte("Port Info Length            = %u");
                              bac_show_nbytes(port_info_length,"Port Information");
                              }
                           }
                        break;
            case 0x08:  /* Establish-Connection-To-Network */
                        pif_show_byte("Message Type                = %u  Establish-Connection-To-Network");
                          sprintf(outstr,"%"FW"s = %%u","Network Number");
                        pif_show_word_hl(outstr);
                        if(pif_get_byte(0) == 0)
                           pif_show_byte("Permanent Connection");
                        else
                           pif_show_byte("Termination Time         = %u seconds");
                        break;
            case 0x09:  /* Disconnect-Connection-To-Network */
                        pif_show_byte("Message Type                = %u  Disconnect-Connection-To-Network");
                          sprintf(outstr,"%"FW"s = %%u","Network Number");
                        pif_show_word_hl(outstr);
                        break;
            default:    pif_show_byte("Invalid Message Type        = %u");
            }  /* End of switch */
          } /* End of if proprietary message type else branch */
          
          pif_show_space();
		}  /* End of else network control message*/
      }   /* End of Detail Lines */
   return length;
}

/**************************************************************************/
int interp_bacnet_AL( char *header, int length )  /* Application Layer interpreter */
/**************************************************************************/
  /* This function is the Application Layer interpreter for BACnet  */
{
   unsigned char buff;   /* 1 octet buffer for manipulation */
   unsigned char x,ID,SEG,MOR,SA,SEQ,NAK,SRV;
   unsigned char pdu_type;
   unsigned char service_choice;

   /* Summary line? */
   if (pi_data_bacnet_AL->do_sum) {
      pif_init (pi_data_bacnet_AL, header, length);
      pdu_type = pif_get_byte(0)>>4; /*Get APDU type */
      x = (pif_get_byte(0)&0x08)>>3; /* x = 1 implies header is 2 bytes longer */
   /* Set up to do cool stuff in summary based on PDU type and service choice */
      switch (pdu_type) {
        case 0: /* Confirmed service request */
                service_choice = pif_get_byte(3+x*2);
                ID = pif_get_byte(2);
                SEG = x;                                  
                if (SEG) {   
                  MOR = (pif_get_byte(0)&0x04)>>2;
                  SA  = (pif_get_byte(0)&0x02)>>1;
                  SEQ = pif_get_byte(3);
                  sprintf(outstr,"%s, ID=%u, SEG=%u, MOR=%u, SA=%u, SEQ=%U",
                          BACnetServicesSupported[service_choice],
                          ID, SEG, MOR, SA, SEQ);
                  }
                else
                  sprintf(outstr,"%s, ID=%u",
                          BACnetServicesSupported[service_choice],
                          ID);
                break;
        case 1: /* Unconfirmed service request */
                service_choice = pif_get_byte(1);
                switch (service_choice) {
                  case 0: /* I-Am */
                    sprintf(outstr,"%s, %lu",BACnetServicesSupported[service_choice+26],
                           pif_get_long_hl(3)&0x003FFFFF);
                    break;
                  case 8: /* Who-Is */
                    if (pif_offset+2>=pif_end_offset)
                      sprintf(outstr,"%s, All Devices",BACnetServicesSupported[service_choice+26]);
                    else {
                      sprintf(outstr,"%s, %lu-%lu",BACnetServicesSupported[service_choice+26],
                      get_bac_unsigned(3,pif_get_byte(2)&0x07),
                      get_bac_unsigned(3+(pif_get_byte(2)&0x07)+1,pif_get_byte(3+(pif_get_byte(2)&0x07))&0x07));
                    }
                    break;  
                  default: sprintf(outstr,"%s",BACnetServicesSupported[service_choice+26]);
                }
                break;
        case 2: /* Simple ACK */
                ID = pif_get_byte(1);
                sprintf(outstr,"%s, ID=%u",PDU_types[pdu_type],ID);
                break;
        case 3: /* Complex ACK */
                ID = pif_get_byte(1);
                SEG = x;                                  
                if (SEG) {   
                  MOR = (pif_get_byte(0)&0x04)>>2;
                  SEQ = pif_get_byte(2);
                  sprintf(outstr,"%s, ID=%u, SEG=%u, MOR=%u, SEQ=%u",
                          PDU_types[pdu_type],
                          ID, SEG, MOR, SEQ);
                  }
                else
                  sprintf(outstr,"%s, ID=%u",
                          PDU_types[pdu_type],
                          ID);
                break;
        case 4: /* Segment ACK */
                ID = pif_get_byte(1);
                NAK = (pif_get_byte(0)&0x02)>>1UL;
                SRV = pif_get_byte(0)&0x01;
                sprintf(outstr,"%s, ID=%u, NAK=%u, SRV=%u",
                PDU_types[pdu_type], ID, NAK, SRV);
                break;
        case 5: /* Error */
                ID = pif_get_byte(1);
                sprintf(outstr,"%s, ID=%u",PDU_types[pdu_type], ID);
                break;
        case 6: /* Reject */
                ID = pif_get_byte(1);
                sprintf(outstr,"%s, ID=%u",PDU_types[pdu_type], ID);
                break;
        case 7: /* Abort */
                ID = pif_get_byte(1);
                sprintf(outstr,"%s, ID=%u",PDU_types[pdu_type], ID);
                break;
      }
	  strcpy( get_sum_line(pi_data_bacnet_AL), outstr );
/*
      sprintf (get_sum_line (pi_data_bacnet_AL),
         "BACnet AL (%s)",outstr);
*/
   }   /* End of Summary Line */
   
   /* Set up for get_int_line calls */
   pi_data_current = pi_data_bacnet_AL;

   /* Detail line? */
   if (pi_data_bacnet_AL->do_int) {
   pif_init (pi_data_bacnet_AL, header, length);
   pif_header (length, "BACnet Application Layer Detail");
   buff = pif_get_byte(0);
   x = (buff & 0xF0)>>4;      /* mask header for APDU type */
   switch (x) {
     case 0: show_confirmed(buff);
        break;
     case 1: show_unconfirmed(buff);
        break;
     case 2: show_simple_ack(buff);
        break;
     case 3: show_complex_ack(buff);
        break;
     case 4: show_segment_ack(buff);
        break;
     case 5: show_error(buff);
        break;
     case 6: show_reject(buff);
        break;
     case 7: show_abort(buff);
        break;
     default: bac_show_byte("Error: Unknown PDU Type","%u");
   };
   pif_show_space();
#if 0
   sprintf (
      get_int_line (pi_data_bacnet_AL,
      header - dlc_header,
      length
      ),
       "[%d octets of BACnet application layer data ]",
       length
       );
   pif_show_space();
#endif
   }   /* End of Detail Lines */

   return length;

}  /* End of Application Layer Interpreter */

/**************************************************************************/
int interp_Message( char *header, int length)  /* message interpreter */
/**************************************************************************/
{
   /* Summary line? */
   if (pi_data_Message->do_sum)
      strcpy( get_sum_line(pi_data_Message), header + 21 );

   /* Detail line? */
   if (pi_data_bacnet_IP->do_int) {
      pif_init (pi_data_Message, header, length);
      pif_header (length, "Message Detail");
	  bac_show_byte("Severity Code","%u");
	  bac_show_long_hl("Script Line","%u");
	  pif_show_nbytes_hex( "Digest                      = %s", 16 );
      pif_show_space();
	  pif_show_ascii( strlen(pif_get_addr()), "%s" );
      pif_show_space();
   }

   return length;
}

/***************************************************************************/
void show_confirmed( unsigned char x )
/***************************************************************************/

/*  This function interprets BACnet confirmed services */

{
   /* -- define array of ptrs to confirmed service interpreter functions -- */

   void (*show_confirmed_service[max_confirmed_services])() = {
      show_acknowledgeAlarm,            /* 0 */
      show_confirmedCOVNotification,    /* 1 */
      show_confirmedEventNotification,  /* 2 */
      show_getAlarmSummary,             /* 3 */
      show_getEnrollmentSummary,        /* 4 */
      show_subscribeCOV,                /* 5 */
      show_atomicReadFile,              /* 6 */
      show_atomicWriteFile,             /* 7 */
      show_addListElement,              /* 8 */
      show_removeListElement,           /* 9 */
      show_createObject,                /* 10 */
      show_deleteObject,                /* 11 */
      show_readProperty,                /* 12 */
      show_readPropertyConditional,     /* 13 */
      show_readPropertyMultiple,        /* 14 */
      show_writeProperty,               /* 15 */
      show_writePropertyMultiple,       /* 16 */
      show_deviceCommunicationControl,  /* 17 */
      show_privateTransfer,             /* 18 */
      show_confirmedTextMessage,        /* 19 */
      show_reinitializeDevice,          /* 20 */
      show_vtOpen,                      /* 21 */
      show_vtClose,                     /* 22 */
      show_vtData,                      /* 23 */
      show_authenticate,                /* 24 */
      show_requestKey                   /* 25 */
   };

   /* --- display the confirmed service header detail --- */

   sprintf(outstr,"%"FW"s = X'%%02X'","First Header Octet");
   bac_show_flag(outstr,0xFF);
   pif_show_flagmask(0xF0,0x00,"BACnet-Confirmed-Request-PDU");
   pif_show_flagbit(0x08,"Segmented Message","Unsegmented Message");
   pif_show_flagbit(0x04,"More Follows","No More Follows");
   pif_show_flagbit(0x03,"Segmented Resp Accepted","Segmented Resp not Accepted");
   sprintf(outstr,"%"FW"s = X'%%02X'","Maximum APDU Response Size Accepted");
   bac_show_flag(outstr,0xFF);
   pif_show_flagmask(0x0F, 0x00,"Up to 50 Octets");
   pif_show_flagmask(0x0F, 0x01,"Up to 128 Octets");
   pif_show_flagmask(0x0F, 0x02,"Up to 206 Octets");
   pif_show_flagmask(0x0F, 0x03,"Up to 480 Octets");
   pif_show_flagmask(0x0F, 0x04,"Up to 1024 Octets");
   pif_show_flagmask(0x0F, 0x05,"Up to 1476 Octets");
   bac_show_byte("Invoke ID","%d");
   if (x & 0x08)  {               /* SEG = 1 */
     bac_show_byte("Sequence Number","%d");
     bac_show_byte("Proposed Window Size","%d");
   }
   pif_show_space();
   if (pif_get_byte(0) >= max_confirmed_services)
      bac_show_byte("Error: Unknown Confirmed Service","%u");
   else
      (*show_confirmed_service[pif_get_byte(0)])(); /* call the service interpreter function */
}

/***************************************************************************/
void show_unconfirmed( unsigned char )
/***************************************************************************/
  /* This function interprets BACnet unconfirmed services */
{
   void (*show_unconfirmed_service[max_unconfirmed_services])() = {
      show_iAm,                         /* 0 */
      show_iHave,                       /* 1 */
      show_unconfirmedCOVNotification,  /* 2 */
      show_unconfEventNotification,     /* 3 */
      show_unconfPrivateTransfer,       /* 4 */
      show_unconfTextMessage,           /* 5 */
      show_timeSynchronization,         /* 6 */
      show_whoHas,                      /* 7 */
      show_whoIs                        /* 8 */
   };
   sprintf(outstr,"%"FW"s = X'%%02X'","First Header Octet");
   bac_show_flag(outstr,0xFF);
   pif_show_flagmask(0xF0,0x10,"BACnet-Unconfirmed-Request-PDU");
   pif_show_flagbit(0x0F,"Unused",NULLP);
   pif_show_space();
   /* call the unconfirmed service interpreter function */
   if (pif_get_byte(0) >= max_unconfirmed_services)
      bac_show_byte("Error: Unknown Unconfirmed Service","%u");
   else
      (*show_unconfirmed_service[pif_get_byte(0)])();
}

/**************************************************************************/
void show_simple_ack( unsigned char )
/**************************************************************************/
{
   sprintf(outstr,"%"FW"s = X'%%02X'","First Header Octet");
   bac_show_flag(outstr,0xFF);
   pif_show_flagmask(0xF0,0x20,"BACnet-SimpleACK-PDU");
   pif_show_flagbit(0x0F,"Unused",NULLP);
   bac_show_byte("Invoke ID","%d");
   bac_show_byte(BACnetServicesSupported[pif_get_byte(0)],"%d");
}

/*************************************************************************/
void show_complex_ack( unsigned char x )
/*************************************************************************/
{
   void (*show_confirmed_service_ACK[max_confirmed_services])(void) = {
      0,0,0,
      show_getAlarmSummaryACK,         /* 3 */
      show_getEnrollmentSummaryACK,    /* 4 */
      0,
      show_atomicReadFileACK,          /* 6 */
      show_atomicWriteFileACK,         /* 7 */
      0,0,
      show_createObjectACK,            /* 10 */
      0,
      show_readPropertyACK,            /* 12 */
      show_readPropertyConditionalACK, /* 13 */
      show_readPropertyMultipleACK,    /* 14 */
      0,0,0,
      show_conf_PrivateTransferACK,    /* 18*/
      0, 0,
      show_vtOpenACK,                  /* 21 */
      0,
      show_vtDataACK,                  /* 23 */
      show_authenticateACK             /* 24 */
   };
   sprintf(outstr,"%"FW"s = X'%%02X'","First Header Octet");
   bac_show_flag(outstr,0xFF);
   pif_show_flagmask(0xF0,0x30,"BACnet-ComplexACK-PDU");
   pif_show_flagbit(0x08,"Segmented Message","Unsegmented Message");
   pif_show_flagbit(0x04,"More Follows","No More Follows");
   pif_show_flagbit(0x03,"Unused",NULLP);
   pif_show_byte("Invoke ID                   = %d");
   if (x & 0x08) /* SEG = 1 */ {
     pif_show_byte   ("Sequence Number             = %d");
     bac_show_byte("Proposed Window Size","%d");
   }
   pif_show_space();
   /* call the confirmed service ACK interpreter function */
   if (pif_get_byte(0) >= max_confirmed_services)
      bac_show_byte("Error: Unknown Complex ACK Type","%u");
   else
   if (x & 0x08) /* SEG = 1 */ {
     pif_show_space();
     bac_show_nbytes( pif_end_offset - pif_offset, "[segmented data]" );
   } else
      (*show_confirmed_service_ACK[pif_get_byte(0)])();
}

/*************************************************************************/
void show_segment_ack( unsigned char )
/*************************************************************************/
{
   sprintf(outstr,"%"FW"s = X'%%02X'","First Header Octet");
   bac_show_flag(outstr,0xFF);
   pif_show_flagmask(0xF0,0x40,"BACnet-SegmentACK-PDU");
   pif_show_flagbit(0x0e,"Unused",NULLP);
   pif_show_flagbit(0x01,"Ack From Server","Ack From Client");
   bac_show_byte("Original Invoke ID","%d");
   bac_show_byte("Sequence Number","%d");
   bac_show_byte("Actual Window Size","%d");
}

/**************************************************************************/
void show_error( unsigned char x )
/**************************************************************************/
{
   void (*show_confirmed_service_error[max_confirmed_services])() = {
      show_error_codes,
      show_error_codes,
      show_error_codes,
      show_error_codes,
      show_error_codes,
      show_error_codes,
      show_error_codes,
      show_error_codes,
      show_createObjectError,  /*change list error */
      show_createObjectError,  /*change list error */
      show_createObjectError,
      show_error_codes,
      show_error_codes,
      show_error_codes,
      show_error_codes,
      show_error_codes,
      show_writePropertyMultipleError,
      show_error_codes,
      show_error_codes,
      show_error_codes,
      show_error_codes,
      show_error_codes,
      show_vtCloseError,
      show_error_codes,
      show_error_codes,
      show_error_codes
   };
   sprintf(outstr,"%"FW"s = X'%%02X'","First Header Octet");
   bac_show_flag(outstr,0xFF);
   pif_show_flagmask(0xF0,0x50,"BACnet-Error-PDU");
   pif_show_flagbit(0x08,"Segmented Message","Unsegmented Message");
   pif_show_flagbit(0x04,"More Follows","No More Follows");
   pif_show_flagbit(0x03,"Unused",NULLP);
   bac_show_byte("Invoke ID","%d");
   if (x & 0x08) /* SEG = 1 */ {
     bac_show_byte   ("Sequence Number","%d");
     bac_show_byte("Proposed Window Size","%d");
   }
   pif_show_space();
   if (pif_get_byte(0) < max_confirmed_services) {
      sprintf(outstr,"%s Service Error",BACnetServicesSupported[pif_get_byte(0)]);
      bac_show_byte(outstr,"%u");
      (*show_confirmed_service_error[pif_get_byte(-1)])();
   }
   else {
      if (pif_get_byte(0) == 0) {
    bac_show_byte("Other Error","%u");
    show_error_codes();
      }
      else
    bac_show_byte("Error: Unknown Error Designation","%u");
   }
}

/**************************************************************************/
void show_reject( unsigned char )
/**************************************************************************/
{
   sprintf(outstr,"%"FW"s = X'%%02X'","First Header Octet");
   bac_show_flag(outstr,0xFF);
   pif_show_flagmask(0xF0,0x60,"BACnet-Reject-PDU");
   pif_show_flagbit(0x0F,"Unused",NULLP);
   bac_show_byte("Original Invoke ID","%d");
   sprintf(outstr,"%"FW"s = %s = (%%u)","Reject Reason",
      BACnetReject[pif_get_byte(0)]);
   pif_show_byte(outstr);
}

/**************************************************************************/
void show_abort( unsigned char )
/**************************************************************************/
{
   sprintf(outstr,"%"FW"s = X'%%02X'","First Header Octet");
   bac_show_flag(outstr,0xFF);
   pif_show_flagmask(0xF0,0x70,"BACnet-Abort-PDU");
   pif_show_flagbit(0x0E,"Unused",NULLP);
   pif_show_flagbit(0x01,"Abort From Server","Abort From Client");
   bac_show_byte("Original Invoke ID","%d");
   sprintf(outstr,"%"FW"s = %s = (%%u)","Abort Reason",
      BACnetAbort[pif_get_byte(0)]);
   pif_show_byte(outstr);
}

/**************************************************************************/
void show_acknowledgeAlarm( void )
/**************************************************************************/
  /* This function interprets AcknowledgeAlarm service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;
   unsigned int event_state;

   bac_show_byte("Acknowledge Alarm Request","%u");
   while(pif_offset < pif_end_offset) {
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) {
    switch(tagval) {
       case 0:  len = show_context_tag("Acknowledging Process Identifier");
           show_bac_unsigned(len);
           break;
       case 1:  show_context_tag("Event Object Identifier");
           show_bac_object_identifier();
           break;
       case 2:  show_context_tag("Event State Acknowledged");
           event_state = pif_get_byte(0);
           if(event_state <= 4)
             bac_show_byte(BACnetEventState[pif_get_byte(0)],"%u");
           else {
             if(event_state < 64) /* reserved range */
                bac_show_byte("Error - Invalid Event State", "%u");
             else
                bac_show_byte("Proprietary Event State", "%u");
             };
           break;
       case 3:  show_context_tag("Time Stamp");
           show_bac_timestamp();
           show_context_tag("Time Stamp");  /* closing tag */
           break;
       case 4: len = show_context_tag("Acknowledgement Source");
               show_bac_charstring(len);
           break;
       case 5:  show_context_tag("Time of Acknowledgement");
           show_bac_timestamp();
           show_context_tag("Time of Acknowledgement");  /* closing tag */
           break;
       default: len = show_context_tag("Unknown tag");
           bac_show_nbytes(len,"Unknown data");
    }
      }
      else
         show_application_data(tagbuff);
   }
}

/**************************************************************************/
void show_confirmedCOVNotification( void )
/**************************************************************************/
  /* This function interprets ConfirmedCOVNotification requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Confirmed COV Notification Request","%u");

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if(tagval == 0){
      len = show_context_tag("Subscriber Process Identifier");
      show_bac_unsigned(len);
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 0 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if(tagval == 1){
      len = show_context_tag("Initiating Device Identifier");
      show_bac_object_identifier();
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 1 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if(tagval == 2){
      show_context_tag("Monitored Object Identifier");
      show_bac_object_identifier();
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 2 Expected!");
      };
   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if(tagval == 3){
      len = show_context_tag("Time Remaining (seconds)");
      show_bac_unsigned(len);
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 3 Expected!");
      };
   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if(tagval == 4){
      show_context_tag("List of Values");  /* opening tag */
      while(pif_offset < (pif_end_offset-1))
         show_bac_property_value();
      show_context_tag("List of Values");  /* closing tag */
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 4 Expected!");
      };
}

/**************************************************************************/
void show_confirmedEventNotification( void )
/**************************************************************************/
  /* This function interprets ConfirmedEventNotification service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len, notify_type;

   bac_show_byte("Confirmed Event Notification","%u");
   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 0)){
      len = show_context_tag("Process Identifier");
      show_bac_unsigned(len);
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 0 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 1)){
      show_context_tag("Initiating Device Identifier");
      show_bac_object_identifier();
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 1 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 2)){
      show_context_tag("Event Object Identifier");
      show_bac_object_identifier();
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 2 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 3)){
      show_context_tag("Time Stamp");
      show_bac_timestamp();
      show_context_tag("Time Stamp");  /* show closing tag */
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 3 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 4)){
      show_context_tag("Notification Class");
      bac_show_byte("Class (1..255)","%u");
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 4 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 5)){
      show_context_tag("Priority");
      bac_show_byte("Priority (1..255)","%u");
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 5 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 6)){
      show_context_tag("Event Type");
      bac_show_byte(BACnetEventType[pif_get_byte(0)],"%u");
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 6 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 7)){
      len = show_context_tag("Message Text");
      show_bac_charstring(len);
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 8)){
      show_context_tag("Notify Type");
      notify_type = pif_get_byte(0);
      bac_show_byte(BACnetNotifyType[pif_get_byte(0)],"%u");
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 8 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 9)){
      show_context_tag("Acknowledgement Required");
      tagbuff = pif_get_byte(0);
      if(tagbuff)  /* TRUE */
         xsprintf(get_int_line(pi_data_current,pif_offset,1),
         "%"FW"s = %>ku %s", "Acknowledgement Required","(TRUE)");
      else
         xsprintf(get_int_line(pi_data_current,pif_offset,1),
         "%"FW"s = %>ku %s", "Acknowledgement Required","(FALSE)");
      pif_show_space();
      }
   else{ /* required for alarm or event notifications */
      if(notify_type != 2){
         pif_show_space();
         pif_show_ascii(0, "Error: Context Tag 9 Expected!");
         };
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 10)){
      show_context_tag("From State");
      bac_show_byte(BACnetEventState[pif_get_byte(0)], "%u");
      }
   else{  /* required for alarm or event notifications */
      if(notify_type != 2){
         pif_show_space();
         pif_show_ascii(0, "Error: Context Tag 10 Expected!");
         };
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 11)){
      show_context_tag("To State");
      bac_show_byte(BACnetEventState[pif_get_byte(0)], "%u");
      }
   else{  /* required for alarm or event notifications */
      if(notify_type != 2){
         pif_show_space();
         pif_show_ascii(0, "Error: Context Tag 11 Expected!");
         };
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 12)){
      show_context_tag("Event Values");
      tagbuff = pif_get_byte(0);
      tagval= (tagbuff&0xF0)>>4;
      switch(tagval){
         case 0:/* change of bitstring */
            show_context_tag("Change of Bitstring Event Parameters");  /* opening tag */

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               len = show_context_tag("Referenced Bitstring");
               show_bac_bitstring(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               len = show_context_tag("Status Flags");
               show_bac_status_flags(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };

            show_context_tag("Change of Bitstring Event Parameters");  /* closing tag */
            break;
         case 1: /* change of state */
            show_context_tag("Change of State Event Parameters");  /* opening tag */

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               show_context_tag("New State");
               show_bac_property_states();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               len = show_context_tag("Status Flags");
               show_bac_status_flags(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };

            show_context_tag("Change of State Event Parameters");  /* closing tag */
            break;
         case 2: /* change of value */
            show_context_tag("Change of Value Event Parameters");  /* opening tag */

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               show_context_tag("New Value");         /* opening tag */
               tagbuff = pif_get_byte(0);
               tagval = (tagbuff&0xF0)>>4;
               switch(tagval){
                  case 0:
                     len = show_context_tag("Changed Bits");
                     show_bac_bitstring(len);
                     break;
                  case 1:
                     show_context_tag("Changed Value");
                     show_bac_real();
                     break;
                  default:
                     pif_show_space();
                     pif_show_ascii(0, "Error: Context Tag 0 or 1 Expected!");
                     break;
                  };
               show_context_tag("New Value");         /* closing tag */
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               len=show_context_tag("Status Flags");
               show_bac_status_flags(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };
            show_context_tag("Change of Value Event Parameters");  /* closing tag */
            break;
         case 3: /* command failure */
            show_context_tag("Command Failure Event Parameters");  /* opening tag */

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               show_context_tag("Command Value"); /* opening tag */

               tagbuff = pif_get_byte(0);
               if((tagbuff & 0x08) && ((tagbuff&0x0f) != 0x0e)){ /* SD context tag */
                  len = show_context_tag("Unknown Context");
                  bac_show_nbytes(len,"Unknown Data");
                  };

               if((tagbuff & 0x08) && ((tagbuff&0x0f) == 0x0e)){ /* PD context tag */
                  show_context_tag("Unknown Context"); /* opening tag */
                  len = 0;
                  tagbuff = tagbuff | 0x01; /* set patern for closing tag */
                  while(pif_get_byte(len) != tagbuff) len++;
                  bac_show_nbytes(len+1,"Unknown Data");
                  show_context_tag("Unknown Context"); /* closing tag */
                  };

               /* normal case */
               show_application_data(tagbuff);
               show_context_tag("Command Value"); /* closing tag */
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               len=show_context_tag("Status Flags");
               show_bac_status_flags(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 2)){
               show_context_tag("Feedback Value");

               tagbuff = pif_get_byte(0);
               if((tagbuff & 0x08) && ((tagbuff&0x0f) != 0x0e)){ /* SD context tag */
                  len = show_context_tag("Unknown Context");
                  bac_show_nbytes(len,"Unknown Data");
                  };

               if((tagbuff & 0x08) && ((tagbuff&0x0f) == 0x0e)){ /* PD context tag */
                  show_context_tag("Unknown Context"); /* opening tag */
                  len = 0;
                  tagbuff = tagbuff | 0x01; /* set patern for closing tag */
                  while(pif_get_byte(len) != tagbuff) len++;
                  bac_show_nbytes(len+1,"Unknown Data");
                  show_context_tag("Unknown Context"); /* closing tag */
                  };

               /* normal case */
               show_application_data(tagbuff);
               show_context_tag("Command Value"); /* closing tag */
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 2 Expected!");
               };

            show_context_tag("Command Failure Event Parameters");  /* closing tag */
            break;
         case 4: /* floating limit */
            show_context_tag("Floating Limit Event Parameters");  /* opening tag */
/*
            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               len = show_context_tag("Referenced Bitstring");
               show_bac_bitstring(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };
*/
            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               show_context_tag("Referenced Value");
               show_bac_real();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               len=show_context_tag("Status Flags");
               show_bac_status_flags(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 2)){
               show_context_tag("Setpoint Value");
               show_bac_real();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 2 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 3)){
               show_context_tag("Error Limit");
               show_bac_real();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 3 Expected!");
               };

            show_context_tag("Floating Limit Event Parameters");  /* closing tag */
            break;
         case 5: /* out of range */
            show_context_tag("Out of Range Event Parameters");  /* opening tag */
            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               show_context_tag("Exceding Value");
               show_bac_real();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               len = show_context_tag("Status Flags");
               show_bac_status_flags(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 2)){
               show_context_tag("Deadband");
               show_bac_real();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 2 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 3)){
               show_context_tag("Exceeded Limit");
               show_bac_real();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 3 Expected!");
               };

            show_context_tag("Out of Range Event Parameters");  /* closing tag */
            break;
         case 6: /* complex event type */
            show_context_tag("Complex Event Parameters");  /* opening tag */
            while(pif_offset < (pif_end_offset-2))
               show_bac_property_value();
            show_context_tag("Complex Event Parameters");  /* closing tag */
            break;
         default:
            pif_show_space();
            bac_show_nbytes(1,"Error: Invalid Context Tag (should be <= 6)!");
            break;
         };
      show_context_tag("Event Values");  /* closing tag */
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 12 Expected!");
      };
}

/**************************************************************************/
void show_getAlarmSummary( void )
/**************************************************************************/
  /* This function interprets GetAlarmSummary service requests */
{
   bac_show_byte("Get Alarm Summary Request","%u");
}

/**************************************************************************/
void show_getEnrollmentSummary( void )
/**************************************************************************/
  /* This function interprets GetEnrollment service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Get Enrollment Summary Request","%u");
   while(pif_offset < pif_end_offset) {
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) {
        switch(tagval) {
           case 0:  show_context_tag("Acknowledgement Filter");
               switch (pif_get_byte(0)) {
                  case 0: bac_show_byte("Select ALL","%u");
                          break;
                  case 1: bac_show_byte("Select ACK'ed","%u");
                          break;
                  case 2: bac_show_byte("Select NOT ACK'ed","%u");
                          break;
                  default: bac_show_byte("Error: Unknown enumeration","%u");
                  } /* end of switch */
               break;
           case 1: show_context_tag("Enrollment Filter"); /* opening tag */
               show_bac_recipient_process();
               show_context_tag("Enrollment Filter");     /* closing tag */
               break;
           case 2:  show_context_tag("Event State Filter");
               switch (pif_get_byte(0)) {
                  case 0: bac_show_byte("Select OFFNORMAL", "%u");
                          break;
                  case 1: bac_show_byte("Select FAULT","%u");
                          break;
                  case 2: bac_show_byte("Select NORMAL","%u");
                          break;
                  case 3: bac_show_byte("Select ALL","%u");
                          break;
                  case 4: bac_show_byte("Select ACTIVE","%u");
                          break;
                  default: bac_show_byte("Error: Unknown enumeration","%u");
                  } /* end of switch */
               break;
           case 3: show_context_tag("Event Type Filter");
               bac_show_byte(BACnetEventType[pif_get_byte(0)],"%u");
               break;
           case 4:  show_context_tag("Event Priority Filter");
               len = show_context_tag("Minimum Priority");
               show_bac_unsigned(len);
               len = show_context_tag("Maximum Priority");
               show_bac_unsigned(len);
               show_context_tag("Event Priority Filter");  /* closing tag */
               break;
           case 5:  len = show_context_tag("Notification Class Filter");
               show_bac_unsigned(len);
               break;
           default: len = show_context_tag("Unknown tag");
               bac_show_nbytes(len,"Error: Unknown data");
           }
        }
      else{  /* application tag */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }  
      }  /* end of while */
}

/**************************************************************************/
void show_subscribeCOV( void )
/**************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Subscribe COV Request","%u");
   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if (tagbuff & 0x08) { /* context tag found */
      if (tagval == 0){
        len = show_context_tag("Subscriber Process Identifier");
        show_bac_unsigned(len);
        }
      else {
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag 0 Expected!");
        pif_offset = pif_end_offset;
        }
      }
      else{  /* application tag  */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }  

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if (tagbuff & 0x08) { /* context tag found */
      if (tagval == 1){
         show_context_tag("Monitored Object Identifier");
         show_bac_object_identifier();
         }
      else {
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag 1 Expected!");
        pif_offset = pif_end_offset;
        }
      }
      else{  /* application tag  */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }

   if (pif_offset < pif_end_offset) { /* both Issue Confirmed Notifications
                                         and Lifetime should be present */
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) { /* context tag found */
         if (tagval == 2){
            show_context_tag("Issue Confirmed Notifications");
            tagbuff = pif_get_byte(0);
            if(tagbuff)  /* TRUE */
               xsprintf(get_int_line(pi_data_current,pif_offset,1),
               "%"FW"s = %>ku %s", "Issue Confirmed Notifications","(TRUE)");
            else
               xsprintf(get_int_line(pi_data_current,pif_offset,1),
               "%"FW"s = %>ku %s", "Issue Confirmed Notifications","(FALSE)");
            pif_show_space();
            }
         else {
           pif_show_space();
           pif_show_ascii(0, "Error: Context Tag 2 Expected!");
           pif_offset = pif_end_offset;
           }
         }
      else{  /* application tag  */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }

      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) { /* context tag found */
         if (tagval == 3){
            show_context_tag("Lifetime (seconds)");
            if(pif_get_byte(0)) /* non zero value means a finite lifetime */
               show_bac_unsigned(len);
            else {
               show_bac_unsigned(len);
               pif_show_ascii(0, "Indefinite Lifetime Requested");
               }
            }
         else {
           pif_show_space();
           pif_show_ascii(0, "Error: Context Tag 3 Expected!");
           pif_offset = pif_end_offset;
           }
         }
         else{  /* application tag  */
           pif_show_space();
           pif_show_ascii(0, "Error: Context Tag Expected!");
           pif_offset = pif_end_offset;
           }
      }
   else { /* this is a cancellation */
      pif_show_space();
      pif_show_ascii(0, "Subscription Cancellation Request");
      }
}

/**************************************************************************/
void show_atomicReadFile( void )
/**************************************************************************/
  /* This function interprets AtomicReadFile service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Atomic Read File Request","%u");
   pif_show_space();
   pif_show_ascii(0,"File Name Object Identifier");
   show_application_data(pif_get_byte(0));
   pif_show_space();
   while(pif_offset < pif_end_offset) {
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) {  /* context tag */
        switch (tagval) {
           case 0:  pif_show_space();
               pif_show_ascii(0,"Access Method");
               show_context_tag("Stream Access");  /* opening tag */
               pif_show_space();
               pif_show_ascii(0,"File Start Position");
               show_application_data(pif_get_byte(0));
               pif_show_space();
               pif_show_ascii(0,"Requested Octet Count");
               show_application_data(pif_get_byte(0));
               show_context_tag("Stream Access");  /* closing tag */
               break;
           case 1:  pif_show_space();
               pif_show_ascii(0,"Access Method");
               show_context_tag("Record Access");  /* opening tag */
               pif_show_space();
               pif_show_ascii(0,"File Start Record");
               show_application_data(pif_get_byte(0));
               pif_show_space();
               pif_show_ascii(0,"Requested Record Count");
               show_application_data(pif_get_byte(0));
               show_context_tag("Record Access");  /* closing tag */
               break;
           default: len = show_context_tag("Unknown tag");
               bac_show_nbytes(len,"Error: Unknown data");
           }  /* end of switch */
        }
      else{  /* application tag */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }  
      }  /* end of while */
}
    
/**************************************************************************/
void show_atomicWriteFile( void )
/**************************************************************************/
  /* This function interprets AtomicWriteFile service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Atomic Write File Request","%u");
   pif_show_space();
   pif_show_ascii(0,"File Name Object Identifier");
   show_application_data(pif_get_byte(0));
   pif_show_space();
   while(pif_offset < pif_end_offset) {  
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) {  /* context tag */
        switch (tagval) {
           case 0: pif_show_space();
               pif_show_ascii(0,"Access Method");
               show_context_tag("Stream Access");  /* opening tag */
               pif_show_space();
               pif_show_ascii(0,"File Start Position");

               /* -- check for append special case -- */
               if(pif_get_byte(1) == 0xff){
                 show_application_tag(pif_get_byte(0));
                 sprintf(outstr,"%"FW"s = `%%s'","Append to End-of-File");
                 pif_show_nbytes_hex(outstr, 1);
                 }
               else
                 show_application_data(pif_get_byte(0));

               pif_show_space();
               pif_show_ascii(0,"File Data");
               show_application_data(pif_get_byte(0));
               show_context_tag("Stream Access");  /* closing tag */
               break;
           case 1: pif_show_space();
               pif_show_ascii(0,"Access Method");
               show_context_tag("Record Access");  /* opening tag */
               pif_show_space();
               pif_show_ascii(0,"File Start Record");

               /* -- check for append special case -- */
               if(pif_get_byte(1) == 0xff){
                 show_application_tag(pif_get_byte(0));
                 sprintf(outstr,"%"FW"s = `%%s'","Append to End-of-File");
                 pif_show_nbytes_hex(outstr, 1);
                 }
               else
                 show_application_data(pif_get_byte(0));

               pif_show_space();
               pif_show_ascii(0,"Record Count");
               show_application_data(pif_get_byte(0));
               pif_show_space();
               pif_show_ascii(0,"File Record Data");
               tagbuff = pif_get_byte(0);
               while((tagbuff & 0x0f) != 0x0f){
                  show_application_data(pif_get_byte(0));
                  tagbuff = pif_get_byte(0);
                  }
               show_context_tag("Record Access");  /* closing tag */
               break;

           default: len = show_context_tag("Unknown tag");
               bac_show_nbytes(len,"Error: Unknown data");
           }  /* end of switch */
        }
      else{  /* application tag */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }  
      }  /* end of while */
}

/**************************************************************************/
void show_addListElement( void )
/*************************************************************************/
  /* This function interprets AddListElement service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   int obj_type,prop_idx,prop_id, vendor_id;
   unsigned int len;

   bac_show_byte("Add List Element Request","%u");
   obj_type = prop_idx = prop_id = vendor_id = -1;
   while(pif_offset < pif_end_offset) { 
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) {  /* context tag */
        switch(tagval) {
           case 0: show_context_tag("Object Identifier");
               obj_type = bac_extract_obj_type();  
               vendor_id = pif_get_byte(0) & 0x80;
               show_bac_object_identifier();
               break;
           case 1: len = show_context_tag("Property Identifier");
               if (len == 1)
                 prop_id = (int)pif_get_byte(0);
               else
                 prop_id = (int)pif_get_word_hl(0);
               show_bac_property_identifier(len);
               break;
           case 2:  show_context_tag("Property Array Index");
               prop_idx = pif_get_byte(0);
               bac_show_byte("Property Array Index","%u");
               break;
           case 3:  /* List of Elements */
               /* determine length of data */
               lenexp = 0;
               while (tagbuff != 0x3f){ /* search for matching closing tag */
                  lenexp += 1;
                  tagbuff = pif_get_byte(lenexp);
                  if (tagbuff == 0x3e){ /* nested opening tag 3 */
                     while (tagbuff != 0x3f){/* search for matching closing tag */
                        lenexp += 1;
                        tagbuff = pif_get_byte(lenexp);
                        }
                     };
                  }

               show_context_tag("List of Elements");  /* opening tag */
               if (vendor_id != 0)
                 bac_show_nbytes(lenexp,"SEQUENCE OF ANY "
                 "(Interpretable only if Object Type is known)");
               else 
                 while (pif_offset<(pif_end_offset-1))  
                   show_bac_ANY(obj_type,prop_id,prop_idx);
               show_context_tag("List of Elements");  /* closing tag */
               break;
           default: len = show_context_tag("Unknown tag");
               bac_show_nbytes(len,"Error: Unknown data");
           }  /* end of switch */
        }
      else{  /* application tag */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }  
      }  /* end of while */
}

/*************************************************************************/
void show_removeListElement( void )
/*************************************************************************/
  /* This function interprets RemoveListElement service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   int obj_type,prop_idx,prop_id,vendor_id;
   unsigned int len;

   bac_show_byte("Remove List Element Request","%u");
   obj_type = prop_idx = prop_id = vendor_id = -1;
   while(pif_offset < pif_end_offset) { 
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) {  /* context tag */
        switch(tagval) {
           case 0: show_context_tag("Object Identifier");
               obj_type = bac_extract_obj_type();  
               vendor_id = pif_get_byte(0) & 0x80;
               show_bac_object_identifier();
               break;
           case 1: len = show_context_tag("Property Identifier");
               if (len == 1)
                 prop_id = (int)pif_get_byte(0);
               else
                 prop_id = (int)pif_get_word_hl(0);
               show_bac_property_identifier(len);
               break;
           case 2:  show_context_tag("Property Array Index");
               prop_idx = pif_get_byte(0);
               bac_show_byte("Property Array Index","%u");
               break;
           case 3:  /* List of Elements */
               /* determine length of data */
               lenexp = 0;
               while (tagbuff != 0x3f){ /* search for matching closing tag */
                  lenexp += 1;
                  tagbuff = pif_get_byte(lenexp);
                  if (tagbuff == 0x3e){ /* nested opening tag 3 */
                     while (tagbuff != 0x3f){/* search for matching closing tag */
                        lenexp += 1;
                        tagbuff = pif_get_byte(lenexp);
                        }
                     };
                  }

               show_context_tag("List of Elements");  /* opening tag */
               if (vendor_id != 0)
                 bac_show_nbytes(lenexp,"SEQUENCE OF ANY "
                 "(Interpretable only if Object Type is known)");
               else 
                 while (pif_offset<(pif_end_offset-1))  
                   show_bac_ANY(obj_type,prop_id,prop_idx);
               show_context_tag("List of Elements");  /* closing tag */
               break;
           default: len = show_context_tag("Unknown tag");
               bac_show_nbytes(len,"Error: Unknown data");
           }  /* end of switch */
        }
      else{  /* application tag */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }  
      }  /* end of while */
}

/*************************************************************************/
void show_createObject( void )
/*************************************************************************/
  /* This function interprets CreateObject service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   /* unsigned int len;
    * int obj_type;
    */

   bac_show_byte("Create Object Request","%u");
   pif_show_space();

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 0)){
      show_context_tag("Object Specifier");

      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      switch(tagval){
         case 0: /* object type choice */
            show_context_tag("Object Type");
            bac_show_byte(BACnetObjectType[pif_get_byte(0)],"%u");
            break;
         case 1:/* object identifier choice */
            show_context_tag("Object Identifier");
            show_bac_object_identifier();
            break;
         default:
            pif_show_space();
            pif_show_ascii(0, "Error: Context Tag 0 or 1 Expected!");
            pif_offset = pif_end_offset;
         };  /* end switch */

      show_context_tag("Object Specifier");
      }
   else {
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 0 Expected!");
      pif_offset = pif_end_offset;
      }
            
   if (pif_offset < pif_end_offset){
      tagbuff = pif_get_byte(0);
      if (tagbuff==0x1e){                  /* list of initial values */
         show_context_tag("List of Initial Values");  /* opening tag */
         while(pif_offset < (pif_end_offset-1))
            show_bac_property_value();
         show_context_tag("List of Initial Values");  /* closing tag */
         }
      else{
         pif_show_space();
         pif_show_ascii(0, "Error: Context Opening Tag 1 Expected!");
         pif_offset = pif_end_offset;
         }
      };  /* end of list of initial values if */
}

/*************************************************************************/
void show_deleteObject( void )
/*************************************************************************/
  /* This function interprets DeleteObject service requests */
{
   bac_show_byte("Delete Object Request","%u");
   show_application_data(pif_get_byte(0));
}

/*************************************************************************/
void show_readProperty( void )
/*************************************************************************/
  /* This function interprets ReadProperty service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Read Property Request","%u");
   tagbuff = pif_get_byte(0);
   while(pif_offset < pif_end_offset) {
      tagval = (tagbuff&0xf0)>>4;
      if (tagbuff & 0x08) {  /* context tag */
        if(tagval > 2) {
          pif_show_space();
         bac_show_nbytes(1,"Error: Invalid Context Tag (should be <= 2)!");
          goto exit;
          }
        len = show_context_tag(BACnetObjectPropertyReference[tagval]);
        switch (tagval) {
          case 0:  show_bac_object_identifier();
                   break;
          case 1:  show_bac_property_identifier(len);
                   break;
          case 2:  show_bac_unsigned(len);  /* array index */
                   break;
          }
        }
      else{  /* application tag */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }  
      tagbuff = pif_get_byte(0);
      }  /* end of while loop */
   exit:;
}

/*************************************************************************/
void show_readPropertyConditional( void )
/*************************************************************************/
  /* This function interprets ReadPropertyConditional service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Read Property Conditional Request","%u");
   while(pif_offset < pif_end_offset) {
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) {
        switch(tagval) {
           case 0: show_context_tag("Object Selection Criteria");  /* opening tag */
               tagbuff = pif_get_byte(0);
               tagval = (tagbuff&0xF0)>>4;
               while (tagbuff != 0x0f) {  /* loop until closing tag */
                 switch(tagval){
                    case 0: show_context_tag("Selection Logic");
                       switch (pif_get_byte(0)) {
                          case 0: bac_show_byte("Logical AND","%u");
                                  break;
                          case 1: bac_show_byte("Logical OR","%u");
                                  break;
                          case 2: bac_show_byte("Select ALL","%u");
                                  break;
                          default: bac_show_byte(
                                  "Error: Invalid enumeration (>2)!","%u");
                          }
                       break;
                    case 1: show_context_tag("List of Selection Criteria");  /* opening tag */
                       tagbuff = pif_get_byte(0);
                       while (tagbuff != 0x1f) {  /* loop until closing tag */
                         switch((pif_get_byte(0)&0xf0)>>4) {
                            case 0: len = show_context_tag("Property Identifier");
                                    show_bac_property_identifier(len);
                                    break;
                            case 1: len = show_context_tag("Property Array Index");
                                    show_bac_unsigned(len);
                                    break;
                            case 2: show_context_tag("Relation Specifier");
                                    bac_show_byte(Relation_Specifier[pif_get_byte(0)],"%u");
                                    break;
                            case 3: show_context_tag("Comparison Value");  /* opening tag */
                                    show_application_data(pif_get_byte(0));
                                    show_context_tag("Comparison Value");  /* closing tag */
                                    break;
                            default: bac_show_byte(
                                    "Error: Invalid Context Tag!","%u");
                            } /*End SWITCH*/
                         tagbuff = pif_get_byte(0);
                         } /*End WHILE*/
                       show_context_tag("List of Selection Criteria");  /* closing tag */
                       break;
                    }  /* end of switch */
                 tagbuff = pif_get_byte(0);
                 tagval = (tagbuff&0xF0)>>4;
                 }  /* end while */
               show_context_tag("Object Selection Criteria");  /* closing tag */
               break;
           case 1: show_context_tag("List of Property References");  /* opening tag */
               while(tagbuff != 0x1f){
                  show_bac_property_ref();
                  tagbuff = pif_get_byte(0);
                  }
               show_context_tag("List of Property References");  /* closing tag */
               break;
           default: len = show_context_tag("Unknown tag");
               bac_show_nbytes(len,"Error: Unknown data");
           }  /* end switch */
        }
      else{  /* application tag */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }  
      }  /* end while */
}

/*************************************************************************/
void show_readPropertyMultiple( void )
/*************************************************************************/
  /* This function interprets ReadPropertyMultiple service requests */
{
   bac_show_byte("Read Property Multiple Request","%u");
   while(pif_offset < pif_end_offset)
     show_bac_read_access_spec();
}

/*************************************************************************/
void show_writeProperty( void )
/*************************************************************************/
  /* This function interprets WriteProperty service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;
   int obj_type, prop_idx,prop_id;

   bac_show_byte("Write Property Request","%u");
   prop_idx = -1;
   while(pif_offset < pif_end_offset) {
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) {
        switch(tagval) {
           case 0:  show_context_tag("Object Identifier");
               obj_type = bac_extract_obj_type();
               show_bac_object_identifier();
               break;
           case 1:  len = show_context_tag("Property Identifier");
               if (len == 1)
                 prop_id = (int)pif_get_byte(0);
               else
                 prop_id = (int)pif_get_word_hl(0);
               show_bac_property_identifier(len);
               break;
           case 2:  len = show_context_tag("Property Array Index");
               prop_idx = pif_get_byte(0);
               show_bac_unsigned(len);
               break;
           case 3:  show_context_tag("Property Value"); /* opening tag */
               show_bac_ANY(obj_type,prop_id,prop_idx);
               show_context_tag("Property Value"); /* closing tag */
               break;
           case 4:  len = show_context_tag("Priority");
               show_bac_unsigned(len);
               break;
           default: len = show_context_tag("Unknown tag");
               bac_show_nbytes(len,"Error: Unknown data");
           }  /* end switch */
        }
      else{  /* application tag */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }  
      }  /* end while */
}

/*************************************************************************/
void show_writePropertyMultiple( void )
/*************************************************************************/
  /* This function interprets WritePropertyMultiple service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;
   int obj_type, prop_idx;

   bac_show_byte("Write Property Multiple Request","%u");
   obj_type = prop_idx = -1;
   while(pif_offset < pif_end_offset) {
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) {  /* context tag */
        switch(tagval) {
           case 0:  show_context_tag("Object Identifier");
               obj_type = bac_extract_obj_type();
               show_bac_object_identifier();
               break;
           case 1:  /* List of Properties */
               show_context_tag("List of Properties");  /* opening tag */
               show_bac_property_value();
               show_context_tag("List of Properties");  /* closing tag */
               break;
           default: len = show_context_tag("Unknown tag");
               bac_show_nbytes(len,"Error: Unknown data");
               break;
           }  /* end of switch */
        }  /* end of if branch */
      else{  /* application tag */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }  
      };  /* end of while */
}

/*************************************************************************/
void show_deviceCommunicationControl( void )
/*************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Device Communication Control Request","%u");
   while(pif_offset < pif_end_offset) {
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) {  /* context tag */
        switch(tagval) {
           case 0:  len = show_context_tag("Time Duration (minutes)");
               show_bac_unsigned(len);
               break;
           case 1:  /* enable/disable flag */
               show_context_tag("Enable/Disable Flag");
               tagbuff = pif_get_byte(0);
               if(tagbuff)  /* TRUE */
                 xsprintf(get_int_line(pi_data_current,pif_offset,1),
                   "%"FW"s = %>ku %s", "Device Communication Status","(DISABLED)");
               else
                 xsprintf(get_int_line(pi_data_current,pif_offset,1),
                   "%"FW"s = %>ku %s", "Device Communication Status","(ENABLED)");
               pif_show_space();
               break;
           case 2:  /* password */
               len = show_context_tag("Password");
               show_bac_charstring(len);
               break;
           default: len = show_context_tag("Unknown tag");
               bac_show_nbytes(len,"Error: Unknown data");
               break;
           }  /* end of switch */
        }  /* end of if branch */
      else{  /* application tag */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }
      };  /* end of while */
}

/*************************************************************************/
void show_privateTransfer( void )
/*************************************************************************/
  /* This function interprets ConfirmedPrivateTransfer service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Private Transfer Request","%u");

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff & 0xF0) >> 4;
   if ((tagval == 0) && (tagbuff & 0x08)) {
      len = show_context_tag("Vendor ID");
      bac_show_byte(BACnetVendorID[pif_get_byte(0)], "%u");
   /*   show_bac_unsigned(len); */
      }
   else {
      pif_show_space();
      pif_show_ascii(0, " Error: Context Tag 0 expected !");
      }

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff & 0xF0) >> 4;
   if ((tagval == 1) && (tagbuff & 0x08)) {
      len = show_context_tag("Service Number");
      show_bac_unsigned(len);
      }
   else {
      pif_show_space();
      pif_show_ascii(0, " Error: Context Tag 1 expected !");
      }

   if(pif_offset < pif_end_offset) { /* optional service parameters */
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff & 0xF0) >> 4;
      if ((tagval == 2) && (tagbuff & 0x08)) {
         show_context_tag("Service Parameters");  /* opening tag */

         while (pif_offset < (pif_end_offset-1)) 
            show_application_data( pif_get_byte(0) );

         /*
            len = pif_end_offset - pif_offset - 1;
            sprintf(outstr,"Service Parameters (%u Octets)",len);
            bac_show_nbytes(len,outstr);
         */

         show_context_tag("Service Parameters");  /* closing tag */
         }
      else {
         pif_show_space();
         pif_show_ascii(0, " Error: Context Tag 2 expected !");
         }
      };
}

/*************************************************************************/
void show_confirmedTextMessage( void )
/*************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Confirmed Text Message Request","%u");
   while(pif_offset < pif_end_offset) {
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) {
        switch(tagval) {
           case 0:  show_context_tag("Text Message Source Device");
               show_bac_object_identifier();
               break;
           case 1:  show_context_tag("Message Class");  /* opening tag */
               switch((pif_get_byte(0)&0xF0)>>4) {
                  case 0: len = show_context_tag("Numeric Class");
                          show_bac_unsigned(len);
                          break;
                  case 1: len = show_context_tag("Character Class");
                          show_bac_charstring(len);
               }
               show_context_tag("Message Class");  /* opening tag */
               break;
           case 2:  show_context_tag("Message Priority");
               if(!pif_get_byte(0))
                  bac_show_byte("Normal","%u");
               else
                  bac_show_byte("Urgent","%u");
               break;
           case 3:  len = show_context_tag("Message");
               show_bac_charstring(len);
               break;
           default: len = show_context_tag("Unknown tag");
               bac_show_nbytes(len,"Error: Unknown data");
           }
        }
      else{  /* application tag */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }  
      } /* end of while */
}

/*************************************************************************/
void show_reinitializeDevice( void )
/*************************************************************************/
  /* This function interprets ReinitializeDevice service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Reinitialize Device Request","%u");
   while(pif_offset < pif_end_offset) {
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) { /* context tag */
        switch(tagval) {
           case 0:  show_context_tag("Reinitialized State of Device");
               if(!pif_get_byte(0))
                 bac_show_byte("Coldstart","%u");
               else
                 bac_show_byte("Warmstart","%u");
               break;
           case 1:  len = show_context_tag("Password");
               show_bac_charstring(len);
               break;
           default: len = show_context_tag("Unknown tag");
               bac_show_nbytes(len,"Error: Unknown data");
           }
        }
      else{  /* application tag */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }  
      }  /* end while */
}

/*************************************************************************/
void show_vtOpen( void )
/*************************************************************************/
  /* This function interprets VT-Open service requests */
{
   bac_show_byte("VT-Open Request","%u");
   pif_show_space();
   pif_show_ascii(0,"VT Class");
   show_application_tag(pif_get_byte(0));
   xsprintf(get_int_line(pi_data_current,pif_offset,1),
           "%"FW"s = %>ku %s", "VT Class Enumeration",
            BACnetVTClass[pif_get_byte(0)]);
   show_application_tag(pif_get_byte(0));
   bac_show_byte("Local VT Session Id","%u");

}

/*************************************************************************/
void show_vtClose( void )
/*************************************************************************/
  /* This function interprets VT-Close service requests */
{
   bac_show_byte("VT-Close Request","%u");
   pif_show_ascii(0,"List of Remote VT Session Identifiers");
   while(pif_offset < pif_end_offset) {
      show_application_tag(pif_get_byte(0));
      bac_show_byte("Session ID","%u");
   }
}

/*************************************************************************/
void show_vtData( void )
/*************************************************************************/
  /* This function interprets VT-Data service requests */
{
   unsigned int len;

   bac_show_byte("VT Data Request","%u");
   show_application_tag(pif_get_byte(0));
   bac_show_byte("VT Session ID","%u");

   pif_show_space();
   pif_show_ascii(0, "New VT Data");

   /* verify that vt data is an octet string */
   if((pif_get_byte(0) & 0xf0) != (OCTET_STRING * 16))
       pif_show_ascii(0, "Error: Octet String expected");

   pif_show_space();
   len = show_application_tag(pif_get_byte(0));
   sprintf(outstr,"VT New Data (%u Octets)",len);
   bac_show_nbytes(len,outstr);

   pif_show_space();
   show_application_tag(pif_get_byte(0));
   bac_show_byte("VT Data Flag (0/1)","%u");

}

/**************************************************************************/
void show_authenticate( void )
/**************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Authenticate Request","%u");
   while(pif_offset < pif_end_offset) {
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) { /* context tag */
        switch(tagval) {
           case 0:  len = show_context_tag("Pseudo Random Number");
               show_bac_unsigned(len);
               break;
           case 1:  len = show_context_tag("Expected Invoke ID");
               show_bac_unsigned(len);
               break;
           case 2:  len = show_context_tag("Operator Name");
               show_bac_charstring(len);
               break;
           case 3:  len = show_context_tag("Operator Password");
               show_bac_charstring(len);
               break;
           case 4:  /* start enciphered session flag */
               show_context_tag("Start Enciphered Session Flag");
               tagbuff = pif_get_byte(0);
               if(tagbuff)  /* TRUE */
                 xsprintf(get_int_line(pi_data_current,pif_offset,1),
                   "%"FW"s = %>ku %s", "Start Enciphered Session","(TRUE)");
               else
                 xsprintf(get_int_line(pi_data_current,pif_offset,1),
                   "%"FW"s = %>ku %s", "Start Enciphered Session","(FALSE)");
               pif_show_space();
               break;
           default: len = show_context_tag("Unknown tag");
               bac_show_nbytes(len,"Error: Unknown data");
           }
        }
      else{  /* application tag */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }  
      }  /* end while */

}

/**************************************************************************/
void show_requestKey( void )
/**************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Request Key Request","%u");

   /* requesting device identifier */
   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if (tagbuff & 0x08) { /* context tag found */
        pif_show_space();
        pif_show_ascii(0, "Error: Application Tag Expected!");
        pif_offset = pif_end_offset;
        }
      else {  /* application tag  */
        if (tagval == 12){
          pif_show_space();
          pif_show_ascii(0, "Requesting DeviceIdentifier");
          len =  show_application_tag(pif_get_byte(0));
          show_bac_object_identifier();
          }
        else {
          pif_show_space();
          pif_show_ascii(0, "Error: Application Tag 12 Expected!");
          pif_offset = pif_end_offset;
          }
      }

   /* requesting device address */
   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if (tagbuff & 0x08) { /* context tag found */
        pif_show_space();
        pif_show_ascii(0, "Error: Application Tag Expected!");
        pif_offset = pif_end_offset;
        }
      else {  /* application tag  */
        pif_show_space();
        pif_show_ascii(0, "Requesting Device Address");
        show_bac_address();
        }

   /* remote device identifer */
   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if (tagbuff & 0x08) { /* context tag found */
        pif_show_space();
        pif_show_ascii(0, "Error: Application Tag Expected!");
        pif_offset = pif_end_offset;
        }
      else {  /* application tag  */
        if (tagval == 12){
          pif_show_space();
          pif_show_ascii(0, "Remote Device Identifier");
          len =  show_application_tag(pif_get_byte(0));
          show_bac_object_identifier();
          }
        else {
          pif_show_space();
          pif_show_ascii(0, "Error: Application Tag 12 Expected!");
          pif_offset = pif_end_offset;
          }
      }

   /* remote device address */
   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if (tagbuff & 0x08) { /* context tag found */
        pif_show_space();
        pif_show_ascii(0, "Error: Application Tag Expected!");
        pif_offset = pif_end_offset;
        }
      else {  /* application tag  */
        pif_show_space();
        pif_show_ascii(0, "Remote Device Address");
        show_bac_address();
        }
}

/*************************************************************************/
void show_iAm( void )
/*************************************************************************/
  /* This function interprets I-Am service requests */
{
   unsigned int len;

   bac_show_byte("I-Am Request","%u");
   pif_show_ascii(0,"I-Am Device Identifier");
   show_application_tag(pif_get_byte(0));
   show_bac_object_identifier();

   pif_show_space();
   pif_show_ascii(0,"Maximum APDU Length Accepted");
   len = show_application_tag(pif_get_byte(0));
   show_bac_unsigned(len);

   /******* old version using header mapping approach
   if(len == 1)
     bac_show_byte("Max APDU Length Supported","%u");
   else
     bac_show_word_hl("Max APDU Length Supported","%u");
   *****************************/

   pif_show_space();
   pif_show_ascii(0,"Segmentation Supported");
   show_application_tag(pif_get_byte(0));
   bac_show_byte(BACnetSegmentation[pif_get_byte(0)],"%u");

   pif_show_space();
   pif_show_ascii(0,"BACnet Vendor ID");
   len = show_application_tag(pif_get_byte(0));
/*   show_bac_unsigned(len); */
   bac_show_byte(BACnetVendorID[pif_get_byte(0)], "%u");
}

/*************************************************************************/
void show_iHave( void )
/*************************************************************************/
  /* This function interprets I-Have service requests */
{
   bac_show_byte("I-Have Request","%u");
   pif_show_space();
   pif_show_ascii(0,"Device Identifier");
   show_application_tag(pif_get_byte(0));
   show_bac_object_identifier();

   pif_show_space();
   pif_show_ascii(0,"Object Identifier");
   show_application_tag(pif_get_byte(0));
   show_bac_object_identifier();

   pif_show_space();
   pif_show_ascii(0,"Object Name");
   show_application_data(pif_get_byte(0));
}

/**************************************************************************/
void show_unconfirmedCOVNotification( void )
/**************************************************************************/
  /* This function interprets unconfirmedCOVNotification requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Unconfirmed COV Notification Request","%u");

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if(tagval == 0){
      len = show_context_tag("Subscriber Process Identifier");
      show_bac_unsigned(len);
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 0 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if(tagval == 1){
      len = show_context_tag("Initiating Device Identifier");
      show_bac_object_identifier();
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 1 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if(tagval == 2){
      show_context_tag("Monitored Object Identifier");
      show_bac_object_identifier();
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 2 Expected!");
      };
   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if(tagval == 3){
      len = show_context_tag("Time Remaining (seconds)");
      show_bac_unsigned(len);
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 3 Expected!");
      };
   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if(tagval == 4){
      show_context_tag("List of Values");  /* opening tag */
      while(pif_offset < (pif_end_offset-1))
         show_bac_property_value();
      show_context_tag("List of Values");  /* closing tag */
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 4 Expected!");
      };
}

/*************************************************************************/
void show_unconfEventNotification( void )
/*************************************************************************/
  /* This function interprets UnconfirmedEventNotification service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len, notify_type;

   bac_show_byte("Unconfirmed Event Notification Request","%u");

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 0)){
      len = show_context_tag("Process Identifier");
      show_bac_unsigned(len);
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 0 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 1)){
      show_context_tag("Initiating Device Identifier");
      show_bac_object_identifier();
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 1 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 2)){
      show_context_tag("Event Object Identifier");
      show_bac_object_identifier();
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 2 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 3)){
      show_context_tag("Time Stamp");
      show_bac_timestamp();
      show_context_tag("Time Stamp");  /* show closing tag */
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 3 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 4)){
      show_context_tag("Notification Class");
      bac_show_byte("Class (1..255)","%u");
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 4 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 5)){
      show_context_tag("Priority");
      bac_show_byte("Priority (1..255)","%u");
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 5 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 6)){
      show_context_tag("Event Type");
      bac_show_byte(BACnetEventType[pif_get_byte(0)],"%u");
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 6 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 7)){
      len = show_context_tag("Message Text");
      show_bac_charstring(len);
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 8)){
      show_context_tag("Notify Type");
      notify_type = pif_get_byte(0);
      bac_show_byte(BACnetNotifyType[pif_get_byte(0)],"%u");
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 8 Expected!");
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 9)){
      show_context_tag("Acknowledgement Required");
      tagbuff = pif_get_byte(0);
      if(tagbuff)  /* TRUE */
         xsprintf(get_int_line(pi_data_current,pif_offset,1),
         "%"FW"s = %>ku %s", "Acknowledgement Required","(TRUE)");
      else
         xsprintf(get_int_line(pi_data_current,pif_offset,1),
         "%"FW"s = %>ku %s", "Acknowledgement Required","(FALSE)");
      pif_show_space();
      }
   else{ /* required for alarm or event notifications */
      if(notify_type != 2){
         pif_show_space();
         pif_show_ascii(0, "Error: Context Tag 9 Expected!");
         };
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 10)){
      show_context_tag("From State");
      bac_show_byte(BACnetEventState[pif_get_byte(0)], "%u");
      }
   else{  /* required for alarm or event notifications */
      if(notify_type != 2){
         pif_show_space();
         pif_show_ascii(0, "Error: Context Tag 10 Expected!");
         };
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 11)){
      show_context_tag("To State");
      bac_show_byte(BACnetEventState[pif_get_byte(0)], "%u");
      }
   else{  /* required for alarm or event notifications */
      if(notify_type != 2){
         pif_show_space();
         pif_show_ascii(0, "Error: Context Tag 11 Expected!");
         };
      };

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if((tagbuff & 0x08) && (tagval == 12)){
      show_context_tag("Event Values");
      tagbuff = pif_get_byte(0);
      tagval= (tagbuff&0xF0)>>4;
      switch(tagval){
         case 0:/* change of bitstring */
            show_context_tag("Change of Bitstring Event Parameters");  /* opening tag */

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               len = show_context_tag("Referenced Bitstring");
               show_bac_bitstring(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               len=show_context_tag("Status Flags");
               show_bac_status_flags(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };

            show_context_tag("Change of Bitstring Event Parameters");  /* closing tag */
            break;
         case 1: /* change of state */
            show_context_tag("Change of State Event Parameters");  /* opening tag */

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               show_context_tag("New State");
               show_bac_property_states();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               len=show_context_tag("Status Flags");
               show_bac_status_flags(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };

            show_context_tag("Change of State Event Parameters");  /* closing tag */
            break;
         case 2: /* change of value */
            show_context_tag("Change of Value Event Parameters");  /* opening tag */

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               show_context_tag("New Value");
               tagbuff = pif_get_byte(0);
               tagval = (tagbuff&0xF0)>>4;
               switch(tagval){
                  case 0:
                     len = show_context_tag("Changed Bits");
                     show_bac_bitstring(len);
                     break;
                  case 1:
                     show_context_tag("Changed Value");
                     show_bac_real();
                     break;
                  default:
                     pif_show_space();
                     pif_show_ascii(0, "Error: Context Tag 1 Expected!");
                     break;
                  };
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               len=show_context_tag("Status Flags");
               show_bac_status_flags(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };
            show_context_tag("Change of Value Event Parameters");  /* closing tag */
            break;
         case 3: /* command failure */
            show_context_tag("Command Failure Event Parameters");  /* opening tag */

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               show_context_tag("Command Value"); /* opening tag */

               tagbuff = pif_get_byte(0);
               if((tagbuff & 0x08) && ((tagbuff&0x0f) != 0x0e)){ /* SD context tag */
                  len = show_context_tag("Unknown Context");
                  bac_show_nbytes(len,"Unknown Data");
                  };

               if((tagbuff & 0x08) && ((tagbuff&0x0f) == 0x0e)){ /* PD context tag */
                  show_context_tag("Unknown Context"); /* opening tag */
                  len = 0;
                  tagbuff = tagbuff | 0x01; /* set patern for closing tag */
                  while(pif_get_byte(len) != tagbuff) len++;
                  bac_show_nbytes(len+1,"Unknown Data");
                  show_context_tag("Unknown Context"); /* closing tag */
                  };

               /* normal case */
               show_application_data(tagbuff);
               show_context_tag("Command Value"); /* closing tag */
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               len=show_context_tag("Status Flags");
               show_bac_status_flags(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 2)){
               show_context_tag("Feedback Value");

               tagbuff = pif_get_byte(0);
               if((tagbuff & 0x08) && ((tagbuff&0x0f) != 0x0e)){ /* SD context tag */
                  len = show_context_tag("Unknown Context");
                  bac_show_nbytes(len,"Unknown Data");
                  };

               if((tagbuff & 0x08) && ((tagbuff&0x0f) == 0x0e)){ /* PD context tag */
                  show_context_tag("Unknown Context"); /* opening tag */
                  len = 0;
                  tagbuff = tagbuff | 0x01; /* set patern for closing tag */
                  while(pif_get_byte(len) != tagbuff) len++;
                  bac_show_nbytes(len+1,"Unknown Data");
                  show_context_tag("Unknown Context"); /* closing tag */
                  };

               /* normal case */
               show_application_data(tagbuff);
               show_context_tag("Command Value"); /* closing tag */
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 2 Expected!");
               };

            show_context_tag("Command Failure Event Parameters");  /* closing tag */
            break;
         case 4: /* floating limit */
            show_context_tag("Floating Limit Event Parameters");  /* opening tag */
            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               len = show_context_tag("Referenced Bitstring");
               show_bac_bitstring(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               len=show_context_tag("Status Flags");
               show_bac_status_flags(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };
            show_context_tag("Floating Limit Event Parameters");  /* closing tag */
            break;
         case 5: /* out of range */
            show_context_tag("Out of Range Notification Parameters");  /* opening tag */
            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               show_context_tag("Exceeding Value");
               show_bac_real();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               len = show_context_tag("Status Flags");
               show_bac_status_flags(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 2)){
               show_context_tag("Deadband");
               show_bac_real();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 2 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 3)){
               show_context_tag("Exceeded Limit");
               show_bac_real();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 3 Expected!");
               };

            show_context_tag("Out of Range Notification Parameters");  /* closing tag */
            break;
         case 6: /* complex event type */
            show_context_tag("Complex Event Parameters");  /* opening tag */
            while(pif_offset < (pif_end_offset-2))
               show_bac_property_value();
            show_context_tag("Complex Event Parameters");  /* closing tag */
            break;
         default:
            pif_show_space();
            bac_show_nbytes(1,"Error: Invalid Context Tag (should be <= 6)!");
            break;
         };
      show_context_tag("Event Values");  /* closing tag */
      }
   else{
      pif_show_space();
      pif_show_ascii(0, "Error: Context Tag 12 Expected!");
      };
}

/*************************************************************************/
void show_unconfPrivateTransfer( void )
/*************************************************************************/
  /* This function interprets UnconfirmedPrivateTransfer service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Private Transfer Request","%u");

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff & 0xF0) >> 4;
   if ((tagval == 0) && (tagbuff & 0x08)) {
      len = show_context_tag("Vendor ID");
      bac_show_byte(BACnetVendorID[pif_get_byte(0)], "%u");
   /*   show_bac_unsigned(len); */
      }
   else {
      pif_show_space();
      pif_show_ascii(0, " Error: Context Tag 0 expected !");
      }

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff & 0xF0) >> 4;
   if ((tagval == 1) && (tagbuff & 0x08)) {
      len = show_context_tag("Service Number");
      show_bac_unsigned(len);
      }
   else {
      pif_show_space();
      pif_show_ascii(0, " Error: Context Tag 1 expected !");
      }

   if(pif_offset < pif_end_offset) { /* optional service parameters */
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff & 0xF0) >> 4;
      if ((tagval == 2) && (tagbuff & 0x08)) {
         show_context_tag("Service Parameters");  /* opening tag */

         while (pif_offset < (pif_end_offset-1)) 
            show_application_data( pif_get_byte(0) );

         /*
            len = pif_end_offset - pif_offset - 1;
            sprintf(outstr,"Service Parameters (%u Octets)",len);
            bac_show_nbytes(len,outstr);
         */

         show_context_tag("Service Parameters");  /* closing tag */
         }
      else {
         pif_show_space();
         pif_show_ascii(0, " Error: Context Tag 2 expected !");
         }
      };
}

/*************************************************************************/
void show_unconfTextMessage( void )
/*************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Unconfirmed Text Message Request","%u");
   while(pif_offset < pif_end_offset) {
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) {
        switch(tagval) {
           case 0:  show_context_tag("Text Message Source Device");
               show_bac_object_identifier();
               break;
           case 1:  show_context_tag("Message Class");  /* opening tag */
               switch((pif_get_byte(0)&0xF0)>>4) {
                  case 0: len = show_context_tag("Numeric Class");
                          show_bac_unsigned(len);
                          break;
                  case 1: len = show_context_tag("Character Class");
                          show_bac_charstring(len);
               }
               show_context_tag("Message Class");  /* opening tag */
               break;
           case 2:  show_context_tag("Message Priority");
               if(!pif_get_byte(0))
                  bac_show_byte("Normal","%u");
               else
                  bac_show_byte("Urgent","%u");
               break;
           case 3:  len = show_context_tag("Message");
               show_bac_charstring(len);
               break;
           default: len = show_context_tag("Unknown tag");
               bac_show_nbytes(len,"Error: Unknown data");
           }
        }
      else{  /* application tag */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }  
    }  /* end while */
}

/*************************************************************************/
void show_timeSynchronization( void )
/*************************************************************************/
  /* This function interprets Time Synchronization requests */
{
   bac_show_byte("Time Synchronization Request","%u");
   show_application_data(pif_get_byte(0)); /*Date*/
   show_application_data(pif_get_byte(0)); /*Time*/
}

/*************************************************************************/
void show_whoHas( void )
/*************************************************************************/
  /* This function interprets Who-Has service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Who-Has Request","%u");
   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;

   if (tagbuff & 0x08) { /* context tag */
      if(tagval == 0){ /* device instance range expected */
         len = show_context_tag("Device Instance Range Low Limit");
         show_bac_unsigned(len);
         tagbuff = pif_get_byte(0);
         tagval = (tagbuff&0xF0)>>4;

         if((tagbuff & 0x08) && (tagval == 1)){ /* range high limit tag */
            len = show_context_tag("Device Instance Range High Limit");
            show_bac_unsigned(len);
            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            }
         else{
            pif_show_space();
            pif_show_ascii(0, "Error: Context Tag 1 Expected!");
            pif_offset = pif_end_offset;
            };
         };

      switch(tagval) {
         case 2:
               show_context_tag("Object Identifier");
               show_bac_object_identifier();
               break;
         case 3: /* object name */
               len = show_context_tag("Object Name");
               show_bac_charstring(len);
               break;
         default: len = show_context_tag("Invalid Context Tag");
               bac_show_nbytes(len,"Error: Unknown data");
         };
      }
   else{  /* application tag */
     pif_show_space();
     pif_show_ascii(0, "Error: Context Tag Expected!");
     pif_offset = pif_end_offset;
     };
}

/*************************************************************************/
void show_whoIs( void )
/*************************************************************************/
  /* This function interprets Who-Is service requests */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Who-Is Request","%u");
   if (pif_offset < pif_end_offset) {
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if(tagval == 0){
         pif_show_space();
         len = show_context_tag("Device Instance Range Low Limit");
         show_bac_unsigned(len);
         }
      else{
        pif_show_ascii(0, "Error: Context Tag 0 Expected!");
        pif_offset = pif_end_offset;
        };

      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if(tagval == 1){
         pif_show_space();
         len = show_context_tag("Device Instance Range High Limit");
         show_bac_unsigned(len);
         }
      else{
        pif_show_ascii(0, "Error: Context Tag 1 Expected!");
        pif_offset = pif_end_offset;
        };

      }
   else
      pif_show_ascii(0,"Request for ALL Devices!");
}

/*************************************************************************/
void show_getAlarmSummaryACK( void )
/*************************************************************************/
  /* This function interprets GetAlarmSummary acknowledgements */
{
   bac_show_byte("Get Alarm Summary Acknowledgement","%u");
   while(pif_offset<pif_end_offset) {
      pif_show_space();
      pif_show_ascii(0,"Object Identifier");
      show_application_tag(pif_get_byte(0));
      show_bac_object_identifier();

      pif_show_space();
      pif_show_ascii(0,"Alarm State");
      show_application_tag(pif_get_byte(0));
      bac_show_byte(BACnetEventState[pif_get_byte(0)],"%u");

      pif_show_space();
      pif_show_ascii(0,"Acknowledged States");
      show_application_tag(pif_get_byte(0));
      show_bac_event_transitions_ackd();
   }
}

/*************************************************************************/
void show_getEnrollmentSummaryACK( void )
/*************************************************************************/
  /* This function interprets GetEnrollmentSummary acknowledgements */
{
   unsigned char tagbuff, tagclass; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Get Enrollment Summary Acknowledgement","%u");
   while(pif_offset<pif_end_offset) {
      pif_show_space();
      pif_show_ascii(0,"Object Identifier");
      show_application_tag(pif_get_byte(0));
      show_bac_object_identifier();

      pif_show_space();
      pif_show_ascii(0,"Event Type");
      show_application_tag(pif_get_byte(0));
      bac_show_byte(BACnetEventType[pif_get_byte(0)],"%u");

      pif_show_space();
      pif_show_ascii(0,"Event State");
      show_application_tag(pif_get_byte(0));
      bac_show_byte(BACnetEventState[pif_get_byte(0)],"%u");

      pif_show_space();
      pif_show_ascii(0,"Priority");
      len = show_application_tag(pif_get_byte(0));
      show_bac_unsigned(len);

      if (pif_offset<pif_end_offset) {
         tagbuff = pif_get_byte(0);
         tagclass = tagbuff&0x07;
         if (tagclass == 2) {
            pif_show_space();
            pif_show_ascii(0,"Notification Class");
            len = show_application_tag(pif_get_byte(0));
            show_bac_unsigned(len);
		 }
	  }
   }
}

/*************************************************************************/
void show_atomicReadFileACK( void )
/*************************************************************************/
  /* This function interprets AtomicReadFile acknowledgements */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Atomic Read File Acknowledgement","%u");
   pif_show_space();
   if(pif_get_byte(0)&0x07)
      show_str_eq_str("End of File","TRUE",1);
   else
      show_str_eq_str("End of File","FALSE",1);
   pif_offset++;
   while(pif_offset < pif_end_offset) {
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) {
        switch(tagval) {
           case 0:
               if((tagbuff & 0x0f) == 0x0e){
                  show_context_tag("Stream Access"); /* opening tag */
                  pif_show_space();
                  pif_show_ascii(0,"File Start Position");
                  show_application_data(pif_get_byte(0));
                  pif_show_space();
                  pif_show_ascii(0,"File Data");
                  /* OCTET STRING follows */
                  }
               else
                  show_context_tag("Stream Access");  /* closing tag */
               break;
           case 1:
               if((tagbuff & 0x0f) == 0x0e){
                  show_context_tag("Record Access");  /* opening tag */
                  pif_show_space();
                  pif_show_ascii(0,"File Start Record");
                  show_application_data(pif_get_byte(0));
                  pif_show_space();
                  pif_show_ascii(0,"Returned Record Count");
                  show_application_data(pif_get_byte(0));
                  pif_show_space();
                  pif_show_ascii(0,"File Record Data");
                  /* Sequence of OCTET STRINGS follows */
                  }
               else
                  show_context_tag("Record Access");  /* closing tag */
               break;
           default: len = show_context_tag("Unknown tag");
               bac_show_nbytes(len,"Error: Unknown data");
           }
        }
      else
        show_application_data(tagbuff);
      }
}

/*************************************************************************/
void show_atomicWriteFileACK( void )
/*************************************************************************/
  /* This function interprets AtomicWriteFile acknowledgements */
{
   unsigned int len;

   bac_show_byte("Atomic Write File Acknowledgement","%u");
   switch((pif_get_byte(0)&0xf0)>>4) {
      case 0: len = show_context_tag("File Start Position");
         show_bac_unsigned(len);
         break;
      case 1: len = show_context_tag("File Start Record");
         show_bac_unsigned(len);
   }
}

/*************************************************************************/
void show_createObjectACK( void )
/*************************************************************************/
  /* This function interprets CreateObject acknowledgements */
{
   bac_show_byte("CreateObject Acknowledgement","%u");
   show_application_data(pif_get_byte(0));
}

/*************************************************************************/
void show_readPropertyACK( void )
/*************************************************************************/
  /* This function interprets ReadProperty acknowledgements */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;
   int obj_type,prop_idx,prop_id;

   prop_idx = -1;  /* initialize to indicate no array index present */

   bac_show_byte("Read Property Acknowledgement","%u");

   while (pif_offset < pif_end_offset) {
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (tagbuff & 0x08) {
    if(tagval > 4) {
       pif_show_space();
       bac_show_nbytes(1,"Error: Invalid Context Tag (should be <= 4)!");
       goto exit;
    }
    switch(tagval) {
       case 0:  show_context_tag("Object Identifier");
           obj_type = bac_extract_obj_type();
           show_bac_object_identifier();
           break;
       case 1:  len = show_context_tag("Property Identifier");
           if (len == 1)
             prop_id = (int)pif_get_byte(0);
           else
             prop_id = (int)pif_get_word_hl(0);
           show_bac_property_identifier(len);
           break;
       case 2:  len = show_context_tag("Property Array Index");
           prop_idx = pif_get_byte(0);
           show_bac_unsigned(len);
           break;
       case 3:  show_context_tag("Property Value");  /* opening tag */
           show_bac_ANY(obj_type,prop_id,prop_idx);
           show_context_tag("Property Value");  /* closing tag */
           break;
       default: len = show_context_tag("Unknown tag");
           bac_show_nbytes(len,"Error: Unknown data");
    }
      }
      else{  /* application tag */
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag Expected!");
        pif_offset = pif_end_offset;
        }  
   }
exit:;
}

/*************************************************************************/
void show_readPropertyConditionalACK( void )
/*************************************************************************/
  /* This function interprets ReadPropertyConditional acknowledgements */
{
   bac_show_byte("Read Property Conditional Acknowledgement","%u");
   show_bac_read_access_result();
}

/*************************************************************************/
void show_readPropertyMultipleACK( void )
/*************************************************************************/
  /* This function interprets ReadPropertyMultiple acknowledgements */
{
   bac_show_byte("Read Property Multiple Acknowledgement","%u");
   show_bac_read_access_result();
}

/*************************************************************************/
void show_conf_PrivateTransferACK( void )
/*************************************************************************/
  /* This function interprets ConfirmedPrivateTransfer acknowledgements */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   bac_show_byte("Private Transfer Acknowledgement","%u");

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff & 0xF0) >> 4;
   if ((tagval == 0) && (tagbuff & 0x08)) {
      len = show_context_tag("Vendor ID");
      bac_show_byte(BACnetVendorID[pif_get_byte(0)], "%u");
   /*   show_bac_unsigned(len); */
      }
   else {
      pif_show_space();
      pif_show_ascii(0, " Error: Context Tag 0 expected !");
      }

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff & 0xF0) >> 4;
   if ((tagval == 1) && (tagbuff & 0x08)) {
      len = show_context_tag("Service Number");
      show_bac_unsigned(len);
      }
   else {
      pif_show_space();
      pif_show_ascii(0, " Error: Context Tag 1 expected !");
      }

   if(pif_offset < pif_end_offset) { /* optional service parameters */
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff & 0xF0) >> 4;
      if ((tagval == 2) && (tagbuff & 0x08)) {
         show_context_tag("Result Block");  /* opening tag */

         while (pif_offset < (pif_end_offset-1)) 
            show_application_data( pif_get_byte(0) );

         /*
            len = pif_end_offset - pif_offset - 1;
            sprintf(outstr,"Service Parameters (%u Octets)",len);
            bac_show_nbytes(len,outstr);
         */

         show_context_tag("Result Block");  /* closing tag */
         }
      else {
         pif_show_space();
         pif_show_ascii(0, " Error: Context Tag 2 expected !");
         }
      };
}

/*************************************************************************/
void show_vtOpenACK( void )
/*************************************************************************/
  /* This function interprets VT-Open acknowledgements */
{
   bac_show_byte("VT Open Acknowledgement","%u");
   show_application_tag(pif_get_byte(0));
   bac_show_byte("Remote VT Session Id","%u");
}

/*************************************************************************/
void show_vtDataACK( void )
/*************************************************************************/
  /* This function interprets VT-Data acknowledgements */
{
   unsigned int len;
   unsigned char tagbuff;

   bac_show_byte("VT Data Acknowledgement","%u");
   show_context_tag("All New Data Accepted");
   tagbuff = pif_get_byte(0);
   if(tagbuff)  /* TRUE */
     xsprintf(get_int_line(pi_data_current,pif_offset,1),
       "%"FW"s = %>ku %s", "All New Data Accepted","(TRUE)");
   else
     xsprintf(get_int_line(pi_data_current,pif_offset,1),
       "%"FW"s = %>ku %s", "All New Data Accepted","(FALSE)");
   pif_show_space();
   if(pif_end_offset != pif_offset){ /* display accepted octet count if present */
     len = show_context_tag("Accepted Octet Count");
     show_bac_unsigned(len);
   }
}

/*************************************************************************/
void show_authenticateACK( void )
/*************************************************************************/
  /* This function interprets Authenticate acknowledgements */
{
   bac_show_byte("Authenticate Acknowledgement","%u");
   pif_show_space();
   pif_show_ascii(0,"Modified Random Number");
   show_application_data(pif_get_byte(0));
}


/*************************************************************************/
void show_createObjectError( void )
/*************************************************************************/
  /* This function displays CreateObject errors, AddListElement errors and
     RemoveListElement errors */
{
  unsigned int len;

  show_context_tag("Error");	/* opening tag */
  show_error_codes();
  show_context_tag("Error");	/* closing tag */

  len = show_context_tag("First Failed Element Number");
  show_bac_unsigned(len);
}

/*************************************************************************/
void show_error_codes( void )
/*************************************************************************/
  /* This function displays Error Codes and Classes */
{  /* unsigned service;
      service = pif_get_byte(-1);
      show_context_tag(BACnetError[service]);  /* opening tag */

   pif_show_ascii(0,"Error Class");
   show_application_tag(pif_get_byte(0));
   bac_show_byte(BACnetErrorClass[pif_get_byte(0)],"%u");
   pif_show_ascii(0,"Error Code");
   show_application_tag(pif_get_byte(0));
   bac_show_byte(BACnetErrorCode[pif_get_byte(0)],"%u");

   /* show_context_tag(BACnetError[service]);  /* closing tag */
}

/*************************************************************************/
void show_writePropertyMultipleError( void )
/*************************************************************************/
  /* This function interprets WritePropertyMultiple errors */
{
   show_context_tag("Error Type"); /* opening tag */
   show_error_codes();
   show_context_tag("Error Type"); /* closing tag */

   show_context_tag("First Failed Write Access Specification"); /* opening tag */
   show_bac_obj_prop_ref();
   show_context_tag("First Failed Write Access Specification"); /* closing tag */
}

/*************************************************************************/
void show_vtCloseError( void )
/*************************************************************************/
  /* This function interprets VT-Close errors */
{
   show_context_tag("Error Type"); /* opening tag */
   show_error_codes();
   show_context_tag("Error Type"); /* closing tag */

   show_context_tag("List of VT Session IDs");	/* opening tag */
   while (pif_offset<pif_end_offset-1) {
      show_application_tag(pif_get_byte(0));
      bac_show_byte("Unclosable VT Session ID","%u");
   }
   show_context_tag("List of VT Session IDs");	/* closing tag */
}

/**************************************************************************/
/* The functions that follow are used to display BACnet application
   (primitive) data     */
/* types. They may be called following context tags where the IMPLICIT     */
/* keyword is presumed (and therefore no primitive tag is available to     */
/* determine length or data type) or from show_application_tag().          */
/**************************************************************************/

/**************************************************************************/
void show_bac_bitstring( unsigned int len )
/**************************************************************************/
/* Displays a bit string with no interpretation of the bit semantics */
{
   unsigned int i;

   /* the tag should be displayed before calling this function */
   bac_show_byte("Number of unused bits","%u");
   sprintf(outstr,"%"FW"s = X'%%02X'","Bitstring Values");
   for(i=0; i<len-1; i++){
     bac_show_flag(outstr,0xFF);
     pif_show_flagbit(0x80,"","");
     pif_show_flagbit(0x40,"","");
     pif_show_flagbit(0x20,"","");
     pif_show_flagbit(0x10,"","");
     pif_show_flagbit(0x08,"","");
     pif_show_flagbit(0x04,"","");
     pif_show_flagbit(0x02,"","");
     pif_show_flagbit(0x01,"","");
     };
}

/**************************************************************************/
void show_bac_charstring( unsigned int len)
/**************************************************************************/
{
   unsigned char charset;
   
   charset = pif_get_byte(0);
   switch(charset){
      case 0: /* ASCII */
             bac_show_byte("ASCII Character Encoding","%u");
             sprintf(outstr,"%"FW"s = %%s","Character string");
             pif_show_ascii(len-1, outstr);
             break;
      case 1: /* MS DBCS */
             bac_show_byte("MS Double Byte Character Encoding","%u");
             sprintf(outstr,"%"FW"s = `%%s'","Character string");
             pif_show_nbytes_hex(outstr, len-1);
             break;
      default:  /* invalid character set */
             sprintf(pif_line(0),"Error: Invalid Chacter Set!");
      };
}


/**************************************************************************/
void show_bac_date( void )
/**************************************************************************/
{
   unsigned char x;

   x = pif_get_byte(0);
   if (x == 255)
      sprintf(outstr,"Unspecified");
   else
      sprintf(outstr,"%u",1900+x);
   show_str_eq_str("Year",outstr,1);
   pif_offset++;

   x = pif_get_byte(0);
   if (x == 255)
      sprintf(outstr,"Unspecified");
   else {
      if ((x>0) && (x<13)) sprintf(outstr,month[x]);
      else sprintf(outstr,month[0]);
      }
   show_str_eq_str("Month",outstr,1);
   pif_offset++;

   x = pif_get_byte(0);
   if (x == 255)
      sprintf(outstr,"Unspecified");
   else
      sprintf(outstr,"%u",x);
   show_str_eq_str("Day of Month",outstr,1);
   pif_offset++;

   x = pif_get_byte(0);
   if (x == 255)
      sprintf(outstr,"Unspecified");
   else {
      if ((x>0) && (x<8)) sprintf(outstr,day_of_week[x]);
      else sprintf(outstr,day_of_week[0]);
   }
   show_str_eq_str("Day of Week",outstr,1);
   pif_offset++;
}

/**************************************************************************/
void show_bac_double( void )
/**************************************************************************/
{  /*  add interpreter code for double precision reals */
}

/**************************************************************************/
void show_bac_real( void )
/**************************************************************************/
{
   double dx;
   unsigned char fstr[4];
   char float_str[20];
   unsigned int i;

   for (i=0;i<4;i++) fstr[i] = pif_get_byte(3-i);
     dx = (double)(*(float *)fstr);
     float_to_ascii(dx,float_str);
     sprintf(outstr,"%"FW"s = %%s","Value of float");
     sprintf(get_int_line(pi_data_bacnet_AL,
        pif_offset,4),
        outstr,float_str);
     pif_offset += 4;
}

/**************************************************************************/
void show_bac_signed ( unsigned int len )
/**************************************************************************/
{
   switch (len) {
     case 1: bac_show_byte("Value (1-octet signed)","%d");
        break;
     case 2: sprintf(outstr,"%"FW"s = %%d",
           "Value (2-octet signed)");
        pif_show_word_hl(outstr);
        break;
     case 4: sprintf(outstr,"%"FW"s = %%ld",
           "Value (4-octet signed)");
        pif_show_long_hl(outstr);
   }
}

/**************************************************************************/
void show_bac_time( void )
/**************************************************************************/
{
   unsigned char x;
   char tempstr[80];

   x = pif_get_byte(0);
   if (x == 255)
      sprintf(outstr,"XX:");
   else
      sprintf(outstr,"%u:",x);

   x = pif_get_byte(1);
   if (x == 255)
      strcat(outstr,"XX:");
   else {
      sprintf(tempstr,"%02u:",x);
      strcat(outstr,tempstr);
   }

   x = pif_get_byte(2);
   if (x == 255)
      strcat(outstr,"XX.");
   else {
      sprintf(tempstr,"%02u.",x);
      strcat(outstr,tempstr);
   }

   x = pif_get_byte(3);
   if (x == 255)
      strcat(outstr,"XX");
   else {
      sprintf(tempstr,"%02u",x);
      strcat(outstr,tempstr);
   }
   
   strcat(outstr,"  (\"XX\" = Unspecified)");
   show_str_eq_str("Time",outstr,4);
   pif_offset += 4;
}

/**************************************************************************/
void show_bac_unsigned( unsigned int len )
/**************************************************************************/
{
   switch (len) {
     case 1: bac_show_byte("Value (1-octet unsigned)","%u");
        break;
     case 2: sprintf(outstr,"%"FW"s = %%u",
        "Value (2-octet unsigned)");
        pif_show_word_hl(outstr);
        break;
     case 3: sprintf(outstr,"%"FW"s = %%lu",
        "Value (3-octet unsigned)");
        sprintf(get_int_line(pi_data_current,pif_offset,3),outstr,
                (pif_get_long_hl(-1)&0x00FFFFFF));
		pif_offset += 3;
        break;
     case 4: sprintf(outstr,"%"FW"s = %%lu",
        "Value (4-octet unsigned)");
        pif_show_long_hl(outstr);
     }
}

/**************************************************************************/
unsigned long get_bac_unsigned( int delta, int len )
/**************************************************************************/
/* Returns as an unsigned long any 1,2,3, or 4-octed unsigned integer     */
{
   switch (len) {
     case 1: return (long)pif_get_byte(delta);
     
     case 2: return (long)pif_get_word_hl(delta);

     case 3: return pif_get_long_hl(delta-1)&0x00FFFFFF;

     case 4: return pif_get_long_hl(delta);
     }
   return (long)0;
}

/*************************************************************************/
/* These functions are used to interpret context specific and application */
/* tags.                                                                  */
/*************************************************************************/

/*************************************************************************/
unsigned int show_context_tag( char *tagstr )
/*************************************************************************/
{
   unsigned char tagbuff;    /* a buffer for tags */
   unsigned char lloc;       /* length location flag */
   unsigned char tloc;       /* tag location flag */
   unsigned char type;       /* tag type */
   unsigned int len;         /* tag length */
   unsigned int x;           /* temp variable */

   tagbuff = pif_get_byte(0);
   pif_show_space();
   type = (tagbuff & 0xF0)>>4;
   if (type <  15)    /* extended length? */
      tloc = 0;       /* no tag extension */
   else {
      tloc = 1;       /* tag extension */
      type = pif_get_byte(1);
   }
   len = (tagbuff & 0x07);
   if(len == 5)
      lloc = 1;   /* length follows tag */
   else
      lloc = 0; 

   bac_show_ctag_flag();
   if(type < 15) {
      bac_show_flagmask(0xF0,tagstr);
      pif_show_flagbit(0x08, "Context Specific Tag", "Application Tag");

      if(len < 5)    bac_show_flagmask(0x07,"Length = %d");
      if(len == 5) pif_show_flagmask(0x07, 0x05, "Extended Length");
      if(len == 6) pif_show_flagmask(0x07, 0x06, "Opening Tag");
      if(len == 7) pif_show_flagmask(0x07, 0x07, "Closing Tag");

      }
   else {
      bac_show_flagmask(0xF0,"Extended Tag = %d");
      if(len < 5)    bac_show_flagmask(0x07,"Length = %d");
      if(len == 5) pif_show_flagmask(0x07, 0x07, "Extended Length");
      if(len == 6) pif_show_flagmask(0x07, 0x07, "Opening Tag");
      if(len == 7) pif_show_flagmask(0x07, 0x07, "Closing Tag");

      pif_show_byte("Non-standard type           = %u");
   };
   if(lloc == 1) {
      x = pif_get_byte(0);
      switch(x) {
         case 254: pif_show_byte("Length in next 2 octets");
                   len = pif_get_word_hl(0);
                   pif_show_word_hl("Length of data              = %u");
                   break;
/* I'm commenting this out for now since it is a bit ludicrous and requires
   returning a LONG INT ...
         case 255: pif_show_byte("Length in next 4 octets");
                   len = pif_get_long_hl(0);
         pif_show_long_hl("Length of data              = %lu");
         break;                   */
         default:  len = x;
         pif_show_byte("Length of data              = %u");
      }
   }
   return(len);
}

/**************************************************************************/
void check_ctag_length( unsigned char tag, unsigned int expected, unsigned int actual )
/**************************************************************************/
{
   if (expected != actual) {
      pif_show_space();
      sprintf(pif_line(0),"Error: Invalid Context Tag Length!");
      sprintf(outstr,"%"FW"s  = %%u","     Tag [%u] length");
      sprintf(pif_line(0),outstr,tag,expected);
      sprintf(outstr,"%"FW"s = %%u","     Actual data length");
      sprintf(pif_line(0),outstr,actual);
   }
}

/**************************************************************************/
unsigned int show_application_data ( unsigned char tagbuff )
/**************************************************************************/
{
   unsigned int len;
   unsigned char type;
   unsigned char lloc,tloc;

   type = (tagbuff & 0xF0)>>4;
   if (type < 15)                   /* Extended type? */
     tloc = 0;                      /* Type in tag octet */  
   else {
     tloc = 1;                      /* Type in first octet following */
     type = pif_get_byte(1); 
   }
   len = (tagbuff & 0x07);
   if (len == 5)                     /* Extended length? */
     lloc = 1;             /* Length follows tag  */
   else
     lloc = 0;             /* Length in tag octet */

   pif_show_space();
   sprintf(outstr,"%"FW"s = X'%%02X' = [%u]","Application Tag",
      ((tagbuff&0xF0)>>4));
   
   bac_show_flag(outstr,0xFF);
   pif_show_flagmask(0xF0,0,"Null");
   pif_show_flagmask(0xF0,BOOLEAN*16,"Boolean");
   pif_show_flagmask(0xF0,UNSIGNED*16,"Unsigned Integer");
   pif_show_flagmask(0xF0,SIGNED*16,"Signed Integer");
   pif_show_flagmask(0xF0,REAL*16,"IEEE Floating Point");
   pif_show_flagmask(0xF0,DOUBLE*16,"IEEE Double Floating Point");
   pif_show_flagmask(0xF0,OCTET_STRING*16,"Octet String");
   pif_show_flagmask(0xF0,CHARACTER_STRING*16,"Character String");
   pif_show_flagmask(0xF0,BIT_STRING*16,"Bit String");
   pif_show_flagmask(0xF0,ENUMERATED*16,"Enumerated");
   pif_show_flagmask(0xF0,DATE*16,"Date");
   pif_show_flagmask(0xF0,TIME*16,"Time");
   pif_show_flagmask(0xF0,OBJECT_IDENTIFIER*16,"Object Identifier");
   pif_show_flagmask(0xF0,0x0d*16,"Reserved for ASHRAE");
   pif_show_flagmask(0xF0,0x0e*16,"Reserved for ASHRAE");
   pif_show_flagmask(0xF0,0x0f*16,"Non-standard type");
   pif_show_flagbit(0x08,"Context Specific Tag","Application Tag");

   switch (type) {
     case 0: pif_show_flagbit(0x07,"Unused","Unused");
             len = 0;
             break;
     case 1: pif_show_flagbit(0x07,"TRUE","FALSE");
             len = 0;
             break;
     default: if (len < 5)
                 bac_show_flagmask(0x07,"Length = %d");
              else
                 pif_show_flagmask(0x07,0x05, "Extended Length");
   }
   if (tloc == 1) bac_show_byte("Non-standard type","%u");
   if ((lloc != 0) && (type > 1)) {
      switch (len = pif_get_byte(0)) {
        case 254: pif_show_byte("Length in next 2 octets");
                  len = pif_get_word_hl(0);
                  sprintf(outstr,"%"FW"s = %%u","Length of data");
                  pif_show_word_hl(outstr);
                  break;

        /* Again, I'm commenting out this case...
        case 255: pif_show_byte("Length in next 4 octets");
                  len = pif_get_long_hl(0);
                  sprintf(outstr,"%"FW"s = %%lu","Length of data");
                  pif_show_long_hl(outstr);
                  break;                        */

        default: bac_show_byte("Length of data","%u");
       }
   }
   if (type < 15) {
     switch (type) {
       case UNSIGNED: show_bac_unsigned(len);
                      break;
       case SIGNED:   show_bac_signed(len);
                      break;
       case REAL:     show_bac_real();
                      break;
       case DOUBLE:   show_bac_double();
                      break;
       case BIT_STRING:
                      sprintf(outstr,"%"FW"s = %%s","Bit string");
                      pif_show_nbytes_hex(outstr,len);
                      break;
       case CHARACTER_STRING:
                      show_bac_charstring(len);
                      break;
       case OCTET_STRING:
                      sprintf(outstr,"%"FW"s = X'%%s'","Octet string");
                      pif_show_nbytes_hex(outstr,len);
                      break;
       case ENUMERATED:
                      switch (len) {
                         case 1: bac_show_byte("Enumeration","%d");
                                 break;
                         case 2: sprintf(outstr,"%"FW"s = %%d","Enumeration");
                                 pif_show_word_hl(outstr);
                                 break;
                      }
                      break;
       case DATE:     show_bac_date();
                      break;
       case TIME:     show_bac_time();
                      break;
       case OBJECT_IDENTIFIER:
                      show_bac_object_identifier();
                      break;
       case 13: case 14:  /* reserved */
                      pif_show_ascii(0,"The tag value is reserved for ASHRAE");
                      break;
       case 15:       /* non-standard */
                      bac_show_nbytes(len,"Non-standard data!");
     }  /* end of type switch */
   }    /* end of if(type) */
   return(len);
}  

/**************************************************************************/
unsigned int show_application_tag( unsigned char tagbuff )
/**************************************************************************/
{
   unsigned int len;
   unsigned char type;
   unsigned char lloc,tloc;

   type = (tagbuff & 0xF0)>>4;
   if (type < 15)                   /* Extended type? */
     tloc = 0;                      /* Type in tag octet */  
   else {
     tloc = 1;                      /* Type in first octet following */
     type = pif_get_byte(1); 
   }
   len = (tagbuff & 0x07);
   if (len == 5)                     /* Extended length? */
     lloc = 1;             /* Length follows tag */
   else
     lloc = 0;             /* Length in tag octet */

   sprintf(outstr,"%"FW"s = X'%%02X'","Application Tag");
   bac_show_flag(outstr,0xFF);
   pif_show_flagmask(0xF0,0x00,"Null");
   pif_show_flagmask(0xF0,BOOLEAN*16,"Boolean");
   pif_show_flagmask(0xF0,UNSIGNED*16,"Unsigned Integer");
   pif_show_flagmask(0xF0,SIGNED*16,"Signed Integer");
   pif_show_flagmask(0xF0,REAL*16,"IEEE Floating Point");
   pif_show_flagmask(0xF0,DOUBLE*16,"IEEE Double Floating Point");
   pif_show_flagmask(0xF0,OCTET_STRING*16,"Octet String");
   pif_show_flagmask(0xF0,CHARACTER_STRING*16,"Character String");
   pif_show_flagmask(0xF0,BIT_STRING*16,"Bit String");
   pif_show_flagmask(0xF0,ENUMERATED*16,"Enumerated");
   pif_show_flagmask(0xF0,DATE*16,"Date");
   pif_show_flagmask(0xF0,TIME*16,"Time");
   pif_show_flagmask(0xF0,OBJECT_IDENTIFIER*16,"Object Identifier");
   pif_show_flagmask(0xF0,0x0d*16,"Reserved for ASHRAE");
   pif_show_flagmask(0xF0,0x0e*16,"Reserved for ASHRAE");
   pif_show_flagmask(0xF0,0x0f*16,"Non-standard type");
   pif_show_flagbit(0x08,"Context Specific Tag","Application Tag");

   switch (type) {
     case 0:  pif_show_flagbit(0x07,"Unused","Unused");
         len = 0;
         break;
     case BOOLEAN:  pif_show_flagbit(0x07,"TRUE","FALSE");
         len = 0;
         break;
     default: if (len < 5)
                bac_show_flagmask(0x07,"Length = %d");
              else
                pif_show_flagmask(0x07,0x07, "Extended Length");
   }
   if (tloc == 1) bac_show_byte("Non-standard type","%u");
   if ((lloc != 0) && (type > 1)) {
      switch (len = pif_get_byte(0)) {
        case 254: pif_show_byte("Length in next 2 octets");
                  len = pif_get_word_hl(0);
                  sprintf(outstr,"%"FW"s = %%u","Length of data");
                  pif_show_word_hl(outstr);
                  break;

        /* Again, I'm commenting out this case...
        case 255: pif_show_byte("Length in next 4 octets");
                  len = pif_get_long_hl(0);
                  sprintf(outstr,"%"FW"s = %%lu","Length of data");
                  pif_show_long_hl(outstr);
                  break;                        */

        default: bac_show_byte("Length of data","%u");
      }
   }
   return(len);
}

/*************************************************************************/
/* These functions are used to interpret all of the BACnet base types    */
/* that appear either as parameters in services or as properties.    */
/*************************************************************************/


/**************************************************************************/
void show_bac_action_command( unsigned int len )
/**************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int end, len1;
   end = pif_offset+len;
   while((unsigned int)pif_offset < end) {
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if (!(tagbuff & 0x80)) {
    pif_show_space();
    bac_show_nbytes(1,"Error: Context Tag expected!");
    goto exit;
      }
      if (tagbuff & 0x80) {
    if(tagval > 6) {
       pif_show_space();
       bac_show_nbytes(1,"Error: Invalid Context Tag (should be <= 6)!");
       goto exit;
    }
    len1 = show_context_tag(BACnetActionCommand[tagval]);
    switch (tagval) {
       case 0:  show_bac_object_identifier();
           show_application_data(pif_get_byte(0));
           break;
       case 1:  show_bac_object_identifier();
           show_application_data(pif_get_byte(0));
           break;
       case 2:  show_bac_object_type();
           break;
       case 3:  show_bac_property_identifier(len1);
           break;
       case 4:  show_bac_unsigned(len1);
           break;
       case 5:  show_application_data(pif_get_byte(0));
           break;
       case 6:  show_bac_unsigned(len1);
    }
      }
      else {
    pif_show_space();
    bac_show_nbytes(1,"Error: Context Tag expected!");
      }
   }
   exit:;
}

/**************************************************************************/
void show_bac_address( void )
/**************************************************************************/
{
   unsigned int			net = 0, len;
   const char			*name;
   unsigned int			i;

   pif_show_ascii(0,"Network Number");
   len = show_application_tag(pif_get_byte(0));

   /* extract the network number and save it */
   for (i = 0; i < len; i++)
	   net = (net << 8) + pif_get_byte(i);
   show_bac_unsigned(len);

   pif_show_ascii(0,"MAC Address");
   len = show_application_tag(pif_get_byte(0));

   name = LookupName( net, (const unsigned char *)msg_origin + pif_offset, len );
   pif_show_nbytes_hex("MAC address        = X'%s'",len);

   if (name) pif_append_ascii( ", %s", name );
}

/**************************************************************************/
void show_bac_calendar_entry( void )
/**************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if (tagbuff & 0x08) {
      if (tagval > 2) {
         pif_show_space();
         bac_show_nbytes(1,"Error: Invalid Choice (should be 0,1 or 2)!");
         goto exit;
         };
      show_context_tag(BACnetCalendarEntry[tagval]);
      switch (tagval) {
         case 0: show_bac_date();
                 break;
         case 1: pif_show_ascii(0,BACnetDateRange[0]);
                 show_application_data(pif_get_byte(0));
                 pif_show_space();
                 pif_show_ascii(0,BACnetDateRange[1]);
                 show_application_data(pif_get_byte(0));
                 show_context_tag(BACnetCalendarEntry[tagval]); /* closing tag */
                 break;
         case 2: show_bac_weeknday();
                 show_context_tag(BACnetCalendarEntry[tagval]); /* closing tag */
         }
      }
   else {
      pif_show_space();
      bac_show_nbytes(1,"Error: Context Tag expected!");
      }
   exit:;
}

/**************************************************************************/
void show_bac_event_parameters( void )
/**************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if (tagbuff & 0x08) {
      if (tagval > 5) {
        pif_show_space();
        bac_show_nbytes(1,"Error: Invalid Tag Number (should be 0-5)!");
        goto exit;
        };
      len = show_context_tag(BACnetEventParameter[tagval]); /* opening tag */
      switch (tagval) {
         case 0: /* change of bitstring */
            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               len = show_context_tag("Time Delay");
               show_bac_unsigned(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               len = show_context_tag("Bitmask");
               show_bac_bitstring(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 2)){
               show_context_tag("List of Bitstring Values");  /* opening tag */
               while(tagbuff != 0x2f){
                  len = show_application_tag(pif_get_byte(0));
                  show_bac_bitstring(len);
                  tagbuff = pif_get_byte(0);
                  };
               show_context_tag("List of Bitstring Values");  /* closing tag */
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 2 Expected!");
               };
            break;

         case 1: /* change of state */
            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               len = show_context_tag("Time Delay");
               show_bac_unsigned(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               len = show_context_tag("List of Values"); /* opening tag */
               while(tagbuff != 0x1f){
                  show_bac_property_states();
                  tagbuff = pif_get_byte(0);
                  };
               len = show_context_tag("List of Values"); /* opening tag */
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };
            break;

         case 2: /* change of value */
            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               len = show_context_tag("Time Delay");
               show_bac_unsigned(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               len = show_context_tag("COV Criteria");  /* opening tag */
               tagbuff = pif_get_byte(0);
               tagval = (tagbuff&0xF0)>>4;
               switch(tagval){
                  case 0:  /* bitmask */
                     len = show_context_tag("Bitmask");
                     show_bac_bitstring(len);
                     break;
                  case 1:  /* referenced property increment */
                     show_context_tag("Referenced Property Increment");
                     show_bac_real();
                     break;
                  default:
                     pif_show_space();
                     pif_show_ascii(0, "Error: Context Tag 0 or 1 Expected!");
                     break;
                  };  /* end of cov criteria switch */
               len = show_context_tag("COV Criteria");  /* closing tag */
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };
            break;

         case 3: /* command failure */
            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               len = show_context_tag("Time Delay");
               show_bac_unsigned(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               show_context_tag("Feedback Property Reference"); /* opening tag */
               show_bac_obj_prop_ref();
               show_context_tag("Feedback Property Reference"); /* closing tag */
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };
            break;

         case 4: /* floating limit */
            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               len = show_context_tag("Time Delay");
               show_bac_unsigned(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               show_context_tag("Setpoint Reference"); /* opening tag */
               show_bac_obj_prop_ref();
               show_context_tag("Setpoint Reference"); /* closing tag */
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 2)){
               show_context_tag("Low Diff Limit");
               show_bac_real();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 2 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 3)){
               show_context_tag("High Diff Limit");
               show_bac_real();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 3 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 4)){
               show_context_tag("Deadband");
               show_bac_real();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 4 Expected!");
               };
            break;

         case 5: /* out of range */
            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 0)){
               len = show_context_tag("Time Delay");
               show_bac_unsigned(len);
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 0 Expected!");
               };
            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 1)){
               show_context_tag("Low Limit");
               show_bac_real();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 1 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 2)){
               show_context_tag("High Limit");
               show_bac_real();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 2 Expected!");
               };

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if((tagbuff & 0x08) && (tagval == 3)){
               show_context_tag("Deadband");
               show_bac_real();
               }
            else{
               pif_show_space();
               pif_show_ascii(0, "Error: Context Tag 3 Expected!");
               };
            break;
         }
      len = show_context_tag(BACnetEventParameter[tagval]); /* closing tag */
   }
   else {
      pif_show_space();
      bac_show_nbytes(1,"Error: Context Tag expected!");
   }
   exit:;
}


/**************************************************************************/
void  show_bac_event_transitions_ackd( void )
/**************************************************************************/
{

// show_application_tag(pif_get_byte(0)); - already called this!
   bac_show_byte("Number of unused bits","%u");
   sprintf(outstr,"%"FW"s = X'%%02X'","Event States Acknowledged");
   bac_show_flag(outstr,0xFF);
   pif_show_flagbit(0x80,"TO-OFFNORMAL transition acknowledged","TO-OFFNORMAL transition unacknowledged");
   pif_show_flagbit(0x40,"TO-FAULT transition acknowledged","TO-FAULT transition unacknowledged");
   pif_show_flagbit(0x20,"TO-NORMAL transition acknowledged","TO-NORMAL transition unacknowledged");
   /*
   pif_show_flagbit(0x10,"Unused bit","Unused bit");
   pif_show_flagbit(0x08,"Unused bit","Unused bit");
   pif_show_flagbit(0x04,"Unused bit","Unused bit");
   pif_show_flagbit(0x02,"Unused bit","Unused bit");
   pif_show_flagbit(0x01,"Unused bit","Unused bit");
   */
}

/**************************************************************************/
void show_bac_object_identifier( void )
/**************************************************************************/
{  /* Display and interpret a BACnet Object Identifier */
#if 0
   typedef union {
     struct {
       unsigned char lo;
       unsigned char hi;
       } byte;
       int word;
   } ureg;

   typedef union {
      struct {
         ureg lo_w;
         ureg hi_w;
      } word;
      unsigned long lword;
   } l_ureg;

   unsigned char obj_id[4];
   ureg obj_type;
   l_ureg instance;

   obj_id[0] = pif_get_byte(0);
   obj_id[1] = pif_get_byte(1);
   obj_id[2] = pif_get_byte(2);
   obj_id[3] = pif_get_byte(3);

   obj_type.byte.hi = obj_id[0]>>6;
   obj_type.byte.lo = obj_id[1]>>6 | obj_id[0]<<2;

   instance.word.hi_w.byte.hi = 0;
   instance.word.hi_w.byte.lo = obj_id[1] & 0x3f;
   instance.word.lo_w.byte.hi = obj_id[2];
   instance.word.lo_w.byte.lo = obj_id[3];
   
   bac_show_nbytes(4,"BACnet Object Identifier");

   if(obj_type.word > 63){ /* proprietary object type */
      sprintf(get_int_line(pi_data_current, pif_offset, 0),
         "               Proprietary Object Type");
      sprintf(get_int_line(pi_data_current, pif_offset, 0),
         "               Object Type = %u",obj_type.word);
      sprintf(get_int_line(pi_data_current, pif_offset, 0),
         "               Instance Number = %lu",instance.lword);
      }
   else{ /* standard object type */
      sprintf(get_int_line(pi_data_current, pif_offset, 0),
         "               Standard Object Type");
      sprintf(get_int_line(pi_data_current, pif_offset, 0),
         "               Object Type = %s",BACnetObjectType[obj_type.word]);
      sprintf(get_int_line(pi_data_current, pif_offset, 0),
         "               Instance Number = %lu",instance.lword);
      };
#else
	long	obj_id;
	int		obj_type;
	long	obj_instance;
	
	for (int i = 0; i < 4; i++)
		obj_id = (obj_id << 8) | (unsigned char)pif_get_byte( i );
	
	obj_type = (obj_id >> 22) & 0x000003FF;
	obj_instance = (obj_id & 0x003FFFFF);
	
	bac_show_nbytes( 4, "BACnet Object Identifier" );
	
	if(obj_type > 63) { /* proprietary object type */
		sprintf(get_int_line(pi_data_current, pif_offset, 0),
			"               Proprietary Object Type");
		sprintf(get_int_line(pi_data_current, pif_offset, 0),
			"               Object Type = %u", obj_type );
		sprintf(get_int_line(pi_data_current, pif_offset, 0),
			"               Instance Number = %lu", obj_instance );
	} else { /* standard object type */
		sprintf(get_int_line(pi_data_current, pif_offset, 0),
			"               Standard Object Type");
		sprintf(get_int_line(pi_data_current, pif_offset, 0),
			"               Object Type = %s",BACnetObjectType[obj_type]);
		sprintf(get_int_line(pi_data_current, pif_offset, 0),
			"               Instance Number = %lu", obj_instance );
	}
#endif

}

/**************************************************************************/
void show_bac_obj_prop_ref( void )
/**************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   tagbuff = pif_get_byte(0);
   while ((tagbuff & 0x0f) != 0x0f) {  /* closing PD tag not yet found */
      tagval = (tagbuff&0xf0)>>4;
      if (tagbuff & 0x08) {  /* context tag */
        if(tagval > 2) {
          pif_show_space();
         bac_show_nbytes(1,"Error: Invalid Context Tag (should be <= 2)!");
          goto exit;
          }
        len = show_context_tag(BACnetObjectPropertyReference[tagval]);
        switch (tagval) {
          case 0:  show_bac_object_identifier();
                   break;
          case 1:  show_bac_property_identifier(len);
                   break;
          case 2:  show_bac_unsigned(len);  /* array index */
                   break;
          }
        }
      else {  /* application tag */
        pif_show_space();
        bac_show_nbytes(1,"Error: Context Tag expected!");
        }
      tagbuff = pif_get_byte(0);
      }  /* end of while loop */
   exit:;
}

/**************************************************************************/
void show_bac_obj_prop_value( void )
/**************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   tagbuff = pif_get_byte(0);
   while ((tagbuff & 0x0f) != 0x0f) {  /* closing PD tag not yet found */
      tagval = (tagbuff&0xf0)>>4;
      if (tagbuff & 0x08) {  /* context tag */
        if(tagval > 4) {
          pif_show_space();
         bac_show_nbytes(1,"Error: Invalid Context Tag (should be <= 4)!");
          goto exit;
          }
        len = show_context_tag(BACnetObjectPropertyValue[tagval]);
        switch (tagval) {
          case 0:  show_bac_object_identifier();
                   break;
          case 1:  show_bac_property_identifier(len);
                   break;
          case 2:  show_bac_unsigned(len);  /* array index */
                   break;
          case 3:  show_application_data(pif_get_byte(0));  /* value */
                   tagbuff = pif_get_byte(0);  /* show closing PD tag */
                   tagval = (tagbuff&0xF0)>>4;
                   show_context_tag(BACnetObjectPropertyValue[tagval]);
                   break;
          case 4:  show_bac_unsigned(len);  /* priority */
          }
        }
      else {  /* application tag */
        pif_show_space();
        bac_show_nbytes(1,"Error: Context Tag expected!");
        }
      tagbuff = pif_get_byte(0);
      }  /* end of while loop */
   exit:;
}

/**************************************************************************/
void show_bac_object_type( void )
/**************************************************************************/
{
   unsigned char x;
   x = pif_get_byte(0);
   bac_show_byte(BACnetObjectType[x],"%u");
}

/**************************************************************************/
void show_bac_property_identifier( unsigned int len )
/**************************************************************************/
{
   unsigned int x;
   switch (len) {
      case 1:  x = (unsigned int)pif_get_byte(0);
          bac_show_byte(BACnetPropertyIdentifier[x],"%u");
          break;
      case 2:  x = pif_get_word_hl(0);
          if(x>max_property_id)
        bac_show_word_hl("Unknown Property Identifier","%u");
          else
        bac_show_word_hl(BACnetPropertyIdentifier[x],"%u");
          break;
      default: bac_show_nbytes(len,"Error: Invalid Property Identifier!");
   }
}

/**************************************************************************/
void show_bac_property_ref( void )
/**************************************************************************/

   /* This function displays only one parameter per call.  The calling
      routine must do the looping.  */
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if (tagbuff & 0x08) {  /* context tag */
      len = show_context_tag(BACnetPropertyReference[tagval]);
      switch (tagval) {
         case 0: show_bac_property_identifier(len);
                 break;
         case 1: show_bac_unsigned(len);
                 break;
         default: pif_show_space();
            bac_show_nbytes(1,"Error: Invalid Context Tag (should be <= 1)!");
            goto exit;
         }
      }
   else { /* application tag */
      pif_show_space();
      bac_show_nbytes(1,"Error: Context Tag expected!");
      }
   exit:;
}

/**************************************************************************/
void show_bac_property_states( void )
/**************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len, val;

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if (tagbuff & 0x08) { /* context tag */
     if (tagval > 10) {
       pif_show_space();
       bac_show_nbytes(1,"Error: Invalid Choice (should be 0-10)!");
       goto exit;
       }
     len = show_context_tag(BACnetPropertyStates[tagval]);
     if(tagval>0) {
       if(len>1)
         val = pif_get_word_hl(0);
       else
         val = pif_get_byte(0);
       }
     switch (tagval) {
       case 0: /* BOOLEAN case */
               if(val == 0)
                  bac_show_nbytes(len,"FALSE");
               else
                  bac_show_nbytes(len, "TRUE");
         show_application_data(pif_get_byte(0));
               break;
       case 1: bac_show_nbytes(len,BACnetBinaryPV[val]);
               break;
       case 2: bac_show_nbytes(len,BACnetEventType[val]);;
               break;
       case 3: bac_show_nbytes(len,BACnetPolarity[val]);
               break;
       case 4: bac_show_nbytes(len,BACnetProgramRequest[val]);
               break;
       case 5: bac_show_nbytes(len,BACnetProgramState[val]);
               break;
       case 6: bac_show_nbytes(len,BACnetProgramError[val]);
               break;
       case 7: bac_show_nbytes(len,BACnetReliability[val]);
               break;
       case 8: bac_show_nbytes(len,BACnetEventState[val]);
               break;
       case 9: bac_show_nbytes(len,BACnetDeviceStatus[val]);
               break;
       case 10: bac_show_nbytes(len,BACnetEngineeringUnits[val]);
       }  /* end switch */
     }  /* end if context tag */
   else {  /* application tag */
      pif_show_space();
      bac_show_nbytes(1,"Error: Context Tag expected!");
      }
   exit:;
}

/**************************************************************************/
void show_bac_property_value( void )
/**************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   tagbuff = pif_get_byte(0);
   while ((tagbuff & 0x0f) != 0x0f) {  /* closing PD tag not yet found */
      tagval = (tagbuff&0xf0)>>4;
      if (tagbuff & 0x08) {  /* context tag */
        if(tagval > 3) {
          pif_show_space();
          bac_show_nbytes(1,"Error: Invalid Context Tag (should be <= 3)!");
          goto exit;
          }
        len = show_context_tag(BACnetPropertyValue[tagval]);
        switch (tagval) {
          case 0:  show_bac_property_identifier(len);
                   break;
          case 1:  show_bac_unsigned(len);  /* array index */
                   break;
          case 2:  show_application_data(pif_get_byte(0));  /* value */
                   tagbuff = pif_get_byte(0);  /* show closing PD tag */
                   tagval = (tagbuff&0xF0)>>4;
      /* we may need a show_bac_ANY here
         show_bac_ANY(obj_type,prop_id,prop_idx);
      */
                   show_context_tag(BACnetPropertyValue[tagval]);  /* closing tag */
                   break;
          case 3:  show_bac_unsigned(len);  /* priority */
          }
        }
      else {  /* application tag */
        pif_show_space();
        bac_show_nbytes(1,"Error: Context Tag expected!");
        }
      tagbuff = pif_get_byte(0);
      }  /* end of while loop */
   exit:;
}

/**************************************************************************/
void show_bac_read_access_result( void )
/**************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;
   int obj_type, prop_idx, prop_id;

   prop_idx = -1;    /* initialize to indicate no array index present */

   while (pif_offset < pif_end_offset) {
      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if ((tagbuff & 0x08) && (tagval == 0)){
         show_context_tag("Object Identifier");
         obj_type = bac_extract_obj_type();
         show_bac_object_identifier();
         }
      else{
         pif_show_space();
         bac_show_nbytes(1, "Error: Context Tag 0 expected!");
         }

      tagbuff = pif_get_byte(0);
      tagval = (tagbuff&0xF0)>>4;
      if ((tagbuff & 0x08) && (tagval == 1)){
         show_context_tag("List of Results");     /* opening tag */

         while((pif_offset<(pif_end_offset-1))&&((pif_get_byte(0)&0x0F)!=0x0F)){
            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if ((tagbuff & 0x08) && (tagval == 2)){
               len = show_context_tag("Property Identifier");
               if (len == 1)
                  prop_id = (int)pif_get_byte(0);
               else
                  prop_id = (int)pif_get_word_hl(0);
               show_bac_property_identifier(len);
               }
            else {
               pif_show_space();
               bac_show_nbytes(1, "Error: Context Tag 2 exptected!");
               }   

            tagbuff = pif_get_byte(0);
            tagval = (tagbuff&0xF0)>>4;
            if ((tagbuff & 0x08) && (tagval == 3)){
               len = show_context_tag("Property Array Index");
               prop_idx = pif_get_byte(0);
               show_bac_unsigned(len);

               tagbuff = pif_get_byte(0);
               tagval = (tagbuff&0xF0)>>4;
               }

            switch(tagval){
               case 4 : show_context_tag("Property Value");
                        show_bac_ANY(obj_type, prop_id, prop_idx);
                        show_context_tag("Property Value");
                        break;
               case 5 : show_context_tag("Property Access Error");  /* opening tag */
                        show_error_codes();
                        show_context_tag("Property Access Error");  /* closing tag */
                        break;
               default: pif_show_space();
                        bac_show_nbytes(1, "Error: Context Tag 4 or 5 expected!");
                        break;
               } /* switch */
            } /* while */

         show_context_tag("List of Results");     /* closing tag */
         }
      else{
         pif_show_space();
         bac_show_nbytes(1,"Error: Context Tag 1 expected!");
         }
      } /* while */
}

/**************************************************************************/
void show_bac_read_access_spec( void )
/**************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if (tagbuff & 0x08) {  /* context tag */
     if(tagval == 0){
       show_context_tag("Object Identifier");
       show_bac_object_identifier();
       }
     else{
       pif_show_space();
       bac_show_nbytes(1,"Error: Context Tag 0 expected!");
       }
     tagbuff = pif_get_byte(0);
     show_context_tag("List of Property References");  /* opening tag */
     while(tagbuff != 0x1f){
       show_bac_property_ref();
       tagbuff = pif_get_byte(0);
     
      } 
     show_context_tag("List of Property References");  /* closing tag */
     }
   else {
     pif_show_space();
     bac_show_nbytes(1,"Error: Context Tag expected!");
     }
}

/**************************************************************************/
void  show_bac_recipient( void )
/**************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if (tagbuff & 0x08) {  /* context tag */
     switch (tagval) {
        case 0: show_context_tag("Device");
           show_bac_object_identifier();
           break;
        case 1:  show_context_tag("Address");  /* opening Tag */
           show_bac_address();
           show_context_tag("Address");  /* closing Tag */
           break;
        default: pif_show_space();
           pif_show_ascii(0, "Error: Context Tag 0 Expected!");
        }

     }
   else{  /* application tag */
     pif_show_space();
     pif_show_ascii(0, "Error: Context Tag Expected!");
     pif_offset = pif_end_offset;
     }  
}

/**************************************************************************/
void  show_bac_recipient_process( void )
/**************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if (tagval == 0){
      show_context_tag("Recipient");  /* opening tag */
      show_bac_recipient();
      show_context_tag("Recipient");  /* closing tag */
      }
   else {
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag 0 Expected!");
        pif_offset = pif_end_offset;
        }

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if (tagval == 1){
      pif_show_space();
      len = show_context_tag("Process Identifier");
      show_bac_unsigned(len);
      }
   else {
        pif_show_space();
        pif_show_ascii(0, "Error: Context Tag 1 Expected!");
        pif_offset = pif_end_offset;
        }
}

/**************************************************************************/
void show_bac_special_event( void )
/**************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;
   
   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if (tagbuff & 0x08) {  /* context tag */
     if(tagval > 3) {
       pif_show_space();
       bac_show_nbytes(1,"Error: Invalid Context Tag (should be <= 3)!");
       goto exit;
       };
     if((tagval != 0) && (tagval != 1)){
       pif_show_space();
       bac_show_nbytes(1,"Error: Context Tag 0 or 1 Expected");
       goto exit;
       };

     if(tagval == 0){
       show_context_tag(BACnetSpecialEvent[tagval]);
       show_bac_calendar_entry();
       show_context_tag(BACnetSpecialEvent[tagval]);  /* closing tag */
       tagbuff = pif_get_byte(0);
       tagval = (tagbuff&0xF0)>>4;
       };

     if(tagval == 1){
       show_context_tag(BACnetSpecialEvent[tagval]);
       show_bac_object_identifier();
       tagbuff = pif_get_byte(0);
       tagval = (tagbuff&0xF0)>>4;
       };

     if(tagval == 2){ /* list of time values */
       show_context_tag(BACnetSpecialEvent[tagval]);  /* opening tag */
       while ((pif_get_byte(0) & 0x0f) != 0x0f) show_bac_time_value();
       show_context_tag(BACnetSpecialEvent[2]);  /* closing tag */
       tagbuff = pif_get_byte(0);
       tagval = (tagbuff&0xF0)>>4;
       }
     else{
       pif_show_space();
       bac_show_nbytes(1,"Error: Context Tag 2 Expected");
       goto exit;
       };
       
     if(tagval == 3){ /* Event Priority */
       len = show_context_tag(BACnetSpecialEvent[tagval]);
       show_bac_unsigned(len);
       }
     else{
       pif_show_space();
       bac_show_nbytes(1,"Error: Context Tag 3 Expected");
       goto exit;
       };
     }
   else {  /* application tag */
     pif_show_space();
     bac_show_nbytes(1,"Error: Context Tag expected!");
     }
   exit:;
}

/**************************************************************************/
void show_bac_status_flags( unsigned int len )
/**************************************************************************/
{
   unsigned char x;
   unsigned int i,j, unused;

           /* len = show_application_tag(x); */
           unused = pif_get_byte(0);
           bac_show_byte("Unused Bits in Last Octet","%u");
           j=0;
           for(i=0;i<((len-1)*8-unused);i++) {
             if(!(i%8)) {
               x = pif_get_byte(0);
               sprintf(outstr,"Bit String Octet [%u]",j++);
               bac_show_byte(outstr,"X'%02X'");
               }
             sprintf(outstr,"   %s",BACnetStatusFlags[i]);
             pif_offset--;
             if(x&0x80)
               show_str_eq_str(outstr,"TRUE",1);
             else
               show_str_eq_str(outstr,"FALSE",1);
             pif_offset++;
             x <<= 1;
             }
}

/**************************************************************************/
void show_bac_timestamp( void )
/**************************************************************************/
{
   unsigned char tagbuff, tagval; /* buffers for tags and tag values */
   unsigned int len;

   tagbuff = pif_get_byte(0);
   tagval = (tagbuff&0xF0)>>4;
   if (tagbuff & 0x08) {  /* context tag */
      if (tagval > 2) {
        pif_show_space();
        bac_show_nbytes(1,"Error: Invalid Context Tag (should be 0,1 or 2)!");
        goto exit;
        }
      len = show_context_tag(BACnetTimeStamp[tagval]);
      switch (tagval) {
        case 0: show_bac_time();
                break;
        case 1: show_bac_unsigned(len);
                break;
        case 2:  /* Primitive date and time follow */
                show_application_data(pif_get_byte(0)); /*Date*/
                show_application_data(pif_get_byte(0)); /*Time*/
                show_context_tag(BACnetTimeStamp[tagval]); /* closing tag */
        }  /* end of switch */
     }
   else {  /* application tag */
      pif_show_space();
      bac_show_nbytes(1,"Error: Context Tag expected!");
   }
exit:;
}

/**************************************************************************/
void show_bac_time_value( void )
/**************************************************************************/
{
   pif_show_space();
   show_application_tag(pif_get_byte(0));
   show_bac_time();

   pif_show_space();
   pif_show_ascii(0,BACnetTimeValue[1]);
   show_application_data(pif_get_byte(0));
}

/**************************************************************************/
void show_bac_VT_session( void )
/**************************************************************************/
{
   pif_show_ascii(0,BACnetVTSession[0]);
   show_application_data(pif_get_byte(0));
   pif_show_ascii(0,BACnetVTSession[1]);
   show_application_data(pif_get_byte(0));
}

/**************************************************************************/
void show_bac_weeknday( void )
/**************************************************************************/
{
   if (pif_get_byte(0) == 255)
      sprintf(outstr,"Unspecified");
   else
      sprintf(outstr,month[pif_get_byte(0)-1]);
   show_str_eq_str("Month",outstr,1);
   pif_offset++;

   if (pif_get_byte(0) == 255)
      sprintf(outstr,"Unspecified");
   else
      sprintf(outstr,"%u",pif_get_byte(0));
   show_str_eq_str("Week of Month",outstr,1);
   pif_offset++;

   if (pif_get_byte(0) == 255)
      sprintf(outstr,"Unspecified");
   else
      sprintf(outstr,day_of_week[pif_get_byte(0)-1]);
   show_str_eq_str("Day of Week",outstr,1);
   pif_offset++;
}

}
