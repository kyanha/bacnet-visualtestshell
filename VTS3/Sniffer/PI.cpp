
#include "stdafx.h"

#include "pi.h"

//-----

BACnetPIInfoPtr		gCurrentInfo;

namespace NetworkSniffer {
	int interp_bacnet_IP( char *, int );
	int interp_bacnet_ETHERNET( char *, int );
	int interp_bacnet_ARCNET( char *, int );
	int interp_bacnet_MSTP( char *, int );
	int interp_bacnet_PTP( char *, int );
	int interp_bacnet_BVLL( char *, int );
	int interp_bacnet_NL( char *, int );
	int interp_bacnet_AL( char *, int );
	int interp_Message( char *, int );
	int interp_BakRestoreMessage( char *, int );	// added by Jingbo Gao, Sep 20 2004

	struct pi_data pi_data_bacnet_IPRec, *pi_data_bacnet_IP = &pi_data_bacnet_IPRec;
	struct pi_data pi_data_bacnet_ETHERNETRec, *pi_data_bacnet_ETHERNET = &pi_data_bacnet_ETHERNETRec;
	struct pi_data pi_data_bacnet_ARCNETRec, *pi_data_bacnet_ARCNET = &pi_data_bacnet_ARCNETRec;
	struct pi_data pi_data_bacnet_MSTPRec, *pi_data_bacnet_MSTP = &pi_data_bacnet_MSTPRec;
	struct pi_data pi_data_bacnet_PTPRec, *pi_data_bacnet_PTP = &pi_data_bacnet_PTPRec;
	struct pi_data pi_data_bacnet_BVLLRec, *pi_data_bacnet_BVLL = &pi_data_bacnet_BVLLRec;
	struct pi_data pi_data_bacnet_NLRec, *pi_data_bacnet_NL = &pi_data_bacnet_NLRec;
	struct pi_data pi_data_bacnet_ALRec, *pi_data_bacnet_AL = &pi_data_bacnet_ALRec;
	struct pi_data pi_data_MessageRec, *pi_data_Message = &pi_data_MessageRec;
	}

//
//	BACnetPIInfo::BACnetPIInfo
//

BACnetPIInfo::BACnetPIInfo( bool summary, bool detail )
{
	summaryLine[0] = 0;
	detailCount = 0;

	SetPIMode( summary, detail );
}

//
//	BACnetPIInfo::~BACnetPIInfo
//

BACnetPIInfo::~BACnetPIInfo( void )
{
	// toss any detail lines allocated
	for (int i = 0; i < detailCount; i++ )
		if (detailLine[i])
			delete detailLine[i];
}

//
//	BACnetPIInfo::SetPIMode
//

void BACnetPIInfo::SetPIMode( bool summary, bool detail )
{
	doSummary = summary;
	doDetail = detail;

	NetworkSniffer::pi_data_bacnet_IP->do_sum = summary;
	NetworkSniffer::pi_data_bacnet_IP->do_int = detail;
	NetworkSniffer::pi_data_bacnet_ETHERNET->do_sum = summary;
	NetworkSniffer::pi_data_bacnet_ETHERNET->do_int = detail;
	NetworkSniffer::pi_data_bacnet_ARCNET->do_sum = summary;
	NetworkSniffer::pi_data_bacnet_ARCNET->do_int = detail;
	NetworkSniffer::pi_data_bacnet_MSTP->do_sum = summary;
	NetworkSniffer::pi_data_bacnet_MSTP->do_int = detail;
	NetworkSniffer::pi_data_bacnet_PTP->do_sum = summary;
	NetworkSniffer::pi_data_bacnet_PTP->do_int = detail;
	NetworkSniffer::pi_data_bacnet_BVLL->do_sum = summary;
	NetworkSniffer::pi_data_bacnet_BVLL->do_int = detail;
	NetworkSniffer::pi_data_bacnet_NL->do_sum = summary;
	NetworkSniffer::pi_data_bacnet_NL->do_int = detail;
	NetworkSniffer::pi_data_bacnet_AL->do_sum = summary;
	NetworkSniffer::pi_data_bacnet_AL->do_int = detail;
	NetworkSniffer::pi_data_Message->do_sum = summary;
	NetworkSniffer::pi_data_Message->do_int = detail;
}

