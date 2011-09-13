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

// Constructor for non-extensible enumerations
BACnetStringTable::BACnetStringTable( const char* const *pStrings,
									  const int			nStrings )
: m_pStrings(pStrings)
, m_nStrings(nStrings)
, m_nReserved(nStrings)
, m_nMax(nStrings)
{
}
	
// Constructor for enumerations defined by BACnet as extensible
BACnetStringTable::BACnetStringTable( const char* const *pStrings,
									  const int		    nStrings,
									  const int		    nReserved,
									  const int		    nMax )
: m_pStrings(pStrings)
, m_nStrings(nStrings)
, m_nReserved(nReserved)
, m_nMax(nMax)
{
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

// Not an explicit datatype: defined inline in BACnetAccumulatorRecord
STRING_TABLE BACnetAccumulatorStatus[] = {
	"normal",
	"starting",
	"recovered",
	"abnormal",
    "failed"
};
BAC_STRINGTABLE(BACnetAccumulatorStatus);

STRING_TABLE BACnetAction[] = {
   "direct",
   "reverse"
   };
BAC_STRINGTABLE(BACnetAction);

STRING_TABLE BACnetBinaryPV[] = {
   "inactive",
   "active"
   };
BAC_STRINGTABLE(BACnetBinaryPV);

STRING_TABLE BACnetDeviceStatus[] = {
   "operational",
   "operational-read-only",
   "download-required",
   "download-in-progress",
   "non-operational",
   "backup-in-progress"		// added by Jingbo Gao, Sep 20 2004
};
BAC_STRINGTABLE_EX(BACnetDeviceStatus, 64, 65536);

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
BAC_STRINGTABLE_EX(BACnetDoorAlarmState, 256, 65536);

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
BAC_STRINGTABLE_EX(BACnetEngineeringUnits, 256, 65536);

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
   "device",
   "object",
   "property",
   "resources",
   "security",
   "services",
   "vt",
   "communication",
};
BAC_STRINGTABLE_EX(BACnetErrorClass, 64, 65536);

STRING_TABLE BACnetErrorCode[] = {
   "other",                              /* 0 */
   "authentication-failed",              /* 1 */
   "configuration-in-progress",          /* 2 */
   "device-busy",                        /* 3 */
   "dynamic-creation-not-supported",     /* 4 */
   "file-access-denied",                 /* 5 */
   "incompatible-security-levels",       /* 6 */
   "inconsistent-parameters",            /* 7 */
   "inconsistent-selection-criterion",   /* 8 */
   "invalid-data-type",                  /* 9 */
   "invalid-file-access-method",         /* 10 */
   "invalid-file-start-position",        /* 11 */
   "invalid-operator-name",              /* 12 */
   "invalid-parameter-data-type",        /* 13 */
   "invalid-time-stamp",                 /* 14 */
   "key-generation-error",               /* 15 */
   "missing-required-parameter",         /* 16 */
   "no-objects-of-specified-type",       /* 17 */
   "no-space-for-object",                /* 18 */
   "no-space-to-add-list-element",       /* 19 */
   "no-space-to-write-property",         /* 20 */
   "no-vt-sessions-available",           /* 21 */
   "property-is-not-a-list",             /* 22 */
   "object-deletion-not-permitted",      /* 23 */
   "object-identifier-already-exists",   /* 24 */
   "operational-problem",                /* 25 */
   "password-failure",                   /* 26 */
   "read-access-denied",                 /* 27 */
   "security-not-supported",             /* 28 */
   "service-request-denied",             /* 29 */
   "timeout",                            /* 30 */
   "unknown-object",                     /* 31 */
   "unknown-property",                   /* 32 */
   "invalid enumeration",                /* 33 */
   "unknown-vt-class",                   /* 34 */
   "unknown-vt-session",                 /* 35 */
   "unsupported-object-type",            /* 36 */
   "value-out-of-range",                 /* 37 */
   "vt-session-already-closed",          /* 38 */
   "vt-session-termination-failure",     /* 39 */
   "write-access-denied",                /* 40 */
   "character-set-not-supported",        /* 41 */
   "invalid-array-index",                /* 42 */
   "cov-subscription-failed",            /* 43 kare.sars@wapice.com */
   "not-cov-property",                   /* 44 | */
   "optional-functionality-not-supported",/*45 | */
   "invalid-configuration-data",         /* 46 | */
   "datatype-not-supported",             /* 47 | */
   "duplicate-name",                     /* 48 | */
   "duplicate-object-id",                /* 49 | */
   "property-is-not-an-array",           /* 50 kare.sars@wapice.com */
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
   // added by addenda b
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
BAC_STRINGTABLE_EX(BACnetErrorCode, 256, 65536);

STRING_TABLE BACnetEventState[] = {
   "normal",
   "fault",
   "offnormal",
   "high-limit",
   "low-limit",
   "life-safety-alarm",
   };
BAC_STRINGTABLE_EX(BACnetEventState, 64, 65536);

STRING_TABLE BACnetEventTransitionBits[] = {
   "to-offnormal",
   "to-fault",
   "to-normal",
   };
BAC_STRINGTABLE(BACnetEventTransitionBits);

//Modified by Zhu Zhenhua, 2004-5-17
STRING_TABLE BACnetEventType[] = {
   "change-of-bitstring",		// 0
   "change-of-state",
   "change-of-value",
   "command-failure",
   "floating-limit",
   "out-of-range",				// 5
   "complex-event-type",
   "deprecated",
   "change-of-life-safety",
   "extended",
   "buffer-ready",				// 10
   "unsigned-range",			// 11 last in 135-2008
   "change-of-status-flags",	// 12
   };
BAC_STRINGTABLE_EX(BACnetEventType, 64, 65536);

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
   "record-access",
   "stream-access"
};
BAC_STRINGTABLE(BACnetFileAccessMethod);

