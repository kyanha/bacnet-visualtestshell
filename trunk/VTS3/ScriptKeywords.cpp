
#include "stdafx.h"
#include "VTS.h"

#include "ScriptBase.h"
#include "ScriptKeywords.h"

//
//	BIP Message Types
//

ScriptTranslateTable ScriptBIPMsgTypeMap[] =
	{ { 0x7AD9DDA9,  0 }					// BVLC-RESULT
	, { 0xDB2C653F,  1 }					// WRITE-BROADCAST-DISTRIBUTION-TABLE
	, { 0xEABBFFDD,  1 }					// WRITE-BDT
	, { 0xBE1E123D,  2 }					// READ-BROADCAST-DISTRIBUTION-TABLE
	, { 0x22846274,  2 }					// READ-BDT
	, { 0xAB90D4E9,  3 }					// READ-BROADCAST-DISTRIBUTION-TABLE-ACK
	, { 0xAE10BC2C,  3 }					// READ-BDT-ACK
	, { 0xA34641EF,  4 }					// FORWARDED-NPDU
	, { 0xF3BF47A9,  5 }					// REGISTER-FOREIGN-DEVICE
	, { 0x44BFC4B4,  6 }					// READ-FOREIGN-DEVICE-TABLE
	, { 0xF2022D0C,  6 }					// READ-FDT
	, { 0x0ACBEA40,  7 }					// READ-FOREIGN-DEVICE-TABLE-ACK
	, { 0x7D8E86C4,  7 }					// READ-FDT-ACK
	, { 0x5365DF07,  8 }					// DELETE-FOREIGN-DEVICE-TABLE-ENTRY
	, { 0x9DBD6C6A,  8 }					// DELETE-FDT-ENTRY
	, { 0x2BCD74BF,  9 }					// DISTRIBUTE-BROADCAST-TO-NETWORK
	, { 0x5C99962D, 10 }					// ORIGINAL-UNICAST-NPDU
	, { 0x911075CC, 11 }					// ORIGINAL-BROADCAST-NPDU
	, { 0, 0 }
	};

//
//	NL Message Types
//

ScriptTranslateTable ScriptNLMsgTypeMap[] =
	{ { 0x294E7816, 0 }						// WHO-IS-ROUTER-TO-NETWORK
	, { 0x0154EC4F, 0 }						// WHOS-RTN
	, { 0x89F9A2D5, 1 }						// I-AM-ROUTER-TO-NETWORK
	, { 0x53C1FED2, 1 }						// IM-RTN
	, { 0xF568D0CC, 2 }						// I-COULD-BE-ROUTER-TO-NETWORK
	, { 0xDEDA7E5F, 2 }						// I-CLD-BE-RTN
	, { 0x4ED9703F, 3 }						// REJECT-MESSAGE-TO-NETWORK
	, { 0xC5D299CB, 3 }						// REJ-MTN
	, { 0xCBCA6F6C, 4 }						// ROUTER-BUSY-TO-NETWORK
	, { 0x39B8FAFE, 4 }						// RBTN
	, { 0x5EEA137B, 5 }						// ROUTER-AVAILABLE-TO-NETWORK
	, { 0x668F275C, 5 }						// RATN
	, { 0x4B02A6C9, 6 }						// INITIALIZE-ROUTING-TABLE
	, { 0xEF02FDF7, 6 }						// IRT
	, { 0x0C092241, 7 }						// INITIALIZE-ROUTING-TABLE-ACK
	, { 0xE6F311EB, 7 }						// IRT-ACK
	, { 0xA35B5DA7, 8 }						// ESTABLISH-CONNECTION-TO-NETWORK
	, { 0x080256D5, 8 }						// ECTN
	, { 0xFDDDF574, 9 }						// DISCONNECT-CONNECTION-TO-NETWORK
	, { 0x07A24D9E, 9 }						// DCTN
	, { 0, 0 }
	};


//
//	AL Message Types
//

