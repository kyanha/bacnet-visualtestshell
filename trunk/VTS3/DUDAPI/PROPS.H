//PROPS.H
//property definitions for standard objects
#include "VTS.h"
#ifndef __PROPS_H_INCLUDED
#define __PROPS_H_INCLUDED

//ParseTypes
#define	ob_id				0					//an object identifier
#define	et					1					//an enumeration table
#define	flt					2					//float
#define	bits				3					//octet of T or F flags
#define	uw					4					//unsigned word
#define	ud					5					//unsigned dword
#define	sw					6					//signed word
#define	u127				7					//1..127
#define	s10					8					//char [10]
#define	s32					9					//char [32]
#define	s64					10					//char [64]
#define	s132				11					//char [132]
#define	propref				12					//obj property reference
#define	dt					13					//date/time
#define	calist				14					//list of calendarentry
#define	act					15					//action array
#define	actext				16					//action_text array
#define	pss					17					//protocol services supported bitstring
#define	pos					18					//protocol objects supported bitstring
#define	vtcl				19					//vt classes
#define	evparm				20					//event parameter
#define	skeys				21					//session keys
#define	tsrecip				22					//time synch recipients
#define	dabind				23					//device address bindings
#define	recip				24					//recipient
#define	paf					25					//priority array flt
#define	statext				26					//state text array
#define	pab					27					//priority array bpv
#define	pau					28					//priority array uw
#define	reciplist			29					//list of BACnetDestination
#define	xsched				30					//exception schedule: array[] of specialevent
#define	wsched				31					//weekly schedule: array[7] of list of timevalue
#define	u16					32					//1..16
#define	setref				33					//setpoint reference
#define	raslist				34					//list of readaccessspecs
#define	stavals				35					//list of unsigned
#define	dtrange				36					//range of dates
#define	lopref				37					//list of object prop refs
#define ebool				38					//boolean enumeration
#define ddate				39					//date
#define ttime				40					//time
#define looref				41					//list of objectid references
#define ssint				42					//short (2-byte) signed integer  MAG 13 FEB 2001
#define LOGREC				43					//LogRecord
#define TSTMP               44             // BACnetTimeStamp
//Added  Parse Time by Liangping Xu,2002-9

#define	devobjpropref	    45					//dev obj property reference

#define calent				46					//single calendar entry, madanner 11/1/02
#define prival				47					//single priority value (could be b, flt, null, r), madanner 11/1/02

//Added by Zhu Zhen hua, 2003-9-4
#define dsofweek            48			//BACnetDaysofWeek
#define eventransbits		49			//BACnetEventTansitionBits
#define recipt				50			//BACnetRecipient
#define destination         51			//BACnetDestination
#define sequenceof			52			//BACnetSequenceOf
#define listof				53			//BACnetListOf
#define arrayof				54			//BACnetArrayOf

#define ptDate 200  // parsetype BACnetDate
#define ptTime 201  // parsetype BACnetTime
#define lobj   202  // array of objects, (parse type)
#define vtse   203  // list of active  vt sessions (parse type)
#define uwarr  204  // array of uw

#define lCOVSub 205		// List of BACnetCOVSubcription,   added Gingbo Gao, 2003-9-1



////
//////////////////////////////////////////////
#define enull				254					//null enumeration
#define	none				255					//no parsing required, runtime generated value



static const char nullstring[5]={"null"};	//"null\0";			//	***001
static const char anystring[4]={"any"};		//"any\0";			//	***001

static const char *stParseTypes[]={
				"object ID",
				"enumeration",
				"float",
				"bit flags",
				"unsigned16",
				"unsigned32",
				"signed16",
				"1..127",
				"char10",
				"char32",
				"char64",
				"char132",
				"prop ref",
				"date time",
				"calendar list",
				"action array",
				"action text",
				"prot services",
				"prot objects",
				"vt classes",
				"event parm",
				"session keys",
				"time recip",
				"devaddr binding",
				"recipient",
				"flt:pri array",
				"state text",
				"bpv:pri array",
				"uw:pri array",
				"dest list",
				"excep sched",
				"weekly sched",
				"1..16",
				"setpt ref",
				"Read list",
				"uns list",
				"date range",
				"obj list",
				"boolean",
				"date",
				"time",
				"obj ref list",
				"short signed int",  //MAG added 13 FEB 2001
				NULL /*always last*/
				};
//Special table for parse types
static const char *stRelationTypes[]={
				"==",
				"!=",
				"< ",
				"> ",
				"<=",
				">=",
				NULL /*always last*/
				};
//Special table for property ids