///////////////////////////////////////////////////////////////////////////
//Added by Zhu Zhenhua, 2004-6-14
STRING_TABLE BACnetLifeSafetyMode[] = {
   "off",					// 0
   "on",
   "test",
   "manned",
   "unmanned",
   "armed",					// 5
   "disarmed",
   "prearmed",
   "slow",
   "fast",
   "disconnected",			// 10
   "enabled",
   "disabled",
   "automatic-release-disabled",
   "default"				// 14 last in 135-2008
};
BAC_STRINGTABLE_EX(BACnetLifeSafetyMode, 256, 65536);

STRING_TABLE BACnetLifeSafetyOperation[] = {
   "none",					// 0
   "silence",
   "silence-audible",
   "silence-visual",
   "reset",
   "reset-alarm",			// 5
   "reset-fault",
   "unsilence",
   "unsilence-audible",
   "unsilence-visual",		// 9 last in 135-2008
};
BAC_STRINGTABLE_EX(BACnetLifeSafetyOperation, 64, 65536);

STRING_TABLE BACnetLifeSafetyState[] = {
   "quiet",					// 0
   "pre-alarm",
   "alarm",
   "fault",
   "fault-pre-alarm",
   "fault-alarm",
   "not-ready",
   "active",
   "tamper",
   "test-alarm",
   "test-active",			// 10
   "test-fault",
   "test-fault-alarm",
   "holdup",
   "duress",
   "tamper-alarm",
   "abnormal",
   "emergency-power",
   "delayed",
   "blocked",
   "local-alarm",			// 20
   "general-alarm",
   "supervisory",
   "test-supervisory"		// 23 last in 135-2008
};
BAC_STRINGTABLE_EX(BACnetLifeSafetyState, 256, 65536);

STRING_TABLE BACnetLimitEnable[] = {
   "lowLimitEnable",
   "highLimitEnable"
   };
BAC_STRINGTABLE(BACnetLimitEnable);

STRING_TABLE BACnetLockStatus[] = {
	"locked",
	"unlocked",
	"fault",
	"unknown",
};
BAC_STRINGTABLE(BACnetLockStatus);

STRING_TABLE BACnetLoggingType[] = {
	"polled",
	"cov",
	"triggered",
};
BAC_STRINGTABLE(BACnetLoggingType);

STRING_TABLE BACnetLogStatus[] = {
	"log-disabled",
	"buffer-purged",
	"log-interrupted",
};
BAC_STRINGTABLE_EX(BACnetLogStatus, 64, 256);

STRING_TABLE  BACnetMaintenance[] = {
	"none",
	"periodic-test",
	"need-service-operational",
	"need-service-inoperative"
};
BAC_STRINGTABLE_EX(BACnetMaintenance, 256, 65536);