ScriptTranslateTable ScriptALMsgTypeMap[] =
	{ { 0x566b432b, 0 }						// CONFIRMED-REQUEST
	, { 0xa17e3788, 1 }						// UNCONFIRMED-REQUEST
	, { 0x324dc228, 2 }						// SIMPLEACK
	, { 0x38a36471, 3 }						// COMPLEXACK
	, { 0x6c7c0630, 4 }						// SEGMENTACK
	, { 0x71ad2a43, 5 }						// ERROR
	, { 0xa9a5413f, 6 }						// REJECT
	, { 0xa1e0029a, 7 }						// ABORT
	, { 0, 0 }
	};

//
//	AL Service Choices
//

ScriptTranslateTable ScriptALConfirmedServiceMap[] =
	{ { 0xab6d1e6a,  0 }					// ACKNOWLEDGEALARM
	, { 0x3cb39a42,  1 }					// CONFIRMEDCOVNOTIFICATION
	, { 0xa3199d6d,  2 }					// CONFIRMEDEVENTNOTIFICATION
	, { 0x82ef96f3,  3 }					// GETALARMSUMMARY
	, { 0x215f7c25,  4 }					// GETENROLLMENTSUMMARY
	, { 0xac01f937,  5 }					// SUBSCRIBECOV
	, { 0x83874b40,  6 }					// ATOMICREADFILE
	, { 0xced53372,  7 }					// ATOMICWRITEFILE
	, { 0x14a62359,  8 }					// ADDLISTELEMENT
	, { 0x3781ff05,  9 }					// REMOVELISTELEMENT
	, { 0x9f0f989e, 10 }					// CREATEOBJECT
	, { 0x130be39e, 11 }					// DELETEOBJECT
	, { 0xc562eb57, 12 }					// READPROPERTY
	, { 0x207267b4, 13 }					// READPROPERTYCONDITIONAL
	, { 0x0f1b6c57, 14 }					// READPROPERTYMULTIPLE
	, { 0xb988389a, 15 }					// WRITEPROPERTY
	, { 0xeaf680ca, 16 }					// WRITEPROPERTYMULTIPLE
	, { 0x3264db4b, 17 }					// DEVICECOMMUNICATIONCONTROL
	, { 0x71e49612, 18 }					// CONFIRMEDPRIVATETRANSFER
	, { 0x4f9f9daf, 19 }					// CONFIRMEDTEXTMESSAGE
	, { 0x0b72caf4, 20 }					// REINITIALIZEDEVICE
	, { 0xc48dddd0, 21 }					// VTOPEN
	, { 0x7dd10cf5, 22 }					// VTCLOSE
	, { 0x0fa4f51c, 23 }					// VTDATA
	, { 0xd08d352b, 24 }					// AUTHENTICATE
	, { 0x9c38b663, 25 }					// REQUESTKEY
	, { 0, 0 }
	};

ScriptTranslateTable ScriptALUnconfirmedServiceMap[] =
	{ { 0xac8ab1d2, 0 }						// I-AM
	, { 0x9e0420f5, 1 }						// I-HAVE
	, { 0x1c25f7cd, 2 }						// UNCONFIRMEDCOVNOTIFICATION
	, { 0x2c0a8678, 3 }						// UNCONFIRMEDEVENTNOTIFICATION
	, { 0x38121a2f, 4 }						// UNCONFIRMEDPRIVATETRANSFER
	, { 0x6cb62a04, 5 }						// UNCONFIRMEDTEXTMESSAGE
	, { 0x64a09d6a, 6 }						// TIMESYNCHRONIZATION
	, { 0x474218f1, 7 }						// WHO-HAS
	, { 0xb0e1c86d, 8 }						// WHO-IS
	, { 0, 0 }
	};

//
//	Error and Abort Reasons
//

