/*	29-Jan-01 [002] JJB  revise for C++, added PICS namespace and typecasts
	23-Mar-98 [001] JN   32-bit version (not all marked) 
						 eg. far pascal __export --> APIENTRY
*/

#include "stdafx.h"									//			***001 Begin
//#include <afx.h>			
//#include <afxwin.h>								//			***001 End

#include <windows.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include <time.h>			// jjb
#include <sys/types.h>		// jjb
#include <sys/timeb.h>		// jjb

#include "BACnet.hpp"
#include "propid.h" 
namespace NetworkSniffer {
	extern char *BACnetPropertyIdentifier[];
}
namespace PICS {															// ***002

#include "db.h"       // contains BACnet datatypes
#include "vtsapi.h" 
#include "bacprim.h"  // contains basic encoding functions

// - jjb #include "resource.h"   // resource identifiers
// - jjb #include "dudclass.h" // derived classes 
#include "dudapi.h"   // export interface
#include "dudtool.h"  // tool functions

#include "props.h"    

#define nPRIO 16    // size of priority arrays
#define nTEST 3     // number of test values
#define nTESTLIST 4 // number of test values for list properties


// test values for list properties:
BACnetCalendarEntry far* gTestCalist[nTESTLIST];
BACnetReadAccessSpecification far* gTestRaslist[nTESTLIST];
BACnetVTClassList far* gTestVtcl[nTESTLIST];
BACnetObjectPropertyReference far* gTestLopref[nTESTLIST];  
BACnetSessionKey far* gTestSkeys[nTESTLIST];
BACnetAddressBinding far* gTestDabind[nTESTLIST];
BACnetRecipient far* gTestRecip[nTESTLIST];
BACnetDestination far* gTestReciplist[nTESTLIST];
BACnetVTSession far* gTestVtse[nTESTLIST];


BACnetGenericArray * CreateAddressBindingList( PICS::BACnetAddressBinding * pAddrBinding );
BACnetObjectIDList * CreateObjectList( PICS::BACnetObjectIdentifier * pobjidlist );
BACnetGenericArray * CreateTimeStampArray( PICS::BACnetTimeStamp  * ptimestamp, int nMax );
BACnetCalendarArray * CreateCalendarList( PICS::BACnetCalendarEntry * pcalentry );

//void CreatePropertyFromEPICS( generic_object * pObj, int PropId, BACnetAnyValue * pbacnetAnyValue );

void AI_CreateProperty( ai_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void AO_CreateProperty( ao_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void AV_CreateProperty( av_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void BI_CreateProperty( bi_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void BO_CreateProperty( bo_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void BV_CreateProperty( bv_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void CA_CreateProperty( calendar_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void CO_CreateProperty( command_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void DE_CreateProperty( device_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void EV_CreateProperty( ee_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void FI_CreateProperty( file_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void GR_CreateProperty( group_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void LO_CreateProperty( loop_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void MSI_CreateProperty( mi_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void MSO_CreateProperty( mo_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void MSV_CreateProperty( msv_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void NC_CreateProperty( nc_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void PR_CreateProperty( program_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void SC_CreateProperty( schedule_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void AVG_CreateProperty( avg_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );
void TR_CreateProperty( trend_obj_type * pObj, BACnetPropertyIdentifier PropId, BACnetAnyValue * pbacnetAnyValue );

octet gTestStavals[nTESTLIST][4]= { {1,2,3,0}, {4,0,0,0}, {0,0,0,0}, {5,0,0,0} };

int 		gTestEnum[nTEST]= {0, 1, 2};
float 		gTestReal[nTEST]= {47.1F, 0.5F, 92.2F};				//					***001

octet 		gTestBits[nTEST]= {0x55, 0x44, 0x33};
int		  	gTestUsedBits= 5;

int			gTestInt[nTEST]=  	{13, 14, 15};

char far*	gTestString[nTEST]= {"abcd", "efgh", "ijkl"};   

BOOL		gTestBool[nTEST]= {TRUE, FALSE, TRUE};

BACnetDateTime gTestDT[nTEST] = { 
									{{96,5,20,2},{9,28,0,0}}, 
									{{96,4,21,3},{9,13,0,0}}, 
									{{96,6,21,2},{8,29,0,0}} 
								};

//																										***001 Begin
float gTestPAF[nTEST][nPRIO]= { {1.1F,2.1F,3.1F,4.1F,5.1F,6.1F,7.1F,8.1F,9.1F,10.1F,11.1F,12.1F,13.1F,14.1F,15.1F,16.1F},
								{1.2F,2.2F,3.2F,4.2F,5.2F,6.2F,7.2F,8.2F,9.2F,10.2F,11.2F,12.2F,13.2F,14.2F,15.2F,16.2F},	
								{1.3F,2.3F,3.3F,4.3F,5.3F,6.3F,7.3F,8.3F,9.3F,10.3F,11.3F,12.3F,13.3F,14.3F,15.3F,16.3F}	
							  };	//																	***001 End

enum BACnetBinaryPV gTestPAB[nTEST][nPRIO]= 
        {
          { INACTIVE,INACTIVE,INACTIVE,INACTIVE,INACTIVE,INACTIVE,INACTIVE,INACTIVE,
            INACTIVE,INACTIVE,INACTIVE,INACTIVE,INACTIVE,INACTIVE,INACTIVE,INACTIVE }, 
          { ACTIVE,ACTIVE,ACTIVE,ACTIVE,ACTIVE,ACTIVE,ACTIVE,ACTIVE,
            ACTIVE,ACTIVE,ACTIVE,ACTIVE,ACTIVE,ACTIVE,ACTIVE,ACTIVE } ,
          { ACTIVE,ACTIVE,ACTIVE,ACTIVE,ACTIVE,ACTIVE,ACTIVE,ACTIVE,
            INACTIVE,INACTIVE,INACTIVE,INACTIVE,INACTIVE,INACTIVE,INACTIVE,INACTIVE } 
        };    

word gTestPAU[nTEST][nPRIO]= { 
								{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16},
								{16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1},
								{16,15,14,13,12,11,10,9,1,2,3,4,5,6,7,8}
							 };

BACnetDateRange gTestDTRange[nTEST]= { 
										{ {96,5,20,0xFF}, {96,5,22,0xFF} },
										{ {95,4,19,0xFF}, {97,6,24,0xFF} },
										{ {95,6,21,0xFF}, {95,6,23,0xFF} }
									 };
									 
//																						***001 Begin
BACnetEventParameter gTestEvparm[nTEST]= {
    {OUT_OF_RANGE, {NULL,8,{0,0,0,0}}, NULL, NULL, 0, 0.0, 17, {NULL}, {NULL}, 1.5F, 1.6F, 1.7F, 1.8F, 1.9F },
    {OUT_OF_RANGE, {NULL,8,{0,0,0,0}}, NULL, NULL, 0, 0.0, 17, {NULL}, {NULL}, 1.1F, 1.2F, 1.3F, 1.4F, 1.5F },
    {OUT_OF_RANGE, {NULL,8,{0,0,0,0}}, NULL, NULL, 0, 0.0, 17, {NULL}, {NULL}, 1.3F, 4.2F, 7.3F, 11.4F, 51.5F }
  };								//													***001 End

BACnetObjectIdentifier gTestLObj[nTEST]= { {NULL,4711}, {NULL,4712}, {NULL,4713} };

BACnetTimeValue gTestTimeValue[nTEST][7]= {
         {
           {NULL, {15,47,59,59}, UNS }, // 0
           {NULL, {15,47,59,59}, UNS }, // 1
           {NULL, {15,47,59,59}, UNS }, // 2
           {NULL, {15,47,59,59}, UNS }, // 3
           {NULL, {15,47,59,59}, UNS }, // 4
           {NULL, {15,47,59,59}, UNS }, // 5
           {NULL, {15,47,59,59}, UNS }  // 6
         },
  
         {
           {NULL, {14,47,59,59}, UNS }, // 0
           {NULL, {14,46,59,59}, UNS }, // 1
           {NULL, {14,45,59,59}, UNS }, // 2
           {NULL, {14,44,59,59}, UNS }, // 3
           {NULL, {14,43,59,59}, UNS }, // 4
           {NULL, {14,42,59,59}, UNS }, // 5
           {NULL, {14,41,59,59}, UNS }  // 6
         },

         {
           {NULL, {16,47,59,59}, UNS }, // 0
           {NULL, {16,46,59,59}, UNS }, // 1
           {NULL, {16,45,59,59}, UNS }, // 2
           {NULL, {16,44,59,59}, UNS }, // 3
           {NULL, {16,43,59,59}, UNS }, // 4
           {NULL, {16,42,59,59}, UNS }, // 5
           {NULL, {16,41,59,59}, UNS }  // 6
         }
    };                  


BACnetTimeValue far* gTestWsched[nTEST][7]; 
static BACnetExceptionSchedule far* gTestXSched[nTEST];

#define nTESTUWNUM 3 								  
word gTestUWArr[nTEST][nTESTUWNUM]= { {1,2,3}, {3,2,1}, {2,3,1} };
								  
char far* gTestActext[nTEST][1]= { {"act1"}, {"act2"}, {"act3"} };

static BACnetActionCommand far* gTestAct[nTEST][1];


static char gBuffer[256];


// ===========================================================================================================
//
// functions 
//
// ===========================================================================================================


// This function is used to allocate dynamic memory and to write test property values
// to the allocated memory.
void CreateTestValues(void)
{ 
 
  // Test value for BACnetActionCommand: --- 

  BACnetActionCommand far* pAct;

  pAct= gTestAct[0][0]= new  BACnetActionCommand;								//			***001
  
  pAct->next= NULL;
  pAct->device_id= 0;
  pAct->object_id= 4711;
  pAct->property_id= 85;
  pAct->pa_index= NotAnArray;
  pAct->value_type= FLT;
  pAct->av.fproperty_value= 1.1F;												//			***001
  pAct->priority= 16;
  pAct->post_delay= 1;
  pAct->quit_on_failure= 0;
  pAct->write_successful= 0;
  
  pAct= gTestAct[1][0]= new  BACnetActionCommand;						//						***001

  pAct->next= NULL;
  pAct->device_id= 0;
  pAct->object_id= 4712;
  pAct->property_id= 85;
  pAct->pa_index= NotAnArray;
  pAct->value_type= FLT;
  pAct->av.fproperty_value= 1.2F;										//						***001
  pAct->priority= 16;
  pAct->post_delay= 2;
  pAct->quit_on_failure= 0;
  pAct->write_successful= 0;

  pAct= gTestAct[2][0]= new  BACnetActionCommand;						//						***001

  pAct->next= NULL;
  pAct->device_id= 0;
  pAct->object_id= 4713;
  pAct->property_id= 85;
  pAct->pa_index= NotAnArray;
  pAct->value_type= FLT;
  pAct->av.fproperty_value= 1.3F;										//						***001
  pAct->priority= 16;          
  pAct->post_delay= 3;
  pAct->quit_on_failure= 0;
  pAct->write_successful= 0;


  // Test Value for BACnetExceptionSchedule: ---

  BACnetExceptionSchedule far* px;
  BACnetSpecialEvent far* pe;
  BACnetTimeValue far* pt;

  px= gTestXSched[0]= new  BACnetExceptionSchedule;					//							***001
  px->size= 1;

  pe= px->special_event= new  BACnetSpecialEvent;					//							***001
  pe->next= NULL;
  pe->choice= 0;
  pe->u.date.year= 96;
  pe->u.date.month= 5;
  pe->u.date.day_of_month= 3;
  pe->u.date.day_of_week= 1;
  pe->event_priority= 16;

  pt= pe->list_of_time_values= new  BACnetTimeValue;				//							***001
  pt->next= NULL;
  pt->time.hour= 7;
  pt->time.minute= 30;
  pt->time.second= 42;
  pt->time.hundredths= 11;
  pt->value_type= UNS;
  pt->av.uproperty_value= 3;
  
  px= gTestXSched[1]= new  BACnetExceptionSchedule;					//							***001
  px->size= 1;

  pe= px->special_event= new  BACnetSpecialEvent;					//							***001
  pe->next= NULL;
  pe->choice= 0;
  pe->u.date.year= 96;
  pe->u.date.month= 4;
  pe->u.date.day_of_month= 2;
  pe->u.date.day_of_week= 2;
  pe->event_priority= 16;

  pt= pe->list_of_time_values= new  BACnetTimeValue;				//							***001
  pt->next= NULL;
  pt->time.hour= 8;
  pt->time.minute= 31;
  pt->time.second= 43;
  pt->time.hundredths= 12;
  pt->value_type= UNS;
  pt->av.uproperty_value= 2;
  

  px= gTestXSched[2]= new  BACnetExceptionSchedule;						//						***001
  px->size= 1;

  pe= px->special_event= new  BACnetSpecialEvent;						//						***001
  pe->next= NULL;
  pe->choice= 0;
  pe->u.date.year= 96;
  pe->u.date.month= 4;
  pe->u.date.day_of_month= 3;
  pe->u.date.day_of_week= 3;
  pe->event_priority= 16;

  pt= pe->list_of_time_values= new  BACnetTimeValue;					//						***001
  pt->next= NULL;
  pt->time.hour= 8;
  pt->time.minute= 33;
  pt->time.second= 44;
  pt->time.hundredths= 13;
  pt->value_type= UNS;
  pt->av.uproperty_value= 3;


  for (int i=0; i<7; i++)
    {
      gTestWsched[0][i]= (BACnetTimeValue far*)&gTestTimeValue[0][i];
      gTestWsched[1][i]= (BACnetTimeValue far*)&gTestTimeValue[1][i];
      gTestWsched[2][i]= (BACnetTimeValue far*)&gTestTimeValue[2][i];
    }
    
    
  // Test Values for ListOf Properties: ---------------------------------------------------------------------
 
  // Date_List (calendar) List of BACnetCalendarEntry
 
  BACnetCalendarEntry far* pCal= gTestCalist[0]= new  BACnetCalendarEntry; // 3 elements		***001

  pCal->choice= 				0;
  pCal->u.date.year= 			96;
  pCal->u.date.month= 			5;
  pCal->u.date.day_of_month= 	20;
  pCal->u.date.day_of_week= 	dontcare;
  pCal->next= 					new  BACnetCalendarEntry;					//					***001
  pCal= pCal->next;

  pCal->choice= 				0;
  pCal->u.date.year= 			96;
  pCal->u.date.month= 			4;
  pCal->u.date.day_of_month= 	19;
  pCal->u.date.day_of_week= 	dontcare;
  pCal->next= 					new  BACnetCalendarEntry;					//					***001
  pCal= pCal->next;

  pCal->choice= 				0;
  pCal->u.date.year= 			96;
  pCal->u.date.month= 			3;
  pCal->u.date.day_of_month= 	18;
  pCal->u.date.day_of_week= 	dontcare;
  pCal->next= NULL;
  
  gTestCalist[1]= gTestCalist[0]->next->next; // 1 element
  gTestCalist[2]= NULL; // empty list
  
  pCal= gTestCalist[3]= new  BACnetCalendarEntry;						//					***001
  pCal->choice= 				0;
  pCal->u.date.year= 			97;
  pCal->u.date.month= 			3;
  pCal->u.date.day_of_month= 	15;
  pCal->u.date.day_of_week= 	dontcare;
  pCal->next= NULL;
  
  // VT_Classes_Supported (device) List of BACnetVTClass

  BACnetVTClassList far* pVtcl= gTestVtcl[0]= new  BACnetVTClassList; // 3 elements				***001
  
  pVtcl->vtclass= DEFAULT_TERMINAL;
  pVtcl->next= new  BACnetVTClassList;										//					***001
  pVtcl= pVtcl->next;
  
  pVtcl->vtclass= ANSI_X34;
  pVtcl->next= new  BACnetVTClassList;										//					***001
  pVtcl= pVtcl->next;

  pVtcl->vtclass= DEC_VT52;
  pVtcl->next= NULL;

  gTestVtcl[1]= gTestVtcl[0]->next->next; // 1 element
  gTestVtcl[2]= NULL; // empty list

  pVtcl= gTestVtcl[3]= new  BACnetVTClassList;								//					***001
  pVtcl->vtclass= IBM_3130;
  pVtcl->next= NULL;
 

  // Active_VT_Sessions (device) List of BACnetVTSession
 
  BACnetVTSession far* pVtse= gTestVtse[0]= new  BACnetVTSession; // 3 elements					***001

  pVtse->local_session_id= 1;
  pVtse->remote_session_id= 1;
  pVtse->remote_address.network_number= 1;
  pVtse->remote_address.mac_address[0]= 'p';
  pVtse->remote_address.address_size= 1; 
  pVtse->next= new  BACnetVTSession;										//					***001
  pVtse= pVtse->next;
  
  pVtse->local_session_id= 2;
  pVtse->remote_session_id= 2;
  pVtse->remote_address.network_number= 2;
  pVtse->remote_address.mac_address[0]= 'o';
  pVtse->remote_address.address_size= 1; 
  pVtse->next= new  BACnetVTSession;										//					***001
  pVtse= pVtse->next;

  pVtse->local_session_id= 3;
  pVtse->remote_session_id= 3;
  pVtse->remote_address.network_number= 3;
  pVtse->remote_address.mac_address[0]= 'e';
  pVtse->remote_address.address_size= 1; 
  pVtse->next= NULL;

  gTestVtse[1]= gTestVtse[0]->next->next; // 1 element
  gTestVtse[2]= NULL; // empty list
 
  pVtse= gTestVtse[3]= new  BACnetVTSession; 								//					***001
  pVtse->local_session_id= 4;
  pVtse->remote_session_id= 4;
  pVtse->remote_address.network_number= 4;
  pVtse->remote_address.mac_address[0]= 'z';
  pVtse->remote_address.address_size= 1; 
  pVtse->next= NULL;
  
  // List_Of_Session_Keys (device) List of BACnetSessionKey
 
  BACnetSessionKey far* pSkeys= gTestSkeys[0]= new  BACnetSessionKey; // 3 elements				***001
  
  pSkeys->session_key[0]= 1; pSkeys->session_key[1]= 2; pSkeys->session_key[2]= 3; pSkeys->session_key[3]= 4;
  pSkeys->session_key[4]= 5; pSkeys->session_key[5]= 6; pSkeys->session_key[6]= 7; pSkeys->session_key[7]= 8;
  pSkeys->peer_address.network_number= 1;
  pSkeys->peer_address.mac_address[0]= 'p';
  pSkeys->peer_address.address_size= 1; 
  pSkeys->next= new  BACnetSessionKey;										//					***001
  pSkeys= pSkeys->next;
  
  pSkeys->session_key[0]= 8; pSkeys->session_key[1]= 6; pSkeys->session_key[2]= 4; pSkeys->session_key[3]= 2;
  pSkeys->session_key[4]= 7; pSkeys->session_key[5]= 5; pSkeys->session_key[6]= 3; pSkeys->session_key[7]= 1;
  pSkeys->peer_address.network_number= 2;
  pSkeys->peer_address.mac_address[0]= 'o';
  pSkeys->peer_address.address_size= 1; 
  pSkeys->next= new  BACnetSessionKey;										//					***001
  pSkeys= pSkeys->next;

  pSkeys->session_key[0]= 1; pSkeys->session_key[1]= 3; pSkeys->session_key[2]= 5; pSkeys->session_key[3]= 7;
  pSkeys->session_key[4]= 2; pSkeys->session_key[5]= 4; pSkeys->session_key[6]= 6; pSkeys->session_key[7]= 8;
  pSkeys->peer_address.network_number= 3;
  pSkeys->peer_address.mac_address[0]= 'e';
  pSkeys->peer_address.address_size= 1; 
  pSkeys->next= NULL;

  gTestSkeys[1]= gTestSkeys[0]->next->next; // 1 element
  gTestSkeys[2]= NULL; // empty list   
  
  pSkeys= gTestSkeys[3]= new  BACnetSessionKey; 							//					***001
  pSkeys->session_key[0]= 1; pSkeys->session_key[1]= 3; pSkeys->session_key[2]= 5; pSkeys->session_key[3]= 7;
  pSkeys->session_key[4]= 2; pSkeys->session_key[5]= 4; pSkeys->session_key[6]= 6; pSkeys->session_key[7]= 8;
  pSkeys->peer_address.network_number= 4;
  pSkeys->peer_address.mac_address[0]= 'z';
  pSkeys->peer_address.address_size= 1; 
  pSkeys->next= NULL;
   
  // Time_Synchronization_Recipients (device)  List of BACnetRecipient
 
  BACnetRecipient far* pRecip= gTestRecip[0]= new  BACnetRecipient; // 3 elements				***001
  
  pRecip->choice= 0;
  pRecip->u.device= 4711;
  pRecip->next= new  BACnetRecipient;										//					***001
  pRecip= pRecip->next;
  
  pRecip->choice= 0;
  pRecip->u.device= 4712;
  pRecip->next= new  BACnetRecipient;										//					***001
  pRecip= pRecip->next;

  pRecip->choice= 0;
  pRecip->u.device= 4713;
  pRecip->next= NULL;

  gTestRecip[1]= gTestRecip[0]->next->next; // 1 element
  gTestRecip[2]= NULL; // empty list   
 
  pRecip= gTestRecip[3]= new  BACnetRecipient; 								//					***001
  pRecip->choice= 0;
  pRecip->u.device= 4714;
  pRecip->next= NULL;
 
  // Device_Address_Binding (device) List of BACnetAddressBinding 
  
  BACnetAddressBinding far* pDabind= gTestDabind[0]= new  BACnetAddressBinding; // 3 elements	***001

  pDabind->device_object_id= 4711;
  pDabind->device_address.network_number= 1;
  pDabind->device_address.mac_address[0]= 'P';
  pDabind->device_address.address_size= 1;
  pDabind->next= new  BACnetAddressBinding;									//					***001
  pDabind= pDabind->next;
  
  pDabind->device_object_id= 4712;
  pDabind->device_address.network_number= 2;
  pDabind->device_address.mac_address[0]= 'O';
  pDabind->device_address.address_size= 1;
  pDabind->next= new  BACnetAddressBinding;									//					***001
  pDabind= pDabind->next;
  
  pDabind->device_object_id= 4713;
  pDabind->device_address.network_number= 3;
  pDabind->device_address.mac_address[0]= 'E';
  pDabind->device_address.address_size= 1;
  pDabind->next= NULL;

  gTestDabind[1]= gTestDabind[0]->next->next; // 1 element
  gTestDabind[2]= NULL; // empty list   

  pDabind= gTestDabind[3]= new  BACnetAddressBinding; 						//					***001
  pDabind->device_object_id= 4714;
  pDabind->device_address.network_number= 4;
  pDabind->device_address.mac_address[0]= 'z';
  pDabind->device_address.address_size= 1;
  pDabind->next= NULL;

  // List_Of_Group_Members (group) List of ReadAccessSpecification
  
  BACnetReadAccessSpecification far* pRaslist= gTestRaslist[0]= new  BACnetReadAccessSpecification; // 3 elements ***001
  
  pRaslist->object_id= 4711;
  pRaslist->list_of_prop_ref= NULL;
  pRaslist->next= new  BACnetReadAccessSpecification;							//					***001
  pRaslist= pRaslist->next;
  
  pRaslist->object_id= 4712;
  pRaslist->list_of_prop_ref= NULL;
  pRaslist->next= new  BACnetReadAccessSpecification;							//					***001
  pRaslist= pRaslist->next;

  pRaslist->object_id= 4713;
  pRaslist->list_of_prop_ref= NULL;
  pRaslist->next= NULL;
 
  gTestRaslist[1]= gTestRaslist[0]->next->next; // 1 element
  gTestRaslist[2]= NULL; // empty list   
  
  pRaslist= gTestRaslist[3]= new  BACnetReadAccessSpecification;				//					***001
  pRaslist->object_id= 4714;
  pRaslist->list_of_prop_ref= NULL;
  pRaslist->next= NULL;

  // Present_Value (group) List of ReadAccessResult


  // Recipient_List (notification class) List of BACnetDestination
  
  BACnetDestination far* pReciplist= gTestReciplist[0]= new  BACnetDestination; // 3 elements***001
  
  pReciplist->valid_days= 0x44;
  pReciplist->from_time.hour= 16;
  pReciplist->from_time.minute= 17;
  pReciplist->from_time.second= dontcare;
  pReciplist->from_time.hundredths= dontcare;
  pReciplist->to_time.hour= 16;
  pReciplist->to_time.minute= 18;
  pReciplist->to_time.second= dontcare;
  pReciplist->to_time.hundredths= dontcare;
  pReciplist->recipient.next= NULL;
  pReciplist->recipient.choice= 0;
  pReciplist->recipient.u.device= 4711;
  pReciplist->process_id= 14;
  pReciplist->notification= 0;
  pReciplist->transitions= 0x55;
  pReciplist->next= new  BACnetDestination;					//					***001
  pReciplist= pReciplist->next;
  
  pReciplist->valid_days= 0x45;
  pReciplist->from_time.hour= 17;
  pReciplist->from_time.minute= 18;
  pReciplist->from_time.second= dontcare;
  pReciplist->from_time.hundredths= dontcare;
  pReciplist->to_time.hour= 17;
  pReciplist->to_time.minute= 19;
  pReciplist->to_time.second= dontcare;
  pReciplist->to_time.hundredths= dontcare;
  pReciplist->recipient.next= NULL;
  pReciplist->recipient.choice= 0;
  pReciplist->recipient.u.device= 4712;
  pReciplist->process_id= 15;
  pReciplist->notification= 0;
  pReciplist->transitions= 0x55;
  pReciplist->next= new  BACnetDestination;								//					***001
  pReciplist= pReciplist->next;

  pReciplist->valid_days= 0x46;
  pReciplist->from_time.hour= 17;
  pReciplist->from_time.minute= 18;
  pReciplist->from_time.second= dontcare;
  pReciplist->from_time.hundredths= dontcare;
  pReciplist->to_time.hour= 19;
  pReciplist->to_time.minute= 18;
  pReciplist->to_time.second= dontcare;
  pReciplist->to_time.hundredths= dontcare;
  pReciplist->recipient.next= NULL;
  pReciplist->recipient.choice= 0;
  pReciplist->recipient.u.device= 4713;
  pReciplist->process_id= 16;
  pReciplist->notification= 0;
  pReciplist->transitions= 0x44;
  pReciplist->next= NULL;

  gTestReciplist[1]= gTestReciplist[0]->next->next; // 1 element
  gTestReciplist[2]= NULL; // empty list   
  
  pReciplist= gTestReciplist[3]= new  BACnetDestination; 					//					***001
  pReciplist->valid_days= 0x47;
  pReciplist->from_time.hour= 18;
  pReciplist->from_time.minute= 16;
  pReciplist->from_time.second= dontcare;
  pReciplist->from_time.hundredths= dontcare;
  pReciplist->to_time.hour= 19;
  pReciplist->to_time.minute= 8;
  pReciplist->to_time.second= dontcare;
  pReciplist->to_time.hundredths= dontcare;
  pReciplist->recipient.next= NULL;
  pReciplist->recipient.choice= 0;
  pReciplist->recipient.u.device= 4714;
  pReciplist->process_id= 17;
  pReciplist->notification= 0;
  pReciplist->transitions= 0x44;
  pReciplist->next= NULL;


  // List_Of_Object_Property_References(schedule) List of BACnetObjectPropertyReference
   
  BACnetObjectPropertyReference far* pLopref= gTestLopref[0]= new  BACnetObjectPropertyReference; // 3 elements***001
  
  pLopref->object_id= 4711;
  pLopref->property_id= 75;
  pLopref->pa_index= NotAnArray;
  pLopref->next= new  BACnetObjectPropertyReference;							//					***001
  pLopref= pLopref->next;
  
  pLopref->object_id= 4712;
  pLopref->property_id= 75;
  pLopref->pa_index= NotAnArray;
  pLopref->next= new  BACnetObjectPropertyReference;							//					***001
  pLopref= pLopref->next;

  pLopref->object_id= 4713;
  pLopref->property_id= 75;
  pLopref->pa_index= NotAnArray;
  pLopref->next= NULL;

  gTestLopref[1]= gTestLopref[0]->next->next; // 1 element
  gTestLopref[2]= NULL; // empty list    

  pLopref= gTestLopref[3]= new  BACnetObjectPropertyReference;							//					***001
  pLopref->object_id= 4714;
  pLopref->property_id= 75;
  pLopref->pa_index= NotAnArray;
  pLopref->next= NULL;

}


// This function is used to release the dynamic allocated memory of the
// test property values.
void DeleteTestValues(void)
{ 
  BACnetExceptionSchedule far* px;
  BACnetSpecialEvent far* pe;
  BACnetTimeValue far* pt;
      
  for (int i= 0; i<nTEST; i++)
    {
      delete gTestAct[i][0];
      
      px= gTestXSched[i];
      pe= px->special_event;
      pt= pe->list_of_time_values;
      
      delete pt;
      delete pe;
      delete px;
    }  
      
  // delete ListOf-Property lists

  BACnetCalendarEntry far* pCal= gTestCalist[0];
  BACnetCalendarEntry far* pCal2;
  while (pCal!=NULL) { pCal2= pCal->next; delete pCal; pCal= pCal2; }
  delete gTestCalist[3];

  BACnetVTClassList far* pVtcl= gTestVtcl[0];
  BACnetVTClassList far* pVtcl2;
  while (pVtcl!=NULL) { pVtcl2= pVtcl->next; delete pVtcl; pVtcl= pVtcl2; }
  delete gTestVtcl[3];

  BACnetVTSession far* pVtse= gTestVtse[0];
  BACnetVTSession far* pVtse2;
  while (pVtse!=NULL) { pVtse2= pVtse->next; delete pVtse; pVtse= pVtse2; }
  delete gTestVtse[3];

  BACnetRecipient far* pRecip= gTestRecip[0];
  BACnetRecipient far* pRecip2;
  while (pRecip!=NULL) { pRecip2= pRecip->next; delete pRecip; pRecip= pRecip2; }
  delete gTestRecip[3];

  BACnetSessionKey far* pSkeys= gTestSkeys[0];
  BACnetSessionKey far* pSkeys2;
  while (pSkeys!=NULL) { pSkeys2= pSkeys->next; delete pSkeys; pSkeys= pSkeys2; }
  delete gTestSkeys[3];

  BACnetAddressBinding far* pDabind= gTestDabind[0];
  BACnetAddressBinding far* pDabind2;
  while (pDabind!=NULL) { pDabind2= pDabind->next; delete pDabind; pDabind= pDabind2; }
  delete gTestDabind[3];

  BACnetReadAccessSpecification far* pRaslist= gTestRaslist[0];
  BACnetReadAccessSpecification far* pRaslist2;
  while (pRaslist!=NULL) { pRaslist2= pRaslist->next; delete pRaslist; pRaslist= pRaslist2; }
  delete gTestRaslist[3];

  BACnetDestination far* pReciplist= gTestReciplist[0];
  BACnetDestination far* pReciplist2;
  while (pReciplist!=NULL) { pReciplist2= pReciplist->next; delete pReciplist; pReciplist= pReciplist2; }
  delete gTestReciplist[3];

  BACnetObjectPropertyReference far* pLopref= gTestLopref[0];
  BACnetObjectPropertyReference far* pLopref2;
  while (pLopref!=NULL) { pLopref2= pLopref->next; delete pLopref; pLopref= pLopref2; }
  delete gTestLopref[3];

}


// encode BACnetDate
octet far* eBACnetDate(octet far* p, const BACnetDate far* pv, int TagNr)
{
  if (pv==NULL) return(p);
  p= eDATESTRING(p,pv->year,pv->month,pv->day_of_month,pv->day_of_week);
  return p;
}

// encode BACnetTime
octet far* eBACnetTime(octet far* p, const BACnetTime far* pv, int TagNr)
{
  if (pv==NULL) return(p);
  p= eTIMESTRING(p,pv->hour,pv->minute,pv->second,pv->hundredths);
  return p;
}

             
// encode BACnetCalendarEntry
octet far* eCALIST(octet far* p, const BACnetCalendarEntry far* pv, int TagNr)
 {
  while (pv!=NULL)
    { 
      switch (pv->choice)
        { case 0: // BACnetDate
            *p++= 0x0E; // opening tag 
              *p++= 0xA4;
              memcpy(p,&pv->u,4); p+= 4;							//					***001
            *p++= 0x0F;  // closing tag 
            break;
          case 1: // BACnetDateRange
            *p++= 0x1E; // opening tag
            p= eDTRANGE(p,(BACnetDateRange far*) &pv->u);
            *p++= 0x1F; // closing tag
            break;
          case 2: // BACnetWeekNDay
            *p++= 0x2E; // opening tag
            p= eOCTETSTRING(p,(octet far*)&pv->u.weekNday,3);
            *p++= 0x2F; // closing tag
            break;
        }    
      pv= pv->next;
    }
  return p;  
}

// encode list of BACnetPropertyReference             
octet far* ePROPREF(octet far* p, const BACnetPropertyReference far* pv, int TagNr)
{ while (pv!=NULL)
    { p= ecUINT(p,0x00,pv->property_id);
      if (pv->pa_index!=NotAnArray) // optional: array index
        { p= ecUINT(p,0x18,pv->pa_index); }
      pv= pv->next;
    }      
  return p;
}

// encode list of BACnetObjectPropertyReference             
octet far* eLOPREF(octet far* p, const BACnetObjectPropertyReference far* pv, int TagNr)
{ while (pv!=NULL)
    { p= etagOBJECTID(p,0x08,pv->object_id);
      p= ecUINT(p,0x18,pv->property_id);
      if (pv->pa_index!=NotAnArray) // optional: array index
        { p= ecUINT(p,0x28,pv->pa_index); }
      pv= pv->next;
    }      
  return p;
}

// encode BACnetReadAccessSpecification             
octet far* eRASLIST(octet far* p, const BACnetReadAccessSpecification far* pv, int TagNr)
{ 
  while (pv!=NULL)
    { // encode object identifier      
      p= etagOBJECTID(p,0x08,pv->object_id);
      // encode property reference list
      *(p++)= 0x1E; // opening tag
      p= ePROPREF(p,pv->list_of_prop_ref);
      *(p++) = 0x1F; // closing tag
      pv= pv->next;
    }
  return p;  
}

// encode BACnetDateTime             
octet far* eDT(octet far* p, const BACnetDateTime far* pv, int TagNr)
{ 
  if (pv==NULL) return p;
  *p++= 0xA4; // application tag
  memcpy(p,&pv->date,4); p+=4;									//					***001
  *p++= 0xB4; // application tag
  memcpy(p,&pv->time,4); p+=4; 									//					***001
  return p;
}

// encode array of object identifier
octet far* eLOBJ(octet far* p, const BACnetObjectIdentifier far* pv, int ArrayIndex, int TagNr)
{ int i;
        
  switch (ArrayIndex)
    {
      case -1: // all elements 
        while (pv!=NULL)
          { // encode object identifier
            p= etagOBJECTID(p,0xC0,pv->object_id);
            pv= pv->next;
          }   
        break;  
        
      case 0: // number of elements
        i= 0;
        while (pv!=NULL)
          { i++;
            pv= pv->next;
          } 
        p= eUINT(p,i);    
        break;  
        
      default: // ArrayIndex element   
        i= ArrayIndex-1;
        while ((pv!=NULL) && (i>0))
          { i--; pv= pv->next; }
        if ((pv==NULL) || (ArrayIndex<0)) return p; // invalid array index  
        p= etagOBJECTID(p,0xC0,pv->object_id);
        break;
    }    
  return p;  
}

// encode float priority array
octet far* ePAF(octet far* p, const float far* pv, int ArrayIndex, int TagNr)
{ int i;
  if (pv==NULL) return p;

  switch(ArrayIndex)
    { 
      case -1: // all elements
        for (i=0; i<nPRIO; i++)
          {   
            if (pv[i]==fpaNULL) 
              p= eNULL(p);
            else 
              p= eREAL(p,pv[i]);
          }
        break;
        
      case 0:  // number of elements
        p= eUINT(p,nPRIO);
        break;
        
      default: // ArrayIndex element     
        if ((ArrayIndex>0)&&(ArrayIndex<=nPRIO))
          {
            if (pv[ArrayIndex-1]==fpaNULL) 
              p= eNULL(p);
            else 
              p= eREAL(p,pv[ArrayIndex-1]);
          }
        break;
    }     
  return p;
}

// encode BACnetBinaryPV
//MOdified by Liangping Xu, 2002-8-5
//octet far* ePAB(octet far* p, const enum BACnetBinaryPV far* pv, int ArrayIndex, int TagNr)
octet far* ePAB(octet far* p, const word far* pv, int ArrayIndex, int TagNr)
{ int i;
  if (pv==NULL) return p;
  
  switch(ArrayIndex)                                                           
    { 
      case -1: // all elements
        for (i=0; i<nPRIO; i++)
          { 
            if (pv[i]!=bpaNULL) p= eENUM(p,pv[i]);
            else p= eNULL(p);
          }
        break;  

      case 0:  // number of elements
        p= eUINT(p,nPRIO);
        break;
        
      default: // ArrayIndex element     
        if ((ArrayIndex>0)&&(ArrayIndex<=nPRIO))
         
		//p= eENUM(p,pv[ArrayIndex-1]);  -- Marked by Liangping Xu, 2002-8-5
		  if(pv[ArrayIndex-1]!=bpaNULL) p= eENUM(p,pv[ArrayIndex-1]);       
            else p= eNULL(p);

        break;
    }
  return p;
}

// encode uw priority array
octet far* ePAU(octet far* p, const word far* pv, int ArrayIndex, int TagNr)
{ int i;
  if (pv==NULL) return p;

  switch(ArrayIndex)
    { 
      case -1: // all elements
        for (i=0; i<nPRIO; i++)
          { 
            if (pv[i]!=upaNULL) p= eUINT(p,pv[i]);
            else p= eNULL(p);
          }
        break;  
          
      case 0:  // number of elements
        p= eUINT(p,nPRIO);
        break;
        
      default: // ArrayIndex element     
        if ((ArrayIndex>0)&&(ArrayIndex<=nPRIO))
          p= eUINT(p,pv[ArrayIndex-1]);
        break;
    }
  return p;
}

// encode uw array  
octet far* eUWARR(octet far* p, const word far* pv, int num, int ArrayIndex, int TagNr)
{ int i;
  if (pv==NULL) return p;
  switch(ArrayIndex)
    { 
      case -1: // all elements
        for (i=0; i<num; i++) p= eUINT(p,pv[i]);
        break;  

      case 0:  // number of elements
        p= eUINT(p,num);
        break;
        
      default: // ArrayIndex element     
        if ((ArrayIndex>0)&&(ArrayIndex<=num))
          p= eUINT(p,pv[ArrayIndex-1]);
        break;
    }    
  return p;
}


// encode action text array
octet far* eACTEXT(octet far* p,char far** pv, int num, int ArrayIndex, int TagNr)
{ int i; word n;
  if (pv==NULL) return p;

  switch(ArrayIndex)
    { 
      case -1: // all elements
        for (i= 0; i<num; i++)
          { if (pv[i]==NULL) break;
            n= strlen(pv[i]);									//					***001
            p= eCHARSTRING(p,pv[i],0x70,n,ANSI,0);
          }  
        break;    

      case 0:  // number of elements
        p= eUINT(p,num);
        break;
        
      default: // ArrayIndex element     
        if ((ArrayIndex>0)&&(ArrayIndex<=num))
          p= eCHARSTRING(p,pv[ArrayIndex-1],0x70,strlen(pv[ArrayIndex-1]),ANSI,0);		//					***001
        break;
    }    
  return p;  
}


// encode array of BACnetActionCommand
octet far* eACT(octet far* p, BACnetActionCommand far** pv, int num, int ArrayIndex, int TagNr)
{ BACnetActionCommand far* pact;
  int i;
  octet far* s;
  if (pv==NULL) return p;
  
  switch (ArrayIndex)
    { 
      case -1: // all elements -------------------------------------------------------------
        for (i=0; i<num; i++)
          {
            pact= pv[i];
            *p++= 0x0E; // open tag
            while (pact!=NULL)
              {
                if (pact->device_id!=0) // optional
                  { p= etagOBJECTID(p,0x00,pact->device_id); }
                p= etagOBJECTID(p,0x10,pact->object_id);  
                p= ecUINT(p,0x20,pact->property_id);
                if (pact->pa_index!=NotAnArray) // optional: array index
                  { p= ecUINT(p,0x30,pact->pa_index); }
                s= p;  
                switch(pact->value_type)
                  { case BPV:
                      p= eENUM(p,pact->av.bproperty_value);
                      break;
                    case UNS:
                      p= eUINT(p,pact->av.uproperty_value);
                      break;
                    case FLT:
                      p= eREAL(p,pact->av.fproperty_value);
                      break;
                  }
                *s= 0x48 | (*s & 0x07);  
                if ( (pact->priority>=1) && (pact->priority<=16) ) // optional: priority
                  { p= ecUINT(p,0x50,pact->priority); }
                if (pact->post_delay!=0) // optional: post_delay
                  { s= p;
                    p= eUINT(p,pact->post_delay); 
                    *s= 0x68 | (*s & 0x07); // context tag [6]
                  }
                p= ecBOOL(p,0x70,pact->quit_on_failure);  
                p= ecBOOL(p,0x80,pact->write_successful);
                pact= pact->next;
              }
            *p++= 0x0F; // close tag  
          } // end for
        break;
        
      case 0: // number of elements -------------------------------------------------------------
        p= eUINT(p,num);
        break;
          
      default: // ArrayIndex element ------------------------------------------------------------
        if ((ArrayIndex<=0)||(ArrayIndex>num)) return p; // invalid ArrayIndex
        pact= pv[ArrayIndex];
        *p++= 0x0E; // open tag
        while (pact!=NULL)
          {
            if (pact->device_id!=0) // optional
              { p= etagOBJECTID(p,0x00,pact->device_id); }
            p= etagOBJECTID(p,0x10,pact->object_id);  
            p= ecUINT(p,0x20,pact->property_id);
            if (pact->pa_index!=NotAnArray) // optional: array index
              { p= ecUINT(p,0x30,pact->pa_index); }
            s= p;  
            switch(pact->value_type)
              { case BPV:
                  p= eENUM(p,pact->av.bproperty_value);
                  break;
                case UNS:
                  p= eUINT(p,pact->av.uproperty_value);
                  break;
                case FLT:
                  p= eREAL(p,pact->av.fproperty_value);
                  break;
              }
            *s= 0x48 | (*s & 0x07);  
            if ( (pact->priority>=1) && (pact->priority<=16) ) // optional: priority
              { p= ecUINT(p,0x50,pact->priority); }
            if (pact->post_delay!=0) // optional: post_delay
              { s= p;
                p= eUINT(p,pact->post_delay); 
                *s= 0x68 | (*s & 0x07); // context tag [6]
              }
            p= ecBOOL(p,0x70,pact->quit_on_failure);  
            p= ecBOOL(p,0x80,pact->write_successful);
            pact= pact->next;
          }
        *p++= 0x0F; // close tag  
        break;
          
    } // end switch
            
  return p;
}               


// encode array of BACnetTimeValue
octet far* eWSCHED(octet far* p, BACnetTimeValue far** pv, int num, int ArrayIndex, int TagNr)
{ int i;
  if (pv==NULL) return p;
  switch(ArrayIndex)
    { 
      case -1: // all elements
        for (i=0; i<num; i++)
          { *p++= 0x0E; // day-schedule [0]
            p= eTIMEVALUE(p,pv[i]);
            *p++= 0x0F;  
          }    
        break;  

      case 0:  // number of elements
        p= eUINT(p,num);
        break;
        
      default: // ArrayIndex element     
        if ((ArrayIndex>0)&&(ArrayIndex<=num))
          { *p++= 0x0E; // day-schedule [0]
            p= eTIMEVALUE(p,pv[ArrayIndex-1]);
            *p++= 0x0F;  
          }  
        break;
    }      
  return p;
}

// encode array BACnetExceptionSchedule
octet far* eXSCHED(octet far* p, BACnetExceptionSchedule far* pv, int ArrayIndex, int TagNr)
{ int i;
  BACnetSpecialEvent far* pse;
  if (pv==NULL) return p;
  
  switch (ArrayIndex)
    { 
    
      case -1: // all elements -------------------------------------------------------------------
        pse= pv->special_event;
        while (pse!=NULL)
          { // encode period
            switch (pse->choice)
              { case 0: // [0] CalendarEntry [0]
                  *p++= 0x0E;
                    *p++= 0x0E;
                      *p++= 0xA4; 
                      memcpy(p,&pse->u.date,4); p+=4; 					//					***001
                    *p++= 0x0F;
                  *p++= 0x0F;
                  break;
                case 1: // [0] CalendarEntry [1]
                  *p++= 0x0E;
                    *p++= 0x1E;
                      p= eDTRANGE(p,&pse->u.date_range);
                    *p++= 0x1F;
                  *p++= 0x0F;
                  break;
                case 2: // [0] CalendarEntry [2]
                  *p++= 0x0E;
                    *p++= 0x2E;
                      p= eOCTETSTRING(p,(octet far*)&pse->u.weekNday,3);
                    *p++= 0x2F;
                  *p++= 0x0F;
                  break;
                case 3: // [1] CalendarReference 
                  *p++= 0x1E;
                    p= etagOBJECTID(p,0xC0,pse->u.calendar_ref.object_id);
                  *p++= 0x1F;
                  break;
              }
            *p++= 0x2E;  
              p= eTIMEVALUE(p,pse->list_of_time_values); // encode timevalue [2]
            *p++= 0x2F;
            p= ecUINT(p,0x30, pse->event_priority);      // encode eventPriority [3]  
            pse= pse->next;
          }
        break;
        
      case 0: // number of elements --------------------------------------------------------------  
        pse= pv->special_event;
        i= 0;
        while (pse!=NULL)
          { i++;
            pse= pse->next;
          }
        p= eUINT(p,i);
        break;
        
      default: // ArrayIndex element -------------------------------------------------------------
        pse= pv->special_event;
        i= ArrayIndex-1;
        while ((pse!=NULL) && (i>0))
          { i--; pse= pse->next; }
        if ((pse==NULL) || (ArrayIndex<0)) return p; // invalid array index  
        switch (pse->choice)
          { case 0: // [0] CalendarEntry [0]
              *p++= 0x0E;
                 *p++= 0x0E;
                   *p++= 0xA4; 
                   memcpy(p,&pse->u.date,4); p+=4; 					//					***001
                 *p++= 0x0F;
              *p++= 0x0F;
              break;
            case 1: // [0] CalendarEntry [1]
              *p++= 0x0E;
                *p++= 0x1E;
                  p= eDTRANGE(p,&pse->u.date_range);
                *p++= 0x1F;
              *p++= 0x0F;
              break;
            case 2: // [0] CalendarEntry [2]
              *p++= 0x0E;
                *p++= 0x2E;
                  p= eOCTETSTRING(p,(octet far*)&pse->u.weekNday,3);
                *p++= 0x2F;
              *p++= 0x0F;
              break;
            case 3: // [1] CalendarReference 
              *p++= 0x1E;
                p= etagOBJECTID(p,0xC0,pse->u.calendar_ref.object_id);
              *p++= 0x1F;
              break;
          }
        *p++= 0x2E;  
          p= eTIMEVALUE(p,pse->list_of_time_values); // encode timevalue [2]
        *p++= 0x2F;
        p= ecUINT(p,0x30, pse->event_priority);      // encode eventPriority [3]  
        pse= pse->next;
        break;  
    }  
  return p;
}     


// encode BACnetDateRange
octet far* eDTRANGE(octet far* p, const BACnetDateRange far* pv, int TagNr)
{ 
  if (pv==NULL) return p;
  *p++= 0xA4; // application tag
  memcpy(p,&pv->start_date,4); p+= 4;					//					***001
  *p++= 0xA4; // application tag
  memcpy(p,&pv->end_date,4); p+= 4;						//					***001
  return p;
}


// encode a context tagged boolean value
octet far* ecBOOL(octet far* p, octet tag, boolean bval)
{
  *p++= (tag & 0xF0) | 0x09; // tag | context 0x08, length 0x01
  if (bval) *p++= 0x01;
  else *p++= 0x00;
  return p;
}  

//encocde a context tagged property value
octet far* ecUINT(octet far* p, octet tag, dword property_id)
{ octet far* s= p;
  p= eUINT(p,property_id);
  *s= (tag & 0xF0) | 0x08 | (*s & 0x07); // tag nr | context | length
  return p;
  
}

// encode BACnetVTClassList
octet far* eVTCL(octet far* p, const BACnetVTClassList far* pv, int TagNr)
{ 
  while (pv!=NULL)
   { p= eENUM(p,pv->vtclass);
     pv= pv->next;
   }
  return p;
}

// encode BACnetEventParam
octet far* eEVPARM(octet far* p, const BACnetEventParameter far* pv, int TagNr)
{ octet far* s;
  //BACnetListBitstringValue far* pbit;
  BACnetPropertyStates far* ps;
  if (pv==NULL) return p;
  switch (pv->event_type)
    {
      case CHANGE_OF_BITSTRING: // [0]
        *p++= 0x0E;
        	s= p;
        	p= eUINT(p,pv->time_delay);
        	*s= 0x08 | (*s & 0x07); // [0]
        	//p= eBITSTRING(p,(byte far*)&pv->bitmask,0x10,8); // [1]
        	*p++= 0x2E; // [2]
        	//pbit= pv->list_bitstring_value;
        	//    while (pbit!=NULL)
        	//      { p= eBITSTRING(p,&pbit->list_bitstring_value,0x80,8);
        	//        pbit= pbit->next;
        	//      }
        	*p++= 0x2F;
        *p++= 0x0F;
        break;
      case CHANGE_OF_STATE: // [1]
        *p++= 0x1E;
        	s= p;
        	p= eUINT(p,pv->time_delay);
        	*s= 0x08 | (*s & 0x07); // [0]
        	*p++= 0x1E; // [1]
        	  ps= pv->list_of_value;
        	  while (ps!=NULL)
        	    { // enum or bool?
        	      ps= ps->next;
        	    }
        	*p++= 0x1F;
        *p++= 0x1F;
        break;
      case CHANGE_OF_VALUE: // [2]
        *p++= 0x2E;
        	s= p;
        	p= eUINT(p,pv->time_delay);
        	*s= 0x08 | (*s & 0x07); // [0]
        	    *p++= 0x1E; // [1]
        	        s=p;
                    p= eREAL(p,pv->low_diff_limit); // ??? where is the variable in the structure?
                    *s= 0x18 | (*s & 0x07); // [1] REAL 
        	    *p++= 0x1F; 
        *p++= 0x2F;
        break;
      case COMMAND_FAILURE: // [3]
        *p++= 0x3E;
        	s= p;
        	p= eUINT(p,pv->time_delay);
        	*s= 0x08 | (*s & 0x07); // [0]
            *p++= 0x1E; 
                p= eLOPREF(p,&pv->setpoint_ref); // [1] 
            *p++= 0x1F; 
        *p++= 0x3F;
        break;
      case FLOATING_LIMIT: // [4]
        *p++= 0x4E;
            s= p;
            p= eUINT(p,pv->time_delay);
            *s= 0x08 | (*s & 0x07); // [0]
            *p++= 0x1E; 
                p= eLOPREF(p,&pv->setpoint_ref); // [1] 
            *p++= 0x1F; 
            s= p;
            p= eREAL(p,pv->low_diff_limit);
            *s= 0x28 | (*s & 0x07); // [2]
            s= p;
            p= eREAL(p,pv->high_diff_limit);
            *s= 0x38 | (*s & 0x07); // [3]
            s= p;
            p= eREAL(p,pv->deadband);
            *s= 0x48 | (*s & 0x07); // [4]
        *p++= 0x4F;
        break;
      case OUT_OF_RANGE: // [5]
        *p++= 0x5E;
        	s= p;
        	p= eUINT(p,pv->time_delay);
        	*s= 0x08 | (*s & 0x07); // [0]
        	s= p;
        	p= eREAL(p,pv->low_limit);
        	*s= 0x18 | (*s & 0x07); // [1]
        	s= p;
        	p= eREAL(p,pv->high_limit);
        	*s= 0x28 | (*s & 0x07); // [2]
        	s= p;
        	p= eREAL(p,pv->deadband);
        	*s= 0x38 | (*s & 0x07); // [3]
        *p++= 0x5F;
        break;
	  //Added by Zhu Zhenhua, 2004-5-20
      case BUFFER_READY: // [10]
        *p++= 0xAE;
        	s= p;
        	p= eUINT(p,pv->notification_threshold);
        	*s= 0x08 | (*s & 0x07); // [0]
        	s= p;
        	p= eDWORD(p,pv->previous_notification_count,0x20,FALSE);
        	*s= 0x18 | (*s & 0x07); // [1]       
        *p++= 0xAF;
        break;
    }
  return p;
}

// encode BACnetSessionKey
octet far* eSKEYS(octet far* p, const BACnetSessionKey far* pv, int TagNr)  
{ 
  while (pv!=NULL)
    { 
      p= eOCTETSTRING(p,(octet *)(pv->session_key),8);								//				***001
      p= eUINT(p,(pv->peer_address.network_number));
      p= eOCTETSTRING(p,(octet *)(pv->peer_address.mac_address),pv->peer_address.address_size);	//	***001
      pv= pv->next;
    }
  return p;
}
                         
// encode BACnetAddressBinding
octet far* eDABIND(octet far* p, const BACnetAddressBinding far* pv, int TagNr)   
{ 
  while (pv!=NULL)
    { // device object id
      p= etagOBJECTID(p,0xC0,pv->device_object_id);
      // BACnetAddress
      p= eUINT(p,pv->device_address.network_number);
      p= eOCTETSTRING(p,(octet *)(pv->device_address.mac_address),pv->device_address.address_size);	//***001
      pv= pv->next;
    } 
  return p;
}
          
// encode BACnetRecipient
octet far* eRECIP(octet far* p, const BACnetRecipient far* pv, int TagNr)     
{
  while (pv!=NULL)
    { 
      if (pv->choice==0) // BACnetObjectIdentifier
        {
          p= etagOBJECTID(p,0x08,pv->u.device);
        }
      else // BACnetAddress
        { *p++= 0x1E;
          p= eUINT(p,pv->u.address.network_number);
          p= eOCTETSTRING(p,(octet *)(pv->u.address.mac_address),pv->u.address.address_size);	//	***001
          *p++= 0x1F;
        }  
      pv= pv->next;
    }
  return p;
}

// encode BACnetDestination list
octet far* eRECIPLIST(octet far* p, const BACnetDestination far* pv, int TagNr) 
{ 
  while (pv!=NULL)
   {              
     p= eBITSTRING(p,(byte far*)&pv->valid_days,0x80,7);
     *p++= 0xB4;
     memcpy(p,&pv->from_time,4); p+= 4;						//					***001
     *p++= 0xB4;
     memcpy(p,&pv->to_time,4); p+= 4;						//					***001
     p= eRECIP(p,&pv->recipient);
     p= eUINT(p,pv->process_id);
     p= eBOOL(p,pv->notification);
     p= eBITSTRING(p,(byte far*)&pv->transitions,0x80,3);
     pv= pv->next;
   }
  return p;
}


// encode BACnetTimeValue
octet far* eTIMEVALUE(octet far* p, BACnetTimeValue far* ptv, int TagNr)
{
  while (ptv!=NULL)
    { *p++= 0xB4; // application tag
      memcpy(p,&ptv->time,4); p+=4;							//					***001
      switch(ptv->value_type)
        { case BPV:
            p= eENUM(p,ptv->av.bproperty_value);
            break;
          case UNS:
            p= eUINT(p,ptv->av.uproperty_value);
            break;
          case FLT:
            p= eREAL(p,ptv->av.fproperty_value);
            break;
        }
      ptv= ptv->next;
    }
  return p;
}


// encode BACnetSetpointReference
octet far* eSETREF(octet far* p, const BACnetObjectPropertyReference far* pv, int TagNr)    
{
  if (pv==NULL) { p= eNULL(p); return p; } // optional: SetpointReference
  while (pv!=NULL)
    { *p++= 0x0E; // [0] opening tag 
       p= eLOPREF(p,pv);
       *p++= 0x0F;
       pv= pv->next;
    }
  return p;
}

// encode list of unsigned
octet far* eSTAVALS(octet far* p,const octet far* pv, int TagNr)   
{ if (p==NULL) return p;
  for (int i=0; pv[i]>0; i++)
    p= eUINT(p,pv[i]);
  return p;
}
             
// encode BACnetVTSession
octet far* eVTSE(octet far* p, const BACnetVTSession far* pv, int TagNr)      
{ 
  while (pv!=NULL)
    { p= eUINT(p,pv->local_session_id);
      p= eUINT(p,pv->remote_session_id);
      // BACnetAddress
      p= eUINT(p,pv->remote_address.network_number);
      p= eOCTETSTRING(p,(octet *)(pv->remote_address.mac_address),pv->remote_address.address_size);
      pv= pv->next;
    }
  return p;
}
  

// This function is used to encode any property value according to the datatype.
// in:		p	buffer
//		msg	action to perform (test value, property value)
// returns:	buffer_address + number_of_encoded_octets
octet far* eASN_1_AnyType(octet far* p, PVMessage far* msg)
{
  int i= msg->TestNr;

  if ( (msg->Action==RND_ENCODEPROP) && ((msg->TestNr<0)||(msg->TestNr>1)) )
    msg->TestNr= 0;

  switch (msg->pt) //parse type
    {
     case ob_id: // an object identifier -----------------------
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP:     // encode from database
           case RND_ENCODEPROP: // encode a test value (never)
             p= etagOBJECTID(p, 0xC0, *((dword far*)msg->pv) );
             break;  
         }  
       break;
       
     case et: // an enumeration table --------------------------
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eENUM(p, *((word far*)(msg->pv)) ); 
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eENUM(p, gTestEnum[i]);   
             break;
         }    
       break;                                                   
    //***Added by LiangPing Xu, 2002-8-5
      case ssint://short signed int
		switch(msg->Action)
		   {
			case SETPROP:
             break;
            case ENCODEPROP: // encode from database
             p= eINT(p, *((short far*)(msg->pv)) ); 
             break;
            case RND_ENCODEPROP: // encode a test value 
             p= eINT(p, gTestInt[i]); 
             break;
		   }
		break;
	   
    //***Ended by LiangPing Xu, 2002-8-5
     case flt: // float ----------------------------------------
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eREAL(p, *((float far*)(msg->pv)) ); 
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eREAL(p, gTestReal[i]);   
             break;
         }    
       break;
       
     case bits: // octet of 1 or 0 flags
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eBITSTRING(p, (byte far*)msg->pv,0x80,msg->UsedBits); 
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eBITSTRING(p,&gTestBits[i],0x80,gTestUsedBits);
             break;
         }    
       break;          

     case u127: // 1..127 -----------------------------------------------
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eUINT(p, *((octet far*)(msg->pv)) ); 
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eUINT(p, gTestInt[i]);   
             break;
         }    
       break;

     case u16:  // 1..16
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eUINT(p, *((octet far*)(msg->pv)) ); 
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eUINT(p, gTestInt[i]);   
             break;
         }    
       break;

     case ud:   // unsigned dword
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eUINT(p, *((dword far*)(msg->pv)) ); 
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eUINT(p, gTestInt[i]);   
             break;
         }    
       break;                     

     case uw:   // unsigned word
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eUINT(p, *((word far*)(msg->pv)) ); 
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eUINT(p, gTestInt[i]);   
             break;
         }    
       break;
       
     case sw:   // signed word ------------------------------------------
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eINT(p, *((short far*)(msg->pv)) ); 
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eINT(p, gTestInt[i]); 
             break;
         }    
       break;
       
     case s10:  // char [10] --------------------------------------------
     case s32:  // char [32]
     case s64:  // char [64]
     case s132: // char [132]
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eCHARSTRING(p,(char far*)msg->pv,0x70,strlen((char far*)msg->pv),ANSI,0); //***001
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eCHARSTRING(p,gTestString[i],0x70,strlen(gTestString[i]),ANSI,0);		//	***001
             break;
         }    
       break;
                                                             
     case ebool: // boolean enumeration ---------------------------------
     
       switch(msg->Action)
         {
           case SET_TRUE:
             p= eBOOL(p, 1); // encode true  value  
             break;
           case SET_FALSE:
             p= eBOOL(p, 0); // encode false value  
             break;  
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eBOOL(p, *((boolean far*)(msg->pv)) ); 
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eBOOL(p, gTestBool[i]); // random value  
             break;
         }    
       break;
       
     case enull: // null enumeration ------------------------------------
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eNULL(p); 
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eNULL(p);  
             break;
         }    
       break;

     case ptDate: // date
       p= eBACnetDate(p,(BACnetDate far*)msg->pv);
       break;
     
     case ptTime: // time
       p= eBACnetTime(p,(BACnetTime far*)msg->pv);
       break;
     
     case dt: // date + time
       switch(msg->Action)
         {
           case SETPROP: 
// jjb             new CBACnetDateTimeDlg((BACnetDateTime far*)msg->pv,CWnd::FromHandle(msg->hw));
             break;
           case ENCODEPROP: // encode from database
             p= eDT(p,(BACnetDateTime far*)msg->pv);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eDT(p,&gTestDT[i]);
             break;
         }    
       break;

     case calist: // list of calendarentry
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eCALIST(p,(BACnetCalendarEntry far*)msg->pv);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eCALIST(p,gTestCalist[i]);
             break;
         }    
       break;

     case paf: // priority array flt
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= ePAF(p,(float far*)msg->pv,msg->ArrayIndex);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= ePAF(p,gTestPAF[i]);
             break;
         }    
       break;

     case pab: // priority array bpv
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
          //   p= ePAB(p,(enum BACnetBinaryPV far*)msg->pv,msg->ArrayIndex);--Marked by Liangping Xu,2002-8-5
            p= ePAB(p,(word far*)msg->pv,msg->ArrayIndex);
			   break;
           case RND_ENCODEPROP: // encode a test value 
             //p= ePAB(p,gTestPAB[i]);    --Marked by Liangping Xu,2002-8-5
			 p= ePAB(p,(const word *)gTestPAB[i]);

             break;
         }    
       break;

     case pau: // priority array uw
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= ePAU(p,(word far*)msg->pv,msg->ArrayIndex);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= ePAU(p,gTestPAU[i]);
             break;
         }    
       break;

     case dtrange: // range of dates
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eDTRANGE(p,(BACnetDateRange far*)msg->pv);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eDTRANGE(p,&gTestDTRange[i]);
             break;
         }    
       break;

     case raslist: // list of readaccessspecs
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eRASLIST(p,(BACnetReadAccessSpecification far*)msg->pv);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eRASLIST(p,gTestRaslist[i]);
             break;
         }    
       break;

     case lobj: // list of objects
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eLOBJ(p,(BACnetObjectIdentifier far*)msg->pv,msg->ArrayIndex);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eLOBJ(p,&gTestLObj[i]);
             break;
         }    
       break;
       
     case act: // action array
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eACT(p,(BACnetActionCommand far**)msg->pv, msg->Num,msg->ArrayIndex);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eACT(p,gTestAct[i],1);
             break;
         }    
       break;

     case statext: // state text array
     case actext:  // action_text array
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eACTEXT(p,(char far**)msg->pv,msg->Num,msg->ArrayIndex);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eACTEXT(p,gTestActext[i],1);
             break;
         }    
       break;

     case vtcl: // vt classes
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eVTCL(p,(BACnetVTClassList far*)msg->pv);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eVTCL(p,gTestVtcl[i]);
             break;
         }    
       break;

