// $Header$
/* file: pi.h; Fformat: tabs 4 */
/*****************************************************************************


                        NETWORK SNIFFER

                              PI.H


This contains the symbols used by both extendible Protocol Interpreters
and other modules of the Sniffer.

Note to Protocol Interpreter implementors:  Do not change any of the
symbols defined here since they were also used to compile other parts
of the Sniffer.


             (C) Copyright 1986-1994 Network General Corporation
                         All rights reserved.


*****************************************************************************/

/*-----------------------------------------------------------------------------

Change log

08/13/86  L. Shustek    Written.
09/07/86  J. Nisbet     Add "PIX" information.
09/10/86  L. Shustek    Expand MAX_SUM_LINE from 50 to 100,  MAX_SUM_LINES from 8 to 10.
09/22/86  J. Nisbet     Change "PIX" to "PIF".  Add "do_name" in pi_data.
09/29/86  J. Nisbet     Add PI name field for detailed interpretation lines.
11/04/86  L. Shustek    Add compiler-type switch setting.
11/06/86  L. Shustek    Move pi_name to the private structure.
11/19/86  L. Shustek    Add Microsoft compiler type-checking.
12/15/86  L. Shustek    Expand summary line width from 100 to 120.
12/18/86  L. Shustek    Add pif_get_addr() from intarp.c.
 1/15/87  L. Shustek    Add TRING, ENET, and PITYPE_xxx symbols in support of 
                        Ethernet version.
 1/24/87  L. Shustek    Add recursive boolean to pi_data.
                        Increase number of detail lines from 150 to 200.
                        Increase number of summary lines from 10 to 12.
 2/25/87  L. Shustek    Add station name type, fmt_station flag, and
                        add_station flag symbols.
 3/07/87  LJS/greep     Add get_code_name stuff.
 3/22/87  greep         Move struct date from pifutil.c to here.
 5/05/87  L. Shustek    Add code_range_struct.  Remove network flags.
 5/08/87  L. Shustek    Add pif_eof() macro from greep's stuff.
 5/18/87  G. Nitzberg   Add struct pi_trap. Adde pif_off_end() if we have run
                        off the end of frame. Also added a set of pif MACROs
                        which check for End of Frame.
 5/25/87  L. Shustek    Add pi_frame_data.
 6/10/87  L. Shustek    Add Arcnet stuff.
 6/23/87  G. Nitzberg   Add "extern int offset_src_addr" and "extern int offset_dst_addr".
 6/24/87  L. Shustek    Add FMT_HEX.
 7/01/87  G. Nitzberg   Add "extern int bytes_not_present".
10/21/87  L. Shustek    Add boolean/TRUE/FALSE if not already defined.
10/23/87  L. Shustek    Add ADDRTYPE_IONET.
10/30/87  L. Shustek    Add LLC_UIA type for Data General protocols.
11/ 4/87  L. Shustek    Add extern disp_base.
 2/ 1/88  L. Shustek    Add extern data_version.
 5/17/88  L. Shustek    Add extern names_version.
 7/ 5/88  L. Shustek    Add pi_data.selected, and do_prescan global.
 7/ 7/88  D. Simon      Add sprintf/CONTEXT/CACHE stuff.
 8/23/88  D. Simon      Add various CTAGs.
 8/25/88  L. Shustek    Make some data "near" for faster reference.
 9/21/88  R. Earle      Add typedef for hardware_details_strings
11/ 8/88  L. Shustek    Increase number of detail lines from 200 to 300.
12/ 7/88  L. Shustek    Add extern size_dlc_addr.
 1/ 4/89  L. Shustek    Add MSG_HANDLE symbols for reassembled messages.
 2/15/89  L. Shustek    Change pif_cskip() macro to allow skip just past
                        the last field without trapping.  (But not others!)
 3/ 2/89  L. Shustek    Change MAX_SUM_LINE to 140 to accomodate worst-case
                        flags/size/time/src/dst line.
 3/16/89  V. Lasker     Add extern for no. of summary lines used.
 4/12/89  V. Lasker     Change CTAG_ISO to _MSG.
 4/24/89  L. Shustek    Add a copy of the assert() macro for internal errors.
 5/12/89  V. Lasker     Add typedef for snap_vendor table.
 6/29/89  L. Shustek	Add pi_trap_active.
12/ 8/89  L. Shustek	Add ADDRTYPE_X25, CTAG_X25, ADDRTYPE_HDLC.
 1/12/90  L. Shustek	Add support for PI forcing.
 3/05/90  P. Kavaler	Add ADDRTYPE_X25_LCN, ADDRTYPE_X25_CALL.
 5/08/90  AJ Nichols	Added CTAG_RMS
 5/18/90  L. Shustek	Add FORCEPI flag.
 9/11/90  P. Kavaler	Add ADDRTYPE_ETHER.
						Add force CTAG's.
 9/19/90  L. Shustek	Add ERR_GENERIC and ERR_INFO for show_err() calls.
 9/20/90  P. Kavaler	Add CTAG_FORCE_HDLC.
10/22/90  J. Davidson   Change hardware_details_strings to accept 14 strings. RFC1060 changes.
10/31/90  P. Kavaler	Add FRLOC definition.
12/10/90  P. DeLaSalle	Add do_real_time.
12/11/90  L. Shustek	Add EXPERT_DECODE switch.
 1/16/91  A. Shacham	Merging pifdecl.h into pi.h
 2/13/91  C. Lum		Add PITYPE_UNREGISTERED, change struct snap_vendor.
 3/11/91  P.de la Salle Add Expert flags, CTAG_NOVELL.
 3/15/91  C. Lum		register_addrtype() now returns a void *.
 3/25/91  C. Lum		Add register_addrtype().
 6/14/91  N. Ramji		Added PITYPE_EXPERT.
 7/ 1/91  L. Shustek	Add pi_info types.
 7/11/91  A. Leung		Add CTAG_LAVC.
 7/11/91  B. Shah		Add CTAG_VINES_APPL and CTAG_VINES_FORCE
 7/12/91  L. Shustek	Add forcepi_rules and "forcepi_rule" macro;
 						remove old forcepi symbols.
 7/23/91  C. Lum		Add changes for EXPERT PI registration.
 7/12/91  L. Shustek	Add forcepi_rules and "forcepi_rule" macro;
 7/24/91  P. Kavaler 	Remove old forcepi symbols, declare do_forcepi_rule.
 8/23/91  L. Shustek	Add charcode stuff for PIs to set character coding.
 8/28/91  L. Shustek	Add sy_mode_type for SY networks.
 8/30/91  L. Shustek	Add ADDRTYPE_IPX; increase MAX_ADDRTYPES from 15 to 20.
 9/ 3/91  L. Shustek	Prototype format_date.
 9/ 4/91  N. Ramji		Typo correction.
10/10/91  L. Shustek	Combine EXPERT_CAPTURE and EXPERT_DECODE into EXPERT.
10/23/91  P. Kavaler	Add iso_trap_active.
10/24/91  A. Leung		Prototyping additions
10/24/91  C. Lum		Add rt_routine to ether_handler struct.
10/30/91  C. Lum		Add GETM_LIMIT parameter to get_mem2().  Format is
						get_mem2(int size, GETM_LIMIT, int limit);
11/18/91  N. Ramji		Added parameters to prototype for cleaner compile.
12/18/91  C. First		Added FMT_FLIP to flip dlc addressses
01/20/92  N. Ramji		Updated prototype of debug_msg().
01/20/92  M. Cappizzi   Added defines to support limited use of get_mem requests.
02/19/92  C. First		Changed FMT_FLIP to FMT_SMT
04/15/92  P. Kavaler	Add CTAG_ISODE.
05/07/92  P. Kavaler	Add ADDRTYPE_FRELAY.
05/22/92  P. Kavaler	Add rt_routine and type name to snap vendor table.
08/10/92  P. Kavaler	Add SY_ROUTER, got rid of SY_OTHER and SY_HDLC_OTHER.
						Add ADDRTYPE_TRING, specify argument to find_ether_type.
09/24/92  G. Cressman	Add CTAG_NICE for Decnet Nice.
10/ 5/92  N. Ramji		Add PITYPE_EXPUTIL for registering expert PIs but not
						adding an asterisk in the menu (useful for adding
						expert utilization statistics for PIs not currently
						decoded).
10/12/92  J. Davidson	Change allow more strings for hardware_detail_strings.
12/03/92  P. Kavaler	Add #define's for xxx_SRC_ROUTING.
12/24/92  A. Leung		Add pi_trap_ptr_first to help fix bug #10901
12/29/92  P. Kavaler	Add embedded_addrtype for synchronous.
01/04/93  P. Mandal     Added ASN_DONTWARN; used to disable the display	of
                        warning box.
01/15/93  C. Lum	    Change MAX_SUM_LINE to 143 to accomodate FDDI timestamp.
 1/21/93  P. Kavaler	Fix compiler warnings about ether_handler.routine.
 3/18/93  P. Kavaler	Add dlc_error so frame-assembly interpreters can avoid
						frames with DLC errors.
 4/21/93  P. Kavaler	Add VITALINK_SRC_ROUTING, NETRONIX_SRC_ROUTING,
						CROSSCOM_SRC_ROUTING.
 7/16/93  P. Kavaler	Increase MAX_INT_LINES to 400.
						Add SY_CISCO and others to sy_mode_type.
 8/31/93  P. Kavaler	Add SY_ATT, change token ring src routing to TR_ bits.
 9/14/93  K. Weinstein	Deleted XNS_STYLE flags, since IPX and XNS are treated
						as separate pi's.	
10/23/93  P. Kavaler	Fix #13148: get rid of WELLFLEET_SRC_ROUTING.
11/22/93  S.Kikkeri/M.Swanson Added function and data definitions for 64 bit 
                        PIF routines
12/15/93  N. Ramji		Renamed PITYPE_EXPUTIL to PITYPE_EXPSTATS.
12/15/93  N. Ramji		Moved PITYPE_ constants to interp.h in an ongoing
						process to cull non-PI stuff out of pi.h.
 1/06/94  P. Kavaler	Add SY_XYPLEX.
						Replace TR_LOW_ROUTING_BIT with TR_REVERSE_BITS. 
						Add ETH_WELLFLEET.
04/01/94  M. Akselrod Added SNA address structure and definitions.
05/11/94  K. Weinstein	Added CTAG_NDSRQST and CTAG_NDSRPLY for Novell's
						NetWare Directory Services PI message reassembly.
 9/21/94  P. Kavaler	Undo SNA address changes.
10/04/94  P. Kavaler	Add TR_RI_PADDING for netbios over dls.
						Increase MAX_INT_LINES to 600.
11/15/94  R. Tacdol  Added pif_show_nbytes_hex2().
 2/23/95  P. Kavaler    Change val_name to use int instead of long, add
							long_val_name for the few who need it (saves
							a lot of memory).

 4/21/95  P. Kavaler	Add ADDRTYPE_LAPD, ADDRTYPE_ISDN and sy_line_type.
						Add SYNC_ADDR_ bit definitions.
 6/26/95  P. Kavaler	Replace SY_ISDN_PRI_T1 & E1 with SY_ISDN_TPOD.
 9/25/95  P. Kavaler	Switch SY_ISDN_TPOD and SY_ISDN_BRI so we can ship
							SY_ISDN_TPOD without shipping SY_ISDN_BRI.
10/11/95  P. Kavaler	Add SY_COMBINET to sy_mode_type.
11/07/95  S. Macdonald  Added Indexed_Choice_Type to support sprintf("%!j").
11/07/95  SJM           Added ADDRTYPE_VINES_TRANS to support VINES-IP Transport
				        layer addressing. More of a formality as we only intend 
                        to use in names table for now.
01/22/96  A. Weaver     Added structures for %!h, %!i and %!j sprintf formats.
 3/04/96  P. Kavaler	Add PICK_STRING_CHAR_VALUES and rename PICK_STRING_*.
 4/02/96  J. Hardin		Remove PPP changes (moved to "elm").
06/17/96  M. Swanson    Add PI_MEM_LIMIT define for get_mem2 third parameter.
08/04/96  G. Gilliom    Added 3rd parameter to create_cache functional prototype.
                        Added define for size of cache header.
08/26/96  A.Babulevich	Added ASYNC_PPP, SY_ASYNC mode and AS_FRAME_PPP for async mode.
------------------------------------------------------------------------------*/