ScriptTranslateTable ScriptALRejectReasonMap[] = 
	{ { 0xeba74173, 0 }						// OTHER
	, { 0x54604281, 1 }						// BUFFER-OVERFLOW
	, { 0xe74ff917, 2 }						// INCONSISTENT-PARAMETERS
	, { 0x33d26973, 3 }						// INVALID-PARAMETER-DATA-TYPE
	, { 0x67ec4254, 4 }						// INVALID-TAG
	, { 0xf4bb55d8, 5 }						// MISSING-REQUIRED-PARAMETER
	, { 0x7538b688, 6 }						// PARAMETER-OUT-OF-RANGE
	, { 0x3e714c43, 7 }						// TOO-MANY-ARGUMENTS
	, { 0x21565c48, 8 }						// UNDEFINED-ENUMERATION
	, { 0x900c11d8, 9 }						// UNRECOGNIZED-SERVICE
	, { 0, 0 }
	};

ScriptTranslateTable ScriptALAbortReasonMap[] = 
	{ { 0xeba74173, 0 }						// OTHER
	, { 0x54604281, 1 }						// BUFFER-OVERFLOW
	, { 0x9cf0a085, 2 }						// INVALID-APDU-IN-THIS-STATE
	, { 0xbd8da0ca, 3 }						// PREEMPTED-BY-HIGHER-PRIORITY-TASK
	, { 0x8d1bd069, 4 }						// SEGMENTATION-NOT-SUPPORTED
	, { 0, 0 }
	};

//
//	Object Type Keywords
//

ScriptTranslateTable ScriptObjectTypeMap[] =
	{ { 0x7DE7AD77,  0 }					// ANALOG-INPUT
	, { 0x4F4FB029,  0 }					// AI
	, { 0xFEE3F6DF,  1 }					// ANALOG-OUTPUT
	, { 0x424AA5F5,  1 }					// AO
	, { 0x6C124D20,  2 }					// ANALOG-VALUE
	, { 0x086F6F63,  2 }					// AV
	, { 0x4EEB918E,  3 }					// BINARY-INPUT
	, { 0x4FAFB960,  3 }					// BI
	, { 0xCFE7DAF6,  4 }					// BINARY-OUTPUT
	, { 0x42AAAF2C,  4 }					// BO
	, { 0x3D163137,  5 }					// BINARY-VALUE
	, { 0x08CF789A,  5 }					// BV
	, { 0x5C6CE8BA,  6 }					// CALENDAR
	, { 0x80DDC343,  6 }					// CAL
	, { 0x7580BFD3,  7 }					// COMMAND
	, { 0x59EB1404,  7 }					// COM
	, { 0xC6DBC30F,  8 }					// DEVICE
	, { 0xCD8A8E14,  8 }					// DEV
	, { 0x92858852,  9 }					// EVENT-ENROLLMENT
	, { 0xBAB1FB08,  9 }					// EVE
	, { 0xE9A3598C, 10 }					// FILE
	, { 0xED2293C4, 11 }					// GROUP
	, { 0x7A76AE6D, 12 }					// LOOP
	, { 0x71B72EC4, 13 }					// MULTISTATE-INPUT
	, { 0x53D01EBD, 13 }					// MI
	, { 0x0FE88C80, 14 }					// MULTISTATE-OUTPUT
	, { 0x46CB1489, 14 }					// MO
	, { 0xA22F2639, 15 }					// NOTIFICATION-CLASS
	, { 0x61353228, 15 }					// NC
	, { 0x3A980404, 16 }					// PROGRAM
	, { 0x71D084EF, 16 }					// PRO
	, { 0x105A8215, 17 }					// SCHEDULE
	, { 0xFA230323, 17 }					// SCH
	, { 0x3771A5E3, 18 }					// AVERAGING
	, { 0x52B95396, 18 }					// AVG
	, { 0xE83992C9, 19 }					// MULTISTATE-VALUE
	, { 0x5D3473DF, 19 }					// MSV
	, { 0xAE9EAB5C, 20 }					// TRENDLOG
	, { 0xC2E8CFF0, 20 }					// TR
	, { 0, 0 }
	};

//
//	Application Layer Keywords
//