STRING_TABLE BACnetNodeType[] = {
   "unknown",
   "system",
   "network",
   "device",
   "organization",
   "area",
   "equipment",
   "point",
   "collection",
   "property",
   "functional",
   "other"
};
BAC_STRINGTABLE(BACnetNodeType);
   
STRING_TABLE BACnetNotifyType[] = {
   "alarm",
   "event",
   "ack-notification"
   };
BAC_STRINGTABLE(BACnetNotifyType);

// This list is also used to create the hash table for scripting.
STRING_TABLE BACnetObjectType[] = {
   "analog-input",          /* 0 */
   "analog-output",         /* 1 */
   "analog-value",          /* 2 */
   "binary-input",          /* 3 */
   "binary-output",         /* 4 */
   "binary-value",          /* 5 */
   "calendar",              /* 6 */
   "command",               /* 7 */
   "device",                /* 8 */
   "event-enrollment",      /* 9 */
   "file",                  /* 10 */
   "group",                 /* 11 */
   "loop",                  /* 12 */
   "multi-state-input",     /* 13 */
   "multi-state-output",    /* 14 */
   "notification-class",    /* 15 */
   "program",               /* 16 */
   "schedule",              /* 17 */
   "averaging",             /* 18 */
   "multi-state-value",     /* 19 */
   "trend-log" ,            /* 20 */		// msdanner 9/04, was "trendlog"
   "life-safety-point",	    /* 21 Zhu Zhenhua 2003-7-24 */	 // msdanner 9/04, was "LIFESAFETYPOINT"
   "life-safety-zone",	    /* 22 Zhu Zhenhua 2003-7-24 */  // msdanner 9/04, was "LIFESAFETYZONE"
   "accumulator",           // 23 Shiyuan Xiao 7/15/2005
   "pulse-converter",       // 24 Shiyuan Xiao 7/15/2005
   "event-log",				// 25 - Addendum B
   "global-group",			// 26 - Addendum B
   "trend-log-multiple",	// 27 - Addendum B
   "load-control",			// 28 - Addendum E 135-2004
   "structured-view",		// 29 - Addendum D
   "access-door",			// 30 Last in 135-2008
   "lighting-output",		// 31
   "access-credential",		// 32
   "access-point",			// 33
   "access-rights",			// 34
   "access-user",			// 35
   "access-zone",			// 36
   "credential-data-input",	// 37
   "network-security",		// 38 Addendum 2008-g
   "bitstring-value",		// 39 Addendum 2008-w
   "characterstring-value", // 40
   "date-pattern-value",	// 41
   "date-value",			// 42
   "datetime-pattern-value",// 43
   "datetime-value",		// 44
   "integer-value",			// 45
   "large-analog-value",	// 46
   "octetstring-value",		// 47
   "positive-integer-value",// 48
   "time-pattern-value",	// 49
   "time-value"				// 50 Last in 2008-w

	// CAUTION: if you add a type here, you must also change MAX_DEFINED_OBJ 
	// and NUM_DEFINED_OBJECTS
	// (which are actually max-plus-one: the NUMBER of defined object types)
	//
	// And add to StandardObjects in Vtsapi32.cpp
	// And other code, sprinkled throughout the universe...
	//
	// For each type, there is also an icon for use in the EpicsTree view,
	// IDB_EPICSTREE  BITMAP "res\\epicstree.bmp"
	// So if you add or implement new Objects, you need to do some artwork there.

};
BAC_STRINGTABLE_EX(BACnetObjectType, 128, 1024);

STRING_TABLE BACnetPolarity[] = {
   "normal",
   "reverse"
};
BAC_STRINGTABLE(BACnetPolarity);

STRING_TABLE BACnetProgramError[] = {
   "normal",
   "load-failed",
   "internal",
   "program",
   "other"
};
BAC_STRINGTABLE_EX(BACnetProgramError, 64, 65536);

STRING_TABLE BACnetProgramRequest[] = {
   "ready",
   "load",
   "run",
   "halt",
   "restart",
   "unload"
};
BAC_STRINGTABLE(BACnetProgramRequest);

STRING_TABLE BACnetProgramState[] = {
   "idle",
   "loading",
   "running",
   "waiting",
   "halted",
   "unloading"
};   
BAC_STRINGTABLE(BACnetProgramState);