/*
	JJB Additions

	All of the Network Sniffer functions and globals are put into their own namespace
	to limit the potential for collision with other code.  This was originally developed 
	for a single application (a Sniffer extension) which uses all global variables for 
	access.

	WARNING

	None of this code is thread safe.  There is one global that points to the "current"
	BACnetPIInfo object.  Because in VTS this is called from the main GUI thread and 
	doesn't let other messages interfere, this should be OK.
*/

//
//	BACnetPIDetail
//

#define MAX_SUM_LINE    143             /* maximum width of a summary line */
#define MAX_SUM_LINES    12             /* maximum summary lines per packet */
#define MAX_INT_LINE    500             /* maximum width of a detail line */
#define MAX_INT_LINES   5000            /* maximum detail lines per packet */

struct BACnetPIDetail {
	int			piOffset;
	int			piLen;
	char		piLine[MAX_INT_LINE];
	};

typedef BACnetPIDetail *BACnetPIDetailPtr;

//
//	BACnetPIInfo
//

class BACnetPIInfo {
	public:
		// list the protocol interpreter types
		enum ProtocolType
			{ ipProtocol = 0
			, ethernetProtocol
			, arcnetProtocol
			, mstpProtocol
			, ptpProtocol
			, msgProtocol					// message from application
			};