ScriptTranslateTable ScriptALMap[] =
	{ { kwNULL,					 1 }
	, { kwBOOL,					 2 }
	, { kwBOOLEAN,				 2 }
	, { kwUNSIGNED,				 3 }
	, { kwUNSIGNED8,			 3 }
	, { kwUNSIGNED16,			 3 }
	, { kwUNSIGNED32,			 3 }
	, { kwINTEGER,				 4 }
	, { kwREAL,					 5 }
	, { kwSINGLE,				 5 }
	, { kwDOUBLE,				 6 }
	, { kwOCTETSTRING,			 7 }
	, { kwCHARSTRING,			 8 }
	, { kwCHARACTERSTRING,		 8 }
	, { kwBITSTRING,			 9 }
	, { kwENUM,					10 }
	, { kwENUMERATED,			10 }
	, { kwDATE,					11 }
	, { kwTIME,					12 }
	, { kwOBJECT,				13 }
	, { kwOBJECTIDENTIFIER,		13 }
	, { kwDEVICE,				14 }
	, { kwDEVICEIDENTIFIER,		14 }
	, { kwPROPERTY,				15 }
	, { kwPROPERTYIDENTIFIER,	15 }
	, { kwOPEN,					16 }
	, { kwOPENTAG,				16 }
	, { kwOPENINGTAG,			16 }
	, { kwCLOSE,				17 }
	, { kwCLOSETAG,				17 }
	, { kwCLOSINGTAG,			17 }
	, { 0, 0 }
	};

//
//	Boolean Value Keywords
//

ScriptTranslateTable ScriptBooleanMap[] =
	{ { 0xEC9BC233, 0 }						// FALSE
	, { 0x1A42850A, 0 }						// F
	, { 0x472B1DC0, 0 }						// NO
	, { 0x1D42CEC2, 0 }						// N
	, { 0x0E3DFF9D, 1 }						// TRUE
	, { 0x1F83060C, 1 }						// T
	, { 0xEF201378, 1 }						// YES
	, { 0x2163341F, 1 }						// Y
	, { 0, 0 }
	};

//
//	Character String Keywords
//

ScriptTranslateTable ScriptCharacterTypeMap[] =
	{ { 0x34504ea6, 0 }						// ANSI
	, { 0xa30b8ce4, 1 }						// IBM
	, { 0x3bc0262e, 1 }						// MICROSOFT
	, { 0xb3b30c8b, 1 }						// DBCS
	, { 0x06118d13, 1 }						// IBM-MICROSOFT-DBCS
	, { 0x3626d7c7, 2 }						// JIS
	, { 0xd6cfdb32, 2 }						// JIS-C-6226
	, { 0xbb2f3de8, 3 }						// UCS-4
	, { 0x43f12c59, 3 }						// ISO-10646-UCS-4
	, { 0xff1d2202, 4 }						// UCS-2
	, { 0x32cdf787, 4 }						// ISO-10646-UCS-2
	, { 0x632522b6, 5 }						// ISO-8859-1
	, { 0, 0 }
	};

//
//	Weekday Keywords
//

ScriptTranslateTable ScriptWeekdayMap[] =
	{ { 0xAA6F2FAF, 1 }						// MON
	, { 0x5CE90128, 1 }						// MONDAY
	, { 0xED28B19F, 2 }						// TUE
	, { 0xB39B56E6, 2 }						// TUESDAY
	, { 0x6EE7D46F, 3 }						// WED
	, { 0x7A3908DB, 3 }						// WEDNESDAY
	, { 0x0044DDB5, 4 }						// THU
	, { 0xEFE97FD7, 4 }						// THURSDAY
	, { 0xA179B08B, 5 }						// FRI
	, { 0x53F38204, 5 }						// FRIDAY
	, { 0xECFC4C5B, 6 }						// SAT
	, { 0x0D213816, 6 }						// SATURDAY
	, { 0x9FAA5CC5, 7 }						// SUN
	, { 0x52242E3E, 7 }						// SUNDAY
	, { 0, 0 }
	};