static const char *stPropIDs[]={
				"acked-transitions",
				"ack-required",
				"action",
				"action-text",
				"active-text",
				"active-vt-sessions",
				"alarm-value",
				"alarm-values",
				"all",
				"all-writes-successful",
				"apdu-segment-timout",
				"apdu-timout",
				"application-software-version",
				"archive",
				"bias",
				"change-of-state-count",
				"change-of-state-time",
				"notification-class",
				"reserved propertyID 18",
				"controlled-variable-reference",
				"controlled-variable-units",
				"controlled-variable-value",
				"cov-increment",
				"datelist",
				"daylight-savings-status",
				"deadband",
				"derivative-constant",
				"derivative-constant-units",
				"description",
				"description-of-halt",
				"device-address-binding",
				"device-type",
				"effective-period",
				"elapsed-active-time",
				"error-limit",
				"event-enabled",
				"event-state",
				"event-type",
				"exception-schedule",
				"fault-values",
				"feedback-value",
				"file-access-method",
				"file-size",
				"file-type",
				"firmware-revision",
				"high-limit",
				"inactive-text",
				"in-process",
				"instance-of",
				"integral-constant",
				"integral-constant-units",
				"issue-confirmed-notifications",
				"limit-enable",
				"list-of-group-members",
				"list-of-object-property-references",
				"list-of-session-keys",
				"local-date",
				"local-time",
				"location",
				"low-limit",
				"manipulated-variable-reference",
				"maximum-output",
				"max-apdu-length-accepted",
				"max-info-frames",
				"max-master",
				"max-pres-value",
				"minimum-off-time",
				"minimum-on-time",
				"minimum-output",
				"min-pres-value",
				"model-name",
				"modification-date",
				"notify-type",
				"number-of-apdu-retries",
				"number-of-states",
				"object-identifier",
				"object-list",
				"object-name",
				"object-property-reference",
				"object-type",
				"optional",
				"out-of-service",
				"output-units",
				"event-parameters",
				"polarity",
				"present-value",
				"priority",
				"priority-array",
				"priority-for-writing",
				"process-identifier",
				"program-change",
				"program-location",
				"program-state",
				"proportional-constant",
				"proportional-constant-units",
				"protocol-conformance-class",
				"protocol-object-types-supported",
				"protocol-services-supported",
				"protocol-version",
				"read-only",
				"reason-for-halt",
				"recipient",
				"recipient-list",
				"reliability",
				"relinquish-default",
				"required",
				"resolution",
				"segmentation-supported",
				"setpoint",
				"setpoint-reference",
				"state-text",
				"status-flags",
				"system-status",
				"time-delay",
				"time-of-active-time-reset",
				"time-of-state-count-reset",
				"time-synchronization-recipients",
				"units",
				"update-interval",
				"utc-offset",
				"vendor-identifier",
				"vendor-name",
				"vt-classes-supported",
				"weekly-schedule",
				"attempted_samples",                        
				"average_value",                            
				"buffer_size",                              
				"client_cov_increment",                     
				"cov_resubscription_interval",              
//				"current_notify_time",                      
				"event_time_stamps",                        
				"log_buffer",                               
				"log_device_object_property",               
				"log_enable",                               
				"log_interval",                             
				"maximum_value",                            
				"minimum_value",                            
				"notification_threshold",                   
			//	"previous_notify_time",                     
				"protocol_revision",                        
				"records_since_notification",               
				"record_count",                             
				"start_time",                               
				"stop_time",                                
				"stop_when_full",                           
				"total_record_count",                       
				"valid_samples",                            
				"window_interval",                          
				"window_samples",                           
				"maximum_value_timestamp",                  
				"minimum_value_timestamp",                  
				"variance_value", 
				"last_notify_record",
				NULL /*always last*/
				};
//qualifiers in PropFlags
#define	O					0x0000				//optional
#define	R					0x0001				//required
#define	W					0x0002				//required writable
#define	Woutofservice		0x0004				//req writable when OutOfService is true
#define	WithGroup			0x0008				//all in PropGroup must be present if any are
#define	NotWithGroup1		0x0010				//must not be present if any in PropGroup=1 are
#define	NotWithGroup2		0x0020				//must not be present if any in PropGroup=2 are
#define	WithService			0x0040				//required if service PropGroup is supported
#define	AtLeast1			0x0080				//at least one from PropGroup must be present
#define	IsArray				0x0100				//property is an array
#define IsCommandable		0x0200				//property is commandable
//services supported in PropGroup
#define	COV					127					//COV reporting
#define	Intr				126					//Instrinsic reporting
#define	VT					125					//VT services
#define	SEGSVC				124					//segmentation
#define	TMASTER				123					//time master, must also be writable
#define	MSTPMASTER			122					//MS/TP master node
//last definition in an object is marked in PropGroup with:
#define	Last		0x80

//NOTE: the following equates MUST represent the order in the AllETs table in STDOBJPR.H!
//msdanner 9/2004 - all reduced by one to eliminate need for "-1" all the time.
#define		eiTF           0
#define		eiReli         1
#define		eiEvState      2
#define		eiEU           3
#define		eiNT           4
#define		eiBPV          5
#define		eiPolar        6
#define		eiDS           7
#define		eiSegOpt       8
#define		eiEvType       9
#define		eiFAM          10
#define		eiLoopAct      11
#define		eiPrErr        12
#define		eiPrState      13
#define		eiPrChg        14
#define 	eiObjectTypes     15
#define		eiStF          16
#define		eiEvTr         17
#define		eiLimEn        18
#define		eiVTCls        19
#define		eiDOW          20
#define		eiYN           21
#define		eiCalType      22
#define		eiMon          23
#define		eiWOM          24
#define		eiAnyDOW       25
#define		eiBPVn         26
#define		eiAppDataTypes 27

#endif //__PROPS_H_INCLUDED