		char				*piBuffer;		// beginning of buffer
		int					piLen;			// length
		int					piOffset;		// offset for current pi

		bool				doSummary;		// generate summary line
		bool				doDetail;		// generate detail lines

		char				summaryLine[MAX_SUM_LINE];
		BACnetPIDetailPtr	detailLine[MAX_INT_LINES];
		int					detailCount;

		BACnetPIInfo( bool summary = false, bool detail = false );
		~BACnetPIInfo( void );

		void SetPIMode( bool summary, bool detail );	// set the mode for all of the PI's
		void Interpret( ProtocolType protocol, char *header, int length );
	};

typedef BACnetPIInfo *BACnetPIInfoPtr;

extern BACnetPIInfoPtr		gCurrentInfo;

/* put all the rest of this cruft in a namespace */
#define USENAMESPACE	1

#if USENAMESPACE
namespace NetworkSniffer {
#endif

/* are we a little endian (PC) or big endian (Macintosh) */
#define LITTLEENDIAN	1
#define BIGENDIAN		0

/* might take advantage of this in some cases */
#define	near		

/* PICK_STRINGS_INT needed a size, can no longer declare an empty array */
#define	PICK_STRINGS_INT_SIZE		16

/* a most disgusting hack of sprintf hack */
void xsprintf( char *dst, char *prstr, char *hdr, char *valu );

/*----------------------------------------------------------------------------

								Miscellaneous symbols

-----------------------------------------------------------------------------*/

#ifndef FORCEPI
#define FORCEPI			1 				/* generate "force PI" code? */
#endif

#ifndef EXPERT
#define EXPERT			((OP1 & 0x20) != 0) /* expert analysis */
#endif

#ifndef PI_STUB
#define PI_STUB			45000			/* limit memory aquisition for */
#endif									/* sniffer/expert coexistence */

#ifndef NULLP
#define NULLP (char *) 0                /* large memory model null pointer */
#endif

#ifndef TRUE
typedef int  boolean;                   /* word-sized boolean */
#define TRUE            1               /* Note: as a word, the msbyte is zero */
#define FALSE           0
#endif

#ifndef ASYNC_PPP
#ifdef	OP1
#define ASYNC_PPP       ((OP1 & 0x1000)!=0) /* Enable async PPP for hs sniffer ? */
#else
#define ASYNC_PPP		0
#endif	/* OP1 */
#endif	/* ASYNC_PPP */

/* type for holding binary coded decimal (BCD) representation of
	a 64 bit numbers.  Each byte holds one BCD digit.  The least
	significant digit is in the zero-th element */
#define BCD_64_SIZE     20              /* max width of decimal value of 64 bit number */
typedef unsigned char BCD64[BCD_64_SIZE];

typedef unsigned long int DLONG[2];

typedef long FRNUM;                     /* frame number type */
typedef unsigned long FRLOC;			/* location of frame header */
typedef unsigned int PORT;				/* port type */

enum sy_mode_type {SY_SNA,      SY_X25, 	 SY_FRELAY,     SY_ROUTER,
#if ASYNC_PPP
				   SY_ASYNC,
#endif	/* ASYNC_PPP */
				   SY_ETHER,    SY_TRING,
				   SY_CISCO,    SY_PPP,      SY_VITALINK,   SY_BANYAN,
				   SY_ACSYS,	SY_PROTEON,  SY_WELLFLEET,  SY_IBM,
				   SY_UB,	    SY_ACC,	     SY_MICROCOM,   SY_UB_HDLC,
				   SY_RETIX,    SY_DEC,	     SY_CROSSCOM,   SY_NOVELL,
				   SY_NAT,		SY_ATT,		 SY_XYPLEX,		SY_COMBINET,
				   SY_UNKNOWN
				 };
enum sy_line_type {SY_RS232,	SY_RS422,	 SY_RS423,		SY_V10,
				   SY_V11,		SY_V35,		 SY_TPOD,		SY_ISDN_TPOD,
				   SY_ISDN_BRI};
#if ASYNC_PPP
enum as_frame_type {AS_FRAME_PPP, AS_FRAME_SLIP, AS_FRAME_CRLF, AS_FRAME_RAW};
#endif	/* ASYNC_PPP */
#ifndef MSOFT                   
#define MSOFT 1                         /* define the compiler */
#endif

#if MSOFT
/* WAS pifdecl.h - start */

typedef void *ADDRTYPE_HANDLE;			/* handle to address type */

/*  Function prototype declarations for Protocol Interpreters   */

void	pif_init(struct pi_data *,void *,int );
void	pif_save(struct pif_info *);
void	pif_restore(struct pif_info *);
char	*pif_get_ascii(int ,int ,char *);
char	*pif_get_ebcdic(int ,int ,char *);
char	*pif_get_lstring(int ,char *);
char	*pif_line(int );
void	pif_show_ascii(int ,char *);
void	pif_append_ascii(const char *, const char *); /* JJB */
void	pif_show_ebcdic(int ,char *);
void	pif_show_lstring(char *);
void	pif_show_space(void);
void	pif_autoscroll(void);
void	pif_header(int ,char *);
void	pif_trailer(void);
int		pif_show_byte(char *);
void	pif_show_flag(char *, int);
void	pif_flag_w_hl(char *, int);
void	pif_flag_w(char *, int);
int		pif_show_flagbit(int ,char *,char *);
boolean pif_flagbit_w(int, char *, char *);
boolean pif_flagbit_w_hl(int, char *, char *);
int		pif_show_flagmask(int ,int ,char *);
void	pif_show_flag_string (char, int, char *, char **);
void	pif_show_flag_value (char, int, char *);
void	pif_show_2byte(char *);
int		pif_show_word(char *);
int		pif_show_word_hl(char *);
long	pif_show_long(char *);
long	pif_show_long_hl(char *);
void	pif_show_4byte(char *);
void	pif_show_5byte(char *);
void	pif_show_6byte(char *);
void	pif_show_7byte(char *);
void    pif_show_8byte(char *);
void    pif_show_nbytes_hex (char *, int);
void    pif_show_nbytes_hex2 (char *, int, int);
void 	pif_show_hbytes_str (char *);
void	pif_show_date(char *);
void	pif_show_date_hl(char *);
void 	pif_make_line (int, int, char *);
void	pif_add_str (char *, int);
int     pif_off_end (void);
char	*pif_off_endp (void);

char	*make_dos_datestr(int ,char *);
char	*make_dos_timestr(int ,char *);
char	*make_c_str(char *,char *,int ,int );
char	*format_date (unsigned long, struct date *, char []);

char	*get_int_line (struct pi_data *, int, int);
char	*get_sum_line (struct pi_data *);
char	*get_cur_int_line( void ); /* JJB */

struct pi_data	*register_pi (char *nstr, char *dstr, int type, void (*fn)(),
	void (*xfn)(), void *piptr, struct pi_data *pid, int count, int *array);

ADDRTYPE_HANDLE register_addrtype (char *, unsigned int, int, void (*)(), boolean (*)(), char *);

int (*pi_interp_addr(int)) (void *, int);
	/*	Or, in English:  a function with an integer argument (the PI number)
		that returns a pointer to a function.  That function (the PI) takes
		a pointer and an integer as arguments and returns an integer. */

int 	do_forcepi_rule (struct pi_data *, PORT, PORT, char *, int);

void	show_err (int, unsigned char *, ...);

int		pi_get_frame (FRNUM, void * *, void * *, void * *);
int		pi_get_frame_by_handle (unsigned long, char **, FRNUM *);
unsigned long pi_get_current_frame_handle (void);
void	pi_free_frame (void);
int		pi_invoke_pis (FRNUM);

//The following eight routines are used by the 64 bit PIF functions
void        pif_cget_dlong_hl (int, DLONG);
char        *uint64_to_str (DLONG, int, char *);
void        dtl_uint64_hl (char *);
char        *cget_uint64_hl_str (int, char *);
static void add_bcd64 (BCD64, BCD64);
static void init_bcd64 (void);
static void ulong_to_bcd64 (long unsigned int, BCD64);
static void double_bcd64 (BCD64);

char	*stradd (char *, char *);
char	*strzadd (char *, char *);
char	*strnadd (char *, char *, int);
char	*strznadd (char *, char *, int);

char	*strchr (char *, char);
char	*strrchr (char *, char);

void	*add_station_name (int, int, void *, char *);
boolean find_station_name (char *, int, char * *, int *);
char    *format_addr (char *, int, void *, int);
boolean	add_frame_addr (int, int, void *);
void	add_frame_flags (char *);
char	*add_ebcdic_name (char *,char *,int ,int );
char	*add_hex_digits (char *,void *,int);
int		rev_word(int );
long	rev_long(long );
char    *get_code_name(struct code_struct *, long int);
char    *get_code_name_q(struct code_struct *, long int);
char	*get_code_range_name   (struct code_range_struct *, unsigned int); 
char	*get_code_range_name_q (struct code_range_struct *, unsigned int);
extern  struct ether_handler *find_ether_type(int);
int 	debug_msg (char *msg, ...);
void	detail_remaining_hex_bytes (char *, int);
char	*add_chosen_string (char *, char **, unsigned, unsigned);

void 	*allocate_context_data (int, int);  /* PI context routines */
void 	*fetch_context_data (int);

void	free_mem (void *);
char	*get_mem (unsigned int);
char	*get_mem2 (unsigned int, int,...);

void	*working (unsigned char *);
void	working_percent (void *, int);
void	done_working (void *, char *);

#if 0
/* Use sprintf in stdio, no need for fancy formatting  - JJB */

/* Prototype also in genl.h */
int sprintf (char *, const char *, ...);
#endif

/* end of WAS pifdecl.h */

#endif

#if 0
/* Use 'traditional' string functions - JJB */

/* Macros to turn traditional string functions into ours, whose advantages
   are:  (1) source string is decompressed, and (2) return is to the NUL
   character at the end of the string.

   We typecast the result to void so that any existing uses of the return
   value will be flagged by the compiler.
*/

#define strcpy(a,b)    ( (void) stradd   (a,b)   )
#define strcat(a,b)    ( (void) strzadd  (a,b)   )
#define strncpy(a,b,n) ( (void) strnadd  (a,b,n) )
#define strncat(a,b,n) ( (void) strznadd (a,b,n) )
#endif


#if 0
#define MAX_SUM_LINE    143             /* maximum width of a summary line */
#define MAX_SUM_LINES    12             /* maximum summary lines per packet */
#define MAX_INT_LINE    100             /* maximum width of a detail line */
#define MAX_INT_LINES   600             /* maximum detail lines per packet */
#endif


#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))   /* minimum value */
#endif
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))   /* maximum value */
#endif