     case evparm: // event parameter
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eEVPARM(p,(BACnetEventParameter far*)msg->pv);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eEVPARM(p,&gTestEvparm[i]);
             break;
         }    
       break;

     case skeys: // session keys
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eSKEYS(p,(BACnetSessionKey far*)msg->pv);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eSKEYS(p,gTestSkeys[i]);
             break;
         }    
       break;

     case dabind: // device address bindings
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eDABIND(p,(BACnetAddressBinding far*)msg->pv); 
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eDABIND(p,gTestDabind[i]); 
             break;
         }    
       break;

     case tsrecip: // time synch recipients
     case recip: // recipient
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eRECIP(p,(BACnetRecipient far*)msg->pv);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eRECIP(p,gTestRecip[i]);
             break;
         }    
       break;

     case reciplist: // list of BACnetDestination
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eRECIPLIST(p,(BACnetDestination far*)msg->pv);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eRECIPLIST(p,gTestReciplist[i]);
             break;
         }    
       break;

     case xsched:    // exception schedule: array[] of specialevent
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eXSCHED(p,(BACnetExceptionSchedule far*)msg->pv,msg->ArrayIndex);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eXSCHED(p,gTestXSched[i]);
             break;
         }    
       break;

     case wsched: // weekly schedule: array[7] of list of timevalue
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eWSCHED(p,(BACnetTimeValue far**)msg->pv,7,msg->ArrayIndex);
             break;
           case RND_ENCODEPROP: // encode a test value
             p= eWSCHED(p,gTestWsched[i],7); 
             break;
         }    
       break;

     case stavals: // list of unsigned
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eSTAVALS(p,(octet far*)msg->pv);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eSTAVALS(p,gTestStavals[i]);
             break;
         }    
       break;
     
     case propref: // list of object prop refs
     case lopref:  // list of object prop refs
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eLOPREF(p,(BACnetObjectPropertyReference far*)msg->pv);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eLOPREF(p,gTestLopref[i]);
             break;
         }    
       break;

     case setref: // setpoint reference
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eSETREF(p,(BACnetObjectPropertyReference far*)msg->pv);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eSETREF(p,gTestLopref[i]);
             break;
         }    
       break;

     case vtse: // list of active  vt sessions (parse type) 
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eVTSE(p,(BACnetVTSession far*)msg->pv);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eVTSE(p,gTestVtse[i]);
             break;
         }
       break;      

     case uwarr: // array of uw
       switch(msg->Action)
         {
           case SETPROP:
             break;
           case ENCODEPROP: // encode from database
             p= eUWARR(p,(word far*)msg->pv,msg->Num);
             break;
           case RND_ENCODEPROP: // encode a test value 
             p= eUWARR(p,gTestUWArr[i],nTESTUWNUM);
             break;
         }    
       break;

    } // switch    
  return p;   
}


