/*--------------------------------------------------------------------------------
	(c)1995-97, PolarSoft(r) Inc. and National Institute for Standards and Technology
	
module:		VTSAPI32.C
desc:		BACnet Standard Objects DLL v2.15
authors:	David M. Fisher, Jack R. Neyer

  
-- NOTE: 135.1 doesn't say how to encode missing OPTIONAL items in a SEQUENCE: 
-- should there be an emtpy pair of commas, or should the item and any comma be
-- omitted?  I believe that the comma MUST be maintained - else there is no
-- way to tell whether the item is present or not, unless it may be discerned
-- from the following item (if any), which isn't always possible.
-- If this interpretation is wrong, this decoder and the samepl EPICS
-- may need to be changed.

135.1 clause 4.4 says
- constructed items are enclosed by curly brackets { } with elements separated by commas
- CHOICE of application-tagged items are represented as such
- CHOICE of context-tagged items are represented by [n] followed by the item
- SEQUENCE-OF (list) are enclosed in parenthesis, with items separated by commas
- Array values are enclosed in curly brackets, with items separated by commas

	  
last edit: 
			16-Apr-2010 JLH Allow for longer strings in various sections
			07-SEP-2004 MSD implemented BIBBs section,
               135.1-2003 consistency tests.
			01-MAR-04 [020] GJB
				1. add a function: preprocstr
				2. modify the string array SectionNames
				3. modify a function: whatischoice
				
	    	29-DEC-03 [019] GJB parse the section: Fails Times
        	08-SEP-03 [018] GJB modified propFlags's index number in DV_CheckOptionalProperty function
			01-SEP-03 [018] GJB enable parser to parse several new properties
			10-Nov-02 [017] xyp added for parsing the value of property whose parse type is none.
			13-Aug-01 [016] JJB added namespace
			09-Dec-97 [015] DMF revise for 32 bit from v0.14
			27-Aug-96 [014] DMF fix 0.13 item 3.
			20-Aug-96 [013] DMF 
				1. add support for propflags in generic_object handling
				2. fix Exception Schedule parse for WeekNDay numeric form of days
				3. fix MSO, and AO parse after NULL in priority array
				4. fix whitespace tolerance in recipient lists
				5. fix parse of days of week
				6. fix daterange when (date .. date) i.e. space after date but not a day of week
				7. fix parse of timevalues
			05-Jul-96 [012] DMF
				1. fix parse of recipients, setpoint references
				2. objectpropertyrefs
				3. string items (setstring)
				4. unknown object-types
				5. parse of booleans
			30-Jun-96 [011] DMF fix parse of priority arrays
			13-Jun-96 [010] DMF really fix 0.09
			12-Jun-96 [009] DMF fix problem with marking std props as present
			28-May-96 [008] DMF support T/F in ParseBitstring
								fix parsing for ActionCommands
			25-May-96 [007] DMF add VTSAPIgetpropertystate
			23-May-96 [006] DMF add rtrim
			16-May-96 [005] DMF add VTSAPIgetdefaultpropinfo
			14-May-96 [004] DMF add VTSAPIgetpropertystates
			04-Dec-95 [003] JN  add VTSgetdefaultparsetype
			22-Sep-95 [002] DMF add array flag, change enum tables
			18-Aug-95 [001] DMF First Cut
-----------------------------------------------------------------------------*/ 

#include "stdafx.h"

#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <ctype.h>
#include <sys\stat.h>
#include <sys\types.h>
#include <assert.h>


namespace PICS {									//									***016

#define _DoStaticPropDescriptors 0

#include "db.h"
#include "stdobj.h"
#include "stdobjpr.h"
#include "vtsapi.h"
#include "VTS.h"
#include "EPICSConsCheck.h"
#include "dudapi.h"
	
/* Suppress pointless warnings of:
 *		warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead. 
 *      To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.	
 */
#pragma warning( disable : 4996 )


///////////////////////////////////////////////////////////////////////
//	function prototypes
BOOL ParseLogRec(BACnetLogRecord *);
BOOL ParseTimeStamp(BACnetTimeStamp *);
BOOL ParseTimeStampArray(BACnetTimeStamp **parray, int arraycount);
BOOL ReadFunctionalGroups(PICSdb *);	//										***008 Begin
BOOL ReadStandardServices(PICSdb *);
BOOL ReadStandardObjects(PICSdb *);
BOOL ReadDataLinkOptions(PICSdb *);
BOOL ReadSpecialFunctionality(PICSdb *);
BOOL ReadObjects(PICSdb *);
BOOL ReadCharsets(PICSdb *);			//										***008 End
BOOL ParseObjectList(BACnetObjectIdentifier **,word *);
BOOL ParseRASlist(BACnetReadAccessSpecification **);
BOOL ParseCalist(BACnetCalendarEntry **);
BOOL ParseTVList(BACnetTimeValue **);
BOOL ParseWeekdaySchedule(BACnetTimeValue *wp[]);
BOOL ParseExceptionSchedule(BACnetExceptionSchedule *);
BOOL ParseProperty(char *,generic_object *,word);
BOOL ParseDateTime(BACnetDateTime *);
BOOL ParseDate(BACnetDate *);
BOOL ParseTime(BACnetTime *);
BOOL ParseBitstring(octet *,word,octet *);
BOOL ParseOctetstring(octet *,word,word *);
BOOL ParseVTClassList(BACnetVTClassList **);
BOOL ParseAddressList(BACnetAddressBinding **);
BOOL ParseDestinationList(BACnetDestination **);
BOOL ParseRecipientList(BACnetRecipient **);
BOOL ParseEventParameter(BACnetEventParameter *);
BOOL ParseSessionKeyList(BACnetSessionKey **);
BOOL ParseRefList(BACnetDeviceObjectPropertyReference	**);
BOOL ParseDevObjList(BACnetDeviceObjectReference **,word );
BOOL ParsePrescale(BACnetPrescale* pt);
BOOL ParseAccumulatorRecord(BACnetAccumulatorRecord* pt);
BACnetRecipient *ParseRecipient(BACnetRecipient *);
BACnetObjectPropertyReference *ParseReference(BACnetObjectPropertyReference	*);
BOOL ParseCOVSubList(BACnetCOVSubscription **);												//*****018
BACnetCOVSubscription *ParseCOVSubscription(BACnetCOVSubscription *);						//*****018
BOOL ReadFailTimes(PICSdb *);																//*****019							
BOOL ReadBIBBSupported(PICSdb *);
BOOL ParseEnumList(BACnetEnumList **, etable *);
BOOL ParseUnsignedList( UnsignedList **elp );
BOOL ParseBooleanList( BooleanList **elp );

//************added by Liangping Xu,2002*****************//
BACnetDeviceObjectPropertyReference *ParseDevObjPropReference(BACnetDeviceObjectPropertyReference *);
BACnetDeviceObjectReference *ParseDevObjReference(BACnetDeviceObjectReference *);

//void CheckPICSObjCons(PICSdb *);
//void CheckObjConsK(PICSdb *);
//void CheckObjConsL(PICSdb *);
//void CheckObjConsM();
//void CheckObjConsF(PICSdb *);
//void CheckObjConsA(PICSdb *);
//void CheckPICSServCons(PICSdb *);
//void CheckServConsD(PICSdb *);
//void CheckServConsI(PICSdb *);
//void CheckServConsJ(PICSdb *);
//void CheckServConsE(PICSdb *);
//void CheckClass(word,TApplServ *,octet  ApplServ[],char *,BOOL);
//void CheckServClassCons(word,octet ApplServ[],char *,BOOL);
//void CheckServFGCons(dword,octet Applserv[],char *,BOOL);
//void CheckPICSPropCons(PICSdb *);      // continue
//void CheckObjRequiredProp(dword,dword,generic_object *,octet);         //continue
//void AI_CheckOptionalProperty(octet servFromPICS[],octet propFlag[]);
//void AO_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void AV_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void BI_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void BO_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void BV_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void CA_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void CO_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void DV_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void EE_CheckOptionalProperty(octet propFlags[]);
//void FI_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void GR_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void LO_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void MI_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void MO_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void MV_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void NC_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void PR_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void SC_CheckOptionalProperty(octet propFlags[]);
//void AVG_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);
//void TR_CheckOptionalProperty(octet servFromPICS[],octet propFlags[]);

// msdanner 9/04:  135.1-2003 EPICS consistency checks added 
void ExpandBitstring(octet *pExpandedResult, octet *pBitstring, int nBits);
void CheckPICSConsistency2003(PICSdb *); // runs all checks
void CheckPICS_BIBB_Cross_Dependency(PICSdb *pd, int iSupportedBIBB, int iDependentBIBB);
void CheckPICSConsProperties(PICSdb *pd, generic_object *obj);
void CheckPICSCons2003A(PICSdb *pd);
void CheckPICSCons2003D(PICSdb *pd);
void CheckPICSCons2003E(PICSdb *pd);
void CheckPICSCons2003F(PICSdb *pd);
void CheckPICSCons2003G(PICSdb *pd);
void CheckPICSCons2003H(PICSdb *pd);
void CheckPICSCons2003I(PICSdb *pd);
void CheckPICSCons2003K(PICSdb *pd);
void CheckPICSCons2003L(PICSdb *pd);


void GetSequence(dword *,dword *,octet);
void BitToArray(octet ObjServ[],DevProtocolSup *);
void PrintToFile(char *);
//
/////////////////////////////////////////////////////////////

BACnetActionCommand *ReadActionCommands(void);
BOOL setstring(char *,word,char *);
char *ReadNext(void);						//										***008
void skipwhitespace(void);					//										***008
char *openarray(char *);
char *strdelim(char *);						//										***008
dword ReadDW(void);
octet ReadB(octet,octet);
word  ReadW(void);
octet ReadBool(void);
octet whatIsChoice(char str[]);
word ReadEnum(etable *);
dword ReadObjID(void);
int CreateTextPICS(char *);
void readline(char *,int);					//										***008
char *Nxt(char *);
char *cvhex(char *,octet *);
BOOL MustBe(char);
BOOL tperror(char *,BOOL);
int ClearCBList(HWND);						//										***003 Begin
int AddCBListText(HWND,char *);
int SelectCBListItem(HWND,int);				//										***003 End
void rtrim(char *);							//										***006
void preprocstr(char *str);					//                                      ***020
///////////////////////////////////////////////////////////////////////
//	Module Constants
#define	tab							0x09
#define	space						0x20
#define	cr							0x0D
#define	lf							0x0A
#define	doublequote					0x22
#define	singlequote					0x27
#define	accentgrave					0x60
#define	badobjid					0xFFFFFFFF	//we presume that no one will use this as a valid object identifier
#define	BeginPics	                &EndPics[7]

// Initialize string pointers to an empty string,
// in case the values are referenced without being set.
PICSdb::PICSdb()
: VendorName(&dummy)
, ProductName(&dummy)
, ProductModelNumber(&dummy)
, ProductDescription(&dummy)
, dummy(0)
{
}

PICSdb::~PICSdb()
{
	// Free any allocated string memory
	if (VendorName != &dummy)
	{
		delete[] VendorName;
	}

	if (ProductName != &dummy)
	{
		delete[] ProductName;
	}

	if (ProductModelNumber != &dummy)
	{
		delete[] ProductModelNumber;
	}

	if (ProductDescription != &dummy)
	{
		delete[] ProductDescription;
	}
}

///////////////////////////////////////////////////////////////////////
// global variables
octet EPICSLengthProtocolServicesSupportedBitstring;    //msdanner 9/2004 - used by test 135.1-2003 (k)
octet EPICSLengthProtocolObjectTypesSupportedBitstring; //msdanner 9/2004 - used by test 135.1-2003 (l)
///////////////////////////////////////////////////////////////////////
// local variables
static bool		cancel=false;						//global cancel flag
static char		lb[256];							//line buffer (current line of input file)
static char		*lp;								//pointer into lb
static word		lc;									//current 1-based line in file
static int			lerrc;								//count of errors
static FILE		*ifile;								//current input file				***008
static int      lPICSErr;                           //added by xlp - but what do I do, and why?

//*****017 begin
static octet	newParseType;						
static word		newPropET;

static boolean bNeedReset=false, bHasReset = false,bHasNoneType=false;
static char		NoneTypeValue[256],NoneTypePropName[20];	 // for store temp address of lp
//******017 end

// msdanner 9/2004: added global consistency error counter
static unsigned int cConsistencyErrors;

// Array of expected bitstring lengths for ProtocolServicesSupported
// based on the Protocol_Revision property as the index.
static octet aCorrectLengthProtocolServicesSupportedBitstring[] = 
{
   35, /* Protocol_Revision = 0 */
   37, /* Protocol_Revision = 1 */
   40, /* Protocol_Revision = 2 */
   40, /* Protocol_Revision = 3 */
   40,  /* Protocol_Revision = 4 */
   40,  /* Protocol_Revision = 5 */
   40,  /* Protocol_Revision = 6 */
   40,  /* Protocol_Revision = 7 */
};

// Array of expected bitstring lengths for ProtocolObjectTypesSupported
// based on the Protocol_Revision property as the index.
static octet aCorrectLengthProtocolObjectTypesSupportedBitstring[] = 
{
   18, /* Protocol_Revision = 0 */
   21, /* Protocol_Revision = 1 */
   23, /* Protocol_Revision = 2 */
   23, /* Protocol_Revision = 3 */
   25,  /* Protocol_Revision = 4 */ // LJT updated 3/27/2007
   30,  /* Protocol_Revision = 5 */
   31,  /* Protocol_Revision = 6 */
   31,  /* protocol_revision = 7 */ // 135-2008
   31,  /* protocol_revision = 8 */ // 135-2008 added objects?
   38,  /* protocol_revision = 9 */  // 135-2008j added objects 32-37
   51,  /* protocol_revision = 10 */ // 135-2008w added 39-50

};

//---------------------------------------------------------------------
//  Large Static Tables
static char	picshdr[]="PICS 0\x0D\x0A";
static char	EndPics[]="End of BACnet Protocol Implementation Conformance Statement";
static char *SectionNames[]={
			"Vendor Name",                                        //0
			"Product Name",                                       //1
			"Product Model Number",                               //2
			"Product Description",                                //3
			"BACnet Conformance Class Supported",                 //4
			"BACnet Functional Groups Supported",                 //5
			"BACnet Standard Application Services Supported",     //6
//			"Standard Object-Types Supported",                    //7
			"Standard Object Types Supported",                    //7                 ***020
			"Data Link Layer Option",                             //8
			"Special Functionality",                              //9
			"List of Objects in test device",                     //10
			"Character Sets Supported",                           //11				***006
			"Fail Times",                                         //12				***019
			"BIBBs Supported",                                    //13  msdanner 8/31/04: added
			"Default Property Value Restrictions"                 //14  msdanner 8/31/04: added
			};
static namedw FunctionalGroups[]={
			"HHWS",							fgHHWS,
			"PCWS",							fgPCWS,
			"COV Event Initiation",			fgCOVEventInitiation,
			"COV Event Response",			fgCOVEventResponse,
			"Event Initiation",				fgEventInitiation,
			"Event Response",				fgEventResponse,
			"Clock",						fgClock,
			"Device Communications",		fgDeviceCommunications,
			"Files",						fgFiles,
			"Time Master",					fgTimeMaster,
			"Virtual Operator Interface",	fgVirtualOPI,
			"Reinitialize",					fgReinitialize,
			"Virtual Terminal",				fgVirtualTerminal
			};


// msdanner 09/2004 - added BIBBs
typedef struct {
    char                  InitExec;                      // flag.  If zero, marks end of list.
    enum                  BACnetApplServ ApplServ;       // number of application service
} bibb_service;

#define MAX_SERVICES_PER_BIBB	8  // maximum number of services required per BIBB    

// Structure to define each BIBB - msdanner 9/5/04
typedef struct {
	char	*name;	 // BIBB name
   bibb_service  aBIBB_Service[MAX_SERVICES_PER_BIBB];  // array of services required for each BIBB
} bibbdef;


// msdanner 8/31/04: added BIBBs definitions.
// These are used by the parsing routines as well as the BIBB consistency checks.
// The structure of these is similar to the old Functional Group definitions
static bibbdef BIBBs[]={
	      "DS-RP-A",
             { { ssInitiate, asReadProperty,  } 
             },  // (An InitExec value of zero marks the end of each list)
			"DS-RP-B",						
             { { ssExecute, asReadProperty    } 
             }, 
			"DS-RPM-A",						
             { { ssInitiate, asReadPropertyMultiple } 
             }, 
			"DS-RPM-B",						
             { { ssExecute, asReadPropertyMultiple  } 
             }, 
			"DS-RPC-A",						
             { { ssInitiate, asReadPropertyConditional  } 
             }, 
			"DS-RPC-B",						
             { { ssExecute, asReadPropertyConditional  } 
             }, 
			"DS-WP-A",						
             { { ssInitiate, asWriteProperty  } 
             }, 
			"DS-WP-B",						
             { { ssExecute, asWriteProperty  } 
             }, 
			"DS-WPM-A",						
             { { ssInitiate, asWritePropertyMultiple } 
             }, 
			"DS-WPM-B",						
             { { ssExecute, asWritePropertyMultiple  } 
             }, 
			"DS-COV-A",						
             { { ssInitiate, asSubscribeCOV                }, 
               { ssExecute,  asConfirmedCOVNotification    }, 
               { ssExecute,  asUnconfirmedCOVNotification  } 
             }, 
			"DS-COV-B",						
             { { ssExecute,  asSubscribeCOV               }, 
               { ssInitiate, asConfirmedCOVNotification   }, 
               { ssInitiate, asUnconfirmedCOVNotification } 
             }, 
			"DS-COVP-A",					
             { { ssInitiate, asSubscribeCOVProperty       }, 
               { ssExecute,  asConfirmedCOVNotification   }, 
               { ssExecute,  asUnconfirmedCOVNotification } 
             }, 
			"DS-COVP-B",					
             { { ssExecute,  asSubscribeCOVProperty       }, 
               { ssInitiate, asConfirmedCOVNotification   }, 
               { ssInitiate, asUnconfirmedCOVNotification } 
             }, 
			"DS-COVU-A",					
             { { ssExecute,  asUnconfirmedCOVNotification } 
             }, 
			"DS-COVU-B",					
             { { ssInitiate, asUnconfirmedCOVNotification } 
             }, 
			"AE-N-A",						
             { { ssExecute,  asConfirmedEventNotification   }, 
               { ssExecute,  asUnconfirmedEventNotification } 
             }, 
			"AE-N-I-B",						
             { { ssInitiate, asConfirmedEventNotification    }, 
               { ssInitiate, asUnconfirmedEventNotification  } 
             }, 
			"AE-N-E-B",						
             { { ssInitiate, asConfirmedEventNotification    }, 
               { ssInitiate, asUnconfirmedEventNotification  } 
             }, 
			"AE-ACK-A",						
             { { ssInitiate, asAcknowledgeAlarm } 
             }, 
			"AE-ACK-B",						
             { { ssExecute,  asAcknowledgeAlarm } 
             }, 
			"AE-ASUM-A",					
             { { ssInitiate, asGetAlarmSummary  } 
             }, 
			"AE-ASUM-B",					
             { { ssExecute,  asGetAlarmSummary  } 
             }, 
			"AE-ESUM-A",					
             { { ssInitiate, asGetEnrollmentSummary } 
             }, 
			"AE-ESUM-B",					
             { { ssExecute,  asGetEnrollmentSummary } 
             }, 
			"AE-INFO-A",					
             { { ssInitiate, asGetEventInformation } 
             }, 
			"AE-INFO-B",					
             { { ssExecute,  asGetEventInformation } 
             }, 
			"AE-LS-A",						
             { { ssInitiate, asLifeSafetyOperation } 
             }, 
			"AE-LS-B",						
             { { ssExecute,  asLifeSafetyOperation } 
             }, 
			"SCHED-A",   /* no specific services required, other requirements are in the code */	
             { { 0 }
             }, 
			"SCHED-I-B", // no specific services required, other requirements are in the code											
             {	{ ssExecute, asReadProperty },
				{ ssExecute, asWriteProperty },
				// these are an OR requirement and handled later
				//{ ssExecute, asTimeSynchronization },
				//{ ssExecute, asUTCTimeSynchronization }
             },								
			"SCHED-E-B", // no specific services required, other requirements are in the code																
             {	{ ssExecute, asReadProperty },
				{ ssExecute, asWriteProperty },
				{ ssInitiate, asWriteProperty },
				// these are an OR requirement and handled later
				//{ ssExecute, asTimeSynchronization },
				//{ ssExecute, asUTCTimeSynchronization }
             },								
			"T-VMT-A",						
             { { ssInitiate, asReadRange } 
             }, 
			"T-VMT-I-B",					
             { { ssExecute,  asReadRange } 
             }, 
			"T-VMT-E-B",  // no specific services required, other requirements are in the code
             {	{ ssExecute, asReadRange },
				{ ssInitiate, asReadProperty }
             }, 
			"T-ATR-A",	
			    { { ssExecute,  asConfirmedEventNotification },
			      { ssInitiate, asReadRange                  }
			    },  					
			"T-ATR-B",						
			    { { ssInitiate, asConfirmedEventNotification },
			      { ssExecute,  asReadRange                  }
			    },  					
			"DM-DDB-A",						
			    { { ssInitiate, asWho_Is },
			      { ssExecute,  asI_Am   }
			    },  					
			"DM-DDB-B",						
			    { { ssExecute,  asWho_Is },
			      { ssInitiate, asI_Am   }
			    },  					
			"DM-DOB-A",						
			    { { ssInitiate, asWho_Has  },
			      { ssExecute,  asI_Have   }
			    },  					
			"DM-DOB-B",						
			    { { ssExecute,  asWho_Has },
			      { ssInitiate, asI_Have  }
			    },  					
			"DM-DCC-A",						
             { { ssInitiate, asDeviceCommunicationControl } 
             }, 
			"DM-DCC-B",						
             { { ssExecute,  asDeviceCommunicationControl } 
             }, 
			"DM-PT-A",					
			    { { ssInitiate, asConfirmedPrivateTransfer     },
			      { ssInitiate, asUnconfirmedPrivateTransfer   }
			    },  					
			"DM-PT-B",						
			    { { ssExecute, asConfirmedPrivateTransfer     },
			      { ssExecute, asUnconfirmedPrivateTransfer   }
			    },  					
			"DM-TM-A",						
			    { { ssInitiate, asConfirmedTextMessage      },
			      { ssInitiate, asUnconfirmedTextMessage    }
			    },  					
			"DM-TM-B",						
			    { { ssExecute, asConfirmedTextMessage       },
			      { ssExecute, asUnconfirmedTextMessage     }
			    },  					
			"DM-TS-A",						
             { { ssInitiate, asTimeSynchronization } 
             }, 
			"DM-TS-B",						
             { { ssExecute,  asTimeSynchronization } 
             }, 
			"DM-UTC-A",						
             { { ssInitiate, asUTCTimeSynchronization } 
             }, 
			"DM-UTC-B",						
             { { ssExecute,  asUTCTimeSynchronization } 
             }, 
			"DM-RD-A",						
             { { ssInitiate, asReinitializeDevice } 
             }, 
			"DM-RD-B",						
             { { ssExecute,  asReinitializeDevice } 
             }, 
			"DM-BR-A",						
			    { { ssInitiate, asAtomicReadFile      },
			      { ssInitiate, asAtomicWriteFile     },
			      { ssInitiate, asCreateObject        },
			      { ssInitiate, asReinitializeDevice  }
			    },  					
			"DM-BR-B",						
			    { { ssExecute,  asAtomicReadFile      },
			      { ssExecute,  asAtomicWriteFile     },
			      { ssExecute,  asReinitializeDevice  }
			    },  					
			"DM-R-A",						
             { { ssExecute,  asUnconfirmedCOVNotification } 
             }, 
			"DM-R-B",						
             { { ssInitiate, asUnconfirmedCOVNotification } 
             }, 
			"DM-LM-A",						
			    { { ssInitiate, asAddListElement       },
			      { ssInitiate, asRemoveListElement    }
			    },  					
			"DM-LM-B",						
			    { { ssExecute,  asAddListElement       },
			      { ssExecute,  asRemoveListElement    }
			    },  					
			"DM-OCD-A",						
			    { { ssInitiate, asCreateObject       },
			      { ssInitiate, asDeleteObject       }
			    },  					
			"DM-OCD-B",						
			    { { ssExecute,  asCreateObject       },
			      { ssExecute,  asDeleteObject       }
			    },  					
			"DM-VT-A",						
			    { { ssInitiate, asVT_Open             },
			      { ssInitiate, asVT_Close            },
			      { ssInitiate, asVT_Data             },
			      { ssExecute,  asVT_Close            },
			      { ssExecute,  asVT_Data             }
			    },  					
			"DM-VT-B",						
			    { { ssExecute,  asVT_Open             },
			      { ssInitiate, asVT_Close            },
			      { ssInitiate, asVT_Data             },
			      { ssExecute,  asVT_Close            },
			      { ssExecute,  asVT_Data             }
			    },  					
			"NM-CE-A",  // network services are not specified in EPICS
             { { 0 }
             },						
			"NM-CE-B",  // network services are not specified in EPICS
             { { 0 }
             },							
			"NM-RC-A",  // network services are not specified in EPICS
             { { 0 }
             },								
			"NM-RC-B",  // network services are not specified in EPICS
             { { 0 }
             },								
			// Added Workstation BIBBs 12/5/2007 LJT
			"DS-V-A",  
             { { ssInitiate, asReadProperty }
             },								
			"DS-AV-A",  
             { { ssInitiate, asReadProperty }
             },								
			"DS-M-A",  
             { { ssInitiate, asWriteProperty }
             },								
			"DS-AM-A",  
             { { ssInitiate, asWriteProperty }
             },								
			"AE-VN-A",  
             {	{ ssExecute, asConfirmedEventNotification },
				{ ssExecute, asUnconfirmedEventNotification }
             },								
			"AE-AVN-A",  
             {	{ ssExecute, asConfirmedEventNotification },
				{ ssExecute, asUnconfirmedEventNotification }
             },								
			"AE-VM-A",  
             {	{ ssInitiate, asReadProperty },
				{ ssInitiate, asWriteProperty }
             },								
			"AE-AVM-A",  
             {	{ ssInitiate, asReadProperty },
				{ ssInitiate, asWriteProperty },
				{ ssInitiate, asCreateObject },
				{ ssInitiate, asDeleteObject }
             },								
			"AE-AS-A",  
             {	{ ssInitiate, asGetEventInformation },
				{ ssInitiate, asGetAlarmSummary }
             },								
			"SCHED-AVM-A",  
             {	{ ssInitiate, asReadProperty },
				{ ssInitiate, asWriteProperty },
				{ ssInitiate, asCreateObject },
				{ ssInitiate, asDeleteObject }
             },								
			"SCHED-VM-A",  
             {	{ ssInitiate, asReadProperty },
				{ ssInitiate, asWriteProperty }
             },								
			"SCHED-WS-A",  
             {	{ ssInitiate, asReadProperty },
				{ ssInitiate, asWriteProperty }
             },								
			"SCHED-WS-I-B",  
             {	{ ssExecute, asReadProperty },
				{ ssExecute, asWriteProperty },
				{ ssExecute, asTimeSynchronization },
				{ ssExecute, asUTCTimeSynchronization }
             },		
			"SCHED-R-B",
			 {	{ ssExecute, asReadProperty },
				{ ssExecute, asTimeSynchronization },
				{ ssExecute, asUTCTimeSynchronization }
			 },
			"T-AVM-A",  
             {	{ ssInitiate, asReadProperty },
				{ ssInitiate, asWriteProperty },
				{ ssInitiate, asCreateObject },
				{ ssInitiate, asDeleteObject },
				{ ssInitiate, asReadRange }
             },								
			"T-VM-I-B",					
             { { ssExecute,  asReadRange } 
             }, 
			"T-VM-E-B",  
             {	{ ssExecute, asReadRange },
				{ ssInitiate, asReadProperty }
             }, 
			"T-V-A",  
             {	{ ssInitiate, asReadRange }
             }, 
			"T-A-A",  
             {	{ ssInitiate, asReadRange },
				{ ssExecute, asConfirmedEventNotification }
             }, 
			"DM-ANM-A",  
			 { { ssInitiate, asWho_Is },
			   { ssExecute,  asI_Am   },
				{ ssExecute,  asWho_Is },
			   { ssInitiate, asI_Am   }
			 },  					
			"DM-ADM-A",  
             {	{ ssInitiate, asReadProperty }
             }, 
			"DM-ATS-A",  
             {	{ ssInitiate, asTimeSynchronization },
				{ ssInitiate, asUTCTimeSynchronization }
             }, 
			"DM-MTS-A",  
             {	{ ssInitiate, asTimeSynchronization },
				{ ssInitiate, asUTCTimeSynchronization }
             }, 
			// These names have been deprecated and could be removed
			"SCH-AVM-A",  
             {	{ ssInitiate, asReadProperty },
				{ ssInitiate, asWriteProperty },
				{ ssInitiate, asCreateObject },
				{ ssInitiate, asDeleteObject }
             },								
			"SCH-VM-A",  
             {	{ ssInitiate, asReadProperty },
				{ ssInitiate, asWriteProperty }
             },								
			"SCH-WS-A",  
             {	{ ssInitiate, asReadProperty },
				{ ssInitiate, asWriteProperty }
             },								
			"SCH-WS-I-B",  
             {	{ ssExecute, asReadProperty },
				{ ssExecute, asWriteProperty },
				{ ssExecute, asTimeSynchronization },
				{ ssExecute, asUTCTimeSynchronization }
             },		
			"SCH-R-B",
			 {	{ ssExecute, asReadProperty },
				{ ssExecute, asTimeSynchronization },
				{ ssExecute, asUTCTimeSynchronization }
			 },
			"SCH-I-B", // no specific services required, other requirements are in the code											
             {	{ ssExecute, asReadProperty },
				{ ssExecute, asWriteProperty },
				{ ssExecute, asTimeSynchronization },
				{ ssExecute, asUTCTimeSynchronization }
             },								
			"SCH-E-B", // no specific services required, other requirements are in the code																
             {	{ ssExecute, asReadProperty },
				{ ssExecute, asWriteProperty },
				{ ssInitiate, asWriteProperty },
				{ ssExecute, asTimeSynchronization },
				{ ssExecute, asUTCTimeSynchronization }
             },								
			};  

// TODO: Duplicate of NetworkSniffer::BACnetServicesSupported
// The order and position of elements in this array is important!
// It must correspond with the bit positions defined by BACnetServicesSupported
// in section 21 of the BACnet standard.  
static char *StandardServices[]={
			"AcknowledgeAlarm",                       //0
			"ConfirmedCOVNotification",               //1
			"ConfirmedEventNotification",             //2
			"GetAlarmSummary",                        //3
			"GetEnrollmentSummary",                   //4
			"SubscribeCOV",                           //5
			"AtomicReadFile",                         //6
			"AtomicWriteFile",                        //7
			"AddListElement",                         //8
			"RemoveListElement",                      //9
			"CreateObject",                           //10
			"DeleteObject",                           //11
			"ReadProperty",                           //12
			"ReadPropertyConditional",                //13
			"ReadPropertyMultiple",                   //14
			"WriteProperty",                          //15
			"WritePropertyMultiple",                  //16
			"DeviceCommunicationControl",             //17
			"ConfirmedPrivateTransfer",               //18
			"ConfirmedTextMessage",                   //19
			"ReinitializeDevice",                     //20
			"VT-Open",                                //21
			"VT-Close",                               //22
			"VT-Data",                                //23
			"Authenticate",                           //24
			"RequestKey",                             //25
			"I-Am",                                   //26   madanner 6/03: "I-AM"
			"I-Have",                                 //27
			"UnconfirmedCOVNotification",             //28   msdanner 9/04: was "UnConfirmed..."
			"UnconfirmedEventNotification",           //29   msdanner 9/04: was "UnConfirmed..."
			"UnconfirmedPrivateTransfer",             //30   msdanner 9/04: was "UnConfirmed..."
			"UnconfirmedTextMessage",                 //31   msdanner 9/04: was "UnConfirmed..."
			"TimeSynchronization",                    //32
			"Who-Has",                                //33
			"Who-Is",                                 //34
			"ReadRange",							  //35   madanner 6/03: "Read-Range"
			"UTCTimeSynchronization",				  //36   madanner 6/03: "UTC-Time-Synchronization"
			"LifeSafetyOperation",		              //37
			"SubscribeCOVProperty",		              //38
			"GetEventInformation"                     //39
			};

// TODO: Near duplicate of NetworkSniffer::BACnetObjectType, except that table
// is upper case with hyphens instead of spaces.
//
// The order and position of elements in this array is important!
// It must correspond with the definition of BACnetObjectType in section 21 
// of the BACnet standard.  
static char *StandardObjects[]={
			"Analog Input",                //0
			"Analog Output",               //1 
			"Analog Value",                //2  
			"Binary Input",                //3    
			"Binary Output",               //4   
			"Binary Value",                //5
			"Calendar",                    //6
			"Command",                     //7
			"Device",                      //8
			"Event Enrollment",            //9
			"File",                        //10
			"Group",                       //11
			"Loop",                        //12
			"Multi-state Input",           //13
			"Multi-state Output",          //14
			"Notification Class",          //15
			"Program",                     //16
			"Schedule",                    //17
			"Averaging",                   //18
			"Multi-state Value",           //19
			"Trend Log",                   //20  madanner 6/03: "Trend-Log"
			"Life Safety Point",           //21  msdanner 9/04: added
			"Life Safety Zone",            //22  msdanner 9/04: added
			"Accumulator",                 //23  Shiyuan Xiao
			"Pulse Converter",              //24  Shiyuan Xiao
			"Event Log",
			"Global Group",
			"Trend Log Multiple",
			"Load Control",
			"Structured View",
			"Access Door",
			"Lighting Output",
			"Access Credential",			/* Addendum 2008-j */
			"Access Point",
			"Access Rights",
			"Access User",
			"Access Zone",
			"Credential Data Input", 		
			"Network Security",				/* addendum 2008-g */
			"BitString Value",				/* addendum 2008-w */
			"CharacterString Value",
			"Date Pattern Value",
			"Date Value",
			"DateTime Pattern Value",
			"DateTime Value",
			"Integer Value",
			"Large Analog Value",
			"OctetString Value",
			"Positive Integer Value",
			"Time Pattern Value",
			"Time Value"					// 50
		};


// The order of these is important.  New ones should be added to the end, or
// at least inserted after the last "Point-To-Point" entry.
// Maximum is specified by MAX_DATALINK_OPTIONS in vts.h
static char *StandardDataLinks[]={
			"ISO 8802-3, 10BASE5",                          //0
			"ISO 8802-3, 10BASE2",                          //1
			"ISO 8802-3, 10BASET",                          //2
			"ISO 8802-3, fiber",                            //3
			"ARCNET, coax star",                            //4
			"ARCNET, coax bus",                             //5
			"ARCNET, twisted pair star",                    //6
			"ARCNET, twisted pair bus",                     //7
			"ARCNET, fiber star",                           //8
			"MS/TP master. Baud rate(s)",                   //9
			"MS/TP slave. Baud rate(s)",                    //10
			"Point-To-Point. EIA 232, Baud rate(s)",        //11  msdanner 9/2004, was "EIA232" (no space)
			"Point-To-Point. Modem, Baud rate(s)",          //12
			"Point-To-Point. Modem, Autobaud range",        //13
			"LonTalk",                                      //14
			"BACnet/IP, 'DIX' Ethernet",                    //15
			"BACnet/IP, PPP",                               //16
			"Other",                                        //17
			};

// Position is important.  Must preserve index numbers.
static char *SpecialFunctionality[]={
			"Maximum APDU size in octets",                       //0
			"Segmented Requests Supported, window size",         //1
			"Segmented Responses Supported, window size",        //2
			"Router",                                            //3
			"BACnet/IP BBMD"                                     //4
			};

static nameoctet Charsets[]={						//								***006 Begin
			"ANSI X3.4",							csANSI,
			"IBM/Microsoft DBCS",					csDBCS,
			"JIS C 6226",							csJIS,
			"ISO 10646 (UCS-4)",					csUCS4,
			"ISO 10646 (UCS-2)",					csUCS2,
			"ISO 8859-1",                           cs8859 
			};										//								***006 End
static char *FailTimes[]={						//								***019 Begin
			"Notification Fail Time",	
			"Internal Processing Fail Time",		
			"Minimum ON/OFF Time",						
			"Schedule Evaluation Fail Time",		
			"External Command Fail Time",				
			"Program Object State Change Fail Time",		
			"Acknowledgement Fail Time",				
			};										//								***019 End

// 5/9/05 Shiyuan Xiao. Support 135.1-2003
static char *MonthNames[] = {
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December",
	"Odd",
	"Even"
};

static char *DOWNames[]={"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"};

// conformance classes: -------------------------------------------------------------------------------------


#define nCC1 1
TApplServ gCC1_Table[nCC1]= // Conformance Class 1
        { 
             { asReadProperty, ssExecute, DEVICE, -1 }, 
        };  

#define nCC2 1
TApplServ gCC2_Table[nCC2]= // Conformance Class 2
        {
             { asWriteProperty, ssExecute, -1, -1 }
        };

#define nCC3 6
TApplServ gCC3_Table[nCC3]= // Conformance Class 3
        { 
             { asI_Am,                  ssInitiate, -1, -1 },
             { asI_Have,                ssInitiate, -1, -1 },
             { asReadPropertyMultiple,  ssExecute,  -1, -1 },
             { asWritePropertyMultiple, ssExecute,  -1, -1 },
             { asWho_Has,               ssExecute,  -1, -1 },
             { asWho_Is,                ssExecute,  -1, -1 }
        };

#define nCC4 6
TApplServ gCC4_Table[nCC4]= // Conformance Class 4
        { 
             { asAddListElement,        ssInitiate|ssExecute, -1, -1 },
             { asRemoveListElement,     ssInitiate|ssExecute, -1, -1 },
             { asReadProperty,          ssInitiate,           -1, -1 },
             { asReadPropertyMultiple,  ssInitiate,           -1, -1 },
             { asWriteProperty,         ssInitiate,           -1, -1 },
             { asWritePropertyMultiple, ssInitiate,           -1, -1 }
        };

#define nCC5 5
TApplServ gCC5_Table[nCC5]= // Conformance Class 5
        { 
             { asCreateObject,            ssExecute,  -1, -1 },
             { asDeleteObject,            ssExecute,  -1, -1 },
             { asReadPropertyConditional, ssExecute,  -1, -1 },
             { asWho_Has,                 ssInitiate, -1, -1 },
             { asWho_Is,                  ssInitiate, -1, -1 }
        };


// functional groups: ----------------------------------------------------------------------------------------

#define nFgClock 4
TApplServ gFgClock[nFgClock]=
   {
      { asTimeSynchronization, ssExecute, DEVICE, LOCAL_TIME },
      { asTimeSynchronization, ssExecute, DEVICE, LOCAL_DATE },
      { asTimeSynchronization, ssExecute, DEVICE, UTC_OFFSET },
      { asTimeSynchronization, ssExecute, DEVICE, DAYLIGHT_SAVINGS_STATUS }
   };

#define nFgHHWS 6
TApplServ gFgHHWS[nFgHHWS]=
        { 
             { asWho_Is,              ssInitiate|ssExecute, -1, -1 },
             { asI_Am,                ssInitiate,           -1, -1 },
             { asReadProperty,        ssInitiate,           -1, -1 },
             { asWriteProperty,       ssInitiate,           -1, -1 },
             { asReinitializeDevice,  ssInitiate,           -1, -1 },
             { asTimeSynchronization, ssInitiate,           -1, -1 }
        };
        
#define nFgPCWS 15
TApplServ gFgPCWS[nFgPCWS]=
        {
             { asWho_Is,                  ssInitiate|ssExecute, -1, -1 },
             { asI_Am,                    ssInitiate,           -1, -1 },
             { asReadProperty,            ssInitiate,           -1, -1 },
             { asReadPropertyConditional, ssInitiate,           -1, -1 },
             { asReadPropertyMultiple,    ssInitiate,           -1, -1 },
             { asWriteProperty,           ssInitiate,           -1, -1 },
             { asWritePropertyMultiple,   ssInitiate,           -1, -1 },
             { asReinitializeDevice,      ssInitiate,           -1, -1 },
             { asTimeSynchronization,     ssInitiate,           -1, -1 },
             { asCreateObject,            ssInitiate,           -1, -1 },
             { asDeleteObject,            ssInitiate,           -1, -1 },   
             { asAddListElement,          ssInitiate,           -1, -1 },
             { asRemoveListElement,       ssInitiate,           -1, -1 },
             { asAtomicReadFile,          ssInitiate,           -1, -1 },
             { asAtomicWriteFile,         ssInitiate,           -1, -1 }
        };

#define nFgEventInit 4
TApplServ gFgEventInit[nFgEventInit]=
        { 
             { asAcknowledgeAlarm,              ssExecute,  -1, -1 },
             { asConfirmedEventNotification,    ssInitiate, -1, -1 },
             { asGetAlarmSummary,               ssExecute,  -1, -1 },
             { asUnconfirmedEventNotification,  ssInitiate, -1, -1 }
        };

#define nFgEventResponse 2
TApplServ gFgEventResponse[nFgEventResponse]=
        { 
             { asAcknowledgeAlarm,              ssInitiate, -1, -1 },
             { asConfirmedEventNotification,    ssExecute,  -1, -1 },
        };        


#define nFgCOVInit 2
TApplServ gFgCOVInit[nFgCOVInit]=        
        {
             { asSubscribeCOV,             ssExecute,  -1, -1 },
             { asConfirmedCOVNotification, ssInitiate, -1, -1 } 
        };

        
#define nFgCOVResponse 2
TApplServ gFgCOVResponse[nFgCOVResponse]=        
        {
             { asSubscribeCOV,             ssInitiate, -1, -1 },
             { asConfirmedCOVNotification, ssExecute,  -1, -1 } 
        };
        

#define nFgFiles 3
TApplServ gFgFiles[nFgFiles]=
        { 
             { asAtomicReadFile,  ssExecute, -1, -1 },
             { asAtomicWriteFile, ssExecute, -1, -1 }// ,
//             { asNN, -1, FILE_O, -1 }
        };


#define nFgReinitialize 1
TApplServ gFgReinitialize[nFgReinitialize]=
        {
             { asReinitializeDevice,      ssExecute,        -1, -1 },
        };

#define nFgVO 3
TApplServ gFgVO[nFgVO]=
        { 
             { asVT_Open,  ssInitiate,            -1, -1 },
             { asVT_Close, ssInitiate|ssExecute,  -1, -1 },
             { asVT_Data,  ssInitiate|ssExecute,  -1, -1 }
             
        };        

#define nFgVT 3
TApplServ gFgVT[nFgVT]=
        { 
             { asVT_Open,             ssExecute,  -1, -1 },
             { asVT_Close, ssInitiate|ssExecute,  -1, -1 },
             { asVT_Data,  ssInitiate|ssExecute,  -1, -1 }
        };        

#define nFgDevCom 1
TApplServ gFgDevCom[nFgDevCom]=
        { 
             { asDeviceCommunicationControl, ssExecute, -1, -1 }
        };        
        
#define nFgTimeMaster 5
TApplServ gFgTimeMaster[nFgTimeMaster]=
  { 
      { asTimeSynchronization, ssExecute, DEVICE, LOCAL_TIME },
      { asTimeSynchronization, ssExecute, DEVICE, LOCAL_DATE },
      { asTimeSynchronization, ssExecute, DEVICE, UTC_OFFSET },
      { asTimeSynchronization, ssExecute, DEVICE, DAYLIGHT_SAVINGS_STATUS },
      { asTimeSynchronization, ssExecute, DEVICE, TIME_SYNCHRONIZATION_RECIPIENTS } 
  };        

//#define MDEBUG
static void print_debug(char *fmt, ...)
{
	#ifdef MDEBUG	// comment out for release versions
	FILE *pFile;
	va_list ap;
	va_start(ap,fmt);

	if (fmt)
	{
		// puts it in the same directory as the EPICS it is parsing
		pFile = fopen("vtsepics.txt","a");
		if (pFile)
		{
			vfprintf(pFile,fmt,ap);
			fclose(pFile);
		}
	}
	#else
	(void)fmt;
	#endif   
}

//======================
// Setup global error file:
static FILE * pfileError = NULL;  


///////////////////////////////////////////////////////////////////////				***007 Begin
//	Get Enumeration string for a given choice of BACnetPropertyStates
//in:	pschoice	the choice of which BACnetPropertyStates enumeration
//		psenum		the enumeration value
//		pbuf		a string buffer to put the description in (at least 32 long)
//out:	>0			the number of standard enumerations
//		=0			the choice or enumeration are invalid

word  APIENTRY VTSAPIgetpropertystate(word pschoice,word psenum, char *pbuf)
{	etable *pet;

	if (pschoice>=(sizeof(PropertyStates)/sizeof(PropertyStates[0])))
		return 0;
	pet=PropertyStates[pschoice];
	if (psenum>=pet->nes) return 0;						//enumeration is too big
	if (pet->estrings[psenum])
		strcpy(pbuf,pet->estrings[psenum]);
	else
		*pbuf=0;										//just return empty string
	return pet->nes;
}														//							***007 End

///////////////////////////////////////////////////////////////////////				***004 Begin
//	Get Enumeration Table for a given choice of BACnetPropertyStates
//in:	pschoice	the choice of which BACnetPropertyStates enumeration
//		plist		the window handle for a combo box to be filled with enumerations
//out:	>0			the number of standard enumerations
//		=0			the choice is invalid

word  APIENTRY VTSAPIgetpropertystates(word pschoice,HWND plist)
{	etable *pet;
	word i;

	print_debug("GPS: Enter gps pschoice = %d hwnd = %d\n",pschoice,plist);
	if (pschoice>=(sizeof(PropertyStates)/sizeof(PropertyStates[0])))
		return 0;
	pet=PropertyStates[pschoice];
	print_debug("GPS: about to ClearCBList\n");
	ClearCBList(plist);
	for (i=0;i<pet->nes;i++)
		if (pet->estrings[i])
			AddCBListText(plist,pet->estrings[i]);
		else
			AddCBListText(plist,"");
	print_debug("GPS: return value %d\n",pet->nes);

	return pet->nes;
}														//							***004 End

///////////////////////////////////////////////////////////////////////
//	Get Property Table information based on standard object type
//in:	objtype		the desired object type (using the standard enumeration)
//		propindex	the property table index, or 0xFFFF meaning get num properties
//		pname		points to a buffer to contain the property name
//out:	if objtype is invalid, or propindex is invalid then 0xFFFFFFFF
//		if propindex was 0xFFFF then return the number of properties for that object type
//		else		return the property ID for that slot
//					in this case, also return the property name in the buffer pname

dword APIENTRY VTSAPIgetpropinfo(word objtype,word propindex,
											char *pname,word *ptype,
											word *pgroup,word *pflags,
											word *pet)
{	propdescriptor	*pt;
	word			np;

	//if (objtype>=etObjectTypes.nes) return 0xFFFFFFFF;	//not a valid object type
	//pt=StdObjects[objtype].sotProps;					//point to table of properties for this guy

	if (objtype <= etObjectTypes.nes)  //not a standard object type
		pt = StdObjects[objtype].sotProps;
	else
		pt = ProprietaryObjProps;	   //point to table of properties for this guy

	np=1;												//always at least one property
	while((pt->PropGroup&Last)==0) {np++;pt++;}			//count num props
	if (propindex==0xFFFF) return (dword)np;			//just say how many properties there are
	if (propindex>=np) return 0xFFFFFFFF;				//invalid property index

	//pt=StdObjects[objtype].sotProps;					//point to table of properties for this guy
	
	if (objtype <= etObjectTypes.nes)  //not a standard object type
		pt = StdObjects[objtype].sotProps;
	else
		pt = ProprietaryObjProps;	   //point to table of properties for this guy

	strcpy(pname,pt[propindex].PropertyName);			//return the name
	if (ptype) *ptype=(word)pt[propindex].ParseType;
	if (pgroup) *pgroup=(word)pt[propindex].PropGroup;
	if (pflags) *pflags=pt[propindex].PropFlags;
	if (pet) *pet=pt[propindex].PropET;
	return pt[propindex].PropID;						//and property ID
}

///////////////////////////////////////////////////////////////////////
//	Get Enumeration Table information
//in:	etindex		index to the enumeration table
//		index		the table index (use 0 to find out how many there are)
//		propstart	if not null, points to word var to receive value of where proprietary enumerations begin (0=none)
//		propmax		if not null, points to dword var to receive max value for proprietary enumerations+1
//		ptext		points to a buffer to contain the enumeration text
//out:	>0			the number of standard enumerations
//		=0			the index is invalid

word APIENTRY VTSAPIgetenumtable(word etindex,word index,word *propstart,dword *propmax,char *ptext)
{	etable *pet;

	if (etindex==0||etindex>(sizeof(AllETs)/sizeof(AllETs[0])))
		return 0;
	pet=AllETs[etindex];
	if (index>=pet->nes) return 0;						//invalid index
	if (propstart) *propstart=pet->propes;				//return start of proprietary enumerations
	if (propmax) *propmax=pet->propmaxes;				//return max proprietary enumerations
	if (pet->estrings[index])
		strcpy(ptext,pet->estrings[index]);			//return the name
	else
		*ptext=0;
	return pet->nes;
}

///////////////////////////////////////////////////////////////////////				***005 Begin
//	Get Default Property information based on property id
//
//note:	Only Matches the FIRST propid found!
//in:	objtype		the object type (0xFFFF if unknown)
//		propid		the property id 
//		ptype		pointer to a word to be filled with parsetype, or NULL
//		pet			pointer to word to be filled with enum table index, or NULL
//out:	false		if no default info available for property id, 
//		true		if default info was available, parameters filled in accordingly

BOOL APIENTRY VTSAPIgetdefaultpropinfo(word objtype,dword propid,word *ptype,word *pet)
{	word			i;
	propdescriptor	*pt;

	if (objtype<etObjectTypes.nes)						//known object type
	{	pt=StdObjects[objtype].sotProps;				//point to table of properties for this guy
		do
		{	if (pt->PropID==propid)						//found our man
			{   if (ptype) *ptype=(word)pt->ParseType;
				if (pet) *pet=pt->PropET;
				return true;
			}
			if (pt->PropGroup&Last) break;
			pt++;										//advance to next one 
		} while(true);
	}
//unknown object type
	for (i=0;i<dParseTypes.npt;i++)
	{	if (propid==dParseTypes.dpt[i].dptpropid)
		{   if (ptype) *ptype=dParseTypes.dpt[i].dptparsetype;
			if (pet) *pet=dParseTypes.dpt[i].dptpet;
			return true;
		}
	}                                                      
	return false;
}														//							***005 End

///////////////////////////////////////////////////////////////////////				***003 Begin
//	Get Default Parse Type based on property id
//in:	propid		the property id 
//		hWnd		the handle to a list control into which parse type(s) are built
//out:	0 			if invalid property id, 
//					else number added to list
//					       parse type text /t parsetype /t eb index /t pflags eg.
//						   "bits/t03/t18/t00"

word APIENTRY VTSAPIgetdefaultparsetype(dword propid,HWND hWnd)
{	word	i,n=0,pt;
	char	pstr[128],t[32];

	ClearCBList(hWnd);
	for (i=0;i<dParseTypes.npt;i++)
	{	if (propid==dParseTypes.dpt[i].dptpropid)
		{   pt=dParseTypes.dpt[i].dptparsetype;
			if (pt==none) strcpy(pstr,"none");
			else strcpy(pstr,stParseTypes[pt]);	//"parse type"
			strcat(pstr,"\t"); 							//tab
			_itoa(pt,t,10);
			strcat(pstr,t);								//parse type
			strcat(pstr,"\t");							//tab
			_itoa(dParseTypes.dpt[i].dptpet,t,10);
			strcat(pstr,t);								//parse type
			strcat(pstr,"\t");							//tab
			_itoa(dParseTypes.dpt[i].dptflags,t,10);
			strcat(pstr,t);								//parse type
            AddCBListText(hWnd,pstr);
			n++;
			if ((i==dParseTypes.npt-1)||(propid!=dParseTypes.dpt[i+1].dptpropid)) break;
		}                                                      
	}
	if (n!=0) SelectCBListItem(hWnd,0);					//select first item
	return n;
}

///////////////////////////////////////////////////////////////////////
//	Clear all items from a List Box

int ClearCBList(HWND hWnd)
{ return (int) SendMessage(hWnd,CB_RESETCONTENT,0, 0L);
}

///////////////////////////////////////////////////////////////////////
//	Add text to a list item in a Combo Box

int AddCBListText(HWND hWnd,char *p)
{ return (int) SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM) ((LPSTR) p));
}