#define assert(b,s) if (!(b)) bad_assertion (s) /* check logic error */

#define  ERR_INFO	  -2		/* generic info: caller supplies all text */
#define	 ERR_GENERIC  -1		/* generic error: caller supplies all text */


/*----------------------------------------------------------------------------

        Higher-level address types. 

You may add new types subject to the MAX_ADDRTYPES limit.
The address types are registered in initpi.c 

-----------------------------------------------------------------------------*/

#define ADDRTYPE_DLC          0    /* DLC (frame address) name type */
#define ADDRTYPE_IP           1    /* TCP/IP name type */
#define ADDRTYPE_XNS          2    /* XNS name type */
#define ADDRTYPE_ISO          3    /* ISO name type */
#define ADDRTYPE_DRP          4    /* DECNET DRP name type */
#define ADDRTYPE_IONET        5    /* Datapoint IONET name type */
#define ADDRTYPE_VINES        6    /* Banyan VINES */
#define ADDRTYPE_X25_LCN      7    /* X.25 LCN */
#define ADDRTYPE_ATALK        8    /* Appletalk */
#define ADDRTYPE_HDLC         9    /* HDLC */
#define ADDRTYPE_SNA		 10	   /* SNA oaf/daf */
#define ADDRTYPE_X25_CALL	 11	   /* X.25 Call */
#define ADDRTYPE_ETHER		 12	   /* Ethernet (for Sync bridge frame) */
#define ADDRTYPE_IPX		 13	   /* Novell's IPX */
#define ADDRTYPE_FRELAY		 14	   /* Frame relay (DLCI number) */
#define ADDRTYPE_TRING		 15	   /* Embedded Token ring (for Sync bridge) */
#define ADDRTYPE_LAPD		 16	   /* LAPD TEI */
#define ADDRTYPE_ISDN		 17	   /* ISDN call reference */
#define ADDRTYPE_VINES_TRANS 18	  /* VINES-IP Transport Layer Address */
#define MAX_ADDRTYPES        20    /* how many such name types there can be */
#define ADDRTYPE_UNKNOWN    255    /* Unknown name type */