//
//	BACnetPIInfo::Interpret
//

void BACnetPIInfo::Interpret( ProtocolType proto, char *header, int length )
{
	// flush the current contents (if any)
	for (int i = 0; i < detailCount; i++ )
		if (detailLine[i])
			delete detailLine[i];

	// reset the output
	summaryLine[0] = 0;
	detailCount = 0;

	// set the context
	gCurrentInfo = this;

	// set the beginning of the buffers
	piBuffer = header;
	piLen = length;

	// set the PI mode to match the settings for this
	SetPIMode( doSummary, doDetail );

	if(doDetail)//only create space for timestamp, do not print any information here
	{
		gCurrentInfo->detailCount=1;
		gCurrentInfo->detailLine[0]=0;
		NetworkSniffer::pif_show_space();
	}
	
	try {
		// call one of the known interpreters
		switch (proto) {
			case ipProtocol:
				NetworkSniffer::interp_bacnet_IP( header, length );
				break;
			case ethernetProtocol:
				NetworkSniffer::interp_bacnet_ETHERNET( header, length );
				break;
			case arcnetProtocol:
				NetworkSniffer::interp_bacnet_ARCNET( header, length );
				break;
			case mstpProtocol:
				NetworkSniffer::interp_bacnet_MSTP( header, length );
				break;
			case ptpProtocol:
				NetworkSniffer::interp_bacnet_PTP( header, length );
				break;
			case msgProtocol:
				NetworkSniffer::interp_Message( header, length );
				break;
			case bakRestoreMsgProtocol:			// added by Jingbo Gao, Sep 20 2004
				NetworkSniffer::interp_BakRestoreMessage( header, length );
		}
	}
	catch (...) {
		// if we run out of detail lines, get_int_line throws -1
	}
}

//-----

//
//	Everything else, all of the other global variables and functions, belong to the 
//	NetworkSniffer namespace.
//