//
//	Month Keywords
//

ScriptTranslateTable ScriptMonthMap[] =
	{ { 0x1D05812E,  1 }					// JAN
	, { 0xC4E47EAA,  1 }					// JANUARY
	, { 0xCEFFBA5E,  2 }					// FEB
	, { 0x76DEB7DA,  2 }					// FEBUARY
	, { 0x513497A7,  3 }					// MAR
	, { 0x88667C8B,  3 }					// MARCH
	, { 0xAC278F91,  4 }					// APR
	, { 0x78905B59,  4 }					// APRIL
	, { 0x6A8ECE9A,  5 }					// MAY
	, { 0x9C4A09D6,  6 }					// JUN
	, { 0x6AA0E76A,  6 }					// JUNE
	, { 0x02C28C6C,  7 }					// JUL
	, { 0x32025C50,  7 }					// JULY
	, { 0x7F8F7FF4,  8 }					// AUG
	, { 0x1DAEAA09,  8 }					// AUGUST
	, { 0x0694A00F,  9 }					// SEP
	, { 0xEDDC4E24,  9 }					// SEPTEMBER
	, { 0x91CFCEC3, 10 }					// OCT
	, { 0x2649B215, 10 }					// OCTOBER
	, { 0x10ED2E8E, 11 }					// NOV
	, { 0x1EB5E48A, 11 }					// MOVEMBER
	, { 0x1B0366A5, 12 }					// DEC
	, { 0x28CC1CA1, 12 }					// DECEMBER
	, { 0, 0 }
	};

//
//	Property Keywords
//