// This function is used to encode any property value and/or service parameters:
//
// 	msg.Action==	GET_PARSE_TYPE:
//		return the parsetype of the property
// 	msg.Action== ASN_1_ANY_PROP:
//		encode the property value
// 	msg.Action== RND_ASN_1_ANY_PROP:
//		encode test property value
// 	any other value for msg.Action:
//		encode ObjectId, PropertyId, ArrayIndex (opt.), 
//		Property value/test value, Priority (opt.)
//
// in:	Buffer		points to a buffer to contain the encoded property value
//		msg			PVMessage, (see struct declaration)
// returns: 0 if ok, else !=0
int EncodeFromDatabase(char far* Buffer, PVMessage far* msg)
{ word object_type; dword object_inst;
  octet far* p; // end of encoded data
  octet far* s; // start of encoded data
  generic_object far* Obj= (generic_object far*) msg->Obj;
  
  p= (octet far*) Buffer;
  msg->BufferLen= 0;
  
  switch (msg->Action)
    {
      case GET_PARSE_TYPE: // return parse type
        break;
    
      case ASN_1_ANY_PROP: // encode Property Value only
        msg->Action= ENCODEPROP;      // new action
        p= eASN_1_AnyType(p,msg);     // encode any property value
        msg->Action= ASN_1_ANY_PROP;  // restore action 
        break;

      case RND_ASN_1_ANY_PROP: // encode test Property Value only
         msg->Action= RND_ENCODEPROP;      // new action
         p= eASN_1_AnyType(p,msg);         // encode any property value
         msg->Action= RND_ASN_1_ANY_PROP;  // restore action 
         break;
         
      default:
        // encode ObjectId ---
        if (msg->Obj==NULL) return(-1); // no object, no property
        object_type= (word)(Obj->object_id>>22);
        object_inst= Obj->object_id & 0x3FFFFF;
        s= p;
        p= eOBJECTID(s,object_type,object_inst); // application tagged
        *s= 0x08 | (*s & 0x07);                  // adjust context tag
        // encode PropId ---
        s= p; 
        p= eUINT(s,msg->PropId);     // application tagged
        *s= 0x18 | (*s & 0x07); // adjust context tag
        // encode ArrayIndex ---
        if (msg->ArrayIndex != -1)
          { s= p;
            p= eUINT(s,msg->ArrayIndex); // application tagged
            *s= 0x28 | (*s & 0x07);      // adjust context tag
          }
        // encode PropVal ---
        *(p++)= 0x3E; // open tag
        p= eASN_1_AnyType(p,msg);
        *(p++)= 0x3F; // closing tag
        // encode Priority ---
        if (msg->Priority != -1)
          { s= p;
            p= eUINT(s,msg->Priority); 
            *s= 0x48 | (*s & 0x07);
          }
        break;
         
    } // switch
  
   *p= '\0'; // end of string
   msg->BufferLen= (char far*)p - (char far*)Buffer; // Buffer length
   return(0); // well done
}                                                             
 
 
int MyRand(int range)
{	  
  srand( (unsigned)time( NULL ) );
  return rand() / (RAND_MAX / range); 
}	  