///////////////////////////////////////////////////////////////////////
//	Select an item in a List Box

int SelectCBListItem(HWND hWnd,int index)
{ return (int) SendMessage(hWnd,CB_SETCURSEL,(WPARAM) index, 0L);
}
//																				***003 End


///////////////////////////////////////////////////////////////////////			***008 Begin
//	Delete an object created by ReadTextPICS
//in:	p		points to the object
void  APIENTRY DeletePICSObject(generic_object *p)
{	word	i;
	void *vp,*vq,*vr;
	char errMsg[300];
    // TCHAR szMessage[300];

	switch(p->object_type)						//release this object's special fields
	{
	case CALENDAR:
		vp=((calendar_obj_type *)p)->date_list;
		while(vp!=NULL)
		{	vq=vp;
			vp=((BACnetCalendarEntry *)vp)->next;
			free(vq);
		}
		break; 
	case COMMAND:
		for (i=0;i<MAX_ACTION_TEXTS;i++)
		{	vp=((command_obj_type *)p)->action[i];
			while(vp!=NULL)
			{	vq=vp;
				vp=((BACnetActionCommand *)vp)->next;
				free(vq);
			}
			if (((command_obj_type *)p)->action_text[i]!=NULL)
				free(((command_obj_type *)p)->action_text[i]);
		}
		break; 
	case DEVICE: 
		vp=((device_obj_type *)p)->object_list;
		while(vp!=NULL)
		{	vq=vp;
			vp=((BACnetObjectIdentifier *)vp)->next;
			free(vq);
		}
		vp=((device_obj_type *)p)->vt_classes_supported;
		while(vp!=NULL)
		{	vq=vp;
			vp=((BACnetVTClassList *)vp)->next;
			free(vq);
		}
		vp=((device_obj_type *)p)->active_vt_sessions;
		while(vp!=NULL)
		{	vq=vp;
			vp=((BACnetVTSession *)vp)->next;
			free(vq);
		}
		vp=((device_obj_type *)p)->list_session_keys;
		while(vp!=NULL)
		{	vq=vp;
			vp=((BACnetSessionKey *)vp)->next;
			free(vq);
		}
		vp=((device_obj_type *)p)->time_synch_recipients;
		while(vp!=NULL)
		{	vq=vp;
			vp=((BACnetRecipient *)vp)->next;
			free(vq);
		}
		vp=((device_obj_type *)p)->device_add_binding;
		while(vp!=NULL)
		{	vq=vp;
			vp=((BACnetAddressBinding *)vp)->next;
			free(vq);
		}
		vp=((device_obj_type *)p)->manual_slave_add_bind;
		while(vp!=NULL)
		{	vq=vp;
			vp=((BACnetAddressBinding *)vp)->next;
			free(vq);
		}
		vp=((device_obj_type *)p)->slave_add_bind;
		while(vp!=NULL)
		{	vq=vp;
			vp=((BACnetAddressBinding *)vp)->next;
			free(vq);
		}
		vp=((device_obj_type *)p)->active_cov_subscriptions;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((BACnetCOVSubscription *)vp)->next;
			free(vq);
		}
		vp=((device_obj_type *)p)->configuration_files;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((BACnetObjectIdentifier *)vp)->next;
			free(vq);
		}
		vp=((device_obj_type *)p)->slave_proxy_enable;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((BooleanList *)vp)->next;
			free(vq);
		}
		vp=((device_obj_type *)p)->auto_slave_disc;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((BooleanList *)vp)->next;
			free(vq);
		}
		vp=((device_obj_type *)p)->structured_object_list;
		while(vp!=NULL)
		{	vq=vp;
			vp=((BACnetObjectIdentifier *)vp)->next;
			free(vq);
		}
		break; 

	case EVENT_ENROLLMENT: 
		vp=((ee_obj_type *)p)->parameter_list.list_bitstring_value;
		while(vp!=NULL)
		{	vq=vp;
			vp=((BACnetListBitstringValue *)vp)->next;
			free(vq);
		}
		vp=((ee_obj_type *)p)->parameter_list.list_of_value;
		while(vp!=NULL)
		{	vq=vp;
			vp=((BACnetPropertyStates *)vp)->next;
			free(vq);
		}
		vp=((ee_obj_type *)p)->event_time_stamps;
		for(i=0; i<3; i++)
		{
			if ( ((ee_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((ee_obj_type *)p)->event_time_stamps[i]);
		}

		break; 
	case FILE_O:
        break;   

	case GROUP:
		vr=((group_obj_type *)p)->list_of_group_members;
		while(vr!=NULL)
		{	vp=((BACnetReadAccessSpecification *)vr)->list_of_prop_ref;
			while(vp!=NULL)
			{	vq=vp;
				vp=((BACnetPropertyReference *)vp)->next;
				free(vq);
			}
			vq=vr;
			vr=((BACnetReadAccessSpecification *)vr)->next;
			free(vq);
		}
		break; 
	case LOOP: 
		vp=((loop_obj_type *)p)->setpoint_ref;
		if (vp!=NULL) free(vp);
		vp=((loop_obj_type *)p)->event_time_stamps;
		for(i=0; i<3; i++)
		{
			if ( ((loop_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((loop_obj_type *)p)->event_time_stamps[i]);
		}

		break; 
	case MULTI_STATE_INPUT: 
		for (i=0;i<MAX_STATE_TEXTS;i++)
		{	if (((mi_obj_type *)p)->state_text[i]!=NULL)
				free(((mi_obj_type *)p)->state_text[i]);
		}
		vp=((mi_obj_type *)p)->event_time_stamps;
		for(i=0; i<3; i++)
		{
			if ( ((mi_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((mi_obj_type *)p)->event_time_stamps[i]);
		}
		vp=((mi_obj_type *)p)->alarm_values;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((UnsignedList *)vp)->next;
			free(vq);
		}
		vp=((mi_obj_type *)p)->fault_values;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((UnsignedList *)vp)->next;
			free(vq);
		}
		break; 
	case MULTI_STATE_OUTPUT: 
		for (i=0;i<MAX_STATE_TEXTS;i++)
		{	if (((mo_obj_type *)p)->state_text[i]!=NULL)
				free(((mo_obj_type *)p)->state_text[i]);
		}
		vp=((mo_obj_type *)p)->event_time_stamps;
		for(i=0; i<3; i++)
		{
			if ( ((mo_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((mo_obj_type *)p)->event_time_stamps[i]);
		}

		break; 
	case PROGRAM:
		break;
	case NOTIFICATIONCLASS: 
		vp=((nc_obj_type *)p)->recipient_list;
		while(vp!=NULL)
		{	vq=vp;
			vp=((BACnetDestination *)vp)->next;
			free(vq);
		}
		break; 
	case SCHEDULE:
		for (i=0;i<7;i++)
		{	vp=((schedule_obj_type *)p)->weekly_schedule[i];
			while(vp!=NULL)
			{	vq=vp;
				vp=((BACnetTimeValue *)vp)->next;
				free(vq);
			}
		}
		vr=((schedule_obj_type *)p)->exception_schedule.special_event;
		while(vr!=NULL)
		{	vp=((BACnetSpecialEvent *)vr)->list_of_time_values;
			while(vp!=NULL)
			{	vq=vp;
				vp=((BACnetTimeValue *)vp)->next;
				free(vq);
			}
			vq=vr;
			vr=((BACnetSpecialEvent *)vr)->next;
			free(vq);
		}
		vp=((schedule_obj_type *)p)->list_obj_prop_ref;
		while(vp!=NULL)
		{	vq=vp;
			vp=((BACnetObjectPropertyReference *)vp)->next;
			free(vq);
		}
		break; 
      case AVERAGING:
        break;   
	  case ANALOG_INPUT:
		vp=((ai_obj_type *)p)->event_time_stamps;
		for(i=0; i<3; i++)
		{
			if ( ((ai_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((ai_obj_type *)p)->event_time_stamps[i]);
		}
		break;
	  case ANALOG_OUTPUT:
		vp=((ao_obj_type *)p)->event_time_stamps;
		for(i=0; i<3; i++)
		{
			if ( ((ao_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((ao_obj_type *)p)->event_time_stamps[i]);
		}
		break;
	  case ANALOG_VALUE:
		vp=((av_obj_type *)p)->event_time_stamps;
		for(i=0; i<3; i++)
		{
			if ( ((av_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((av_obj_type *)p)->event_time_stamps[i]);
		}
		break;
	  case BINARY_INPUT:
		vp=((bi_obj_type *)p)->event_time_stamps;
		for(i=0; i<3; i++)
		{
			if ( ((bi_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((bi_obj_type *)p)->event_time_stamps[i]);
		}
		break;
	  case BINARY_OUTPUT:
		vp=((bo_obj_type *)p)->event_time_stamps;
		for(i=0; i<3; i++)
		{
			if ( ((bo_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((bo_obj_type *)p)->event_time_stamps[i]);
		}
		break;
	  case BINARY_VALUE:
		vp=((bv_obj_type *)p)->event_time_stamps;
		for(i=0; i<3; i++)
		{
			if ( ((bv_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((bv_obj_type *)p)->event_time_stamps[i]);
		}
		break;
	  case ACCUMULATOR:
		vp=((accumulator_obj_type *)p)->event_time_stamps;
		for(i=0; i<3; i++)
		{
			if ( ((accumulator_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((accumulator_obj_type *)p)->event_time_stamps[i]);
		}
		break;
	  case MULTI_STATE_VALUE: // msdanner 9/2004
		for (i=0;i<MAX_STATE_TEXTS;i++)
		{	if (((msv_obj_type *)p)->state_text[i]!=NULL)
				free(((msv_obj_type *)p)->state_text[i]);
		}
		vp=((msv_obj_type *)p)->event_time_stamps;
		for(i=0; i<3; i++)
		{
			if ( ((msv_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((msv_obj_type *)p)->event_time_stamps[i]);
		}
		vp=((msv_obj_type *)p)->alarm_values;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((UnsignedList *)vp)->next;
			free(vq);
		}
		vp=((msv_obj_type *)p)->fault_values;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((UnsignedList *)vp)->next;
			free(vq);
		}
		break; 
      case TREND_LOG:
  		vp=((trend_obj_type *)p)->event_time_stamps;
		for(i=0; i<3; i++)
		{
			if ( ((trend_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((trend_obj_type *)p)->event_time_stamps[i]);
		}
        break;
	  case LIFE_SAFETY_POINT:
		vp=((lifesafetypoint_obj_type *)p)->event_time_stamps;
		for(i=0; i<3; i++)
		{
			if ( ((lifesafetypoint_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((lifesafetypoint_obj_type *)p)->event_time_stamps[i]);
		}
		vp=((lifesafetypoint_obj_type *)p)->member_of;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((BACnetDeviceObjectReference *)vp)->next;
			free(vq);
		}
		vp=((lifesafetypoint_obj_type *)p)->accepted_modes;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((BACnetEnumList *)vp)->next;
			free(vq);
		}
		vp=((lifesafetypoint_obj_type *)p)->life_safety_alarm_values;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((BACnetEnumList *)vp)->next;
			free(vq);
		}
		vp=((lifesafetypoint_obj_type *)p)->alarm_values;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((BACnetEnumList *)vp)->next;
			free(vq);
		}
		vp=((lifesafetypoint_obj_type *)p)->fault_values;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((BACnetEnumList *)vp)->next;
			free(vq);
		}
		  break;
	  case LIFE_SAFETY_ZONE:
		vp=((lifesafetyzone_obj_type *)p)->event_time_stamps;
		for(i=0; i<3; i++)
		{
			if ( ((lifesafetyzone_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((lifesafetypoint_obj_type *)p)->event_time_stamps[i]);
		}
		vp=((lifesafetyzone_obj_type *)p)->member_of;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((BACnetDeviceObjectReference *)vp)->next;
			free(vq);
		}
		vp=((lifesafetyzone_obj_type *)p)->zone_members;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((BACnetDeviceObjectReference *)vp)->next;
			free(vq);
		}
		vp=((lifesafetyzone_obj_type *)p)->accepted_modes;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((BACnetEnumList *)vp)->next;
			free(vq);
		}
		vp=((lifesafetyzone_obj_type *)p)->life_safety_alarm_values;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((BACnetEnumList *)vp)->next;
			free(vq);
		}
		vp=((lifesafetyzone_obj_type *)p)->alarm_values;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((BACnetEnumList *)vp)->next;
			free(vq);
		}
		vp=((lifesafetyzone_obj_type *)p)->fault_values;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((BACnetEnumList *)vp)->next;
			free(vq);
		}

		  break;
	  case PULSE_CONVERTER:
		vp=((pulseconverter_obj_type *)p)->event_time_stamps;
		for(i=0; i<3; i++)
		{
			if ( ((pulseconverter_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((pulseconverter_obj_type *)p)->event_time_stamps[i]);
		}
		break;

	  case STRUCTURED_VIEW:
		vp=((sv_obj_type *)p)->subordinate_list;
		while(vp!=NULL)
		{
			vq=vp;
			vp=((BACnetDeviceObjectReference *)vp)->next;
			free(vq);
		}

		for (i=0; i < MAX_SV_ANNOTATIONS; i++)
		{	
			if (((sv_obj_type *)p)->subordinate_annotations[i]!=NULL)
				free(((sv_obj_type *)p)->subordinate_annotations[i]);
		}
		break;

      case TREND_LOG_MULTIPLE:
		for(i=0; i<3; i++)
		{
			if (((tlm_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((tlm_obj_type *)p)->event_time_stamps[i]);
		}
        break;

      case EVENT_LOG:
		for(i=0; i<3; i++)
		{
			if (((el_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((el_obj_type *)p)->event_time_stamps[i]);
		}
        break;

      case LOAD_CONTROL:
		for(i=0; i<MAX_SHED_LEVELS; i++)
		{
			if (((lc_obj_type *)p)->shed_level_descriptions[i]!=NULL)
				free(((lc_obj_type *)p)->shed_level_descriptions[i]);
		}

		vp = ((lc_obj_type *)p)->shed_levels;
		while (vp != NULL)
		{
			vq = vp;
			vp = ((UnsignedList *)vp)->next;
			free(vq);
		}
		
		for(i=0; i<3; i++)
		{
			if (((lc_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((lc_obj_type *)p)->event_time_stamps[i]);
		}
        break;

	  case ACCESS_DOOR:
		vp = ((ad_obj_type *)p)->alarm_values;
		while (vp != NULL)
		{
			vq = vp;
			vp = ((BACnetEnumList*)vp)->next;
			free(vq);
		}

		vp = ((ad_obj_type *)p)->fault_values;
		while (vp != NULL)
		{
			vq = vp;
			vp = ((BACnetEnumList*)vp)->next;
			free(vq);
		}

		vp = ((ad_obj_type *)p)->door_members;
		while (vp != NULL)
		{
			vq = vp;
			vp = ((BACnetDeviceObjectReference*)vp)->next;
			free(vq);
		}

		vp = ((ad_obj_type *)p)->masked_alarm_values;
		while (vp != NULL)
		{
			vq = vp;
			vp = ((BACnetEnumList*)vp)->next;
			free(vq);
		}
		
		for(i=0; i<3; i++)
		{
			if (((ad_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((ad_obj_type *)p)->event_time_stamps[i]);
		}
		break;

	  case CHARACTERSTRING_VALUE:
		 for(i=0; i<MAX_FAULT_STRINGS; i++)
		 {
			if ( ((charstring_obj_type *)p)->alarm_values[i]!=NULL)
				free(((charstring_obj_type *)p)->alarm_values[i]);
		 }
		 for(i=0; i<MAX_FAULT_STRINGS; i++)
		 {
			if ( ((charstring_obj_type *)p)->fault_values[i]!=NULL)
				free(((charstring_obj_type *)p)->fault_values[i]);
		 }
		 for(i=0; i<3; i++)
		 {
			if ( ((charstring_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((charstring_obj_type *)p)->event_time_stamps[i]);
		 }
		 break; 

	  case INTEGER_VALUE:
		 for(i=0; i<3; i++)
		 {
			if ( ((integer_obj_type *)p)->event_time_stamps[i]!=NULL)
				free(((integer_obj_type *)p)->event_time_stamps[i]);
		 }
		 break;

	  case DATETIME_VALUE:
		  // No allocations to clean up
		  break;
	
	  default:
		  // Someone forgot to implement delete code for this object type
		  sprintf( errMsg, "**WARNING**: No delete code for this object %s (type = %d), will probably leak \n", 
					p->object_name, p->object_type ); 
		  // tperror( errMsg, true);
		  // PrintToFile( errMsg );
		  OutputDebugString( _T( errMsg ) );
		  // assert(false);		// upside the head for developers
		  // Attempt the deletion anyway
		  break;
	}
	free(p);									//and the object itself					***008 End
}

///////////////////////////////////////////////////////////////////////
//	Open and Read a TPI file (i.e. EPICS text file)
///////////////////////////////////////////////////////////////////////
bool APIENTRY ReadTextPICS(
        char *tp, // filename string
        PICSdb *pd, // EPICS database
        int *errc, // error counter
        int *errPICS) // returns the error
{	
	int			i,j;
	generic_object *pd2;  // line added by MAG for debug only
	lPICSErr=-1;
	
	//madanner 6/03: wasn't ini tializing cancel
	cancel = false;
	::DeleteFile( FILE_CHECK_EPICS_CONS );		//madanner 4/4
	pfileError = fopen( FILE_CHECK_EPICS_CONS,"a+");

// looks to be a duplicate of the below therefore did not enable this line.  LJT 8/31/2005	
//	memset(pd->BACnetFailTimes,ftNotSupported,sizeof(pd->BACnetFailTimes));	//default is not supported // added by Kare Sars

	pd->Database=NULL;
	memset(pd->BACnetStandardObjects,soNotSupported,sizeof(pd->BACnetStandardObjects));	     //added by xlp,2002-11
	memset(pd->BACnetStandardServices,ssNotSupported,sizeof(pd->BACnetStandardServices));	//added by xlp,2002-11
	// initialize to no BIBBs supported
	memset(pd->BIBBSupported,0,sizeof(pd->BIBBSupported));	//default is not supported
   memset(pd->BACnetFailTimes,ftNotSupported,sizeof(pd->BACnetFailTimes));	//default is not supported
	pd->BACnetFunctionalGroups=0;				//default is none
    // default is no data links supported
	memset(pd->DataLinkLayerOptions, 0, sizeof(pd->DataLinkLayerOptions)); //default is none

	// Initialize Special Functionality items, in case this section is omitted in EPICS
	pd->RouterFunctions=rfNotSupported;			//default is none
	pd->SegmentedRequestWindow=pd->SegmentedResponseWindow=0;
	pd->MaxAPDUSize=50;
    pd->BBMD=0;  // default is no BBMD support

	print_debug("RTP: open file '%.275s'\n",tp);
	lerrc=0;									//no errors yet
	if ((ifile=fopen(tp,"r"))==NULL)			//open input file						***008
	{	tperror(_strerror(tp),false);
		goto rtpexit;
	}
	
	lc=0;
	readline(lb,8);								//read header line from the file		***008
	if (strnicmp(lb,picshdr,6))				//invalid signature
	{	tperror("This file does not contain a supported form of Text PICS!",false);
		goto rtpclose;
	}
	print_debug("RTP: Read line '%s'\n",lb);		// MAG

	readline(lb,sizeof(lb));					//read a line from the file				***008
	lp=&lb[0];
	print_debug("RTP: rl1: Read line '%s'\n",lb);		// MAG

	if (stricmp(lb,BeginPics))				//invalid signature
	{	tperror("Invalid Text PICS signature.",false);
		goto rtpclose;
	}

	while (feof(ifile)==0&&!cancel)				//										***008
	{	readline(lb,sizeof(lb));				//read a line from the file 			***008
		print_debug("RTP: rl2: Read line '%s'\n",lb);		// MAG

		if (lb[0])								//not a blank line
		{	if (stricmp(lb,EndPics)==0)		//found the end
				break;							//we're done
			if ((lp=strchr(lb,':'))==NULL)		//must have a section name
			{	lp=&lb[0];
	         	if (tperror("Expected section name here...",true))
					goto rtpclose;
			}
			else
			{	*lp++=0;						//make asciz section name and lp points to args
				for (i=0;i<(sizeof(SectionNames)/sizeof(SectionNames[0]));i++) {
				  // preprocessing, replace score and underscore with whitespace
				  preprocstr(lb);				// remove score and underscore		***020
				  if (stricmp(lb,SectionNames[i])==0)	//we found a matching section name
				  {	
					switch(i)
					{
					int nameLen;
					case 0:
						// Allocate as much space as we need (actually, more than enough due to quoting)
						nameLen = strlen(lp) + 1;
						pd->VendorName = new char[nameLen];
						if (setstring(pd->VendorName,nameLen,lp)) goto rtpclose;
						break;
					case 1:
						nameLen = strlen(lp) + 1;
						pd->ProductName = new char[nameLen];
						if (setstring(pd->ProductName,nameLen,lp)) goto rtpclose;
						break;
					case 2:
						nameLen = strlen(lp) + 1;
						pd->ProductModelNumber = new char[nameLen];
						if(setstring(pd->ProductModelNumber,nameLen,lp)) goto rtpclose;
						break;
					case 3:
						nameLen = strlen(lp) + 1;
						pd->ProductDescription = new char[nameLen];
						if (setstring(pd->ProductDescription,nameLen,lp)) goto rtpclose;
						break;
					case 4:
						skipwhitespace();							//			***008
						j=atoi(lp);				//get BACnet Conformance Class Supported
						if (j<1||j>6)
						{	if(tperror("Invalid conformance class",true)) goto rtpclose;}
						else
							pd->BACnetConformanceClass=j;
						break;
					case 5:
						if (ReadFunctionalGroups(pd)) goto rtpclose;	//			***008
						break;
					case 6:
						if (ReadStandardServices(pd)) goto rtpclose;	//			***008
						break;
					case 7:
						if (ReadStandardObjects(pd)) goto rtpclose;		//			***008
						break;
					case 8:
						if (ReadDataLinkOptions(pd)) goto rtpclose;		//			***008
						break;
					case 9:
						if (ReadSpecialFunctionality(pd)) goto rtpclose;	//		***008
						break;
					case 10:
						if (ReadObjects(pd)) goto rtpclose;	//						***008
						break;					//									***006 Begin
					case 11:
						if (ReadCharsets(pd)) goto rtpclose;	//					***006 End
						break;
					case 12:
						if (ReadFailTimes(pd)) goto rtpclose;	//					***019
						break;
					case 13: // BIBBs supported
						if (ReadBIBBSupported(pd)) goto rtpclose;
						break;
					//case 14: // TODO:  Default Property Value Restrictions
						//if (ReadDefaultPropertyValueRestrictions(pd)) goto rtpclose;
					//	break;
					}
					i=0;						//remember that we found one
					break;
				  };
			}																		
			if (i)							//couldn't find this one
			{	lp[-1]=':';
				if (tperror("Unknown section name",true))
					goto rtpclose;
			}
			}
		}
	};
rtpclose:
	fclose(ifile);							 //									***008
//////////////////////////////////////////////////////////////
// add EPICS consistency Check ,xlp,2002-11

	// madanner 6/03:  skip consistency tests if user canceled parse due to problems
	if ( !cancel )
	{
//		::DeleteFile("c:\\EPICSConsChk.txt");
		// msdanner 9/04: Old consistency checks based on conformance class are no longer applicable
		//CheckPICSObjCons(pd);                  
		//CheckPICSServCons(pd);               
		//CheckPICSPropCons(pd);      
      // New 135.1-2003 checks
      if (lerrc == 0)  // if no syntax errors, proceed with EPICS consistency tests
      {
		   lPICSErr=0;  // enables counting of consistency errors
		   CheckPICSConsistency2003(pd);  // msdanner 9/04: EPICS consistency checks specified by 135.1-2003
		   *errPICS=lPICSErr;	// return count of consistency errors
      }
   }

    //clear the global variables
	ProtocolServSup.PropSupValue=NULL;
	ProtocolServSup.ObjServNum=0;
	ProtocolObjSup.PropSupValue=NULL;
	ProtocolObjSup.ObjServNum=0;
	memset(ObjInTestDB,0,sizeof(ObjInTestDB));
	memset(DevObjList,0,sizeof(DevObjList));
//////////////////////////////////////////////////////////////
rtpexit:
	*errc=lerrc;

	pd2 = pd->Database;
	print_debug("Begin database printout.\n");
	while(pd2 != NULL){
		print_debug("object id %d (%d) object name = '%s' desc = '%s'\n",pd2->object_id,(pd2->object_id)>>22,pd2->object_name,pd2->description);
		pd2 = (generic_object *)pd2->next;
	}
	print_debug("End database printout.\n");

	if ( pfileError != NULL )		// Close main error file
	{
		fflush( pfileError );
		fclose(pfileError);
	}
	pfileError = NULL;

	return !cancel;
}

///////////////////////////////////////////////////////////////////////
//	Read Functional Groups section of a TPI file
//in:	ifile		file stream
//		pd			database structure to be filled in from PICS
//out:	true		if cancel selected

BOOL ReadFunctionalGroups(PICSdb *pd)
{	int			i;

	ReadNext();									//point to next token				***008
	if (lp==NULL||*lp++!='{')					//no open token
		return tperror("Expected { here",true);
		
	while (lp!=NULL)
	{	ReadNext();								//point to next token				***008
		if (*lp=='}'||lp==NULL) break;			//return, we're done with these
 		for (i=0;i<(sizeof(FunctionalGroups)/sizeof(FunctionalGroups[0]));i++)
		  if (stricmp(lp,FunctionalGroups[i].name)==0) //found it
		  {	pd->BACnetFunctionalGroups|=FunctionalGroups[i].dwcons;
			i=0;
			break;
		  }
		if (i)
		{	if (tperror("Unknown Functional Group",true))
				return true;
		}
	}
	return false;		
}

///////////////////////////////////////////////////////////////////////				***006 Begin
//	Read Charactersets section of a TPI file
//in:	ifile		file stream
//		pd			database structure to be filled in from PICS
//out:	true		if cancel selected

BOOL ReadCharsets(PICSdb *pd)
{	int			i;

	ReadNext();									//point to next token				***008
	if (lp==NULL||*lp++!='{')					//no open token
		return tperror("Expected { here",true);
		
	pd->BACnetCharsets=0;						//default is none
	while (lp!=NULL)
	{	ReadNext();								//point to next token				***008
		rtrim(lp);
		if (*lp=='}'||lp==NULL) break;			//return, we're done with these
 		for (i=0;i<(sizeof(Charsets)/sizeof(Charsets[0]));i++)
		  if (stricmp(lp,Charsets[i].name)==0) //found it
		  {	pd->BACnetCharsets|=Charsets[i].octetcons;
			i=0;
			break;
		  }
		if (i)
		{	if (tperror("Unknown Character Set",true))
				return true;
		}
	}
	return false;		
}												//									***006 End

///////////////////////////////////////////////////////////////////////				***006 Begin
//	Read BIBBs Supported section of a TPI file
//in:	ifile		file stream
//		pd			database structure to be filled in from PICS
//out:	true		if cancel selected

BOOL ReadBIBBSupported(PICSdb *pd)
{	int			i;

	ReadNext();									//point to next token				***008
	if (lp==NULL||*lp++!='{')					//no open token
		return tperror("Expected { here",true);

	// initialize to no BIBBs supported
	memset(pd->BIBBSupported,0,sizeof(pd->BIBBSupported));	//default is not supported
	while (lp!=NULL)
	{	ReadNext();								//point to next token				***008
		rtrim(lp);
		if (*lp=='}'||lp==NULL) break;			//return, we're done with these
 		for (i=0;i<(sizeof(BIBBs)/sizeof(BIBBs[0]));i++)
		  if (stricmp(lp,BIBBs[i].name)==0) //found it
		  {	pd->BIBBSupported[i] = 1;    	// Mark this BIBB supported
			i=0;
			break;
		  }
		if (i)
		{	if (tperror("Unknown BIBB",true))
				return true;
		}
	}
	return false;		
}												//									***006 End

///////////////////////////////////////////////////////////////////////
//	Read Standard Services section of a TPI file
//in:	ifile		file stream
//		pd			database structure to be filled in from PICS
//out:	true		if cancel selected

BOOL ReadStandardServices(PICSdb *pd)
{	int			i;
	char		*p;

	ReadNext();									//point to next token				***008
	if (lp==NULL||*lp++!='{')					//no open token
		return tperror("Expected { here",true);
		
	memset(pd->BACnetStandardServices,ssNotSupported,sizeof(pd->BACnetStandardServices));	//default is not supported
	while (lp!=NULL)
	{	ReadNext();								//point to next token				***008
		if (*lp=='}'||lp==NULL) break;			//return, we're done with these
		i=-1;									//assume there is a problem
		if ((p=strchr(lp,space))!=NULL)			//find the delimiter for supported stuff
		{	*p++=0;								//make service name be asciz
	 		for (i=0;i<(sizeof(StandardServices)/sizeof(StandardServices[0]));i++)
			  if (stricmp(lp,StandardServices[i])==0) //found it
			  {	if (strstr(p,"Initiate")!=NULL)	//supports initiate
			  		pd->BACnetStandardServices[i]|=ssInitiate;
				if (strstr(p,"Execute")!=NULL)	//supports execute
			  		pd->BACnetStandardServices[i]|=ssExecute;
			  	i=(pd->BACnetStandardServices[i]==ssNotSupported)?-1:0;
				break;
			  }
		}
		if (i<0)
		//no delimiter after the service, or couldn't find initiate or execute
		{	p=strchr(lp,0);
			if (tperror("Expected 'Initiate' or 'Execute' here",true))
				return true;
		}
		else if (i>0)
		{	if (tperror("Unknown Standard Service",true))
				return true;
		}
	}
	return false;		
}

///////////////////////////////////////////////////////////////////////
//	Read Standard Objects section of a TPI file
//in:	ifile		file stream
//		pd			database structure to be filled in from PICS
//out:	true		if cancel selected

BOOL ReadStandardObjects(PICSdb *pd)
{	int			i;
	char		*pcre,*pdel;
	octet		sup;

	ReadNext();									//point to next token				***008
	if (lp==NULL||*lp++!='{')					//no open token
		return tperror("Expected { here",true);
		
	memset(pd->BACnetStandardObjects,soNotSupported,sizeof(pd->BACnetStandardObjects));	//default is not supported
	while (lp!=NULL)
	{	ReadNext();								//point to next token				***008
		if (*lp=='}'||lp==NULL) break;			//return, we're done with these
		sup=soSupported;
		if ((pcre=strstr(lp,"Createable"))!=NULL) //supports create
			sup|=soCreateable;
		if ((pdel=strstr(lp,"Deleteable"))!=NULL) //supports delete
			sup|=soDeleteable;
		if (pcre!=NULL)	pcre[-1]=0;				//cheesy way to "remove" this from the string
		if (pdel!=NULL)	pdel[-1]=0;				//cheesy way to "remove" this from the string

 		for (i=0;i<(sizeof(StandardObjects)/sizeof(StandardObjects[0]));i++)
			if (stricmp(lp,StandardObjects[i])==0) //found it
			{	pd->BACnetStandardObjects[i]=sup;
				i=0;
				break;
			}
		if (i>0)
		{	if (tperror("Unknown Standard Object",true))
				return true;
		}
	}
	if (pd->BACnetStandardObjects[DEVICE]==soNotSupported)
		return tperror("The Device Object was not present in the list of Standard Objects!",true);
	return false;		
}

///////////////////////////////////////////////////////////////////////
//	Read Data Link Layer Options section of a TPI file
//in:	ifile		file stream
//		pd			database structure to be filled in from PICS
//out:	true		if cancel selected

BOOL ReadDataLinkOptions(PICSdb *pd)
{	int			i,j;
	BOOL		got9600;
	char		*p;
	dword		*dp;

	ReadNext();									//point to next token				***008
	if (lp==NULL||*lp++!='{')					//no open token
		return tperror("Expected { here",true);
		
	memset(pd->DataLinkLayerOptions, 0, sizeof(pd->DataLinkLayerOptions)); //default is none
	pd->PTPAutoBaud[0]=pd->PTPAutoBaud[1]=0;
	for (i=0;i<16;i++)
	{	
		pd->MSTPmasterBaudRates[i]=0;
		pd->MSTPslaveBaudRates[i]=0;
		pd->PTP232BaudRates[i]=0;
		pd->PTPmodemBaudRates[i]=0;
	}
	while (lp!=NULL)
	{	ReadNext();								//point to next token				***008
		if (*lp=='}'||lp==NULL) break;			//return, we're done with these
		if ((p=strchr(lp,':'))!=NULL)			//colon in this one
			*p++=0;								//make it asciz there
		rtrim(lp);								//trim trailing blanks				***006
 		for (i=0;i<(sizeof(StandardDataLinks)/sizeof(StandardDataLinks[0]));i++)
		  if (stricmp(lp,StandardDataLinks[i])==0) //found it
		  {	pd->DataLinkLayerOptions[i] = 1;    //mark this data link supported
		  	switch(i)							//some of these need extra handling
		  	{
		  	case 9:								//MS/TP master
		  		dp=&pd->MSTPmasterBaudRates[0];
		  		goto rdlorates;
		  	case 10:							//MS/TP slave
				dp=&pd->MSTPslaveBaudRates[0];
		  		goto rdlorates;
		  	case 11:							//PTP 232
				dp=&pd->PTP232BaudRates[0];
		  		goto rdlorates;
		  	case 12:							//PTP modem, fixed baud rates
				dp=&pd->PTPmodemBaudRates[0];
rdlorates:		got9600=false;
				lp=p;							//point to argument(s)
				p[-1]=':';
				for (j=0;j<16;j++)				//read up to 16 baudrates
					if ( (*lp==0) || ((dp[j]=ReadDW())==0) )
						break;					//stop as soon as we fail to find another one
					else if (dp[j]==9600)
						got9600=true;			//remember if we find 9600 baud
						
				if ((i==9||i==10)&&got9600==false)	//MS/TP must include 9600 baud
				{	if (tperror("MS/TP devices must support 9600 baud!",true))
						return true;
				}
				break;
		  	case 13:							//PTP autobaud
				lp=p;							//point to argument(s)
				p[-1]=':';
		  		if ((pd->PTPAutoBaud[0]=ReadDW())==0)
				{	if (tperror("Expected Autobaud range 'from baudrate' here!",true))
						return true;
				}
				else
				{	skipwhitespace();
					if (strnicmp(lp,"to",2)==0)
					{	lp+=2;					//skip over the 'to'
						if ((pd->PTPAutoBaud[1]=ReadDW())==0)
						{	if (tperror("Expected Autobaud range 'to baudrate' here!",true))
							return true;
						}
					}
					else
					{	if (tperror("Expected 'to' here!",true))
							return true;
					}
				};
		  	}
			i=0;
			break;
		  }
		if (i)
		{	if (tperror("Unknown Data Link Layer Option",true))
				return true;
		}
	}
	return false;		
}

///////////////////////////////////////////////////////////////////////
//	Read Special Functionality Options section of a TPI file
//in:	ifile		file stream
//		pd			database structure to be filled in from PICS
//out:	true		if cancel selected

BOOL ReadSpecialFunctionality(PICSdb *pd)
{	int			i;
	char		*p;
	octet		*wp;
	dword		d;

	ReadNext();									//point to next token				***008
	if (lp==NULL||*lp++!='{')					//no open token
		return tperror("Expected { here",true);

	// Initialize Special Functionality items
	pd->RouterFunctions=rfNotSupported;			//default is none
	pd->SegmentedRequestWindow=pd->SegmentedResponseWindow=0;
	pd->MaxAPDUSize=50;
    pd->BBMD=0;  // default is no BBMD support
	while (lp!=NULL)
	{	ReadNext();								//point to next token				***008
		if (*lp=='}'||lp==NULL) break;			//return, we're done with these
		if ((p=strchr(lp,':'))!=NULL)			//colon in this one
			*p++=0;								//make it asciz there
 		for (i=0;i<(sizeof(SpecialFunctionality)/sizeof(SpecialFunctionality[0]));i++)
		  if (stricmp(lp,SpecialFunctionality[i])==0) //found it
		  {	switch(i)
		  	{
		  	case 0:								//max apdu size
				lp=p;							//point to argument(s)
				p[-1]=':';
				d=ReadDW();						//parse a window argument
				if (d<50||d>1476)				//APDU size must be 50-1476
				{	if (tperror("APDU sizes must be 50..1476!",true))
						return true;
				}
				pd->MaxAPDUSize=LOWORD(d);
				break;
		  	case 1:								//request window
				wp=&pd->SegmentedRequestWindow;
		  		goto rsfwin;
		  	case 2:								//response window
				wp=&pd->SegmentedResponseWindow;
rsfwin:			lp=p;							//point to argument(s)
				p[-1]=':';
				d=ReadDW();						//parse a window argument
				if (d<1||d>127)					//window size must be 1-127
				{	if (tperror("Window sizes must be 1..127!",true))
						return true;
				}
				*wp=LOBYTE(d);
				break;
		  	case 3:								//Router
		  		pd->RouterFunctions=rfSupported;
				break;
            case 4:
				pd->BBMD=1;                     //BBMD functionality is supported
				break;
		  	}
			i=0;
			break;
		  }
		if (i)
		{	if (tperror("Unknown Special Functionality Option",true))
				return true;
		}
	}
	return false;		
}

///////////////////////////////////////////////////////////////////////
//	Read Special Functionality Options section of a TPI file
//in:	ifile		file stream
//		pd			database structure to be filled in from PICS
//out:	true		if cancel selected

BOOL ReadFailTimes(PICSdb *pd)
{	
	int			i;
	char		*p;
	dword		d;
	BOOL        flag;

	ReadNext();									//point to next token		
	if (lp==NULL||*lp++!='{')					//no open token
		return tperror("Expected { here",true);

	memset(pd->BACnetFailTimes,ftNotSupported,sizeof(pd->BACnetFailTimes));	//default is not supported
	while (lp!=NULL) {
		ReadNext();
		if (*lp=='}'||lp==NULL) break;
		if ((p=strchr(lp,':'))==NULL){			//colon in this one
			return tperror("Expected { here",true);
		}
		*p++=0;	
		flag = FALSE;
		for (i=0;i<(sizeof(FailTimes)/sizeof(FailTimes[0]));i++)
		{
			if (stricmp(lp,FailTimes[i])==0)	//found Fail Time
			{
				lp=p;							//point to argument(s)
				p[-1]=':';
				d=ReadDW();						//parse a argument
				pd->BACnetFailTimes[i] = d;
				flag = TRUE;
				break;
			}
		}
		if (flag == FALSE) {
			if (tperror("Unknown Fail Time Option",true))
				return true;
		}
	}

	return FALSE;
}


	
///////////////////////////////////////////////////////////////////////
//	Read Object Database section of a TPI file
//in:	ifile		file stream
//		pd			database structure to be filled in from PICS
//out:	true		if cancel selected

BOOL ReadObjects(PICSdb *pd)
{	
	char	*pn;							//property name pointer
	char	objname[512];
	BOOL	WeKnowObjectType;
	word	objtype;						//enumeration value for object type
	dword	objid;							//object identifier
	octet	fType,fID,fName;				//										***014
	generic_object	*pobj,*po,*polast;		//pointers to objects
    BOOL    i;                            
	ReadNext();								//point to next token					***008
	if (lp==NULL||*lp++!='{')				//no open token
		return tperror("Expected { here",true);

	while (lp!=NULL&&!cancel)
	{
nextobject:										//										***012
		
		//added for reset all flag, ***********017
		bNeedReset = false;
		bHasNoneType = false;
		bHasReset = false;
		
		lp = ReadNext();						//point to next token					***008
		if (lp == NULL || *lp=='}') break;			//return, we're done with these
		if (*lp)								//not a blank line
		{	
			if (*lp=='{')						//begin a new object
			{	
				WeKnowObjectType=false;			//don't know what kind yet
				objtype=0;						//no object found yet
				objname[0]=0;
				fType=fID=fName=0;				//										***014
				pobj=NULL;
				while (true)					//(lp!=NULL)							***006
				{	
					lp = ReadNext();					//point to next token					***008
					if ((lp==NULL) || (*lp=='}')) break;	//done with this object
					if (objtype==0xFFFF)		//										***012
						goto nextobject;		//once we find a bogus object type, we skip the rest of the object def
					if (*lp)					//ignore blank lines
					{	
						skipwhitespace();		//point to first char of name
						pn=lp;
						if ((lp=strchr(pn,':'))==NULL)		//find its end
						{	
							lp=strchr(pn,0);	//point to the end
							if (tperror("Expected : after property name here!",true))
								return true;
						}
						*lp++=0;				//make property name asciz
						if (WeKnowObjectType)
						{	
							if (stricmp(pn,"object-type")==0)
							{	
								lp[-1]=':';
								if ( objtype <=etObjectTypes.propes && objtype!=ReadEnum(&etObjectTypes) )
								{	
									if (tperror("The object-type does not agree with the object-identifier!",true))
										return true;
									objtype=0xFFFF;		//pretend objid was bad
								}
								//added by xlp,2002-11
//								if (objtype != 0xFFFF)  // msdanner 9/2004
								if (objtype < MAX_DEFINED_OBJ)
								{
								   ObjInTestDB[objtype].object_type=objtype;       
								   ObjInTestDB[objtype].ObjIDSupported|=soSupported;
								   i=ObjInTestDB[objtype].ObjInstanceNum;
								   ObjInTestDB[objtype].ObjInstanceNum++;
								   ObjInTestDB[objtype].ObjInstance[i]=objid&0x003fffff;     
								}
								//ended by xlp,2002-11

								pobj->propflags[typeProp]|=PropIsPresent; //remember we saw object type		***014 Begin
								while (*lp==space||*lp==',') lp++;	//skip any more whitespace or commas
								if (*lp=='W'||*lp=='w')				//prop is writeable
									pobj->propflags[typeProp]|=PropIsWritable; //							***014 End
								continue;
							}
							else if (stricmp(pn,"object-identifier")==0)
							{	
								lp[-1]=':';
								objid=ReadObjID();
								if (objtype!=(word)(objid>>22))
								{	
									if (tperror("The object-type does not agree with the object-identifier!",true))
										return true;
									objtype=0xFFFF;		//pretend objid was bad
								}
								//added by xlp,2002-11
//							    if (objtype != 0xFFFF) // msdaner 9/2004
								if (objtype < MAX_DEFINED_OBJ)
							    {
								  ObjInTestDB[objtype].object_type=objtype;       
								  ObjInTestDB[objtype].ObjIDSupported|=soSupported;
								  i=ObjInTestDB[objtype].ObjInstanceNum;
								  ObjInTestDB[objtype].ObjInstanceNum++;
								  ObjInTestDB[objtype].ObjInstance[i]=objid&0x003fffff;     
								}
								//ended by xlp,2002-11
								pobj->propflags[idProp]|=PropIsPresent;	//remember we saw ID				***014 Begin
								while (*lp==space||*lp==',') lp++;	//skip any more whitespace or commas
								if (*lp=='W'||*lp=='w')				//prop is writeable
									pobj->propflags[idProp]|=PropIsWritable; //								***014 End
								continue;
							}
							print_debug("RO: About to PP\n");
							if (ParseProperty(pn,pobj,objtype)) return true;
							
							//Added for resetting the value, *********017
							if(bNeedReset & bHasNoneType)
								if(ParseProperty(NoneTypePropName,pobj,objtype)) return true;		
							
							print_debug("RO: Done PP'ing\n");
						}
						else							//don't know what kind of object this is yet
						{	
							if (stricmp(pn,"object-type")==0)
							{	
								lp[-1]=':';
								if ((objtype=ReadEnum(&etObjectTypes))!=0xFFFF)
								{	
									WeKnowObjectType=true;
								    objid=((dword)objtype)<<22;
									fType|=PropIsPresent; //remember we have the type						***014 Begin
									while (*lp==space||*lp==',') lp++;	//skip any more whitespace or commas
									if (*lp=='W'||*lp=='w')				//prop is writeable
										fType|=PropIsWritable;			//									***014 End
								}
							}
							else if (stricmp(pn,"object-identifier")==0)
							{	
								lp[-1]=':';
								if ((objid=ReadObjID())!=badobjid)
								{	WeKnowObjectType=true;
									objtype=(word)(objid>>22);
									fID|=PropIsPresent;	//remember we had the id							***014 Begin
									while (*lp==space||*lp==',') lp++;	//skip any more whitespace or commas
									if (*lp=='W'||*lp=='w')				//prop is writeable
										fID|=PropIsWritable;			//									***014 End
								}
								else
									objtype=0xFFFF;		//object identifier was bogus
							}
							else if (stricmp(pn,"object-name")==0)
							{	
								lp[-1]=':';
								if (setstring(objname,sizeof(objname),lp)) return true;
								fName|=PropIsPresent;	//remember we have the name							***014 Begin
								while (*lp==space||*lp==',') lp++;	//skip any more whitespace or commas
								if (*lp=='W'||*lp=='w')				//prop is writeable
									fType|=PropIsWritable;			//										***014 End
							}
							else
							{	
								lp[-1]=':';
								if (tperror("Must identify the object-identifier or object-type before defining this property!",true))
									return true;
							}
							if (WeKnowObjectType)		//just found out what type it is
							{	
								// 5-23-05 Shiyuan Xiao.
//								if (objtype>=etObjectTypes.propes)	//this is a proprietary object type
//								{	
//									tperror("Sorry, this version does not support Proprietary Objects in TextPICS!",true);
//									objtype=0xFFFF;		//pretend objid was bad
//									continue;
//								}

								if (objtype>=etObjectTypes.propes)
								{
									if ((pobj=(generic_object *)malloc(sizeof(proprietary_obj_type)))==NULL)		//can't allocate space for it
									{	
										tperror("Can't allocate space for this object...",true);
										objtype=0xFFFF;		//pretend objid was bad
										continue;
									}
									print_debug("LJT: object=%x",pobj);
									memset(pobj, 0, sizeof(proprietary_obj_type));	//zero it out first
									pobj->next = NULL;
									pobj->object_id = objid;
									pobj->object_type = objtype;
									memcpy(&pobj->object_name[0], &objname[0],32);

									pobj->propflags[typeProp]|=fType;	//found type	***014 Begin
									pobj->propflags[idProp]|=fID;		//found id
									pobj->propflags[nameProp]|=fName;	//found name	***014 End

									continue;
								}
								
								if ((pobj=(generic_object *)malloc(StdObjects[objtype].sotSize))==NULL)		//can't allocate space for it
								{	tperror("Can't allocate space for this object...",true);
									objtype=0xFFFF;		//pretend objid was bad
									continue;
								}
								print_debug("LJT: gen object=%x",pobj);

								memset(pobj,0,StdObjects[objtype].sotSize);	//zero it out first
								pobj->next=NULL;
								pobj->object_id=objid;
								pobj->object_type=objtype;
								memcpy(&pobj->object_name[0],&objname[0],32);
								pobj->propflags[typeProp]|=fType;	//found type	***014 Begin
								pobj->propflags[idProp]|=fID;		//found id
								pobj->propflags[nameProp]|=fName;	//found name	***014 End

							    // msdanner 9/2004: remember pointer to Device Object for consistency checks
								if (objtype == DEVICE)
									pd->pDeviceObject = (device_obj_type *)pobj;
							}
						}
					}
				}
				//here we've found the end of the object definition
				if (pobj)						//									***012 Begin
				{	if (pobj->object_name[0]==0) //make sure object is named
					{	tperror("Object must have a name...",true);
						objtype=0xFFFF;			//pretend objid was bad
					}
					if (objtype==0xFFFF)		//something was wrong with it
					{	free(pobj);			//toss allocated object
						return true;			//fail
					}
					po=pd->Database;			//check for uniqueness of objid
					polast = NULL;          
					while (po!=NULL)
					{	
						if (objid==po->object_id)	//oops, we already have this one!
						{	tperror("Object Identifier is not unique!",true);
							free(pobj);		//toss allocated object
							return true;		//fail
						}
						polast = po;  // msdanner 9/2004 - remember the last object in the list
						po=(generic_object *)po->next;			//try next one
					}
					// msdanner 9/2004 - add new objects to the end instead of the beginning
					//pobj->next=pd->Database;	//link it in
					//pd->Database=pobj;
					if (polast)
						polast->next = pobj;  // add new object after the last one
					else
						pd->Database = pobj;  // This is the first object, so set head pointer.
				}								//									***012 End
			}
			else								//anything else is junk
			{	if (tperror("Expected '{' to begin an object definition here!",true))
					return true;
			}
		}
	}
	return false;		
}

/*
//=====================================================================//
//EPICS consistency Object types check  added by xlp,2002-11
//in:
//out:
void CheckPICSPropCons(PICSdb *pd)
{   word objtype; 
    BOOL stdObjflag=false;
    generic_object *obj;
	obj=pd->Database;
	while(obj){
 	    objtype=obj->object_type;
        switch(objtype){
        case ANALOG_INPUT:
			//Check required properties for AI objects
			CheckObjRequiredProp(0,0x00000acf,obj,12);
			AI_CheckOptionalProperty(pd->BACnetStandardServices,obj->propflags);
		    break;
        case ANALOG_OUTPUT:
			CheckObjRequiredProp(0,0x0000c6cf,obj,16);
            AO_CheckOptionalProperty(pd->BACnetStandardServices,obj->propflags); 
			break;
        case ANALOG_VALUE:
			CheckObjRequiredProp(0,0x0000036f,obj,10);
            AV_CheckOptionalProperty(pd->BACnetStandardServices,obj->propflags);
			break;
        case BINARY_INPUT:
			CheckObjRequiredProp(0,0x000006cf,obj,12);
            BI_CheckOptionalProperty(pd->BACnetStandardServices,obj->propflags);
			break;
        case BINARY_OUTPUT:
			CheckObjRequiredProp(0,0x003006cf,obj,22);
			BO_CheckOptionalProperty(pd->BACnetStandardServices,obj->propflags);
			break;
        case BINARY_VALUE:
			CheckObjRequiredProp(0,0x0000016f,obj,10);
			BV_CheckOptionalProperty(pd->BACnetStandardServices,obj->propflags);
			break;
        case CALENDAR:
			CheckObjRequiredProp(0,0x0000002f,obj,6);
			break;
        case COMMAND:
			CheckObjRequiredProp(0,0x000000ef,obj,8);
			break;
        case DEVICE:
			CheckObjRequiredProp(0,0x8603e9ff,obj,32);  // Flags changed by W Fowlie Aug 2003:protocol-conformance-class no longer required
			DV_CheckOptionalProperty(pd->BACnetStandardServices,obj->propflags);
			break;
        case EVENT_ENROLLMENT:
			CheckObjRequiredProp(0,0x000007f7,obj,12);
			EE_CheckOptionalProperty(obj->propflags);
			break;
        case FILE_O:
			CheckObjRequiredProp(0,0x000003f7,obj,10);
			break;
        case GROUP:
			CheckObjRequiredProp(0,0x00000037,obj,6);
			break;
        case LOOP:
			CheckObjRequiredProp(0,0x0803fd6f,obj,28);
			LO_CheckOptionalProperty(pd->BACnetStandardServices,obj->propflags);
			break;
        case MULTI_STATE_INPUT:
			CheckObjRequiredProp(0,0x000006cf,obj,12);
			MI_CheckOptionalProperty(pd->BACnetStandardServices,obj->propflags);
			break;
        case MULTI_STATE_OUTPUT:
            CheckObjRequiredProp(0,0x000036cf,obj,14);
			MO_CheckOptionalProperty(pd->BACnetStandardServices,obj->propflags);
			break;
        case MULTI_STATE_VALUE:
			CheckObjRequiredProp(0,0x0000036f,obj,10);
			MV_CheckOptionalProperty(pd->BACnetStandardServices,obj->propflags);
			break;
        case NOTIFICATIONCLASS:
			CheckObjRequiredProp(0,0x000000f7,obj,8);
			break;
        case PROGRAM:
			CheckObjRequiredProp(0,0x0000141f,obj,14);
			PR_CheckOptionalProperty(pd->BACnetStandardServices,obj->propflags);
			break;
        case SCHEDULE:
            CheckObjRequiredProp(0,0x0000032f,obj,10);
			SC_CheckOptionalProperty(obj->propflags);
			break;
        case AVERAGING:
			CheckObjRequiredProp(0,0x00007caf,obj,16);
			//
			break;
        case TREND_LOG:
			CheckObjRequiredProp(0,0x0010f817,obj,22);
			TR_CheckOptionalProperty(pd->BACnetStandardServices,obj->propflags);
			break;
	    default:
		  //TRACE("CreatePropertyFromDB called for unknown object type");
           stdObjflag=true;
		  //ASSERT(0);
			break;
		  //tperror("This Object Type does't exist in Database!",false);
		}
		obj=(generic_object *)obj->next;
		if(stdObjflag) break;
	}
	obj=NULL;
	return;
}

//=====================================================================//
//Check properties of AI from EPICS ,added by xlp,2002-11
//=====================================================================//
void AI_CheckOptionalProperty(octet servs[MAX_SERVS_SUPP],octet propFlags[64])
{
  char checkMsg[150];

  // Check if any AI properties for intrinsic reporting exist
  if (propFlags[16] || propFlags[17] || propFlags[18] || propFlags[19] || 
      propFlags[20] || propFlags[21] || propFlags[22] || propFlags[23] || 
      propFlags[24])
  {
    // If any do, they all must
    if (!(propFlags[16] && propFlags[17] && propFlags[18] && propFlags[19] && 
          propFlags[20] && propFlags[21] && propFlags[22] && propFlags[23] && 
          propFlags[24]))
    {
      sprintf(checkMsg,"PropGroup(time_delay,notify_type...) are required because of intrinsic reporting supported by AI objects!\n");
      //PrintToFile(checkMsg);
      tperror(checkMsg,false);    
    }
  }

	//Check if properties of AI objects support cov reporting.
	if((servs[asSubscribeCOV])||(servs[asConfirmedCOVNotification])
				||(servs[asUnconfirmedCOVNotification])){
				  if(!(propFlags[15])){
                    sprintf(checkMsg,"Cov_Increment Property is  required because of cov reporting supported by AI objects!\n");
					//PrintToFile(checkMsg);
					tperror(checkMsg,false);		
				}
			}
	return;
}

//=====================================================================//
//Check properties of AO from EPICS ,added by xlp,2002-11
//=====================================================================//
void AO_CheckOptionalProperty(octet servs[MAX_SERVS_SUPP],octet propFlags[64])
{    char checkMsg[150]; 
	//Check if properties of AO objects support cov reporting.
	if((servs[asSubscribeCOV])||(servs[asConfirmedCOVNotification])
		 ||(servs[asUnconfirmedCOVNotification])){
			if(!(propFlags[16])){
                 sprintf(checkMsg,"Cov_Increment Property is  required because of cov reporting supported by AO objects!\n");
				 //PrintToFile(checkMsg);
				 tperror(checkMsg,false);		
				}
		}

  // Check if any AO properties for intrinsic reporting exist
  if (propFlags[17] || propFlags[18] || propFlags[19] || propFlags[20] || 
      propFlags[21] || propFlags[22] || propFlags[25] || propFlags[23] || 
      propFlags[24])
    {
    // If any do, they all must
      if (!(propFlags[17] && propFlags[18] && propFlags[19] && propFlags[20] && 
            propFlags[21] && propFlags[22] && propFlags[25] && propFlags[23] && 
            propFlags[24]))
      {
          sprintf(checkMsg,"PropGroup(time_delay,notify_type...) are required because of intrinsic reporting supported by AO objects!\n");
          //PrintToFile(checkMsg);
          tperror(checkMsg,false);    
      }
    }

  return;
}

//=====================================================================//
//Check properties of AV from EPICS ,added by xlp,2002-11
//=====================================================================//
void AV_CheckOptionalProperty(octet servs[MAX_SERVS_SUPP],octet propFlags[64])
{     char checkMsg[150];
	  //PropGroup 1 Cons check for priority_array and relinquish_default
	  if(((propFlags[11])|(propFlags[10]))&&!((propFlags[11])&(propFlags[10]))){
	      sprintf(checkMsg,"priority_array and relinquish_default must exist simultaneously for AV objects!\n");
		  //PrintToFile(checkMsg);
		  tperror(checkMsg,false);		
		}
	  //Check if properties of Av objects support cov reporting.
	  if((servs[asSubscribeCOV])||(servs[asConfirmedCOVNotification])
			||(servs[asUnconfirmedCOVNotification])){
			 if(!(propFlags[12])){
                  sprintf(checkMsg,"Cov_Increment Property is  required because of cov reporting supported by AV objects!\n");
					//PrintToFile(checkMsg);
					tperror(checkMsg,false);		
				}
			}

  // Check if any AV properties for intrinsic reporting exist
  if (propFlags[13] || propFlags[14] || propFlags[15] || propFlags[16] || 
      propFlags[17] || propFlags[18] || propFlags[19] || propFlags[20] || 
      propFlags[21])
  {
    // If any do, they all must
    if (!(propFlags[13] && propFlags[14] && propFlags[15] && propFlags[16] && 
          propFlags[17] && propFlags[18] && propFlags[19] && propFlags[20] && 
          propFlags[21]))
    {
      sprintf(checkMsg,"PropGroup(time_delay,notify_type...) are required because of intrinsic reporting supported by AV objects!\n");
      //PrintToFile(checkMsg);
      tperror(checkMsg,false);    
    }
  }

  return;
}
//=====================================================================//
//Check properties of BI from EPICS ,added by xlp,2002-11
//=====================================================================//
void BI_CheckOptionalProperty(octet servs[MAX_SERVS_SUPP],octet propFlags[64])
{   char checkMsg[150];
	//PropGroup 1 Cons check for inactive_text and active_text
	if(((propFlags[11])|(propFlags[12]))&&!((propFlags[11])&(propFlags[12]))){
		sprintf(checkMsg,"inactive_text and active_text must exist simultaneously for BI objects!\n");
		//PrintToFile(checkMsg);
		tperror(checkMsg,false);		
	}
	//PropGroup 2 Cons check for 3 properties,including change_of_state_time
	if(((propFlags[13])|(propFlags[14])|(propFlags[15]))
			&&!((propFlags[13])&(propFlags[14])&(propFlags[15]))){
				sprintf(checkMsg,"PropGroup,including change_of_state_time must exist simultaneously for BI objects!\n");
				//PrintToFile(checkMsg);
				tperror(checkMsg,false);		
	}
	//PropGroup 3 Cons check for Elapsed_Active_Time and Time_of_Active_Time_Reset
	if(((propFlags[16])|(propFlags[17]))&&!((propFlags[16])&(propFlags[17]))){
		sprintf(checkMsg,"Elapsed_Active_Time and Time_of_Active_Time_Reset must exist simultaneously for BI objects!\n");
		//PrintToFile(checkMsg);
		tperror(checkMsg,false);		
	}

  // Check if any BI properties for intrinsic reporting exist
  if (propFlags[18] || propFlags[19] || propFlags[20] || 
      propFlags[21] || propFlags[22] || propFlags[23])
  {
    // If any do, they all must
    if (!(propFlags[18] && propFlags[19] && propFlags[20] && 
          propFlags[21] && propFlags[22] && propFlags[23]))
    {
      sprintf(checkMsg,"PropGroup(time_delay,notify_type...) are required because of intrinsic reporting supported by BI objects!\n");
      //PrintToFile(checkMsg);
      tperror(checkMsg,false);    
    }
  }

  return;
}

//=====================================================================//
//Check properties of BO from EPICS ,added by xlp,2002-11
//=====================================================================//
void BO_CheckOptionalProperty(octet servs[MAX_SERVS_SUPP],octet propFlags[64])
{   char checkMsg[150];
	//PropGroup 1 Cons check for inactive_text and active_text
	if(((propFlags[11])|(propFlags[12]))&&!((propFlags[11])&(propFlags[12]))){
		sprintf(checkMsg,"inactive_text and active_text must exist simultaneously for BO objects!\n");
		//PrintToFile(checkMsg);
		tperror(checkMsg,false);		
	}
	//PropGroup 2 Cons check for 3 properties,including change_of_state_time
	if(((propFlags[13])|(propFlags[14])|(propFlags[15]))
			&&!((propFlags[13])&(propFlags[14])&(propFlags[15]))){
				sprintf(checkMsg,"PropGroup,including change_of_state_time must exist simultaneously for BO objects!\n");
				//PrintToFile(checkMsg);
				tperror(checkMsg,false);		
	}
	//PropGroup 3 Cons check for Elapsed_Active_Time and Time_of_Active_Time_Reset
	if(((propFlags[16])|(propFlags[17]))&&!((propFlags[16])&(propFlags[17]))){
		sprintf(checkMsg,"Elapsed_Active_Time and Time_of_Active_Time_Reset must exist simultaneously for BO objects!\n");
		//PrintToFile(checkMsg);
		tperror(checkMsg,false);		
	}

  // Check if any BO properties for intrinsic reporting exist
  if (propFlags[22] || propFlags[23] || propFlags[24] || 
      propFlags[25] || propFlags[26] || propFlags[27])
  {
    if(!(propFlags[22] && propFlags[23] && propFlags[24] && 
         propFlags[25] && propFlags[26] && propFlags[27]))
    {
      sprintf(checkMsg,"PropGroup(time_delay,notify_type...) are required because of intrinsic reporting supported by BO objects!\n");
      //PrintToFile(checkMsg);
      tperror(checkMsg,false);    
    }
  }

      return;
}
//=====================================================================//
//Check properties of BV from EPICS ,added by xlp,2002-11
//=====================================================================//
void BV_CheckOptionalProperty(octet servs[MAX_SERVS_SUPP],octet propFlags[64])
{   char checkMsg[150];
	//PropGroup 1 Cons check for inactive_text and active_text
    if(((propFlags[9])|(propFlags[10]))&&!((propFlags[9])&(propFlags[10]))){
			sprintf(checkMsg,"inactive_text and active_text must exist simultaneously for BV objects!\n");
			//PrintToFile(checkMsg);
			tperror(checkMsg,false);		
	}
	//PropGroup 2 Cons check for 3 properties,including change_of_state_time
	if(((propFlags[11])|(propFlags[12])|(propFlags[13]))
			&&!((propFlags[11])&(propFlags[12])&(propFlags[13]))){
				sprintf(checkMsg,"PropGroup,including change_of_state_time must exist simultaneously for BV objects!\n");
				//PrintToFile(checkMsg);
				tperror(checkMsg,false);		
	}
	//PropGroup 3 Cons check for Elapsed_Active_Time and Time_of_Active_Time_Reset
	if(((propFlags[14])|(propFlags[15]))&&!((propFlags[14])&(propFlags[15]))){
			sprintf(checkMsg,"Elapsed_Active_Time and Time_of_Active_Time_Reset must exist simultaneously for BV objects!\n");
			//PrintToFile(checkMsg);
			tperror(checkMsg,false);		
	}
	//PropGroup 4 Cons check for priority_array and relinquish_default
	if(((propFlags[18])|(propFlags[19]))&&!((propFlags[18])&(propFlags[19]))){
		    sprintf(checkMsg,"priority_array and relinquish_default must exist simultaneously for BV objects!\n");
			//PrintToFile(checkMsg);
			tperror(checkMsg,false);		
	}

  // Check if any BV properties for intrinsic reporting exist
  if (propFlags[20] || propFlags[21] || propFlags[22] || 
      propFlags[23] || propFlags[24] || propFlags[25])
  {
    // If any do, they all must
    if (!(propFlags[20] && propFlags[21] && propFlags[22] && 
          propFlags[23] && propFlags[24] && propFlags[25]))
    {
      sprintf(checkMsg,"PropGroup(time_delay,notify_type...) are required because of intrinsic reporting supported by BV objects!\n");
      //PrintToFile(checkMsg);
      tperror(checkMsg,false);    
    }
  }

  return;
}
//=====================================================================//
//Check properties of DV from EPICS ,added by xlp,2002-11
//=====================================================================//
void DV_CheckOptionalProperty(octet servs[MAX_SERVS_SUPP],octet propFlags[64])
{   char checkMsg[150];
    //PropGroup 1 check for VT_Classes_Supported and Active_VT_Sessions
	if((servs[asVT_Open])&(servs[asVT_Data])&(servs[asVT_Close])){
//         if(!((propFlags[18])&(propFlags[19]))){
			// since new property -Max_segmentation_accepted has been added,
			// the index number corresponding to the property VT_Classes_Supported is 19 
			// the index number corresponding to the property Active_VT_Sessions is 20 
		   if(!((propFlags[19])&(propFlags[20]))){								// *****018 
 		     sprintf(checkMsg,"VT_Classes_Supported and Active_VT_Sessions should be required when Device object support VT services!\n");
			 //PrintToFile(checkMsg);
		     tperror(checkMsg,false);		
		}
	}
	else{
//		if(((propFlags[18])|(propFlags[19]))&&!((propFlags[18])&(propFlags[19]))){
		if(((propFlags[19])|(propFlags[20]))&&!((propFlags[19])&(propFlags[20]))){		// *****018 
			sprintf(checkMsg,"VT_Classes_Supported and Active_VT_Sessions must exist simultaneously for Device object!\n");
			//PrintToFile(checkMsg);
			tperror(checkMsg,false);		
		}
	}
			//if Segment is supported the property APDU_Segment_Timeout is required for Device Object
			//if(!(propFlags[24])) PrintToFile("The property APDU_Segment_Timeout is required if Segment supported\n!");

            //if device is Time Master,property Time_Synchronization_Recipients is required
			//

            //if device is master node for MS/TP,property Max_Master and Max_Info_Frame
			//
return;
}
//=====================================================================//
//Check properties of EE from EPICS ,added by xlp,2002-11
//=====================================================================//
void EE_CheckOptionalProperty(octet propFlags[64])
{   char checkMsg[150];
    //check Notification_Class property
	if(!(propFlags[12])&&!(propFlags[11])){
		sprintf(checkMsg,"Event Enrollment object:the Notification_Class property shall be present if the Recipient property is not!\n");
		//PrintToFile(checkMsg);
		tperror(checkMsg,false);
	}
	//all of the following properties shall be present if the Notification_Class property is not
	if(!(propFlags[11])){
		if(!((propFlags[12])&(propFlags[13])&(propFlags[14])&(propFlags[15]))){
			sprintf(checkMsg,"Event Enrollment object:PropGroup(Recipient property...)shall be present if the Notification_Class property is not!\n");
			//PrintToFile(checkMsg);
			tperror(checkMsg,false);
		}
	}
  return;
}
//=====================================================================//
//Check properties of loop from EPICS ,added by xlp,2002-11
//=====================================================================//
void LO_CheckOptionalProperty(octet servs[MAX_SERVS_SUPP],octet propFlags[64])
{   char checkMsg[150];
 	//PropGroup 1 Cons check for Proportional_Constant and Proportional_Constant_Units
	if(((propFlags[18])|(propFlags[19]))&&!((propFlags[18])&(propFlags[19]))){
		 sprintf(checkMsg,"Proportional_Constant and Proportional_Constant_Units must exist simultaneously for Loop objects!\n");
		 //PrintToFile(checkMsg);
		 tperror(checkMsg,false);		
	}
	//PropGroup 2 Cons check for Integral_Constant and Integral_Constant_Units
	if(((propFlags[20])|(propFlags[21]))&&!((propFlags[20])&(propFlags[21]))){
		sprintf(checkMsg,"Integral_Constant and Integral_Constant_Units must exist simultaneously for Loop objects!\n");
		//PrintToFile(checkMsg);
		tperror(checkMsg,false);		
	}
	//PropGroup 3 Cons check for Derivative_Constant and Derivative_Constant_Units
	if(((propFlags[22])|(propFlags[23]))&&!((propFlags[22])&(propFlags[23]))){
		sprintf(checkMsg,"Derivative_Constant and Derivative_Constant_Units must exist simultaneously for Loop objects!\n");
		//PrintToFile(checkMsg);
		tperror(checkMsg,false);		
	}
	//Check if properties of Loop objects support cov reporting.
	if((servs[asSubscribeCOV])||(servs[asConfirmedCOVNotification])
		 ||(servs[asUnconfirmedCOVNotification])){
			if(!(propFlags[28])){
                sprintf(checkMsg,"Cov_Increment Property is  required because of cov reporting supported by Loop objects!\n");
				//PrintToFile(checkMsg);
				tperror(checkMsg,false);		
			}
	}

  // Check if any Loop properties for intrinsic reporting exist
  if (propFlags[29] || propFlags[30] || propFlags[31] || 
      propFlags[32] || propFlags[33] || propFlags[34])
  {
    // If any do, they all must
    if (!(propFlags[29] && propFlags[30] && propFlags[31] && 
          propFlags[32] && propFlags[33] && propFlags[34]))
    {
      sprintf(checkMsg,"PropGroup(time_delay,notify_type...) are required because of intrinsic reporting supported by Loop objects!\n");
      //PrintToFile(checkMsg);
      tperror(checkMsg,false);    
    }  
  }

  return;
}

//=====================================================================//
//Check properties of Multi-State-Input from EPICS ,added by xlp,2002-11
//=====================================================================//
void MI_CheckOptionalProperty(octet servs[MAX_SERVS_SUPP],octet propFlags[64])
{    char checkMsg[150];
  // Check if any MI properties for intrinsic reporting exist
  if (propFlags[12] || propFlags[13] || propFlags[14] || propFlags[15] || 
//      propFlags[16] || propFlags[17] || propFlags[18])
      propFlags[16] || propFlags[17] || propFlags[18] || propFlags[19]) //modified by Jingbo Gao, the Event_Time_stamps is required if intrinsic reporting is supportedby this oject
  {
    // If any do, they all must
    if (!(propFlags[12] && propFlags[13] && propFlags[14] && propFlags[15] && 
//          propFlags[16] && propFlags[17] && propFlags[18]))
          propFlags[16] && propFlags[17] && propFlags[18] && propFlags[19])) //modified by Jingbo Gao
    {
      sprintf(checkMsg,"PropGroup(time_delay,notify_type...) are required because of intrinsic reporting supported by MI objects!\n");
      //PrintToFile(checkMsg);
      tperror(checkMsg,false);    
    }
  }

  return;
}

//=====================================================================//
//Check properties of Multi-State-Output from EPICS ,added by xlp,2002-11
//=====================================================================//
void MO_CheckOptionalProperty(octet servs[MAX_SERVS_SUPP],octet propFlags[64])
{
  char checkMsg[150];

  // Check if any MO properties for intrinsic reporting exist
  if (propFlags[14] || propFlags[15] || propFlags[16] || 
      propFlags[17] || propFlags[18] || propFlags[19])
  {
    if (!(propFlags[14] && propFlags[15] && propFlags[16] && 
          propFlags[17] && propFlags[18] && propFlags[19]))
    {
      sprintf(checkMsg,"PropGroup(time_delay,notify_type...) are required because of intrinsic reporting supported by MO objects!\n");
      //PrintToFile(checkMsg);
      tperror(checkMsg,false);		
    }
  }

  return;
}
*/
/*
//=====================================================================//
//Check properties of Multi-State-Value from EPICS ,added by xlp,2002-11
//=====================================================================//
void MV_CheckOptionalProperty(octet servs[MAX_SERVS_SUPP],octet propFlags[64])
{   char checkMsg[150];
  //check Reliability property
  if((propFlags[16])&&!(propFlags[7])){
    sprintf(checkMsg,"Mv object type:Reliability shall be required if Fault_Values is present!\n");
    //PrintToFile(checkMsg);
    tperror(checkMsg,false);		
  }

	//PropGroup 1 Cons check for Priority_Array and Relinquish_Default
	if(((propFlags[11])|(propFlags[12]))&&!((propFlags[11])&(propFlags[12]))){
		sprintf(checkMsg,"Priority_Array and Relinquish_Default must exist simultaneously for MV objects!\n");
		//PrintToFile(checkMsg);
		tperror(checkMsg,false);		
	}

  // Check if any MV properties for intrinsic reporting exist
  if (propFlags[13] || propFlags[14] || propFlags[15] || propFlags[16] || 
      propFlags[17] || propFlags[18] || propFlags[19] || propFlags[20])
  {
    // If any do, they all must
    if (!(propFlags[13] && propFlags[14] && propFlags[15] && propFlags[16] && 
          propFlags[17] && propFlags[18] && propFlags[19] && propFlags[20]))
    {
      sprintf(checkMsg,"PropGroup(time_delay,notify_type...) are required because of intrinsic reporting supported by MV objects!\n");
      //PrintToFile(checkMsg);
      tperror(checkMsg,false);
    }
  }

  return;
}

//=====================================================================//
//Check properties of Program from EPICS ,added by xlp,2002-11
//=====================================================================//
void PR_CheckOptionalProperty(octet servs[MAX_SERVS_SUPP],octet propFlags[64])
{   char checkMsg[150];
	//PropGroup 1 Cons check for Reason_For_Halt and Description_Of_Halt
	if(((propFlags[5])|(propFlags[6]))&&!((propFlags[5])&(propFlags[6]))){
		sprintf(checkMsg,"Reason_For_Halt and Description_Of_Halt must exist simultaneously for Program objects!\n");
		//PrintToFile(checkMsg);
		tperror(checkMsg,false);		
	}

    return;
}

//=====================================================================//
//Check properties of Schedule from EPICS ,added by xlp,2002-11
//=====================================================================//
void SC_CheckOptionalProperty(octet propFlags[64])
{   char checkMsg[150];
	//PropGroup 1 of which one property must be required at least
	if(!((propFlags[6])|(propFlags[7]))){
			sprintf(checkMsg,"at least one of the two properties(Weekly_Schedule and Exception_Schedule) must exist for Schedule objects!\n");
			//PrintToFile(checkMsg);
			tperror(checkMsg,false);		
	}
    return;
}

//=====================================================================//
//Check properties of Trend Log from EPICS ,added by xlp,2002-11
//=====================================================================//
void TR_CheckOptionalProperty(octet servs[MAX_SERVS_SUPP],octet propFlags[64])
{
  char checkMsg[150];

  // Check if any TL properties for intrinsic reporting exist
  if (propFlags[16] || propFlags[17] || propFlags[18] || propFlags[19] || 
      propFlags[21] || propFlags[22] || propFlags[23] || propFlags[24] || 
      propFlags[25])
  {
    // If any do, they all must
    if (!(propFlags[16] && propFlags[17] && propFlags[18] && propFlags[19] && 
          propFlags[21] && propFlags[22] && propFlags[23] && propFlags[24] && 
          propFlags[25]))
    {
      sprintf(checkMsg,"PropGroup(notification_threshold,notify_type...) are required because of intrinsic reporting supported by Trend Log objects!\n");
      //PrintToFile(checkMsg);
      tperror(checkMsg,false);    
    }
  }

  return;
}
*/
/*
////////////////////////////////////////////////////////////////////////
//EPICS consistency Object types check  added by xlp,2002-11
//in:
//out:
void CheckObjRequiredProp(dword hi_propFlag,dword lo_propFlag,generic_object *obj,octet Num)
{
  int i;
	octet  pIndex=0;
	char   *objName;
	char   *propName;
	char   errMsg[200]; 
	word   objtype;
	dword  objInstance;
	propdescriptor *pd;
	dword flag1=0x00000001;
	dword flag2=0x00000001;
	objtype=obj->object_type;
	pd=StdObjects[objtype].sotProps;
	objInstance=obj->object_id;
	objInstance&=0x003fffff;
    objName=StandardObjects[objtype];
	for(i=0;i<Num;i++){
     	propName=pd->PropertyName;
		if(i<32){ 
		  if(lo_propFlag&flag1){
				if(!((obj->propflags[pIndex])&1)){
					sprintf(errMsg,"135.1-2003 5.(i): "
					   "Object (%s,%u) must contain required property %s.\n",objName,objInstance,propName);
					//PrintToFile(errMsg);
					tperror(errMsg,false);
				}
			}
		  flag1=flag1<<1;
		}
		 else{
  		   if(hi_propFlag&flag2){
				if(!((obj->propflags[pIndex])&1)){
					sprintf(errMsg,"135.1-2003 5.(i): "
					   "Object (%s,%u) must contain required property %s.\n",objName,objInstance,propName);
					//PrintToFile(errMsg);
					tperror(errMsg,false);
				}
			}
		   flag2=flag2<<1;
		 }

			pIndex++;
			pd++;
		}

    return;
}
*/
////////////////////////////////////////////////////////////////////////
//EPICS consistency Object types check  added by xlp,2002-11
//in:
//out:
/*
void CheckPICSObjCons(PICSdb *pd)
{ 
//PICS Cons Check a:
  CheckObjConsA(pd);
//PICS Cons Check K:
  CheckObjConsK(pd);
//PICS Cons Check L: 
  CheckObjConsL(pd);
//PICS Cons Check m:
  CheckObjConsM();
//PICS Cons Check f:
  CheckObjConsF(pd);
  return;
 }
*/
////////////////////////////////////////////////////////////////////////
//EPICS consistency Object types check  added by xlp,2002-11
//in:
//out:
/*
void CheckPICSServCons(PICSdb *pd)
{  
   CheckServConsD(pd);
   CheckServConsE(pd);
   CheckServConsI(pd);
   CheckServConsJ(pd);
   return;
}
*/
////////////////////////////////////////////////////////////////////////
//EPICS consistency Application Services check  added by xlp,2002-11
//in:
//out:
void PrintToFile(char *outBuf)
{
//	FILE *op;  
    word strLength;
	if ( lPICSErr != -1 )
		lPICSErr++;
	if(outBuf == NULL) return;
	strLength=strlen(outBuf);
//	op = fopen("c:\\EPICSConsChk.txt","a+");
//	if(op == NULL) return;
	if ( pfileError == NULL )
		return;
//	fprintf(op,"%s",outBuf);
	fprintf(pfileError,"%s",outBuf);
//	fclose(op);

	return;

}
////////////////////////////////////////////////////////////////////////
//EPICS consistency Application Services check  added by xlp,2002-11
//in:
//out:
/*
void CheckClass(word n, TApplServ * pApplServ, octet  ApplServSup[],char *errorMsg,BOOL IniExeFlag)
{   int i; 
    BOOL ServNum;
	char opj[200];
    char *ServName;

	if(!IniExeFlag){
	  for(i=0;i<n;i++)
	  {  ServNum=pApplServ[i].ApplServ;
	     if((ApplServSup[ServNum]&(pApplServ[i].InitExec))!=pApplServ[i].InitExec){
			  ServName=StandardServices[ServNum];
			  if(pApplServ[i].InitExec == ssInitiate)
				sprintf(opj,"%s%s standard service should be supported with Initiate !\n",errorMsg,ServName);
			  else
				  if(pApplServ[i].InitExec == ssExecute)
					sprintf(opj,"%s%s standard service should be supported with Execute !\n",errorMsg,ServName);
				  else
					sprintf(opj,"%s%s standard service should be supported with Initiate and Execute !\n",errorMsg,ServName);	
			  //PrintToFile(opj);
			  tperror(opj,false);
		 }
	   }
	}
	else if(IniExeFlag==ssExecute){
	  for(i=0;i<n;i++)
	  {  if(((pApplServ[i].InitExec)&ssExecute)==ssExecute){
		     ServNum=pApplServ[i].ApplServ;
	         if((ApplServSup[ServNum]&(pApplServ[i].InitExec)&ssExecute)!=ssExecute){
			     ServName=StandardServices[ServNum];
			     sprintf(opj,"%s%s standard service should be supported with Initiate and Execute !\n",errorMsg,ServName);
			     //PrintToFile(opj);
			     tperror(opj,false);
			 }
	      }
	      else continue;
	  }
	}
    return;
}
*/
////////////////////////////////////////////////////////////////////////
//EPICS consistency Application Services check  added by xlp,2002-11
//in:
//out:
/*
void CheckServClassCons(word Num,octet ApplServSup[MAX_SERVS_SUPP],char *errorMsg,BOOL IniExeFlag)
{

	switch(Num){
	case 6:
		CheckClass(nFgClock,gFgClock,ApplServSup,errorMsg,IniExeFlag);
		CheckClass(nFgPCWS,gFgPCWS,ApplServSup,errorMsg,IniExeFlag);
		CheckClass(nFgEventInit,gFgEventInit,ApplServSup,errorMsg,IniExeFlag);
		CheckClass(nFgEventResponse,gFgEventResponse,ApplServSup,errorMsg,IniExeFlag);
		CheckClass(nFgFiles,gFgFiles,ApplServSup,errorMsg,IniExeFlag);
	case 5:
		CheckClass(nCC5,gCC5_Table,ApplServSup,errorMsg,IniExeFlag);
	case 4:
		CheckClass(nCC4,gCC4_Table,ApplServSup,errorMsg,IniExeFlag);
	case 3:
		CheckClass(nCC3,gCC3_Table,ApplServSup,errorMsg,IniExeFlag);
	case 2:
		CheckClass(nCC2,gCC2_Table,ApplServSup,errorMsg,IniExeFlag);
	default:
		CheckClass(nCC1,gCC1_Table,ApplServSup,errorMsg,IniExeFlag);
		break;
	}
   return;
}
*/


////////////////////////////////////////////////////////////////////////
//EPICS consistency Application Services check  added by xlp,2002-11
//in:
//out:
/*
void CheckServConsD(PICSdb *pd)
{   word classNum;
    char errorMsg[50]="EPICS Services Cons Check Error:";
    classNum=pd->BACnetConformanceClass;
	CheckServClassCons(classNum, pd->BACnetStandardServices,errorMsg,0);
	return;
}
*/
////////////////////////////////////////////////////////////////////////
//EPICS consistency Application Services check  added by xlp,2002-11
//in:
//out:
/*
void CheckServFGCons(dword nFG,octet Applserv[],char *errMsg,BOOL IniExeFlag)
{   dword nFGFlag=0x00000001; 
	dword nFGNum;
    int i; 
	if(!nFG) return;
	for(i=0;i<13;i++){
		nFGNum=nFG&nFGFlag;
		switch(nFGNum){
		case fgHHWS:
			CheckClass(nFgHHWS,gFgHHWS,Applserv,errMsg,IniExeFlag);
			break;
		case fgPCWS:
			CheckClass(nFgPCWS,gFgPCWS,Applserv,errMsg,IniExeFlag);
			break;
		case fgCOVEventInitiation:
			CheckClass(nFgCOVInit,gFgCOVInit,Applserv,errMsg,IniExeFlag);
			break;
        case fgCOVEventResponse:
			CheckClass(nFgCOVResponse,gFgCOVResponse,Applserv,errMsg,IniExeFlag);
			break;
		case fgEventInitiation:
			CheckClass(nFgEventInit,gFgEventInit,Applserv,errMsg,IniExeFlag);
			break;
		case fgEventResponse:
			CheckClass(nFgEventResponse,gFgEventResponse,Applserv,errMsg,IniExeFlag);
			break;
		case fgClock:
			CheckClass(nFgClock,gFgClock,Applserv,errMsg,IniExeFlag);
			break;
		case fgDeviceCommunications:
			CheckClass(nFgDevCom,gFgDevCom,Applserv,errMsg,IniExeFlag);
			break;
		case fgFiles:
			CheckClass(nFgFiles,gFgFiles,Applserv,errMsg,IniExeFlag);
			break;
		case fgTimeMaster:
			CheckClass(nFgTimeMaster,gFgTimeMaster,Applserv,errMsg,IniExeFlag);
			break;
		case fgVirtualOPI:
			CheckClass(nFgVO,gFgVO,Applserv,errMsg,IniExeFlag);
			break;
		case fgReinitialize:
			CheckClass(nFgReinitialize,gFgReinitialize,Applserv,errMsg,IniExeFlag);
			break;
		case fgVirtualTerminal:
			CheckClass(nFgVT,gFgVT,Applserv,errMsg,IniExeFlag);
			break;
		default: return;
			break;
	//		tperror()
		}
       if(nFGFlag!=0x80000000) nFGFlag<<=1;
	}

}
*/
////////////////////////////////////////////////////////////////////////
//EPICS consistency Application Services check  added by xlp,2002-11
//in:
//out:
/*
void CheckServConsI(PICSdb *pd)
{   dword nFG;
    char errorMsg[50]="EPICS Services Cons Check Error:"; 
    nFG=pd->BACnetFunctionalGroups;  
    CheckServFGCons(nFG,pd->BACnetStandardServices,errorMsg,0);
	return;
}
*/
////////////////////////////////////////////////////////////////////////
//EPICS consistency Application Services check  added by xlp,2002-11
//in:
//out:
void BitToArray(octet ObjServ[],DevProtocolSup *DevProp)
{   octet  bitflag; 
    octet  byteNum;
	octet *p;
	BOOL  result;
	int i,j,temp;
	int count=0;
	bitflag=0x80;
    memset(ObjServ,0,Max_ObjServ_Num);
	temp=DevProp->ObjServNum;  
	if(temp%8) byteNum=temp/8;
	else byteNum=temp/8-1;
	temp--;
	p=DevProp->PropSupValue;  
	for(i=0;i<=byteNum;i++){
		for(j=0;j<8;j++){
    			if(count>temp) break;
				result=*p&bitflag;
				bitflag>>=1;
                if(result) 
					ObjServ[count]=Protocol_Serv_Sup;
				count++;
		}
			bitflag=0x80;
			p++;
	}
   return;
}
////////////////////////////////////////////////////////////////////////
//EPICS consistency Application Services check  added by xlp,2002-11
//in:
//out:
/*
void CheckServConsE(PICSdb *pd)
{   word classNum;
    char errMsg[50]="EPICS Services Cons Check Error:";
    octet ObjServ[MAX_SERVS_SUPP];
	memset(ObjServ,0,MAX_SERVS_SUPP);
    classNum=pd->BACnetConformanceClass;

	BitToArray(ObjServ,&ProtocolServSup);
	CheckServClassCons(classNum, ObjServ,errMsg,ssExecute);

}
*/

////////////////////////////////////////////////////////////////////////
//EPICS consistency Application Services check  added by xlp,2002-11
//in:
//out:
/*
void CheckServConsJ(PICSdb *pd)
{   char errorMsg[50]="EPICS Services Cons Check Error:";
	octet ObjServ[MAX_SERVS_SUPP];
    dword nFG;
    nFG=pd->BACnetFunctionalGroups;  
    BitToArray(ObjServ,&ProtocolServSup);
    CheckServFGCons(nFG,ObjServ,errorMsg,ssExecute);
	return;
}
*/
////////////////////////////////////////////////////////////////////////
//EPICS Object Types consistency check  added by xlp,2002-11
//Test A: for Object types supported by Conformance Class
/*
void CheckObjConsA(PICSdb *pd)
{ word nClassNum;
  dword nFG; 
  BOOL flag=false;
  char opj[200];
  nClassNum=pd->BACnetConformanceClass;
  nFG=pd->BACnetFunctionalGroups;
  switch(nClassNum){
  case 6: 
      //check obj type for clock FG supported by Cons Class 6 
    if (nFG&fgClock) {
      if (pd->BACnetStandardObjects[DEVICE]==soNotSupported) {
        sprintf(opj,"EPICS Object Consistency check error: "
          "The Device Object type should be included to support the Clock Functional Group!\n");
        //PrintToFile(opj);
        tperror(opj,false);
      }
    }
    else{
       sprintf(opj,"EPICS Object Consistency check error: "
          "Conformance Class 6 claims that Clock Functional Group must be supported!\n");
       //PrintToFile(opj);
         tperror(opj,false);
    }
    //check obj type for EventInitiation and EventResponse Functional Group
    if((nFG&fgEventInitiation)||(nFG&fgEventResponse))     //Event Initiation/Response FG Obj Cons Check
    {
      if(pd->BACnetStandardServices[asConfirmedCOVNotification]||pd->BACnetStandardServices[asUnconfirmedCOVNotification]) //FG support COV Notification Services
        if(pd->BACnetStandardObjects[EVENT_ENROLLMENT]==soNotSupported) {
          sprintf(opj,"EPICS Object Consistency check error: "
            "Event Enrollment Object type should be included to support the Event Initiation|Response Functional Group!\n");
          //PrintToFile(opj);
          tperror(opj,false);
        }
      if(pd->BACnetStandardServices[asConfirmedEventNotification]||pd->BACnetStandardServices[asUnconfirmedEventNotification]) {
        if(pd->BACnetStandardObjects[NOTIFICATIONCLASS])
        {
          if(pd->BACnetStandardObjects[ANALOG_INPUT]||pd->BACnetStandardObjects[ANALOG_OUTPUT]||pd->BACnetStandardObjects[ANALOG_VALUE])
            flag = true;
          else if(pd->BACnetStandardObjects[BINARY_OUTPUT]||pd->BACnetStandardObjects[BINARY_VALUE])
            flag = true;
          else if(pd->BACnetStandardObjects[MULTI_STATE_OUTPUT])
            flag = true;
          else if(pd->BACnetStandardObjects[LOOP])
            flag = true;
          else if(!flag){
            sprintf(opj,"EPICS Object Consistency check error: "
              "at least one of Std Obj types listed by Table 13-2 should be included "
              "to support the Event Initiation|Response Functional Group! \n");
            //PrintToFile(opj);
            tperror(opj,false);
          }
        }
        else{
          sprintf(opj,"EPICS Object Consistency check error: "
            "Notification Class Object type should be included to support "
            "the Event Initiation|Response Functional Group! \n");
          //PrintToFile(opj);
          tperror(opj,false);
        }
      }
    }
    else{
      sprintf(opj,"EPICS Object Consistency check error: "
        "Conformance Class 6 claims that EventInitiation and EventResponse FGs must be supported! \n");
      //PrintToFile(opj);
      tperror(opj,false);
    }

      if(nFG&fgFiles){
      if(pd->BACnetStandardObjects[FILE_O]==soNotSupported){
        sprintf(opj,"EPICS Object Consistency check error: "
          "File Object type should be included to support the File Functional Group! \n");
        //PrintToFile(opj);
          tperror(opj,false);       
      }
    }
    else{
      sprintf(opj,"EPICS Object Consistency check error: "
        "Conformance Class 6 claims that File Functional Group must be supported! \n");
      //PrintToFile(opj);
      tperror(opj,false);
    }
    if(!(nFG&fgPCWS)){
      sprintf(opj,"EPICS Object Consistency check error: "
        "Conformance Class 6 claims that PCWorkStation Functional Group must be supported! \n");
      //PrintToFile(opj);
      tperror(opj,false);
    }
    
  case 5: 
  case 4: 
  case 3: 
  case 2: 
  default:                 //Conformance Class 1 must be supported;
    if(pd->BACnetStandardObjects[DEVICE]==soNotSupported){
      sprintf(opj,"EPICS Object Consistency check error: "
        "The Device Object type must be included to meet the requirement by Conformance Class 1!\n");
      //PrintToFile(opj);
      tperror(opj,false);
    }
    break;
  }
  return;
}
*/
/*
////////////////////////////////////////////////////////////////////////
//EPICS Object Types consistency check  added by xlp,2002-11
//Test F: for Object types supported by Functional Group 
void CheckObjConsF(PICSdb *pd)
{ dword nFG;
  BOOL flag=false; 
  char opj[200];
  nFG=pd->BACnetFunctionalGroups;

  if((nFG&fgClock)||(nFG&fgTimeMaster))   
    if (pd->BACnetStandardObjects[DEVICE]==soNotSupported){  
      sprintf(opj,"EPICS Object Consistency check error: "
        "The Device Object type should be included to support the Clock|TimeMaster Functional Group!\n");
      //PrintToFile(opj);
      tperror(opj,false);
    }

  if((nFG&fgEventInitiation)||(nFG&fgEventResponse))     //Event Initiation/Response FG Obj Cons Check
  {
    if(pd->BACnetStandardServices[asConfirmedCOVNotification]||pd->BACnetStandardServices[asUnconfirmedCOVNotification]) //FG support COV Notification Services
      if(pd->BACnetStandardObjects[EVENT_ENROLLMENT]==soNotSupported){
        sprintf(opj,"EPICS Object Consistency check error: "
          "Event Enrollment Object type should be included to support the Event Initiation|Response Functional Group!\n");
        //PrintToFile(opj);
        tperror(opj,false);
      }
       //check if the FGs support intrinsic report
    if(pd->BACnetStandardServices[asConfirmedEventNotification]||pd->BACnetStandardServices[asUnconfirmedEventNotification])
      if(pd->BACnetStandardObjects[NOTIFICATIONCLASS])
      {
        if(pd->BACnetStandardObjects[ANALOG_INPUT]||pd->BACnetStandardObjects[ANALOG_OUTPUT]||pd->BACnetStandardObjects[ANALOG_VALUE])
          flag = true;
        else if(pd->BACnetStandardObjects[BINARY_OUTPUT]||pd->BACnetStandardObjects[BINARY_VALUE])
          flag = true;
        else if(pd->BACnetStandardObjects[MULTI_STATE_OUTPUT])
          flag = true;
        else if(pd->BACnetStandardObjects[LOOP])
          flag = true;
        else if(!flag) {
          sprintf(opj,"EPICS Object Consistency check error: "
            "at least one of Std Obj types listed by Table 13-2 should be included "
            "to support the Event Initiation|Response Functional Group! \n");
          //PrintToFile(opj);
          tperror(opj,false);
        }
      }
      else{
        sprintf(opj,"EPICS Object Consistency check error: "
          "Notification Class Object type should be included "
          "to support the Event Initiation|Response Functional Group! \n");
        //PrintToFile(opj); 
        tperror(opj,false);
      }
  }

  if(nFG&fgFiles){
    if(pd->BACnetStandardObjects[FILE_O]==soNotSupported){
      sprintf(opj,"EPICS Object Consistency check error: "
        "File Object type should be included to support the File Functional Group! \n");
      //PrintToFile(opj);
      tperror(opj,false);
    }
  }
}
*/

////////////////////////////////////////////////////////////////////////
//EPICS Object Types consistency check  added by xlp,2002-11
//Test K: Consistency check between Standard Object Types Supported Section
//and Object Types listed by Protocos_Object_types_Supported property
/*
void CheckObjConsK(PICSdb *pd)
{ char errMsg[300];
  char *objName;
  octet *p;
  BOOL  i;
  octet BACnetStandardObjChk[MAX_DEFINED_OBJ];
  octet ProtocolObjectSup[MAX_DEFINED_OBJ];

  p=ProtocolObjSup.PropSupValue;
  memset(ProtocolObjectSup,0,MAX_DEFINED_OBJ);
  BitToArray(ProtocolObjectSup,&ProtocolObjSup);
  for(i=0;i<MAX_DEFINED_OBJ;i++) {
    BACnetStandardObjChk[i]=pd->BACnetStandardObjects[i];
    if(((BACnetStandardObjChk[i]|ProtocolObjectSup[i])!=0) &&
       ((BACnetStandardObjChk[i]&ProtocolObjectSup[i])==0)) {
      objName=StandardObjects[i];
      sprintf(errMsg,"EPICS Object Consistency check error: "
        "%s object type listed by protocol-object-types-supported property "
        "does not have a one-to-one correspondence with that supported by the "
        "Standard Object Types Supported section!\n",objName);
      //PrintToFile(errMsg);
      tperror(errMsg,false);
    }
  }
}
*/
////////////////////////////////////////////////////////////////////////
//EPICS Object Types consistency check  added by xlp,2002-11
//Test L: Consistency check between Standard Object Types Supported Section
//and Test Database Section
/*
void CheckObjConsL(PICSdb *pd)
{ char errMsg[100];
  char *objName;
  octet BACnetStandardObjChk[MAX_DEFINED_OBJ];
  for(int i=0;i<MAX_DEFINED_OBJ;i++) {
    BACnetStandardObjChk[i]= pd->BACnetStandardObjects[i]; 
    if((BACnetStandardObjChk[i])&&!(ObjInTestDB[i].ObjIDSupported)) {
      objName=StandardObjects[i];
      sprintf(errMsg,"EPICS Object Consistency check error: "
        "%s Object missing in the test database!\n",objName);
      //PrintToFile(errMsg);
      tperror(errMsg,false);
    }
  }
}
*/
///////////////////////////////////////////////////////////////////////
//PICS Consistency Check m:
//Consistency check between the objects listed in the Object_List
//property of the Device object and objects included in the test
//database,both including proprietary objects.
/*
void CheckObjConsM()
{ octet num1,num2;
  int i,j; 
  char errMsg[200];
  char *objName;
  dword *p1,*p2;
  dword  instance1,instance2;

  for(i=0;i<MAX_DEFINED_OBJ;i++) {
    num1=ObjInTestDB[i].ObjInstanceNum;
    num2=DevObjList[i].ObjInstanceNum;
    if(num1!=num2) {
      objName=StandardObjects[i];
      sprintf(errMsg,"EPICS Object Consistency check error: "
        "A(n) %s object is mismatched or missing between "
        "the device object-list and the List of Objects!\n",objName);
      //PrintToFile(errMsg);
      tperror(errMsg,false);
    }
    else{
      p1=ObjInTestDB[i].ObjInstance;
      p2=DevObjList[i].ObjInstance;
      GetSequence(p1,p2,num1);
      for(j=0;j<num1;j++) {
        instance1=ObjInTestDB[i].ObjInstance[j];
        instance2=DevObjList[i].ObjInstance[j];
        if(instance1!=instance2) {
          objName=StandardObjects[i];
          sprintf(errMsg,"EPICS Object Consistency check error: "
            "%s %u in the device object-list does not match "
            "%s %u in the List of Objects!\n",objName,instance2,objName,instance1);
          //PrintToFile(errMsg);
          tperror(errMsg,false);
          break;
        }
      }
    }
  }
  return;
}
*/
///////////////////////////////////////////////////////////////////////
//simple Sequence algorithm,added by xlp,2002-11
void GetSequence(dword *p1,dword *p2,octet num)
{  int i,j;
   dword temp;
   for(i=0;i<num-1;i++)
	   for(j=0;j<num-i-1;j++){
		   if(*(p1+j)>=*(p1+j+1)){
			   temp=*(p1+j);*(p1+j)=*(p1+j+1);*(p1+j+1)=temp;
		   }
		   if(*(p2+j)>=*(p2+j+1)){
			   temp=*(p2+j);*(p2+j)=*(p2+j+1);*(p2+j+1)=temp;
		   }
	   }
	   return;
}
///////////////////////////////////////////////////////////////////////
//	parse a property value
//in:	pn		points to property name string
//		pobj	points to an object structure of type objtype
//		objtype	is the object type
//
//out:	true	if cancel selected

BOOL ParseProperty(char *pn,generic_object *pobj,word objtype)
{	propdescriptor *pd,*newpd;
	word			pindex,ub,i;
	void 		far	*pstruc;
	dword			dw, *dwp;
	char			b[512],q;
	octet			db,dm;
	etable 			*ep;
	float		far	*fp;
	word		far	*wp;
    //char        *p;  MAG 31JAN06 variable p not used
	char		**cp;
	octet		oParseType;// *op, MAG 31JAN06 variable op not used
	BACnetActionCommand **acp;
	BACnetDateRange		*dtp;

	void 		far	*pvalue_type;

	print_debug("PP: Enter ParseProperty, search for '%s' objtype %d\n",pn,objtype);	//MAG

	if (objtype <= etObjectTypes.propes)
	{
		pd = StdObjects[objtype].sotProps;	//point to property descriptor table for this object type
	}
	else
	{
		pd = ProprietaryObjProps;
	}
	
	pindex=0;
    do
	{	if (stricmp(pn,pd->PropertyName)==0)	//found this property name
		{	pstruc=(octet *)pobj+pd->StrucOffset;	//make pointer to where the value is stored
			
			//Save the property name which parse type is none. *************017
			if(pd->ParseType==none) strcpy(NoneTypePropName,pn);
			
			if(bNeedReset && bHasNoneType)
			{
				lp = &NoneTypeValue[0];
			}
			else
			{
				pn=strchr(pn,0);					//restore the ':' after the property name
				*pn=':';
			}

			skipwhitespace();					//point to where the value should be					***013 Begin
			print_debug("PP: pd->ParseType == %d\n",pd->ParseType); //MAG
			//Modified by Liangping Xu,2002-9
            //p=lp;
			//if(strchr(p,42)||strchr(p,63))
			if (*lp=='?'||*lp=='*')						//property value is unspecified
////////////////////////////////////////////////
			{	pobj->propflags[pindex]|=ValueUnknown;	//we don't know what the value is
				if(pd->ParseType == none) NoneTypeValue[0] = *lp;   //added for storeing the value of property,********017
				lp++;							//skip ?												***014
			}
			else								//has a property value									***013 End
			{
				oParseType= pd->ParseType;
				if(bNeedReset & bHasNoneType)
				{
					oParseType = newParseType;
					pd->PropET = newPropET;
					bHasReset = true;
				}

				switch(oParseType)
				{
				case evparm:					//event parameter
					if (ParseEventParameter((BACnetEventParameter *)pstruc)) return true;
					break;
				case setref:					//setpoint reference
					//note: we can't tell the difference between a ParseReference which had an error
					//		and returned NULL, vs. an intentional *no reference* which also returns
					//		NULL.
					*(BACnetObjectPropertyReference **)pstruc=ParseReference(NULL);	//				***012
//					ParseRefList((BACnetObjectPropertyReference **)pstruc);
					break;
				case propref:					//obj property reference
					ParseReference((BACnetObjectPropertyReference *)pstruc);
					break;

				//////Modified by Liangping Xu,2002-9////
				case devobjpropref:					//obj property reference
                    ParseDevObjPropReference((BACnetDeviceObjectPropertyReference *)pstruc);
					break;
				case lodoref:
					ParseDevObjList((BACnetDeviceObjectReference **)pstruc, (word) pd->PropID );
					break;
				case recip:						//recipient
					ParseRecipient((BACnetRecipient *)pstruc);						//				***012
					break;
				case skeys:						//session keys
					if (ParseSessionKeyList((BACnetSessionKey **)pstruc)) return true;
					break;
				case wsched:					//weekly schedule: array[7] of list of timevalue
					if (ParseWeekdaySchedule((BACnetTimeValue **)pstruc)) return true;
					break;
				case xsched:					//exception schedule: array[] of specialevent
					if (ParseExceptionSchedule((BACnetExceptionSchedule *)pstruc)) return true;
					break;
				case reciplist:					//list of BACnetDestination
					if (ParseDestinationList((BACnetDestination **)pstruc)) return true;
					break;
				case tsrecip:					//time synch recipients
					if (ParseRecipientList((BACnetRecipient **)pstruc)) return true;
					break;
				case dabind:					//device address bindings
					if (ParseAddressList((BACnetAddressBinding **)pstruc)) return true;
					break;
				case raslist:					//read address specifications
					if (ParseRASlist((BACnetReadAccessSpecification **)pstruc)) return true;
					break;
				case calist:					//list of calendarentry
					if (ParseCalist((BACnetCalendarEntry **)pstruc)) return true;
					break;
				case looref:					//list of object ids
					if (ParseObjectList(((BACnetObjectIdentifier **)pstruc),&((device_obj_type *)pobj)->num_objects)) return true;
					break;
				case lopref:					//list of objectpropertyreferences
					
					if (ParseRefList((BACnetDeviceObjectPropertyReference **)pstruc)) return true;
		
					//Adde for resetting value of present-value, **********017
					if(objtype == 0x11)  //object type is schedule
					{
						BACnetDeviceObjectPropertyReference *temp;
						temp = *(BACnetDeviceObjectPropertyReference **)pstruc;
						if (temp==NULL) break;
						dword refobjid = temp->Objid;
						dword refpropid = temp->wPropertyid;
						newpd=StdObjects[refobjid>>22].sotProps;			//point to property descriptor table for this object type
						do
						{	
							if (refpropid == newpd->PropID)
							{
								newPropET = newpd->PropET;
								newParseType = newpd->ParseType;
								
								pvalue_type= (octet*)pobj+ (size_t)((char *)&(((schedule_obj_type *)0)->value_type) - (char *)0 );
								*(dword *)pvalue_type = newParseType;

								bNeedReset = true;
								break;
							}
							if (newpd->PropGroup&Last)
								return tperror("Invalid Property Name- Check Spelling",true); 
							newpd++;									//advance to next table entry
						}while(true);
					}
					break;
				case vtcl:						//vt classes
					if (ParseVTClassList((BACnetVTClassList **)pstruc)) return true;
					break;
				case pss:						//protocol services supported bitstring
               // msdanner 9/2004 - now handles bitstrings up to MAX_BITSTRING,
               // regardless of the number of defined protocol services
					if (ParseBitstring((octet *)pstruc,
					    MAX_BITSTRING,                                  /* max bits to parse */
					    &EPICSLengthProtocolServicesSupportedBitstring)) /* how many bits were parsed? */
					    return true;
               EPICSLengthProtocolServicesSupportedBitstring++;  // zero-based correction
					ProtocolServSup.ObjServNum=sizeof(StandardServices)/sizeof(StandardServices[0]);   
					ProtocolServSup.PropSupValue=(octet *)pstruc; 
					
					break;
				case pos:						//protocol objects supported bitstring
               // msdanner 9/2004 - now handles bitstrings up to MAX_BITSTRING,
               // regardless of the number of defined protocol object types
					if (ParseBitstring((octet *)pstruc,
					    MAX_BITSTRING,                                  /* max bits to parse */
					    &EPICSLengthProtocolObjectTypesSupportedBitstring)) /* how many bits were parsed? */
					    return true;
               EPICSLengthProtocolObjectTypesSupportedBitstring++;  // zero-based correction
					ProtocolObjSup.PropSupValue=(octet *)pstruc;       
					ProtocolObjSup.ObjServNum=sizeof(StandardObjects)/sizeof(StandardObjects[0]);   
					break;
				case dt:						//date/time
					if (ParseDateTime((BACnetDateTime *)pstruc)) return true;
					break;
                //** Modified by Liangping Xu,2002-9
				case dtrange:					//daterange
					skipwhitespace();
					if ( *lp == '(' && *(lp+1) == '(' )
						lp++;  // if surrounded by opening brackets skip
	                 print_debug("PD:  ParseDateRange starts now\n");	
					dtp=(BACnetDateRange *)pstruc;
					if (ParseDate(&dtp->start_date)) return true;
                    if(*lp=='.'&&*(lp+1)=='.')     
					    lp=strstr(lp,"..")+2;
                    else
					{	
						skipwhitespace();
						if ( *lp=='-' )
							lp=strstr(lp,"-")+1;
						else
							if (MustBe(',')) return true;
					}
					 skipwhitespace();
	                 print_debug("PD: about to second first date\n");                        
					if (ParseDate(&dtp->end_date)) return true;
					if ( *lp == ')' )
						lp++;   // skip last close if its there
					break;
				case ddate:
					skipwhitespace();
					if (ParseDate((BACnetDate *)pstruc)) return true;
					break;
				case ttime:
					skipwhitespace();
					if (ParseTime((BACnetTime *)pstruc)) return true;
					break;
				case act:						//action array
					acp=(BACnetActionCommand **)pstruc;
					// Here, the PropET table property gives the number of array entries
					assert( pd->PropET > 0 );
					for (i=0; i<pd->PropET; i++) 
						acp[i]=NULL;	//init all slots to NULL values ***011
					
					i=0;
					if ((lp=openarray(lp))==NULL) return true;
					
					while (i < pd->PropET)	//									***008 Begin
					{	
						if ((acp[i]=ReadActionCommands())==NULL) return true;	//failed for some reason	
						i++;					//									***011 Begin
						while (*lp==space||*lp==',') lp++;	//skip separation between list elements
						if (*lp==0) 
							if (ReadNext()==NULL) break;
						if (*lp=='}') 
						{	lp++;
							break;				//close this array out
						}						//									***011 End
					}
					((command_obj_type *)pobj)->num_actions=i;
					break;
				case eboollist:         // list of booleans
					if (ParseBooleanList( (BooleanList **)pstruc )) return true;
					break;
				case stavals:			// list of unsigned
					if (ParseUnsignedList( (UnsignedList **)pstruc )) return true;
/*
					//list of states					***006 Begin
					op=(octet *)pstruc;
					for (i=0;i<32;i++) op[i]=0;	//									***011
					i=0;
					skipwhitespace();
					if (MustBe('(')) return true;
					
					while (*lp&&i<32)
					{	if ((db=ReadB(1,32))!=dontcare) op[i]=db;
						i++;					//									***011 Begin
						if(*(lp-1) == ')') lp--;  // MAG
						while (*lp==space||*lp==',') lp++;	//skip separation between list elements
						if (*lp==0) 
							if (ReadNext()==NULL) break;
						if (*lp==')') 
						{	lp++;
							break;				//close this array out
						}						//									***011 End
					}
*/
					break;
				case statext:					//state text array
				case actext:					//action_text array
					cp=(char **)pstruc;
					// Here, the PropET table property gives the number of array entries
					assert( pd->PropET > 0 );
					for (i=0; i < pd->PropET; i++) 
						cp[i]=NULL;	//init all slots to NULL values	***011
					i=0;
					if ((lp=openarray(lp))==NULL) return true;
					// Allow empty first line, after the bracket
					if (*lp==0) 
						if (ReadNext()==NULL) break;
					
					while (*lp && (i < pd->PropET) )
					{	
						// Allow up to 80 chars (was 32) since b[] is 512 bytes, and
						// we'll malloc the string storage below to fit the actual size.
						if (setstring(b,80,lp)) return true;	//put string in buffer b
						if (b[0])				//if string isn't null
						{	ub=strlen(b)+1;   //reqd length
							if ((cp[i]=(char *)malloc(ub))==NULL)
							{	tperror("Can't Get String Space!",true);
								return true;
							}
							strcpy(cp[i],b);	//copy it
							print_debug("LJT: action_text=%x",cp[i]);

						}
						i++;					//									***011 Begin
						while (*lp==space||*lp==',') lp++;	//skip separation between list elements
						if (*lp==0) 
							if (ReadNext()==NULL) break;
						if (*lp=='}') 
						{	lp++;
							break;				//close this array out
						}						//									***011 End
					}
					break;
				case pae:						// priority array enums
				case pab:						//priority array bpv
					wp=(word *)pstruc;
					for (i=0;i<16;i++) wp[i]=bpaNULL; //init all slots to NULL values	***011
					i=0;
					if ((lp=openarray(lp))==NULL) return true;
					
					while (*lp&&i<16)
					{	ep=AllETs[pd->PropET]; //point to enumeration table for this guy
						if ((ub=ReadEnum(ep))==0xFFFF)
						{	tperror("Invalid Enumeration",true);
							return true;
						}
						*wp++=ub;
						i++;
						lp--;					//need to point to delimiter		***011 Begin
						while (*lp==space||*lp==',') lp++; //skip separation between list elements
						if (*lp==0) 
							if (ReadNext()==NULL) break;
						if (*lp=='}') 
						{	lp++;
							break;				//close this array out
						}						//									***011 End
					}
					break;
				case pau:						//priority array uw
					wp=(word *)pstruc;
					for (i=0;i<16;i++) wp[i]=upaNULL; //init all slots to NULL values	***011
					i=0;
					if ((lp=openarray(lp))==NULL) return true;
					
					while (*lp&&i<16)
					{	if (*lp=='n'||*lp=='N')	//we'll assume he means 'NULL'
						{	*wp++=upaNULL;		//									***013 Begin
							while (*lp&&*lp!=space&&*lp!=','&&*lp!='}') lp++; //skip rest of NULL ***014
						}						//									***013 End
						else					//we'll assume there's a number here
						{
							*wp++=ReadW();
							if ( *(lp-1)=='}' ) lp--;  // put back last read char.
						}
						i++;					//									***011 Begin
						while (*lp==space||*lp==',') lp++; //skip separation between list elements
						if (*lp==0) 
							if (ReadNext()==NULL) break;
						if (*lp=='}') 
						{	lp++;
							break;				//close this array out
						}						//									***011 End
					}
					break;
				case ptPai:						//priority array int32
					dwp=(dword *)pstruc;
					for (i=0;i<16;i++) dwp[i]=upaNULL; //init all slots to NULL values	***011
					i=0;
					if ((lp=openarray(lp))==NULL) return true;
					
					while (*lp&&i<16)
					{	if (*lp=='n'||*lp=='N')	//we'll assume he means 'NULL'
						{	*dwp++=upaNULL;		//									***013 Begin
							while (*lp&&*lp!=space&&*lp!=','&&*lp!='}') lp++; //skip rest of NULL ***014
						}						//									***013 End
						else					//we'll assume there's a number here
						{
							*dwp++=ReadDW();
							if ( *(lp-1)=='}' ) lp--;  // put back last read char.
						}
						i++;					//									***011 Begin
						while (*lp==space||*lp==',') lp++; //skip separation between list elements
						if (*lp==0) 
							if (ReadNext()==NULL) break;
						if (*lp=='}') 
						{	lp++;
							break;				//close this array out
						}						//									***011 End
					}
					break;
				case paf:						//priority array flt
					fp=(float *)pstruc;
					for (i=0;i<16;i++) fp[i]=fpaNULL; //init all slots to NULL values	***011
					i=0;						//									***011
					if ((lp=openarray(lp))==NULL) return true;
					
					while (*lp&&i<16)
					{
						while(*lp==space){lp++;       // skip the space **add by xlp**
						}                         
						if (*lp=='n'||*lp=='N')	//we'll assume he means 'NULL'
						{	*fp++=fpaNULL;		//									***013 Begin
							while (*lp&&*lp!=space&&*lp!=','&&*lp!='}') lp++; //skip rest of NULL ***014
						}						//									***013 End
						else					//we'll assume there's a number here
							*fp++=(float)atof(lp);
						i++;					//									***011 Begin
						if ((strdelim(",}"))==NULL) break;
						if (lp[-1]=='}') break;	//									***011 End
					}
					break;
				case ob_id:						//an object identifier
					if ((dw=ReadObjID())==badobjid) return true;
					*(dword *)pstruc=dw;
					break;
				case ebool:						//boolean value 					***006 Begin
					*(octet *)pstruc=ReadBool();	//							***012
					break;						//									***006 End
				case etl:
					ep=AllETs[pd->PropET];
					if (ParseEnumList((BACnetEnumList **)pstruc, ep )) return true;
					break;
				case et:						//an enumeration table
					ep=AllETs[pd->PropET];	//point to enumeration table for this guy
					if ((ub=ReadEnum(ep))==0xFFFF)
					{	tperror("Invalid Enumeration",true);
						return true;
					}
					*(word *)pstruc=ub;
					break;
				case bits:						//octet of T or F flags
					db=0;
					dm=0x80;
					while (q=*lp++)
					{	if (q==')') break;
						if (q==',') dm>>=1;
						if (q=='t'||q=='T') db|=dm;
					}
					*(octet *)pstruc=db;
					break;
				case sw:						//signed word
					*(int *)pstruc=atoi(lp);
					break;
				case ssint:						// case added  by Liangping Xu, 2002-8-5
					*(int*)pstruc =(int)atoi(lp);	
					break;
				case flt:						//float
					*(float *)pstruc=(float)atof(lp);
					while(*lp!=0&&*lp!=space&&*lp!=',') 
						lp++;  // now move past word we just read;
					break;
				case uwarr:
				case uw:						//unsigned word
				//Note: this is a hack to handle the case of NotificationClass Priority array (3 unsigned words)
					if (pd->PropFlags&IsArray)	//this is an array of words			***012 Begin
					{	wp=(word *)pstruc;
						i=0;
						if ((lp=openarray(lp))==NULL) return true;
						
						while (*lp&&i<3)		//no more than 3
						{	*wp++=ReadW();
							i++;
							if(*(lp-1) == '}') lp--;  // MAG  error fix for priority object
							while (*lp==space||*lp==',') lp++;	//skip separation between list elements
							if (*lp==0) 
								if (ReadNext()==NULL) break;
							if (*lp=='}') 
							{	lp++;
								break;			//close this array out
							}
						}
					}
					else						//just a word						***012 End
						*(word *)pstruc=ReadW();
					break;
				case ud:						//unsigned dword
				case ptInt32:					// signed long
					*(dword *)pstruc=ReadDW();
					break;
				case u16:						//1..16
					ub=16;
					goto ppub;
				case u127:						//1..127
					ub=127;
	ppub:			dw=ReadDW();
					if (dw==0||dw>ub)			//it's out of range
					{	sprintf(b,"Must use an Unsigned Integer 1..%u here!",ub);
						if (tperror(b,true)) return true;
						dw=(dword) ub;			//stick at upper bound
					}
					*(word *)pstruc=(word)dw;
					break;
				case s10:						//char [10]
					ub=10;
					goto ppstub;
				case s32:						//char [32]
					ub=32;
					goto ppstub;
				case s64:						//char [64]
					ub=64;
					goto ppstub;
				case s132:						//char [132]
					ub=132;
	ppstub:         if (setstring((char *)pstruc,ub,lp)) return true;
                    break;
                case LOGREC:
                    //if (ParseLogRec((BACnetLogRecord *)pstruc)) return true;
                 	break;
                case TSTMP:
					if (ParseTimeStamp((BACnetTimeStamp *)pstruc)) return true;
					break;
				case TSTMParr:		// madanner 9/04
					{						
						if( ParseTimeStampArray((BACnetTimeStamp **)pstruc, 3) ) 
							return true;
					}
					break;
				// *****018  begin
				case lCOVSub:		//List of BACnetCOVSubcription
					if (ParseCOVSubList((BACnetCOVSubscription **)pstruc)) return true;
					break;
				//*****018  end
				case escale: //BACnetScale Shiyuan Xiao
					{
						BACnetScale* pscale = (BACnetScale *)pstruc;
						float val =(float)atof(lp);
						if(val > (float)((int)val))
						{
							pscale->choice = 0;
							pscale->u.floatScale = val;
						}
						else
						{
							pscale->choice = 1;
							pscale->u.integerScale = (int)val;
						}
							
					}					
					break;
				case eprescl: //BACnetPrescale Shiyuan Xiao
					if (ParsePrescale((BACnetPrescale *)pstruc))
						return true;
					break;
				case eaclr: //BACnetAccumulatorRecord Shiyuan Xiao
					if (ParseAccumulatorRecord((BACnetAccumulatorRecord *)pstruc))
						return true;
					break;
				case none:
					// reset the datatype of present value according to EPICS,  by Yiping XU, 2002/11/2
						bHasNoneType = true;
						for(i =0; ;i++)
						{
							if(*lp==0) break;
							NoneTypeValue[i] = *lp;
							lp++;
						}
					break;
				case shedlevel:
					{
						BACnetShedLevel* pshedlevel = (BACnetShedLevel *)pstruc;
						pshedlevel->choice = -1;
						if (*lp == '[')
						{
							lp++;
							pshedlevel->choice = (unsigned char)ReadDW();
							// lp++;  // skip over ']'
						}
						float val =(float)atof(lp);
						while(*lp!=0&&*lp!=space&&*lp!=',') 
							lp++;  // now move past word we just read;
						if (pshedlevel->choice == -1)
						{
							if(val > (float)((int)val))
							{
								pshedlevel->choice = 0;
								pshedlevel->sl.uproperty_value = (int)val;
							}
							else
							{
								pshedlevel->choice = 2;
								pshedlevel->sl.fproperty_value = val;
							}
						}	
						else
						{
							if (pshedlevel->choice == 2)
								pshedlevel->sl.fproperty_value = val;
							else
								pshedlevel->sl.uproperty_value = (int)val;
						}
					}					
					break;
				default:
				//Note:	If we get here it is always an error because the default case means that
				//		this is not a supported parsetype and therefore must always be ?,i.e. an
				//		unknown value. As of ***013, ? values are handled before we get here, so
				//		if we got here then something other than ? was specified as the value.
					if (MustBe('?')) return true;
				}

				//****************017
				if(bHasReset)
				{
					pd->ParseType = none;
					pd->PropET = 0;
					bHasReset = false;
					bNeedReset = false;
					bHasNoneType =false;
				}

			}
			pobj->propflags[pindex]|=PropIsPresent;	//set the bit for this property	***013 Begin
			while (*lp==space||*lp==',') lp++;		//skip any more whitespace or commas ***014
			if (*lp=='W'||*lp=='w')				//prop is writeable
				pobj->propflags[pindex]|=PropIsWritable;	//						***013 End
			print_debug("PP: RETURN\n");
			return false;						//we're done parsing
		}
		if (pd->PropGroup&Last)
		{
			if (objtype <= etObjectTypes.propes)
			{
				return tperror("Invalid Property Name- Check Spelling",true); 
			}
			else
			{
				//ignore unknown property of proprietary objects
				return false;
			}
		}
		pd++;									//advance to next table entry
		pindex++;
	}
	while(true);
}

///////////////////////////////////////////////////////////////////////
//	read a BACnetEventParameter from the buffer lp points to
//
//note: (bitstring) may be B'101010' or (T,F...)
//
//	(timedelay,(bitmask),(bitstring),(bitstring)... or
//	(timedelay,state,state,state...) or
//	(timedelay,(bitstring)) or (timedelay,refpropincrement) or
//	(timedelay,((objtype,instance),propname)) or
//	(timedelay,((objtype,instance),propname),lowdiff,hidiff,deadband) or
//	(timedelay,low,hi,deadband)
//in:	lp		points to current position in buffer lb
//		evp		points to the BACnetEventParameter to be filled in
//		Note:	evp->event_type MUST be initialized before calling this function!
//out:	true	if an error occurred
//		lp		points past the delimiter ) unless it was the end of the buffer

BOOL ParseEventParameter(BACnetEventParameter *evp)
{	BACnetListBitstringValue 	*pbv=NULL, *qbv=NULL;
	BACnetPropertyStates 		*pv=NULL, *qv=NULL;

	skipwhitespace();
	if (MustBe('(')) return true;
	skipwhitespace();
	switch(evp->event_type)
	{
	case CHANGE_OF_BITSTRING:					//0
		evp->time_delay=ReadW();
		skipwhitespace();
		if (ParseBitstring(&evp->bitmask.bitstring_value[0],
							sizeof(evp->bitmask.bitstring_value)*8,
							&evp->bitmask.bitstring_length)) return true;
		if (MustBe(',')) return true;
		evp->list_bitstring_value=NULL;			//no bitstrings initially
		while(feof(ifile)==0)
		{   while (*lp==space||*lp==',') lp++;	//skip separation between list elements
			if (*lp==0)
				if (ReadNext()==NULL) break;	//									***008
			if (*lp==')') break;				//close this list out
			//here we have (bitstring)...
			if ((qbv=(tagListBitstringValue *)malloc(sizeof(BACnetListBitstringValue)))==NULL)
			{	tperror("Can't Get Object Space!",true);
				break;
			}
			print_debug("LJT: ListBitstringValue=%x",qbv);

			if (ParseBitstring(&qbv->bitstring_value[0],
							sizeof(qbv->bitstring_value)*8,
							&qbv->bitstring_length)) break;
			qbv->next=NULL;							//link onto the list
			if (evp->list_bitstring_value==NULL)
				evp->list_bitstring_value=qbv;		//remember first guy we made
			else
				pbv->next=qbv;					//link new guy on the end of the list
			pbv=qbv;							//remember new guy is now the last guy
			qbv=NULL;
		}
		if (qbv!=NULL) free(qbv);
		break;
	case CHANGE_OF_STATE:						//1
		evp->time_delay=ReadW();
		evp->list_of_value=NULL;				//no values initially
		while(feof(ifile)==0)
		{   while (*lp==space||*lp==',') lp++;	//skip separation between list elements
			if (*lp==0)
				if (ReadNext()==NULL) break;	//									***008
			if (*lp==')') break;				//close this list out
			//here we have state,state...
			if ((qv=(tagPropertyStates *)malloc(sizeof(BACnetPropertyStates)))==NULL)
			{	tperror("Can't Get Object Space!",true);
				break;
			}
			print_debug("LJT: PropertyStates=%x",qv);

			qv->enum_value=ReadW();
			qv->next=NULL;						//link onto the list
			if (evp->list_of_value==NULL)
				evp->list_of_value=qv;			//remember first guy we made
			else
				pv->next=qv;					//link new guy on the end of the list
			pv=qv;								//remember new guy is now the last guy
			qv=NULL;
			if (lp[-1]==')') lp--;
		}
		if (qv!=NULL) free(qv);
		break;
	case CHANGE_OF_VALUE:						//2
		evp->time_delay=ReadW();
		skipwhitespace();
		if (*lp=='('||*lp=='B'||*lp=='b')		//cov-criteria is a bitmask
		{	evp->use_property_increment=false;
			if (ParseBitstring(&evp->bitmask.bitstring_value[0],
								sizeof(evp->bitmask.bitstring_value)*8,
								&evp->bitmask.bitstring_length)) return true;
		}
		else									//cov-criteria is a property-increment
		{	evp->use_property_increment=true;
			evp->ref_property_increment=(float)atof(lp);
			if ((strdelim(")"))==NULL) return true;
			lp--;
		}
		break;
	case COMMAND_FAILURE:						//3
		evp->time_delay=ReadW();
		ParseReference(&evp->feed_prop_ref);
		break;
	case FLOATING_LIMIT:						//4
		evp->time_delay=ReadW();	
		ParseReference(&evp->setpoint_ref);
		if (MustBe(',')) return true;
		evp->low_diff_limit=(float)atof(lp);
		if ((strdelim(","))==NULL) return true;
		evp->high_diff_limit=(float)atof(lp);
		if ((strdelim(","))==NULL) return true;
		evp->deadband=(float)atof(lp);
		if ((strdelim(")"))==NULL) return true;
		lp--;
		break;
	case OUT_OF_RANGE:							//5
		evp->time_delay=ReadW();
		evp->low_limit=(float)atof(lp);
		if ((strdelim(","))==NULL) return true;
		evp->high_limit=(float)atof(lp);
		if ((strdelim(","))==NULL) return true;
		evp->deadband=(float)atof(lp);
		if ((strdelim(")"))==NULL) return true;
		lp--;
		break;
//Modified by Zhu Zhenhua, 2004-5-20
	case BUFFER_READY:						//10
		evp->notification_threshold=ReadW();
		if (*(lp-1) == ',')
			lp--;  // backup one we went too far // todo: really need to fix ReadW
		if ((strdelim(","))==NULL) return true;
		evp->previous_notification_count=ReadDW();
		if (*(lp-1) == ')')
			lp--;  // backup one we went too far // todo: really need to fix ReadW
		if ((strdelim(")"))==NULL) return true;
		lp--;
		break;
	}
	return MustBe(')');
}

// LJT 10/10/2005 -- added to reduce code redundancy
// Return a pointer to the propdescriptor for the property, or NULL if none found
// Assumes either
// - "property-identifier,index" if an index is specified, or
// - "property-identifier," if no index is specifed (i.e., not an array, or "all elements)
// See the NOTE at the top of this file for a discussion of this encoding, with
// the comma REQUIRED to encode the presence or absence of the optional index
//
// Called from
// - ParseRASlist (BACnetReadAccessSpecifications) - all-elements is quite legal
// - ParseReference (BACnetObjectPropertyReference) - all-elements is unlikely
// - ParseDevObjPropReference (BACnetDeviceObjectPropertyReference) - all-elements is unlikely
// - ReadActionCommands (BACnetActionCommand) - all-elements is unlikely
//
propdescriptor* validatePropertyNameAndIndexCode(dword dw, unsigned long *propId, unsigned short *index)
{
	char				pn[40];
	propdescriptor		*pd;

	// isolate property name
	int i=0;
	while(*lp&&*lp!=','&&*lp!=space&&*lp!=')'&&*lp!='}'&&*lp!='['&&i<39)
	{	
		if (*lp=='_') *lp='-';				//change _ to -
		pn[i++]=*lp++;
	}
	pn[i]=0;								//force it to be asciz

	pd=StdObjects[(word)(dw>>22)].sotProps;	//point to property descriptor table for this object type
	do
	{	
		if (stricmp(pn,pd->PropertyName)==0)	//found this property name
		{	
			*propId=pd->PropID;
		    break;
		}
		if (pd->PropGroup&Last)
		{	
			tperror("Unknown Property Name",true);
			return NULL;  // return NOT FOUND
		}
		pd++;								//advance to next table entry
	}
	while(true);

	*index = NotAnArray;

   	skipwhitespace();
	if (*lp == ',')
	{
		// Comma must be followed by an index, or by another separator
		// if the index is omitted
		++lp;
		skipwhitespace();
		int hasIndex = IsDigit( *lp );
		if (hasIndex)
		{
			*index = ReadW();
			if (*(lp-1) == ')' || *(lp-1) == '}' || *(lp-1) == ',')
				lp--;  // backup one we went too far // todo: really need to fix ReadW
		
			// It is (at least in some cases) OK to specify an array without an index
			// to denote the entire array.
			// It is NOT OK to specify an index for a non-array
			if ((pd->PropFlags & IsArray) == 0)
			{
				tperror("Index specified for non-array property",true);
				return NULL;  // error:
			}
		}
	}
	
	return pd;
}

///////////////////////////////////////////////////////////////////////				***008 Begin
//	read a list BACnetReadAccessSpecifications from the buffer lp points to
//	  (((objtype,instance),propid,,propid,arindex),((objtype,instance),propid,,propid,),((objtype,instance),propid,,propid,)...)
//  See the NOTE at the top of this file for a discussion of this encoding, with
//  the comma REQUIRED to encode the presence or absence of the optional index
//in:	lp		points to current position in buffer lb
//		rasp	points to a variable which should point to a list of BACnetReadAccessSpecifications
//out:	true	if an error occurred
//		else	*rasp points to a list of BACnetReadAccessSpecifications
//		lp		points past the delimiter ) unless it was the end of the buffer

BOOL ParseRASlist(BACnetReadAccessSpecification **rasp)
{	
	BACnetReadAccessSpecification	*fp=NULL,*p=NULL,*q=NULL;
	BACnetPropertyReference			*pp=NULL, *pq=NULL;
	dword				dw;
	BOOL				rasfail=true;				
				
	*rasp=NULL;									//initially there is no list
	skipwhitespace();
//	if (MustBe('{')) return true;  //MAG change from '('
	if (MustBe('(')) return true;  
	while(true)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//   Note that we require "empty" list elements to use proper syntax (...),(),(...)
		//   but (...),,(...) is treated the same as (...),(...)
		//2. (	i.e. the beginning of a new BACnetReadAccessSpecification in the list
		//3. )				i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0) 
			if (ReadNext()==NULL) break;
		//if (*lp=='}') // MAG was )
		if ( *lp==')' && *(lp+1)== 0 )
		{	lp++;
			break;								//close this list out
		}
		//if (MustBe('(')) goto rasx;
		if (MustBe('{')) goto rasx; // 135.1

		//here we have (objtype,instance),propid,propid...)...
		if ((q=(tagReadAccessSpecification *)malloc(sizeof(BACnetReadAccessSpecification)))==NULL)
		{	tperror("Can't Get Object Space!",true);
			goto rasx;
		}
		print_debug("LJT: readAccessSpecification=%x",q);

		dw=ReadObjID();
		if (dw==badobjid)
		{	if (tperror("Must use an Object Identifier here!",true))
				goto rasx;
		}
		q->object_id=dw;
		if ((strdelim(","))==NULL) goto rasrem;
		q->list_of_prop_ref=NULL;
		pp=NULL;
		//----------------------------------------------------
		while(feof(ifile)==0)
		{
			while (*lp==space||*lp==',') lp++;		//skip separation between list elements
			if (*lp==0) 
				if (ReadNext()==NULL) break;
			if (*lp==')') 
			{	//lp++;  //MAG
				lp +=2;  // MAG
				break;								//close this list out
			}
			if ((pq=(tagPropertyReference *)malloc(sizeof(BACnetPropertyReference)))==NULL)
			{	tperror("Can't Get Object Space!",true);
				goto rasx;
			}
			print_debug("LJT: PropertyReference=%x",pq);

			if(*lp == '(') lp++; // MAG added to skip leading '('

			// validate property name and optional index code here
			if (validatePropertyNameAndIndexCode(dw, &(pq->property_id), &(pq->pa_index) ) == NULL)
				goto rasx;  // error end here

			pq->next=NULL;							//link onto the list
			if (q->list_of_prop_ref==NULL)
				q->list_of_prop_ref=pq;				//remember first guy we made
			else
				pp->next=pq;						//link new guy on the end of the list
			pp=pq;									//remember new guy is now the last guy
			pq=NULL;
        }

		//if (lp[-1]!=')')
		if (lp[-1]!='}') //135.1
rasrem:	{	tperror("Expected ) here!",true);
			goto rasx;
		}
		q->next=NULL;							//link onto the list
		if (fp==NULL)
			fp=q;								//remember first guy we made
		else
			p->next=q;							//link new guy on the end of the list
		p=q;									//remember new guy is now the last guy
		q=NULL;
	}
	rasfail=false;
rasx:
	*rasp=fp;
	if (q!=NULL) free(q);						//don't lose this block!
	if (pq!=NULL) free(pq);					//don't lose this block!
	return rasfail;								//									***008 End
}

///////////////////////////////////////////////////////////////////////				***008 Begin
//	read an array of (BACnetSpecialEvent) from the buffer lp points to
//	{((t,v),(t,v)),((t,v),(t,v)),...}
//in:	lp		points to current position in buffer lb
//		xp		pointer to exception schedule to be initialized
//out:	true	if an error occurred
//		lp		points past the delimiter ) unless it was the end of the buffer

BOOL ParseExceptionSchedule(BACnetExceptionSchedule *xp)
{	BACnetSpecialEvent	*fp=NULL,*p=NULL,*q=NULL;
	BOOL				xfail=true;
	char				*ep;
	word				i;

	xp->special_event=NULL;						//initially there are no lists
	xp->size=0;
	
	skipwhitespace();
	if (*lp=='?') return false;					//? means no list
	if (MustBe('{')) return true;
	while(feof(ifile)==0)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//   Note that we require "empty" list elements to use proper syntax (...),(),(...)
		//   but (...),,(...) is treated the same as (...),(...)
		//2. (			i.e. the beginning of a new BACnetSpecialEvent in the array
		//3. )			i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0)
			if (ReadNext()==NULL) return true;
		if (*lp=='}') 
		{	lp++;
			break;								//close this list out
		}
        
		if (MustBe('(')) goto xx;
		//here we have (dateoption),(timevaluelist),eventpriority),...)
		if ((q=(tagSpecialEvent *)malloc(sizeof(BACnetSpecialEvent)))==NULL)
		{	tperror("Can't Get Object Space!",true);
			goto xx;
		}
		print_debug("LJT: SpecialEvent=%x\n",q);

		skipwhitespace();
		if (MustBe('(')) goto xx;
		if ((ep=strchr(lp,')'))==NULL)
		{	lp=strchr(lp,0);
			tperror("Expected ) here!",true);
			goto xx;
		}
		//here, lp points to one of 4 formats:
		//	Calendar,instance
		//	month,weekofmonth,dayofweek
		//	date..date
		//	date
		//
		//where date is in one of two formats:
		//	month/day/year dow			i.e. month is numeric
		//	day-mon-year dow			i.e. month is text like Jan
		//in either case dow must be separated from year by a space, or not be there at all
		//The .. between dates in a daterange is significant and must be literally 2 dots in a row
		
		if (strnicmp(lp,"Calendar",8)==0)		//it's a calendar reference
		{	q->choice=3;
			lp--;								//ReadObjID needs to point to the (
			q->u.calendar_ref.object_id=ReadObjID();
			q->u.calendar_ref.next=NULL;
		}
		else
		{	*ep=0;								//remember this end pointer
			if (strchr(lp,',')!=NULL)			//must be WeekNDay
			{	q->choice=2;					//WeekNDay
				skipwhitespace();
				q->u.weekNday.month=dontcare;
				q->u.weekNday.day=dontcare;
				q->u.weekNday.week=dontcare;
				if (IsDigit(*lp))				//use numeric form of month
				    q->u.weekNday.month=ReadB(1,12);
				else							//use monthname
			    {	for (i=0;i<14;i++)
			    		if (strnicmp(lp,MonthNames[i],3)==0)
			    		{	q->u.weekNday.month=(octet)i+1;	//months are 1-12
			    			break;
			    		}
			    	strdelim(",");				//skip past comma
				}
				if (*lp=='l'||*lp=='L')			//Last week
				{	q->u.weekNday.week=6;
					strdelim(",");
				}
				else
					q->u.weekNday.week=ReadB(1,6);
				if (IsDigit(*lp))				//use numeric form of dayofweek		***013 Begin
					q->u.weekNday.day=ReadB(1,7);
				else
			    {	for(i=0;i<7;i++)
			    		if (strnicmp(lp,DOWNames[i],3)==0)
			    		{	q->u.weekNday.day=(octet)i+1;		//days are 1-7
			    			break;
			    		}
			    	lp+=3;
			    }								//									***013 End
			}
			else if (strstr(lp,"..")!=NULL)	//must be daterange
			{	q->choice=1;					//DateRange
				if (ParseDate(&q->u.date_range.start_date)) goto xx;
				lp=strstr(lp,"..")+2;			//skip delimiter
				if (ParseDate(&q->u.date_range.end_date)) goto xx;
			}
			else								//must be date
			{	q->choice=0;					//Date
				if (ParseDate(&q->u.date)) goto xx;
			}
			lp=ep;
			*lp++=')';
		}
		print_debug("PES: About to strdelim lp = '%s'\n",lp);
		if (strdelim(",")==NULL) goto xx;

		print_debug("PES: About to PTVL lp = '%s'\n",lp);
		if (ParseTVList(&q->list_of_time_values)) goto xx;
		
		print_debug("PES: About to strdelim2 lp = '%s'\n",lp);
		if (strdelim(",")==NULL) goto xx;

		print_debug("PES: about to ReadW lp = '%s'\n",lp);
		q->event_priority=ReadW();

        xp->size++;
		print_debug("PES: ReadW returns %d\n",q->event_priority);

		q->next=NULL;							//link onto the list
		if (fp==NULL)
			fp=q;								//remember first guy we made
		else
			p->next=q;							//link new guy on the end of the list
		p=q;									//remember new guy is now the last guy
		q=NULL;
		if(( *(lp-1) == '(')&&(*lp == '(')) lp--;  // MAG
		print_debug("PES: end while loop, lp = '%s'\n",lp);
	}
	xfail=false;
xx:
	xp->special_event=fp;
	if (q!=NULL) free(q);						//don't lose this block!
	return xfail;								//									***008 End
}												//									***008 End

///////////////////////////////////////////////////////////////////////				***008 Begin
//	read an array of (list of BACnetTimeValue) from the buffer lp points to
//	{((t,v),(t,v)),((t,v),(t,v)),...}
//in:	lp		points to current position in buffer lb
//		wp		an array of pointer variables to be
//				initialized to point to the created lists of BACnetTimeValues
//out:	true	if an error occurred
//		lp		points past the delimiter ) unless it was the end of the buffer

BOOL ParseWeekdaySchedule(BACnetTimeValue *wp[])
{	word	i;
				
	for (i=0;i<7;i++) wp[i]=NULL;				//initially there are no lists
	
	skipwhitespace();
	if (*lp=='?') return false;					//? means no list
	if (MustBe('{')) return true;
	i=0;
	while(feof(ifile)==0)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//   Note that we require "empty" list elements to use proper syntax (...),(),(...)
		//   but (...),,(...) is treated the same as (...),(...)
		//2. (			i.e. the beginning of a new BACnetTimeValue list in the array
		//3. )			i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0)
			if (ReadNext()==NULL) return true;
		if (*lp=='}') 
		{	lp++;
			break;								//close this list out
		}
        
        if (i>6)
        {	tperror("Weekly Schedule cannot contain more than 7 days!",true);
        	return true;
        }
        
		//here we have (list of timevalues),...
        if (ParseTVList(&wp[i])) return true;
        i++;
	}
	return false;
}												//									***008 End

///////////////////////////////////////////////////////////////////////				***008 Begin
//	read a list BACnetTimeValues from the buffer lp points to
//	({time,value},{time,value}...)
//in:	lp		points to current position in buffer lb
//		tvp		points to a variable which should point to a list of BACnetTimeValues
//out:	true	if an error occurred
//		else	*tvp points to a list of BACnetTimeValues
//		lp		points past the delimiter ) unless it was the end of the buffer

BOOL ParseTVList(BACnetTimeValue **tvp)
{	BACnetTimeValue	*fp=NULL,*p=NULL,*q=NULL;
	BOOL				tvfail=true;
	char			*ep;				
				
	*tvp=NULL;									//initially there is no list
	skipwhitespace();
	if (MustBe('(')) return true;
	while(true)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//   Note that we require "empty" list elements to use proper syntax (...),(),(...)
		//   but (...),,(...) is treated the same as (...),(...)
		//2. (	i.e. the beginning of a new BACnetTimeValue in the list
		//3. )				i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0) 
			if (ReadNext()==NULL) break;
		if (*lp==')') 
		{	lp++;
			break;								//close this list out
		}

		if (MustBe('{')) goto tvx;

		//here we have time,value),...)
		if ((q=(tagTimeValue *)malloc(sizeof(BACnetTimeValue)))==NULL)
		{	tperror("Can't Get Object Space!",true);
			goto tvx;
		}
		print_debug("LJT: TimeValue=%x\n",q);

		if ((ep=strchr(lp,'}'))==NULL)
		{	lp=strchr(lp,0);
			tperror("Expected ) at end of (time,value...",true);
			goto tvx;
		}
		*ep=0;									//make it "time,value" asciz
		if (ParseTime(&q->time)) goto tvx;
		if (strdelim(",")==NULL) goto tvx;
		skipwhitespace();
		if (*lp=='a'||*lp=='A')					//BPV Active
		{	q->value_type=BPV;
			q->av.bproperty_value=ACTIVE;
		}
		else if (*lp=='i'||*lp=='I')			//									***013
		{	q->value_type=BPV;
			q->av.bproperty_value=INACTIVE;
		}
		else if ( strstr( lp, "unknown" ) != NULL )
		{
			// enumerated unknown value
			lp+=8;  // skip the text 'unknown:'
			q->value_type=BPV;
			q->av.uproperty_value=ReadW();
		}
		else if (strchr(lp,'.')==NULL)		//doesn't have a dot, must be unsigned
		{	
			q->value_type=UNS;
			q->av.uproperty_value=ReadW();
		}
		else									//must be float
		{	q->value_type=FLT;
			q->av.fproperty_value=(float)atof(lp);
		}

		q->next=NULL;							//link onto the list
		if (fp==NULL)
			fp=q;								//remember first guy we made
		else
			p->next=q;							//link new guy on the end of the list
		p=q;									//remember new guy is now the last guy
		q=NULL;
		*ep++='}';
		lp=ep;

		// loop back around to see if we have another {time,value} pair
	}
	tvfail=false;
tvx:
	*tvp=fp;
	if (q!=NULL) free(q);						//don't lose this block!
	return tvfail;								//									***008 End
}

// Added the propID argument to distinguish between Lists and Arrays when parsing
// The handling of BACnetListOfDeviceObjectReference[] is implemented the same
// here whether a list or array, so we can get away with this as long as we cover it
// in parsing.
BOOL ParseDevObjList(BACnetDeviceObjectReference **refp, word propID)
{
	BACnetDeviceObjectReference	*fp=NULL,*p=NULL,*q=NULL;
	BOOL				reffail=true;
	char openChar  = '(';  // starts with '('  SEQ OF lists are enclosed by ()
	char closeChar = ')';
				
	*refp=NULL;									//initially there is no list
	skipwhitespace();

	// Adjust for the property(s) that are actually Arrays, not Lists
	if ( propID == SUBORDINATE_LIST )
	{
		openChar  = '{';		// Arrays open with brace
		closeChar = '}';
	}

	if ( MustBe( openChar ) ) return true; 
	while(true)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//   Note that we require "empty" list elements to use proper syntax (...),(),(...)
		//   but (...),,(...) is treated the same as (...),(...)
		//2. (	i.e. the beginning of a new BACnetObjectPropertyReference in the list
		//3. )				i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0) 
			if (ReadNext()==NULL) break;
		if (*lp== closeChar) // found empty list
		{	lp++;
			break;								//close this list out
		}

		if ((q=ParseDevObjReference(NULL))==NULL)	goto refx;
		if((*lp == 0) && (*(lp-1) == '}')) lp--;  // MAG fix for parser error in schedule object 14 FEB 2001

		q->next=NULL;							//link onto the list
		if (fp==NULL)
			fp=q;								//remember first guy we made
		else
			p->next=q;							//link new guy on the end of the list
		p=q;									//remember new guy is now the last guy
		q=NULL;
	}
	reffail=false;
refx:
	*refp=fp;
	if (q!=NULL) free(q);						//don't lose this block!
	return reffail;								//								***008 End
}

///////////////////////////////////////////////////////////////////////				***008 Begin
//	read a list BACnetObjectPropertyReferences from the buffer lp points to
//	(((objtype,instance),propertyname[arrayindex]),((objtype,instance),propertyname[arrayindex])...)
//in:	lp		points to current position in buffer lb
//		refp	points to a variable which should point to a list of BACnetObjectPropertyReferences
//out:	true	if an error occurred
//		else	*refp points to a list of BACnetObjectPropertyReferences
//		lp		points past the delimiter ) unless it was the end of the buffer

BOOL ParseRefList(BACnetDeviceObjectPropertyReference **refp)
{	BACnetDeviceObjectPropertyReference	*fp=NULL,*p=NULL,*q=NULL;
	BOOL				reffail=true;				
				
	*refp=NULL;									//initially there is no list
	skipwhitespace();

	if (MustBe('(')) return true; // starts with '('  SEQ OF lists are enclosed by ()
	while(true)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//   Note that we require "empty" list elements to use proper syntax (...),(),(...)
		//   but (...),,(...) is treated the same as (...),(...)
		//2. (	i.e. the beginning of a new BACnetObjectPropertyReference in the list
		//3. )				i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0) 
			if (ReadNext()==NULL) break;
		if (*lp==')') // found empty list
		{	lp++;
			break;								//close this list out
		}

		if ((q=ParseDevObjPropReference(NULL))==NULL)	goto refx;
		if((*lp == 0) && (*(lp-1) == '}')) lp--;  // MAG fix for parser error in schedule object 14 FEB 2001

		q->next=NULL;							//link onto the list
		if (fp==NULL)
			fp=q;								//remember first guy we made
		else
			p->next=q;							//link new guy on the end of the list
		p=q;									//remember new guy is now the last guy
		q=NULL;
	}
	reffail=false;
refx:
	*refp=fp;
	if (q!=NULL) free(q);						//don't lose this block!
	return reffail;								//								***008 End
}

///////////////////////////////////////////////////////////////////////
//	read a BACnetObjectPropertyReference from the buffer lp points to
//	  ((objtype,instance),propertyname,arrayindex) or
//	  ((objtype,instance),propertyname,) or ?
//  See the NOTE at the top of this file for a discussion of this encoding, with
//  the comma REQUIRED to encode the presence or absence of the optional index
//in:	lp		points to current position in buffer lb
//		inq		points to a BACnetObjectPropertyReference to be filled in (or NULL)
//out:	NULL	if an error occurred
//		else	pointer to newly created BACnetObjectPropertyReference
//		lp		points past the delimiter ) unless it was the end of the buffer

BACnetObjectPropertyReference *ParseReference(BACnetObjectPropertyReference	*inq)
{	BACnetObjectPropertyReference	*q=NULL;
	dword	dw;
	char    openChar = ' ';
	
	bool    isSetpointRef = false;
	if ( inq == NULL )
		isSetpointRef = true;

	//here we have {(objtype,instance),propertyname[arrayindex],2}
	//here we have {{(objtype,instance),propertyname[arrayindex],2}}
	//           or (objtype, instance), propertyname[arrayindex]
	skipwhitespace();
	
	// allow empty valule
	if (*lp=='?') return NULL;
	if (*lp == '}' && *(lp+1) == '}' ) return NULL;

	if (isSetpointRef)
		if (MustBe('{')) return NULL;   // this is the setpoint-reference type therefore must start with {{

	if ((*lp == '(' && *(lp+1) == '(') || *lp == '{') 
	{
		openChar = *lp;
		lp++;  // advance to next location  -- the starting { is not required here
	}

	if (inq==NULL)
	{	if ((q=(tagObjectPropertyReference *)malloc(sizeof(BACnetObjectPropertyReference)))==NULL)
		{	tperror("Can't Get Object Space!",true);
			return NULL;
		}
		print_debug("LJT: ObjectPropertyReference=%x\n",q);

	}
	else
		q=inq;

	skipwhitespace();
	// if((*lp != '(') && (*(lp-1) == '(')) lp--;  // MAG
	dw=ReadObjID();
	if (dw==badobjid)
	{	
		tperror("Must use an Object Identifier here!",true);	//					***006
		goto oprfail;
	}
	q->object_id=dw;

	// madanner 6/03: Can't just skip to next delimeter... we must barf on non-white space
	skipwhitespace();
	if ( MustBe(',')) goto oprfail;
	skipwhitespace();						//										***006

	// validate property name and index code here
	if (validatePropertyNameAndIndexCode(dw, &q->property_id, &q->pa_index ) == NULL)
		goto oprfail;
	
	if ( openChar != ' ' )  // meaning we opened with this character
	{
		skipwhitespace();
		char closeChar = ')';
		if ( openChar == '{' )
			closeChar = '}';
		if ( MustBe( closeChar ) ) goto oprfail;
	}
	q->next=NULL;							//										***012 End

	if (isSetpointRef)
		if (MustBe('}')) goto oprfail;   // note it must end in extra }

	return q;
	
oprfail:
	if (inq==NULL) free(q);
	return NULL;
}

//*********added by Liangping Xu,2002-9*************************//

///////////////////////////////////////////////////////////////////////
//	read a BACnetDeviceObjectPropertyReference from the buffer lp points to
//	  ((objtype,instance),propertyname,arrayindex) or
//	  ((objtype,instance),propertyname,) or ?
//  See the NOTE at the top of this file for a discussion of this encoding, with
//  the comma REQUIRED to encode the presence or absence of the optional index
//in:	lp		points to current position in buffer lb
//		inq		points to a BACnetDeviceObjectPropertyReference to be filled in (or NULL)
//out:	NULL	if an error occurred
//		else	pointer to newly created BACnetObjectPropertyReference
//		lp		points past the delimiter ) unless it was the end of the buffer

BACnetDeviceObjectPropertyReference *ParseDevObjPropReference(BACnetDeviceObjectPropertyReference	*inq)
{	BACnetDeviceObjectPropertyReference	*q=NULL;
	dword	dw,objId_Ref;
	
	//here we have ((objtype,instance),propertyname,arrayindex)...
	skipwhitespace();
	if (*lp=='?') return NULL;
	if ( *lp=='{' || (*lp=='(' && *(lp+1)!='(') )
		*lp++;
	else
		return NULL;

	if (inq==NULL)
	{	
		if ((q=(BACnetDeviceObjectPropertyReference *)malloc(sizeof(BACnetDeviceObjectPropertyReference)))==NULL)
		{	
			tperror("Can't Get Object Space!",true);
			return NULL;
		}
		print_debug("LJT: DeviceObjectPropertyReference=%x\n",q);
	}
	else
	{
		q=inq;
	}

    //initial the Values
    q->DeviceObj =0xffffffff;
    q->Objid =0xffffffff;
	q->wPropertyid=0xffffffff;
	q->ulIndex=NotAnArray;

	skipwhitespace();
	dw=ReadObjID();
	if (dw==badobjid)
	{	tperror("Must use an Object Identifier here!",true);	//					***006
		goto oprfail;
	}

	q->Objid=dw; 
	objId_Ref=dw;
	goto propcheck;

	// madanner 6/03: Can't just skip to next delimeter... we must barf on non-white space
	skipwhitespace();
	if ( MustBe(',')) goto oprfail;
	skipwhitespace();						//										***006

    //Read Obj_id that's referred
	objId_Ref=ReadObjID();
	if (objId_Ref==badobjid)
	{	tperror("Must use an Object Identifier here!",true);	//					***006
		goto oprfail;
	}
    q->Objid=objId_Ref;

propcheck:
	// madanner 6/03: Can't just skip to next delimeter... we must barf on non-white space
	skipwhitespace();
	if ( MustBe(',')) goto oprfail;

	skipwhitespace();

	// validate property name and index here
	if (validatePropertyNameAndIndexCode(dw, &q->wPropertyid, &q->ulIndex ) == NULL)
		goto oprfail;

	// need to read device ObjId here OPTIONAL
    skipwhitespace();
	if ( *lp==',' ) 
	{
		*lp++;
		dw=ReadObjID();
		q->DeviceObj = dw;
	}


	if ( *lp=='}' || *lp==')' )
		*lp++;
	else
		goto oprfail;

	return q;
	
oprfail:
	if (inq==NULL) free(q);
	return NULL;
}
//
////////////////////////////////////////////////////////////////////
BACnetDeviceObjectReference *ParseDevObjReference(BACnetDeviceObjectReference	*inq)
{	
	BACnetDeviceObjectReference	*q=NULL;
	dword	dw,objId_Ref;
	word	objtype_Ref;
	
	//here we have ((objtype,instance),propertyname[arrayindex])...
	skipwhitespace();
	if (*lp=='?') return NULL;
	if ( *lp=='{' || (*lp=='(' && *(lp+1)!='(') )
		*lp++;
	else
		return NULL;

	if (inq==NULL)
	{	if ((q=(BACnetDeviceObjectReference *)malloc(sizeof(BACnetDeviceObjectReference)))==NULL)
		{	tperror("Can't Get Object Space!",true);
			return NULL;
		}
		print_debug("LJT: DeviceObjectReference=%x\n",q);

	}
	else
		q=inq;

    //initial the Values
    q->DeviceObj =0xffffffff;
    q->Objid =0xffffffff;

	skipwhitespace();
	dw=ReadObjID();
	if (dw==badobjid)
	{	tperror("Must use an Object Identifier here!",true);	//					***006
		goto oprfail;
	}
    if((objtype_Ref=(word)(dw>>22))==0x0008) 
	{
		 q->DeviceObj=dw;
		 // now require reading of the objectId
		skipwhitespace();
		if ( MustBe(',')) goto oprfail;
		skipwhitespace();						//										***006

		//Read Obj_id that's referred
		objId_Ref=ReadObjID();
		if (objId_Ref==badobjid)
		{	tperror("Must use an Object Identifier here!",true);	//					***006
			goto oprfail;
		}
		q->Objid=objId_Ref;
	 
	}
	else
	{ 
 	   q->Objid=dw; 
	   objId_Ref=dw;
	}

	// need to read device ObjId here OPTIONAL
    skipwhitespace();
	if ( *lp=='}' || *lp==')' )
		*lp++;
	else
		goto oprfail;

	return q;
	
oprfail:
	if (inq==NULL) free(q);
	return NULL;
}

///////////////////////////////////////////////////////////////////////				***008 Begin
//	read a list BACnetCalendarEntrys from the buffer lp points to
//	((m/d/y dow),(d-m-y dow),(m,wom,dow),(date..date)...)
//  ((dow, d-m-y), (date..date)...) Shiyuan Xiao. According to standard 135.1-2003.

//in:	lp		points to current position in buffer lb
//		calp	points to a variable which should point to a list of BACnetCalendarEntrys
//out:	true	if an error occurred
//		else	*calp points to a list of BACnetCalendarEntrys
//		lp		points past the delimiter ) unless it was the end of the buffer

BOOL ParseCalist(BACnetCalendarEntry **calp)
{	BACnetCalendarEntry	*fp=NULL,*p=NULL,*q=NULL;
	//word				i;  MAG 31JAN06 variable i not used
	char			*ep;
	BOOL				calfail=true;				
				
	*calp=NULL;									//initially there is no list
	skipwhitespace();
	if (MustBe('(')) return true;     //Modified by xlp
	while(true)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//   Note that we require "empty" list elements to use proper syntax (...),(),(...)
		//   but (...),,(...) is treated the same as (...),(...)
		//2. {	i.e. the beginning of a new BACnetCalendarEntry in the list
		//3. }				i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0) 
			if (ReadNext()==NULL) break;
		if (*lp==')')    //Modified by xlp
		{	lp++;
			break;								//close this list out
		}
		if (MustBe('{')) goto calx;

		//here we have calentry)...     
		if ((ep=strchr(lp,'}'))==NULL)
		{	lp=strchr(lp,0);
			tperror("Expected } here!",true);
			goto calx;
		}
		*ep++=0;								//remember this end pointer
		//here, lp points to one of 3 formats:
		//	month,weekofmonth,dayofweek
		//	date..date
		//	date
		//
		//where date is in one of two formats:
		//	month/day/year dow			- month is numeric, is optional (if present, separated by a space)
        //  dow, day-month-year     - month is full name (e.g. January)
		
		if ((q=(tagCalendarEntry *)malloc(sizeof(BACnetCalendarEntry)))==NULL)
		{	tperror("Can't Get Object Space!",true);
			goto calx;
		}
		print_debug("LJT: CalendarEntry=%x\n",q);

		if (strchr(lp,'X')!=NULL)				//must be WeekNDay
		{	
			q->choice=2;						//WeekNDay
			skipwhitespace();
			q->u.weekNday.month=dontcare;
			q->u.weekNday.day=dontcare;
			q->u.weekNday.week=dontcare;
			
			lp+=2;  // skip the X'

			cvhex(lp,&q->u.weekNday.month);
			cvhex(lp+2,&q->u.weekNday.week);
			cvhex(lp+4,&q->u.weekNday.day);
	    	lp+=6;
		}
//***Modified by Liangping Xu,2002-9
		// Modified by LJT 10/27/2005
		else if (strstr(lp,"(")!=NULL)  // either date or daterange LJT
		{	
			q->choice=0;						// assume date for now
			if (ParseDate(&q->u.date)) goto calx;
//		    if (ParseDate(&q->u.date_range.start_date)) goto calx;

			while (*lp==space) lp++;		//skip spaces but leave ',' list elements
//			skipwhitespace();
			// skip the , if that is the delimeter between date range
			if ( *lp==',' )
			{
				q->choice=1;						//DateRange
				q->u.date_range.start_date = q->u.date;
				lp++;
				if (ParseDate(&q->u.date_range.end_date)) goto calx;
			}
		}
//////////////////////////////////////////////////////////////

		else
		{	
			// invalid entry
			goto calx;
		}
		lp=ep;
		ep[-1]='}';  // put end of item back
		q->next=NULL;							//link onto the list
		if (fp==NULL)
			fp=q;								//remember first guy we made
		else
			p->next=q;							//link new guy on the end of the list
		p=q;									//remember new guy is now the last guy
		q=NULL;
	}
	calfail=false;
calx:
	*calp=fp;
	if (q!=NULL) free(q);						//don't lose this block!
	return calfail;								//								***008 End
}
////////////////////////////////////////////////////////////////////
// This function is called to determine the type of the values in the
// event-time-stamp.
////////////////////////////////////////////////////////////////////
octet whatIsChoice(char str[])
{
  unsigned char ucindex = 0;
  octet status = 33; /* return value - set to be invalid */
   
  skipwhitespace();
	 
  // Start checking 
//   while(*lp != '/' && *lp != ':' && *lp != ')' && ucindex < 7)
// while(*lp != '/' && *lp != '-' && *lp != ':' && *lp != ')' && ucindex < 7)     // ***020
 while(*lp != '/' && *lp != '-' && *lp != ':' && *lp != '}' && ucindex < 7)     // ***020
 {
    lp++;
    ucindex++;
  }

  if (ucindex <= 5)
  {
    switch (*lp)
	{
	  // Found the time delimiter.
      case ':':		   
		// move the line pointer to the first char in the field
        lp -= ucindex;  
        status = 0;	 
        break;
	  // Found the date-time delimiter.
      case '/':	
	  case '-':									// ***020
        lp -= ucindex;
        status = 20;	  
        break;
	  // Found the sequence number.
      case '}':
        str[ucindex] = *lp;
		// do we have an int in front of the delimiter?
        while(*lp >= '0' && *lp <= '9' && ucindex > 0 || *lp == '}') 
          str[--ucindex] = *--lp;
		// wildcard is ok.
        if (*lp == '?') 
		  status = 10; 
		// we have a well formed int. life is good.
        else if (ucindex == 0)
          status = 11;	      
        break;
	}
  }
  //error: The Sequence Number is > 65535.
  else
    status = 12;

  return (status);
} // end whatIsChoice

// LJT 10/7/2005 rewrote
///////////////////////////////////////////////////////////////////
//  This function parses the Time Stamp data presented in the PICS
//  Format: DateTime ,Time & Sequence number -> {(6/21/2001,11:32:43.0),(12:14:01.05),(65535)}  
//  DatetTime can also be writed out in such way: (23-MAR-95, 18:50:21.2)              
//  The sequence number may be assigned any value from 0 - 65535 or the default "?".  
//	in:	lp		points to current position in buffer lb
//			inq		points to a BACnetObjectPropertyReference to be filled in (or NULL)
//	out:	NULL	if an error occurred
//			else	pointer to newly created BACnetObjectPropertyReference
//			lp		points past the delimiter ) unless it was the end of the buffer
///////////////////////////////////////////////////////////////
BOOL ParseTimeStamp(BACnetTimeStamp *ptstamp)
{
	octet           oSwval;
    char			amyLine[20];

    skipwhitespace();

      switch (oSwval = whatIsChoice(amyLine))
      {
        case 00:   // The choice is time.
			ptstamp->choice = 0;
			if ( ParseTime(&ptstamp->u.time) )
			{
				tperror("Invalid time format",true);
				return true;
			}
        break;
        case 10:	  // The choice is sequence number, wildcard option.
               ptstamp->u.sequence_number = 0;	// wildcard
			   goto numch;
        case 11:	  // The choice is sequence number number 0-65535.

               ptstamp->u.sequence_number = atoi(amyLine);
			   while (*++lp != '}');  // skip ahead to the closing bracket because atoi doesn't adjust lp location
numch:     ptstamp->choice = 1;	//
        break;
		case 12:
             if (tperror("Too many digits or spaces before /",true)) return true;
		break;

        case 20:	  // The choice is Date-Time.
 		    ptstamp->choice = 2;	//
			if ( ParseDateTime(&ptstamp->u.date_time) )
			{
				tperror("Invalid Date-Time format.", true);
				return true;
			}
        break;
		default:
             if (tperror("Cannot decode Event-Time-Stamp.",true)) return true;
	    break;
       }
   	   return false;
}  // end ParseTimeStamp

// LJT 10/7/2005 
BOOL ParseTimeStampArray(BACnetTimeStamp *parray[], int arraycount)
{
	BACnetTimeStamp *q=NULL;

	skipwhitespace();

    if (MustBe('{'))
        return true;	

	for(int i=0; i < arraycount; i++)
	{
		skipwhitespace();
		if ( MustBe('{')) return true;  // each element surrounded by '{'
				
		if ((q=(tagTimeStamp *)malloc(sizeof(BACnetTimeStamp)))==NULL)
		{	
			tperror("Can't Get Object Space!",true);
			return true;
		}
  		print_debug("LJT: TimeStamp=%x\n",q);
	    if ( ParseTimeStamp(q) )
		{
			char tmp[50];
			sprintf(tmp,"Parse error in event-time-stamps, index=%d",i+1);
			tperror(tmp,true);
			return true;  // error occurred
		}
		skipwhitespace();
		if ( MustBe('}'))
			return true;   // must close each array item
		// each array item is separated by a comma
        skipwhitespace();
		if ( (i+1) < arraycount )
			if ( MustBe(',')) return true;

		parray[i] = q;
		q=NULL;
	}

	if (q!=NULL) free(q);						//don't lose this block!

	// check for the closing bracket
	if ( MustBe('}'))
		return true;

	return false;
}

///////////////////////////////////////////////////////////////////////
//	parse a date/time value
//in:	lp		points to (date,time)
//		dtp		points to structure to initialize
//out:	true	if cancel selected

/*
Any of the following should be valid ...
   (5,29-SEP-1989), 01:00:00.0 
   (Friday,29-SEP-1989), 01:00:00.0 
   (29-SEP-1989, 5), 01:00:00.0 
   (29-SEP-1989, Friday), 01:00:00.0 
   (9/29/1989, Friday), 01:00:00.0 

*/
BOOL ParseDateTime(BACnetDateTime *dtp)
{	
	skipwhitespace();
    if (ParseDate(&dtp->date)) return true;
	if (MustBe(',')) return true;  // ',' separates the date and time text
    if (ParseTime(&dtp->time)) return true;
	return false;
}

// LJT 10/7/2005
int getDayOfWeek(char* tok)  // returns 0 if not day of week, -1 if any
{
	int i = 0;
	lp = tok;
	skipwhitespace();
	if ( tok[0] != '*' && IsAlpha(tok[0]) )
	{
	   for(i=0;i<7;i++)
	   {
		if (strnicmp(DOWNames[i], tok, strlen(DOWNames[i])) == 0)
		{	
			return i+1;  // day of week is 1 - 7
		}
	   }
	   return 0;
	}				
	else
	{
		if (tok[0] == '*') 
			return -1;
		else
		{
			// assumes lp points to tok
			return ReadB(1,7);  // need to validate number from 1 to 7
		}
	}
}

// LJT 10/7/2005
BOOL hasDateSeparators(char* tok)
{
	if ( strchr(tok, '-') != NULL || strchr(tok, '/') != NULL )
	{
		return true;
	}
	return false;
}

// LJT 10/7/2005
BOOL ParseJustDatePart(BACnetDate *dtp, char* tok)
{
	octet	db;
	int i = 0;
	lp = tok;
    if ((db=ReadB(1,31))!=dontcare)				//not wild
    {
		if (lp[-1]=='/')						//was it month/day/year?
		{	
			print_debug("PD: find month first\n");
			if (db>12)							//yes
			{	
				tperror("Month must be 1-12!",true);
				return true;
			}

			dtp->month=db;
		}
		else									//must be day-month-year
		{	
			print_debug("PD: find day of month first\n");
			if (db>31)
			{
				tperror("Day of month must be 1-31!",true);
				return true;
			}
			dtp->day_of_month=db;
		}
    } 
	else 
		print_debug("PD: first value not specified\n");
	
	print_debug("PD: About to read second set, db = %d lp = '%s' lp[-1] = '%c' \n",db,lp,&lp[-1]);

    if (lp[-1]=='/')      //was it month/day/year?
    {	
		print_debug("PD: second set slash case\n");

		if ((db=ReadB(1,31))!=dontcare)			//we'll check for valid days later
		dtp->day_of_month=db;
    }
    else if (lp[-1]=='-')						//day-month-year
    {	
		print_debug("PD: second set dash case\n");

		for (i=0;i<12;i++)
			if (strnicmp(lp,MonthNames[i],3)==0)  // only look at first 3 chars
			{	
				dtp->month=(octet)i+1;			//months are 1-12
				char* tmp = strchr(lp,'-');
				int extralength = tmp==NULL ? 0 : strlen(tmp);
			    lp += strlen(lp) - extralength;	    //added by Liangping Xu
			    break;
			}
			if ((strdelim("-"))==NULL)
			{	
				tperror("Must use monthname-year here!",true);
			    return true;
			}
    }
    else if (lp[-1]!=',')
	{	tperror("Must use month/day/year or day-monthname-year here!",true);
	return true;
	}
    
	print_debug("PD: About to read third set\n");
	BOOL flag = TRUE;
	if( strchr(lp, ')') == NULL )
		flag = FALSE;
    if (i=ReadW())								//not wild
    {	
		//Shiyuan Xiao 7/25/2005
		if(flag && strchr(lp, ')') == NULL)
			lp--;
		
		if (i>2154)								//can't represent this date
		{	
			tperror("Can't represent dates beyond 2154!",true);
			return true;
		}

		if (i>254&&i<1900)
		{	
			tperror("Can't represent this year!",true);
			return true;
		}

		if (i>=1900) 
			i-=1900;// MAG fix bug here when date==1900 by change > to >= 08 FEB 2001

		// LJT adjusted so that a reference to '01' would mean 2001 not 1901
		if ( i < 60 ) // assume 2000 instead of 1900 so adjust...
			i += 100;

		dtp->year=(octet)i;
    }
	return false;
}

///////////////////////////////////////////////////////////////////////
//	parse a date value
//in:	lp		points to date
//		dtp		points to structure to initialize
//out:	true	if cancel selected
/*
Any of the following should be valid ...
   (5,29-SEP-1989)
   (Friday,29-SEP-1989) 
   (29-SEP-1989, 5)
   (29-SEP-1989, Friday)
   (9/29/1989, Friday)

*/
// LJT 10/7/2005 rewrote
BOOL ParseDate(BACnetDate *dtp)
{
	print_debug("PD: Enter ParseDate, lp = '%s'\n",lp);	//MAG
	if (MustBe('(')) return true;  // date is surrounded by ()

	// fill initially with ANY values
	dtp->year=dontcare;
	dtp->month=dontcare;
	dtp->day_of_month=dontcare;
	dtp->day_of_week=dontcare;
	skipwhitespace();
	print_debug("PD: about to read first set\n");

	// look for ',' before first ')'
	char cpylp[500];  // will lp every be any longer? 
	strcpy( cpylp, lp );
    char* tok1 = strtok(cpylp, ",)");
    if ( tok1 == NULL )
		return true;   // did not find either a , or a )
	char* tok2 = strtok(NULL, ")");
	if ( tok2 == NULL )
		return true;  // did not find ending ')'

	if ( hasDateSeparators(tok1) )
	{
		// this must be the date part, tok2 must be day of week
		char* savelp = lp;  // location of pointer
		int saveLen = strlen(tok1);
		ParseJustDatePart(dtp, tok1);
		lp = savelp + saveLen + 1;
		savelp = lp;
		saveLen = strlen(tok2);
		dtp->day_of_week=(octet)getDayOfWeek(tok2);
		lp = savelp + saveLen;  // leave ending ')'
	}
	else
	{
		// this must be the day of week, tok2 must be the date part
		char* savelp = lp;  // location of pointer
		dtp->day_of_week=(octet)getDayOfWeek(tok1);
		lp = savelp + strlen(tok1) + 1;
		savelp = lp;
		ParseJustDatePart(dtp, tok2);
		lp = savelp + strlen(tok2);  // leave ending ')'
	}
	print_debug("PD: Exit ParseDate normal\n");
	if (MustBe(')')) return true;
	return false;
}

///////////////////////////////////////////////////////////////////////
//	parse a time value
//in:	lp		points to time
//		tp		points to structure to initialize
//out:	true	if cancel selected

BOOL ParseTime(BACnetTime *tp)
{	
	tp->hour=dontcare;
	tp->minute=dontcare;
	tp->second=dontcare;
	tp->hundredths=dontcare;
	skipwhitespace();
    tp->hour=ReadB(0,23);
    if (lp[-1]==':')
    {	tp->minute=ReadB(0,59);
    	if (lp[-1]==':')
    	{	tp->second=ReadB(0,59);
    		if (lp[-1]=='.')
    			tp->hundredths=ReadB(0,99);
    	}
    }
    lp--;										//point back to delimiter
	return false;
}

///////////////////////////////////////////////////////////////////////
//	parse a bitstring value as either of two forms:
//		B'11011...1011'
//  or
//		( T,F,T,F,...)		i.e. T or True or F or False in a list					***008
//note:	after the initial B' or ( the list may span multiple lines
//
//in:	lp		points to B'11011...1011' or (...)
//		bsp		points to structure to initialize
//		nbits	how many bits maximum
//		nbf		points to octet to update with number of bits found (or NULL)
//out:	true	if cancel selected

BOOL ParseBitstring(octet *bsp,word nbits,octet *nbf)
{	octet	db;
	char	term[3];
	BOOL	isbits;								//									***008
    //modified by xlp,2002-11
	memset(bsp,0,nbits/8+1);					//initialize to 0
	if (nbf) *nbf=0;							//no bits found yet

	skipwhitespace();
	if ((*lp=='b'||*lp=='B')&&(lp[1]=='\''||lp[1]=='`'))	//						***008 Begin
	{//it's a bitstring
		isbits=true;
		term[0]='\'';							//terminator
		lp+=2;									//skip over B'
	}
	else if (*lp=='(')
	{//it's a flag list
		isbits=false;
		term[0]=')';							//terminator
		lp++;									//skip over (
	}
	else
	{	tperror("Expected B'bitstring' or (T,F...) bitflag list here!",true);
		return true;
	}
	term[1]=',';
	term[2]=0;									//i.e. term is asciz string
	db=0x80;
	while(feof(ifile)==0)
	{	if (*lp==0) 
			if (ReadNext()==NULL) break;
		if (*lp==term[0])
		{	lp++;
			break;								//found terminator
		}
		if (nbits)
		{	if (isbits)
			{	if (*lp=='1')
					*bsp|=db;
				else
					if (*lp!='0')
					{	tperror("Bitstring must contain 0s or 1s!",true);
						return true;
					}
				lp++;							//skip 0 or 1
			}
			else
			{	skipwhitespace();
				if (*lp=='t'||*lp=='T')
					*bsp|=db;
//madanner 6/03
//Bug in delimeter search... order of delim chars is significant in strdelim function
//Used here as ",)" so we'll find comma first...   Was: ")," which found paren first and skipped
//reading entire bitstring
//				if (strdelim(&term[0])==NULL) break;
				if (strdelim(",)")==NULL) break;

				if (lp[-1]==term[0])  // )
					break;
				if (lp[-1]==term[1]&&lp[-2]==term[0])  // ), 
				{
					lp--;  // backup one to leave the comma
					break;
				}
			}										//								***008 End
			if ((db>>=1)==0)
			{	db=0x80;
				bsp++;
			}
			if (nbf) *nbf+=1;					//update num bits found
			nbits--;
		}
		else
		{	tperror("Expected end of bitstring here!",true);
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////
//	parse an octetstring value as X'hexchars'
//in:	lp		points to X'hexchars'
//		osp		points to structure to initialize
//		nmax	how many octets maximum
//		ncount	points to word to receive string size in octets (may be NULL)
//out:	true	if cancel selected

BOOL ParseOctetstring(octet *osp,word nmax,word *ncount)
{	memset(osp,0,nmax);						//initialize to 0
	if (ncount!=NULL) *ncount=0;
	skipwhitespace();
	if ((*lp!='x'&&*lp!='X')||lp[1]!='\'')
	{	tperror("Expected X'octetstring' here!",true);
		return true;
	}
	lp+=2;										//skip over X'
	while(*lp&&*lp!='\'')
	{	if (nmax)
		{   lp=cvhex(lp,osp);					//convert some chars
		    osp++;
		    if (ncount!=NULL) (*ncount)++;
			nmax--;
		}
		else
		{	tperror("Expected ' for end of octetstring here!",true);
			return true;
		}
	}
	lp++;										//skip trailing '
	return false;
}

BOOL ParsePrescale(BACnetPrescale* pt)
{
	skipwhitespace();
	if (*lp=='?') 
		return false;					

	if (MustBe('{')) 
		return true;
	pt->multiplier = ReadW();
	lp--;
	
	if (MustBe(',')) 
		return true;

	skipwhitespace();	
    pt->moduloDivide = ReadW();
	lp--;

	skipwhitespace();
	if (MustBe('}')) 
		return true;
	
	return false;
}

BOOL ParseAccumulatorRecord(BACnetAccumulatorRecord* pt)
{
	skipwhitespace();
	if (*lp=='?') 
		return false;					

	if (MustBe('{')) 
		return true;
	
	ParseDateTime(&pt->timestamp);

	skipwhitespace();
	if (MustBe(',')) 
		return true;

	skipwhitespace();
	pt->presentValue = ReadW();

	skipwhitespace();
	pt->accumulatedValue = ReadW();

    pt->accumulatorStatus = ReadEnum(&etAccumulatorStatus);
	
	return false;
}
///////////////////////////////////////////////////////////////////////
//	read an array of BACnetActionCommands from the buffer lp points to
//  ({actioncommand},{actioncommand},{actioncommand}...)
//in:	lp		points to current position in buffer lb
//out:	NULL	if an error occurred
//		else	pointer to a list of BACnetActionCommands
//		lp		points past the delimiter } unless it was the end of the buffer
//
// - BACnetActionList is a list of BACnetActionCommand
// - 135.1 4.4.1 c says that lists are enclosed in parenthesis, with
//   elements separated by commas
// - each BACnetActionCommand is a SEQUENCE
// - 135.1 4.4 says that SEQUENCE are enclosed curly brackets
//
BACnetActionCommand *ReadActionCommands()
{	BACnetActionCommand	*firstp=NULL,*p=NULL,*q=NULL;
	propdescriptor		*pd;
	dword				dw;

	// List is enclosed by parenthesis
	skipwhitespace();							//									***008
	if (MustBe('('))
		return NULL;
	
	while (true)
	{   
		// here lp must point to:
		// 1. a comma or whitespace which we ignore as a separator between list elements.
		//    Note that we require "empty" list elements to use proper syntax (...),(),(...)
		//    but (...),,(...) is treated the same as (...),(...)
		// 2. (	i.e. the beginning of a new BACnetActionCommand in the list
		// 3. )				i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		
		if (*lp==0) 
			if (ReadNext()==NULL) break;		//									***008
		
		if (*lp==')') 
		{	
			lp++;
			break;								//close this list out
		}
		
		if (MustBe('{')) break;

		// here we have (BACnetActionCommand)...
		if ((q=(tagActionCommand *)malloc(sizeof(BACnetActionCommand)))==NULL)
		{	
			tperror("Can't Get Object Space!",true);
			break;
		}
		print_debug("LJT: ActionCommand=%x\n",q);

		q->device_id=badobjid;
		if (*lp!=',')							//									***008 Begin
		{	dw=ReadObjID();
			if ((word)(dw>>22)!=DEVICE)
			{	
				if (tperror("Must use a Device Object Identifier here!",true))
					break;
			}
			q->device_id=dw;
			if ((strdelim(","))==NULL) goto acprem;
		}
		else
		{
			lp++;								//skip comma						***008 End
		}
		
		dw=ReadObjID();
		if (dw==badobjid)
		{	
			if (tperror("Must use an Object Identifier here!",true))
				break;
		}
		q->object_id=dw;
		if ((strdelim(","))==NULL) goto acprem;
		skipwhitespace();					//									***008
		
		// validate property name and index here
		pd = validatePropertyNameAndIndexCode(dw, &q->property_id, &q->pa_index);
		if (pd == NULL)
			break;  // Error already output

		if ((strdelim(","))==NULL) goto acprem;	//								***012 End
		if (pd->ParseType==flt)					//it's a floating value
		{	
			q->value_type=FLT;
			q->av.fproperty_value=(float)atof(lp);
			if ((strdelim(","))==NULL) goto acprem;
		}
		else if (pd->PropET==eiBPV)				//it's a BPV
		{	
			q->value_type=BPV;
			q->av.bproperty_value=INACTIVE;		//assume inactive
			if (*lp=='a'||*lp=='A')
				q->av.bproperty_value=ACTIVE;
			if ((strdelim(","))==NULL) goto acprem;
		}
		else									//must be unsigned
		{	
			q->value_type=UNS;
			q->av.uproperty_value=ReadW();
		}
		
		// TODO: should we require the comma even
		// for non-commandable properties?
		if (pd->PropFlags&IsCommandable)		//only need priority for commandables	***008 Begin
			q->priority=ReadB(0,16);
		else
			if ((strdelim(","))==NULL) goto acprem;	//							***008 End
		
		q->post_delay=ReadW();			// OPTIONAL
		q->quit_on_failure=ReadBool();
		q->write_successful=ReadBool();
		if (lp[-1]!='}')
acprem:	{	tperror("Expected ) here!",true);
			break;
		}
		q->next=NULL;							//link onto the list			***008 Begin
		if (firstp==NULL)
			firstp=q;							//remember first guy we made
		else
			p->next=q;							//link new guy on the end of the list
		p=q;									//remember new guy is now the last guy
		q=NULL;
	}
	if (q!=NULL) free(q);						//don't lose this block!
	return firstp;								//								***008 End
}

BOOL ParseBooleanList( BooleanList **elp )
{
	BooleanList	*p=NULL,*q=NULL;
	word			value;
				
	*elp=NULL;									//initially there is no list
	
	skipwhitespace();
	if (MustBe('(')) return true;
	while(feof(ifile)==0)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//2. {              the beginning of a new unsigned in the list
		//3. }				i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0)
			if (ReadNext()==NULL) break;		//									***008
		if (*lp==')') 
		{	lp++;
			break;								//close this list out
		}

		if ((q=(tagBooleanList *)malloc(sizeof(BooleanList)))==NULL)
		{	tperror("Can't Get Object Space!",true);
			break;
		}
		print_debug("LJT: BooleanList=%x\n",q);

        q->next = NULL;

		skipwhitespace();
		if (*lp=='{') 
			lp++;

		if ((value=ReadBool())!=0xFFFF)
		{	
			q->value = (unsigned char)value;
         //msdanner 9/2004 - new items now added to the end, not beginning
         if (p)
         {
            // if aready one item in the list
            p->next = q;
            p = q;  // new end of list
         }
         else
         {
            // first item found ...
            *elp = q;
         }
			p=q;
		}
		else
			free(q);							//give this one up

		lp--;  // don't eat last character
		if (*lp=='}') 
			lp++;

		q=NULL;
		if (lp[-1]==')') break;					//list is done						***008
	}
	if (q!=NULL) free(q);						//don't lose this block!
	return false;
}

BOOL ParseUnsignedList( UnsignedList **elp )
{
	UnsignedList	*p=NULL,*q=NULL;
	word			value;
				
	*elp=NULL;									//initially there is no list
	
	skipwhitespace();
	if (MustBe('(')) return true;
	while(feof(ifile)==0)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//2. {              the beginning of a new unsigned in the list
		//3. }				i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0)
			if (ReadNext()==NULL) break;		//									***008
		if (*lp==')') 
		{	lp++;
			break;								//close this list out
		}

		if ((q=(tagUnsignedList *)malloc(sizeof(UnsignedList)))==NULL)
		{	tperror("Can't Get Object Space!",true);
			break;
		}
		print_debug("LJT: UnsignedList=%x\n",q);

        q->next = NULL;
		if ((value=ReadW())!=0xFFFF)
		{	
			q->value=value;
         //msdanner 9/2004 - new items now added to the end, not beginning
         if (p)
         {
            // if aready one item in the list
            p->next = q;
            p = q;  // new end of list
         }
         else
         {
            // first item found ...
            *elp = q;
         }
			p=q;
		}
		else
			free(q);							//give this one up

		q=NULL;
		if (lp[-1]==')') break;					//list is done						***008
	}
	if (q!=NULL) free(q);						//don't lose this block!
	return false;
}

BOOL ParseEnumList(BACnetEnumList **elp, etable *etbl)
{
	BACnetEnumList	*p=NULL,*q=NULL;
	word						value;
				
	*elp=NULL;									//initially there is no list
	
	skipwhitespace();
	if (MustBe('(')) return true;
	while(feof(ifile)==0)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//2. the beginning of a new BACnetVTClass enumeration in the list
		//3. )				i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0)
			if (ReadNext()==NULL) break;		//									***008
		if (*lp==')') 
		{	lp++;
			break;								//close this list out
		}

		if ((q=(tagEnumList *)malloc(sizeof(BACnetEnumList)))==NULL)
		{	tperror("Can't Get Object Space!",true);
			break;
		}
		print_debug("LJT: EnumList=%x\n",q);

        q->next = NULL;
		if ((value=ReadEnum(etbl))!=0xFFFF)
		{	q->value=value;
         //msdanner 9/2004 - new items now added to the end, not beginning
         if (p)
         {
            // if aready one item in the list
            p->next = q;
            p = q;  // new end of list
         }
         else
         {
            // first item found ...
            *elp = q;
         }
			p=q;
		}
		else
			free(q);							//give this one up

		q=NULL;
		if (lp[-1]==')') break;					//list is done						***008
	}
	if (q!=NULL) free(q);						//don't lose this block!
	return false;
}

///////////////////////////////////////////////////////////////////////
//	read a list of BACnetVTClasses from the buffer lp points to
//	(class,class,class...)
//in:	lp		points to current position in buffer lb
//		vtclp	points to a BACnetVTClassList pointer variable to be
//				initialized to point to the created list of BACnetVTClasses
//out:	true	if an error occurred
//		lp		points past the delimiter ) unless it was the end of the buffer

BOOL ParseVTClassList(BACnetVTClassList **vtclp)
{
	BACnetVTClassList	*p=NULL,*q=NULL;
	word				vtc;
				
	*vtclp=NULL;									//initially there is no list
	
	skipwhitespace();
	if (MustBe('(')) return true;
	while(feof(ifile)==0)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//2. the beginning of a new BACnetVTClass enumeration in the list
		//3. )				i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0)
			if (ReadNext()==NULL) break;		//									***008
		if (*lp==')') 
		{	lp++;
			break;								//close this list out
		}

		if ((q=(tagVTClassList *)malloc(sizeof(BACnetVTClassList)))==NULL)
		{	tperror("Can't Get Object Space!",true);
			break;
		}
		print_debug("LJT: VTClassList=%x\n",q);

      q->next = NULL;
		if ((vtc=ReadEnum(&etVTClasses))!=0xFFFF)
		{	q->vtclass=(BACnetVTClass)vtc;
         //msdanner 9/2004 - new items now added to the end, not beginning
         if (p)
         {
            // if aready one item in the list
            p->next = q;
            p = q;  // new end of list
         }
         else
         {
            // first item found ...
            *vtclp = q;
         }
			p=q;
		}
		else
			free(q);							//give this one up

		q=NULL;
		if (lp[-1]==')') break;					//list is done						***008
	}
	if (q!=NULL) free(q);						//don't lose this block!
	// *vtclp=p;  msdanner 9/2004 - now assigned above to preserve list order
	return false;
}

///////////////////////////////////////////////////////////////////////				***008 Begin
//	read an array of BACnetObjectIdentifiers from the buffer lp points to
//	{(objtype,instance),(objtype,instance),...}
//in:	lp		points to current position in buffer lb
//		dalp	points to a BACnetObjectIdentifier pointer variable to be
//				initialized to point to the created list of BACnetObjectIdentifiers
//		nump	points to a word counter for the number of items found
//out:	true	if an error occurred
//		lp		points past the delimiter ) unless it was the end of the buffer

BOOL ParseObjectList(BACnetObjectIdentifier **dalp,word *nump)
{	BACnetObjectIdentifier	*firstp=NULL,*p=NULL,*q=NULL;
	word  objtype;
	dword objectInstance; 
	int i;

	*dalp=NULL;									//initially there is no list
	*nump=0;
	
	skipwhitespace();
	if (*lp=='?') return false;					//? means no list
	if (MustBe('{')) return true;
	while(feof(ifile)==0)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//   Note that we require "empty" list elements to use proper syntax (...),(),(...)
		//   but (...),,(...) is treated the same as (...),(...)
		//2. (			i.e. the beginning of a new BACnetObjectIdentifier in the list
		//3. )			i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0)
			if (ReadNext()==NULL) break;		//									***008
		if (*lp=='}') 
		{	lp++;
			break;								//close this list out
		}

		//here we have (objtype,instance),...

		if ((q=(tagObjectIdentifier *)malloc(sizeof(BACnetObjectIdentifier)))==NULL)
		{	tperror("Can't Get Object Space!",true);
			break;
		}
		print_debug("LJT: ObjectIdentifier=%x\n",q);

		//Modified by xlp,2002-11
        objectInstance=ReadObjID(); 
        objtype=(word)(objectInstance>>22);
		if ( objtype < MAX_DEFINED_OBJ )
		{	// don't add proprietary objects here
			DevObjList[objtype].object_type=objtype;       
			DevObjList[objtype].ObjIDSupported|=soSupported;
			i=DevObjList[objtype].ObjInstanceNum;
			DevObjList[objtype].ObjInstanceNum++;
			DevObjList[objtype].ObjInstance[i]=objectInstance&0x003fffff;     
		}
		//ended by xlp,2002-11
		
		q->object_id=objectInstance;
		q->next=NULL;							//link onto the list
		if (firstp==NULL)
			firstp=q;							//remember first guy we made
		else
			p->next=q;							//link new guy on the end of the list
		p=q;									//remember new guy is now the last guy
		q=NULL;
		*nump+=1;
	}
	if (q!=NULL) free(q);						//don't lose this block!
	*dalp=firstp;
	return false;
}												//									***008 End

///////////////////////////////////////////////////////////////////////
//	read a list of BACnetAddressBindings from the buffer lp points to
//	(((device,instance),network,macaddr),((device,instance),network,macaddr),((device,instance),network,macaddr)...)
//in:	lp		points to current position in buffer lb
//		dalp	points to a BACnetAddressBinding pointer variable to be
//				initialized to point to the created list of BACnetAddressBindings
//out:	true	if an error occurred
//		lp		points past the delimiter ) unless it was the end of the buffer

BOOL ParseAddressList(BACnetAddressBinding **dalp)
{	BACnetAddressBinding	*p=NULL,*q=NULL;
	dword						dw;
				
	*dalp=NULL;									//initially there is no list
	
	skipwhitespace();
	if (*lp=='?') return false;					//? means no list					***008
	if (MustBe('(')) return true;
	while(feof(ifile)==0)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//   Note that we require "empty" list elements to use proper syntax (...),(),(...)
		//   but (...),,(...) is treated the same as (...),(...)
		//2. {			i.e. the beginning of a new BACnetAddressBinding in the list
		//3. }			i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0)
			if (ReadNext()==NULL) break;		//									***008
		if (*lp==')') 
		{	lp++;
			break;								//close this list out
		}
		if (MustBe('{')) break;  // each item surrounded by '{'

		//here we have (device,instance),network,macaddr)...

		if ((q=(tagAddressBinding *)malloc(sizeof(BACnetAddressBinding)))==NULL)
		{	tperror("Can't Get Object Space!",true);
			break;
		}
		print_debug("LJT: AddressBinding=%x\n",q);

		dw=ReadObjID();
		if ((word)(dw>>22)!=DEVICE)
		{	if (tperror("Must use a Device Object Identifier here!",true))
				break;
		}
		q->device_object_id=dw;
		skipwhitespace();
		if ((strdelim(","))==NULL) goto alprem;
		skipwhitespace();
        q->device_address.network_number=ReadW();
        if (ParseOctetstring(&q->device_address.mac_address[0],
        					 sizeof(q->device_address.mac_address),
        					 &q->device_address.address_size)) break;
		skipwhitespace();
		if (*lp++!='}')   // close individual item
alprem:	{	lp--;
			tperror("Expected ) here!",true);
			break;
		}
		q->next=p;								//link onto the list
		p=q;
		q=NULL;
	}
	if (q!=NULL) free(q);						//don't lose this block!
	*dalp=p;
	return false;
}

///////////////////////////////////////////////////////////////////////
//	read a list of BACnetSessionKeys from the buffer lp points to
//	((key,network,macaddr),(key,network,macaddr),(key,network,macaddr)...)
//in:	lp		points to current position in buffer lb
//		dalp	points to a BACnetSessionKey pointer variable to be
//				initialized to point to the created list of BACnetSessionKeys
//out:	true	if an error occurred
//		lp		points past the delimiter ) unless it was the end of the buffer

BOOL ParseSessionKeyList(BACnetSessionKey **dalp)
{	
	BACnetSessionKey	*p=NULL,*q=NULL;
				
	*dalp=NULL;									//initially there is no list
	
	skipwhitespace();
	if (MustBe('(')) return true;
	while(feof(ifile)==0)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//   Note that we require "empty" list elements to use proper syntax (...),(),(...)
		//   but (...),,(...) is treated the same as (...),(...)
		//2. {			i.e. the beginning of a new BACnetSessionKey in the list
		//3. }			i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0)
			if (ReadNext()==NULL) break;		//									***008
		if (*lp==')') 
		{	lp++;
			break;								//close this list out
		}
		if (MustBe('{')) break; // each list item is surrounded by {}

		//here we have key,network,macaddr}...

		if ((q=(tagSessionKey *)malloc(sizeof(BACnetSessionKey)))==NULL)
		{	tperror("Can't Get Object Space!",true);
			break;
		}
		print_debug("LJT: SessionKey=%x\n",q);

        if (ParseOctetstring(&q->session_key[0],
        					 sizeof(q->session_key),NULL)) break;
		if ((strdelim(","))==NULL) goto skprem;
        q->peer_address.network_number=ReadW();
        if (ParseOctetstring(&q->peer_address.mac_address[0],
        					 sizeof(q->peer_address.mac_address),
        					 &q->peer_address.address_size)) break;
		if (*lp++!='}')
skprem:	{	lp--;
			tperror("Expected } here!",true);
			break;
		}
		q->next=p;								//link onto the list
		p=q;
		q=NULL;
	}
	if (q!=NULL) free(q);						//don't lose this block!
	*dalp=p;
	return false;
}

BOOL normalBitstring(char *src)
{
	char *ep = src;
	BOOL found = false;
	if ((ep=strchr(src,','))==NULL)
	{	
		ep=strchr(src,0);
	}
	else
		*ep++=0;								//remember this end pointer

	for(int i=0; i<7; i++)
	{
		if (strnicmp(ep,DOWNames[i],3)==0)
		{
			found=true;
			break;
		}
	}
	ep[-1]=',';  // put it back like we found it
	return !found;  // if we found a day of week text it is NOT normal bitstring
}

///////////////////////////////////////////////////////////////////////
//	read a list of BACnetDEstination from the buffer lp points to
//	((days,from,to,(recipient),procid,conf,transitions),(days,from,to,(recipient),procid,conf,transitions)...)
//in:	lp		points to current position in buffer lb
//		dalp	points to a BACnetDestination pointer variable to be
//				initialized to point to the created list of BACnetDestinations
//out:	true	if an error occurred
//		lp		points past the delimiter ) unless it was the end of the buffer

BOOL ParseDestinationList(BACnetDestination **dalp)
{	
	BACnetDestination	*p=NULL,*q=NULL;
	char	c;
	octet	dm;
	word	i;
	char found_day;  // MAG

	*dalp=NULL;									//initially there is no list
	
	print_debug("PDL: Enter ParseDestinationList\n");
	skipwhitespace();
	if (MustBe('(')) return true;
	while(feof(ifile)==0)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//   Note that we require "empty" list elements to use proper syntax (...),(),(...)
		//   but (...),,(...) is treated the same as (...),(...)
		//2. {			i.e. the beginning of a new BACnetDestination in the list
		//3. }			i.e. the closing part of the list
		print_debug("PDL: start loop lp = '%s'\n",lp);
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0)
			if (ReadNext()==NULL) break;		//									***008
		if (*lp==')') 
		{	lp++;
			break;								//close this list out
		}

		if (MustBe('{')) break;

		//here we have days,from,to,(recipient),procid,conf,transitions),...

		if ((q=(tagDestination *)malloc(sizeof(BACnetDestination)))==NULL)
		{	tperror("Can't Get Object Space!",true);
			break;
		}
		print_debug("LJT: Destination=%x\n",q);

		//look for (days of week),
		skipwhitespace();						//									***013
		print_debug("PDL: About to check weekdays\n");
		q->valid_days=0;
		// test if days of week are entered as (T,F,T,F,F,T,T) or (MON,WED,SAT,SUN)
		if (normalBitstring(lp))
		{
			unsigned char d = 0;
			ParseBitstring(&q->valid_days, 7, &d);
		}
		else
		{
			// special bitstring handling for days of week
			if (MustBe('(')) break;
			print_debug("PDL: pre while 1\n");
			while(*lp&&*lp!=')')
			{   
				while (*lp==space||*lp==',') lp++;	//skip separation between list elements
				print_debug("PDL: post while 2 lp = '%s'\n",lp);
				if (*lp==')') break;				//done								***013
				found_day = 0;  // MAG
	    		for(i=0;i<7;i++)
	    			if (strnicmp(lp,DOWNames[i],3)==0)
	    			{	q->valid_days|=(octet)(0x80>>i);	//Monday is 80, Sunday is 2
						while (*lp&&*lp!=' '&&*lp!=','&&*lp!=')') lp++;	//find delim	***014
						found_day = 1;
	    				break;
	    			}
				print_debug("PDL: post while 3 lp = '%s'\n",lp);
				if(!found_day)
					return tperror("Expected Day of Week Here",true);

			}
			print_debug("PDL: Past check weekdays\n");
			if (MustBe(')')) break;
		}	
		if (strdelim(",")==NULL) break;
		ParseTime(&q->from_time);
		print_debug("PDL: Past min-sec-hsec\n");

		if (strdelim(",")==NULL) break;
		ParseTime(&q->to_time);
		print_debug("PDL: Past min-sec-hsec 2\n");
		
		if (strdelim(",")==NULL) break;
		if (ParseRecipient(&q->recipient)==NULL) break;
		
		if (strdelim(",")==NULL) break;
		q->process_id=ReadW();
		q->notification=ReadBool();
		skipwhitespace();
		if (MustBe('(')) break;
		q->transitions=0;
		dm=0x80;
		while (c=*lp++)
		{	if (c==')') break;
			if (c==',') dm>>=1;
			if (c=='t'||c=='T') q->transitions|=dm;
		}
		skipwhitespace();
		if (MustBe('}')) break;
		q->next=p;								//link onto the list
		p=q;
		q=NULL;
		print_debug("PDL: End of loop\n");
	}
	if (q!=NULL) free(q);						//don't lose this block!
	*dalp=p;
	print_debug("PDL: Return\n");
	return false;
}

///////////////////////////////////////////////////////////////////////
//	read a list of BACnetRecipient from the buffer lp points to
//	((device,instance) or (network,macaddr),...)
//in:	lp		points to current position in buffer lb
//		dalp	points to a BACnetRecipient pointer variable to be
//				initialized to point to the created list of BACnetRecipients
//out:	true	if an error occurred
//		lp		points past the delimiter ) unless it was the end of the buffer

BOOL ParseRecipientList(BACnetRecipient **dalp)
{	BACnetRecipient	*p=NULL,*q=NULL;
				
	*dalp=NULL;									//initially there is no list
	
	skipwhitespace();
	if (MustBe('(')) return true;
	while(feof(ifile)==0)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//   Note that we require "empty" list elements to use proper syntax (...),(),(...)
		//   but (...),,(...) is treated the same as (...),(...)
		//2. (			i.e. the beginning of a new BACnetRecipient in the list
		//3. )			i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0)
			if (ReadNext()==NULL) break;		//									***008
		if (*lp==')') 
		{	lp++;
			break;								//close this list out
		}
        skipwhitespace();
		if (MustBe('{')) break;
		if ((q=ParseRecipient(NULL))!=NULL)
		{	q->next=p;							//link onto the list
			p=q;
			q=NULL;
		}
        skipwhitespace();
		if (MustBe('}')) break;
	}
	if (q!=NULL) free(q);						//don't lose this block!
	*dalp=p;
	return false;
}

///////////////////////////////////////////////////////////////////////
//	read a BACnetRecipient from the buffer lp points to
//	(device,instance) or (network,macaddr)
//in:	lp		points to current position in buffer lb
//out:	NULL	if an error occurred
//		else	pointer to newly created BACnetRecipient
//		lp		points past the delimiter ) unless it was the end of the buffer

BACnetRecipient *ParseRecipient(BACnetRecipient *inq)
{	BACnetRecipient	*q=NULL;
	dword				dw;
	
	skipwhitespace();
    
	if (inq==NULL)
	{   if ((q=(tagRecipient *)malloc(sizeof(BACnetRecipient)))==NULL)
		{	tperror("Can't Get Object Space!",true);
			return NULL;
		}
		print_debug("LJT: Recipient=%x\n",q);

	}
	else
		q=inq;
	q->next=NULL;

	//here we have (device,instance) or network,macaddr ...
	if (IsDigit(*lp))						//must be network,macaddress
	{	
		q->choice=1;						//address
        q->u.address.network_number=ReadW();
        if (ParseOctetstring(&q->u.address.mac_address[0],
        					 sizeof(q->u.address.mac_address),
        					 &q->u.address.address_size)) goto brfail;
	}
	else									//must be (device,instance)
	{	
		dw=ReadObjID();
		if ((word)(dw>>22)!=DEVICE)
		{	if (tperror("Must use a Device Object Identifier here!",true))
				goto brfail;
		}
		q->choice=0;						//device
		q->u.device=dw;
	}
	return q;

brfail:
	if (inq==NULL) free(q);				//don't release unless we malloc'd it
	return NULL;
}


//*****018 begin
///////////////////////////////////////////////////////////////////////
//	read a list of BACnetCOVSubscription from the buffer lp 
//in:	lp		points to current position in buffer lb
//		dalp	points to a BACnetCOVSubscription pointer variable to be
//				initialized to point to the created list of BACnetCOVSubscription
//out:	true	if an error occurred
//		lp		points past the delimiter ) unless it was the end of the buffer
BOOL ParseCOVSubList(BACnetCOVSubscription **covsub)
{
	BACnetCOVSubscription	*p=NULL,*q=NULL;
				
	*covsub=NULL;									//initially there is no list
	
	skipwhitespace();
	if (MustBe('(')) return true;
	while(feof(ifile)==0)
	{   //here lp must point to:
		//1. a comma or whitespace which we ignore as a separator between list elements.
		//   Note that we require "empty" list elements to use proper syntax (...),(),(...)
		//   but (...),,(...) is treated the same as (...),(...)
		//2. (			i.e. the beginning of a new BACnetRecipient in the list
		//3. )			i.e. the closing part of the list
		while (*lp==space||*lp==',') lp++;		//skip separation between list elements
		if (*lp==0)
			if (ReadNext()==NULL) break;		//									***008

		if (*lp==')') 
		{	lp++;
			break;								//close this list out
		}
			
		if ((q=ParseCOVSubscription(NULL))!=NULL)
		{	q->next=p;							//link onto the list
			p=q;
			q=NULL;
		}
	}
	if (q!=NULL) free(q);						//don't lose this block!
	*covsub=p;
	return false;
}

///////////////////////////////////////////////////////////////////////
//	read a BACnetCOVSubscription from the buffer lp points to
//	(RecipientProcess, ObjectPropertyReference, Boolean, Unsigned) 
//in:	lp		points to current position in buffer lb
//out:	NULL	if an error occurred
//		else	pointer to newly created BACnetCOVSubscription
//		lp		points past the delimiter ) unless it was the end of the buffer
BACnetCOVSubscription *ParseCOVSubscription(BACnetCOVSubscription *inq)
{
	BACnetCOVSubscription	*q=NULL;
//	(((0, (Device, Instance 12)), 300),	((Analog Input, 1), Present-Value), TRUE, 100, 1.0)
	skipwhitespace();
	//here we have (RecipientProcess, ObjectPropertyReference, Boolean, Unsigned) ...
	if (MustBe('{')) return NULL;
	
	if (inq==NULL)
	{   
		if ((q=(tagCOVSubscription *)malloc(sizeof(BACnetCOVSubscription)))==NULL)
		{	
			tperror("Can't Get Object Space!",true);
			return NULL;
		}
		print_debug("LJT: COVSubscription=%x\n",q);

	}
	else{
		q=inq;
	}
		
	q->next=NULL;
	skipwhitespace();

	//parse RecipientProcess
	//here we have (Recipient, Unsigned) ...
	//	(((0, (Device, Instance 12)), 300)
	if (MustBe('{')) goto brfail;
	skipwhitespace();
	if ( ParseRecipient(&(q->recipient.recipient)) == NULL ) {
		goto brfail;
	}
    
	if ((strdelim(","))==NULL) goto brfail;
	skipwhitespace();
	q->recipient.process_id = ReadW();
    lp--;
	if (MustBe('}')) goto brfail;

	if ((strdelim(","))==NULL) goto brfail;
	skipwhitespace();
	// parse ObjectPropertyReference
	//	((Analog Input, 1), Present-Value)
	if ((ParseReference(&(q->monitoredPropertyReference)))==NULL)	goto brfail;
		
	// parse boolean
	if ((strdelim(","))==NULL) goto brfail;
	skipwhitespace();
	q->notification = ReadBool();
	lp--;

	// parse unsigned
	if ((strdelim(","))==NULL) goto brfail;
	skipwhitespace();
	q->timeRemaining = ReadW();	
	lp--;

	// parse real
	skipwhitespace();
	if ( *lp==',' )   // meaning there is a COV increment
	{
		if ((strdelim(","))==NULL) goto brfail;
		skipwhitespace();
		q->covIncrement = (float)atof(lp);
	}
	if ((strdelim("}"))==NULL) goto brfail;
	return q;
	
brfail:
	if (inq==NULL) free(q);				//don't release unless we malloc'd it
	return NULL;
}
//*****018 end


///////////////////////////////////////////////////////////////////////
//  5/13/05 Shiyuan xiao. Support ASHRAE Standard 135.1-2003
//	parse a string parameter
//in:	p		points to string variable to contain the result
//		ps		max size of p
//		param	points to parameter to be parsed
//
//The param pointer should point to a string like:
//		"some string"
//or	'some string'
//or	`somestring'
//out:	true		if cancel selected

BOOL setstring(char *p,word ps, char *param)
{
	char	q;
	word	i;

	// TODO: a GLOBAL pointer???  Is its value used after we return?
	// Search shows that ALL calls to setstring pass lp as param...
	lp = param;									//									***008
	skipwhitespace();
	q = *lp++;
	if ((q == doublequote) || (q == singlequote) || (q == accentgrave))
	{
		// 135.1 clause 4.3 says that strings
		// - may be enclosed by double quotes
		// - may be enclosed by single quotes
		// - may be enclosed between an accent grave and a single quote
		// - ends at the end of the line, regardless of quoting
		if (q == accentgrave)
			q = singlequote;
		
		for (i = 0; i < (ps-1); i++)			//copy until end of line, end of string or ps chars copied
		{
			if (*lp == q)
			{	
				lp++;							//skip trailing quote
				break;
			}
			else if (*lp == 0 || *lp == 0x0a)	//0x0a, the end of the line
			{
				break;							//found end of line
			}
			else
			{
				*p++ = *lp++;
			}
		}

		*p=0;									//mark end with asciz
		
		return false;
	}
	
	return tperror("Expected string parameter here",true);
}

///////////////////////////////////////////////////////////////////////
//	read a non-zero dword from the buffer lp points to
//in:	lp		points to current position in buffer lb
//out:	0		if end of buffer
//		else	the number
//		lp		points past the delimiter unless it was the end of the buffer

dword ReadDW()
{	dword	d=0;
	char	c;

	skipwhitespace();
	while( IsDigit( c = *(lp++) ) )  
	{
		d = (d*10L)+(c-'0');	
	}
	
	if (c=='?') c=*lp++;						//pretend ? is a valid digit
	if (c==0) lp--;								//stick at end of buffer

// ideally we want this to leave any closing or delimeters but too many places 
// using this function are taking pains to work around this bug  LJT
//	if (c==','||c==')'||c=='}')
//		lp--;   // don't eat the last character  LJT
	return d;
}


///////////////////////////////////////////////////////////////////////
//	read a word from the buffer lp points to
//in:	lp		points to current position in buffer lb
//out:	0		if end of buffer
//		else	the number
//		lp		points past the delimiter unless it was the end of the buffer

word ReadW()
{	dword d;

	d=ReadDW();
	if (d>65535)
	{	lp--;
		tperror("Must use an Unsigned Integer 0..65535 here!",true);
		d=0;
	}
	return (word)d;
}                

///////////////////////////////////////////////////////////////////////
//	read a byte from the buffer lp points to
//in:	lp		points to current position in buffer lb
//		lb,ub	range of acceptable values
//out:	255		if out of range
//		else	the number
//		lp		points past the delimiter unless it was the end of the buffer

octet ReadB(octet lb,octet ub)
{	octet d=0;
	char b[64],c;

	print_debug("RB: Enter ReadB lp = '%s'\n",lp);
	skipwhitespace();
	while (IsDigit(c=*lp++))
		d=(d*10)+(c-'0');
	if (c=='?') 
	{	c=*lp++;								//pretend ? is a valid digit
		d=dontcare;
	}
	//Added by xuyiping 2002-8-29
	if (c=='*') 
	{	c=*lp++;								//pretend * is a valid digit
		d=dontcare;
	}
	if (c==0) lp--;								//stick at end of buffer
	if (d!=dontcare)
	{	if(d<lb||d>ub)
		{	lp--;
			sprintf(b,"Must use an Unsigned Integer %u..%u here!",lb,ub);
			tperror(b,true);
			d=dontcare;
		}
	}
	return d;
}                

///////////////////////////////////////////////////////////////////////
//	read a boolean (true/false) from the buffer lp points to
//in:	lp		points to current position in buffer lb
//out:	0/1
//		lp		points past the delimiter unless it was the end of the buffer

octet ReadBool()
{	char	q;
	octet	v=0;
	
	while (q=*lp++)
	{	if (q==')'||q=='}'||q==','||q==' ') break;    // msdanner 9/2004, added space delimeter
		if (q=='t'||q=='T') v=1;
	}
	return v;
}

///////////////////////////////////////////////////////////////////////
//	read an enumeration name from the buffer lp points to
//in:	lp			points to current position in buffer lb
//		enumtable	table of pointers to enumeration names (tsize of them)
//		tsize		number of names in the table
//		firstprop	the index of the first proprietary value, or 0 meaning not extensible
//out:	0xFFFF		if end of buffer or invalid enumeration
//		else		the enumeration (note that ? results in enumeration 0)
//		lp			points past the delimiter unless it was the end of the buffer

word ReadEnum(etable *etp)
{	char	c,e[33];
	word	i;
	int j;  // MAG

	print_debug("RE: Enter ReadEnum lp = '%s'\n",lp);
	skipwhitespace();
	i=0;
	while (c=*lp)								//until the end of the buffer
	{	lp++;									//advance scan
		if (c=='_'||c==space) c='-';			//convert underscore or space to dash	***006
		if (c=='?'||c=='-'||IsAlnum(c))			//if its a valid part of an enumeration name
		{	if (i<32) e[i++]=c;}				//save this character
		else									//found the delimiter
			break;
	}
    e[i]=0;  //add by Liangping Xu,2002-8
	print_debug("RE: find enum '%s'\n",e);
	
	j = strlen(e);
	print_debug("RE: spot j-2 = %d spot j-1 = %d\n",e[j-2],e[j-1]);
	if((j > 3)&&(e[j-2] == '-')&&((e[j-1] = 'w')||(e[j-1] = 'W'))){ // MAG 3 MAR 2001 fix for string ending in '-w' (writeable)
		print_debug("RE: fix end string: was '%s'",e);
		lp -=2;
		e[j-2] = 0;
		print_debug(" now '%s'\n",e);
	}

	if (i)
	{	e[i]=0;									//always leave asciz in buffer
		if (e[0]=='?') return 0;				//? defaults to enumeration 0
		for (i=0;i<etp->nes;i++)
		{	
			if (etp->estrings[i])				//make sure it's not null			***006
				if (stricmp(e,etp->estrings[i])==0)
				{	//matching enumeration
					print_debug("RE: find match (%d)- return\n",i);
					return i;
				}
		}
		if (stricmp(e,"unknown")==0)
		{
			i=(word)ReadDW();
			return i;
		}
		if (stricmp(e,"proprietary")==0)
		{	
			if (etp->propes)
			{	
				if ((i=(word)ReadDW())>=etp->propes)
					return i;
				tperror("Proprietary enumeration cannot use the reserved range for this property!",true);
			}
			else
				tperror("This is not an extensible enumeration!",true);
			return 0xFFFF;
		}
	}	
	print_debug("RE: Return w/o enumeration\n");
	tperror("Expected an Enumeration Name here!",true);
	return 0xFFFF;
}

///////////////////////////////////////////////////////////////////////
//	read an object identifier (objecttype,instance) from the buffer lp points to
//in:	lp			points to current position in buffer lb
//		objid		points to the dword to receive the object identifier
//out:	badobjid	if it was an invalid object identifier,
//		else		the object identifier
//		lp			points past the delimiter unless it was the end of the buffer

dword ReadObjID()
{	word	objtype;
	dword	id;

	print_debug("ROI: Enter ReadObjID\n");
	skipwhitespace();
	if (*lp++!='(')
	{	tperror("Expected ( before (objecttype,instance) here!",true);
		goto roidx;
	}

	// 5/19/2005 Shiyuan Xiao. Support proprietary object
	if( strnicmp(lp, "proprietary", strlen("proprietary")) == 0 )
	{
		lp += strlen("proprietary");
		skipwhitespace();
		objtype = ReadW();		
	}
	else 
	if ((objtype=ReadEnum(&etObjectTypes))==0xFFFF)
	{
		goto roidx;
	}
	
	print_debug("ROI: object type %s %d\n",etObjectTypes.estrings[objtype], objtype);
	skipwhitespace();						//									***006 Begin
	if (strnicmp(lp,"instance ",9)==0)		//ignore instance here
		lp+=9;									//									***006 End
	id=ReadDW();
	print_debug("ROI: object instance %d\n",id);
	if (lp[-1]==')')							//it ended with a closing paren, it's ok
	{	if (id<(1L<<22))						//valid instance number
			return (((dword)objtype)<<22)+id;	//save the object identifier as a dword
		tperror("Object Instance must be 0..4194303!",true);
		goto roidx;
	}	
	tperror("Expected ')' after instance here!",true);

roidx:
	return badobjid;
}

///////////////////////////////////////////////////////////////////////
//	find the next non-whitespace in a file
//in:	lp		points to current position in buffer lb
//		ifile	file stream 
//out:	NULL	if eof,
//		else	pointer to next non-whitespace
//				(in this case lp also points to it)

char *ReadNext()								//										***008
{
	do
	{	if (feof(ifile)) return NULL;			//end of file							***008
		readline(lb,sizeof(lb));				//read a line from the file 			***008
		lp=&lb[0];
		skipwhitespace();
	} while (*lp==0);							//this was a blank line
	return lp;
}

///////////////////////////////////////////////////////////////////////				***008 Begin
//	find the end of whitespace in a string
//in:	lp		points to string 
//out:	lp		points to first non-whitespace char

void skipwhitespace()
{	while (*lp==space) lp++;
	if (*lp==0) ReadNext();						//									***008 End
}

///////////////////////////////////////////////////////////////////////
//	find the first element of an array or list after whitespace{whitespace
//in:	p		points to string 
//out:	points to first non-whitespace char or NULL if no { was found

char *openarray(char *p)
{	BOOL	foundlb=false;

	while (*p==space||*p=='{')
	{	if (*p=='{')
			if (foundlb)
				break;							//treat second { as "first element"
			else
				foundlb=true;					//well, we found one
		p++;									//skip it
	}
	if (foundlb)
		return p;
	else
	{	tperror("Expected { here...",true);
		return NULL;							//didn't find {
	}
}

///////////////////////////////////////////////////////////////////////
//	Create a TPI file
//in:	tp		points to file name string
//out:	0		success
//		else	error code

int CreateTextPICS(char *tp)
{	FILE	*f;
	
	if ((f=fopen(tp,"w"))==NULL) return errno;	//return system error code for it
    return 0;
}

///////////////////////////////////////////////////////////////////////
//	Read a line from a text file, squishing out redundant white space and comments
//in:	lp		points to a line buffer
//		lps		size of buffer lp
//		ifile	file stream to read from
//out:	lp		filled with the line, ends in 0
//		lc		updated

static void readline(char *lp,int lps)
{	char	*dp,*sp,c;
	BOOL	HaveNonWS=FALSE;

	fgets(lp,lps,ifile);						//get a line from the file			***008
	dp=sp=lp;
	while (*sp)
	{	switch(c=*sp++)
		{
		case space:
		case tab:
		case cr:
		case lf:
			while (c=*sp)
		  	  if (c==space||c==tab||c==cr||c==lf) 
		  		sp++; 							//skip white space
		  	  else
		  		break;
			if (*sp==0) goto rlexit;			//we're done
			if (HaveNonWS)
				*dp++=space;					//convert a white space sequence to just a single space
			break;
		case '-':								//comment?
			if (*sp=='-') goto rlexit;			//yes, ignore to the end
			*dp++=c;
			HaveNonWS=TRUE;
			break; 
		case accentgrave:
			//c=singlequote;						//matching quote is singlequote
			//goto rlquote;
			// 5/13/05 shiyuan xiao. 135.1-2003
		case doublequote:
		case singlequote:
			*dp++=c;
			while (*sp&&c!=*sp)
				*dp++=*sp++;
			*dp++=c;
			if (*sp) sp++;						//consume closing quote unless it's the end of the line
			HaveNonWS=TRUE;
			break;
		default:
			*dp++=c;
			HaveNonWS=TRUE;
		}
	}
rlexit:
	*dp=0;										//mark the end with asciz

	// madanner 6/03:  Problem leaving an extra space on the valid line if 
	// a comment exists further down the line... so just trim it off
	rtrim(lp);

	lc++;										//bump line count
//	printf("%.3u:%s\n",lc,lp);					//*** DEBUG ***
}

/////////////////////////////////////////////////////////////////////// 
//	Find the next comma or EOS
//in:	p	points to the beginning of the string to look in
//out:	p	unchanged, but the comma (if found) is changed to asciz
//		returns pointer past asciz if comma was found or NULL if EOS

char *Nxt(char *p)
{	char *q;									//temp pointer
	if ((q=strchr(p,','))!=NULL)				//got a comma
		*q++=0x00;								//make it asciz and point past it	
	return q;									//return pointer to next string
}

///////////////////////////////////////////////////////////////////////			***008 Begin 
//	Find the next delimiter
//in:	lp	points to the beginning of the string to look in
//		d	points to the set of delimiter characters
//out:	returns pointer past delimiter if one was found or NULL if EOF
//		lp	is also that pointer

char *strdelim(char *d)
{	char *q,*dq;								//temp pointers

	while(feof(ifile)==0)
	{	dq=d;									//point to list of delimiters
		while (*dq&&*lp)						//for each delimiter character	***006
		{	if ((q=strchr(lp,*dq++))!=NULL)		//got a delimiter
			{	lp=q+1;
				return lp;						//return pointer past delimiter
			}
		}
		ReadNext();
	}
	return NULL;								//not found						***008 End
}

///////////////////////////////////////////////////////////////////////			***006 Begin 
//	Trim whitespace off the end of a string
//in:	p	points to the beginning of the string to look in

void rtrim(char *p)
{	
	//madanner 6/03: This function never actually worked... 
	//so, let's do it again.

	for ( char * q = p + strlen(p) - 1; q >= p && (*q == ' ' || *q == 0x09); q-- )
		*q = 0;

//	char *q;									//temp pointer
//	q=strchr(p,0);								//find the asciz
//	while (p!=q)
//	{	if(*q==' '||*q==0x09)
//			*q--=0;
//		else
//		  return;
//	}
}												//								***006 End

///////////////////////////////////////////////////////////////////////			***020 Begin
//	Preprocess a string before parsing, 
//  for example: replace score or underscore with space.
//  other processes can be added if necessary
//in:	str		points to the beginning of the string to be modified

void preprocstr(char *str)
{	
	char* p;
	while (p = strchr(str, '-')) {
		*p = ' ';
	}
	while (p = strchr(str, '_')) {
		*p = ' ';
	}
	
}																			// ***020 End

/////////////////////////////////////////////////////////////////////// 
//	Convert HEX chars to binary octet
//in:	src		points to 2 hex chars
//		dst		points to octet to receive the value
//out:	ptr to 1st non-hex char, or 2 past src
char *cvhex(char *src,octet *dst)
{	if (!IsXDigit(*src))
	{	*dst=0;									//assume none
		return src;
	}
	if (IsDigit(*src))
		*dst=*src-'0';
	else
		*dst=(*src&0xDF)-55;
	src++;
	if (!IsXDigit(*src)) return src;
	if (IsDigit(*src))
		*dst=(*dst<<4)+(*src-'0');
	else
		*dst=(*dst<<4)+((*src&0xDF)-55);
	src++;
	return src;
}

/////////////////////////////////////////////////////////////////////// 
//	check for a character during parsing and complain if not found
//in:	c			the expected character
//		lp			points to where it should be
//out:	true		if error and cancel selected (also sets cancel=true)
BOOL MustBe(char c)
{	char	b[20];

	if (*lp++!=c)
	{	lp--;
		sprintf(b,"Expected %c here!",c);
		return tperror(b,true);
	}
	return false;
}

/////////////////////////////////////////////////////////////////////// 
//	display an error message dialog and update the error count
//in:	mp			points to specific message
//		showline	true if need to show source line and position
//out:	true		if cancel selected (also sets cancel=true)

BOOL tperror(char *mp,BOOL showline)
{	static char m[512];
	char 		*p,c;
	
	m[0]=0;
	if ( lPICSErr == -1 )
		lerrc++;

	// madanner 6/03: add error title to account for usage of afx (no title beyond VTS)
	if ( pfileError == NULL )
		strcpy(m, "Read Text PICS Error:\n\n");

	p=strchr(m,0);

   // msdanner 9/2004: if logging consistency errors, include the error number
   // on each line.
   if ( lPICSErr > -1)
   {
      p += sprintf(p, "%d) ", lPICSErr+1); // plus 1 because it has not been incremented yet
   }

	if (showline)
	{
		sprintf(p,"Line %u: ",lc);				//add line number first
		p=strchr(m,0);								//find asciz
	}

	if ( pfileError == NULL || !showline )
		sprintf(p,"%s\n",mp);
	else
		sprintf(p,"%s:  ",mp);

	if (showline)
	{	p=strchr(p,0);							//find asciz
//		lp--;									//back up by one
		c=*lp;									//save the character we "broke" on
		*lp++=0;								//make it asciz there temporarily
		sprintf(p,"%s<%c>%s\n",lb,c,lp);
	}

//	MessageBeep(MB_ICONEXCLAMATION);
//	cancel=(MessageBox(NULL,m,"Read Text PICS Error",  MB_ICONEXCLAMATION|(showline?MB_OKCANCEL:MB_OK))==IDCANCEL)?TRUE:FALSE;

	if ( pfileError == NULL )
		cancel = AfxMessageBox(m,  MB_ICONEXCLAMATION | (showline ? MB_OKCANCEL : MB_OK)) == IDCANCEL;
	else
		PrintToFile(m);

	// deal with silly MS BOOL vs. bool
	return cancel ? TRUE : FALSE;
}

// msdanner 9/04 added:  
// Returns the maximum number of standard services
int GetStandardServicesSize()
{
   return sizeof(StandardServices)/sizeof(StandardServices[0]);
}

// msdanner 9/04 added:  
// Returns a string representing the standard service indexed by i 
char *GetStandardServicesName(int i)
{
	return StandardServices[i];
}

// msdanner 9/04 added:  
// Returns the maximum number of potential BIBBs
int GetBIBBSize()
{
	return MAX_BIBBS;
}

// msdanner 9/04 added:  
// Returns a string representing the BIBB corresponding to index i 
char *GetBIBBName(int i)
{
	return BIBBs[i].name;
}

// msdanner 9/04 added:  
// Returns the maximum number of potential Object Types
int GetObjectTypeSize()
{
   return sizeof(StandardObjects)/sizeof(StandardObjects[0]);
}

// msdanner 9/04 added:  
// Returns a string representing the Object Type indexed by i 
char *GetObjectTypeName(int i)
{
	return StandardObjects[i];
}

// msdanner 9/04 added:  
// Returns the maximum number of potential Data Link Options
int GetDataLinkSize()
{
	return  sizeof(StandardDataLinks)/sizeof(StandardDataLinks[0]);
}

// msdanner 9/04 added:  
// Returns a string representing the Data Link Option indexed by i 
// and any baud rate options.
// Caller must allocate the memory to hold the resulting string
// and pass in a pointer to the allocation in pstrResult.
// Allocation should be at least 200 bytes
void GetDataLinkString(int i, PICSdb *pd, char *pstrResult)
{
	int j;
	char *pz;
    dword *dp;

	if (!pstrResult)
		return;

	// Construct the first part of the string, the data link name
	pz = pstrResult;
	pz += sprintf(pz, "%s", StandardDataLinks[i]);   // get Data Link name 

	// append optional baud rates 
	switch (i)
	{
  	case 9:								//MS/TP master
  		dp=&pd->MSTPmasterBaudRates[0];
		break;
	case 10:                            //MS/TP slave
		dp=&pd->MSTPslaveBaudRates[0];  
		break;
 	case 11:							//PTP 232
		dp=&pd->PTP232BaudRates[0];
		break;
  	case 12:							//PTP modem, fixed baud rates
		dp=&pd->PTPmodemBaudRates[0];
		break;
	}

	// If list of fixed baud rates, append them to the string
	if ( (i >= 9) && (i <= 12) )
	{
		for (j=0; (j<16) && dp[j]; j++)
		{
		   // If this is the first one
		   if ( j == 0 )
  		      pz += sprintf(pz, ": %u", dp[j]);  // add first baud rate
		   else
              pz += sprintf(pz, ", %u", dp[j]);  // else, add another one
		}
	}
	else if ( i == 13 )  // PTP auto-baud range - only 2 baud rates to add
	{
		pz += sprintf(pz, ": %u to %u",
			pd->PTPAutoBaud[0],
			pd->PTPAutoBaud[1] );
	}
    return;

}

// msdanner 9/04 added:  
// Returns the maximum number of potential Character Sets
int GetCharsetSize()
{
	return  sizeof(Charsets)/sizeof(Charsets[0]);
}

// msdanner 9/04 added:  
// Returns a string representing the Charset matching csTag 
char *GetCharsetName(octet csTag)
{
	int j;
	for (j=0;j<(sizeof(Charsets)/sizeof(Charsets[0]));j++)
	{
		if ((int)(Charsets[j].octetcons) == csTag)
			return Charsets[j].name;
	}
	return NULL;
		
}

// msdanner 9/2004 - function to expand a packed bitstring into an array
// of ocetes, with each octet being a 0 or 1 depending on the bitstring
void ExpandBitstring(octet *pExpandedResult, octet *pBitstring, int nBits)
{
   octet mask;
   while (nBits)
   {
      for (mask = 0x80; mask && nBits; mask>>=1, nBits--)
      {
         *pExpandedResult++ = ((*pBitstring & mask) != 0);
      }
      pBitstring++;  // next 8 bits
   }
}


// msdanner 9/04 added:  
// Returns the maximum number of Special Functionality choices
int GetSpecialFunctionalitySize()
{
	return  sizeof(SpecialFunctionality)/sizeof(SpecialFunctionality[0]);
}

// msdanner 9/04 added:  
// Returns a Special Functionality string at index i 
char *GetSpecialFunctionalityName(int i)
{
	return SpecialFunctionality[i];
}

//////////////////////////////////////////////////////////////////////////////
// msdanner 9/04 added:  
// Run all EPICS Consistency checls specified by standard 135.1-2003.
//
// The older EPICS consistency checks based on conformance class
// and functional groups have been replaced by checks that are based on BIBBs.
// Some of the older tests are still relevant and are launched from this 
// new function.
//
// 5.  EPICS CONSISTENCY TESTS
// 
// Each implementation shall be tested to ensure consistency among
// interrelated data elements. These tests shall include:
// 
// (a)	All object types required by the specified BIBBs shall be indicated
// as supported in the Standard Object Types Supported section of the
// EPICS. (Similar to the old test "A", but using BIBBs instead of
// conformance class.)
// 
// (b)	A minimum of one instance of each object type required by the
// specified BIBBs shall be included in the test database. (Similar to the
// old test "B", but using BIBBs instead of conformance class.)
// 
// (c) The Object_Types_Supported property of the Device object in the test
// database shall indicate support for each object type required by the
// supported BIBBs.  (Similar to the old test "C", but using BIBBs instead
// of conformance class.)
// 
// (d) All application services required by the supported BIBBs shall be
// indicated as supported in the BACnet Standard Application Services
// Supported section of the EPICS with Initiate and Execute indicated as
// required by the supported BIBBs. (Similar to the old test "D", but using
// BIBBs instead of conformance class.)
// 
// (e) The Application_Services_Supported property of the Device object in
// the test database shall indicate support for each application service
// for which the supported BIBBs requires support for execution of the
// service. (Similar to the old test "E", but using BIBBs instead of
// conformance class.)
// 
// (f) The object types listed in the Standard Object Types Supported
// section of the EPICS shall have a one-to-one correspondence with object
// types listed in the Object_Types_Supported property of the Device object
// contained in the test database.	(Identical to the old test "K".)
// 
// (g) For each object type listed in the Standard Object Types Supported
// section of the EPICS there shall be at least one object of that type in
// the test database. (Identical to the old test "L".)
// 
// (h) There shall be a one-to-one correspondence between the objects
// listed in the Object_List property of the Device object and the objects
// included in the test database. The Object_List property and the test
// database shall both include all proprietary objects. Properties of
// proprietary objects that are not required by BACnet Clause 23.4.3 need
// not be included in the test database. (Identical to the old test "M".)
// 
// (i) For each object included in the test database, all required
// properties for that object as defined in Clause 12 of BACnet shall be
// present. In addition, if any of the properties supported for an object
// require the conditional presence of other properties, their presence
// shall be verified. (Identical to the old test "N".)
// 
// (j) For each property that is required to be writable, that property
// shall be marked as writable in the EPICS. (New test.)
// 
// (k) The length of the Protocol_Services_Supported bitstring shall have
// the number of bits defined for BACnetProtocolServicesSupported for the
// IUT's declared protocol revision.  (New test.)
// 
// (l) The length of the Protocol_Object_Types_Supported bitstring shall
// have the number of bits defined for BACnetObjectTypesSupported for the
// IUT's declared protocol revision. (New test.)
// 
void CheckPICSConsistency2003(PICSdb *pd)
{
   cConsistencyErrors=0;  // Reset global PICS error count. 

   // Make sure that each Object Type required by a BIBB
   // exists in the Standard Object Types Supported section, 
   // as well as cross dependencies between BIBBs.
   // 135.1-2003, section 5.(a)
   CheckPICSCons2003A(pd);

   // 135.1-2003, section 5.(b) is covered by test (g) below

   // 135.1-2003, section 5.(c) is covered by test (f) below

   // 135.1-2003, section 5.(d) test
   // Make sure application services required by each BIBB are
   // listed in the Standard Application Services Supported section. 
   CheckPICSCons2003D(pd);

   // 135.1-2003, section 5.(e) test
   // Make sure the services marked "Execute" in the 
   // 'BACnet Standard Application Services Supported' section match
   // the services marked supported in the Application_Services_Supported
   // Property of the Device Object. 
   // This is a two-way check.
   CheckPICSCons2003E(pd);

	// 135.1-2003, section 5.(f) 
   // Make sure the Objects listed in the Standard Object Types section
   // match the bits in the Objects_Types_Supported property of the Device.
   CheckPICSCons2003F(pd);


	// Make sure each Object type in the Standard Object Types Supported section
   // is represented in the test database.
	// 135.1-2003, section 5.(b) & 5.(g) 
   CheckPICSCons2003G(pd);

	// 135.1-2003, section 5.(h) 
   CheckPICSCons2003H(pd);

	// 135.1-2003, section 5.(i) 
	// 135.1-2003, section 5.(j) 
   CheckPICSCons2003I(pd);

	// 135.1-2003, section 5.(k) 
   CheckPICSCons2003K(pd);

	// 135.1-2003, section 5.(l) 
   CheckPICSCons2003L(pd);

}


/////////////////////////////////////////////////////////////////////////////
// Some BIBBs require support for other BIBBs.
// This is a helper function to perform that cross check.
// iSupportedBIBB = the BIBB that requires another BIBB.
// iDependentBIBB = the BIBB that iSupportedBIBB requires.
//
void CheckPICS_BIBB_Cross_Dependency(PICSdb *pd, int iSupportedBIBB, int iDependentBIBB)
{
   char opj[200];
   if ( !pd->BIBBSupported[iDependentBIBB] )
   {
     sprintf(opj,"BIBB %s requires support for BIBB %s.\n", 
          BIBBs[iSupportedBIBB].name, 
          BIBBs[iDependentBIBB].name );
     tperror(opj,false);
   }
   return;
} 



///////////////////////////////////////////////////////////////////////////////
// EPICS Consistency test specified by 135.1-2003, section 5(a)
//
// (a)	All object types required by the specified BIBBs shall be indicated
// as supported in the Standard Object Types Supported section of the
// EPICS. (Similar to the old test "A", but using BIBBs instead of
// conformance class.)
// 
// This function also runs checks for cross-BIBB dependencies,
// which is not a test that is specified in 135.1-2003 section 5, 
// but is clearly stated in the BIBB definition section of the BACnet standard.
// 
// In addition, this function also checks for specific properties
// that must be present for certain BIBBs. TODO
//  
void CheckPICSCons2003A(PICSdb *pd)
{ 
   int i;
   char opj[300];
   for (i = 0; i < MAX_BIBBS; i++ )
   {
      // scan for supported BIBBs
      if (pd->BIBBSupported[i])
      {
         switch (i)
         {
            case bibbAE_N_I_B:  // requires support for either NC or EE Objects
               if ( (pd->BACnetStandardObjects[NOTIFICATIONCLASS]==soNotSupported) &&
                    (pd->BACnetStandardObjects[EVENT_ENROLLMENT]==soNotSupported) )
               {
                 sprintf(opj,"135.1-2003 5.(a): "
                   "BIBB AE-N-I-B requires support for Intrinsic or Algorithmic reporting, "
                   "which implies support for the Event Enrollment or Notification Class object "
                   "in the \"Standard Object Types Supported\" section.\n");
                 tperror(opj,false);
               }  
               break;

            case bibbAE_N_E_B: // requires support for AE-N-I-B & DS-RP-A & EE Object
               CheckPICS_BIBB_Cross_Dependency(pd,i,bibbAE_N_I_B); 
               CheckPICS_BIBB_Cross_Dependency(pd,i,bibbDS_RP_A); 
               if ( pd->BACnetStandardObjects[EVENT_ENROLLMENT]==soNotSupported )
               {
                 sprintf(opj,"135.1-2003 5.(a): "
                   "BIBB AE-N-E-B requires support for the Event Enrollment object "
                   "in the \"Standard Object Types Supported\" section.\n");
                 tperror(opj,false);
               }  
               break;

            case bibbAE_LS_B: // requires support for Life Safety Point or Life Safety Zone object
               if ( (pd->BACnetStandardObjects[LIFE_SAFETY_POINT]==soNotSupported) &&
                    (pd->BACnetStandardObjects[LIFE_SAFETY_ZONE]==soNotSupported) )
               {
                 sprintf(opj,"135.1-2003 5.(a): "
                   "BIBB AE-LS-B requires support for the Life Safety Point or Life Safety Zone object "
                   "in the \"Standard Object Types Supported\" section.\n");
                 tperror(opj,false);
               }  
               break;

            case bibbSCHED_A:  // requires support for DS-RP-A and DS-WP-A
               CheckPICS_BIBB_Cross_Dependency(pd,i,bibbDS_WP_A); 
               CheckPICS_BIBB_Cross_Dependency(pd,i,bibbDS_RP_A);
               break; 

            case bibbSCHED_I_B:  // requires DS-RP-B, DS-WP-B, Calendar & Schedule objects
                                 // and either DM-TS-B or DM-UTC-B
               CheckPICS_BIBB_Cross_Dependency(pd,i,bibbDS_RP_B); 
               CheckPICS_BIBB_Cross_Dependency(pd,i,bibbDS_WP_B);
               if ( pd->BACnetStandardObjects[CALENDAR]==soNotSupported )
               {
                 sprintf(opj,"135.1-2003 5.(a): "
                   "BIBB SCHED-I-B requires support for the Calendar object "
                   "in the \"Standard Object Types Supported\" section.\n");
                 tperror(opj,false);
               }  
               if ( pd->BACnetStandardObjects[SCHEDULE]==soNotSupported )
               {
                 sprintf(opj,"135.1-2003 5.(a): "
                   "BIBB SCHED-I-B requires support for the Schedule object "
                   "in the \"Standard Object Types Supported\" section.\n");
                 tperror(opj,false);
               }  
               if ( !pd->BIBBSupported[bibbDM_TS_B] &&
                    !pd->BIBBSupported[bibbDM_UTC_B] )
               {
                 sprintf(opj,"BIBB SCHED-I-B requires support for either DM-TS-B or DM-UTC-B.\n"); 
                 tperror(opj,false);
               }
               break;

            case bibbSCHED_E_B:  // requires support for SCHED-I-B and DS-WP-A
				CheckPICS_BIBB_Cross_Dependency(pd,i,bibbSCHED_I_B); 
               CheckPICS_BIBB_Cross_Dependency(pd,i,bibbDS_WP_A);
               break;

			case bibbSCH_E_B:
			   CheckPICS_BIBB_Cross_Dependency(pd,i,bibbSCH_I_B); 
               CheckPICS_BIBB_Cross_Dependency(pd,i,bibbDS_WP_A);
               break;
            case bibbT_VMT_I_B:  // both require support for TREND_LOG object
            case bibbT_ATR_B:
               if ( pd->BACnetStandardObjects[TREND_LOG]==soNotSupported )
               {
                 sprintf(opj,"135.1-2003 5.(a): "
                   "BIBB %s requires support for the Trend Log object "
                   "in the \"Standard Object Types Supported\" section.\n",
                   BIBBs[i].name );
                 tperror(opj,false);
               }  
               break;

            case bibbT_VMT_E_B:  // requires support for T-VMT-I-B and DS-RP-A 
               CheckPICS_BIBB_Cross_Dependency(pd,i,bibbT_VMT_I_B); 
               CheckPICS_BIBB_Cross_Dependency(pd,i,bibbDS_RP_A);
               break;


            case bibbDM_TS_A:
            case bibbDM_UTC_A:
                               // requires presence of Time_Synchronization_Recipients 
                               // property in the Device Object.
                               // TODO
               break;

            case bibbDM_UTC_B: // Same requirements as DM_TS_B, but adds
                               // the requirement for the presence of UTC_Offset 
                               // and Daylight_Saving_Status in the Device Object.
                               // (deliberate fall-through to next case)
            case bibbDM_TS_B:  // requires the presence of the Local_Time and Local_Date
                               // properties of the Device Object.
                               // TODO
               break;


            case bibbDM_R_B:   // requires the presence of the Time_Of_Device_Restart
                               // and Last_Restart_Reason properties 
                               // in the Device Object.
                               // TODO
               break;

// LJT Updated this list with the new Workstation Bibbs and their dependencies
			case bibbDS_V_A:
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_RP_A);
				break;
			case bibbAE_VN_A:
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbAE_N_A);
				break;
			case bibbAE_AVN_A:
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbAE_VN_A);
				break;
			case bibbAE_VM_A:
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_RP_A);
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_WP_A);
				break;
			case bibbAE_AVM_A:
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_RP_A);
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_WP_A);
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDM_OCD_A);
				break;
			case bibbSCH_AVM_A:
			case bibbSCHED_AVM_A:
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_RP_A);
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_WP_A);
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDM_OCD_A);
				break;
			case bibbSCH_VM_A:
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_RP_A);
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_WP_A);
				break;
			case bibbSCH_WS_A:
			case bibbSCHED_WS_A:
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_RP_A);
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_WP_A);
				break;
			case bibbSCH_WS_I_B:
			case bibbSCHED_WS_I_B:
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_RP_B);
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_WP_B);
                if ( !pd->BIBBSupported[bibbDM_TS_B] &&
                    !pd->BIBBSupported[bibbDM_UTC_B] )
                {
                   sprintf(opj,"BIBB SCHED-WS-I-B requires support for either DM-TS-B or DM-UTC-B.\n"); 
                   tperror(opj,false);
                }
               if ( pd->BACnetStandardObjects[SCHEDULE]==soNotSupported )
               {
                 sprintf(opj,"135.1-2003 5.(a): "
                   "BIBB SCHED-WS-I-B requires support for the Schedule object "
                   "in the \"Standard Object Types Supported\" section.\n");
                 tperror(opj,false);
               }  

				break;
			case bibbSCH_R_B:
			case bibbSCHED_R_B:
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_RP_B);
                if ( !pd->BIBBSupported[bibbDM_TS_B] &&
                    !pd->BIBBSupported[bibbDM_UTC_B] )
                {
                   sprintf(opj,"BIBB SCHED-R-B requires support for either DM-TS-B or DM-UTC-B.\n"); 
                   tperror(opj,false);
                }
                if ( pd->BACnetStandardObjects[SCHEDULE]==soNotSupported )
                {
                  sprintf(opj,"135.1-2003 5.(a): "
                   "BIBB SCHED-R-B requires support for the Schedule object "
                   "in the \"Standard Object Types Supported\" section.\n");
                  tperror(opj,false);
                }  
				break;
			case bibbT_AVM_A:
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_RP_A);
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_WP_A);
				break;
			case bibbT_A_A:
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbT_ATR_A);
				break;
			case bibbDM_ANM_A:
				// 17-May-2010 DDO removed erroneous Dependency upon DM_DDB_B
				// CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDM_DDB_B);
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDM_DDB_A);
				break;
			case bibbDM_ADM_A:
				// 17-May-2010 DDO removed erroneous Dependency upon DS_AV_A
				// replacing it with correct Dependency upon DS_RP_A
				CheckPICS_BIBB_Cross_Dependency(pd, i, bibbDS_RP_A);
				break;
		 }
      }
   }

  return;
}