ScriptTranslateTable ScriptPropertyMap[] =
	{ { 0xD2C8E5BA,   0 }					// ACKED-TRANSITIONS
	, { 0x7B14B2C1,   1 }					// ACK-REQUIRED
	, { 0x412831F6,   2 }					// ACTION
	, { 0xD9B8BC52,   3 }					// ACTION-TEXT
	, { 0x191D1621,   4 }					// ACTIVE-TEXT
	, { 0x9EBC7807,   5 }					// ACTIVE-VT-SESSIONS
	, { 0x6D6C0816,   6 }					// ALARM-VALUE
	, { 0xFB3F577A,   7 }					// ALARM-VALUES
	, { 0x92E9C8CB,   8 }					// ALL
	, { 0x9EC2E38C,   9 }					// ALL-WRITES-SUCCESSFUL
	, { 0x9C2F5CC2,  10 }					// APDU-SEGMENT-TIMEOUT
	, { 0x61CAB451,  11 }					// APDU-TIMEOUT
	, { 0xD2476890,  12 }					// APPLICATION-SOFTWARE-VERSION
	, { 0xA3A2DE5E,  13 }					// ARCHIVE
	, { 0xAC5F9EB9,  14 }					// BAIS
	, { 0xDC62963D,  15 }					// CHANGE-OF-STATE-COUNT
	, { 0x59DA69D0,  16 }					// CHANGE-OF-STATE-TIME
//	, { 0xXXXXXXXX,  17 }					// see NOTIFICATION-CLASS
//	, { 0xXXXXXXXX,  18 }					// deleted
	, { 0x30ABA767,  19 }					// CONTROLLED-VARIABLE-REFERENCE
	, { 0xE6E8E6CD,  20 }					// CONTROLLED-VARIABLE-UNITS
	, { 0x5D8F717B,  21 }					// CONTROLLED-VARIABLE-VALUE
	, { 0x533D4619,  22 }					// COV-INCREMENT
	, { 0x7836ADF3,  23 }					// DATELIST
	, { 0x549D27C7,  24 }					// DAYLIGHT-SAVINGS-TIME
	, { 0xFC57BCA5,  25 }					// DEADBAND
	, { 0x50E4A6CC,  26 }					// DERIVATIVE-CONSTANT
	, { 0x53A94439,  27 }					// DERIVATIVE-CONSTANT-UNITS
	, { 0xFC4184B0,  28 }					// DESCRIPTION
	, { 0x545345BD,  29 }					// DESCRIPTION-OF-HALT
	, { 0x5B30BCF5,  30 }					// DEVICE-ADDRESS-BINDING
	, { 0xA9D91124,  31 }					// DEVICE-TYPE
	, { 0x7B9E250F,  32 }					// EFFECTIVE-PERIOD
	, { 0x3749C65F,  33 }					// ELAPSED-ACTIVE-TIME
	, { 0xEAB41C26,  34 }					// ERROR-LIMIT
	, { 0xFCDAF7EA,  35 }					// EVENT-ENABLE
	, { 0x8B3EDE45,  36 }					// EVENT-STATE
	, { 0xA3512088,  37 }					// EVENT-TYPE
	, { 0x14DCAFD3,  83 }					// EVENT-PARAMETERS
	, { 0x1006DA3E,  38 }					// EXCEPTION-SCHEDULE
	, { 0x5ECB0648,  39 }					// FAULT-VALUES
	, { 0x56181F0D,  40 }					// FEEDBACK-VALUE
	, { 0xFFB2BF6A,  41 }					// FILE-ACCESS-METHOD
	, { 0xBBA878C9,  42 }					// FILE-SIZE
	, { 0x75ACD62F,  43 }					// FILE-TYPE
	, { 0x994F7B6D,  44 }					// FIRMWARE-VERSION
	, { 0x33D5FACC,  45 }					// HIGH-LIMIT
	, { 0x7BF647A4,  46 }					// INACTIVE-TEXT
	, { 0x35640442,  47 }					// IN-PROCESS
	, { 0xF762EB13,  48 }					// INSTANCE-OF
	, { 0xB127830F,  49 }					// INTEGRAL-CONSTANT
	, { 0xC99DB0A0,  50 }					// INTEGRAL-CONSTANT-UNITS
	, { 0x2EB54B81,  51 }					// ISSUE-CONFIRMED-NOTIFICATIONS
	, { 0x143F3245,  52 }					// LIMIT-ENABLE
	, { 0x91B5E524,  53 }					// LIST-OF-GROUP-MEMBERS
	, { 0xCF63D8A1,  54 }					// LIST-OF-OBJECT-PROPERTY-REFERENCES
	, { 0xA1D61611,  55 }					// LIST-OF-SESSION-KEYS
	, { 0x204D60D1,  56 }					// LOCAL-DATE
	, { 0x96685458,  57 }					// LOCAL-TIME
	, { 0x1D788616,  58 }					// LOCATION
	, { 0xB87DC3B4,  59 }					// LOW-LIMIT
	, { 0xEE9D6B1B,  60 }					// MANIPULATED-VARIABLE-REFERENCE
	, { 0xC1CA1939,  61 }					// MAXIMUM-OUTPUT
	, { 0x9BFEA8B6,  62 }					// MAX-APDU-LENGTH-ACCEPTED
	, { 0x3F6E54AB,  63 }					// MAX-INFO-FRAMES
	, { 0x3CB7BEB8,  64 }					// MAX-MASTER
	, { 0xA963610B,  65 }					// MAX-PRES-VALUE
	, { 0x01F908A7,  66 }					// MINIMUM-OFF-TIME
	, { 0xB04C1D6D,  67 }					// MINIMUM-ON-TIME
	, { 0x5B734337,  68 }					// MINIMUM-OUTPUT
	, { 0x430C8B09,  69 }					// MIN-PRES-VALUE
	, { 0x3B04506E,  70 }					// MODEL-NAME
	, { 0xEFAFF308,  71 }					// MODIFICATION-DATE
	, { 0xC4C5900C,  17 }					// NOTIFICATION-CLASS
	, { 0x7B445305,  72 }					// NOTIFY-TYPE
	, { 0xA274AE5F,  73 }					// NUMBER-OF-APDU-RETRIES
	, { 0x53163088,  74 }					// NUMBER-OF-STATES
	, { 0xEFD39023,  75 }					// OBJECT-IDENTIFIER
	, { 0xE10A423A,  76 }					// OBJECT-LIST
	, { 0x1C6E0833,  77 }					// OBJECT-NAME
	, { 0xD4EC967F,  78 }					// OBJECT-PROPERTY-REFERENCE
	, { 0x1204F8C9,  79 }					// OBJECT-TYPE
	, { 0xE8B689C3,  80 }					// OPTIONAL
	, { 0x8234FD41,  81 }					// OUT-OF-SERVICE
	, { 0xCF5CB087,  82 }					// OUTPUT-UNITS
//	, { 0xXXXXXXXX,  83 }					// see EVENT-PARAMETERS
	, { 0x6EA83676,  84 }					// POLARITY
	, { 0xC4FC82DB,  85 }					// PRESENT-VALUE
	, { 0x45B0B875,  86 }					// PRIORITY
	, { 0xBD86E34D,  87 }					// PRIORITY-ARRAY
	, { 0xAAF64072,  88 }					// PRIORITY-FOR-WRITING
	, { 0xFD38DEB6,  89 }					// PROCESS-IDENTIFIER
	, { 0x84BD27B1,  90 }					// PROGRAM-CHANGE
	, { 0xF6C4F02A,  91 }					// PROGRAM-LOCATION
	, { 0x8F5C8F25,  92 }					// PROGRAM-STATE
	, { 0x4264B4A4,  93 }					// PROPORTIONAL-CONSTANT
	, { 0x0121A66D,  94 }					// PROPORTIONAL-CONSTANT-UNITS
	, { 0x24FD3E1D,  95 }					// PROTOCOL-CONFORMANCE-CLASS
	, { 0x72D66985,  96 }					// PROTOCOL-TYPES-SUPPORTED
	, { 0xB4C9E525,  97 }					// PROTOCOL-SERVICES-SUPPORTED
	, { 0x8187E6DB,  98 }					// PROTOCOL-VERSION
	, { 0xD023E3E3,  99 }					// READ-ONLY
	, { 0xC5611A2C, 100 }					// REASON-FOR-HALT
	, { 0x788F1962, 101 }					// RECIPIENT
	, { 0x47F52455, 102 }					// RECIPIENT-LIST
	, { 0x77DB2094, 103 }					// RELIABILITY
	, { 0x9B12078B, 104 }					// RELINQUISH-DEFAULT
	, { 0x764659ED, 105 }					// REQUIRED
	, { 0x50C6871E, 106 }					// RESOLUTION
	, { 0x7053A72B, 107 }					// SEGMENTATION-SUPPORTED
	, { 0xE3E4B784, 108 }					// SETPOINT
	, { 0x04566513, 109 }					// SETPOINT-REFERENCE
	, { 0xD7201E1B, 110 }					// STATE-TEXT
	, { 0xDE1DCE4D, 111 }					// STATUS-FLAGS
	, { 0x414C7A0F, 112 }					// SYSTEM-STATUS
	, { 0x7405A2F0, 113 }					// TIME-DELAY
	, { 0xCFC0E52D, 114 }					// TIME-OF-ACTIVE-TIME-RESET
	, { 0x4819BCEE, 115 }					// TIME-OF-STATE-COUNT-RESET
	, { 0xC6301FD8, 116 }					// TIME-SYNCHRONIZATION-RECIPIENTS
	, { 0x6E680555, 117 }					// UNITS
	, { 0x076DBEF9, 118 }					// UPDATE-INTERVAL
	, { 0x967878D0, 119 }					// UTC-OFFSET
	, { 0x1AD595F2, 120 }					// VENDOR-IDENTIFIER
	, { 0x47700E02, 121 }					// VENDOR-NAME
	, { 0x70A3D8F0, 122 }					// VT-CLASSES-SUPPORTED
	, { 0x85001409, 123 }					// WEEKLY-SCHEDULE
	, { 0, 0 }
	};