#define ADDRSIZE             16    /* max size of station address in bytes */


struct ipx_address {			   /* format of ADDRTYPE_IPX */
	char	network [4];
	char	node [6];
	};

/* Bits in Synchronous DLC address (ADDRTYPE_DLC) */
#define SYNC_ADDR_IS_DTE		0x80	/* TRUE if DTE, FALSE if DCE */
#define SYNC_ADDR_CHAN_D		0x40	/* TRUE if D channel, FALSE if not */
#define SYNC_ADDR_CHAN_B		0x1f	/* B channel number, 0 if none */

/*----------------------------------------------------------------------------

                        Frame context tags

These are the identifying tags for "context data" stored with each frame
during the initial prescan and stored with each frame for subsequent
interpretation. The length and format of the context data is known only by
the PI.

You may define new tags and use them in allocate_context_data() and
fetch_context_data() calls.  The total amount of context data and magnitude
of the tags are limited by the implementation. 

-----------------------------------------------------------------------------*/

/* Max 31 CTAGs, 8 bytes per tag, 16 bytes total with 1 byte overhead per tag */
#define CTAG_ATALK      1       /* Appletalk */
#define CTAG_VINES      2       /* Banyan VINES */
#define CTAG_MSG        3       /* MSG reassembly (Used by ISO and X) */
#define CTAG_SUN_RPC    4       /* SUN Remote Procedure Call */
#define CTAG_DEC        5       /* DECNET */
#define CTAG_IP         6       /* TCP/IP (Used now for UDP protocols) */
#define CTAG_AT         7       /* AppleTalk */
#define CTAG_XWIN       8       /* X Windows */
#define CTAG_X25        9       /* X.25 */
#define CTAG_RMS		10		/* Datapoint RMS */
#define CTAG_NOVELL    	11		/* Novell prescan	*/
#define CTAG_VINES_APPL 12    	/* Banyan VINES service */
#define CTAG_LAVC       13		/* LAVC prescan */
#define CTAG_ISODE      14		/* ISODE */
#define CTAG_NICE       15      /* DECnet NICE */
#define CTAG_NDSRQST	16		/* Novell's NDS rqsts (needed for reassembly) */
#define CTAG_NDSRPLY	17		/* Novell's NDS rplys (needed for reassembly) */


/*----------------------------------------------------------------------------

                Symbols for pi_info() calls made by PIs

These symbols are used for calls in initaddr.c to pi_info, whereby the
PIs tell about themselves and any unusual abilities or characteristics.

-----------------------------------------------------------------------------*/

enum pi_info_types {
	PI_INFO_PORTS			/* tell about protocol ports */
	};

struct pi_info_ports {
	char *name;					/* pointer to name of port */
	boolean decimal;			/* should display in decimal instead of hex */
	int		numports;			/* number of ports per frame */
	int		portlength;			/* length of port in bytes */
	PORT min_port, max_port;	/* min and max port values */
	};

boolean pi_info (struct pi_data *, enum pi_info_types, void *);


/*----------------------------------------------------------------------------

                Symbols for PI character code specification

-----------------------------------------------------------------------------*/

struct charcode_table {		/* structure which defines a character code: */
	char	*title;			/* title of the character code */
	int		nchars;			/* number of characters */
	char	*to_ascii;		/* to-ASCII translate table; NULLP for ASCII */
	char	*from_ascii;	/* from-ASCII translate table; NULLP for ASCII */
	};

extern struct charcode_table/* built-in character codes: */
	code_ascii,				/* ASCII character code */
	code_ebcdic;			/* EBCDIC character code */

void set_charcode (			/* function to set character code for a frame: */
	struct pi_data *,		/* which PI is doing it */
	int,					/* offset of the frame area from msg_origin */
	int,					/* length of the frame area */
	struct charcode_table *	/* what character code that area uses */
	);

int get_forced_charcode (void);	/* 0 if not forced, 1 if ascii, 2 if ebcdic */


/*----------------------------------------------------------------------------

                Symbolic parameters passed to core routines

-----------------------------------------------------------------------------*/

/* format_stn() flags */

#define FMT_SRC         0x0100  /* is this a source address? */
#define FMT_BOTH        0x0200  /* both numeric and symbolic? */
#define FMT_PAD         0x0400  /* pad to "namewidth" characters if shorter? */
#define FMT_TRIM        0x0800  /* trim to "namewidth" characters if longer? */
#define FMT_NONAME      0x1000  /* suppress the name; do PI formatting on addr */
#define FMT_PARENS      0x2000  /* use xx(nn) instead of xx,nn if both hex and symbolic */
#define FMT_HEX         0x4000  /* use hex (not octal, etc.) for numeric part */
#define FMT_SMT			0x8000  /* bit flip the address before conversions */

/* add_station_name() flags */

#define ASN_SRC         FMT_SRC  /* 0x0100  is this a source address? */
#define ASN_REPL        0x0200  /* replace existing name */
#define ASN_NOREPL      0x0000  /* don't replace existing name */
#define ASN_MUSTADD     0x0400  /* must add, even if blank name quota exceeded */
#define ASN_DONTWARN    0x0800  /* don't display warning box */

/* add_frame_addr() flags */

#define AFA_SRC         FMT_SRC /* 0x0100  this a source address */
#define AFA_DST         0x0000  /* this a destination address */
#define AFA_SYMBOLIC    0x0200  /* this is a symbolic address */
#define AFA_BROADCAST   0x0400  /* this is a broadcast/multicast address */

/* get_mem2() flags */

#define GETM_COND       0x0001  /* this is a conditional request */
#define GETM_NORMAL     0x0002  /* this must be "normal" memory */
#define GETM_LIMIT		0x0004	/* allocated only if free memory left would be */
								/* greater than 3rd parameter */
#define PI_MEM_LIMIT    10000   /* 10000 byte limit used as the get_mem2 */
                                /* third parameter in the PIs            */


/*----------------------------------------------------------------------------

                Protcol Interpreter symbols

-----------------------------------------------------------------------------*/

/*
        The Protocol Interpreter data structure, whose address
        is return by the register function register_pi().
*/

struct  pi_data {
        boolean     do_sum;         /* generate summary lines? */
        boolean     do_int;         /* generate interpretation (detail) lines? */
        boolean     do_count;       /* only count summary lines? */
        boolean     do_names;       /* add symbolic station names? */
        boolean     recursive;      /* is this a recursive call? */
        boolean     selected;       /* is this PI selected in the menu? */
		struct forcepi_rule	*forcepi_rules;	/* linked list of "Force PI" rules */
        };


/*	This macro returns the number of bytes processed by a protocol to
	which we were forced.
*/