// This list is also used to create the hash table for scripting.
STRING_TABLE BACnetPropertyIdentifier[] = {
   "acked-transitions",                /* 0 */
   "ack-required",                     /* 1 */
   "action",                           /* 2 */
   "action-text",                      /* 3 */
   "active-text",                      /* 4 */
   "active-vt-sessions",               /* 5 */
   "alarm-value",                      /* 6 */
   "alarm-values",                     /* 7 */
   "all",                              /* 8 */
   "all-writes-successful",            /* 9 */
   "apdu-segment-timeout",             /* 10 */
   "apdu-timeout",                     /* 11 */
   "application-software-version",     /* 12 */
   "archive",                          /* 13 */
   "bias",                             /* 14 */
   "change-of-state-count",            /* 15 */
   "change-of-state-time",             /* 16 */
   "notification-class",               /* 17 renamed from "class" in 2nd public review */
   "unused-propertyid-18",             /* 18 */
   "controlled-variable-reference",    /* 19 */
   "controlled-variable-units",        /* 20 */
   "controlled-variable-value",        /* 21 */
   "cov-increment",                    /* 22 */
   "date-list",                        /* 23 */
   "daylight-savings-status",          /* 24 */
   "deadband",                         /* 25 */
   "derivative-constant",              /* 26 */
   "derivative-constant-units",        /* 27 */
   "description",                      /* 28 */
   "description-of-halt",              /* 29 */
   "device-address-binding",           /* 30 */
   "device-type",                      /* 31 */
   "effective-period",                 /* 32 */
   "elapsed-active-time",              /* 33 */
   "error-limit",                      /* 34 */
   "event-enable",                     /* 35 */
   "event-state",                      /* 36 */
   "event-type",                       /* 37 */
   "exception-schedule",               /* 38 */
   "fault-values",                     /* 39 */
   "feedback-value",                   /* 40 */
   "file-access-method",               /* 41 */
   "file-size",                        /* 42 */
   "file-type",                        /* 43 */
   "firmware-revision",                /* 44 */
   "high-limit",                       /* 45 */
   "inactive-text",                    /* 46 */
   "in-process",                       /* 47 */
   "instance-of",                      /* 48 */
   "integral-constant",                /* 49 */
   "integral-constant-units",          /* 50 */
   "unused-was-issue-confirmed-notifications",    /* 51 deleted in version 1 revision 4*/
   "limit-enable",                     /* 52 */
   "list-of-group-members",            /* 53 */
   "list-of-object-property-references",  /* 54 zhu zhenhua 2003-7-24 */
   "list-of-session-keys",             /* 55 */
   "local-date",                       /* 56 */
   "local-time",                       /* 57 */
   "location",                         /* 58 */
   "low-limit",                        /* 59 */
   "manipulated-variable-reference",   /* 60 */
   "maximum-output",                   /* 61 */
   "max-apdu-length-accepted",         /* 62 */
   "max-info-frames",                  /* 63 */
   "max-master",                       /* 64 */
   "max-pres-value",                   /* 65 */
   "minimum-off-time",                 /* 66 */
   "minimum-on-time",                  /* 67 */
   "minimum-output",                   /* 68 */
   "min-pres-value",                   /* 69 */
   "model-name",                       /* 70 */
   "modification-date",                /* 71 */
   "notify-type",                      /* 72 */
   "number-of-apdu-retries",           /* 73 */
   "number-of-states",                 /* 74 */
   "object-identifier",                /* 75 */
   "object-list",                      /* 76 */
   "object-name",                      /* 77 */
   "object-property-reference",        /* 78 zhu zhenhua 2003-7-24 */
   "object-type",                      /* 79 */
   "optional",                         /* 80 */
   "out-of-service",                   /* 81 */
   "output-units",                     /* 82 */
   "event-parameters",                 /* 83 */
   "polarity",                         /* 84 */
   "present-value",                    /* 85 */
   "priority",                         /* 86 */
   "priority-array",                   /* 87 */
   "priority-for-writing",             /* 88 */
   "process-identifier",               /* 89 */
   "program-change",                   /* 90 */
   "program-location",                 /* 91 */
   "program-state",                    /* 92 */
   "proportional-constant",            /* 93 */
   "proportional-constant-units",      /* 94 */
   "protocol-conformance-class",       /* 95 */
   "protocol-object-types-supported",  /* 96 */
   "protocol-services-supported",      /* 97 */
   "protocol-version",                 /* 98 */
   "read-only",                        /* 99 */
   "reason-for-halt",                  /* 100 */
   "recipient",                        /* 101 */
   "recipient-list",                   /* 102 */
   "reliability",                      /* 103 */
   "relinquish-default",               /* 104 */
   "required",                         /* 105 */
   "resolution",                       /* 106 */
   "segmentation-supported",           /* 107 */
   "setpoint",                         /* 108 */
   "setpoint-reference",               /* 109 */
   "state-text",                       /* 110 */
   "status-flags",                     /* 111 */
   "system-status",                    /* 112 */
   "time-delay",                       /* 113 */
   "time-of-active-time-reset",        /* 114 */
   "time-of-state-count-reset",        /* 115 */
   "time-synchronization-recipients",  /* 116 */
   "units",                            /* 117 */
   "update-interval",                  /* 118 */
   "utc-offset",                       /* 119 */
   "vendor-identifier",                /* 120 */
   "vendor-name",                      /* 121 */
   "vt-classes-supported",             /* 122 */
   "weekly-schedule",                  /* 123 */   
   "attempted-samples",                /* 124 */
   "average-value",                    /* 125 */
   "buffer-size",                      /* 126 */
   "client-cov-increment",             /* 127 */
   "cov-resubscription-interval",      /* 128 */
   "unused-was-current-notify-time",   /* 129  added by zhu zhenhua, 2004-5-11 deleded in version 1 rev 3 */
   "event-time-stamps",                /* 130 */
   "log-buffer",                       /* 131 */
   "log-device-object-property",       /* 132 zhu zhenhua 2003-7-24 */
   "enable",                           /* 133 changed from log-enable in 135-2004b-5 */
   "log-interval",                     /* 134 */
   "maximum-value",                    /* 135 */
   "minimum-value",                    /* 136 */
   "notification-threshold",           /* 137 */
   "unused-was-previous-notify-time",  /* 138   added by zhu zhenhua, 2004-5-11 deleted in version 1 rev 3 */
   "protocol-revision",                /* 139 */
   "records-since-notification",       /* 140 */
   "record-count",                     /* 141 */
   "start-time",                       /* 142 */
   "stop-time",                        /* 143 */
   "stop-when-full",                   /* 144 */
   "total-record-count",               /* 145 */            
   "valid-samples",                    /* 146 */
   "window-interval",                  /* 147 */
   "window-samples",                   /* 148 */
   "maximum-value-timestamp",          /* 149 */
   "minimum-value-timestamp",          /* 150 */
   "variance-value",                   /* 151 */
   "active-cov-subscriptions",          /* 152 xiao shiyuan 2002-7-18 */
   "backup-failure-timeout",            /* 153 xiao shiyuan 2002-7-18 */		
   "configuration-files",               /* 154 xiao shiyuan 2002-7-18 */
   "database-revision",                 /* 155 xiao shiyuan 2002-7-18 */
   "direct-reading",                    /* 156 xiao shiyuan 2002-7-18 */
   "last-restore-time",					/* 157 xiao shiyuan 2002-7-18 */
   "maintenance-required",				/* 158 xiao shiyuan 2002-7-18 */
   "member-of",							/* 159 xiao shiyuan 2002-7-18 */
   "mode",								/* 160 xiao shiyuan 2002-7-18 */
   "operation-expected",				/* 161 xiao shiyuan 2002-7-18 */
   "setting",							/* 162 xiao shiyuan 2002-7-18 */
   "silenced",							/* 163 xiao shiyuan 2002-7-18 */
   "tracking-value",					/* 164 xiao shiyuan 2002-7-18 */
   "zone-members",						/* 165 xiao shiyuan 2002-7-18 */
   "life-safety-alarm-values",			/* 166 xiao shiyuan 2002-7-18 */
   "max-segments-accepted",				/* 167 xiao shiyuan 2002-7-18 */
   "profile-name",                      /* 168 xiao shiyuan 2002-7-18 */
   "auto-slave-discovery",				/* 169 ljt 2005-10-12   */
   "manual-slave-address-binding",		/* 170 ljt 2005-10-12   */
   "slave-address-binding",				/* 171 ljt 2005-10-12   */
   "slave-proxy-enable",				/* 172 ljt 2005-10-12   */
   "last-notify-record",				/* 173 zhu zhenhua  2004-5-11 */
   "schedule-default",                 // 174 shiyuan xiao 7/15/2005
   "accepted-modes",                   // 175 shiyuan xiao 7/15/2005
   "adjust-value",                     // 176 shiyuan xiao 7/15/2005
   "count",                            // 177 shiyuan xiao 7/15/2005 
   "count-before-change",              // 178 shiyuan xiao 7/15/2005
   "count-change-time",                // 179 shiyuan xiao 7/15/2005		
   "cov-period",                       // 180 shiyuan xiao 7/15/2005
   "input-reference",                  // 181 shiyuan xiao 7/15/2005
   "limit-monitoring-interval",        // 182 shiyuan xiao 7/15/2005
   "logging-device",                   // 183 shiyuan xiao 7/15/2005
   "logging-record",                   // 184 shiyuan xiao 7/15/2005  
   "prescale",                         // 185 shiyuan xiao 7/15/2005  
   "pulse-rate",                       // 186 shiyuan xiao 7/15/2005
   "scale",                            // 187 shiyuan xiao 7/15/2005
   "scale-factor",                     // 188 shiyuan xiao 7/15/2005  
   "update-time",                      // 189 shiyuan xiao 7/15/2005 
   "value-before-change",              // 190 shiyuan xiao 7/15/2005
   "value-set",                        // 191 shiyuan xiao 7/15/2005
   "value-change-time",                 // 192 shiyuan xiao 7/15/2005
   // added addendum b (135-2004)
	"align-intervals",					// 193
	"group-members-names",				// 194
	"interval-offset",					// 195
	"last-restart-reason",				// 196
	"logging-type",						// 197
	"member-status-flags",				// 198
	"notification-period",				// 199
	"previous-notify-record",			// 200
	"requested-update-interval",		// 201
	"restart-notification-recipients",	// 202
	"time-of-device-restart",			// 203
	"time-synchronization-interval",	// 204
	"trigger",							// 205
	"utc-time-syncrhonization-recipients",  // 206
	// added by addenda d
	"node-subtype",						// 207
	"node-type",						// 208
	"structured-object-list",			// 209
	"subordinate-annotations",			// 210
	"subordinate-list",					// 211
	// added by addendum e 135-2004
	"actual-shed-level",				// 212
	"duty-window",						// 213
	"expected-shed-level",				// 214
	"full-duty-baseline",				// 215
    /* enumerations 216-217 are used in Addendum i to ANSI/ASHRAE 135-2004 */
    "blink-priority-threshold",			// 216
    "blink-time",						// 217
	"requested-shed-level",				// 218
	"shed-duration",					// 219
	"shed-level-descriptions",			// 220
	"shed-levels",						// 221
	"state-description",				// 222
	/* enumerations 223-225 are used in Addendum i to ANSI/ASHRAE 135-2004 */
	"fade-time",
	"lighting-command",
	"lighting-command-priority",
    /* enumerations 226-235 are used in Addendum f to ANSI/ASHRAE 135-2004 */
	"door-alarm-state",					// 226
	"door-extended-pulse-time",
	"door-members",
	"door-open-too-long-time",
	"door-pulse-time",					// 230
	"door-status",
	"door-unlock-delay-time",
	"lock-status",
	"masked-alarm-values",
	"secured-status",					// 235 last in 135-2008
	// Contributions from the bacnet-stack project http://sourceforge.net/projects/bacnet/develop :
	/* enumerations 236-243 are used in Addendum i to ANSI/ASHRAE 135-2004 */
	"off-delay",
	"on-delay",
	"power",
	"power-on-value",
	"progress-value",
	"ramp-rate",
	"step-increment",
	"system-failure-value",
	/* enumerations 244-311 are used in Addendum j to ANSI/ASHRAE 135-2004 */
	"absentee-limit",
	"access-alarm-events",
	"access-doors",
	"access-event",
	"access-event-authentication-factor",
	"access-event-credential",
	"access-event-time",
	"access-transaction-events",
	"accompaniment",
	"accompaniment-time",
	"activation-time",
	"active-authentication-policy",
	"assigned-access-rights",
	"authentication-factors",
	"authentication-policy-list",
	"authentication-policy-names",
	"authentication-status",
	"authorization-mode",
	"belongs-to",
	"credential-disable",
	"credential-status",
	"credentials",
	"credentials-in-zone",
	"days-remaining",
	"entry-points",
	"exit-points",
	"expiry-time",
	"extended-time-enable",
	"failed-attempt-events",
	"failed-attempts",
	"failed-attempts-time",
	"last-access-event",
	"last-access-point",
	"last-credential-added",
	"last-credential-added-time",
	"last-credential-removed",
	"last-credential-removed-time",
	"last-use-time",
	"lockout",
	"lockout-relinquish-time",
	"master-exemption",
	"max-failed-attempts",
	"members",
	"muster-point",
	"negative-access-rules",
	"number-of-authentication-policies",
	"occupancy-count",
	"occupancy-count-adjust",
	"occupancy-count-enable",
	"occupancy-exemption",
	"occupancy-lower-limit",
	"occupancy-lower-limit-enforced",
	"occupancy-state",
	"occupancy-upper-limit",
	"occupancy-upper-limit-enforced",
	"passback-exemption",
	"passback-mode",
	"passback-timeout",
	"positive-access-rules",
	"reason-for-disable",
	"supported-formats",
	"supported-format-classes",
	"threat-authority",
	"threat-level",
	"trace-flag",
	"transaction-notification-class",
	"user-external-identifier",
	"user-information-reference",
	/* enumerations 312-313 are used in Addendum k to ANSI/ASHRAE 135-2004 */
	"character-set",
	"strict-character-mode",

   /* enumerations 314-316 are used in Addendum ? */
   "prop-id-314",
	"prop-id-315",
	"prop-id-316",

   /* enumerations 317-323 are used in Addendum j to ANSI/ASHRAE 135-2004 */
	"user-name",
	"user-type",
	"uses-remaining",
	"zone-from",
	"zone-to",
	"access-event-tag",
	"global-identifier",
	/* enumerations 324-325 are used in Addendum i to ANSI/ASHRAE 135-2004 */
	"binary-active-value",
	"binary-inactive-value",
    /* enumeration 326 is used in Addendum j to ANSI/ASHRAE 135-2004 */
	"verification-time",
	/* enumerations 327-341 are used in Addendum ? */
	"prop-id-327",
	"prop-id-328",
	"prop-id-329",
	"prop-id-330",
	"prop-id-331",
	"prop-id-332",
	"prop-id-333",
	"prop-id-334",
	"prop-id-335",
	"prop-id-336",
	"prop-id-337",

	/* enumerations 338-341 are used in Addendum n to ANSI/ASHRAE 135-2008 */
	"backup-and-restore-state",   // 338
	"backup-preparation-time",
	"restore-completion-time",
	"restore-preparation-time",

   /* enumerations 342-344 are defined in Addendum 2008-w */	
	"bit-mask",
	"bit-text",
	"is-utc",		// 344

   /* enumerations 345-xxx are used in Addendum ? */
	//"prop-id-345",
	//"prop-id-346",
	//"prop-id-347",
	//"prop-id-348",
	//"prop-id-349",

	// CAUTION: if you add a property here, you must also change MAX_PROP_ID
	// (which is actually max-plus-one: the NUMBER of defined properties)
};
BAC_STRINGTABLE_EX(BACnetPropertyIdentifier, 512, 0x7FFFFFFF);

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
BAC_STRINGTABLE_EX(BACnetPropertyStates, 64, 256);

