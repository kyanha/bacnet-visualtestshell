//Contents of VTSAPI.H
#ifndef __VTSAPI_H_INCLUDED
#define __VTSAPI_H_INCLUDED

typedef unsigned char byte;
typedef unsigned char octet;
typedef unsigned long dword;
typedef unsigned short word;
typedef unsigned short uint;

#include	"db.h"
#include	"stdobj.h"
#include    "VTS.h"

#ifndef _etable
typedef struct {
	word	propes;								//proprietary enumerations begin at this value
	dword	propmaxes;							//max value for proprietary+1
	word	nes;								//number of string pointers which follow
	char	*estrings[1];						//table of pointers to strings
	} etable;
#define _etable 0
#endif

typedef struct {
	char	*name;
	dword	dwcons;
	} namedw;

typedef struct {
	char	*name;
	octet	octetcons;
	} nameoctet;

// structure to hold default property value restrictions defined in EPICS
typedef struct {
	double long dlUnsignedMinimum;
	double long dlUnsignedMaximum;
	double long dlSignedMinimum;
	double long dlSignedMaximum;
	float  fRealMinimum;
	float  fRealMaximum;
	float  fRealResolution;
	double dDoubleMinimum;
	double dDoubleMaximum;
	double dDoubleResolution;
	BACnetDate dateMinimum;
	BACnetDate dateMaximum;
	double long dlMaxOctetStringLength;
	double long dlMaxCharacterStringLength;
	double long dlMaxListLength;
	double long dlMaxVariableLengthArray;
} defaultrangelimits;


typedef	struct {
	char	VendorName[64];
	char	ProductName[128];
	char	ProductModelNumber[64];
	char	ProductDescription[128];
	word	BACnetConformanceClass;
	word	MaxAPDUSize;
	octet	SegmentedRequestWindow;
	octet	SegmentedResponseWindow;
	word	RouterFunctions;
	dword	BACnetFunctionalGroups;				//bitmap of functional groups supported
	octet	BACnetCharsets;						//bitmap of std charactersets supported
	octet	BACnetStandardServices[MAX_SERVS_SUPP];			//array of standard services supported
	octet	BACnetStandardObjects[MAX_DEFINED_OBJ];			//array of standard objects supported
	octet	DataLinkLayerOptions[MAX_DATALINK_OPTIONS];     //array of data link layers supported
	dword	MSTPmasterBaudRates[16];
	dword	MSTPslaveBaudRates[16];
	dword	PTP232BaudRates[16];
	dword	PTPmodemBaudRates[16];
	dword	PTPAutoBaud[2];
	dword	BACnetFailTimes[MAX_FAIL_TIMES];                    //array of Fail Times           29/12/2003  GJB
	octet   BIBBSupported[MAX_BIBBS];  	//array of booleans indicating support for each BIBB
    defaultrangelimits defaultlimits;
	generic_object far *Database;
	octet   BBMD;                                 //indicates support for BBMD functionality
	} PICSdb;

//bits in RouterFunctions
#define		rfNotSupported			0
#define		rfSupported				1

//bits in BACnetFunctionalGroups
#define		fgHHWS					0x00000001
#define		fgPCWS					0x00000002
#define		fgCOVEventInitiation	0x00000004
#define		fgCOVEventResponse		0x00000008
#define		fgEventInitiation		0x00000010
#define		fgEventResponse			0x00000020
#define		fgClock					0x00000040
#define		fgDeviceCommunications	0x00000080
#define		fgFiles					0x00000100
#define		fgTimeMaster			0x00000200
#define		fgVirtualOPI			0x00000400
#define		fgReinitialize			0x00000800
#define		fgVirtualTerminal		0x00001000

//bits in BACnetStandardServices
#define		ssNotSupported			0
#define		ssInitiate				1
#define		ssExecute				2

//bits in BACnetStandardObjects
#define		soNotSupported			0
#define		soSupported				1
#define		soCreateable			2
#define		soDeleteable			4

//bits in DataLinkLayerOptions
//#define		dlISO88023_10BASE5		0x0001
//#define		dlISO88023_10BASE2		0x0002
//#define		dlISO88023_10BASET		0x0004
//#define		dlISO88023_fiber		0x0008
//#define		dlARCNETcoaxstar		0x0010
//#define		dlARCNETcoaxbus			0x0020
//#define		dlARCNETtpstar			0x0040
//#define		dlARCNETtpbus			0x0080
//#define		dlARCNETfiberstar		0x0100
//#define		dlMSTPmaster			0x0200
//#define		dlMSTPslave				0x0400
//#define		dlPTP232				0x0800
//#define		dlPTPmodem				0x1000
//#define		dlPTPautobaudmodem		0x2000
//#define		dlLonTalk				0x4000
//#define		dlOther					0x8000