// This function is used to convert an octet value to a hex representation. 
// in:	Buffer		points to a buffer to contain hex value, eg: 0x3E
//		Flag		octet, eg: 0011 1110
void Hex(char far* Buffer, octet Flag)
{
  Buffer[0]= '0';
  Buffer[1]= 'x';
  Buffer[2]= 30 + (Flag & 0xF0);
  Buffer[3]= 30 + (Flag & 0x0F);
  Buffer[4]= '\0';
}

void HexStr(char far* Buffer, char far* OctetStr, int s, int n)
{ 
  n= s+n;
  int j= 0;
  for (int i= s; i<n; i++)
    {
      Hex(Buffer+j,OctetStr[i]);
      j+= 4;
    }
  Buffer[j]= 0;  
}


// This function is used to convert a double value into a string representation.
// in: flt     value to be converted
//     Buffer  points to a buffer to contain string representation
void CvtFltStr(double fval, char far* Buffer)
{  
  _gcvt(fval,7,Buffer);
}
double CvtStrFlt(char far* Buffer)
{ 
  return( atof(Buffer) );
}


// ===========================================================================================================
// Functions for loading type of data and pointer to buffer for encoding later, checking now, etc.
// ===========================================================================================================


extern "C" void CreatePropertyFromEPICS( PICS::generic_object * pObj, int PropId, BACnetAnyValue * pbacnetAnyValue )
{
	switch(pObj->object_type)  
    { 
      case ANALOG_INPUT:			AI_CreateProperty( (ai_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );		break;
      case ANALOG_OUTPUT:			AO_CreateProperty( (ao_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );		break;
      case ANALOG_VALUE:			AV_CreateProperty( (av_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );		break;
      case BINARY_INPUT:			BI_CreateProperty( (bi_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );		break;
      case BINARY_OUTPUT:			BO_CreateProperty( (bo_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );		break;
      case BINARY_VALUE:			BV_CreateProperty( (bv_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );		break;
      case CALENDAR:				CA_CreateProperty( (calendar_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );	break;
      case COMMAND:					CO_CreateProperty( (command_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );	break;
      case DEVICE:					DE_CreateProperty( (device_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );	break;
      case EVENT_ENROLLMENT:		EV_CreateProperty( (ee_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );		break;
      case FILE_O:					FI_CreateProperty( (file_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );	break;
      case GROUP:					GR_CreateProperty( (group_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );	break;
      case LOOP:					LO_CreateProperty( (loop_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );	break;
      case MULTI_STATE_INPUT:		MSI_CreateProperty( (mi_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );	break;
      case MULTI_STATE_OUTPUT:		MSO_CreateProperty( (mo_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );	break;
      case MULTI_STATE_VALUE:		MSV_CreateProperty( (msv_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );	break;
      case NOTIFICATIONCLASS:		NC_CreateProperty( (nc_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );		break;
      case PROGRAM:					PR_CreateProperty( (program_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );	break;
      case SCHEDULE:				SC_CreateProperty( (schedule_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );	break;
      case AVERAGING:				AVG_CreateProperty( (avg_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );	break;
      case TREND_LOG:				TR_CreateProperty( (trend_obj_type *) pObj, (BACnetPropertyIdentifier) PropId, pbacnetAnyValue );	break; // msdanner 9/04, was "TRENDLOG"
	  default:
		  TRACE("CreatePropertyFromDB called for unknown object type");
		  ASSERT(0);
    }   
}


void AI_CreateProperty( ai_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case PRESENT_VALUE:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->pv));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case DEVICE_TYPE:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->device_type));
			break;
		case STATUS_FLAGS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(4, &pObj->status_flags));
			break;
		case EVENT_STATE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->state));
			break;
		case RELIABILITY:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->reliability));
			break;
		case OUT_OF_SERVICE:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->out_of_service));
			break;
		case UPDATE_INTERVAL:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->update_interval));
			break;
		case UNITS:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->units));
			break;
		case MIN_PRES_VALUE:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->min_pres_value));
			break;
		case MAX_PRES_VALUE:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->max_pres_value));
			break;
		case RESOLUTION:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->resolution));
			break;
		case COV_INCREMENT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->cov_increment));
			break;
		case TIME_DELAY:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->time_delay));
			break;
		case NOTIFICATION_CLASS:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->notification_class));
			break;
		case HIGH_LIMIT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->high_limit));
			break;
		case LOW_LIMIT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->low_limit));
			break;
		case DEADBAND:	
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->deadband));
			break;
		case LIMIT_ENABLE:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(2, &pObj->limit_enable));
			break;
		case EVENT_ENABLE:	
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->event_enable));
			break;
		case ACKED_TRANSITIONS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->acked_transitions));
			break;
		case NOTIFY_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->notify_type));
			break;
		case EVENT_TIME_STAMPS:		
			pbacnetAnyValue->SetObject(TSTMP, CreateTimeStampArray(pObj->event_time_stamps, 3));
			break;
		default:
			TRACE("INVALID TYPE in AI_CreateProperty");
			ASSERT(0);
	}
}