STRING_TABLE BACnetReliability[] = {
   "no-fault-detected",
   "no-sensor",
   "over-range",
   "under-range",
   "open-loop",
   "shorted-loop",
   "no-output",
   "unreliable-other",
   "process-error",
   "multi-state-fault",
   "configuration-error", // 10
   // added addendum B (135-2004)
   "member-fault",
   "communication-failure",	// 12 last in 135-2008
};
BAC_STRINGTABLE_EX(BACnetReliability, 64, 65536);

STRING_TABLE BACnetRestartReason[] = {
	"unknown",
	"coldstart",
	"warmstart",
	"detected-power-lost",
	"detected-power-off",
	"hardware-watchdog",
	"software-watchdog",
	"suspended",			// 7 last in 135-2008
};
BAC_STRINGTABLE_EX(BACnetRestartReason, 64, 256);

STRING_TABLE BACnetResultFlags[] = {
   "first-item",
   "last-item",
   "more-items"
};
BAC_STRINGTABLE(BACnetResultFlags);

STRING_TABLE BACnetShedState[] = {
	"shed-inactive",
	"shed-request-pending",
	"shed-compliant",
	"shed-non-compliant",
};
BAC_STRINGTABLE(BACnetShedState);

STRING_TABLE BACnetSegmentation[] = {
   "segmented-both",
   "segmented-transmit",
   "segmented-receive",
   "no-segmentation"
};
BAC_STRINGTABLE(BACnetSegmentation);