namespace NetworkSniffer {

//-----

struct  pi_data		gPIData;	/* global pi_data */

//-----

struct  pi_data * near pif_pi;  /* Ptr to current pi */
int     near pif_offset;        /* Current offset */
int     near pif_end_offset;    /* Offset for last+1 byte */
int     near pif_flagbit_indent;/* Number of blanks for flagbit */
char    pif_header_msg[];       /* Saved header message */

// char *  near dlc_header;        /* ptr to start of frame */
char *  near msg_origin;        /* ptr to start of frame or message */
// int     near offset_src_addr;   /* offset to frame source address */
// int     near offset_dst_addr;   /* offset to frame destination address */
// int     near size_dlc_addr;     /* size in bytes of DLC addresses */
// int     near bytes_not_present; /* Bytes not in frame due to Truncation */
// int     near true_size;         /* True original size of the frame */
// FRNUM   near pi_frame;          /* the current frame number */
// char *  near pi_frame_data;     /* ptr to data space in the frame header */
// int     near disp_base;         /* the number display base, when variable */
// int     near data_version;      /* sequence number for capture data */
// int          names_version;     /* sequence number for names table */
// boolean near do_prescan;        /* are we doing a prescan of frames? */
// int     near n_summary_lines;   /* no of summary lines currently used */
// int		near embedded_addrtype; /* if sync, type of embedded DLC addr */
// boolean near dlc_error;			/* CRC or other data link error */

//-----

char *get_sum_line( struct pi_data * )
{
	// no more than one at a time
	if (gCurrentInfo->summaryLine[0])
		throw (-1);
	
	return gCurrentInfo->summaryLine;
}

char *get_int_line( struct pi_data *, int offset, int length, int nodeType )
{
	BACnetPIDetailPtr	dp
	;
	
	// make sure we don't try and grab too many
	if (gCurrentInfo->detailCount >= MAX_INT_LINES)
		throw (-1);
	
	// allocate a new detail line buffer
	gCurrentInfo->detailLine[gCurrentInfo->detailCount++] = dp = new BACnetPIDetail;
	dp->piOffset = gCurrentInfo->piOffset + offset;
	dp->piLen = length;
	dp->piLine[0] = 0;

	dp->piNodeType = nodeType;		// added by Lei Chengxin 2003-7-22
	
	// return a pointer to the character buffer
	return dp->piLine;
}

char *get_cur_int_line( void )
{
	return gCurrentInfo->detailLine[gCurrentInfo->detailCount-1]->piLine;
}

//-----

//
//	rev_word
//
//	Returns the parameter value with the bytes reversed, used for translating from 
//	big endian to little endian and vice versa.
//

int rev_word( int arg )
{
	return	((arg & 0x00FF) << 8)
		|	((arg & 0xFF00) >> 8)
		;
}

//
//	rev_long
//
//	Returns the parameter value with the bytes reversed, used for translating from 
//	big endian to little endian and vice versa.
//

long rev_long( long arg )
{
	return	((arg & 0xFF000000) >> 24)
		|	((arg & 0x00FF0000) >> 8)
		|	((arg & 0x0000FF00) << 8)
		|	((arg & 0x000000FF) << 24)
		;
}

//-----

//
//	pif_init
//
//	Initialize a protocol interpreter control block.  The real sniffer has an 
//	elaborate registration mechanism for making sure the the interpreter gets 
//	called in the correct "mode" and there are a bunch of fields that are 
//	not implemented.
//

void pif_init( struct pi_data *pi_data_your_protocol, void *p, int len )
{
	/* set up the current pi_data */
	pif_pi = pi_data_your_protocol;
	
	/* set up the globals */
	msg_origin = (char *)p;
	pif_offset = 0;
	pif_end_offset = len;
	pif_flagbit_indent = 0;
	
	/* stash the offset from the true beginning */
	gCurrentInfo->piOffset = ((char *)p - gCurrentInfo->piBuffer);
	
	pif_pi->do_sum = gCurrentInfo->doSummary;	/* generate summary lines? */
	pif_pi->do_int = gCurrentInfo->doDetail;	/* generate interpretation (detail) lines? */
	pif_pi->do_count = false;	/* only count summary lines? */
	pif_pi->do_names = true;	/* add symbolic station names? */
	pif_pi->recursive = false;	/* is this a recursive call? */
	pif_pi->selected = true;	/* is this PI selected in the menu? */
	pif_pi->forcepi_rules = 0;	/* linked list of "Force PI" rules */
}

//
//	pif_header
//
//	Generate a header line for a block of octets that are about to be interpreted 
//	in detail.
//

void pif_header( int len, char *header_string )
{
	char	*s
	;
	
	// generate the header line in the detail
	s = get_int_line( pif_pi, pif_offset, len );
	sprintf( s, "----- %s -----", header_string );
	
	// output a blank line
	pif_show_space();
}

//
//	pif_line
//

char *pif_line( int len )
{
	char	*s
	;
	
	s = get_int_line( pif_pi, pif_offset, len );
	
	pif_offset += len;
	return s;
}

//
//	pif_get_ascii
//

char *pif_get_ascii( int offset, int len, char result_str[] )
{
	char	c
	,		*src = pif_get_addr() + offset
	,		*dst = result_str
	;
	
	// transfer ASCII printable chars
	while (len-- && ((c = *src++) != 0))
		*dst++ = ((c >= ' ') && (c <= '~')) ? c : '.';
	
	// null terminate the end and return a pointer to it
	*dst = 0;
	return dst;
}

//
//	pif_show_ascii
//

void pif_show_ascii( int len, char *prstr )
{
	char	*s
	,		buff[ MAX_INT_LINE ]
	;
	
	// get a detail line
	s = get_int_line( pif_pi, pif_offset, len );
	
	// get the ASCII data and format it in the buffer
	pif_get_ascii( 0, len, buff );
	sprintf( s, prstr, buff );
	
	// update the offset
	pif_offset += len;
}

//
//	pif_append_ascii
//

void pif_append_ascii( const char *fmt, const char *data )
{
	char	*s = get_cur_int_line()
	;

	sprintf( s + strlen(s), fmt, data );
}

//
//	pif_show_nbytes_hex
//
/* replaced with method below in order to correct buffer overruns
void pif_show_nbytes_hex( char *prstr, int byte_count )
{
	static char	hex[] = "0123456789ABCDEF"
	;
	char		*s, *dst
	,			buff[ MAX_INT_LINE ]
	;
	
	// get a detail line
	s = get_int_line( pif_pi, pif_offset, byte_count );
	
	// build a hex string
	dst = buff;
	while (byte_count--) {
		int x = pif_get_byte(0);
		*dst++ = hex[ (x >> 4) & 0x0F ];
		*dst++ = hex[ x & 0x0F ];
		pif_offset++;
	}
	*dst = 0;
	
	// format the data into the detail line
	sprintf( s, prstr, buff );
}
*/
// new method provided by Buddy Lott for bug #1606849
void pif_show_nbytes_hex( char *prstr, int byte_count )
{
	static char hex[] = "0123456789ABCDEF";
	char *s, *dst
	,	buff[ MAX_INT_LINE + 4]; /* use the extra 3 bytes for a '...' when data is dropped*/
	
	int spaceleft = MAX_INT_LINE - strlen(prstr); // keeps track of how much of the buffer should be used.

	// since each byte takes at least 2 characters, lets make sure that the space left is even
	spaceleft = (spaceleft%2)?spaceleft-1:spaceleft;

	// fill in the buffer '.' mark some dropped data!
	memset(buff,'.',MAX_INT_LINE + 3);

	// get a detail line
	s = get_int_line( pif_pi, pif_offset, byte_count );

	// build a hex string
	dst = buff;
	while (spaceleft > 0 && byte_count--) {
		int x = pif_get_byte(0);
		*dst++ = hex[ (x >> 4) & 0x0F ];
		*dst++ = hex[ x & 0x0F ];
		spaceleft -=2;
		pif_offset++;
	}

	if ( byte_count > 0 )
	{
		*(dst+3) = 0;
		pif_offset += byte_count;
	}
	else
	{
	*dst = 0;
	}

	// format the data into the detail line
	sprintf( s, prstr, buff );
}


//
//	pif_show_long_hl
//

long pif_show_long_hl( char *prstr )
{
	//Modifyed by Zhu Zhenhua 2003-7-22, the code to add "-" before the number if it is signed and negative
	//else do as before	
	CString str = prstr;
	CString str1 = "Value (4-octet signed)";
	unsigned char necValue = (0x80 & (unsigned char)pif_get_byte(0));
	char			*s
	;
	unsigned long	arg
	;
	
	// get a detail line
	s = get_int_line( pif_pi, pif_offset, 4 );
	
	// get the long data and format it in the buffer
	arg = pif_get_long_hl(0);
	if(str.Find(str1)!= -1)
		if(necValue)
		{
			int nIndex = str.Find("=")+2;
			str.Insert(nIndex,'-');
			unsigned long nValue = (unsigned short)(0x100000000 - arg);
			strcpy(prstr,str);
			sprintf(s,prstr,nValue);
			pif_offset += 4;
			return arg;
		}
	sprintf( s, prstr, arg );	

	// update the offset
	pif_offset += 4;	
	return arg;
}

//
//	pif_show_slong_hl
//      LJT 10/11/2007 - added to fix handling of signed integers which encode to 3 bytes

long pif_show_slong_hl( char *prstr )
{
	//Modifyed by Zhu Zhenhua 2003-7-22, the code to add "-" before the number if it is signed and negative
	//else do as before	
	CString str = prstr;
	CString str1 = "Value (3-octet signed)";
	unsigned char necValue = (0x80 & (unsigned char)pif_get_byte(0));
	char			*s
	;
	unsigned long	arg
	;
	
	// get a detail line
	s = get_int_line( pif_pi, pif_offset, 3 );
	
	// get the long data and format it in the buffer
	arg = pif_get_long_hl(0);
	arg = arg >>8;   // Note: the pif_get_long_hl read 4 bytes which is 1 too many so here we strip it back off.  LJT
	if(str.Find(str1)!= -1)
		if(necValue)
		{
			int nIndex = str.Find("=")+2;
			str.Insert(nIndex,'-');
			unsigned long nValue = (unsigned short)(0x100000000 - arg);
			strcpy(prstr,str);
			sprintf(s,prstr,nValue);
			pif_offset += 3;
			return arg;
		}
	sprintf( s, prstr, arg );	

	// update the offset
	pif_offset += 3;	
	return arg;
}

//
//	pif_show_word_hl
//

int pif_show_word_hl( char *prstr )
{
	//Modifyed by Zhu Zhenhua 2003-7-22, the code to add "-" before the number if it is signed and negative
	//else do as before	
	CString str = prstr;
	CString str1 = "Value (2-octet signed)";
	unsigned char necValue = (0x80 & (unsigned char)pif_get_byte(0));
	char			*s
		;
	unsigned short	arg
		;
	
	// get a detail line
	s = get_int_line( pif_pi, pif_offset, 2 );
	
	// get the short data and format it in the buffer
	arg = pif_get_word_hl(0);
	if(str.Find(str1)!= -1)
		if(necValue)
		{	
			int nIndex = str.Find("=")+2;
			str.Insert(nIndex,'-');
			unsigned short nValue = (unsigned short)(0x10000 - arg);
			strcpy(prstr,str);
			sprintf(s,prstr,nValue);
			pif_offset += 2;
			return arg;
		}
			
		sprintf( s, prstr, arg );		
		// update the offset
		pif_offset += 2;	
		return arg;

}

//
//	pif_show_byte
//

int pif_show_byte( char *prstr )
{
	char			*s
	;
	unsigned char	arg
	;
	
	// get a detail line
	s = get_int_line( pif_pi, pif_offset, 1 );
	
	// get the long data and format it in the buffer
	arg = pif_get_byte(0);
	sprintf( s, prstr, arg );
	
	// update the offset
	pif_offset += 1;
	
	return arg;
}

//
//	pif_show_flag
//

void pif_show_flag( char *prstr, int mask )
{
	char	*s
	,		buff[ MAX_INT_LINE ]
	;
	int		flag
	;
	
	// get a detail line
	s = get_int_line( pif_pi, pif_offset, 1 );
	for (int i = 0; i < pif_flagbit_indent; i++)
		*s++ = ' ';
	
	// get the byte
	flag = pif_get_byte(0);
	
	// format the mask
	*s++ = (mask & 0x80) ? ((flag & 0x80) ? '1' : '0') : '.';
	*s++ = (mask & 0x40) ? ((flag & 0x40) ? '1' : '0') : '.';
	*s++ = (mask & 0x20) ? ((flag & 0x20) ? '1' : '0') : '.';
	*s++ = (mask & 0x10) ? ((flag & 0x10) ? '1' : '0') : '.';
	*s++ = ' ';
	*s++ = (mask & 0x08) ? ((flag & 0x08) ? '1' : '0') : '.';
	*s++ = (mask & 0x04) ? ((flag & 0x04) ? '1' : '0') : '.';
	*s++ = (mask & 0x02) ? ((flag & 0x02) ? '1' : '0') : '.';
	*s++ = (mask & 0x01) ? ((flag & 0x01) ? '1' : '0') : '.';
	
	strcpy( buff, prstr );
	strcat( buff, " = %d" );
	sprintf( s, buff, flag );
	
	// update the offset
	pif_offset += 1;
}

//
//	pif_show_flagbit
//

int pif_show_flagbit( int bit, char *truestr, char *falsestr )
{
	// get a detail line
	char *s = get_int_line( pif_pi, pif_offset-1, 1 )
	;
	
	// get the previous byte
	int		flag = pif_get_byte(-1)
	;
	
	// indent to line up '='
	for (int i = 0; i < pif_flagbit_indent; i++)
		*s++ = ' ';
	
	// format the mask
	*s++ = (bit & 0x80) ? ((flag & 0x80) ? '1' : '0') : '.';
	*s++ = (bit & 0x40) ? ((flag & 0x40) ? '1' : '0') : '.';
	*s++ = (bit & 0x20) ? ((flag & 0x20) ? '1' : '0') : '.';
	*s++ = (bit & 0x10) ? ((flag & 0x10) ? '1' : '0') : '.';
	*s++ = ' ';
	*s++ = (bit & 0x08) ? ((flag & 0x08) ? '1' : '0') : '.';
	*s++ = (bit & 0x04) ? ((flag & 0x04) ? '1' : '0') : '.';
	*s++ = (bit & 0x02) ? ((flag & 0x02) ? '1' : '0') : '.';
	*s++ = (bit & 0x01) ? ((flag & 0x01) ? '1' : '0') : '.';
	*s++ = ' ';
	*s++ = '=';
	*s++ = ' ';
	
	if (truestr && !falsestr)
		strcpy( s, truestr );
	else
	if ((bit & flag) != 0)
		strcpy( s, truestr );
	else
		strcpy( s, falsestr );
	
	return ((bit & flag) != 0);
}

//
//	pif_show_flagmask
//

int pif_show_flagmask( int maskbits, int value, char *prstr )
{
	// get the previous byte
	int		flag = pif_get_byte(-1)
	;
	
	// if the masked bits don't match the value, skip it
	if ((flag & maskbits) != value)
		return 0;
	
	// get a detail line
	char *s = get_int_line( pif_pi, pif_offset-1, 1 )
	;
	
	// indent to line up '='
	for (int i = 0; i < pif_flagbit_indent; i++)
		*s++ = ' ';
	
	// format the mask
	*s++ = (maskbits & 0x80) ? ((flag & 0x80) ? '1' : '0') : '.';
	*s++ = (maskbits & 0x40) ? ((flag & 0x40) ? '1' : '0') : '.';
	*s++ = (maskbits & 0x20) ? ((flag & 0x20) ? '1' : '0') : '.';
	*s++ = (maskbits & 0x10) ? ((flag & 0x10) ? '1' : '0') : '.';
	*s++ = ' ';
	*s++ = (maskbits & 0x08) ? ((flag & 0x08) ? '1' : '0') : '.';
	*s++ = (maskbits & 0x04) ? ((flag & 0x04) ? '1' : '0') : '.';
	*s++ = (maskbits & 0x02) ? ((flag & 0x02) ? '1' : '0') : '.';
	*s++ = (maskbits & 0x01) ? ((flag & 0x01) ? '1' : '0') : '.';
	*s++ = ' ';
	*s++ = '=';
	*s++ = ' ';
	
	strcpy( s, prstr );
	
	// success
	return 1;
}

//
//	pif_show_space
//
//	Simply outputs a blank detail line.
//

void pif_show_space( void )
{

/*	disabled for TreeList Detail View
	char	*s
	;
	
	// get a detail line, make sure it's empty
	s = get_int_line( pif_pi, pif_offset, 0 );
	*s = 0;
*/
}

//
//	xsprintf
//
//	The sprintf() function in the real sniffer has an extension to allow the 
//	pif_offset to be updated as a side effect.  It takes the type of object 
//	to know how far to update.  In the BACnet sniffer functions, only '%>ku'
//	is used (which uses one byte, yet another extension) so that's the only 
//	thing I check for.  Oh, and it's also used only once.
//

void xsprintf( char *dst, char *prstr, char *hdr, char *valu )
{
	char	buff[ MAX_INT_LINE ]
	;
	int		hackOffset
	;
	
	// look for the magic string
	for (hackOffset = 0; ; hackOffset++)
		if (strncmp(prstr+hackOffset,"%>ku",4) == 0)
			break;
	
	// replace the string with a regular %d
	strncpy( buff, prstr, hackOffset );
	strcpy( buff+hackOffset, "%d" );
	strcat( buff, prstr+hackOffset+4 );
	
	// now use the real sprintf
	sprintf( dst, buff, hdr, (unsigned char)pif_get_byte(0), valu );
	
	// update passed the byte
	pif_offset += 1;
}

// end of the namespace
}