///////////////////////////////////////////////////////////////////////////////
// EPICS Consistency test specified by 135.1-2003, section 5(d)
//
// (d) All application services required by the supported BIBBs shall be
// indicated as supported in the BACnet Standard Application Services
// Supported section of the EPICS with Initiate and Execute indicated as
// required by the supported BIBBs. (Similar to the old test "D", but using
// BIBBs instead of conformance class.)
// 
void CheckPICSCons2003D(PICSdb *pd)
{
   int i;
   char opj[300];
   char *errmsg;
   bibb_service *pBibb_rule;  // pointer to each bibb_service rule in the BIBB definition
   for (i = 0; i < MAX_BIBBS; i++ )
   {
      // Scan for supported BIBBs, cross checking the services supported section
      // when support id found for a BIBB
      if (pd->BIBBSupported[i])
      {
         pBibb_rule = BIBBs[i].aBIBB_Service;  // point to first BIBB rule
         while (pBibb_rule->InitExec) // while not at end of this BIBB's rules
         {
            // If the BIBB rule requires a service that has not been
            // defined in the StandardServices section, this is an error.
            if ((pd->BACnetStandardServices[pBibb_rule->ApplServ] & pBibb_rule->InitExec)==0)
            {
               if (pBibb_rule->InitExec == ssInitiate) 
                  errmsg = "Initiate";
               else
                  errmsg = "Execute";
                     
               sprintf(opj,"135.1-2003 5.(d): "
                 "BIBB %s requires the Device to %s the %s application service,"
                 " and support for this service has not been included in the "
                 "\"BACnet Standard Application Services Supported\" section.\n",
                 BIBBs[i].name,
                 errmsg,
                 StandardServices[pBibb_rule->ApplServ] );
               tperror(opj,false);
               
            }
            pBibb_rule++;  // next rule please
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
// EPICS Consistency test specified by 135.1-2003, section 5(e)
//
// (e) The Application_Services_Supported property of the Device object in
// the test database shall indicate support for each application service
// for which the supported BIBBs requires support for execution of the
// service. (Similar to the old test "E", but using BIBBs instead of
// conformance class.)
// 
// This function does not actually use the BIBBs for reference since
// the consistency check between the BIBBS and the Application Services Supported
// section has already been done. It runs a consistency check between the 
// Application Services Supported section and the Application_Serices_Supported
// property of the Device Object, but only for the "Execute" services.
// 
void CheckPICSCons2003E(PICSdb *pd)
{   
    int i;
    char opj[300];
    octet Application_Services_Supported[MAX_BITSTRING]; // one byte per bit
    octet InStandardAppSection;
    int  iNumStandardServices;
	 memset(Application_Services_Supported,0,MAX_BITSTRING);

    iNumStandardServices = GetStandardServicesSize(); 
    if (!pd->pDeviceObject)
       return;
 	 ExpandBitstring(Application_Services_Supported,pd->pDeviceObject->protocol_services_supported, iNumStandardServices);

    for (i=0; i<iNumStandardServices; i++)
    {
       InStandardAppSection = ((pd->BACnetStandardServices[i] & ssExecute) != 0);
       // Exclusive OR test between the property and the EPICS section.
       // If either is set and not the other, this is an error.
       if( (InStandardAppSection ^ Application_Services_Supported[i]) )  // XOR
       {
         sprintf(opj,"135.1-2003 5.(e): "
           "Support for execution of the %s application service is not consistent "
           "between the \"BACnet Standard Application Services Supported\" "
           "section and the Application_Services_Supported Property of the Device Object.\n", 
           StandardServices[i]);
         tperror(opj,false);
       }
    }

}


///////////////////////////////////////////////////////////////////////////////
// EPICS Consistency test specified by 135.1-2003, section 5(c) & 5(f)
//
// Since the 2003A test covers which Objects should be in the 
// Standard Object Types Supported section for each BIBB,
// this function just needs to check that each object listed in that
// section is indeed defined in the Object_Types_Supported property
// of the Device Object in the database.  This covers both of the
// following consistency checks in 135.1-2003:
//         
// (c) The Object_Types_Supported property of the Device object in the test
// database shall indicate support for each object type required by the
// supported BIBBs.  (Similar to the old test "C", but using BIBBs instead
// of conformance class.)
// 
// (f) The object types listed in the Standard Object Types Supported
// section of the EPICS shall have a one-to-one correspondence with object
// types listed in the Object_Types_Supported property of the Device object
// contained in the test database.	(Identical to the old test "K".)
// 
void CheckPICSCons2003F(PICSdb *pd)
{ char errMsg[300];
  int  i, iNumStandardObjects;
  octet InStandardObjectSection;
  octet ProtocolObjectSup[MAX_BITSTRING];

  memset(ProtocolObjectSup,0,MAX_BITSTRING);
  iNumStandardObjects = GetObjectTypeSize();
  if (!pd->pDeviceObject)
     return;
  ExpandBitstring(ProtocolObjectSup,pd->pDeviceObject->object_types_supported, iNumStandardObjects);

  for(i=0;i<MAX_DEFINED_OBJ;i++) {
    InStandardObjectSection = (pd->BACnetStandardObjects[i] != 0); // convert to boolean
    // Exclusive OR test between the property and the EPICS section.
    // If either is set and not the other, this is an error.
    if( (InStandardObjectSection ^ ProtocolObjectSup[i]) )  // XOR
    {
      sprintf(errMsg,"135.1-2003 5.(f): "
        "Object type %s is not consistent between the \"Standard Object Types Supported\" "
        "section and the Object_Types_Supported Property of the Device Object.\n", StandardObjects[i]);
      tperror(errMsg,false);
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
// EPICS Consistency test specified by 135.1-2003, section 5(b) & 5(g)
//
// Since the 2003A test covers which Objects should be in the 
// Standard Object Types Supported section for each BIBB,
// this function just needs to check that each object listed in that
// section is indeed in the database.  This covers both of the
// following consistency checks in 135.1-2003:
//         
// (b)	A minimum of one instance of each object type required by the
// specified BIBBs shall be included in the test database. (Similar to the
// old test "B", but using BIBBs instead of conformance class.)
// 
// (g) For each object type listed in the Standard Object Types Supported
// section of the EPICS there shall be at least one object of that type in
// the test database. (Identical to the old test "L".)
// 
void CheckPICSCons2003G(PICSdb *pd)
{ 
  char errMsg[300];
  for(int i=0;i<MAX_DEFINED_OBJ;i++) {
    if( (pd->BACnetStandardObjects[i]) && !(ObjInTestDB[i].ObjIDSupported)) {
      sprintf(errMsg,"135.1-2003 5.(g): "
        "Object type %s is listed in the \"Standard Object Types Supported\" section "
        "but no Objects of that type are defined in the test database.\n",StandardObjects[i]);
      tperror(errMsg,false);
    }
  }
}

// Scans the test database looking for an object with
// a matching ObjectID.  If found, returns a pointer to the Object.
// If not found, returns NULL.
generic_object *FindObjectInDatabase(PICSdb *pd, dword ObjectID)
{
   generic_object *po;
   // pointer to the start of objects in the test database
   po = pd->Database;
   while (po)
   {
      if (po->object_id == ObjectID)
         return po;
      po=(generic_object *)po->next;
   }
   return NULL;
}

// Finds an Object matching the ObjectID in the Object_List
// propery of the Device Object defined in the test database.
// If found, returns TRUE.
// If not found, returns FALSE.
bool FindObjectInObjectList(PICSdb *pd, dword ObjectID)
{
   BACnetObjectIdentifier *pObjectID; 
   if (pd->pDeviceObject == NULL)
      return FALSE; // Device Object not parsed yet

   // Pointer to the parsed Object_List property of the Device Object
   pObjectID = pd->pDeviceObject->object_list;

   while (pObjectID) 
   {
      if (pObjectID->object_id == ObjectID)
         return TRUE;
      pObjectID = pObjectID->next;
   }
   // TODO: Add check of structured_object_list here?
   return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// EPICS Consistency test specified by 135.1-2003, section 5(h)
// 
// (h) There shall be a one-to-one correspondence between the objects
// listed in the Object_List property of the Device object and the objects
// included in the test database. The Object_List property and the test
// database shall both include all proprietary objects. Properties of
// proprietary objects that are not required by BACnet Clause 23.4.3 need
// not be included in the test database. (Identical to the old test "M".)
// 
// This test first steps through the Object_List and verifies that
// every Object found in the Object_List has been defined in the test database.
// Then it does a reverse lookup by stepping through the test database
// and making sure that every Object found is also in the Object_List.
// This strategy should uncover any and all mismatches.
//
void CheckPICSCons2003H(PICSdb *pd)
{
   char errMsg[300];
   generic_object *po;
   BACnetObjectIdentifier *pObjectID; 
   dword id, objinstance; // temps
   word  objtype;

   dword dwMaxStdObj = sizeof(StandardObjects)/sizeof(StandardObjects[0]);


   // pointer to the start of objects in the test database
   po = pd->Database;
   if (po == NULL)
      return;
   // Pointer to the parsed Object_List property of the Device Object
   if (pd->pDeviceObject == NULL)
      return; 
   pObjectID = pd->pDeviceObject->object_list;

   // If Object_List has not been parsed, we cannot do this test.
   if (!pObjectID)
      return;

   // Step through the Object_List
   while (pObjectID)
   {
      id = pObjectID->object_id;
      if (FindObjectInDatabase(pd, id) == NULL)
      {
         SplitObjectId(id, &objtype, &objinstance);
         if (objtype < dwMaxStdObj)
         {
            sprintf(errMsg,"135.1-2003 5.(h): "
              "(%s,%u) is defined in the Object_List but does not appear in the test database.\n",
              StandardObjects[objtype], objinstance );
         }
         else 
         {
            sprintf(errMsg,"135.1-2003 5.(h): "
              "(proprietary %u,%u) is defined in the Object_List but does not appear in the test database.\n",
              objtype, objinstance );
         }
         tperror(errMsg,false);
      }
      pObjectID = pObjectID->next; // next in the Object_List
   }

   // Now step through the test database and verify that every object in the database
   // is also in the Object_List.
   while (po)
   {
      id=po->object_id;
      if (!FindObjectInObjectList(pd, id))
      {
         SplitObjectId(id, &objtype, &objinstance);
         if (objtype < dwMaxStdObj)
         {
            sprintf(errMsg,"135.1-2003 5.(h): "
              "(%s,%u) is defined in the test database but does not appear in the Object_List.\n",
              StandardObjects[objtype], objinstance );
         }
         else 
         {
            sprintf(errMsg,"135.1-2003 5.(h): "
              "(proprietary %u,%u) is defined in the test database but does not appear in the Object_List.\n",
              objtype, objinstance );
         }
         tperror(errMsg,false);
      }
      po=(generic_object *)po->next;
   }
   return;
}

// Helper function for CheckPICSCons2003I.
// This function is called for each object in the test database
// to check if required (and writable) properties are present.
// This function also handles the special case of the Device Object,
// which applies conditions that are outside the scope of 
// the Device Object itself.
void CheckPICSConsProperties(PICSdb *pd, generic_object *obj)
{
	char   errMsg[300]; 
	word   objtype;
	dword  objInstance;
	propdescriptor *propdesc;
   octet  GroupRequired[128]; // flags corresponding to PropGroup
   int    i = 0;
   octet  group = 0;

   // Clear group detection flags
   memset(GroupRequired, 0, sizeof(GroupRequired));

   // derive object type and object instance number from object ID
   SplitObjectId(obj->object_id, &objtype, &objInstance);

   // set property descriptor to point to property descriptors for this object type
	propdesc=StdObjects[objtype].sotProps;

   // loop through the standard properties for this object type
	while (1) 
	{
      // If property is required, and it was not parsed in EPICS, log an error.
      if ( (propdesc->PropFlags & R) && !(obj->propflags[i] & PropIsPresent) )
      {
			sprintf(errMsg,"135.1-2003 5.(i): "
				"(%s,%u) must contain required property \"%s\".\n",StandardObjects[objtype],objInstance,propdesc->PropertyName);
			tperror(errMsg,false);
      }
      // If the property is present, and the property is required to be writable, 
      // but is not marked with a 'W' in EPICS, log an error.
      // We exclude Commandable properties from this check and properties that
      // are only writable when Out_Of_Service is set.
      else if (  (propdesc->PropFlags & W) && 
                !(propdesc->PropFlags & (IsCommandable | Woutofservice)) &&
                !(obj->propflags[i] & PropIsWritable) )
      {
			sprintf(errMsg,"135.1-2003 5.(j): "
				"(%s,%u) must have property \"%s\" marked writable.\n",StandardObjects[objtype],objInstance,propdesc->PropertyName);
			tperror(errMsg,false);
      }
	  else if ( propdesc->PropFlags & AtLeast1 )
	  {
		  // special property here -- need to find at least 1 of these when we do we can stop looking.
	  }
	  
      // Test for the presence of properties that are marked with 'footnotes' 
      // that cause other properties to be present if this property is present.
      // These are remembered for a second pass through the properties to
      // check for missing properties.
      group = propdesc->PropGroup & ~Last; // mask off the "Last" indicator bit
      // If this property is in a "group" (footnote), and it is present in the database, 
      // mark this whole group as required.
      if (group && (obj->propflags[i] & PropIsPresent) ) //&& !(propdesc->PropFlags&&AtLeast1 == AtLeast1))  // LJT 4/17/2008 added AtLeast1Check 
         GroupRequired[group] = 1;


      if (propdesc->PropGroup & Last)
         break;  // if this is the last property definition, exit loop
      propdesc++; // next propertydefinition for this object
      i++;  // next index into propflags
	} 

   ///////////////////
   // !! 2nd pass !!
   //////////////////

   // reset property descriptor to point to property descriptors for this object type
	propdesc=StdObjects[objtype].sotProps;
   i = 0; 

   // Make a second pass through the standard properties, looking for
   // missing "footnoted" properties that belong to groups that
   // were detected during the first pass.
   if (objtype != DEVICE)  // don't do these checks on the Device Object (see else below)
   {
	   while (1) 
	   {
         group = propdesc->PropGroup & ~Last;
         // If property belongs to a group, and another property was detected
         // in this same group in the first pass, and this property
         // is not present in the database,log an error.
         if ( group && GroupRequired[group] && !(obj->propflags[i] & PropIsPresent) )
         {
			   sprintf(errMsg,"135.1-2003 5.(i): "
				   "(%s,%u) must contain conditionally required property \"%s\".\n",StandardObjects[objtype],objInstance,propdesc->PropertyName);
			   tperror(errMsg,false);
         }

         // Are we done?
         if (propdesc->PropGroup & Last)
            break;  // if this is the last property definition, exit loop
         propdesc++; // next propertydefinition for this object
         i++;  // next index into propflags
	   } 
   }
   else // objtype == DEVICE
   {
      // These are special tests run on the Device Object to check for properties that
      // are conditionally required based on support for certain *services* or
      // data link options.
	   while (1) 
	   {
         int required;
         required = 0;
         switch (propdesc->PropID)
         {
            case MAX_SEGMENTS_ACCEPTED:
            case APDU_SEGMENT_TIMEOUT:
               // If segmentation is supported, these are required.
               if ( pd->SegmentedRequestWindow || pd->SegmentedResponseWindow )
                  required = R;
               break;

            case VT_CLASSES_SUPPORTED:
            case ACTIVE_VT_SESSIONS:
               // If VT services are supported, these are required.
               if  ( (pd->BACnetStandardServices[asVT_Open] & ssExecute) ||
                     (pd->BACnetStandardServices[asVT_Close] & ssExecute) ||
                     (pd->BACnetStandardServices[asVT_Data] & ssExecute) )
                   required = R;
               break;

            case LOCAL_TIME:
            case LOCAL_DATE:
               // If the device supports the execution of TimeSynchronization
               // or UTCTimeSynchronization, these are required.
               if  ( pd->BACnetStandardServices[asTimeSynchronization] & ssExecute )
                   required = R;
               // !!! WARNING !!! - deliberate fall-through to the next case - no break!
            case UTC_OFFSET:
            case DAYLIGHT_SAVINGS_STATUS:
                // If the device supports the execution of UTCTimeSynchronization, these are required.
               if  ( pd->BACnetStandardServices[asUTCTimeSynchronization] & ssExecute )
                   required = R;
               break;

            case TIME_SYNCHRONIZATION_RECIPIENTS:
               // If supports DM-TS-A or DM-UTC-A, this must be present & WRITABLE!
               if ( pd->BIBBSupported[bibbDM_TS_A] || pd->BIBBSupported[bibbDM_UTC_A] )
                  required = W;
               break;

            case MAX_MASTER:
            case MAX_INFO_FRAMES:
               // If MS/TP Master, these are required
               if ( pd->DataLinkLayerOptions[9] )  // ugly hard-coded 9
                  required = R;
               break;

            case CONFIGURATION_FILES:
            case LAST_RESTORE_TIME:
               // If Backup and restore is supported, these must be present.
               if ( pd->BIBBSupported[bibbDM_BR_B] )
                  required = R;
               break;

            case BACKUP_FAILURE_TIMEOUT:
               // If Backup and restore is supported, this must be present & WRITABLE!
               if ( pd->BIBBSupported[bibbDM_BR_B] )
                  required = W;
               break;

            case ACTIVE_COV_SUBSCRIPTIONS:
                // If the device supports the execution of SubscribeCOV or SubscribeCOVProperty,
                // this property is required.
               if  ( (pd->BACnetStandardServices[asSubscribeCOV]         & ssExecute) ||
                     (pd->BACnetStandardServices[asSubscribeCOVProperty] & ssExecute) )
                   required = R;
               break;
         }


         // If property is required, and it was not parsed in EPICS, log an error.
         if (required && !(obj->propflags[i] & PropIsPresent) )
         {
			   sprintf(errMsg,"135.1-2003 5.(i): "
				   "(%s,%u) must contain conditionally required property \"%s\".\n",StandardObjects[objtype],objInstance,propdesc->PropertyName);
			   tperror(errMsg,false);
         }
         // If property must be writable and is not, log an error.
         else if ( (required & W) && !(obj->propflags[i] & PropIsWritable) )
         {
			   sprintf(errMsg,"135.1-2003 5.(i): "
				   "(%s,%u) must have property \"%s\" marked writable.\n",StandardObjects[objtype],objInstance,propdesc->PropertyName);
			   tperror(errMsg,false);
         }

         // Are we done?
         if (propdesc->PropGroup & Last)
            break;  // if this is the last property definition, exit loop
         propdesc++; // next propertydefinition for this object
         i++;  // next index into propflags
	   } 
   }

   return;
}


///////////////////////////////////////////////////////////////////////////////
// EPICS Consistency test specified by 135.1-2003, section 5(i) & (j)
// 
// (i) For each object included in the test database, all required
// properties for that object as defined in Clause 12 of BACnet shall be
// present. In addition, if any of the properties supported for an object
// require the conditional presence of other properties, their presence
// shall be verified. (Identical to the old test "N".)
// 
// (j) For each property that is required to be writable, that property
// shall be marked as writable in the EPICS. (New test.)
// 
void CheckPICSCons2003I(PICSdb *pd)
{  
   generic_object *obj;
	obj=pd->Database;
   // Loop through the entire database ...
	while(obj)
	{
      // check for required, conditionally required, and mandatory writable properties

	  // 5-24-2005 Shiyuan Xiao. Ignore unstandard object
	  if(obj->object_type <= etObjectTypes.propes)
		  CheckPICSConsProperties(pd, obj); 
	  
	  obj=(generic_object *)obj->next;
   }
	return;
}

///////////////////////////////////////////////////////////////////////////////
// EPICS Consistency test specified by 135.1-2003, section 5(k)
// 
// (k) The length of the Protocol_Services_Supported bitstring shall have
// the number of bits defined for BACnetProtocolServicesSupported for the
// IUT's declared protocol revision.  (New test.)
// 
void CheckPICSCons2003K(PICSdb *pd)
{  
	char   errMsg[300]; 
   octet ExpectedLength;
   device_obj_type *pDevice = pd->pDeviceObject;
   // Make sure we have a pointer to the Device Object.
   if (!pDevice)
      return;

   // Bounds check.  If protocol_revision is a higher number than what we know about,
   // skip this test.
   if (pDevice->protocol_rev >= (sizeof(aCorrectLengthProtocolServicesSupportedBitstring)/
                                sizeof(aCorrectLengthProtocolServicesSupportedBitstring[0])) )
      return;

   ExpectedLength = aCorrectLengthProtocolServicesSupportedBitstring[pDevice->protocol_rev];
   if (EPICSLengthProtocolServicesSupportedBitstring != ExpectedLength)
   {
	   sprintf(errMsg,"135.1-2003 5.(k): "
		   "For Protocol_Revision of %d, the length of Protocol_Services_Supported "
		   "must be %d bits, but it is %d bits.\n",
		   pDevice->protocol_rev,
		   ExpectedLength,
		   EPICSLengthProtocolServicesSupportedBitstring);
	   tperror(errMsg,false);
   }
	return;
}

///////////////////////////////////////////////////////////////////////////////
// EPICS Consistency test specified by 135.1-2003, section 5(l)
// 
// (l) The length of the Protocol_Object_Types_Supported bitstring shall
// have the number of bits defined for BACnetObjectTypesSupported for the
// IUT's declared protocol revision. (New test.)
// 
void CheckPICSCons2003L(PICSdb *pd)
{  
	char   errMsg[300]; 
   octet ExpectedLength;
   device_obj_type *pDevice = pd->pDeviceObject;
   // Make sure we have a pointer to the Device Object.
   if (!pDevice)
      return;

   // Bounds check.  If protocol_revision is a higher number than what we know about,
   // skip this test.
   if (pDevice->protocol_rev >= (sizeof(aCorrectLengthProtocolObjectTypesSupportedBitstring)/
                                sizeof(aCorrectLengthProtocolObjectTypesSupportedBitstring[0])) )
      return;

   ExpectedLength = aCorrectLengthProtocolObjectTypesSupportedBitstring[pDevice->protocol_rev];
   if (EPICSLengthProtocolObjectTypesSupportedBitstring != ExpectedLength)
   {
	   sprintf(errMsg,"135.1-2003 5.(l): "
		   "For Protocol_Revision of %d, the length of Protocol_Object_Types_Supported "
		   "must be %d bits, but it is %d bits.\n",
		   pDevice->protocol_rev,
		   ExpectedLength,
		   EPICSLengthProtocolObjectTypesSupportedBitstring);
	   tperror(errMsg,false);
   }
	return;
}

void *GetEnumTable(int iTableIndex)
{
   return AllETs[iTableIndex];
}


}