#define check_forcepi_rules(pid, src_port, dst_port, datap, datalen)		\
	(pid->forcepi_rules == NULLP ? 0								  	\
		: do_forcepi_rule (pid, src_port, dst_port, datap, datalen))



/*
        LLC frame types for frames with information field.
        Stored in llc_type.
*/

  extern  int near   llc_type;          /* type as follows: */

  #define LLC_I         0               /* Sequenced information */
  #define LLC_UI        1               /* Unnumbered information */
  #define LLC_TEST      2               /* Test probe */
  #define LLC_XID       3               /* Exchange ID */
  #define LLC_FRMR      4               /* Frame reject */
  #define LLC_UIA       5               /* Unnumbered information acked (for DG) */


/*  Embedded Ethernet interpreter:  3rd argument */

#define	ETH_NORMAL 				0		/* Dst addr, src addr, len, routing */
#define ETH_INCLUDES_FCS		0x01	/* Checksum at end of frame */	
#define	ETH_NO_SRC_ROUTING		0x02	/* Dst addr, src addr, len */
#define ETH_WELLFLEET			0x04	/* Wellfleet router */

/*  Embedded Token Ring interpreter:  3rd argument */

#define TR_NORMAL				0		/* Ordinary token ring format */
#define TR_INCLUDES_FCS			0x01	/* Checksum at end of frame */
#define	TR_REVERSE_BITS			0x02 	/* Reverse address bits */
#define TR_NO_MAC_HEADER		0x04	/* No MAC header */
#define TR_TWO_EXTRA_BYTES		0x08	/* Two bytes between src addr and RI */
#define TR_FORCE_SRC_ROUTING	0x10	/* Force source routing */
#define TR_RI_PADDING			0x20	/* RI field padded to 18 bytes! */

/*-----------------------------------------------------------------------------

                        External references

------------------------------------------------------------------------------*/

extern  char *  near dlc_header;        /* ptr to start of frame */
extern  char *  near msg_origin;        /* ptr to start of frame or message */
extern  int     near offset_src_addr;   /* offset to frame source address */
extern  int     near offset_dst_addr;   /* offset to frame destination address */
extern  int     near size_dlc_addr;     /* size in bytes of DLC addresses */
extern  int     near bytes_not_present; /* Bytes not in frame due to Truncation */
extern  int     near true_size;         /* True original size of the frame */
extern  FRNUM   near pi_frame;          /* the current frame number */
extern  char *  near pi_frame_data;     /* ptr to data space in the frame header */
#define PI_FRAME_DATA_SIZE  3           /* number of such bytes available */
extern  int     near disp_base;         /* the number display base, when variable */
extern  int     near data_version;      /* sequence number for capture data */
extern  int          names_version;     /* sequence number for names table */
extern  boolean near do_prescan;        /* are we doing a prescan of frames? */
extern  int     near n_summary_lines;   /* no of summary lines currently used */
extern	int		near embedded_addrtype; /* if sync, type of embedded DLC addr */
extern  boolean near dlc_error;			/* CRC or other data link error */


/*-----------------------------------------------------------------------------

                        PIF symbols

------------------------------------------------------------------------------*/


/* type for 64 bit unsigned integers */
#define DLONG_MSL	1		/* position of most significant long */
#define DLONG_LSL	0		/* position of least significant long */

#define MAX_HEADER_LINE 64              /* maximum size of header string */

struct  pif_info {
        struct  pi_data *pi;            /* Ptr to our pi */
        int     offset;                 /* Current offset from msg_origin */
        int     end_offset;             /* Offset for last+1 byte */
        char    header_msg [MAX_HEADER_LINE]; /* header message text */
        };

/*      PIF external routines     */

#ifndef PIF_MAIN
extern  struct  pi_data * near pif_pi;  /* Ptr to current pi */
extern  int     near pif_offset;        /* Current offset */
extern  int     near pif_end_offset;    /* Offset for last+1 byte */
extern  int     near pif_flagbit_indent;/* Number of blanks for flagbit */
extern  char    pif_header_msg[];       /* Saved header message */

extern  long    rev_long();             /* Reverse a long word */
extern  char    *pif_get_ascii();       /* move asciiz string */
extern  char    *pif_get_ebcdic();      /* move ebcdic string */
extern  char    *pif_get_lstring();     /* move asciiz lstring */
extern  char    *pif_line();            /* get a detail line buffer */
#endif


/*      PIF macros      */

/* The following MACROs will check the last byte addressed for End of Frame 
   If the last byte is greater than pif_end_offset the MACRO will execute
   pif_off_end and return to the most recent end-of-frame trap routine. */

#define pif_cget_byte(n)     ( (pif_offset+n >= pif_end_offset)  ?  (char)pif_off_end() : \
                             (*(msg_origin + pif_offset + (n))) )
#define pif_cget_word(n)     ( (pif_offset+n >= pif_end_offset-1)  ?  pif_off_end() : \
                             (*(int *)(msg_origin + pif_offset + (n))) )
#define pif_cget_word_hl(n)  ( (pif_offset+n >= pif_end_offset-1)  ?  pif_off_end() : \
                             (rev_word(*(int *)(msg_origin + pif_offset + (n)))) )
#define pif_cget_long(n)     ( (pif_offset+n >= pif_end_offset-3)  ?  (long)pif_off_end() : \
                             (*(long *)(msg_origin + pif_offset + (n))) )
#define pif_cget_long_hl(n)  ( (pif_offset+n >= pif_end_offset-3)  ?  (long)pif_off_end() : \
                             (rev_long(*(long *)(msg_origin + pif_offset + (n)))) )

#define pif_cget_addr()      ( (pif_offset >= pif_end_offset)  ?  pif_off_endp() : \
                             (msg_origin + pif_offset) )
#define pif_cset(p)          ( (pif_offset+p >= pif_end_offset)  ?  pif_off_end() : \
                             (pif_offset = ((char *)(p) - msg_origin)) )
#define pif_cskip(n)         ( (pif_offset+n > pif_end_offset)  ?  pif_off_end() : \
                             (pif_offset += (n)) )
/* Note: pif_cskip() uses ">" instead of ">=" to allow it to skip just past
         the last field of an area being interpreted. */


/* The following MACROs will not check the last byte addressed for End of Frame */

#define pif_get_byte(n)      ((unsigned char)(*(msg_origin + pif_offset + (n))))

#if LITTLEENDIAN
/*  These are little endian defines - JJB */
#define pif_get_word(n)      (*(short *)(msg_origin + pif_offset + (n)))
#define pif_get_word_hl(n)   (rev_word(*(short *)(msg_origin + pif_offset + (n))))
#define pif_get_long(n)      (*(long *)(msg_origin + pif_offset + (n)))
#define pif_get_long_hl(n)   (rev_long(*(long *)(msg_origin + pif_offset + (n))))
#endif