//bits in BACnetCharsets
#define		csANSI					0x01		//ANSI X3.4
#define		csDBCS					0x02		//IBM/Microsoft DBCS
#define		csJIS					0x04		//JIS C 6226
#define		csUCS4					0x08		//ISO 10646 (UCS-4)
#define		csUCS2					0x10		//ISO 10646 (UCS-2)
#define     cs8859                  0x20        //ISO 8859-1

// enumerated BIBBs
enum {
	bibbDS_RP_A = 0,	
	bibbDS_RP_B,
	bibbDS_RPM_A,
	bibbDS_RPM_B,
	bibbDS_RPC_A,
	bibbDS_RPC_B,
	bibbDS_WP_A,
	bibbDS_WP_B,
	bibbDS_WPM_A,
	bibbDS_WPM_B,
	bibbDS_COV_A,
	bibbDS_COV_B,
	bibbDS_COVP_A,
	bibbDS_COVP_B,
	bibbDS_COVU_A,
	bibbDS_COVU_B,
	bibbAE_N_A,
	bibbAE_N_I_B,
	bibbAE_N_E_B,
	bibbAE_ACK_A,
	bibbAE_ACK_B,
	bibbAE_ASUM_A,
	bibbAE_ASUM_B,
	bibbAE_ESUM_A,
	bibbAE_ESUM_B,
	bibbAE_INFO_A,
	bibbAE_INFO_B,
	bibbAE_LS_A,
	bibbAE_LS_B,
	bibbSCHED_A,
	bibbSCHED_I_B,
	bibbSCHED_E_B,
	bibbT_VMT_A,
	bibbT_VMT_I_B,
	bibbT_VMT_E_B,
	bibbbibb_ATR_A,
	bibbT_ATR_B,
	bibbDM_DDB_A,
	bibbDM_DDB_B,
	bibbDM_DOB_A,
	bibbDM_DOB_B,
	bibbDM_DCC_A,
	bibbDM_DCC_B,
	bibbDM_PT_A,
	bibbDM_PT_B,
	bibbDM_TM_A,
	bibbDM_TM_B,
	bibbDM_TS_A,
	bibbDM_TS_B,
	bibbDM_UTC_A,
	bibbDM_UTC_B,
	bibbDM_RD_A,
	bibbDM_RD_B,
	bibbDM_BR_A,
	bibbDM_BR_B,
	bibbDM_R_A,
	bibbDM_R_B,
	bibbDM_LM_A,
	bibbDM_LM_B,
	bibbDM_OCD_A,
	bibbDM_OCD_B,
	bibbDM_VT_A,
	bibbDM_VT_B,
	bibbNM_CE_A,
	bibbNM_CE_B,
	bibbNM_RC_A,
	bibbNM_RC_B
};

//bits in Fail Times
#define		ftNotSupported			0           //29/12/2003        GJB

#ifdef __cplusplus					//so this header can be used with C++
extern "C" {
#endif
dword APIENTRY VTSAPIgetpropinfo(word,word,char *,word *,word *,word *,word *);
word  APIENTRY VTSAPIgetenumtable(word,word,word *,dword *,char *);
word  APIENTRY VTSAPIgetdefaultparsetype(dword,HWND);
BOOL  APIENTRY VTSAPIgetdefaultpropinfo(word,dword,word *,word *);
word  APIENTRY VTSAPIgetpropertystates(word,HWND);
word  APIENTRY VTSAPIgetpropertystate(word,word,char *);
bool  APIENTRY ReadTextPICS(char *,PICSdb *,int *,int *);
void  APIENTRY DeletePICSObject(generic_object *);
int   GetStandardServicesSize(void);
char *GetStandardServicesName(int i);
int   GetBIBBSize(void);
char *GetBIBBName(int i);
int   GetObjectTypeSize(void);
char *GetObjectTypeName(int i);
int   GetDataLinkSize(void);
void  GetDataLinkString(int i, PICSdb *pd, char *pstrResult);
int   GetCharsetSize(void);
char *GetCharsetName(octet csTag);
int   GetSpecialFunctionalitySize(void);
char *GetSpecialFunctionalityName(int i);


#ifdef __cplusplus					//end of extern "C" declarations
}
#endif
#endif //__VTSAPI_H_INCLUDED