void AO_CreateProperty( ao_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case PRESENT_VALUE:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->pv));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case DEVICE_TYPE:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->device_type));
			break;
		case STATUS_FLAGS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(4, &pObj->status_flags));
			break;
		case EVENT_STATE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->state));
			break;
		case RELIABILITY:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->reliability));
			break;
		case OUT_OF_SERVICE:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->out_of_service));
			break;
		case UNITS:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->units));
			break;
		case MIN_PRES_VALUE:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->min_pres_value));
			break;
		case MAX_PRES_VALUE:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->max_pres_value));
			break;
		case RESOLUTION:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->resolution));
			break;
		case PRIORITY_ARRAY:    
			pbacnetAnyValue->SetObject(paf, new BACnetPriorityArray(pObj->priority_array, nPRIO, fpaNULL));
			break;
		case RELINQUISH_DEFAULT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->relinquish_default));
			break;
		case COV_INCREMENT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->cov_increment));
			break;
		case TIME_DELAY:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->time_delay));
			break;
		case NOTIFICATION_CLASS:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->notification_class));
			break;
		case HIGH_LIMIT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->high_limit));
			break;
		case LOW_LIMIT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->low_limit));
			break;
		case DEADBAND:	
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->deadband));
			break;
		case LIMIT_ENABLE:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(2, &pObj->limit_enable));
			break;
		case EVENT_ENABLE:	
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->event_enable));
			break;
		case ACKED_TRANSITIONS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->acked_transitions));
			break;
		case NOTIFY_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->notify_type));
			break;
		case EVENT_TIME_STAMPS:		
			pbacnetAnyValue->SetObject(TSTMP, CreateTimeStampArray(pObj->event_time_stamps, 3));
			break;
		default:
			TRACE("INVALID TYPE in AO_CreateProperty");
			ASSERT(0);
	}
}


void AV_CreateProperty( av_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case PRESENT_VALUE:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->pv));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case STATUS_FLAGS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(4, &pObj->status_flags));
			break;
		case EVENT_STATE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->state));
			break;
		case RELIABILITY:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->reliability));
			break;
		case OUT_OF_SERVICE:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->out_of_service));
			break;
		case UNITS:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->units));
			break;
		case PRIORITY_ARRAY:    
			pbacnetAnyValue->SetObject(paf, new BACnetPriorityArray(pObj->priority_array, nPRIO, fpaNULL));
			break;
		case RELINQUISH_DEFAULT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->relinquish_default));
			break;
		case COV_INCREMENT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->cov_increment));
			break;
		case TIME_DELAY:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->time_delay));
			break;
		case NOTIFICATION_CLASS:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->notification_class));
			break;
		case HIGH_LIMIT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->high_limit));
			break;
		case LOW_LIMIT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->low_limit));
			break;
		case DEADBAND:	
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->deadband));
			break;
		case LIMIT_ENABLE:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(2, &pObj->limit_enable));
			break;
		case EVENT_ENABLE:	
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->event_enable));
			break;
		case ACKED_TRANSITIONS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->acked_transitions));
			break;
		case NOTIFY_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->notify_type));
			break;
		case EVENT_TIME_STAMPS:		
			pbacnetAnyValue->SetObject(TSTMP, CreateTimeStampArray(pObj->event_time_stamps, 3));
			break;
		default:
			TRACE("INVALID TYPE in AV_CreateProperty");
			ASSERT(0);
	}
}

void BI_CreateProperty( bi_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case PRESENT_VALUE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->pv));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case DEVICE_TYPE:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->device_type));
			break;
		case STATUS_FLAGS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(4, &pObj->status_flags));
			break;
		case EVENT_STATE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->state));
			break;
		case RELIABILITY:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->reliability));
			break;
		case OUT_OF_SERVICE:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->out_of_service));
			break;
		case POLARITY:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->polarity));
			break;
		case INACTIVE_TEXT:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->inactive_text));
			break;
		case ACTIVE_TEXT:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->active_text));
			break;
		case CHANGE_OF_STATE_TIME:
			pbacnetAnyValue->SetObject(dt, new ::BACnetDateTime(pObj->cos_time.date.year, pObj->cos_time.date.month, 
													pObj->cos_time.date.day_of_month, pObj->cos_time.time.hour, 
													pObj->cos_time.time.minute, pObj->cos_time.time.second, 0));
			break;
		case CHANGE_OF_STATE_COUNT:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->cos_count));
			break;
		case TIME_OF_STATE_COUNT_RESET:
			pbacnetAnyValue->SetObject(dt, new ::BACnetDateTime(pObj->time_of_state_count_reset.date.year, pObj->time_of_state_count_reset.date.month, 
													pObj->time_of_state_count_reset.date.day_of_month, pObj->time_of_state_count_reset.time.hour, 
													pObj->time_of_state_count_reset.time.minute, pObj->time_of_state_count_reset.time.second, 0));
			break;
		case ELAPSED_ACTIVE_TIME:
			pbacnetAnyValue->SetObject(ud, new BACnetUnsigned((unsigned long) pObj->elapsed_active_time));
			break;
		case TIME_OF_ACTIVE_TIME_RESET:
			pbacnetAnyValue->SetObject(dt, new ::BACnetDateTime(pObj->time_of_active_time_reset.date.year, pObj->time_of_active_time_reset.date.month, 
													pObj->time_of_active_time_reset.date.day_of_month, pObj->time_of_active_time_reset.time.hour, 
													pObj->time_of_active_time_reset.time.minute, pObj->time_of_active_time_reset.time.second, 0));
			break;
		case TIME_DELAY:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->time_delay));
			break;
		case ALARM_VALUE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->alarm_value));
			break;
		case NOTIFICATION_CLASS:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->notification_class));
			break;
		case EVENT_ENABLE:	
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->event_enable));
			break;
		case ACKED_TRANSITIONS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->acked_transitions));
			break;
		case NOTIFY_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->notify_type));
			break;
		case EVENT_TIME_STAMPS:		
			pbacnetAnyValue->SetObject(TSTMP, CreateTimeStampArray(pObj->event_time_stamps, 3));
			break;
		default:
			TRACE("INVALID TYPE in BI_CreateProperty");
			ASSERT(0);
	}
}

void BO_CreateProperty( bo_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case PRESENT_VALUE:
			pbacnetAnyValue->SetObject(et, new BACnetBinaryPriV((int) pObj->pv));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case DEVICE_TYPE:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->device_type));
			break;
		case STATUS_FLAGS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(4, &pObj->status_flags));
			break;
		case EVENT_STATE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->state));
			break;
		case RELIABILITY:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->reliability));
			break;
		case OUT_OF_SERVICE:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->out_of_service));
			break;
		case POLARITY:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->polarity));
			break;
		case INACTIVE_TEXT:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->inactive_text));
			break;
		case ACTIVE_TEXT:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->active_text));
			break;
		case CHANGE_OF_STATE_TIME:             
			pbacnetAnyValue->SetObject(dt, new ::BACnetDateTime(pObj->cos_time.date.year, pObj->cos_time.date.month, 
													pObj->cos_time.date.day_of_month, pObj->cos_time.time.hour, 
													pObj->cos_time.time.minute, pObj->cos_time.time.second, 0));
			break;
		case CHANGE_OF_STATE_COUNT:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->cos_count));
			break;
		case TIME_OF_STATE_COUNT_RESET:
			pbacnetAnyValue->SetObject(dt, new ::BACnetDateTime(pObj->time_of_state_count_reset.date.year, pObj->time_of_state_count_reset.date.month, 
													pObj->time_of_state_count_reset.date.day_of_month, pObj->time_of_state_count_reset.time.hour, 
													pObj->time_of_state_count_reset.time.minute, pObj->time_of_state_count_reset.time.second, 0));
			break;
		case ELAPSED_ACTIVE_TIME:
			pbacnetAnyValue->SetObject(ud, new BACnetUnsigned((unsigned long) pObj->elapsed_active_time));
			break;
		case TIME_OF_ACTIVE_TIME_RESET:
			pbacnetAnyValue->SetObject(dt, new ::BACnetDateTime(pObj->time_of_active_time_reset.date.year, pObj->time_of_active_time_reset.date.month, 
													pObj->time_of_active_time_reset.date.day_of_month, pObj->time_of_active_time_reset.time.hour, 
													pObj->time_of_active_time_reset.time.minute, pObj->time_of_active_time_reset.time.second, 0));
			break;
		case MINIMUM_OFF_TIME:
			pbacnetAnyValue->SetObject(ud, new BACnetUnsigned((unsigned long) pObj->min_off_time));
			break;
		case MINIMUM_ON_TIME:
			pbacnetAnyValue->SetObject(ud, new BACnetUnsigned((unsigned long) pObj->min_on_time));
			break;
		case PRIORITY_ARRAY:    
			pbacnetAnyValue->SetObject(pab, new BACnetPriorityArray((int *) pObj->priority_array, nPRIO, bpaNULL));
			break;
		case RELINQUISH_DEFAULT:
			pbacnetAnyValue->SetObject(et, new BACnetBinaryPriV((int) pObj->relinquish_default));
			break;
		case TIME_DELAY:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->time_delay));
			break;
		case NOTIFICATION_CLASS:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->notification_class));
			break;
		case FEEDBACK_VALUE:
			pbacnetAnyValue->SetObject(et, new BACnetBinaryPriV((int) pObj->feedback_value));
			break;
		case EVENT_ENABLE:	
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->event_enable));
			break;
		case ACKED_TRANSITIONS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->acked_transitions));
			break;
		case NOTIFY_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->notify_type));
			break;
		case EVENT_TIME_STAMPS:		
			pbacnetAnyValue->SetObject(TSTMP, CreateTimeStampArray(pObj->event_time_stamps, 3));
			break;
		default:
			TRACE("INVALID TYPE in BO_CreateProperty");
			ASSERT(0);
	}
}

void BV_CreateProperty( bv_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case PRESENT_VALUE:
			pbacnetAnyValue->SetObject(et, new BACnetBinaryPriV((int) pObj->pv));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case STATUS_FLAGS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(4, &pObj->status_flags));
			break;
		case EVENT_STATE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->state));
			break;
		case RELIABILITY:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->reliability));
			break;
		case OUT_OF_SERVICE:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->out_of_service));
			break;
		case INACTIVE_TEXT:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->inactive_text));
			break;
		case ACTIVE_TEXT:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->active_text));
			break;
		case CHANGE_OF_STATE_TIME:             
			pbacnetAnyValue->SetObject(dt, new ::BACnetDateTime(pObj->cos_time.date.year, pObj->cos_time.date.month, 
													pObj->cos_time.date.day_of_month, pObj->cos_time.time.hour, 
													pObj->cos_time.time.minute, pObj->cos_time.time.second, 0));
			break;
		case CHANGE_OF_STATE_COUNT:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->cos_count));
			break;
		case TIME_OF_STATE_COUNT_RESET:
			pbacnetAnyValue->SetObject(dt, new ::BACnetDateTime(pObj->time_of_state_count_reset.date.year, pObj->time_of_state_count_reset.date.month, 
													pObj->time_of_state_count_reset.date.day_of_month, pObj->time_of_state_count_reset.time.hour, 
													pObj->time_of_state_count_reset.time.minute, pObj->time_of_state_count_reset.time.second, 0));
			break;
		case ELAPSED_ACTIVE_TIME:
			pbacnetAnyValue->SetObject(ud, new BACnetUnsigned((unsigned long) pObj->elapsed_active_time));
			break;
		case TIME_OF_ACTIVE_TIME_RESET:
			pbacnetAnyValue->SetObject(dt, new ::BACnetDateTime(pObj->time_of_active_time_reset.date.year, pObj->time_of_active_time_reset.date.month, 
													pObj->time_of_active_time_reset.date.day_of_month, pObj->time_of_active_time_reset.time.hour, 
													pObj->time_of_active_time_reset.time.minute, pObj->time_of_active_time_reset.time.second, 0));
			break;
		case MINIMUM_OFF_TIME:
			pbacnetAnyValue->SetObject(ud, new BACnetUnsigned((unsigned long) pObj->min_off_time));
			break;
		case MINIMUM_ON_TIME:
			pbacnetAnyValue->SetObject(ud, new BACnetUnsigned((unsigned long) pObj->min_on_time));
			break;
		case PRIORITY_ARRAY:    
			pbacnetAnyValue->SetObject(pab, new BACnetPriorityArray((int *) pObj->priority_array, nPRIO, bpaNULL));
			break;
		case RELINQUISH_DEFAULT:
			pbacnetAnyValue->SetObject(et, new BACnetBinaryPriV((int) pObj->relinquish_default));
			break;
		case TIME_DELAY:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->time_delay));
			break;
		case NOTIFICATION_CLASS:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->notification_class));
			break;
		case ALARM_VALUE:
			pbacnetAnyValue->SetObject(et, new BACnetBinaryPriV((int) pObj->alarm_value));
			break;
		case EVENT_ENABLE:	
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->event_enable));
			break;
		case ACKED_TRANSITIONS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->acked_transitions));
			break;
		case NOTIFY_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->notify_type));
			break;
		case EVENT_TIME_STAMPS:		
			pbacnetAnyValue->SetObject(TSTMP, CreateTimeStampArray(pObj->event_time_stamps, 3));
			break;
		default:
			TRACE("INVALID TYPE in BV_CreateProperty");
			ASSERT(0);
	}
}