// This list is also used to create the hash table for scripting.
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
   "GetEventInformation"            /* 39 last in 135-2008 */
};                       
BAC_STRINGTABLE(BACnetServicesSupported);

STRING_TABLE  BACnetSilencedState[] = {
	"unsilenced",
	"audible-silenced",
	"visible-silenced",
	"all-silenced"
};
BAC_STRINGTABLE_EX(BACnetSilencedState, 64, 65536);

STRING_TABLE BACnetStatusFlags[] = {
   "in-alarm",
   "fault",
   "overridden",
   "out-of-service"
};
BAC_STRINGTABLE(BACnetStatusFlags);

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
BAC_STRINGTABLE_EX(BACnetVendorID, 0x7FFFFFFF, 0x7FFFFFFF);

STRING_TABLE BACnetVTClass[] = {
   "default-terminal",
   "ansi-x3-64",
   "dec-vt52",
   "dec-vt100",
   "dec-vt220",
   "hp-700-94",
   "ibm-3130"
};
BAC_STRINGTABLE_EX(BACnetVTClass, 64, 65536);

// Used for bitstring BACnetDaysOfWeek, where Monday == 0
// See also day_of_week
STRING_TABLE BACnetDaysOfWeek[] = {
   "monday",
   "tuesday",
   "wednesday",
   "thursday",
   "friday",
   "saturday",
   "sunday"
};
BAC_STRINGTABLE(BACnetDaysOfWeek);

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
   "ASHRAE-Reserved",
   "Vendor-Proprietary-Message"
};
BAC_STRINGTABLE(NL_msgs);

