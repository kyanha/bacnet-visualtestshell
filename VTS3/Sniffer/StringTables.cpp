/*  ------ BACnet string table s--------------- */

#include "stdafx.h"
#include "StringTables.h"

namespace NetworkSniffer {

// Return a test buffer for short term use, such as as an argument
// for sprintf.  Buffers are allocated from a small rotary set, so
// long-term usage will result in the buffer being re-used.
char* TempTextBuffer()
{
#define TEMP_TEST_N_BUFFERS 10
#define TEMP_TEST_BUFFERLENGTH 100
	static char buffers[TEMP_TEST_N_BUFFERS ][ TEMP_TEST_BUFFERLENGTH ];
	static int ix;

	ix = (ix + 1) % TEMP_TEST_N_BUFFERS;
	return buffers[ix];
}


// Return a string containing text for the specified enumerated value.
// If the value is undefined, the string will show the pUndefined title and the numeric value
const char* BACnetStringTable::EnumString( int theIndex, const char *pUndefined /* = NULL */ ) const
{
	const char *pRet;	
	if (theIndex < m_nStrings)
	{
		pRet = m_pStrings[ theIndex ];
	}
	else
	{
		if (pUndefined == NULL)
		{
			pUndefined = "";
		}

		char *pTxt = TempTextBuffer();
		sprintf( pTxt, "%s%d", pUndefined, theIndex );
		pRet = pTxt;
	}

	return pRet;
}

// Fill a CComboBox with the contents of the string table
void BACnetStringTable::FillCombo( CComboBox &theCombo ) const
{
	for (int ix = 0; ix < m_nStrings; ix++)
	{
		theCombo.AddString( m_pStrings[ ix ] );
	}
}


STRING_TABLE FalseTrue[] = {
	"False",
	"True"
};
BAC_STRINGTABLE(FalseTrue);

STRING_TABLE ApplicationTypes[] = {
   "Null",				// 0
   "Boolean",
   "Unsigned Integer",
   "Signed Integer",
   "Real",				// 4
   "Double",
   "Octet String",
   "Character String",
   "Bit String",		// 8
   "Enumerated",
   "Date",
   "Time",
   "BACnetObjectIdentifier" // 12
   };
BAC_STRINGTABLE(ApplicationTypes);

STRING_TABLE BACnetAction[] = {
   "DIRECT",
   "REVERSE"
   };
BAC_STRINGTABLE(BACnetAction);

STRING_TABLE BACnetActionList[] = {
	"Action"
};
BAC_STRINGTABLE(BACnetActionList);

STRING_TABLE BACnetActionCommand[] = {
   "Device Identifier",
   "Object Identifier",
   "Property Identifier",
   "Property Array Index",
   "Property Value",
   "Priority",
   "Post Delay",
   "Quit On Failure",
   "Write Successful"
   };
BAC_STRINGTABLE(BACnetActionCommand);

STRING_TABLE BACnetAddressBinding[] = {
   "Device Object Identifier",
   "Device Address"
};
BAC_STRINGTABLE(BACnetAddressBinding);

STRING_TABLE BACnetBinaryPV[] = {
   "INACTIVE",
   "ACTIVE"
   };
BAC_STRINGTABLE(BACnetBinaryPV);

STRING_TABLE BACnetCalendarEntry[] = {
   "Date",
   "Date Range",
   "WeekNDay"
    };
BAC_STRINGTABLE(BACnetCalendarEntry);

STRING_TABLE BACnetClientCOV[] = {
	"real-increment",
	"default-increment"
};
BAC_STRINGTABLE(BACnetClientCOV);

STRING_TABLE BACnetScale[] = {
	"floatScale",
	"integerScale"
};
BAC_STRINGTABLE(BACnetScale);

STRING_TABLE BACnetDateRange[] = {
   "Start Date",
   "End Date"
};
BAC_STRINGTABLE(BACnetDateRange);

STRING_TABLE BACnetDateTime[] = {
   "Date",
   "Time"
};
BAC_STRINGTABLE(BACnetDateTime);

STRING_TABLE BACnetTimeStamp[]= {
   "Time",
   "SequenceNumber",
   "DateTime"
};
BAC_STRINGTABLE(BACnetTimeStamp);

// Used for bitstring BACnetDaysOfWeek, where Monday == 0
// See also day_of_week
STRING_TABLE BACnetDaysOfWeek[] = {
   "Monday",
   "Tuesday",
   "Wednesday",
   "Thursday",
   "Friday",
   "Saturday",
   "Sunday"
};
BAC_STRINGTABLE(BACnetDaysOfWeek);

STRING_TABLE BACnetDestination[] = {
   "Valid Days",
   "From Time",
   "To Time",
   "Recipient",
   "Process Identifier",
   "Issue Confirmed Notifications",
   "Transitions"
};
BAC_STRINGTABLE(BACnetDestination);

STRING_TABLE BACnetDeviceObjectReference[] = {
	"Device Identifier",
	"Object Identifier"
};
BAC_STRINGTABLE(BACnetDeviceObjectReference);

//Added By Zhu Zhenhua, 2004-5-17
STRING_TABLE BACnetDeviceObjectPropertyReference[] = {
   "Object Identifier",
   "Property Identifier",
   "Property Array Index",
   "Device Identifier"
};
BAC_STRINGTABLE(BACnetDeviceObjectPropertyReference);

STRING_TABLE BACnetDeviceObjectPropertyValue[] = {
   "Device Identifier"
   "Object Identifier",
   "Property Identifier",
   "Property Array Index",
   "Value"
};
BAC_STRINGTABLE(BACnetDeviceObjectPropertyValue);

STRING_TABLE BACnetDeviceStatus[] = {
   "Operational",
   "Operational-read-only",
   "Download-required",
   "Download-in-progress",
   "Non-Operational",
   "Backup-in-porgress"		// added by Jingbo Gao, Sep 20 2004
};
BAC_STRINGTABLE(BACnetDeviceStatus);

STRING_TABLE BACnetDoorAlarmState[] = {
	"normal",
	"alarm",
	"door-open-too-long",
	"forced-open",
	"tamper",
	"door-fault",
	"lock-down",
	"free-access",
	"egress-open"
};
BAC_STRINGTABLE(BACnetDoorAlarmState);

STRING_TABLE BACnetDoorSecuredStatus[] = {
	"secured",
	"unsecured",
	"unknown",
};
BAC_STRINGTABLE(BACnetDoorSecuredStatus);

STRING_TABLE BACnetDoorStatus[] = {
	"closed",
	"open",
	"unknown",
};
BAC_STRINGTABLE(BACnetDoorStatus);

STRING_TABLE BACnetDoorValue[] = {
	"lock",
	"unlock",
	"pulse-unlock",
	"extended-pulse-unlock",
};
BAC_STRINGTABLE(BACnetDoorValue);

STRING_TABLE BACnetEngineeringUnits[] = {
/* Area */
   "Square-meters",             /* 0 */
   "Square-feet",               /* 1 */

/* Electrical */
   "Milliamperes",              /* 2 */
   "Amperes",                   /* 3 */
   "Ohms",                      /* 4 */
   "Volts",                     /* 5 */
   "Kilovolts",                 /* 6 */
   "Megavolts",                 /* 7 */
   "Volt-amperes",              /* 8 */
   "Kilovolt-amperes",          /* 9 */
   "Megavolt-amperes",          /* 10 */
   "Volt-amperes-reactive",     /* 11 */
   "Kilovolt-amperes-reactive", /* 12 */
   "Megavolt-amperes-reactive", /* 13 */
   "Degrees phase",             /* 14 */
   "Power factor",              /* 15 */

/* Energy */
   "Joules",                    /* 16 */
   "Kilojoules",                /* 17 */
   "Watt-hours",                /* 18 */
   "Kilowatt-hours",            /* 19 */
   "BTUs",                      /* 20 */
   "Therms",                    /* 21 */
   "Ton-hours",                 /* 22 */

/* Enthalpy */
   "Joules-per-kilogram-dry-air", /* 23 */
   "BTUs-per-pound-dry-air",      /* 24 */

/* Frequency */
   "Cycles-per-hour",            /* 25 */
   "Cycles-per-minute",          /* 26 */
   "Hertz",                      /* 27 */

/* Humidity */
   "Grams-of-water-per-kilogram-dry-air", /* 28 */
   "Percent-relative-humidity",           /* 29 */

/* Length */
   "Millimeter",                /* 30 */
   "Meters",                    /* 31 */
   "Inches",                    /* 32 */
   "Feet",                      /* 33 */

/* Light */
   "Watts-per-square-foot",     /* 34 */
   "Watts-per-square-meter",    /* 35 */
   "Lumens",                    /* 36 */
   "Luxes",                     /* 37 */
   "Foot-candles",              /* 38 */

/* Mass */
   "Kilograms",                 /* 39 */
   "Pounds-mass",               /* 40 */
   "Tons",                      /* 41 */

/* Mass Flow */
   "Kilograms-per-second",      /* 42 */
   "Kilograms-per-minute",      /* 43 */
   "Kilograms-per-hour",        /* 44 */
   "Pounds-mass-per-minute",    /* 45 */
   "Pounds-mass-per-hour",      /* 46 */

/* Power */
   "Watts",                     /* 47 */
   "Kilowatts",                 /* 48 */
   "Megawatts",                 /* 49 */
   "BTUs-per-hour",             /* 50 */
   "Horsepower",                /* 51 */
   "Tons (refrigeration)",      /* 52 */

/* Pressure */
   "Pascals",                      /* 53 */
   "Kilopascals",                  /* 54 */
   "Bars",                         /* 55 */
   "Pounds-force-per-square-inch", /* 56 */
   "Centimeters-of-water",         /* 57 */
   "Inches-of-water",              /* 58 */
   "Millimeters-of-mecury",        /* 59 */
   "Centimeters-of-mecury",        /* 60 */
   "Inches-of-mecury",             /* 61 */

/* Temperature */
   "Degrees-Celsius",           /* 62 */
   "Degrees-Kelvin",            /* 63 */
   "Degrees-Fahrenheit",        /* 64 */
   "Degree-days-Celsius",       /* 65 */
   "Degree-days-Fahrenheit",    /* 66 */

/* Time */
   "Years",   /* 67 */
   "Months",  /* 68 */
   "Weeks",   /* 69 */
   "Days",    /* 70 */
   "Hours",   /* 71 */
   "Minutes", /* 72 */
   "Seconds", /* 73 */

/* Velocity */
   "Meters-per-second",   /* 74 */
   "Kilometers-per-hour", /* 75 */
   "Feet-per-second",     /* 76 */
   "Feet-per-minute",     /* 77 */
   "Miles-per-hour",      /* 78 */

/* Volume */
   "Cubic-feet",        /* 79 */
   "Cubic-meters",      /* 80 */
   "Imperial-Gallons",  /* 81 */
   "Liters",            /* 82 */
   "US-Gallons",        /* 83 */

/* Volumetric Flow */
   "Cubic-feet-per-minute",        /* 84 */
   "Cubic-meters-per-second",      /* 85 */
   "Imperial-gallons-per minute",  /* 86 */
   "Liters-per-second",            /* 87 */
   "Liters-per-minute",            /* 88 */
   "US-Gallons-per minute",        /* 89 */

/* Other */
   "Degrees-angular",               /* 90 */
   "Degrees-Celcius-per-hour",      /* 91 */
   "Degrees-Celcius-per-minute",    /* 92 */
   "Degrees-Fahrenheit-per-hour",   /* 93 */
   "Degrees-Fahrenheit-per-minute", /* 94 */
   "No-units",                      /* 95 */
   "Parts-per-million",             /* 96 */
   "Parts-per-billion",             /* 97 */
   "Percent",                       /* 98 */
   "Percent-per-second",            /* 99 */
   "Per-minute",                    /* 100 */
   "Per-second",                    /* 101 */
   "PSI-Per-Degree-Fahrenheit",     /* 102 */
   "Radians",                       /* 103 */
   "Revolutions-per-minute",        /* 104 */

   /* added in second or third public review */
   "Currency 1",        /* 105 */
   "Currency 2",        /* 106 */
   "Currency 3",        /* 107 */
   "Currency 4",        /* 108 */
   "Currency 5",        /* 109 */
   "Currency 6",        /* 110 */
   "Currency 7",        /* 111 */
   "Currency 8",        /* 112 */
   "Currency 9",        /* 113 */
   "Currency 10",       /* 114 */

   "square-inches",							/* 115 */
   "square-centimeters",					/* 116 */
   "btus-per-pound",						/* 117 */
   "centimeters",							/* 118 */
   "pounds-mass-per-second",				/* 119 */
   "delta-degrees-Fahrenheit",				/* 120 */
   "delta-degrees-Kelvin",					/* 121 */

   "Kilohms",                              /* 122 */
   "Megohms",                              /* 123 */
   "Millivolts",                           /* 124 */
   "Kilojoules-per-kilogram",              /* 125 */
   "Megajoules",                           /* 126 */
   "Joules-per-degree-Kelvin",             /* 127 */
   "Joules-per-kilogram-degree-Kelvin",    /* 128 */
   "Kilohertz",                            /* 129 */
   "Megahertz",                            /* 130 */
   "Per-hour",                             /* 131 */
   "Milliwatts",                           /* 132 */
   "Hectopascals",                         /* 133 */
   "Millibars",                            /* 134 */
   "Cubic-meters-per-hour",                /* 135 */
   "Liters-per-hour",                      /* 136 */
   "Kilowatt-hours-per-square-meter",      /* 137 */
   "Kilowatt-hours-per-square-foot",       /* 138 */
   "Megajoules-per-square-meter",          /* 139 */
   "Megajoules-per-square-foot",           /* 140 */
   "Watts-per-square-meter-degree-Kelvin", /* 141 */
   // New units added 3/9/2008
   "Cubic-feet-per-second",					/* 142 */
   "Percent-obscuration-per-foot",			/* 143 */
   "Percent-obscuration-per-meter", 		/* 144 */
   "miliohms", 								/* 145 */
   "megawatt-hours", 						/* 146 */
   "kilo-btus",								/* 147 */
   "mega-btus",								/* 148 */
   "kilojoules-per-kilogram-dry-air",		/* 149 */
   "megajoules-per-kilogram-dry-air",		/* 150 */
   "kilojoules-per-degree-Kelvin",			/* 151 */
   "megajoules-per-degree-Kelvin",			/* 152 */
   "newton",								/* 153 */
   "grams-per-second",						/* 154 */
   "grams-per-minute",						/* 155 */
   "tons-per-hour",							/* 156 */
   "kilo-btus-per-hour",					/* 157 */
   "Hundredths-seconds",					/* 158 */
   "milliseconds",							/* 159 */
   "newton-meters",							/* 160 */
   "millimeters-per-second",				/* 161 */
   "millimeters-per-minute", 				/* 162 */
   "meters-per-minute",						/* 163 */
   "meters-per-hour",						/* 164 */
   "cubic-meters-per-minute",				/* 165 */
   "meters-per-second-per-second",			/* 166 */
   "amperes-per-meter",						/* 167 */
   "amperes-per-square-meter",				/* 168 */
   "ampere-square-meters",					/* 169 */
   "farads",								/* 170 */
   "henrys",								/* 171 */
   "ohm-meters",							/* 172 */
   "siemens",								/* 173 */
   "siemens-per-meter",						/* 174 */
   "teslas",								/* 175 */
   "volts-per-degree-Kelvin",				/* 176 */
   "volts-per-meter",						/* 177 */
   "webers",								/* 178 */
   "candelas",								/* 179 */
   "candelas-per-square-meter",				/* 180 */
   "degrees-Kelvin-per-hour",				/* 181 */
   "degrees-Kelvin-per-minute",				/* 182 */
	"joule-seconds",						/* 183 */
	"radians-per-second",					/* 184 */
	"square-meters-per-Newton",				/* 185 */
	"kilograms-per-cubic-meter",			/* 186 */
	"newton-seconds",						/* 187 */
	"newtons-per-meter",					/* 188 */
	"watts-per-meter-per-degree-Kelvin",	/* 189 last definition in 135-2008 */
	// Added by Addenda H (135-2004)
	"micro-siemens",                        /* 190 */
	"cubic-feet-per-hour",                  /* 191 */
	"us-gallons-per-hour",                  /* 192 */
};
BAC_STRINGTABLE(BACnetEngineeringUnits);

STRING_TABLE BACnetError[] = {
/* Alarm and Event Services */
   "AcknowledgeAlarm Error Choice",
   "ConfirmedCOVNotification Error Choice",
   "ConfirmedEventNotification Error Choice",
   "GetAlarmSummary Error Choice",
   "GetEnrollmentSummary Error Choice",
   "SubscribeCOV Error Choice",

/* File Access Services */
   "AtomicReadFile Error Choice",
   "AtomicWriteFile Error Choice",

/* Object Access Services */
   "AddListElement Error Choice",
   "RemoveListElement Error Choice",
   "CreateObject Error Choice",
   "DeleteObject Error Choice",
   "ReadProperty Error Choice",
   "ReadPropertyConditional Error Choice",
   "ReadPropertyMultiple Error Choice",
   "WriteProperty Error Choice",
   "WritePropertyMultiple Error Choice",

/* Remote Device Management Services */
   "DeviceCommunicationControl Error Choice",
   "ConfirmedPrivateTransfer Error Choice",
   "ConfirmedTextMessage Error Choice",
   "ReinitializeDevice Error Choice",

/* Virtual Terminal Services */
   "VT-Open Error Choice",
   "VT-Close Error Choice",
   "VT-Data Error Choice",

/* Security Services */
   "Authenticate Error Choice",
   "RequestKey Error Choice",

/* services added after 1995 */
	"ReadRange Error Choice",
	"Life Safety Operation Error Choice",
	"SubscribeCOVProperty Error Choice",
	"GetEventInformation Error Choice",
};
BAC_STRINGTABLE(BACnetError);

STRING_TABLE BACnetErrorClass[] = {
   "Device",
   "Object",
   "Property",
   "Resources",
   "Security",
   "Services",
   "VT",
   "Communication",
};
BAC_STRINGTABLE(BACnetErrorClass);

STRING_TABLE BACnetErrorCode[] = {
   "Other",                              /* 0 */
   "Authentication-failed",              /* 1 */
   "Configuration-in-progress",          /* 2 */
   "Device-busy",                        /* 3 */
   "Dynamic-creation-not-supported",     /* 4 */
   "File-access-denied",                 /* 5 */
   "Incompatible-security-levels",       /* 6 */
   "Inconsistent-parameters",            /* 7 */
   "Inconsistent-selection-criterion",   /* 8 */
   "Invalid-data-type",                  /* 9 */
   "Invalid-file-access-method",         /* 10 */
   "Invalid-file-start-position",        /* 11 */
   "Invalid-operator-name",              /* 12 */
   "Invalid-parameter-data-type",        /* 13 */
   "Invalid-time-stamp",                 /* 14 */
   "Key-generation-error",               /* 15 */
   "Missing-required-parameter",         /* 16 */
   "No-objects-of-specified-type",       /* 17 */
   "No-space-for-object",                /* 18 */
   "No-space-to-add-list-element",       /* 19 */
   "No-space-to-write-property",         /* 20 */
   "No-vt-sessions-available",           /* 21 */
   "Property-is-not-a-list",             /* 22 */
   "Object-deletion-not-permitted",      /* 23 */
   "Object-identifier-already-exists",   /* 24 */
   "Operational-problem",                /* 25 */
   "Password-failure",                   /* 26 */
   "Read-access-denied",                 /* 27 */
   "Security-not-supported",             /* 28 */
   "Service-request-denied",             /* 29 */
   "Timeout",                            /* 30 */
   "Unknown-object",                     /* 31 */
   "Unknown-property",                   /* 32 */
   "Invalid Enumeration",                /* 33 */
   "Unknown-vt-class",                   /* 34 */
   "Unknown-vt-session",                 /* 35 */
   "Unsupported-object-type",            /* 36 */
   "Value-out-of-range",                 /* 37 */
   "VT-session-already-closed",          /* 38 */
   "VT-session-termination-failure",     /* 39 */
   "Write-access-denied",                /* 40 */
   "Character-set-not-supported",        /* 41 */
   "Invalid-array-index",                /* 42 */
   "Cov-subscription-failed",            /* 43 kare.sars@wapice.com */
   "Not-cov-property",                   /* 44 | */
   "Optional-functionality-not-supported",/*45 | */
   "Invalid-configuration-data",         /* 46 | */
   "Datatype-not-supported",             /* 47 | */
   "Duplicate-name",                     /* 48 | */
   "Duplicate-object-id",                /* 49 | */
   "Property-is-not-an-array",           /* 50 kare.sars@wapice.com */
   // Added by Addenda B PPR3 (135-2004)
   "abort-buffer-overflow",              /* 51 */
   "abort-invalid-apdu-in-this-state",
   "abort-preempted-by-higher-priority-task",
   "abort-segmentation-not-supported",
   "abort-proprietary",
   "abort-other",                        /* 56 */
   "invalid-tag",
   "network-down",
   "reject-buffer-overflow",
   "reject-inconsistent-parameters",
   "reject-invalid-parameter-data-type",
   "reject-invalid-tag",                 /* 62 */
   "reject-missing-required-parameter",
   "reject-parameter-out-of-range",
   "reject-too-many-arguments",
   "reject-undefined-enumeration",
   "reject-unrecognized-service",        /* 67 */
   "reject-proprietary",                 /* 68 */
   "reject-other",
   "unknown-device",
   "unknown-route",
   "value-not-totalized",                /* 72 */
   // Added by Addenda D (135-2004)
   "invalid-event-state",                /* 73 */
   "no-alarm-configured",                /* 74 */
   // added by Addenda B
   "log-buffer-full",                    // 75
   "logged-value-purged",                // 76
   "no-property-specified",              // 77
   "not-configured-for-triggered-logging", // 78
   // added by Addenda H (135-2004)
   "unknown-subscription",	             // 79
   "error-80",                           // reserved in 135-2008
   "error-81",
   "error-82",
   "communication-disabled",	         // 83 last value in 135-2008
};
BAC_STRINGTABLE(BACnetErrorCode);

// Added Addendum B (135-2004)
STRING_TABLE BACnetEventLogRecord[] = {
	"timestamp",
	"logDatum",
	"log-status",
	"notification",
	"time-change"
};
BAC_STRINGTABLE(BACnetEventLogRecord);

//Modified by Zhu Zhenhua, 2004-5-20
STRING_TABLE BACnetEventParameter[] = {
   "Change of Bitstring",
   "Change of State",
   "Change of Value",
   "Command Failure",
   "Floating Limit",
   "Out of Range",
   "Complex",	
   "Deprecated",
   "Change of Life Safety",
   "Extended",
   "Buffer Ready",
   "Unsigned Range",
   "Change of status-flags",
   };
BAC_STRINGTABLE(BACnetEventParameter);

STRING_TABLE BACnetEventState[] = {
   "NORMAL",
   "FAULT",
   "OFFNORMAL",
   "HIGH-LIMIT",
   "LOW-LIMIT",
   "LIFE-SAFETY-ALARM",
   };
BAC_STRINGTABLE(BACnetEventState);

STRING_TABLE BACnetEventTransitionBits[] = {
   "TO-OFFNORMAL",
   "TO-FAULT",
   "TO-NORMAL",
   };
BAC_STRINGTABLE(BACnetEventTransitionBits);

//Modified by Zhu Zhenhua, 2004-5-17
STRING_TABLE BACnetEventType[] = {
   "CHANGE-OF-BITSTRING",
   "CHANGE-OF-STATE",
   "CHANGE-OF-VALUE",
   "COMMAND-FAILURE",
   "FLOATING-LIMIT",
   "OUT-OF-RANGE",
   "COMPLEX-EVENT-TYPE",   
   "DEPRECATED",            
   "CHANGE-OF-LIFE-SAFETY", 
   "EXTENDED",			
   "BUFFER-READY",
   "UNSIGNED-RANGE",
   "CHANGE-OF-STATUS-FLAGS",
   };
BAC_STRINGTABLE(BACnetEventType);

STRING_TABLE Acknowledgement_Filter[] = {
   "all",
   "acked",
   "not-acked"
};
BAC_STRINGTABLE(Acknowledgement_Filter);

STRING_TABLE EventState_Filter[] = {
   "offnormal",
   "fault",
   "normal",
   "all",
   "active"
};
BAC_STRINGTABLE(EventState_Filter);

STRING_TABLE BACnetFileAccessMethod[] = {
   "RECORD-ACCESS",
   "STREAM-ACCESS",
   "RECORD-AND-STREAM-ACCESS"	// note removed after 1995
};
BAC_STRINGTABLE(BACnetFileAccessMethod);

//Added by Zhu Zhenhua, 2004-5-25
STRING_TABLE BACnetEventSummary[] = {
   "Object Indentifier",
   "Event State",
   "Acknowledged Transitions",
   "Event Time Stamps",
   "Notify Type",
   "Event Enable",
   "Event Priorities"	
};
BAC_STRINGTABLE(BACnetEventSummary);

///////////////////////////////////////////////////////////////////////////
//Added by Zhu Zhenhua, 2004-6-14
STRING_TABLE BACnetLifeSafetyMode[] = {
   "off",
   "on",
   "test",
   "manned",
   "unmanned",
   "armed",
   "disarmed",
   "prearmed",
   "slow",
   "fast",
   "disconnected",
   "enabled",
   "disabled",
   "automatic-release-disabled",
   "default"
};
BAC_STRINGTABLE(BACnetLifeSafetyMode);

STRING_TABLE BACnetLifeSafetyOperation[] = {
   "none",
   "silence",
   "silence-audible",
   "silence-visual",
   "reset",
   "reset-alarm",
   "reset-fault",
   "unsilence",
   "unsilence-audible",
   "unsilence-visual",
};
BAC_STRINGTABLE(BACnetLifeSafetyOperation);

STRING_TABLE BACnetLifeSafetyState[] = {
   "quiet",
   "pre-alarm",
   "alarm",
   "fault",
   "fault-pre-alarm",
   "fault-alarm",
   "not-ready",
   "active",
   "tamper",
   "test-alarm",
   "test-active",
   "test-fault",
   "test-fault-alarm",
   "holdup",
   "duress",
   "tamper-alarm",
   "abnormal",
   "emergency-power",
   "delayed",
   "blocked",
   "local-alarm",
   "general-alarm",
   "supervisory",
   "test-supervisory"
};
BAC_STRINGTABLE(BACnetLifeSafetyState);

STRING_TABLE BACnetAccumulatorStatus[] = {
	"normal",
	"starting",
	"recovered",
	"abnormal",
    "failed"
};
BAC_STRINGTABLE(BACnetAccumulatorStatus);

STRING_TABLE  BACnetMaintenance[] = {
	"None",
	"Periodic-test",
	"Need-Service-Operational",
	"Need-Service-Inoperative"
};
BAC_STRINGTABLE(BACnetMaintenance);

STRING_TABLE  BACnetSilencedState[] = {
	"Unsilenced",
	"Audible-Silenced",
	"Visible-Silenced",
	"All-Silenced"
};
BAC_STRINGTABLE(BACnetSilencedState);

STRING_TABLE BACnetLimitEnable[] = {
   "LOW-LIMIT-ENABLE",
   "HIGH-LIMIT-ENABLE"
   };
BAC_STRINGTABLE(BACnetLimitEnable);

STRING_TABLE BACnetLockStatus[] = {
	"locked",
	"unlocked",
	"fault",
	"unknown",
};
BAC_STRINGTABLE(BACnetLockStatus);

STRING_TABLE BACnetLogData[] = {
	"Log-status",
	"Log-Data",
	"Time-Change",
};
BAC_STRINGTABLE(BACnetLogData);

STRING_TABLE BACnetLoggingType[] = {
	"polled",
	"cov",
	"triggered",
};
BAC_STRINGTABLE(BACnetLoggingType);

STRING_TABLE BACnetLogMultipleRecord[] = {
	"timestamp",
	"logData",
};
BAC_STRINGTABLE(BACnetLogMultipleRecord);

STRING_TABLE BACnetLogRecord [] = {
   "TimeStamp",
   "LogDatum", 
   "StatusFlags"
   };
BAC_STRINGTABLE(BACnetLogRecord);

STRING_TABLE BACnetLogStatus[] = {
	"log-disabled",
	"buffer-purged",
	"log-interrupted",
};
BAC_STRINGTABLE(BACnetLogStatus);

STRING_TABLE BACnetNotifyType[] = {
   "ALARM",
   "EVENT",
   "ACK_NOTIFICATION"
   };
BAC_STRINGTABLE(BACnetNotifyType);

// Added addendum B (135-2004)
STRING_TABLE BACnetPropertyAccessResult[] = {
	"Object Identifier",
	"Property Identifier",
	"Property Array Index",
	"Device Identifier",
	"Access Result",
};
BAC_STRINGTABLE(BACnetPropertyAccessResult);

// Added from 135-2008
STRING_TABLE BACnetShedLevel[] = {
	"Percent",
	"Level",
	"Amount",
};
BAC_STRINGTABLE(BACnetShedLevel);

STRING_TABLE BACnetShedState[] = {
	"shed-inactive",
	"shed-request-pending",
	"shed-compliant",
	"shed-non-compliant",
};
BAC_STRINGTABLE(BACnetShedState);

STRING_TABLE BACnetReadRangeACK[] = {
   "Object Identifier",
   "Property Identifier",
   "Property Array Index",
   "ResultsFlag",
   "ItemCount",
   "ItemData",
   "First Sequence Number",
};
BAC_STRINGTABLE(BACnetReadRangeACK);

STRING_TABLE BACnetReadRangeRequest[] = {
   "Object Identifier",
   "Property Identifier",
   "Property Array Index",
   "Reference Index",
   "Reference Time",
   "TimeRange"
};
BAC_STRINGTABLE(BACnetReadRangeRequest);

STRING_TABLE BACnetObjectPropertyValue[] = {
   "Object Identifier",
   "Property Identifier",
   "Property Array Index",
   "Value",
   "Priority"
};
BAC_STRINGTABLE(BACnetObjectPropertyValue);

STRING_TABLE BACnetObjectType[] = {
   "ANALOG-INPUT",          /* 0 */
   "ANALOG-OUTPUT",         /* 1 */
   "ANALOG-VALUE",          /* 2 */
   "BINARY-INPUT",          /* 3 */
   "BINARY-OUTPUT",         /* 4 */
   "BINARY-VALUE",          /* 5 */
   "CALENDAR",              /* 6 */
   "COMMAND",               /* 7 */
   "DEVICE",                /* 8 */
   "EVENT-ENROLLMENT",      /* 9 */
   "FILE",                  /* 10 */
   "GROUP",                 /* 11 */
   "LOOP",                  /* 12 */
   "MULTISTATE-INPUT",      /* 13 */
   "MULTISTATE-OUTPUT",     /* 14 */
   "NOTIFICATION-CLASS",    /* 15 */
   "PROGRAM",               /* 16 */
   "SCHEDULE",              /* 17 */
   "AVERAGING",             /* 18 */
   "MULTISTATE-VALUE",      /* 19 */
   "TREND-LOG" ,            /* 20 */		// msdanner 9/04, was "TRENDLOG"
   "LIFE-SAFETY-POINT",	    /* 21 Zhu Zhenhua 2003-7-24 */	 // msdanner 9/04, was "LIFESAFETYPOINT"
   "LIFE-SAFETY-ZONE",	    /* 22 Zhu Zhenhua 2003-7-24 */  // msdanner 9/04, was "LIFESAFETYZONE"
   "ACCUMULATOR",           // 23 Shiyuan Xiao 7/15/2005
   "PULSE-CONVERTER",       // 24 Shiyuan Xiao 7/15/2005
   "EVENT-LOG",			  // 25	- Addendum B
   "GLOBAL-GROUP",		  // 26 - Addendum B
   "TREND-LOG-MULTIPLE",  // 27 - Addendum B
   "LOAD-CONTROL",		  // 28 - Addendum E 135-2004
   "STRUCTURED-VIEW",     // 29 - Addendum D
   "ACCESS-DOOR",		  // 30 Last in 135-2008
	"LIGHTING-OUTPUT",
	"ACCESS-CREDENTIAL",	// Addendum 2008-j
	"ACCESS-POINT",
	"ACCESS-RIGHTS",
	"ACCESS-USER",
	"ACCESS-ZONE",
	"CREDENTIAL-DATA-INPUT",
	"NETWORK-SECURITY",		// 38 Addendum 2008-g
	"BITSTRING-VALUE",		// Addendum 2008-w
	"CHARACTERSTRING-VALUE",
	"DATE-PATTERN-VALUE",
	"DATE-VALUE",
	"DATETIME-PATTERN-VALUE",
	"DATETIME-VALUE",
	"INTEGER-VALUE",
	"LARGE-ANALOG-VALUE",
	"OCTETSTRING-VALUE",
	"POSITIVE-INTEGER-VALUE",
	"TIME-PATTERN-VALUE",
	"TIME-VALUE"		// 50 Last in 2008-w

   // CAUTION: if you add a type here, you must also change MAX_DEFINED_OBJ
   // (which is actually max-plus-one: the NUMBER of defined object types)
};
BAC_STRINGTABLE(BACnetObjectType);


STRING_TABLE BACnetObjectTypesSupported[] = {
   "ANALOG-INPUT",          /* 0 */
   "ANALOG-OUTPUT",         /* 1 */
   "ANALOG-VALUE",          /* 2 */
   "BINARY-INPUT",          /* 3 */
   "BINARY-OUTPUT",         /* 4 */
   "BINARY-VALUE",          /* 5 */
   "CALENDAR",              /* 6 */
   "COMMAND",               /* 7 */
   "DEVICE",                /* 8 */
   "EVENT-ENROLLMENT",      /* 9 */
   "FILE",                  /* 10 */
   "GROUP",                 /* 11 */
   "LOOP",                  /* 12 */
   "MULTISTATE-INPUT",      /* 13 */
   "MULTISTATE-OUTPUT",     /* 14 */
   "NOTIFICATION-CLASS",    /* 15 */
   "PROGRAM",               /* 16 */
   "SCHEDULE",              /* 17 */
   "AVERAGING",             /* 18 */
   "MULTISTATE-VALUE",      /* 19 */
   "TREND-LOG",             /* 20 */	  // msdanner 9/04, was "TRENDLOG"
   "LIFE-SAFETY-POINT",     /* 21 */	  // msdanner 9/04, added
   "LIFE-SAFETY-ZONE",      /* 22 */	  // msdanner 9/04, added.  JLH: 1/25/2010 added missing comma
   "ACCUMULATOR",           //23 Shiyuan Xiao 7/15/2005
   "PULSE-CONVERTER",       //24 Shiyuan Xiao 7/15/2005
   "EVENT-LOG",			  // 25	- Addendum B
   "GLOBAL-GROUP",		  // 26 - Addendum B
   "TREND-LOG-MULTIPLE",  // 27 - Addendum B
   "LOAD-CONTROL",		  // 28 - Addendum E 135-2004
   "STRUCTURED-VIEW",     // 29 - Addendum D
   "ACCESS-DOOR",			// 30 Last in 135-2008
	"LIGHTING-OUTPUT",
	"ACCESS-CREDENTIAL",
	"ACCESS-POINT",			// Addendum 2008-j
	"ACCESS-RIGHTS",
	"ACCESS-USER",
	"ACCESS-ZONE",
	"CREDENTIAL-DATA-INPUT",
	"NETWORK-SECURITY",		// 38 Addendum 2008-g
	"BITSTRING-VALUE",		// Addendum 2008-w
	"CHARACTERSTRING-VALUE",
	"DATE-PATTERN-VALUE",
	"DATE-VALUE",
	"DATETIME-PATTERN-VALUE",
	"DATETIME-VALUE",
	"INTEGER-VALUE",
	"LARGE-ANALOG-VALUE",
	"OCTETSTRING-VALUE",
	"POSITIVE-INTEGER-VALUE",
	"TIME-PATTERN-VALUE",
	"TIME-VALUE"		// 50 Last in 2008-w

};
BAC_STRINGTABLE(BACnetObjectTypesSupported);

STRING_TABLE BACnetPolarity[] = {
   "NORMAL",
   "REVERSE"
};
BAC_STRINGTABLE(BACnetPolarity);

STRING_TABLE BACnetPrescale[] = {
	"multiplier",
	"moduleDivide",
};
BAC_STRINGTABLE(BACnetPrescale);

STRING_TABLE BACnetProgramError[] = {
   "NORMAL",
   "LOAD-FAILED",
   "INTERNAL",
   "PROGRAM",
   "OTHER"
};
BAC_STRINGTABLE(BACnetProgramError);

STRING_TABLE BACnetProgramRequest[] = {
   "READY",
   "LOAD",
   "RUN",
   "HALT",
   "RESTART",
   "UNLOAD"
};
BAC_STRINGTABLE(BACnetProgramRequest);

STRING_TABLE BACnetProgramState[] = {
   "IDLE",
   "LOADING",
   "RUNNING",
   "WAITING",
   "HALTED",
   "UNLOADING"
};   
BAC_STRINGTABLE(BACnetProgramState);

// MAX_PROP_ID = the number of elements in this array. It is located in Vts.h
// TODO: Do these REALLY need spaces after them?
STRING_TABLE BACnetPropertyIdentifier[] = {
   "Acked_Transitions ",                /* 0 */
   "Ack_Required ",                     /* 1 */
   "Action ",                           /* 2 */
   "Action_Text ",                      /* 3 */
   "Active_Text ",                      /* 4 */
   "Active_VT_Sessions ",               /* 5 */
   "Alarm_Value ",                      /* 6 */
   "Alarm_Values ",                     /* 7 */
   "All ",                              /* 8 */
   "All_Writes_Successful ",            /* 9 */
   "APDU_Segment_Timeout ",             /* 10 */
   "APDU_Timeout ",                     /* 11 */
   "Application-software-version ",     /* 12 */
   "Archive ",                          /* 13 */
   "Bias ",                             /* 14 */
   "Change_Of_State_Count ",            /* 15 */
   "Change_Of_State_Time ",             /* 16 */
   "Notification_Class ",               /* 17  renamed in 2nd public review*/
   "Invalid Enumeration",               /* 18 */
   "Controlled_Variable_Reference ",    /* 19 */
   "Controlled_Variable_Units ",        /* 20 */
   "Controlled_Variable_Value ",        /* 21 */
   "COV_Increment ",                    /* 22 */
   "Date-list ",                        /* 23 */
   "Daylight_Savings_Status ",          /* 24 */
   "Deadband ",                         /* 25 */
   "Derivative_Constant ",              /* 26 */
   "Derivative_Constant_Units ",        /* 27 */
   "Description ",                      /* 28 */
   "Description_Of_Halt ",              /* 29 */
   "Device_Address_Binding ",           /* 30 */
   "Device_Type ",                      /* 31 */
   "Effective_Period ",                 /* 32 */
   "Elapsed_Active_Time ",              /* 33 */
   "Error_Limit ",                      /* 34 */
   "Event_Enable ",                     /* 35 */
   "Event_State ",                      /* 36 */
   "Event_Type ",                       /* 37 */
   "Exception_Schedule ",               /* 38 */
   "Fault_Values ",                     /* 39 */
   "Feedback_Value ",                   /* 40 */
   "File_Access_Method ",               /* 41 */
   "File_Size ",                        /* 42 */
   "File_Type ",                        /* 43 */
   "Firmware_Revision ",                /* 44 */
   "High_Limit ",                       /* 45 */
   "Inactive_Text ",                    /* 46 */
   "In_Process ",                       /* 47 */
   "Instance_Of ",                      /* 48 */
   "Integral_Constant ",                /* 49 */
   "Integral_Constant_Units ",          /* 50 */
   "unused-was-Issue_Confirmed_Notifications ",    /* 51 deleted in version 1 revision 4*/
   "Limit_Enable ",                     /* 52 */
   "List_Of_Group_Members ",            /* 53 */
   "List_Of_Object_Property_References ",  /* 54 Zhu Zhenhua 2003-7-24 */
   "List_Of_Session_Keys ",             /* 55 */
   "Local_Date ",                       /* 56 */
   "Local_Time ",                       /* 57 */
   "Location ",                         /* 58 */
   "Low_Limit ",                        /* 59 */
   "Manipulated_Variable_Reference ",   /* 60 */
   "Maximum_Output ",                   /* 61 */
   "Max_Apdu_Length_Accepted ",         /* 62 */
   "Max_Info_Frames ",                  /* 63 */
   "Max_Master ",                       /* 64 */
   "Max_Pres_Value ",                   /* 65 */
   "Minimum_Off_Time ",                 /* 66 */
   "Minimum_On_Time ",                  /* 67 */
   "Minimum_Output ",                   /* 68 */
   "Min_Pres_Value ",                   /* 69 */
   "Model_Name ",                       /* 70 */
   "Modification_Date ",                /* 71 */
   "Notify_Type ",                      /* 72 */
   "Number_Of_APDU_Retries",            /* 73 */
   "Number_Of_States ",                 /* 74 */
   "Object_Identifier ",                /* 75 */
   "Object_List ",                      /* 76 */
   "Object_Name ",                      /* 77 */
   "Object_Property_Reference ",        /* 78 Zhu Zhenhua 2003-7-24 */
   "Object_Type ",                      /* 79 */
   "Optional ",                         /* 80 */
   "Out_Of_Service ",                   /* 81 */
   "Output_Units ",                     /* 82 */
   "Event-Parameters ",                 /* 83 */
   "Polarity ",                         /* 84 */
   "Present_Value ",                    /* 85 */
   "Priority ",                         /* 86 */
   "Priority_Array ",                   /* 87 */
   "Priority_For_Writing ",             /* 88 */
   "Process_Identifier ",               /* 89 */
   "Program_Change ",                   /* 90 */
   "Program_Location ",                 /* 91 */
   "Program_State ",                    /* 92 */
   "Proportional_Constant ",            /* 93 */
   "Proportional_Constant_Units ",      /* 94 */
   "Protocol_Conformance_Class ",       /* 95 */
   "Protocol_Object_Types_Supported ",  /* 96 */
   "Protocol_Services_Supported ",      /* 97 */
   "Protocol_Version ",                 /* 98 */
   "Read_Only ",                        /* 99 */
   "Reason_For_Halt ",                  /* 100 */
   "Recipient ",                        /* 101 */
   "Recipient_List ",                   /* 102 */
   "Reliability ",                      /* 103 */
   "Relinquish_Default ",               /* 104 */
   "Required ",                         /* 105 */
   "Resolution ",                       /* 106 */
   "Segmentation_Supported ",           /* 107 */
   "Setpoint ",                         /* 108 */
   "Setpoint_Reference ",               /* 109 */
   "State_Text ",                       /* 110 */
   "Status_Flags ",                     /* 111 */
   "System_Status ",                    /* 112 */
   "Time_Delay ",                       /* 113 */
   "Time_Of_Active_Time_Reset ",        /* 114 */
   "Time_Of_State_Count_Reset ",        /* 115 */
   "Time_Synchronization_Recipients ",  /* 116 */
   "Units ",                            /* 117 */
   "Update_Interval ",                  /* 118 */
   "UTC_Offset ",                       /* 119 */
   "Vendor_Identifier ",                /* 120 */
   "Vendor_Name ",                      /* 121 */
   "Vt_Classes_Supported ",             /* 122 */
   "Weekly_Schedule ",                  /* 123 */   
   "Attempted_Samples ",                /* 124 */
   "Average_Value ",                    /* 125 */
   "Buffer_Size ",                      /* 126 */
   "Client_Cov_Increment ",             /* 127 */
   "Cov_Resubscription_Interval ",      /* 128 */
   "unused-was-Current_Notify_Time",    /* 129  Added by Zhu Zhenhua, 2004-5-11 deleded in version 1 rev 3 */
   "Event_Time_Stamps ",                /* 130 */
   "Log_Buffer ",                       /* 131 */
   "Log_Device_Object_Property ",       /* 132 Zhu Zhenhua 2003-7-24 */
   "Enable ",                           /* 133 changed from Log_Enable in 135-2004b-5 */
   "Log_Interval ",                     /* 134 */
   "Maximum_Value ",                    /* 135 */
   "Minimum_Value ",                    /* 136 */
   "Notification_Threshold ",           /* 137 */
   "unused-was-Previous_Notify_Time",   /* 138   Added by Zhu Zhenhua, 2004-5-11 deleted in version 1 rev 3 */
   "Protocol_Revision ",                /* 139 */
   "Records_Since_Notification ",       /* 140 */
   "Record_Count ",                     /* 141 */
   "Start_Time ",                       /* 142 */
   "Stop_Time ",                        /* 143 */
   "Stop_When_Full ",                   /* 144 */
   "Total_Record_Count ",               /* 145 */            
   "Valid_Samples ",                    /* 146 */
   "Window_Interval ",                  /* 147 */
   "Window_Samples ",                   /* 148 */
   "Maximum_Value_Timestamp ",          /* 149 */
   "Minimum_value_Timestamp ",          /* 150 */
   "Variance_Value ",                   /* 151 */
   "Active_Cov_Subscription",           /* 152 Xiao Shiyuan 2002-7-18 */
   "backup-failure-timeout",            /* 153 Xiao Shiyuan 2002-7-18 */		
   "configuration-files",               /* 154 Xiao Shiyuan 2002-7-18 */
   "database-revision",                 /* 155 Xiao Shiyuan 2002-7-18 */
   "direct-reading",                    /* 156 Xiao Shiyuan 2002-7-18 */
   "last-restore-time",					/* 157 Xiao Shiyuan 2002-7-18 */
   "maintenance-required",				/* 158 Xiao Shiyuan 2002-7-18 */
   "member-of",							/* 159 Xiao Shiyuan 2002-7-18 */
   "mode",								/* 160 Xiao Shiyuan 2002-7-18 */
   "operation-expected",				/* 161 Xiao Shiyuan 2002-7-18 */
   "setting",							/* 162 Xiao Shiyuan 2002-7-18 */
   "silenced",							/* 163 Xiao Shiyuan 2002-7-18 */
   "tracking-value",					/* 164 Xiao Shiyuan 2002-7-18 */
   "zone-members",						/* 165 Xiao Shiyuan 2002-7-18 */
   "life-safety-alarm-values",			/* 166 Xiao Shiyuan 2002-7-18 */
   "max-segments-accepted",				/* 167 Xiao Shiyuan 2002-7-18 */
   "Profile_Name",                      /* 168 Xiao Shiyuan 2002-7-18 */
   "auto-slave-discovery",				/* 169 LJT 2005-10-12   */
   "manual-slave-address-binding",		/* 170 LJT 2005-10-12   */
   "slave-address-binding",				/* 171 LJT 2005-10-12   */
   "slave-proxy-enable",				/* 172 LJT 2005-10-12   */
   "last_notify_record",				/* 173 Zhu Zhenhua  2004-5-11 */
   "Schedule_Default",                 // 174 Shiyuan Xiao 7/15/2005
   "Accepted_Modes",                   // 175 Shiyuan Xiao 7/15/2005
   "Adjust_Value",                     // 176 Shiyuan Xiao 7/15/2005
   "Count",                            // 177 Shiyuan Xiao 7/15/2005 
   "Count_Before_Change",              // 178 Shiyuan Xiao 7/15/2005
   "Count_Change_Time",                // 179 Shiyuan Xiao 7/15/2005		
   "Cov_Period",                       // 180 Shiyuan Xiao 7/15/2005
   "Input_Reference",                  // 181 Shiyuan Xiao 7/15/2005
   "Limit_Monitoring_Interval",        // 182 Shiyuan Xiao 7/15/2005
   "Logging_Device",                   // 183 Shiyuan Xiao 7/15/2005
   "Logging_Record",                   // 184 Shiyuan Xiao 7/15/2005  
   "Prescale",                         // 185 Shiyuan Xiao 7/15/2005  
   "Pulse_Rate",                       // 186 Shiyuan Xiao 7/15/2005
   "Scale",                            // 187 Shiyuan Xiao 7/15/2005
   "Scale_Factor",                     // 188 Shiyuan Xiao 7/15/2005  
   "Update_Time",                      // 189 Shiyuan Xiao 7/15/2005 
   "Value_Before_Change",              // 190 Shiyuan Xiao 7/15/2005
   "Value_Set",                        // 191 Shiyuan Xiao 7/15/2005
   "Value_Change_Time",                 // 192 Shiyuan Xiao 7/15/2005
   // added Addendum B (135-2004)
	"Align_Intervals",					// 193
	"Group_Members_Names",				// 194
	"Interval_Offset",					// 195
	"Last_Restart_Reason",				// 196
	"Logging_Type",						// 197
	"Member_Status_Flags",				// 198
	"Notification_Period",				// 199
	"Previous_Notify_Record",			// 200
	"Requested_Update_Interval",		// 201
	"Restart_Notification_Recipients",	// 202
	"Time_Of_Device_Restart",			// 203
	"Time_Synchronization_Interval",	// 204
	"Trigger",							// 205
	"Utc_Time_Syncrhonization_Recipients",  // 206
	// Added by addenda D
	"node-subtype", // 207
	"node-type",   // 208
	"structured-object-list",  // 209
	"subordinate-annotation",  // 210
	"subordinate-list", // 211
	// added by addendum E 135-2004
	"actual-shed-level",	// 212
	"duty-window",			// 213
	"expected-shed-level",	// 214
	"full-duty-baseline",	// 215
	"unknown-216",
	"unknown-217",
	"requested-shed-level",	// 218
	"shed-duration",		// 219
	"shed-level-descriptions", // 220
	"shed-levels",			// 221
	"state-description",	// 222
	"unknown-223",
	"unknown-224",
	"unknown-225",
	"door-alarm-state",		// 226
	"door-extended-pulse-time",
	"door-members",
	"door-open-too-long-time",
	"door-pulse-time",		// 230
	"door-status",
	"door-unlock-delay-time",
	"lock-status",
	"masked-alarm-values",
	"secured-status",		// 235 last in 135-2008
	// CAUTION: if you add a property here, you must also change MAX_PROPID
	// (which is actually max-plus-one: the NUMBER of defined properties)
};
BAC_STRINGTABLE(BACnetPropertyIdentifier);

// Added by Addenda D
STRING_TABLE BACnetNodeType[] = {
   "UNKNOWN",
   "SYSTEM",
   "NETWORK",
   "DEVICE",
   "ORGANIZATION",
   "AREA",
   "EQUIPMENT",
   "POINT",
   "COLLECTION",
   "PROPERTY",
   "FUNCTIONAL",
   "OTHER"
};
BAC_STRINGTABLE(BACnetNodeType);
   
STRING_TABLE BACnetPropertyReference[] = {
   "Property Identifier",
   "Property Array Index"
};
BAC_STRINGTABLE(BACnetPropertyReference);

STRING_TABLE BACnetPropertyStates[] = {
   "Boolean-value",
   "Binary-value",
   "Event-type",
   "Polarity",
   "Program-change",
   "Program-state",
   "Reason-for-halt",
   "Reliability",
   "State",
   "System-status",
   "Units",
   "Unsigned-value",
   "Life-safety-mode",
   "Life-safety-state",
   "Restart-reason",
   "Door-alarm-state",
};
BAC_STRINGTABLE(BACnetPropertyStates);

STRING_TABLE BACnetPropertyValue[] = {
   "Property Identifier",
   "Property Array Index",
   "Value",
   "Priority"
};
BAC_STRINGTABLE(BACnetPropertyValue);

STRING_TABLE BACnetRecipient[] = {
   "Device",
   "Address"
};
BAC_STRINGTABLE(BACnetRecipient);

STRING_TABLE BACnetRecipientProcess[] = {
   "Recipient",
   "Process Identifier"
};
BAC_STRINGTABLE(BACnetRecipientProcess);

STRING_TABLE BACnetReliability[] = {
   "NO-FAULT-DETECTED",
   "NO-SENSOR",
   "OVER-RANGE",
   "UNDER-RANGE",
   "OPEN-LOOP",
   "SHORTED-LOOP",
   "NO-OUTPUT",
   "UNRELIABLE-OTHER",
   "PROCESS-ERROR",
   "MULTI-STATE-FAULT",
   "CONFIGURATION-ERROR", // 10
   // added addendum B (135-2004)
   "MEMBER-FAULT",
   "COMMUNICATION-FAILURE",
};
BAC_STRINGTABLE(BACnetReliability);

STRING_TABLE BACnetRestartReason[] = {
	"unknown",
	"coldstart",
	"warmstart",
	"detected-power-lost",
	"detected-power-off",
	"hardware-watchdog",
	"software-watchdog",
	"suspended",
};
BAC_STRINGTABLE(BACnetRestartReason);

STRING_TABLE BACnetSegmentation[] = {
   "SEGMENTED-BOTH",
   "SEGMENTED-TRANSMIT",
   "SEGMENTED-RECEIVE",
   "NO-SEGMENTATION"
};
BAC_STRINGTABLE(BACnetSegmentation);

STRING_TABLE BACnetServicesSupported[] = {
/* Alarm and Event Services */
   "AcknowledgeAlarm",              /* 0 */
   "ConfirmedCOVNotification",      /* 1 */
   "ConfirmedEventNotification",    /* 2 */
   "GetAlarmSummary",               /* 3 */
   "GetEnrollmentSummary",          /* 4 */
   "SubscribeCOV",                  /* 5 */

/* File Access Services */
   "AtomicReadFile",                /* 6 */
   "AtomicWriteFile",               /* 7 */

/* Object Access Services */
   "AddListElement",                /* 8 */
   "RemoveListElement",             /* 9 */
   "CreateObject",                  /* 10 */
   "DeleteObject",                  /* 11 */
   "ReadProperty",                  /* 12 */
   "ReadPropertyConditional",       /* 13 */
   "ReadPropertyMultiple",          /* 14 */
   "WriteProperty",                 /* 15 */
   "WritePropertyMultiple",         /* 16 */

/* Remote Device Management Services */
   "DeviceCommunicationControl",    /* 17 */
   "ConfirmedPrivateTransfer",      /* 18 */
   "ConfirmedTextMessage",          /* 19 */
   "ReinitializeDevice",            /* 20 */

/* Virtual Terminal Services */
   "VT-Open",                       /* 21 */
   "VT-Close",                      /* 22 */
   "VT-Data",                       /* 23 */

/*  Security Services */
   "Authenticate",                  /* 24 */
   "RequestKey",                    /* 25 */

/* Unconfirmed Services */
   "I-Am",                          /* 26 */
   "I-Have",                        /* 27 */
   "UnconfirmedCOVNotification",    /* 28 */
   "UnconfirmedEventNotification",  /* 29 */
   "UnconfirmedPrivateTransfer",    /* 30 */
   "UnconfirmedTextMessage",        /* 31 */
   "TimeSynchronization",           /* 32 */
   "Who-Has",                       /* 33 */
   "Who-Is",                        /* 34 */

/* Added after 1995 */
   "ReadRange",                     /* 35 */
   "UtcTimeSynchronization"  ,      /* 36 */
   "LifeSafetyOperation",           /* 37 */
   "SubscribeCOVProperty",          /* 38 */ 
   "GetEventInformation"            /* 39 */
};                       
BAC_STRINGTABLE(BACnetServicesSupported);

STRING_TABLE BACnetSessionKey[] = {
   "Session Key",
   "Peer Address"
};
BAC_STRINGTABLE(BACnetSessionKey);

STRING_TABLE BACnetSpecialEvent[] = {
   "Period (CalendarEntry)",
   "Period (CalendarReference)",
   "ListOfTimeValues",
   "EventPriority"
};
BAC_STRINGTABLE(BACnetSpecialEvent);

STRING_TABLE BACnetStatusFlags[] = {
   "IN-ALARM",
   "FAULT",
   "OVERRIDDEN",
   "OUT-OF-SERVICE"
};
BAC_STRINGTABLE(BACnetStatusFlags);

STRING_TABLE BACnetResultFlags[] = {
   "FIRST-ITEM",
   "LAST-ITEM",
   "MORE-ITEMS"
};
BAC_STRINGTABLE(BACnetResultFlags);

STRING_TABLE BACnetVendorID[] = {
   "ASHRAE",				// 0
   "NIST",
   "Trane",
   "McQuay",
   "PolarSoft",
   "Johnson Controls",		// 5
   "American Auto-Matrix",
   "Staefa",
   "Delta Controls",
   "Landis & Gyr",
   "Andover Controls",		// 10
   "Siebe",
   "Orion Analysis",
   "Teletrol",
   "Cimetrics Technology",
   "Cornell University",	// 15
   "Carrier",
   "Honeywell",
   "Alerton",
   "Tour & Andersson",
   "Hewlett-Packard",		// 20
   "Dorsette's Inc.",
   "Cerberus AG",
   "York",
   "Automated Logic",
   "Control Systems International",	// 25
   "Phoenix Controls Corporation",
   "Innovex",
   "KMC Controls",				// 28

   "Xn Technologies, Inc.",		//29
   "Hyundai Information Technology Co., Ltd.",    //30
   "Tokimec Inc.",				//31
   "Simplex",					//32
   "North Building Technologies Limited",    //33
   "Notifier",					//34
   "Reliable Controls Corporation",    //35
   "Tridium Inc.",				//36
   "Sierra Monitor Corp.",		//37
   "Silicon Energy",			//38
   "Kieback & Peter GmbH & Co KG",    //39
   "Anacon Systems, Inc.",		//40
   "Systems Controls & Instruments, LLC",    //41
   "Lithonia Lighting",			//42
   "Micropower Manufacturing",	//43
   "Matrix Controls",			//44
   "METALAIRE",					//45
   "ESS Engineering",			//46
   "Sphere Systems Pty Ltd.",   //47
   "Walker Technologies Corporation",    //48
   "H I Solutions, Inc.",		//49
   "MBS GmbH",					//50
   "SAMSON AG",    //51
   "Badger Meter Inc.",    //52
   "DAIKIN Industries Ltd.",    //53
   "NARA Controls Inc.",    //54
   "Mammoth Inc.",    //55
   "Liebert Corporation",    //56
   "SEMCO Incorporated",    //57
   "Air Monitor Corporation",    //58
   "TRIATEK, LLC",    //59
   "NexLight",    //60
   "Multistack",    //61
   "TSI Incorporated",    //62
   "Weather-Rite, Inc.",    //63
   "Dunham-Bush",    //64
   "Reliance Electric",    //65
   "LCS Inc.",    //66
   "Regulator Australia PTY Ltd.",    //67
   "Touch-Plate Lighting Controls",    //68
   "Amann GmbH",    //69
   "RLE Technologies",    //70
   "Cardkey Systems",    //71
   "SECOM Co., Ltd.",    //72
   "ABB Gebäudetechnik AG Bereich NetServ",    //73
   "KNX Association cvba",    //74
   "Institute of Electrical Installation Engineers of Japan (IEIEJ)",    //75
   "Nohmi Bosai, Ltd.",    //76
   "Carel S.p.A.",    //77
   "AirSense Technology, Inc.",    //78
   "Hochiki Corporation",    //79
   "Fr. Sauter AG",    //80
   "Matsushita Electric Works, Ltd.",    //81
   "Mitsubishi Electric Corporation, Inazawa Works",    //82
   "Mitsubishi Heavy Industries, Ltd.",    //83
   "ITT Bell & Gossett",    //84
   "Yamatake Building Systems Co., Ltd.",    //85
   "The Watt Stopper, Inc.",    //86
   "Aichi Tokei Denki Co., Ltd.",    //87
   "Activation Technologies, LLC",    //88
   "Saia-Burgess Controls, Ltd.",    //89
   "Hitachi, Ltd.",    //90
   "Novar Corp./Trend Control Systems Ltd.",    //91
   "Mitsubishi Electric Lighting Corporation",    //92
   "Argus Control Systems, Ltd.",    //93
   "Kyuki Corporation",    //94
   "Richards-Zeta Building Intelligence, Inc.",    //95
   "Scientech R&D, Inc.",    //96
   "VCI Controls, Inc.",    //97
   "Toshiba Corporation",    //98
   "Mitsubishi Electric Corporation Air Conditioning & Refrigeration Systems Works",    //99
   "Custom Mechanical Equipment, LLC",    //100
   "ClimateMaster",    //101
   "ICP Panel-Tec, Inc.",    //102
   "D-Tek Controls",    //103
   "NEC Engineering, Ltd.",    //104
   "PRIVA BV",    //105
   "Meidensha Corporation",    //106
   "JCI Systems Integration Services",    //107
   "Freedom Corporation",    //108
   "Neuberger Gebäudeautomation GmbH",    //109
   "Sitronix",    //110
   "Leviton Manufacturing",    //111
   "Fujitsu Limited",    //112
   "Emerson Network Power",    //113
   "S. A. Armstrong, Ltd.",    //114
   "Visonet AG",    //115
   "M&M Systems, Inc.",    //116
   "Custom Software Engineering",    //117
   "Nittan Company, Limited",    //118
   "Elutions Inc. (Wizcon Systems SAS)",    //119
   "Pacom Systems Pty., Ltd.",    //120
   "Unico, Inc.",    //121
   "Ebtron, Inc.",    //122
   "Scada Engine",    //123
   "AC Technology Corporation",    //124
   "Eagle Technology",    //125
   "Data Aire, Inc.",    //126
   "ABB, Inc.",    //127
   "Transbit Sp. z o. o.",    //128
   "Toshiba Carrier Corporation",    //129
   "Shenzhen Junzhi Hi-Tech Co., Ltd.",    //130
   "Tokai Soft",    //131
   "Blue Ridge Technologies",    //132
   "Veris Industries",    //133
   "Centaurus Prime",    //134
   "Sand Network Systems",    //135
   "Regulvar, Inc.",    //136
   "Fastek International, Ltd.",    //137
   "PowerCold Comfort Air Solutions, Inc.",    //138
   "I Controls",    //139
   "Viconics Electronics, Inc.",    //140
   "Yaskawa Electric America, Inc.",    //141
   "DEOS control systems GmbH",    //142
   "Digitale Mess- und Steuersysteme AG",    //143
   "Fujitsu General Limited",    //144
   "Project Engineering S.r.l.",    //145
   "Sanyo Electric Co., Ltd.",    //146
   "Integrated Information Systems, Inc.",    //147
   "Temco Controls, Ltd.",    //148
   "Airtek Technologies, Inc.",    //149
   "Advantech Corporation",    //150
   "Titan Products, Ltd.",    //151
   "Regel Partners",    //152
   "National Environmental Product",    //153
   "Unitec Corporation",    //154
   "Kanden Engineering Company",    //155
   "Messner Gebäudetechnik GmbH",    //156
   "Integrated.CH",    //157
   "EH Price Limited",    //158
   "SE-Electronic GmbH",    //159
   "Rockwell Automation",    //160
   "Enflex Corp.",    //161
   "ASI Controls",    //162
   "SysMik GmbH Dresden",    //163
   "HSC Regelungstechnik GmbH",    //164
   "Smart Temp Australia Pty. Ltd.",    //165
   "Cooper Controls",    //166
   "Duksan Mecasys Co., Ltd.",    //167
   "Fuji IT Co., Ltd.",    //168
   "Vacon Plc",    //169
   "Leader Controls",    //170
   "Cylon Controls, Ltd.",    //171
   "Compas",    //172
   "Mitsubishi Electric Building Techno-Service Co., Ltd.",    //173
   "Building Control Integrators",    //174
   "ITG Worldwide (M) Sdn Bhd",    //175
   "Lutron Electronics Co., Inc.",    //176
   "Cooper-Atkins Corporation",    //177
   "LOYTEC Electronics GmbH",    //178
   "ProLon",    //179
   "Mega Controls Limited",    //180
   "Micro Control Systems, Inc.",    //181
   "Kiyon, Inc.",    //182
   "Dust Networks",    //183
   "Advanced Building Automation Systems",    //184
   "Hermos AG",    //185
   "CEZIM",    //186
   "Softing",    //187
   "Lynxspring",    //188
   "Schneider Toshiba Inverter Europe",    //189
   "Danfoss Drives A/S",    //190
   "Eaton Corporation",    //191
   "Matyca S.A.",    //192
   "Botech AB",    //193
   "Noveo, Inc.",    //194
   "AMEV",    //195
   "Yokogawa Electric Corporation",    //196
   "GFR Gesellschaft für Regelungstechnik",    //197
   "Exact Logic",    //198
   "Mass Electronics Pty Ltd dba Innotech Control Systems Australia",    //199
   "Kandenko Co., Ltd.",    //200
   "DTF, Daten-Technik Fries",    //201
   "Klimasoft, Ltd.",    //202
   "Toshiba Schneider Inverter Corporation",    //203
   "Control Applications, Ltd.",    //204
   "KDT Systems Co., Ltd.",    //205
   "Onicon Incorporated",    //206
   "Automation Displays, Inc.",    //207
   "Control Solutions, Inc.",    //208
   "Remsdaq Limited",    //209
   "NTT Facilities, Inc.",    //210
   "VIPA GmbH",    //211
   "TSC21 Association of Japan",    //212
   "BBP Energie Ltee",    //213
   "HRW Limited",    //214
   "Lighting Control & Design, Inc.",    //215
   "Mercy Electronic and Electrical Industries",    //216
   "Samsung SDS Co., Ltd",    //217
   "Impact Facility Solutions, Inc.",    //218
   "Aircuity",    //219
   "Control Techniques, Ltd.",    //220
   "OpenGeneral Pty., Ltd.",    //221
   "WAGO Kontakttechnik GmbH & Co. KG",    //222
   "Cerus Industrial",    //223
   "Chloride Power Protection Company",    //224
   "Computrols, Inc.",    //225
   "Phoenix Contact GmbH & Co. KG",    //226
   "Grundfos Management A/S",    //227
   "Ridder Drive Systems",    //228
   "Soft Device SDN BHD",    //229
   "Integrated Control Technology Limited",    //230
   "AIRxpert Systems, Inc.",    //231
   "Microtrol Limited",    //232
   "Red Lion Controls",    //233
   "Digital Electronics Corporation",    //234
   "Ennovatis GmbH",    //235
   "Serotonin Software Technologies, Inc.",    //236
   "LS Industrial Systems Co., Ltd.",    //237
   "Square D Company",    //238
   "S Squared Innovations, Inc.",    //239
   "Aricent Ltd.",    //240
   "EtherMetrics, LLC",    //241
   "Industrial Control Communications, Inc.",    //242
   "Paragon Controls, Inc.",    //243
   "A. O. Smith Corporation",    //244
   "Contemporary Control Systems, Inc.",    //245
   "Intesis Software SL",    //246
   "Ingenieurgesellschaft N. Hartleb mbH",    //247
   "Heat-Timer Corporation",    //248
   "Ingrasys Technology, Inc.",    //249
   "Costerm Building Automation",    //250
   "WILO SE",    //251
   "Embedia Technologies Corp.",    //252
   "Technilog",    //253
   "HR Controls Ltd. & Co. KG",    //254
   "Lennox International, Inc.",    //255
   "RK-Tec Rauchklappen-Steuerungssysteme GmbH & Co. KG",    //256
   "Thermomax, Ltd.",    //257
   "ELCON Electronic Control, Ltd.",    //258
   "Larmia Control AB",    //259
   "BACnet Stack at SourceForge",    //260
   "G4S Security Services A/S",    //261
   "Sitek S.p.A.",    //262
   "Cristal Controles",    //263
   "Regin AB",    //264
   "Dimension Software, Inc. ",    //265
   "SynapSense Corporation",    //266
   "Beijing Nantree Electronic Co., Ltd.",    //267
   "Camus Hydronics Ltd.",    //268
   "Kawasaki Heavy Industries, Ltd. ",    //269
   "Critical Environment Technologies",    //270
   "ILSHIN IBS Co., Ltd.",    //271
   "ELESTA Energy Control AG",    //272
   "KROPMAN Installatietechniek",    //273
   "Baldor Electric Company",    //274
   "INGA mbH",    //275
   "GE Consumer & Industrial",    //276
   "Functional Devices, Inc.",    //277
   "ESAC",    //278
   "M-System Co., Ltd.",    //279
   "Yokota Co., Ltd.",    //280
   "Hitranse Technology Co., LTD",    //281
   "Federspiel Controls",    //282
   "Kele, Inc.",    //283
   "Opera Electronics, Inc.",    //284
   "Gentec",    //285
   "Embedded Science Labs, LLC",    //286
   "Parker Hannifin Corporation",    //287
   "MaCaPS International Limited",    //288
   "Link4 Corporation",    //289
   "Romutec Steuer-u. Regelsysteme GmbH ",    //290
   "Pribusin, Inc.",    //291
   "Advantage Controls",    //292
   "Critical Room Control",    //293
   "LEGRAND",    //294
   "Tongdy Control Technology Co., Ltd. ",    //295
   "ISSARO Integrierte Systemtechnik",    //296
   "Pro-Dev Industries",    //297
   "DRI-STEEM",    //298
   "Creative Electronic GmbH",    //299
   "Swegon AB",    //300
   "Jan Brachacek",    //301
   "Hitachi Appliances, Inc.",    //302
   "Real Time Automation, Inc.",    //303
   "ITEC Hankyu-Hanshin Co.",    //304
   "Cyrus E&M Engineering Co., Ltd. ",    //305
   "Racine Federated, Inc.",    //306
   "Verari Systems, Inc. ",    //307
   "Elesta GmbH Building Automation ",    //308
   "Securiton",    //309
   "OSlsoft, Inc.",    //310
   "Hanazeder Electronic GmbH ",    //311
   "Honeywell Security Deutschland, Novar GmbH",    //312
   "Siemens Energy & Automation, Inc.",    //313
   "ETM Professional Control GmbH",    //314
   "Meitav-tec, Ltd.",    //315
   "Janitza Electronics GmbH ",    //316
   "MKS Nordhausen",    //317
   "De Gier Drive Systems B.V. ",    //318
   "Cypress Envirosystems",    //319
   "SMARTron s.r.o.",    //320
   "Verari Systems, Inc.",    //321
   "K-W Electronic Service, Inc.",    //322
   "ALFA-SMART Energy Management",    //323
   "Telkonet, Inc.",    //324
   "Securiton GmbH",    //325
   "Cemtrex, Inc.",    //326
   "Performance Technologies, Inc.",    //327
   "Xtralis (Aust) Pty Ltd",    //328
   "TROX GmbH",    //329
   "Beijing Hysine Technology Co., Ltd",    //330
   "RCK Controls, Inc.",    //331
   "ACELIA",    //332
   "Novar/Honeywell",    //333
   "The S4 Group, Inc.",    //334
   "Schneider Electric",    //335
   "LHA Systems",    //336
   "GHM engineering Group, Inc.",    //337
   "Cllimalux S.A.",    //338
   "VAISALA Oyj",    //339
   "COMPLEX (Beijing) Technology, Co., LTD.",    //340
   "SCADAmetrics",    //341
   "POWERPEG NSI Limited",    //342
   "BACnet Interoperability Testing Services, Inc.",    //343
   "Teco a.s.",    //344
   "Plexus Technology Limited",    //345
   "Energy Focus, Inc.",    //346
   "Powersmiths International Corp.",    //347
   "Nichibei Co., Ltd.",    //348
   "HKC Technology Ltd.",    //349
   "Ovation Networks, Inc.",    //350
   "Setra Systems",    //351
   "AVG Automation",    //352
   "ZXC Ltd.",    //353
   "Byte Sphere",    //354
   "Generiton Co., Ltd.",    //355
   "Holter Regelarmaturen GmbH & Co. KG",    //356
   "Bedford Instruments, LLC",    //357
   "Standair Inc.",    //358
   "WEG Automation - R&D",    //359
   "Prolon Control Systems ApS",    //360
   "Inneasoft",    //361
   "ConneXSoft GmbH",    //362
   "CEAG Notlichtsysteme GmbH",    //363
   "Distech Controls, Inc.",    //364
   "Industrial Technology Research Institute",    //365
   "ICONICS, Inc.",    //366
   "IQ Controls s.c.",    //367
   "OJ Electronics A/S",    //368
   "Rolbit Ltd.",    //369
   "Synapsys Solutions Ltd.",    //370
   "ACME Engineering Prod. Ltd.",    //371
   "Zener Electric Pty, Ltd.",    //372
   "Selectronix, Inc.",    //373
   "Gorbet & Banerjee, LLC.",    //374
   "IME",    //375
   "Stephen H. Dawson Computer Service",    //376
   "Accutrol, LLC",    //377
   "Schneider Elektronik GmbH",    //378
   "Alpha-Inno Tec GmbH",    //379
   "ADMMicro, Inc.",    //380
   "Greystone Energy Systems, Inc.",    //381
   "CAP Technologie",    //382
   "KeRo Systems",    //383
   "Domat Control System s.r.o.",    //384
   "Efektronics Pty. Ltd.",    //385
   "Hekatron Vertriebs GmbH",    //386
   "Securiton AG",    //387
   "Carlo Gavazzi Controls SpA",    //388
   "Chipkin Automation Systems",    //389
   "Savant Systems, LLC",    //390
   "Simmtronic Lighting Controls",    //391
   "Abelko Innovation AB",    //392
   "Seresco Technologies Inc.",    //393
   "IT Watchdogs",    //394
   "Automation Assist Japan Corp.",    //395
   "Thermokon Sensortechnik GmbH",    //396
   "EGauge Systems, LLC",    //397
   "Quantum Automation (ASIA) PTE, Ltd.",    //398
   "Toshiba Lighting & Technology Corp.",    //399
   "SPIN Engenharia de Automação Ltda.",    //400
   "Logistics Systems & Software Services India PVT. Ltd.",    //401
   "Delta Controls Integration Products",    //402
   "Focus Media",    //403
   "LUMEnergi Inc.",    //404
   "Kara Systems",    //405
   "RF Code, Inc.",    //406
   "Fatek Automation Corp.",    //407
   "JANDA Software Company, LLC",    //408
   "Open System Solutions Limited",    //409
   "Intelec Systems PTY Ltd.",    //410
   "Ecolodgix, LLC",    //411
   "Douglas Lighting Controls",    //412
   "Intelligente Sensoren Aktoren Tech",    //413
   "AREAL",    //414
   "Beckhoff Automation GmbH",    //415
   "IPAS GmbH",    //416
   "KE2 Therm Solutions"    //417
   // Updated 27 Feb 2010 from http://www.bacnet.org/VendorID/BACnet%20Vendor%20IDs.htm
   
   // TODO add more here ...
};
BAC_STRINGTABLE(BACnetVendorID);

STRING_TABLE BACnetTimeValue[] = {
   "Time",
   "Value"
};
BAC_STRINGTABLE(BACnetTimeValue);

STRING_TABLE BACnetVTClass[] = {
   "Default Terminal class",
   "ANSI X3.64 class",
   "DEC VT52 class",
   "DEC VT100 class",
   "DEC VT220 class",
   "HP 700/94 class",
   "IBM 3130 class"
};
BAC_STRINGTABLE(BACnetVTClass);

STRING_TABLE BACnetVTSession[] = {
   "Local VT-Session ID",
   "Remote VT-Session ID",
   "Remote VT-Address"
};
BAC_STRINGTABLE(BACnetVTSession);

//Xiao Shiyuan 2002-7-23
STRING_TABLE BACnetCOVSubscription[] = {
	"Recipient",
    "Monitored Property Reference",
	"Issue Confirmed Notifications",
	"Time remaining",
	"COV increment"
};
BAC_STRINGTABLE(BACnetCOVSubscription);

STRING_TABLE BACnetWeekNDay[] = {
   "Month",
   "Week of Month",
   "Day of Week"
};
BAC_STRINGTABLE(BACnetWeekNDay);

// Used for Date etc., where Monday == 1
// See also BACnetDaysOfWeek
STRING_TABLE day_of_week[] = {
   "Invalid Day",  /* 0 */
   "Monday",       /* 1 */
   "Tuesday",      /* 2 */
   "Wednesday",    /* 3 */
   "Thursday",     /* 4 */
   "Friday",       /* 5 */
   "Saturday",     /* 6 */
   "Sunday"        /* 7 */
};
BAC_STRINGTABLE(day_of_week);

STRING_TABLE month[] = {
   "Invalid Month", /* 0 */
   "January",       /* 1 */
   "February",      /* 2 */
   "March",         /* 3 */
   "April",         /* 4 */
   "May",           /* 5 */
   "June",          /* 6 */
   "July",          /* 7 */
   "August",        /* 8 */
   "September",     /* 9 */
   "October",       /* 10 */
   "November",      /* 11 */
   "December",      /* 12 */
   "Odd",			/* 13 */
   "Even"			/* 14 */
};
BAC_STRINGTABLE(month);

STRING_TABLE PDU_types[] = {
   "Confirmed Service",
   "Unconfirmed Service",
   "Simple ACK",
   "Complex ACK",
   "Segment ACK",
   "Error",
   "Reject",
   "Abort"
};
BAC_STRINGTABLE(PDU_types);

// madanner 11/12/02, added to support variable stuffing for PDU type,
// values written to var must decode back into values

STRING_TABLE PDU_typesENUM[] = {
   "CONFIRMED-REQUEST",
   "UNCONFIRMED-REQUEST",
   "SIMPLEACK",
   "COMPLEXACK",
   "SEGMENTACK",
   "ERROR",
   "REJECT",
   "ABORT"
};
BAC_STRINGTABLE(PDU_typesENUM);

STRING_TABLE NL_msgs[] = {
   "Who-Is-Router-To-Network",
   "I-Am-Router-To-Network",
   "I-Could-Be-Router-To-Network",
   "Reject-Message-To-Network",
   "Router-Busy-To-Network",
   "Router-Available-To-Network",
   "Initialize-Routing-Table",
   "Initialize-Routing-Table-Ack",
   "Establish-Connection-To-Network",
   "Disconnect-Connection-To-Network",
   "ASHRAE Reserved",
   "Vendor Proprietary Message"
};
BAC_STRINGTABLE(NL_msgs);

STRING_TABLE BACnetReject[] = {
   "Other",                              /*0*/
   "Buffer-overflow",                    /*1*/
   "Inconsistent-parameters",            /*2*/
   "Invalid-parameter-datatype",         /*3*/
   "Invalid-tag",                        /*4*/
   "Missing-required-tag",               /*5*/
   "Parameter-out-or-range",             /*6*/
   "Too-many-arguments",                 /*7*/
   "Undefined-enumeration",              /*8*/
   "Unrecognized-service"                /*9*/
};
BAC_STRINGTABLE(BACnetReject);

STRING_TABLE BACnetAbort[] = {
   "Other",                             /*0*/
   "Buffer-overflow",                   /*1*/
   "Invalid-APDU-in-this-state",        /*2*/
   "Preempted-by-higher-priority-task", /*3*/
   "Segmentation-not-supported"         /*4*/
};
BAC_STRINGTABLE(BACnetAbort);

// Value for ReadPropertyConditionl selectionLogic
STRING_TABLE Selection_Logic[] = {
   "and",
   "or",
   "all"
};
BAC_STRINGTABLE(Selection_Logic);

// Value for ReadPropertyConditionl relationSpecifier
STRING_TABLE Relation_Specifier[] = {
   "Equal (=)",
   "Not Equal (!=)",
   "Less Than (<)",
   "Greater Than (>)",
   "Less Than or Equal (<=)",
   "Greater Than or Equal (>=)"
};
BAC_STRINGTABLE(Relation_Specifier);

STRING_TABLE BVLL_Function[] = {
   "BVLC-Result",
   "Write-Broadcast-Distribution-Table",
   "Read-Broadcast-Distribution-Table",
   "Read-Broadcast-Distribution-Table-Ack",
   "Forwarded-NPDU",
   "Register-Foreign-Device",
   "Read-Foreign-Device-Table",
   "Read-Foreign-Device-Table-Ack",
   "Delete-Foreign-Device-Table-Entry",
   "Distribute-Broadcast-To-Network",
   "Original-Unicast-NPDU",
   "Original-Broadcast-NPDU"
};
BAC_STRINGTABLE(BVLL_Function);

STRING_TABLE BACnetConfirmedServiceChoice[] = {
   "AcknowledgeAlarm",              /* 0 */
   "ConfirmedCOVNotification",      /* 1 */
   "ConfirmedEventNotification",    /* 2 */
   "GetAlarmSummary",               /* 3 */
   "GetEnrollmentSummary",          /* 4 */
   "SubscribeCOV",                  /* 5 */
   "AtomicReadFile",                /* 6 */
   "AtomicWriteFile",               /* 7 */
   "AddListElement",                /* 8 */
   "RemoveListElement",             /* 9 */
   "CreateObject",                  /* 10 */
   "DeleteObject",                  /* 11 */
   "ReadProperty",                  /* 12 */
   "ReadPropertyConditional",       /* 13 */
   "ReadPropertyMultiple",          /* 14 */
   "WriteProperty",                 /* 15 */
   "WritePropertyMultiple",         /* 16 */
   "DeviceCommunicationControl",    /* 17 */
   "ConfirmedPrivateTransfer",      /* 18 */
   "ConfirmedTextMessage",          /* 19 */
   "ReinitializeDevice",            /* 20 */
   "VT-Open",                       /* 21 */
   "VT-Close",                      /* 22 */
   "VT-Data",                       /* 23 */
   "Authenticate",                  /* 24 */
   "RequestKey",                    /* 25 */
   "ReadRange",                     /* 26 */
   "LifeSafetyOperation",           /* 27 */
   "SubscribeCOVProperty",          /* 28 */
   "GetEventInformation"            /* 29 */
   // CAUTION: if you add a service here, you must also change max_confirmed_services
   // (which is actually max-plus-one: the NUMBER of defined services)
};                       
BAC_STRINGTABLE(BACnetConfirmedServiceChoice);

STRING_TABLE BACnetUnconfirmedServiceChoice[] = {
   "I-Am",                          /* 0 */
   "I-Have",                        /* 1 */
   "UnconfirmedCOVNotification",    /* 2 */
   "UnconfirmedEventNotification",  /* 3 */
   "UnconfirmedPrivateTransfer",    /* 4 */
   "UnconfirmedTextMessage",        /* 5 */
   "TimeSynchronization",           /* 6 */
   "Who-Has",                       /* 7 */
   "Who-Is",                        /* 8 */
   "UTCTimeSynchronization"         /* 9 */
   // CAUTION: if you add a service here, you must also change max_unconfirmed_services
   // (which is actually max-plus-one: the NUMBER of defined services)
};                       
BAC_STRINGTABLE(BACnetUnconfirmedServiceChoice);

STRING_TABLE BACnetReinitializedStateOfDevice[] = {
	"coldstart",	/* 0 */
	"warmstart",
	"startbackup",
	"endbackup",
	"startrestore",
	"endrestore",
	"abortrestore"	/* 6 */
};
BAC_STRINGTABLE(BACnetReinitializedStateOfDevice);

STRING_TABLE DeviceCommControl_Command[] = {
	"enable",	/* 0 */
	"disable",
	"disable-initiation"
};
BAC_STRINGTABLE(DeviceCommControl_Command);

STRING_TABLE TextMessage_Priority[] = {
	"normal",	/* 0 */
	"urgent"
};
BAC_STRINGTABLE(TextMessage_Priority);

} // end namespace NetworkSniffer