void CA_CreateProperty( calendar_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case PRESENT_VALUE:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->pv));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case DATE_LIST:
			pbacnetAnyValue->SetObject(calist, CreateCalendarList(pObj->date_list));
			break;
		default:
			TRACE("INVALID TYPE in CA_CreateProperty");
			ASSERT(0);
	}
}


void CO_CreateProperty( command_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case PRESENT_VALUE:
			pbacnetAnyValue->SetObject(ud, new BACnetUnsigned((unsigned long) pObj->pv));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case IN_PROCESS:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->in_process));
			break;
		case ALL_WRITES_SUCCESSFUL:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->all_writes_successful));
			break;
		case ACTION:
			TRACE0("ACTION (act) type creation not implemented yet");
			ASSERT(0);
	        //msg->pv= Obj->action;
			//msg->pt= act;  
			//msg->Num= Obj->num_actions;
			break;
		case ACTION_TEXT:
			pbacnetAnyValue->SetObject(actext, new BACnetTextArray(pObj->action_text, 32));
			break;
		default:
			TRACE("INVALID TYPE in CO_CreateProperty");
			ASSERT(0);
	}
}

void DE_CreateProperty( device_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case SYSTEM_STATUS:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->system_status));
			break;
		case VENDOR_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->vendor_name));
			break;
		case VENDOR_IDENTIFIER:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->vendor_id));
			break;
		case MODEL_NAME:
			pbacnetAnyValue->SetObject(s32, new BACnetCharacterString(pObj->model_name));
			break;
		case FIRMWARE_REVISION:
			pbacnetAnyValue->SetObject(s10, new BACnetCharacterString(pObj->firmware_rev));
			break;
		case APPLICATION_SOFTWARE_VERSION:
			pbacnetAnyValue->SetObject(s32, new BACnetCharacterString(pObj->application_software_ver));
			break;
		case LOCATION:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->location));
			break;
		case PROTOCOL_VERSION:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->protocol_ver));
			break;
		case PROTOCOL_CONFORMANCE_CLASS:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->protocol_conf_class));
			break;
		case PROTOCOL_SERVICES_SUPPORTED:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(35, pObj->protocol_services_supported));
			break;
		case PROTOCOL_OBJECT_TYPES_SUPPORTED:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(18, pObj->object_types_supported));
			break;
		case OBJECT_LIST:
			pbacnetAnyValue->SetObject(lobj, CreateObjectList(pObj->object_list));
			break;
		case MAX_APDU_LENGTH_ACCEPTED:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->max_apdu_length_accepted));
			break;
		case SEGMENTATION_SUPPORTED:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->segmentation_supported));
			break;
		case VT_CLASSES_SUPPORTED:
			TRACE0("VT_CLASSES_SUPPORTED (vtcl) type creation not implemented yet");
			ASSERT(0);
			//msg->pv= Obj->vt_classes_supported;
			//msg->pt= vtcl;
			break;
		case ACTIVE_VT_SESSIONS:
			TRACE0("ACTIVE_VT_SESSIONS (vtse) type creation not implemented yet");
			ASSERT(0);
			//msg->pv= Obj->active_vt_sessions;
			//msg->pt= vtse;
			break;
		case LOCAL_TIME:      
			pbacnetAnyValue->SetObject(ptTime, new ::BACnetTime());
			break;
		case LOCAL_DATE:      
			pbacnetAnyValue->SetObject(ptDate, new ::BACnetDate());
			break;
		case UTC_OFFSET:              
			pbacnetAnyValue->SetObject(ssint, new BACnetInteger(pObj->utc_offset));
			break;
		case DAYLIGHT_SAVINGS_STATUS:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->day_savings_status));
			break;
		case APDU_SEGMENT_TIMEOUT:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->apdu_segment_timeout));
			break;
		case APDU_TIMEOUT:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->apdu_timeout));
			break;
		case NUMBER_OF_APDU_RETRIES:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->number_apdu_retries));
			break;
		case LIST_OF_SESSION_KEYS:
			TRACE0("LIST_OF_SESSION_KEYS (skeys) type creation not implemented yet");
			ASSERT(0);
			//msg->pv= Obj->list_session_keys;
			//msg->pt= skeys;
			break;
		case TIME_SYNCHRONIZATION_RECIPIENTS:
			TRACE0("TIME_SYNCHRONIZATION_RECIPIENTS (tsrecip) type creation not implemented yet");
			ASSERT(0);
			//msg->pv= Obj->time_synch_recipients;
			//msg->pt= tsrecip;
			break;
		case MAX_MASTER:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->max_master));
			break;
		case MAX_INFO_FRAMES:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->max_info_frames));
			break;
		case DEVICE_ADDRESS_BINDING:
			pbacnetAnyValue->SetObject(dabind, CreateAddressBindingList(pObj->device_add_binding));
	        break;
		default:
			TRACE("INVALID TYPE in DE_CreateProperty");
			ASSERT(0);
	}
}

void EV_CreateProperty( ee_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case EVENT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->parameter_list.event_type));
			break;
		case NOTIFY_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->notify_type));
			break;
		case EVENT_PARAMETERS:
			TRACE0("EVENT_PARAMETERS (evparm) type creation not implemented yet");
			ASSERT(0);
			//msg->pv= &Obj->parameter_list;
			//msg->pt= evparm;
			break;
		case OBJECT_PROPERTY_REFERENCE:
			TRACE0("OBJECT_PROPERTY_REFERENCE (lopref) type creation not implemented yet");
			ASSERT(0);
			//msg->pv= &Obj->obj_prop_ref;
			//msg->pt= lopref;
			break;
		case EVENT_STATE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->state));
			break;
		case EVENT_ENABLE:	
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->event_enable));
			break;
		case ACKED_TRANSITIONS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->acked_transitions));
			break;
		case NOTIFICATION_CLASS:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->notification_class));
			break;
		case RECIPIENT:     
			TRACE0("RECIPIENT (recip) type creation not implemented yet");
			ASSERT(0);
			//msg->pv= &Obj->recipient;
			//msg->pt= recip;
			break;
		case PROCESS_IDENTIFIER:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->process_id));
			break;
		case PRIORITY:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->priority));
			break;
		case ISSUE_CONFIRMED_NOTIFICATIONS:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->issue_conf_notifications));
			break;
		case EVENT_TIME_STAMPS:		
			pbacnetAnyValue->SetObject(TSTMP, CreateTimeStampArray(pObj->event_time_stamps, 3));
			break;
		default:
			TRACE("INVALID TYPE in EE_CreateProperty");
			ASSERT(0);
	}
}

void FI_CreateProperty( file_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case FILE_TYPE:
			pbacnetAnyValue->SetObject(s32, new BACnetCharacterString(pObj->file_type));
			break;
		case FILE_SIZE:
			pbacnetAnyValue->SetObject(ud, new BACnetUnsigned((unsigned long) pObj->file_size));
			break;
		case MODIFICATION_DATE: 
			pbacnetAnyValue->SetObject(dt, new ::BACnetDateTime(pObj->mod_date.date.year, pObj->mod_date.date.month, 
													pObj->mod_date.date.day_of_month, pObj->mod_date.time.hour, 
													pObj->mod_date.time.minute, pObj->mod_date.time.second, 0));
			break;
		case ARCHIVE:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->archive));
			break;
		case READ_ONLY:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->read_only));
			break;
		case FILE_ACCESS_METHOD: 
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->access_method));
			break;
		default:
			TRACE("INVALID TYPE in FI_CreateProperty");
			ASSERT(0);
	}
}

void GR_CreateProperty( group_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case LIST_OF_GROUP_MEMBERS: 
			TRACE0("LIST_OF_GROUP_MEMBERS (raslist) type creation not implemented yet");
			ASSERT(0);
			//msg->pv= Obj->list_of_group_members;
			//msg->pt= raslist;
			break;
		default:
			TRACE("INVALID TYPE in FI_CreateProperty");
			ASSERT(0);
	}
}


void LO_CreateProperty( loop_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case PRESENT_VALUE:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->pv));
			break;
		case STATUS_FLAGS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(4, &pObj->status_flags));
			break;
		case EVENT_STATE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->state));
			break;
		case RELIABILITY:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->reliability));
			break;
		case OUT_OF_SERVICE:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->out_of_service));
			break;
		case UPDATE_INTERVAL:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->update_interval));
			break;
		case OUTPUT_UNITS: 
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->output_units));
			break;
		case MANIPULATED_VARIABLE_REFERENCE:
			TRACE0("MANIPULATED_VARIABLE_REFERENCE (lopref) type creation not implemented yet");
			ASSERT(0);
			//msg->pv= &Obj->man_var_ref;
			//msg->pt= lopref;
		    break;
		case CONTROLLED_VARIABLE_REFERENCE:
			TRACE0("CONTROLLED_VARIABLE_REFERENCE (lopref) type creation not implemented yet");
			ASSERT(0);
			//msg->pv= &Obj->cont_var_ref;
			//msg->pt= lopref;
			break;
		case CONTROLLED_VARIABLE_VALUE:  
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->cont_var_value));
			break;
		case CONTROLLED_VARIABLE_UNITS:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->cont_var_units));
			break;
		case SETPOINT_REFERENCE:
			TRACE0("SETPOINT_REFERENCE (setref) type creation not implemented yet");
			ASSERT(0);
			//msg->pv= Obj->setpoint_ref;
			//msg->pt= setref;
			break;
		case SETPOINT:     
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->setpoint));
			break;
		case ACTION:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->action));
			break;
		case PROPORTIONAL_CONSTANT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->proportional_const));
			break;
		case PROPORTIONAL_CONSTANT_UNITS:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->proportional_const_units));
			break;
		case INTEGRAL_CONSTANT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->integral_const));
			break;
		case INTEGRAL_CONSTANT_UNITS:  
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->integral_const_units));
			break;
		case DERIVATIVE_CONSTANT:  
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->derivative_const));
			break;
		case DERIVATIVE_CONSTANT_UNITS:  
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->derivative_const_units));
			break;
		case BIAS:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->bias));
			break;
		case MAXIMUM_OUTPUT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->max_output));
			break;
		case MINIMUM_OUTPUT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->min_output));
			break;
		case PRIORITY_FOR_WRITING:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->priority_for_writing));
			break;
		case COV_INCREMENT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->cov_increment));
			break;
		case TIME_DELAY:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->time_delay));
			break;
		case NOTIFICATION_CLASS:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->notification_class));
			break;
		case ERROR_LIMIT:
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->error_limit));
			break;
		case EVENT_ENABLE:	
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->event_enable));
			break;
		case ACKED_TRANSITIONS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->acked_transitions));
			break;
		case NOTIFY_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->notify_type));
			break;
		case EVENT_TIME_STAMPS:		
			pbacnetAnyValue->SetObject(TSTMP, CreateTimeStampArray(pObj->event_time_stamps, 3));
			break;
		default:
			TRACE("INVALID TYPE in LO_CreateProperty");
			ASSERT(0);
	}
}


void MSI_CreateProperty( mi_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case PRESENT_VALUE:
			pbacnetAnyValue->SetObject(ud, new BACnetUnsigned((unsigned long) pObj->pv));
			break;
		case DEVICE_TYPE:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->device_type));
			break;
		case STATUS_FLAGS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(4, &pObj->status_flags));
			break;
		case EVENT_STATE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->state));
			break;
		case RELIABILITY:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->reliability));
			break;
		case OUT_OF_SERVICE:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->out_of_service));
			break;
		case NUMBER_OF_STATES:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->num_of_states));
			break;
		case STATE_TEXT:
			pbacnetAnyValue->SetObject(statext, new BACnetTextArray(pObj->state_text, 32));
			break;
		case TIME_DELAY:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->time_delay));
			break;
		case NOTIFICATION_CLASS:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->notification_class));
			break;
		case ALARM_VALUES:
			pbacnetAnyValue->SetObject(stavals, new BACnetUnsignedArray(pObj->alarm_values));
			break;
		case FAULT_VALUES:
			pbacnetAnyValue->SetObject(stavals, new BACnetUnsignedArray(pObj->fault_values));
			break;
		case EVENT_ENABLE:	
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->event_enable));
			break;
		case ACKED_TRANSITIONS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->acked_transitions));
			break;
		case NOTIFY_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->notify_type));
			break;
		case EVENT_TIME_STAMPS:		
			pbacnetAnyValue->SetObject(TSTMP, CreateTimeStampArray(pObj->event_time_stamps, 3));
			break;
		default:
			TRACE("INVALID TYPE in MSI_CreateProperty");
			ASSERT(0);
	}
}


void MSO_CreateProperty( mo_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case PRESENT_VALUE:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned((unsigned long) pObj->pv));
			break;
		case DEVICE_TYPE:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->device_type));
			break;
		case STATUS_FLAGS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(4, &pObj->status_flags));
			break;
		case EVENT_STATE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->state));
			break;
		case RELIABILITY:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->reliability));
			break;
		case OUT_OF_SERVICE:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->out_of_service));
			break;
		case NUMBER_OF_STATES:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->num_of_states));
			break;
		case STATE_TEXT:
			pbacnetAnyValue->SetObject(statext, new BACnetTextArray(pObj->state_text, 32));
			break;
		case PRIORITY_ARRAY:    
			pbacnetAnyValue->SetObject(pau, new BACnetPriorityArray(pObj->priority_array, nPRIO, upaNULL));
			break;
		case RELINQUISH_DEFAULT:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->relinquish_default));
			break;
		case TIME_DELAY:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->time_delay));
			break;
		case NOTIFICATION_CLASS:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->notification_class));
			break;
		case FEEDBACK_VALUE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->feedback_value));
			break;
		case EVENT_ENABLE:	
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->event_enable));
			break;
		case ACKED_TRANSITIONS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->acked_transitions));
			break;
		case NOTIFY_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->notify_type));
			break;
		case EVENT_TIME_STAMPS:		
			pbacnetAnyValue->SetObject(TSTMP, CreateTimeStampArray(pObj->event_time_stamps, 3));
			break;
		default:
			TRACE("INVALID TYPE in MSO_CreateProperty");
			ASSERT(0);
	}
}


void MSV_CreateProperty( msv_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case PRESENT_VALUE:
			pbacnetAnyValue->SetObject(ud, new BACnetUnsigned((unsigned long) pObj->present_value));
			break;
		case STATUS_FLAGS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(4, &pObj->status_flags));
			break;
		case EVENT_STATE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->event_state));
			break;
		case RELIABILITY:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->reliability));
			break;
		case OUT_OF_SERVICE:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->out_of_service));
			break;
		case NUMBER_OF_STATES:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->number_of_states));
			break;
		case STATE_TEXT:
			pbacnetAnyValue->SetObject(statext, new BACnetTextArray(pObj->state_text));
			break;
		case PRIORITY_ARRAY:    
			pbacnetAnyValue->SetObject(pau, new BACnetPriorityArray(pObj->priority_array, nPRIO, upaNULL));
			break;
		case RELINQUISH_DEFAULT:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->relinquish_default));
			break;
		case TIME_DELAY:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->time_delay));
			break;
		case NOTIFICATION_CLASS:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->notification_class));
			break;
		case ALARM_VALUE:					// array of 10 words...  was 'uw' type (I think by mistake)
			pbacnetAnyValue->SetObject(stavals, new BACnetUnsignedArray(pObj->alarm_values, 10));
			break;
		case FAULT_VALUES:					// array of 10 words   was 'uw' type (I think by mistake)
			pbacnetAnyValue->SetObject(stavals, new BACnetUnsignedArray(pObj->fault_values, 10));
			break;         	       
		case EVENT_ENABLE:	
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->event_enable));
			break;
		case ACKED_TRANSITIONS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->acked_transitions));
			break;
		case NOTIFY_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->notify_type));
			break;
		case EVENT_TIME_STAMPS:		
			pbacnetAnyValue->SetObject(TSTMP, CreateTimeStampArray(pObj->event_time_stamps, 3));
			break;
		default:
			TRACE("INVALID TYPE in MSV_CreateProperty");
			ASSERT(0);
	}
}



void NC_CreateProperty( nc_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case NOTIFICATION_CLASS:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->notification_class));
			break;
		case PRIORITY:
			pbacnetAnyValue->SetObject(uwarr, new BACnetUnsignedArray(pObj->priority, 3));
			break;
		case ACK_REQUIRED:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->ack_required));
			break;
		case RECIPIENT_LIST:
			TRACE0("RECIPIENT_LIST (reciplist) type creation not implemented yet");
			ASSERT(0);
			//msg->pv= Obj->recipient_list;
			//msg->pt= reciplist;
			break;
		default:
			TRACE("INVALID TYPE in NC_CreateProperty");
			ASSERT(0);
	}
}