#if BIGENDIAN
/* These are big endian defines - JJB */
#define pif_get_word(n)      (rev_word(*(short *)(msg_origin + pif_offset + (n))))
#define pif_get_word_hl(n)   (*(short *)(msg_origin + pif_offset + (n)))
#define pif_get_long(n)      (rev_long(*(long *)(msg_origin + pif_offset + (n))))
#define pif_get_long_hl(n)   (*(long *)(msg_origin + pif_offset + (n)))
#endif

#define pif_get_addr()       (msg_origin + pif_offset)
#define pif_set(p)           (pif_offset = ((char *)(p) - msg_origin))
#define pif_skip(n)          (pif_offset += (n))
#define pif_eof(n)           (pif_offset + n >= pif_end_offset)


struct pi_trap {        /* setjmp/longjmp structure; used for end-of-frame trap */
    int  jmp_buf[9];    /* retoff, retseg, sp, bp, si, di, ds, es, ss */
};
extern  struct pi_trap  *pi_trap_ptr;           /* ptr to pi setjmp trap */
extern  struct pi_trap  *pi_trap_ptr_first;     /* ptr to first setjmp trap */
extern  struct pi_trap  *pi_trap_ptr_last;      /* ptr to last setjmp trap */
extern  boolean	near	pi_trap_active;			/* is the pi trap armed? */
extern  boolean	near	iso_trap_active;		/* is the ISO trap armed? */


/*----------------------------------------------------------------------------

                Symbols for the extended sprintf() function 

-----------------------------------------------------------------------------*/

#define LINE_OVERRUN 1
#define GET_SUM_LINE (char *) 1
#define GET_INT_LINE (char *) 2

/*----------------------------------------------------------------------------

                Structures for the extended sprintf() function
                See "Specification and User Guide Sprintf Enhancements"
                
-----------------------------------------------------------------------------*/
/* ===== Structure for %!h sprintf format ===== */
typedef struct {
   unsigned int  min_value;
   unsigned int  max_value;
   char         *string_loc;
} PICK_STRING_INT_RANGE;   
   
/* ===== Structure for %!i sprintf format ===== */
typedef struct {
   unsigned int  min_value;
   unsigned int  max_value;
   char         *strings[PICK_STRINGS_INT_SIZE];
} PICK_STRINGS_INT;   
   
/* ===== Structure for %!j sprintf format ===== */
typedef struct {
   unsigned int  index_value;
   char         *string_loc;
} PICK_STRING_INT_VALUES;   
   
/* ===== Structure for %k!j sprintf format ===== */
typedef struct {
   unsigned char  index_value;
   char         *string_loc;
} PICK_STRING_CHAR_VALUES;   

/*-----------------------------------------------------------------------------

                Symbols for the caching routines

------------------------------------------------------------------------------*/

typedef char CACHE_DATA;

int create_cache (unsigned, unsigned, char *);
CACHE_DATA *allocate_cached_data (int, unsigned);
CACHE_DATA *search_cache (int, int (*) (), void *);
void delete_cached_data (int, CACHE_DATA *);
void delete_cache (int);

#define MAX_CACHE_ENTRY_SIZE 1024
#define INVALID_CACHE_HANDLE -1
#define CACHE_HDR 12
#define CACHE_ENTRY_HDR 8


/*-----------------------------------------------------------------------------

                Symbols for message reassembly

------------------------------------------------------------------------------*/

typedef void * MSG_HANDLE;
#define NULL_HANDLE NULLP

MSG_HANDLE msg_map_start (struct pi_data *, void *, int);
void msg_map_add (MSG_HANDLE, int, int, FRNUM, int);
void msg_map_activate (MSG_HANDLE);


/*-----------------------------------------------------------------------------

					 Symbols for non-PIF utility routines

------------------------------------------------------------------------------*/


/* Structure used with "%!j" sprintf format. Used as an array of
	null-terminated structures with the last member being 0, nullp.
	The last non-null item in array will will be the default.
*/

typedef struct
{
	unsigned int iIdx;		/* Unique Index Number 0..65535 	*/
	char *		 cDescr;		/* Text descriptor. 					*/
} Indexed_Choice_Type;

/*
	General format for lookup from number to name.  If the values are dense,
	they can be put in an array.  If they are sparse, they can be put in
	a list which will be searched.
*/

struct long_val_name {           /* a single value-name pair */
	long value;
	char *name;
};

struct val_name {               /* a single value-name pair */
    int value;
    char *name;
};

/* If max_value is -1, then randoms points to long_val_name */
struct code_struct {            /* number-to-name lookup table */
    long int min_value;
    long int max_value;
    char **names;               /* Array of names from min_value to max_value */
    struct val_name *randoms;   /* Array of val_name structures */
};

struct code_range_struct {      /* number_range-to-name lookup table */
    unsigned int min_value;                     
    unsigned int max_value;
    char        *name;          /* name to use if in range min_value to max_value */
};                              /* (last entry has NULLP) */


struct date {            /* date and time stamp */
    char    year; 
    char    month;
    char    day;
    char    hour;
    char    minute;
    char    second;
};


/* Format of ethertype table used by SNAP and ARP interpreters. */

struct ether_handler {
    unsigned int *ether_types;  /* ptr to list of ethertypes ending in 0 */
    char *name;                 /* ptr to (long) descriptive name */
    void (*routine)(char *, int, ...);	/* ptr to PI or NULLP if none */
    void (*rt_routine)(char *, int);/* ptr to expert PI or NULLP if none */
};

/* Format of snap_vendor table used by SNAP interpreter */

struct snap_vendor {
        char vendor_id [3];                     /* 3-byte vendor ID assigned by IEEE */
        char *vendor_name;                      /* vendor name string */
		char *type_name;						/* snap type name string */
        unsigned int *snap_types;				/* ptr to snap_type list */
	    void (**routine)();						/* ptr to PI or NULLP if none */
		void (*rt_routine)();					/* ptr to expert PI */
	    int parm3;                              /* optional 3rd parm. (for ddp) */
};

/* Structure for hardware_address_type_details. */

typedef struct {
        int min;
        int max;
        char *strings[18];
        } hardware_details_strings;

#if USENAMESPACE
}
#endif

/* end of pi.h */