// This list is also used to create the hash table for scripting.
STRING_TABLE BACnetReject[] = {
   "other",                              /*0*/
   "buffer-overflow",                    /*1*/
   "inconsistent-parameters",            /*2*/
   "invalid-parameter-data-type",        /*3*/
   "invalid-tag",                        /*4*/
   "missing-required-parameter",         /*5*/
   "parameter-out-of-range",             /*6*/
   "too-many-arguments",                 /*7*/
   "undefined-enumeration",              /*8*/
   "unrecognized-service"                /*9*/
};
BAC_STRINGTABLE_EX(BACnetReject, 64, 256);

// This list is also used to create the hash table for scripting.
STRING_TABLE BACnetAbort[] = {
   "other",                             /*0*/
   "buffer-overflow",                   /*1*/
   "invalid-apdu-in-this-state",        /*2*/
   "preempted-by-higher-priority-task", /*3*/
   "segmentation-not-supported"         /*4*/
};
BAC_STRINGTABLE_EX(BACnetAbort, 64, 256);

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

// These have initial upper case for nicer display, since they
// are not actually used as an enumeration.
// This list is used to create the hash table for scripting.
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
   "TtOpen",                        /* 21 */
   "VtClose",                       /* 22 */
   "VtData",                        /* 23 */
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

// Not an explicit datatype: defined inline in ReinitializeDevice-Request
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

// Not an explicit datatype: defined inline in DeviceCommunicationsControl-Request
STRING_TABLE DeviceCommControl_Command[] = {
	"enable",	/* 0 */
	"disable",
	"disable-initiation"
};
BAC_STRINGTABLE(DeviceCommControl_Command);

// Not an explicit datatype: defined inline in ConfirmedTextMessage-Request
STRING_TABLE TextMessage_Priority[] = {
	"normal",	/* 0 */
	"urgent"
};
BAC_STRINGTABLE(TextMessage_Priority);


STRING_TABLE BACnetBackupState[] = {
   "idle",
   "preparing-for-backup",
   "preparing-for-restore",
   "performing-a-backup",
   "performing-a-restore",
   "backup-failure",
   "restore-failure"
};
BAC_STRINGTABLE(BACnetBackupState);

} // end namespace NetworkSniffer