void PR_CreateProperty( program_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case PROGRAM_STATE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->prog_state));
			break;
		case PROGRAM_CHANGE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->prog_change));
			break;
		case REASON_FOR_HALT:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->reason_for_halt));
			break;
		case DESCRIPTION_OF_HALT:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->description_of_halt));
			break;
		case PROGRAM_LOCATION:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->prog_location));
			break;
		case INSTANCE_OF:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->instance_of));
			break;
		case STATUS_FLAGS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(4, &pObj->status_flags));
			break;
		case RELIABILITY:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->reliability));
			break;
		case OUT_OF_SERVICE:
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->out_of_service));
			break;
		default:
			TRACE("INVALID TYPE in PR_CreateProperty");
			ASSERT(0);
	}
}



void SC_CreateProperty( schedule_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case PRESENT_VALUE:
			switch(pObj->value_type)
			{
				case et:		pbacnetAnyValue->SetObject(et, new BACnetBinaryPriV(pObj->pv.bproperty_value));
							break;
				case uw:		pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->pv.uproperty_value));
							break;
				case flt:		pbacnetAnyValue->SetObject(flt, new BACnetReal(pObj->pv.fproperty_value));
							break;
				default:	TRACE0("INVALID TYPE in SC_CreateProperty for PV");  ASSERT(0);
			}
			break;
		case EFFECTIVE_PERIOD:
			pbacnetAnyValue->SetObject(dtrange, new ::BACnetDateRange(pObj->effective_period.start_date.year, pObj->effective_period.start_date.month, pObj->effective_period.start_date.day_of_month, 
																	pObj->effective_period.end_date.year, pObj->effective_period.end_date.month, pObj->effective_period.end_date.day_of_month));
			break;
		case WEEKLY_SCHEDULE:
			TRACE0("WEEKLY_SCHEDULE (wsched) type creation not implemented yet");
			ASSERT(0);
			//msg->pv= Obj->weekly_schedule;
			//msg->pt= wsched;
			break;
		case EXCEPTION_SCHEDULE:
			TRACE0("EXCEPTION_SCHEDULE (xsched) type creation not implemented yet");
			ASSERT(0);
			//msg->pv= &Obj->exception_schedule;
			//msg->pt= xsched;                    
			break;
		case LIST_OF_OBJECT_PROPERTY_REFERENCES:
			TRACE0("LIST_OF_OBJECT_PROPERTY_REFERENCES (reciplist) type creation not implemented yet");
			ASSERT(0);
			//msg->pv= Obj->list_obj_prop_ref;
			//msg->pt= lopref;
			break;
		case PRIORITY_FOR_WRITING:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->priority_for_writing));
			break;
		default:
			TRACE("INVALID TYPE in SC_CreateProperty");
			ASSERT(0);
	}
}


void AVG_CreateProperty( avg_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case MINIMUM_VALUE:		       
			pbacnetAnyValue->SetObject(uw, new BACnetReal(pObj->minimum_value));
			break;
		case MINIMUM_VALUE_TIMESTAMP:  
			pbacnetAnyValue->SetObject(dt, new ::BACnetDateTime(pObj->minimum_value_timestamp.date.year, pObj->minimum_value_timestamp.date.month, 
													pObj->minimum_value_timestamp.date.day_of_month, pObj->minimum_value_timestamp.time.hour, 
													pObj->minimum_value_timestamp.time.minute, pObj->minimum_value_timestamp.time.second));
			break;         	       
		case AVERAGE_VALUE:		       
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->average_value));
			break;
		case VARIANCE_VALUE:	       
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->variance_value));
			break;
		case MAXIMUM_VALUE:		       
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->maximum_value));
			break;
		case MAXIMUM_VALUE_TIMESTAMP:  
			pbacnetAnyValue->SetObject(dt, new ::BACnetDateTime(pObj->maximum_value_timestamp.date.year, pObj->maximum_value_timestamp.date.month, 
													pObj->maximum_value_timestamp.date.day_of_month, pObj->maximum_value_timestamp.time.hour, 
													pObj->maximum_value_timestamp.time.minute, pObj->maximum_value_timestamp.time.second));
			break;         	       
		case ATTEMPTED_SAMPLES:		   
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->attempted_samples));
			break;
		case VALID_SAMPLES:	           
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->valid_samples));
			break;
		case OBJECT_PROPERTY_REFERENCE:
			TRACE0("OBJECT_PROPERTY_REFERENCE (propref) type creation not implemented yet");
			ASSERT(0);
			//msg->pv = &Obj->obj_prop_ref;       
			//msg->pt = propref;       
			break;         	       
		case WINDOW_INTERVAL:		   
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->window_interval));
			break;
		case WINDOW_SAMPLES:		   
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->window_samples));
			break;
		default:
			TRACE("INVALID TYPE in AVG_CreateProperty");
			ASSERT(0);
	}
}



void TR_CreateProperty( trend_obj_type far * pObj, BACnetPropertyIdentifier propertyID, BACnetAnyValue * pbacnetAnyValue )
{
	ASSERT(pbacnetAnyValue != NULL);

	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
			pbacnetAnyValue->SetObject(ob_id, new ::BACnetObjectIdentifier((unsigned int) pObj->go.object_id));
			break;
		case OBJECT_NAME:
			pbacnetAnyValue->SetObject(s64, new BACnetCharacterString(pObj->go.object_name));
			break;
		case OBJECT_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->go.object_type));
			break;
		case DESCRIPTION:
			pbacnetAnyValue->SetObject(s132, new BACnetCharacterString(pObj->go.description));
			break;
		case LOG_ENABLE:  			     
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->log_enable));
			break;
		case START_TIME:  	 		     
			pbacnetAnyValue->SetObject(dt, new ::BACnetDateTime(pObj->start_time.date.year, pObj->start_time.date.month, 
													pObj->start_time.date.day_of_month, pObj->start_time.time.hour, 
													pObj->start_time.time.minute, pObj->start_time.time.second));
			break;         	       
		case STOP_TIME:  				 
			pbacnetAnyValue->SetObject(dt, new ::BACnetDateTime(pObj->stop_time.date.year, pObj->stop_time.date.month, 
													pObj->stop_time.date.day_of_month, pObj->stop_time.time.hour, 
													pObj->stop_time.time.minute, pObj->stop_time.time.second));
			break;         	       
		case LOG_DEVICE_OBJECT_PROPERTY:  
			TRACE0("LOG_DEVICE_OBJECT_PROPERTY (propref) type creation not implemented yet");
			ASSERT(0);
			//msg->pv = &Obj->log_device_object_property;       
			//msg->pt = propref;      
			break;         	       
		case LOG_INTERVAL:  			     
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->log_interval));
			break;
		case COV_RESUBSCRIPTION_INTERVAL: 
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->cov_resubscription_interval));
			break;
		case CLIENT_COV_INCREMENT:  	     
			pbacnetAnyValue->SetObject(flt, new BACnetReal((float) pObj->client_cov_increment));
			break;
		case STOP_WHEN_FULL:  		     
			pbacnetAnyValue->SetObject(ebool, new BACnetBoolean((int) pObj->stop_when_full));
			break;
		case BUFFER_SIZE:  			     
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->buffer_size));
			break;
		case LOG_BUFFER:  			     
			TRACE0("LOG_BUFFER (LOGREC) type creation not implemented yet");
			ASSERT(0);
			//msg->pv = &Obj->log_buffer;       
			//msg->pt = LOGREC;       
			break;         	       
		case RECORD_COUNT:  			     
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->record_count));
			break;
		case TOTAL_RECORD_COUNT:  	     
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->total_record_count));
			break;
		case NOTIFICATION_THRESHOLD:      
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->notification_threshold));
			break;
		case RECORDS_SINCE_NOTIFICATION:  
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->records_since_notification));
			break;
/*
					case PREVIOUS_NOTIFY_TIME:  	     
						pbacnetAnyValue->SetObject(dt, new ::BACnetDateTime(pObj->previous_notify_time.date.year, pObj->previous_notify_time.date.month, 
																pObj->previous_notify_time.date.day_of_month, pObj->previous_notify_time.time.hour, 
																pObj->previous_notify_time.time.minute, pObj->previous_notify_time.time.second));
						break;         	       
					case CURRENT_NOTIFY_TIME:  	     
						pbacnetAnyValue->SetObject(dt, new ::BACnetDateTime(pObj->current_notify_time.date.year, pObj->current_notify_time.date.month, 
																pObj->current_notify_time.date.day_of_month, pObj->current_notify_time.time.hour, 
																pObj->current_notify_time.time.minute, pObj->current_notify_time.time.second));
						break;  */
			       	       
		case EVENT_STATE:  			     
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->event_state));
			break;
		case NOTIFICATION_CLASS:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->notification_class));
			break;
		case EVENT_ENABLE:	
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->event_enable));
			break;
		case ACKED_TRANSITIONS:
			pbacnetAnyValue->SetObject(bits, new BACnetBitString(3, &pObj->acked_transitions));
			break;
		case NOTIFY_TYPE:
			pbacnetAnyValue->SetObject(et, new BACnetEnumerated((int) pObj->notify_type));
			break;
		case EVENT_TIME_STAMPS:		
			pbacnetAnyValue->SetObject(TSTMP, CreateTimeStampArray(pObj->event_time_stamps, 3));
			break;
		//Added by Zhu Zhenhua,  2004-5-11
		case LAST_NOTIFY_RECORD:
			pbacnetAnyValue->SetObject(uw, new BACnetUnsigned(pObj->notification_class));
			break;
		default:
			TRACE("INVALID TYPE in TR_CreateProperty");
			ASSERT(0);
	}
}




// Create separate function to hide the nasty internal EPICS structures until we can 
// replace them with BACnet objects

BACnetCalendarArray * CreateCalendarList( PICS::BACnetCalendarEntry * pcalentry )
{
	BACnetCalendarArray * pbacnetCalList = new BACnetCalendarArray();

	for ( ; pcalentry != NULL; pcalentry = pcalentry->next )
	{
		::BACnetCalendarEntry * pcalentryObj;

		switch(pcalentry->choice)
		{
		case 0:		pcalentryObj = new ::BACnetCalendarEntry(new ::BACnetDate(pcalentry->u.date.year, pcalentry->u.date.month, pcalentry->u.date.day_of_month));
						break;
			case 1:		pcalentryObj = new ::BACnetCalendarEntry(new ::BACnetDateRange(pcalentry->u.date_range.start_date.year, pcalentry->u.date_range.start_date.month, pcalentry->u.date_range.start_date.day_of_month,
																						pcalentry->u.date_range.end_date.year, pcalentry->u.date_range.end_date.month, pcalentry->u.date_range.end_date.day_of_month));
						break;
			case 2:		pcalentryObj = new ::BACnetCalendarEntry(new ::BACnetWeekNDay(pcalentry->u.weekNday.month, pcalentry->u.weekNday.week, pcalentry->u.weekNday.day));
						break;
		}
		pbacnetCalList->Add(pcalentryObj);
	}

	return pbacnetCalList;
}



BACnetGenericArray * CreateTimeStampArray( PICS::BACnetTimeStamp  * ptimestamp, int nMax )
{
	BACnetGenericArray * pbacnetArray = new BACnetGenericArray(nMax);

	for ( int i = 0; i < nMax; i++ )
		switch(ptimestamp[i].choice)
		{
			case 0:		pbacnetArray->Add( new ::BACnetTimeStamp(new ::BACnetTime(ptimestamp[i].u.time.hour, ptimestamp[i].u.time.minute, ptimestamp[i].u.time.second, ptimestamp[i].u.time.hundredths)));
						break;
			case 1:		pbacnetArray->Add( new ::BACnetTimeStamp(new BACnetUnsigned(ptimestamp[i].u.sequence_number)));
						break;
			case 2:		pbacnetArray->Add( new ::BACnetTimeStamp(new ::BACnetDateTime(ptimestamp[i].u.date_time.date.year, ptimestamp[i].u.date_time.date.month, ptimestamp[i].u.date_time.date.day_of_month,
										ptimestamp[i].u.date_time.time.hour, ptimestamp[i].u.date_time.time.minute, ptimestamp[i].u.date_time.time.second, ptimestamp[i].u.date_time.time.hundredths)));
						break;
		}

	return pbacnetArray;
}



BACnetObjectIDList * CreateObjectList( PICS::BACnetObjectIdentifier * pobjidlist )
{
	BACnetObjectIDList * pbacnetList = new BACnetObjectIDList();

	for ( ; pobjidlist != NULL; pobjidlist = pobjidlist->next )
		pbacnetList->Add(new ::BACnetObjectIdentifier((unsigned int) pobjidlist->object_id));

	return pbacnetList;
}