/*
$Log$
Revision 1.3  2002/09/25 02:47:10  xyp
modified  MAX_INT_LINE    500

Revision 1.2  2002/09/20 09:46:47  xyp
modified MAX_INT_LINE = 200

Revision 1.1.1.1  2002/07/19 01:39:20  xyp
no message

Revision 1.3  2002/05/14 18:03:02  joelbender
Getting synchronized for the next release.

Revision 1.2  2001/08/14 19:59:47  joelbender
C9 bug fixes

Revision 1.1.1.1  2001/05/30 15:13:02  joelbender
Initial release

 * 
 *    Rev 3.67   26 Aug 1996 19:15:34   ANDYB
 * Added ASYNC_PPP, SY_ASYNC and AS_FRAME_PPP for async mode.
 * 
 *    Rev 3.61.1.5   14 Aug 1996 13:05:20   ANDYB
 * Merge with SNIFFER_5.05
 * 
 *    Rev 3.66   06 Aug 1996 10:26:42   gilliom
 *  
 * 
 *    Rev 3.65   18 Jun 1996 12:05:34   swanson
 * 
 *    Rev 3.64   02 Apr 1996 14:00:40   hardin
 * Removed PPP changes.
 * 
 *    Rev 3.61.1.4   09 Jul 1996 16:42:02   ANDYB
 * Change #ifdef ASYNC_PPP to #if, also define ASYNC_PPP here for
 * PI and RTPI files.
 * 
 *    Rev 3.61.1.3   27 Jun 1996 18:36:52   ANDYB
 * Add as_frame_type for ASYNC_PPP.
 * 
 *    Rev 3.61.1.2   27 Jun 1996 15:43:14   ANDYB
 * Change SY_ASYNC_PPP to SY_ASYNC.
 * 
 *    Rev 3.61.1.1   01 Apr 1996 20:27:06   andyb
 * Added SY_ASYNC_PPP mode fore ASYNC_PPP.
 * 
 *    Rev 3.63   07 Mar 1996 18:04:40   andyb
 * No changes, journal update
 * 
 *    Rev 3.62   07 Mar 1996 16:18:34   andyb
 * Added ASYNC_PPP mode *only* for hs sniffer with ASYNC_PPP enabled.
 * 
 *    Rev 3.61   04 Mar 1996 22:24:46   peterk
 * 
 *    Rev 3.60   23 Jan 1996 11:44:30   unknown
 * 
 *    Rev 3.59   11 Nov 1995 11:55:00   swanson
 * 
 *    Rev 3.58   26 Oct 1995 15:50:04   peterk
 * 
 *    Rev 3.55.1.2   30 Sep 1995 14:16:46   peterk
 * 
 *    Rev 3.55.1.1   08 Aug 1995 16:08:26   peterk
 * 
 *    Rev 3.55   15 Nov 1994 23:11:02   REYT
 *  
 * 
 *    Rev 3.54   06 Oct 1994 10:57:52   peterk
 * 
 *    Rev 3.53   20 Jul 1994 17:09:42   swanson
 * 
 *    Rev 3.51   26 May 1994 11:57:04   peterk
 * 
 *    Rev 3.50   15 Dec 1993 13:41:46   ramji
 *  
 * 
 *    Rev 3.49   22 Nov 1993 21:05:36   davidson
 * 
 *    Rev 3.48   25 Oct 1993 11:46:10   peterk
 * 
 *    Rev 3.47   27 Sep 1993 14:28:12   peterk
 * Add ATT router, TR_* falgs, rename NETRONIX to ACSYS
 * 
 *    Rev 3.45   21 Sep 1993 23:58:20   davidson
 * 
 *    Rev 3.44   02 Sep 1993 16:15:46   ramji
 *  
 * 
 *    Rev 3.43   26 Jan 1993 14:13:26   ramji
 *  
 * 
 *    Rev 3.42   19 Jan 1993 12:59:06   ramji
 *  
 * 
 *    Rev 3.41   12 Jan 1993 14:02:48   ramji
 *  
 * 
 *    Rev 3.40   04 Jan 1993 15:44:30   ramji
 *  
 * 
 *    Rev 3.39   29 Dec 1992 11:09:26   davidson
 * 
 *    Rev 3.38   15 Dec 1992 22:16:34   Lum
 *  
 * 
 *    Rev 3.37   13 Oct 1992 20:47:42   ramji
 *  
 * 
 *    Rev 3.36   06 Oct 1992 21:48:36   ramji
 *  
 * 
 *    Rev 3.35   29 Sep 1992 11:09:12   ramji
 *  
 * 
 *    Rev 3.34   08 Sep 1992 18:10:02   davidson
 *  
 * 
 * 
 *    Rev 3.33   15 Apr 1992 15:24:24   Lum
 *  
 * 
 *    Rev 3.32   24 Feb 1992 15:13:16   Lum
 *  
 * 
 *    Rev 3.31   20 Jan 1992 22:11:06   Lum
 *  
 * 
 *    Rev 3.30   06 Jan 1992 19:38:32   Lum
 *  
 * 
 *    Rev 3.29   18 Nov 1991 14:14:00   shustek
 *  
 * 
 *    Rev 3.28   04 Nov 1991 13:52:08   shustek
 * 
 *    Rev 3.27   28 Oct 1991 16:24:36   shustek
 * 
 *    Rev 3.26   14 Oct 1991 14:35:18   shustek
 * 
 *    Rev 3.25   04 Sep 1991 21:09:56   shustek
 * 
 *    Rev 3.24   03 Sep 1991 18:11:08   shustek
 * 
 *    Rev 3.23   30 Aug 1991 20:48:08   shustek
 * 
 *    Rev 3.22   28 Aug 1991 20:21:34   shustek
 * 
 *    Rev 3.21   27 Aug 1991 10:30:58   shustek
 * 
 *    Rev 3.20   24 Jul 1991 17:41:34   shustek
 * 
 *    Rev 3.19   24 Jul 1991 11:44:32   shustek
 * C. Lum
 * 
 *    Rev 3.18   22 Jul 1991 10:32:38   shustek
 * 
 *    Rev 3.17   11 Jul 1991 14:20:40   shustek
 * 
 *    Rev 3.16   10 Jul 1991 16:33:56   shustek
 * L. Shustek: Add pi_info.
 * 
 *    Rev 3.15   24 Jun 1991 17:34:46   shustek
 * N. Ramji: Add PITYPE_EXPERT.
 * 
 *    Rev 3.14   26 Mar 1991 14:55:40   shustek
 * C. Lum change
 * 
 *    Rev 3.13   15 Mar 1991 11:24:48   shustek
 * P. DeLaSalle changes for expert system; CTAG_NOVELL
 * 
 *    Rev 3.12   05 Mar 1991 12:08:34   shustek
 * C. Lum changes
 * 
 *    Rev 3.11   16 Jan 1991 13:22:30   shacham
 * pifdecl.h was merged into pi.h
 * 
 *    Rev 3.10   11 Dec 1990 19:59:02   shustek
 * Initial version control
*/