BACnetGenericArray * CreateAddressBindingList( PICS::BACnetAddressBinding * pAddrBinding )
{
	BACnetGenericArray * pbacnetArray = new BACnetGenericArray();

	for ( ; pAddrBinding != NULL; pAddrBinding = pAddrBinding->next )
		pbacnetArray->Add(new ::BACnetAddressBinding(pAddrBinding->device_object_id, pAddrBinding->device_address.network_number, 
													pAddrBinding->device_address.mac_address, pAddrBinding->device_address.address_size));

	return pbacnetArray;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 

bool AI_CheckProperty(BACnetPropertyIdentifier propertyID)
{
	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case PRESENT_VALUE:
		case DESCRIPTION:
		case DEVICE_TYPE:
		case STATUS_FLAGS:
		case EVENT_STATE:
		case RELIABILITY:
		case OUT_OF_SERVICE:
		case UPDATE_INTERVAL:
		case UNITS:
		case MIN_PRES_VALUE:
		case MAX_PRES_VALUE:
		case RESOLUTION:
		case COV_INCREMENT:
		case TIME_DELAY:
		case NOTIFICATION_CLASS:
		case HIGH_LIMIT:
		case LOW_LIMIT:
		case DEADBAND:	
		case LIMIT_ENABLE:
		case EVENT_ENABLE:	
		case ACKED_TRANSITIONS:
		case NOTIFY_TYPE:
		case EVENT_TIME_STAMPS:
		case PROFILE_NAME:
			return true;
		default:
			return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool AO_CheckProperty(BACnetPropertyIdentifier propertyID)
{
	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case PRESENT_VALUE:
		case DESCRIPTION:
		case DEVICE_TYPE:
		case STATUS_FLAGS:
		case EVENT_STATE:
		case RELIABILITY:
		case OUT_OF_SERVICE:
		case UNITS:
		case MIN_PRES_VALUE:
		case MAX_PRES_VALUE:
		case RESOLUTION:
		case PRIORITY_ARRAY:    
		case RELINQUISH_DEFAULT:
		case COV_INCREMENT:
		case TIME_DELAY:
		case NOTIFICATION_CLASS:
		case HIGH_LIMIT:
		case LOW_LIMIT:
		case DEADBAND:	
		case LIMIT_ENABLE:
		case EVENT_ENABLE:	
		case ACKED_TRANSITIONS:
		case NOTIFY_TYPE:
		case EVENT_TIME_STAMPS:
		case PROFILE_NAME:
			return true;
		default:
			return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool AV_CheckProperty(BACnetPropertyIdentifier propertyID)
{
	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case PRESENT_VALUE:
		case DESCRIPTION:
		case STATUS_FLAGS:
		case EVENT_STATE:
		case RELIABILITY:
		case OUT_OF_SERVICE:
		case UNITS:
		case PRIORITY_ARRAY:    
		case RELINQUISH_DEFAULT:
		case COV_INCREMENT:
		case TIME_DELAY:
		case NOTIFICATION_CLASS:
		case HIGH_LIMIT:
		case LOW_LIMIT:
		case DEADBAND:	
		case LIMIT_ENABLE:
		case EVENT_ENABLE:	
		case ACKED_TRANSITIONS:
		case NOTIFY_TYPE:
		case EVENT_TIME_STAMPS:
		case PROFILE_NAME:
			return true;
		default:
			return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool BI_CheckProperty(BACnetPropertyIdentifier propertyID)
{
	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case PRESENT_VALUE:
		case DESCRIPTION:
		case DEVICE_TYPE:
		case STATUS_FLAGS:
		case EVENT_STATE:
		case RELIABILITY:
		case OUT_OF_SERVICE:
		case POLARITY:
		case INACTIVE_TEXT:
		case ACTIVE_TEXT:
		case CHANGE_OF_STATE_TIME:
		case CHANGE_OF_STATE_COUNT:
		case TIME_OF_STATE_COUNT_RESET:
		case ELAPSED_ACTIVE_TIME:
		case TIME_OF_ACTIVE_TIME_RESET:
		case TIME_DELAY:
		case ALARM_VALUE:
		case NOTIFICATION_CLASS:
		case EVENT_ENABLE:	
		case ACKED_TRANSITIONS:
		case NOTIFY_TYPE:
		case EVENT_TIME_STAMPS:
		case PROFILE_NAME:	
			return true;
		default:
			return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool BO_CheckProperty(BACnetPropertyIdentifier propertyID)
{
	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case PRESENT_VALUE:
		case DESCRIPTION:
		case DEVICE_TYPE:
		case STATUS_FLAGS:
		case EVENT_STATE:
		case RELIABILITY:
		case OUT_OF_SERVICE:
		case POLARITY:
		case INACTIVE_TEXT:
		case ACTIVE_TEXT:
		case CHANGE_OF_STATE_TIME:             
		case CHANGE_OF_STATE_COUNT:
		case TIME_OF_STATE_COUNT_RESET:
		case ELAPSED_ACTIVE_TIME:
		case TIME_OF_ACTIVE_TIME_RESET:
		case MINIMUM_OFF_TIME:
		case MINIMUM_ON_TIME:
		case PRIORITY_ARRAY:    
		case RELINQUISH_DEFAULT:
		case TIME_DELAY:
		case NOTIFICATION_CLASS:
		case FEEDBACK_VALUE:
		case EVENT_ENABLE:	
		case ACKED_TRANSITIONS:
		case NOTIFY_TYPE:
		case EVENT_TIME_STAMPS:
		case PROFILE_NAME:	
				return true;
		default:
				return false;
	}	
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool BV_CheckProperty(BACnetPropertyIdentifier propertyID)
{
	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case PRESENT_VALUE:
		case DESCRIPTION:
		case STATUS_FLAGS:
		case EVENT_STATE:
		case RELIABILITY:
		case OUT_OF_SERVICE:
		case INACTIVE_TEXT:
		case ACTIVE_TEXT:
		case CHANGE_OF_STATE_TIME:             
		case CHANGE_OF_STATE_COUNT:
		case TIME_OF_STATE_COUNT_RESET:
		case ELAPSED_ACTIVE_TIME:
		case TIME_OF_ACTIVE_TIME_RESET:
		case MINIMUM_OFF_TIME:
		case MINIMUM_ON_TIME:
		case PRIORITY_ARRAY:    
		case RELINQUISH_DEFAULT:
		case TIME_DELAY:
		case NOTIFICATION_CLASS:
		case ALARM_VALUE:
		case EVENT_ENABLE:	
		case ACKED_TRANSITIONS:
		case NOTIFY_TYPE:
		case EVENT_TIME_STAMPS:
		case PROFILE_NAME:			
			return true;
		default:
			return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool CA_CheckProperty(BACnetPropertyIdentifier propertyID)
{
	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case PRESENT_VALUE:
		case DESCRIPTION:
		case DATE_LIST:
		case PROFILE_NAME:
				return true;
		default:
				return false;
	}

}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool CO_CheckProperty(BACnetPropertyIdentifier propertyID)
{
	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case PRESENT_VALUE:
		case DESCRIPTION:
		case IN_PROCESS:
		case ALL_WRITES_SUCCESSFUL:
		case ACTION:
		case ACTION_TEXT:
		case PROFILE_NAME:
				return true;
		default:
               return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool DE_CheckProperty(BACnetPropertyIdentifier propertyID)
{
		switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case DESCRIPTION:
		case SYSTEM_STATUS:
		case VENDOR_NAME:
		case VENDOR_IDENTIFIER:
		case MODEL_NAME:
		case FIRMWARE_REVISION:
		case APPLICATION_SOFTWARE_VERSION:
		case LOCATION:
		case PROTOCOL_VERSION:
		case PROTOCOL_REVISION:
		case PROTOCOL_SERVICES_SUPPORTED:
		case PROTOCOL_OBJECT_TYPES_SUPPORTED:
		case OBJECT_LIST:
		case MAX_APDU_LENGTH_ACCEPTED:
		case SEGMENTATION_SUPPORTED:
		case VT_CLASSES_SUPPORTED:
		case ACTIVE_VT_SESSIONS:
		case LOCAL_TIME:      
		case LOCAL_DATE:      
		case UTC_OFFSET:              
		case DAYLIGHT_SAVINGS_STATUS:
		case APDU_SEGMENT_TIMEOUT:
		case APDU_TIMEOUT:
		case NUMBER_OF_APDU_RETRIES:
		case LIST_OF_SESSION_KEYS:
		case TIME_SYNCHRONIZATION_RECIPIENTS:
		case MAX_MASTER:
		case MAX_INFO_FRAMES:
		case DEVICE_ADDRESS_BINDING:
		case DATABASE_REVISION:
		case CONFIGURATION_FILES:
		case LAST_RESTORE_TIME:
		case BACKUP_FAILURE_TIMEOUT:
		case ACTIVE_COV_SUBSCRIPTION:
		case PROFILE_NAME:
		case MAX_SEGMENTS_ACCEPTED: // Zhu Zhenhua  2003-11-21 
				return true;
		default:
				return false;
		}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool EV_CheckProperty(unsigned int propertyID)
{
		switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case DESCRIPTION:
		case EVENT_TYPE:
		case NOTIFY_TYPE:
		case EVENT_PARAMETERS:
		case OBJECT_PROPERTY_REFERENCE:
		case EVENT_STATE:
		case EVENT_ENABLE:	
		case ACKED_TRANSITIONS:
		case NOTIFICATION_CLASS:
		case RECIPIENT:     
		case PROCESS_IDENTIFIER:
		case PRIORITY:
		case ISSUE_CONFIRMED_NOTIFICATIONS:	
		case EVENT_TIME_STAMPS:
		case PROFILE_NAME:
			return true;
		default:
            return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool FI_CheckProperty(BACnetPropertyIdentifier propertyID)
{
		switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case DESCRIPTION:
		case FILE_TYPE:
		case FILE_SIZE:
		case MODIFICATION_DATE: 
		case ARCHIVE:
		case READ_ONLY:
		case FILE_ACCESS_METHOD: 
		case RECORD_COUNT:
		case PROFILE_NAME:
			return true;
		default:
            return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool GR_CheckProperty(BACnetPropertyIdentifier propertyID)
{
		switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case DESCRIPTION:
		case LIST_OF_GROUP_MEMBERS: 
		case PRESENT_VALUE:
		case PROFILE_NAME:
			return true;
		default:
			return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool LO_CheckProperty(BACnetPropertyIdentifier propertyID)
{
		switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case DESCRIPTION:
		case PRESENT_VALUE:
		case STATUS_FLAGS:
		case EVENT_STATE:
		case RELIABILITY:
		case OUT_OF_SERVICE:
		case UPDATE_INTERVAL:
		case OUTPUT_UNITS: 
		case MANIPULATED_VARIABLE_REFERENCE:
		case CONTROLLED_VARIABLE_REFERENCE:
		case CONTROLLED_VARIABLE_VALUE:  
		case CONTROLLED_VARIABLE_UNITS:
		case SETPOINT_REFERENCE:
		case SETPOINT:     
		case ACTION:
		case PROPORTIONAL_CONSTANT:
		case PROPORTIONAL_CONSTANT_UNITS:
		case INTEGRAL_CONSTANT:
		case INTEGRAL_CONSTANT_UNITS:  
		case DERIVATIVE_CONSTANT:  
		case DERIVATIVE_CONSTANT_UNITS:  
		case BIAS:
		case MAXIMUM_OUTPUT:
		case MINIMUM_OUTPUT:
		case PRIORITY_FOR_WRITING:
		case COV_INCREMENT:
		case TIME_DELAY:
		case NOTIFICATION_CLASS:
		case ERROR_LIMIT:
		case EVENT_ENABLE:	
		case ACKED_TRANSITIONS:
		case NOTIFY_TYPE:
		case EVENT_TIME_STAMPS:
		case PROFILE_NAME:			
				return true;
		default:
				return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool MSI_CheckProperty(BACnetPropertyIdentifier propertyID)
{
		switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case DESCRIPTION:
		case PRESENT_VALUE:
		case DEVICE_TYPE:
		case STATUS_FLAGS:
		case EVENT_STATE:
		case RELIABILITY:
		case OUT_OF_SERVICE:
		case NUMBER_OF_STATES:
		case STATE_TEXT:
		case TIME_DELAY:
		case NOTIFICATION_CLASS:
		case ALARM_VALUES:
		case FAULT_VALUES:
		case EVENT_ENABLE:	
		case ACKED_TRANSITIONS:
		case NOTIFY_TYPE:	
			return true;
		default:
			return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool MSO_CheckProperty(BACnetPropertyIdentifier propertyID)
{
		switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case DESCRIPTION:
		case PRESENT_VALUE:
		case DEVICE_TYPE:
		case STATUS_FLAGS:
		case EVENT_STATE:
		case RELIABILITY:
		case OUT_OF_SERVICE:
		case NUMBER_OF_STATES:
		case STATE_TEXT:
		case PRIORITY_ARRAY:    
		case RELINQUISH_DEFAULT:
		case TIME_DELAY:
		case NOTIFICATION_CLASS:
		case FEEDBACK_VALUE:
		case EVENT_ENABLE:	
		case ACKED_TRANSITIONS:
		case NOTIFY_TYPE:
		case EVENT_TIME_STAMPS:
		case PROFILE_NAME:			
			return true;
		default:
			return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool MSV_CheckProperty(BACnetPropertyIdentifier propertyID)
{
		switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case PRESENT_VALUE:
		case DESCRIPTION:
		case STATUS_FLAGS:
		case EVENT_STATE:
		case RELIABILITY:
		case OUT_OF_SERVICE:
		case NUMBER_OF_STATES:
		case STATE_TEXT:
		case PRIORITY_ARRAY:    
		case RELINQUISH_DEFAULT:
		case TIME_DELAY:
		case NOTIFICATION_CLASS:
		case ALARM_VALUES:					// array of 10 words...  was 'uw' type (I think by mistake)
		case FAULT_VALUES:					// array of 10 words   was 'uw' type (I think by mistake)      	       
		case EVENT_ENABLE:	
		case ACKED_TRANSITIONS:
		case NOTIFY_TYPE:
		case EVENT_TIME_STAMPS:
		case PROFILE_NAME:			
			return true;
		default:
			return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool NC_CheckProperty(BACnetPropertyIdentifier propertyID)
{
		switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case DESCRIPTION:
		case NOTIFICATION_CLASS:
		case PRIORITY:
		case ACK_REQUIRED:
		case RECIPIENT_LIST:
		case PROFILE_NAME:
			return true;
		default:
			return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool PR_CheckProperty(BACnetPropertyIdentifier propertyID)
{
		switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case DESCRIPTION:
		case PROGRAM_STATE:
		case PROGRAM_CHANGE:
		case REASON_FOR_HALT:
		case DESCRIPTION_OF_HALT:
		case PROGRAM_LOCATION:
		case INSTANCE_OF:
		case STATUS_FLAGS:
		case RELIABILITY:
		case OUT_OF_SERVICE:
		case PROFILE_NAME:
			return true;
		default:
			return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool SC_CheckProperty(BACnetPropertyIdentifier propertyID)
{
		switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case DESCRIPTION:
		case PRESENT_VALUE:
		case EFFECTIVE_PERIOD:
		case WEEKLY_SCHEDULE:
		case EXCEPTION_SCHEDULE:
		case LIST_OF_OBJECT_PROPERTY_REFERENCES:
		case PRIORITY_FOR_WRITING:
		case PROFILE_NAME:
			return true;
		default:
			return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool AVG_CheckProperty(BACnetPropertyIdentifier propertyID)
{
	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case DESCRIPTION:
		case MINIMUM_VALUE:		       
		case MINIMUM_VALUE_TIMESTAMP:         	       
		case AVERAGE_VALUE:		       
		case VARIANCE_VALUE:	       
		case MAXIMUM_VALUE:		       
		case MAXIMUM_VALUE_TIMESTAMP:          	       
		case ATTEMPTED_SAMPLES:		   
		case VALID_SAMPLES:	           
		case OBJECT_PROPERTY_REFERENCE:      	       
		case WINDOW_INTERVAL:		   
		case WINDOW_SAMPLES:
		case PROFILE_NAME:	
			return true;
		default:
			return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool TR_CheckProperty(BACnetPropertyIdentifier propertyID)
{
	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case DESCRIPTION:
		case LOG_ENABLE:  			     
		case START_TIME:  	 		            	       
		case STOP_TIME:  				          	       
		case LOG_DEVICE_OBJECT_PROPERTY:          	       
		case LOG_INTERVAL:  			     
		case COV_RESUBSCRIPTION_INTERVAL: 
		case CLIENT_COV_INCREMENT:  	     
		case STOP_WHEN_FULL:  		     
		case BUFFER_SIZE:  			     
		case LOG_BUFFER:  			             	       
		case RECORD_COUNT:  			     
		case TOTAL_RECORD_COUNT:  	     
		case NOTIFICATION_THRESHOLD:      
		case RECORDS_SINCE_NOTIFICATION:  
	//	case PREVIOUS_NOTIFY_TIME:  	             	       
	//	case CURRENT_NOTIFY_TIME:  	// Modified by Zhu Zhenhua 2004-5-11     	       
		case EVENT_STATE:  			     
		case NOTIFICATION_CLASS:
		case EVENT_ENABLE:	
		case ACKED_TRANSITIONS:
		case NOTIFY_TYPE:
		case EVENT_TIME_STAMPS:	
		case PROFILE_NAME:
		case LAST_NOTIFY_RECORD:  // Added by Zhu Zhenhua 2004-5-11
			return true;
		default:
			return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool LSP_CheckProperty(BACnetPropertyIdentifier propertyID)
{
	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case PRESENT_VALUE:
		case TRACKING_VALUE:
		case DESCRIPTION:
		case DEVICE_TYPE:
		case STATUS_FLAGS:
		case EVENT_STATE:
		case RELIABILITY:
		case OUT_OF_SERVICE:
		case MODE:
		case TIME_DELAY:
		case NOTIFICATION_CLASS:
		case LIFE_SAFETY_ALARM_VALUES:
		case ALARM_VALUES:
		case FAULT_VALUES:	
		case EVENT_ENABLE:  			     
		case ACKED_TRANSITIONS:  	 		            	       
		case NOTIFY_TYPE:  				          	       
		case EVENT_TIME_STAMPS:          	       
		case SILENCED:  			     
		case OPERATION_EXPECTED: 
		case MAINTENANCE_REQUIRED:  	     
		case SETTING:  		     
		case DIRECT_READING:  			     
		case UNITS:  			             	       
		case MEMBER_OF:  			     
		case PROFILE_NAME:
			return true;
		default:
			return false;
	}
}

// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool LSZ_CheckProperty(BACnetPropertyIdentifier propertyID)
{
	switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case PRESENT_VALUE:
		case TRACKING_VALUE:
		case DESCRIPTION:
		case DEVICE_TYPE:
		case STATUS_FLAGS:
		case EVENT_STATE:
		case RELIABILITY:
		case OUT_OF_SERVICE:
		case MODE:
		case TIME_DELAY:
		case NOTIFICATION_CLASS:
		case LIFE_SAFETY_ALARM_VALUES:
		case ALARM_VALUES:
		case FAULT_VALUES:	
		case EVENT_ENABLE:  			     
		case ACKED_TRANSITIONS:  	 		            	       
		case NOTIFY_TYPE:  				          	       
		case EVENT_TIME_STAMPS:          	       
		case SILENCED:  			     
		case OPERATION_EXPECTED: 
		case MAINTENANCE_REQUIRED:  	     
		case ZONE_MEMBERS:  		     
		case MEMBER_OF:  			      			     
		case PROFILE_NAME:
			return true;
		default:
			return false;
	}
}

bool SC_CheckPropery(BACnetPropertyIdentifier propertyID)
{
		switch ( propertyID )
	{
		case OBJECT_IDENTIFIER:
		case OBJECT_NAME:
		case OBJECT_TYPE:
		case DESCRIPTION:
		case PRESENT_VALUE:
		case EFFECTIVE_PERIOD:
		case WEEKLY_SCHEDULE:
		case EXCEPTION_SCHEDULE:
		case LIST_OF_OBJECT_PROPERTY_REFERENCES:
		case PRIORITY_FOR_WRITING:
			return true;
		default:
			return false;
	}
}

} // end of PICS namespace


// Added by Zhu Zhenhua 2003-7-22
// To check whether the propertyID is one of the Object's standard properties 
bool Check_Obj_Prop(int ObjType, unsigned int propertyID)
{	
	BACnetPropertyIdentifier propID = (BACnetPropertyIdentifier) propertyID;
	switch(ObjType)
	{
	case analogInput:
		return PICS::AI_CheckProperty(propID);
	case analogOutput:
		return PICS::AO_CheckProperty(propID);
	case analogValue:
		return PICS::AV_CheckProperty(propID);
	case binaryInput:
		return PICS::BI_CheckProperty(propID);
	case binaryOutput:
		return PICS::BO_CheckProperty(propID);
	case binaryValue:
		return PICS::BV_CheckProperty(propID);
	case calendar:
		return PICS::CA_CheckProperty(propID);
	case command:
		return PICS::CO_CheckProperty(propID);
	case device:
		return PICS::DE_CheckProperty(propID);
	case eventEnrollment:
		return PICS::EV_CheckProperty(propID);
	case file:
		return PICS::FI_CheckProperty(propID);					
	case group:
		return PICS::GR_CheckProperty(propID);						
	case loop:
		return PICS::LO_CheckProperty(propID);						
	case multistateInput:
		return PICS::MSI_CheckProperty(propID);				
	case multistateOutput:
		return PICS::MSO_CheckProperty(propID);		
	case notificationClass:
		return PICS::NC_CheckProperty(propID);				
	case program:
		return PICS::PR_CheckProperty(propID);					
	case schedule:
		return PICS::SC_CheckProperty(propID);
	case averaging:
		return PICS::AVG_CheckProperty(propID);
	case multistateValue:
		return PICS::MSV_CheckProperty(propID);
	case trendlog:
		return PICS::TR_CheckProperty(propID);
	case lifesafetypoint:
		return PICS::LSP_CheckProperty(propID);
	case lifesafetyzone:
		return PICS::LSZ_CheckProperty(propID);

	default:
		return false;
	}
}



