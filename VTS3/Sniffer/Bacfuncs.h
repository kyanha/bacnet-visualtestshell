   /*   ------ BACnet base type character arrays --------------- */

char *BACnetAction[] = {
   "DIRECT",
   "REVERSE"
   };

char *BACnetActionCommand[] = {
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

char *BACnetAddress[] = {
   "Network number",
   "MAC address"
   };

char *BACnetAddressBinding[] ={
   "Device Object Identifier",
   "Device Address"
};

char *BACnetBinaryPV[] = {
   "INACTIVE",
   "ACTIVE"
   };

char *BACnetCalendarEntry[] = {
   "Date",
   "Date Range",
   "WeekNDay"
    };

char *BACnetDateRange[] = {
   "Start Date",
   "End Date"
   };

char *BACnetDateTime[] = {
   "Date",
   "Time"
   };

char *BACnetDaysOfWeek[] = {
   "Monday",
   "Tuesday",
   "Wednesday",
   "Thursday",
   "Friday",
   "Saturday",
   "Sunday"
   };

char *BACnetDestination[] = {
   "Valid Days",
   "From Time",
   "To Time",
   "Recipient",
   "Process Identifier",
   "Issue Confirmed Notifications",
   "Transitions"
   };

char *BACnetDeviceStatus[] = {
   "Operational",
   "Operational-read-only",
   "Download-required",
   "Download-in-progress",
   "Non-Operational"
   };

char *BACnetEngineeringUnits[] = {
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

   "Invalid Units",     /* 115 */
   "Invalid Units",     /* 116 */
   "Invalid Units",     /* 117 */
   "Invalid Units",     /* 118 */
   "Invalid Units",     /* 119 */
   "Invalid Units",     /* 120 */
   "Invalid Units",     /* 121 */

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
   };

char *BACnetError[] = {
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
   "RequestKey Error Choice"
   };

char *BACnetErrorClass[] = {
   "Device",
   "Object",
   "Property",
   "Resources",
   "Security",
   "Services",
   "VT"
   };

char *BACnetErrorCode[] = {
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
   "Invalid-array-index"                 /* 42 */
   };

char *BACnetEventParameter[] = {
   "Change of Bitstring",
   "Change of State",
   "Change of Value",
   "Command Failure",
   "Floating Limit",
   "Out of Range"
   };

char *BACnetEventState[] = {
   "NORMAL",
   "FAULT",
   "OFFNORMAL",
   "HIGH-LIMIT",
   "LOW-LIMIT",
   };

char *BACnetEventTransitionBits[] = {
   "TO-OFFNORMAL",
   "TO-FAULT",
   "TO-NORMAL",
   };

char *BACnetEventType[] = {
   "CHANGE-OF-BITSTRING",
   "CHANGE-OF-STATE",
   "CHANGE-OF-VALUE",
   "COMMAND-FAILURE",
   "FLOATING-LIMIT",
   "OUT-OF-RANGE",
   };

char *BACnetFileAccessMethod[] ={
   "RECORD-ACCESS",
   "STREAM-ACCESS",
   "RECORD-AND-STREAM-ACCESS"
   };

char *BACnetLimitEnable[] = {
   "LOW-LIMIT-ENABLE",
   "HIGH-LIMIT-ENABLE"
   };

char *BACnetNotifyType[] = {
   "ALARM",
   "EVENT",
   "ACK_NOTIFICATION"
   };

char *BACnetObjectPropertyReference[] = {
   "Object Identifier",
   "Property Identifier",
   "Property Array Index"
   };

char *BACnetObjectPropertyValue[] = {
   "Object Identifier",
   "Property Identifier",
   "Property Array Index",
   "Value",
   "Priority"
   };

char *BACnetObjectType[] = {
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
   "SCHEDULE"               /* 17 */
   };

char *BACnetObjectTypesSupported[] = {
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
   "SCHEDULE"               /* 17 */
   };

char *BACnetPolarity[] = {
   "NORMAL",
   "REVERSE"
   };

char *BACnetProgramError[] = {
   "NORMAL",
   "LOAD-FAILED",
   "INTERNAL",
   "PROGRAM",
   "OTHER"
   };

char *BACnetProgramRequest[] = {
   "READY",
   "LOAD",
   "RUN",
   "HALT",
   "RESTART",
   "UNLOAD"
   };

char *BACnetProgramState[] = {
   "IDLE",
   "LOADING",
   "RUNNING",
   "WAITING",
   "HALTED",
   "UNLOADING"
   };   

char *BACnetPropertyIdentifier[] = {
   "Acked_Transitions property",                /* 0 */
   "Ack_Required Property",                     /* 1 */
   "Action property",                           /* 2 */
   "Action_Text property",                      /* 3 */
   "Active_Text property",                      /* 4 */
   "Active_VT_Sessions property",               /* 5 */
   "Alarm_Value property",                      /* 6 */
   "Alarm_Values property",                     /* 7 */
   "All property",                              /* 8 */
   "All_Writes_Successful property",            /* 9 */
   "APDU_Segment_Timeout property",             /* 10 */
   "APDU_Timeout property",                     /* 11 */
   "Application-software-version property",     /* 12 */
   "Archive property",                          /* 13 */
   "Bias property",                             /* 14 */
   "Change_Of_State_Count property",            /* 15*/
   "Change_Of_State_Time property",             /* 16 */
   "Notification_Class property",               /* 17  renamed in 2nd public review*/
   "Invalid Property Enumeration",              /* 18*/
   "Controlled_Variable_Reference property",    /* 19 */
   "Controlled_Variable_Units property",        /* 20 */
   "Controlled_Variable_Value property",        /* 21 */
   "COV_Increment property",                    /* 22 */
   "Datelist property",                         /* 23 */
   "Daylight_Savings_Status property",          /* 24 */
   "Deadband property",                         /* 25 */
   "Derivative_Constant property",              /* 26 */
   "Derivative_Constant_Units property",        /* 27 */
   "Description property",                      /* 28 */
   "Description_Of_Halt property",              /* 29 */
   "Device_Address_Binding property",           /* 30 */
   "Device_Type property",                      /* 31 */
   "Effective_Period property",                 /* 32 */
   "Elapsed_Active_Time property",              /* 33 */
   "Error_Limit property",                      /* 34 */
   "Event_Enable property",                     /* 35 */
   "Event_State property",                      /* 36 */
   "Event_Type property",                       /* 37 */
   "Exception_Schedule property",               /* 38 */
   "Fault_Values property",                     /* 39 */
   "Feedback_Value property",                   /* 40 */
   "File_Access_Method property",               /* 41 */
   "File_Size property",                        /* 42 */
   "File_Type property",                        /* 43 */
   "Firmware_Revision property",                /* 44 */
   "High_Limit property",                       /* 45 */
   "Inactive_Text property",                    /* 46 */
   "In_Process property",                       /* 47 */
   "Instance_Of property",                      /* 48 */
   "Integral_Constant property",                /* 49 */
   "Integral_Constant_Units property",          /* 50 */
   "Issue_Confirmed_Notifications property",    /* 51 */
   "Limit_Enable property",                     /* 52 */
   "List_Of_Group_Members property",            /* 53 */
   "List_Of_Object_Property_References property",  /* 54 */
   "List_Of_Session_Keys property",             /* 55 */
   "Local_Date property",                       /* 56 */
   "Local_Time property",                       /* 57 */
   "Location property",                         /* 58 */
   "Low_Limit property",                        /* 59 */
   "Manipulated_Variable_Reference property",   /* 60 */
   "Maximum_Output property",                   /* 61 */
   "Max_Apdu_Length_Accepted property",         /* 62 */
   "Max_Info_Frames property",                  /* 63 */
   "Max_Master property",                       /* 64 */
   "Max_Pres_Value property",                   /* 65 */
   "Minimum_Off_Time property",                 /* 66 */
   "Minimum_On_Time property",                  /* 67 */
   "Minimum_Output property",                   /* 68 */
   "Min_Pres_Value property",                   /* 69 */
   "Model_Name property",                       /* 70 */
   "Modification_Date property",                /* 71 */
   "Notify_Type property",                      /* 72 */
   "Number_Of_APDU_Retries",                    /* 73 */
   "Number_Of_States property",                 /* 74 */
   "Object_Identifier property",                /* 75 */
   "Object_List property",                      /* 76 */
   "Object_Name property",                      /* 77 */
   "Object_Property_Reference property",        /* 78 */
   "Object_Type property",                      /* 79 */
   "Optional property",                         /* 80 */
   "Out_Of_Service property",                   /* 81 */
   "Output_Units property",                     /* 82 */
   "Event-Parameters property",                 /* 83 */
   "Polarity property",                         /* 84 */
   "Present_Value property",                    /* 85 */
   "Priority property",                         /* 86 */
   "Priority_Array property",                   /* 87 */
   "Priority_For_Writing property",             /* 88 */
   "Process_Identifier property",               /* 89 */
   "Program_Change property",                   /* 90 */
   "Program_Location property",                 /* 91 */
   "Program_State property",                    /* 92 */
   "Proportional_Constant property",            /* 93 */
   "Proportional_Constant_Units property",      /* 94 */
   "Protocol_Conformance_Class property",       /* 95 */
   "Protocol_Object_Types_Supported property",  /* 96 */
   "Protocol_Services_Supported property",      /* 97 */
   "Protocol_Version property",                 /* 98 */
   "Read_Only property",                        /* 99 */
   "Reason_For_Halt property",                  /* 100 */
   "Recipient property",                        /* 101 */
   "Recipient_List property",                   /* 102 */
   "Reliability property",                      /* 103 */
   "Relinquish_Default property",               /* 104 */
   "Required property",                         /* 105 */
   "Resolution property",                       /* 106 */
   "Segmentation_Supported property",           /* 107 */
   "Setpoint property",                         /* 108 */
   "Setpoint_Reference property",               /* 109 */
   "State_Text property",                       /* 110 */
   "Status_Flags property",                     /* 111 */
   "System_Status property",                    /* 112 */
   "Time_Delay property",                       /* 113 */
   "Time_Of_Active_Time_Reset property",        /* 114 */
   "Time_Of_State_Count_Reset property",        /* 115 */
   "Time_Synchronization_Recipients property",  /* 116 */
   "Units property",                            /* 117 */
   "Update_Interval property",                  /* 118 */
   "UTC_Offset property",                       /* 119 */
   "Vendor_Identifier property",                /* 120 */
   "Vendor_Name property",                      /* 121 */
   "Vt_Classes_Supported property",             /* 122 */
   "Weekly_Schedule property",                  /* 123 */
   };

char *BACnetPropertyReference[] = {
   "Property Identifier",
   "Property Array Index"
   };

char *BACnetPropertyStates[] = { 
   "Boolean-value",
   "Binary-value"
   "Event-type",
   "Polarity",
   "Program-change",
   "Program-state",
   "Reason-for-halt",
   "Reliability",
   "State",
   "System-status",
   "Units"
   };

char *BACnetPropertyValue[] = {
   "Property Identifier",
   "Property Array Index",
   "Value",
   "Priority"
   };

char *BACnetRecipient[] = {
   "Device",
   "Address"
   };

char *BACnetRecipientProcess[] = {
   "Recipient",
   "Process Identifier"
   };

char *BACnetReliability[] = {
   "NO-FAULT-DETECTED",
   "NO-SENSOR",
   "OVER-RANGE",
   "UNDER-RANGE",
   "OPEN-LOOP",
   "SHORTED-LOOP",
   "NO-OUTPUT",
   "UNRELIABLE-OTHER",
   "PROCESS-ERROR"
   };

char *BACnetSegmentation[] = {
   "SEGMENTED-BOTH",
   "SEGMENTED-TRANSMIT",
   "SEGMENTED-RECEIVE",
   "NO-SEGMENTATION"
   };

char *BACnetServicesSupported[] = {
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
   "Who-Is"                         /* 34 */
   };

char *BACnetSessionKey[] = {
   "Session Key",
   "Peer Address"
   };

char *BACnetSpecialEvent[] = {
   "Period (CalendarEntry)",
   "Period (CalendarReference)",
   "ListOfTimeValues",
   "EventPriority"
   };

char *BACnetStatusFlags[] = {
   "IN-ALARM",
   "FAULT",
   "OVERRIDDEN",
   "OUT_OF_SERVICE"
   };

char *BACnetTimeStamp[] = {
   "Time",
   "Sequence Number",
   "Date-Time"
   };

char *BACnetVendorID[] = {
   "ASHRAE",
   "NIST",
   "Trane",
   "McQuay",
   "PolarSoft",
   "Johnson Controls",
   "Auto-Matrix",
   "Staefa",
   "Delta Controls",
   "Landis & Gyr",
   "Andover Controls",
   "Siebe",
   "Orion Analysis",
   "Teletrol",
   "Cimetrics Technology",
   "Cornell University",
   "Carrier",
   "Honeywell",
   "Alerton",
   "Tour & Andersson",
   "Hewlett-Packard",
   "Dorsette's Inc.",
   "Cerberus AG",
   "York",
   "Automated Logic",
   "Control Systems International",
   "Phoenix Controls Corporation",
   "Innovex",
   "KMC Controls"
   };

char *BACnetTimeValue[] = {
   "Time",
   "Value"
   };

char *BACnetVTClass[] ={
   "Default Terminal class",
   "ANSI X3.64 class",
   "DEC VT52 class",
   "DEC VT100 class",
   "DEC VT220 class",
   "HP 700/94 class",
   "IBM 3130 class"
   };

char *BACnetVTSession[] = {
   "Local VT-Session ID",
   "Remote VT-Session ID",
   "Remote VT-Address"
   };

char *BACnetWeekNDay[] = {
   "Month",
   "Week of Month",
   "Day of Week"
   };

char *day_of_week[] = {
   "Invalid Day",  /* 0 */
   "Monday",       /* 1 */
   "Tuesday",      /* 2 */
   "Wednesday",    /* 3 */
   "Thursday",     /* 4 */
   "Friday",       /* 5 */
   "Saturday",     /* 6 */
   "Sunday"        /* 7 */
   };

char *month[] = {
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
   "December"       /* 12 */
   };

char *PDU_types[] = {
   "Confirmed Service",
   "Unconfirmed Service",
   "Simple ACK",
   "Complex ACK",
   "Segment ACK",
   "Error",
   "Reject",
   "Abort"
   };

char *NL_msgs[] = {
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

char *BACnetReject[] = {
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

char *BACnetAbort[] = {
   "Other",                             /*0*/
   "Buffer-overflow",                   /*1*/
   "Invalid-APDU-in-this-state",        /*2*/
   "Preempted-by-higher-priority-task", /*3*/
   "Segmentation-not-supported"         /*4*/
   };

char *Relation_Specifier[] = {
   "Equal (=)",
   "Not Equal (!=)",
   "Less Than (<)",
   "Greater Than (>)",
   "Less Than or Equal (<=)",
   "Greater Than or Equal (>=)"
   };

char *BVLL_Function[] = {
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

/**************************************************************************/
/* The functions that follow are used to complement the PIF functions     */
/* supplied by Network General. There primary purpose is to allow greater */
/* flexibility in displaying the various BACnet constructs.        */
/**************************************************************************/

/*************************************************************************/
void bac_show_byte ( char *label, char *format_str )
/*************************************************************************/
/* Advantage: Allows FW alignment */
{
   sprintf(outstr,"%"FW"s = ",label); /* Set up alignment of output */
   strcat(outstr,format_str);
   sprintf(get_int_line(pi_data_current,pif_offset,1),outstr,(unsigned char)pif_get_byte(0));
   pif_offset += 1;
}

const char* LookupName( int net, const unsigned char *addr, int len );

/*************************************************************************/
void bac_show_enetaddr ( char *label )
/*************************************************************************/
{
   const unsigned char *addr = (const unsigned char *)msg_origin + pif_offset
   ;
   const char	*name
   ;

   /* check for a broadcast */
   if (memcmp(addr,"\377\377\377\377\377\377",6) == 0)
      name = LookupName( 0, 0, 0 );
   else
      name = LookupName( 0, addr, 6 );

   sprintf(outstr,"%"FW"s = %%02X-%%02X-%%02X-%%02X-%%02X-%%02X",label); /* Set up alignment of output */
   sprintf(get_int_line(pi_data_current,pif_offset,6), outstr
      , pif_get_byte(0)
      , pif_get_byte(1)
      , pif_get_byte(2)
      , pif_get_byte(3)
      , pif_get_byte(4)
      , pif_get_byte(5)
      );
   pif_offset += 6;

   if (name) pif_append_ascii( ", %s", name );
}

/*************************************************************************/
void bac_show_bipaddr ( char *label )
/*************************************************************************/
{
   const char	*name = LookupName( 0, (const unsigned char *)msg_origin + pif_offset, 6 )
   ;

   sprintf(outstr,"%"FW"s = %%u.%%u.%%u.%%u:0x%%04X",label); /* Set up alignment of output */
   sprintf(get_int_line(pi_data_current,pif_offset,6), outstr
      , pif_get_byte(0)
      , pif_get_byte(1)
      , pif_get_byte(2)
      , pif_get_byte(3)
      , (unsigned short)pif_get_word_hl(4)
      );
   pif_offset += 6;

   if (name) pif_append_ascii( ", %s", name );
}

/*************************************************************************/
void bac_show_ctag_flag ( void )
/*************************************************************************/
/* Advantage: Allows FW alignment and extraction of tag value as [X] */
{
   unsigned char x;

   x = pif_get_byte(0);
   if((x & 0x06) == 0x06)  /* paired delimiter tag */
      sprintf(outstr,"%"FW"s = X'%02X' = [%u]","PD Context Specific Tag",
      x,((x&0xF0)>>4));
   else  /* single delimiter tag */
      sprintf(outstr,"%"FW"s = X'%02X' = [%u]","SD Context Specific Tag",
      x,((x&0xF0)>>4));

   sprintf(get_int_line(pi_data_current,pif_offset,1),outstr);
   pif_flagbit_indent = strcspn(outstr,"=") - 10;
   if (pif_flagbit_indent < 0) pif_flagbit_indent = 0;
   pif_offset += 1;
}

/*************************************************************************/
void bac_show_flag ( char outstr[80], unsigned char mask )
/*************************************************************************/
/* Advantage: Allows flags to be shown with format specifiers other than
   just %d. Thus flags can be displayed in hex. */
{
   unsigned char x;
   x = pif_get_byte(0) & mask;
   sprintf(get_int_line(pi_data_current,pif_offset,1),outstr,x);
   pif_flagbit_indent = strcspn(outstr,"=") - 10;
   if (pif_flagbit_indent < 0) pif_flagbit_indent = 0;
   pif_offset += 1;
}

/**************************************************************************/
void bac_show_nbytes( unsigned int len, char *str )
/**************************************************************************/
/* Advantage: Highlights n bytes without 99 byte limitation of
   pif_show_nbytes_hex. Does not do any interpretation */
{
   sprintf(get_int_line(pi_data_current,pif_offset,len),str);
   pif_offset += len;
}

/*************************************************************************/
void bac_show_word_hl ( char *label, char *format_str )
/*************************************************************************/
/* Advantage: Allows FW alignment */
{
  sprintf(outstr,"%"FW"s = ",label); /* Set up alignment of output */
  strcat(outstr,format_str);
  sprintf(get_int_line(pi_data_current,pif_offset,2),outstr,
     pif_get_word_hl(0));
  pif_offset += 2;
}

/*************************************************************************/
void bac_show_long_hl ( char *label, char *format_str )
/*************************************************************************/
/* Advantage: Allows FW alignment */
{
  sprintf(outstr,"%"FW"s = ",label); /* Set up alignment of output */
  strcat(outstr,format_str);
  sprintf(get_int_line(pi_data_current,pif_offset,4),outstr,
     pif_get_long_hl(0));
  pif_offset += 4;
}

/**************************************************************************/
void float_to_ascii( double x, char *outstr)
/**************************************************************************/
{
   sprintf( outstr, "%3.1f", x );
/*
	It looks like this horrible code is a way to get floating point
	formatting.  I recommend it be taken out and shot.
	
   int i,j;
   int dec,sign;
   char *instrg;

   instrg = fcvt(x,5,&dec,&sign);

   if (sign == 0)
      outstr[0] = '+';
   else
      outstr[0] = '-';

   for (i=0,j=1;i<strlen(instrg);i++,j++) {
      if (dec == i) {
        outstr[j] = '.';
        j += 1;
      }
      outstr[j] = instrg[i];
   }
   outstr[j] = 0;
*/
}

/*************************************************************************/
void show_str_eq_str ( char *str1, char *str2, unsigned int len )
/*************************************************************************/
{
  sprintf(get_int_line(pi_data_current,pif_offset,len),
     "%"FW"s = %s",str1,str2);
}

/*************************************************************************/
void bac_show_flagmask ( unsigned char mask, char outstr[80] )
/*************************************************************************/
/* Advantage: Allows the value of the masked-off field to be displayed
   in a formatted string to right of multi-bit field */
{
   unsigned char bitptr,i,j,startbit,numbits,val,saveval,firstbit;
   char strvar[80];

/* Save the byte that is being displayed.                                */

   saveval = pif_get_byte(-1);

/* This part of the routine prepends blanks to the output string to line */
/* things up with the previous pif_show_flagbit or pif_show_flagmask.    */

   for (i=0;i<pif_flagbit_indent;i++) strvar[i] = ' ';

/* This part of the routine evaluates the mask and finds the position of */
/* the starting '1' bit and the number of '1' bits. The MSB is bit 7.    */

   firstbit = 0;
   startbit = 0;
   numbits  = 0;
   val = mask;

   for (j=8;j>0;j--) {
      if ((val/(1<<(j-1)))==0) {
        if (firstbit) {
          numbits = startbit - (j-1);
          firstbit = 0;
        }
      }
      else {
         val=val-(1<<(j-1));
         if (!firstbit) {
            firstbit = 1;
            startbit = j-1;
         }
      }
      if ((j==1) && (firstbit))
         numbits = startbit + 1;
   }

/* This part of the routine takes the value to be placed in the bit field */
/* and encodes it as 0's and 1's, placing it in STRVAR in PIF format.     */
      
   bitptr = 7;
   while (bitptr > startbit) {
      if (bitptr == 3) {
         strvar[i] = ' ';
         i += 1;
      }
      strvar[i] = '.';
      bitptr -= 1;
      i += 1;
   }

/* saveval will now contain the value in the bit field, right shifted */

   saveval = ((saveval & mask)>>(8-((7-startbit)+numbits)));
   val = saveval;

   for (j=numbits;j>0;j--) {
      if (bitptr == 3) {
         strvar[i] = ' ';
         i += 1;
      }
      if ((val/(1<<(j-1)))==0) {
         strvar[i]='0';
      }
      else {
         strvar[i]='1';
         val=val-(1<<(j-1));
      }
      i += 1;
      bitptr -= 1;
   }
   for (j=8-((7-startbit)+numbits);j>0;j--) {
      if (bitptr == 3) {
         strvar[i] = ' ';
         i += 1;
      }
      strvar[i] = '.';
      i += 1;
      bitptr -= 1;
   }
   strvar[i] = '\0';
   strcat(strvar," = ");
   strcat(strvar,outstr);
   sprintf(get_int_line(pi_data_current,pif_offset-1,1),strvar,saveval);
}

/**************************************************************************/
void show_bac_ANY( int obj_type, unsigned int prop_id, int prop_idx)
/**************************************************************************/
{
   unsigned char x, unused;
   unsigned int len;
   unsigned int i,j,len1;

   x = pif_get_byte(0);
   switch (prop_id) {
      case ACKED_TRANSITIONS:  /* bit string */
           show_bac_event_transitions_ackd();
           break;
      case ACK_REQUIRED:  /* bit string */
           show_bac_event_transitions_ackd();
           break;
      case ACTION: switch (obj_type) {
/***         Command Object: ARRAY OF (LIST OF BACnetActionCommands)
                     case 8: /* command object 
                        if (prop_idx == 0) {
                          pif_show_ascii(0,"Array Size");
                          show_application_data(x);
                          };
                        while ((pif_get_byte(0) & 0x0f) != 0x0f) {
                          show_bac_action_command(len);
                          };
                        break;
***/
                     case 13: /* loop object */
                         len1 = show_application_tag(x);
                         show_str_eq_str("Control Action",
                         BACnetAction[pif_get_byte(0)],len1);
                         pif_offset += len1;
                     }
              break;
      case ACTION_TEXT:  /* ARRAY OF Character Strings */
           if (prop_idx == 0) {
             show_application_tag(x);
             bac_show_byte("Array Size","%u");
             }
           else
             while ((x&0x0f) != 0x0f) {
               /* handles single element or entire array */
               show_application_data(x);
               x = pif_get_byte(0);
               };
           break;
      case ACTIVE_TEXT:  /* Character String  */
           show_application_data(x);
           break;
      case ACTIVE_VT_SESSIONS:
           while ((pif_get_byte(0) & 0x0f) != 0x0f) show_bac_VT_session();
           break;
      case ALARM_VALUE:
           show_application_data(x);
           break;
      case ALARM_VALUES:
           while(x != 0x3f){
             show_application_data(x);
             x = pif_get_byte(0);
             };
           break;
      case ALL:  /* No Data */
           break;
      case ALL_WRITES_SUCCESSFUL:   /* Boolean  */
           show_application_data(x);
           break;
      case APDU_SEGMENT_TIMEOUT:  /* Unsigned */
           show_application_data(x);
           break;
      case APDU_TIMEOUT:  /* Unsigned */
           show_application_data(x);
           break;
      case APPLICATION_SOFTWARE_VERSION:  /* Character String  */
           show_application_data(x);
           break;
      case ARCHIVE:  /* Boolean */
           show_application_data(x);
           break;
      case BIAS:   /*  Real  */
           show_application_data(x);
           break;
      case CHANGE_OF_STATE_COUNT:  /*  Unsigned  */
           show_application_data(x);
           break;
      case CHANGE_OF_STATE_TIME:   /*  BACnet DateTime  */
           show_application_data(x);
           show_application_data(pif_get_byte(0));
           break;
      case CONTROLLED_VARIABLE_REFERENCE: /*  BACnetObjectPropertyReference  */
           show_bac_obj_prop_ref();
           break;
      case CONTROLLED_VARIABLE_UNITS: /*  BACnetEngineeringUnits  */
           show_application_tag(x);
           bac_show_byte(BACnetEngineeringUnits[pif_get_byte(0)],"%u");
           break;
      case CONTROLLED_VARIABLE_VALUE:  /*  Real  */
           show_application_data(x);
           break;
      case COV_INCREMENT:
           show_application_data(x);
           break;
      case DATELIST:  /* List of BACnetCalendarEntry  */
           while ((pif_get_byte(0) & 0x0f) != 0x0f) show_bac_calendar_entry();
           break;
      case DAYLIGHT_SAVINGS_STATUS:  /* Boolean  */
           show_application_data(x);
           break;
      case DEADBAND:
           show_application_data(x);
           break;
      case DERIVATIVE_CONSTANT: /* Real  */
           show_application_data(x);
           break;
      case DERIVATIVE_CONSTANT_UNITS: /* Enumeration  */
           show_application_tag(x);
           bac_show_byte(BACnetEngineeringUnits[pif_get_byte(0)],"%u");
           break;
      case DESCRIPTION: /* Character String  */
           show_application_data(x);
           break;
      case DESCRIPTION_OF_HALT: /* Character String */
           show_application_data(x);
           break;
      case DEVICE_TYPE: /*  Character String  */
           show_application_data(x);
           break;
      case DEVICE_ADDRESS_BINDING:  /* sequence of BACnetAddressBinding */
           while ((x & 0x0f) != 0x0f){
            show_application_tag(x);
            show_bac_object_identifier();
            pif_show_space();
            pif_show_ascii(0, "Device Address");
            show_bac_address();
            x = pif_get_byte(0);
            };
           break;
      case EFFECTIVE_PERIOD: /*  BACnetDateRAnge  */
           pif_show_ascii(0,"Start Date");
           show_application_data(x);
           pif_show_space();
           pif_show_ascii(0,"End Date");
           show_application_data(pif_get_byte(0));
           break;
      case ELAPSED_ACTIVE_TIME: /* Unsigned  */
           show_application_data(x);
           break;
      case ERROR_LIMIT:
           show_application_data(x);
           break;
      case EVENT_ENABLE: /* Bitstring  */
           len1 = show_application_tag(x);
           unused = pif_get_byte(0);
           bac_show_byte("Unused Bits in Last Octet","%u");
           j=0;
           for(i=0;i<((len1-1)*8-unused);i++) {
             if(!(i%8)) {
               x = pif_get_byte(0);
               sprintf(outstr,"Bit String Octet [%u]",j++);
               bac_show_byte(outstr,"X'%02X'");
               }
             sprintf(outstr,"   %s",BACnetEventTransitionBits[i]);
             pif_offset--;
             if(x&0x80)
               show_str_eq_str(outstr,"TRUE",1);
             else
               show_str_eq_str(outstr,"FALSE",1);
             pif_offset++;
             x <<= 1;
             }
           break;
      case EVENT_PARAMETERS: /* List of BACnetEventParameters  */
             show_bac_event_parameters();
           break;
      case EVENT_STATE:  /*  BACnetEventState */
           show_application_tag(x);
           bac_show_byte(BACnetEventState[pif_get_byte(0)],"%u");
           break;
      case EVENT_TYPE: /* BACnetEventType  */
           len1 = show_application_tag(x);
           show_str_eq_str("Event Type",BACnetEventType[pif_get_byte(0)],len1);
           pif_offset += len1;
           break;
      case EXCEPTION_SCHEDULE: /*  ARRAY of BACnet SpecialEvents  */
           if (prop_idx == 0) {
             show_application_tag(x);
             bac_show_byte("Array Size","%u");
             };
           while ((pif_get_byte(0) & 0x0f) != 0x0f) {
             show_bac_special_event();
             };
           break;
      case FAULT_VALUES:
          while(x != 0x3f){
             show_application_data(x);
             x = pif_get_byte(0);
             };
           break;
      case FEEDBACK_VALUE:
           show_application_tag(x);
           bac_show_byte(BACnetBinaryPV[pif_get_byte(0)],"%u");
           break;
      case FILE_ACCESS_METHOD:  /* Enumerated  */
           show_application_data(x);
           break;
      case FILE_SIZE: /*  Unsigned  */
           show_application_data(x);
           break;
      case FILE_TYPE: /*  Character String  */
           show_application_data(x);
           break;
      case FIRMWARE_REVISION: /* Character String  */
           show_application_data(x);
           break;
      case HIGH_LIMIT:
           show_application_data(x);
           break;
      case INACTIVE_TEXT: /*  Character String  */
           show_application_data(x);
           break;
      case IN_PROCESS: /* Boolean  */
           show_application_data(x);
           break;
      case INSTANCE_OF: /* Character String  */
           show_application_data(x);
           break;
      case INTEGRAL_CONSTANT: /* Real  */
           show_application_data(x);
           break;
      case INTEGRAL_CONSTANT_UNITS: /* Enumerated  */
           show_application_tag(x);
           bac_show_byte(BACnetEngineeringUnits[pif_get_byte(0)],"%u");
           break;
      case ISSUE_CONFIRMED_NOTIFICATIONS:
           show_application_data(x);
           break;
      case LIMIT_ENABLE:
           len1 = show_application_tag(x);
           unused = pif_get_byte(0);
           bac_show_byte("Unused Bits in Last Octet","%u");
           j=0;
           for(i=0;i<((len1-1)*8-unused);i++) {
             if(!(i%8)) {
               x = pif_get_byte(0);
               sprintf(outstr,"Bit String Octet [%u]",j++);
               bac_show_byte(outstr,"X'%02X'");
               }
             sprintf(outstr,"   %s",BACnetLimitEnable[i]);
             pif_offset--;
             if(x&0x80)
               show_str_eq_str(outstr,"TRUE",1);
             else
               show_str_eq_str(outstr,"FALSE",1);
             pif_offset++;
             x <<= 1;
             }
           break;
      case LIST_OF_GROUP_MEMBERS:
      case LIST_OF_OBJ_PROP_REFERENCES: /* List of ReadAccessSpecs  */
           while ((pif_get_byte(0) & 0x0f) != 0x0f)
               show_bac_read_access_spec();
           break;
/***/      case LIST_OF_SESSION_KEYS:
           break;
      case LOCAL_DATE: /* Date */
           show_application_data(x);
           break;
      case LOCAL_TIME: /*  Time  */
           show_application_data(x);
           break;
      case LOCATION: /* Character String  */
           show_application_data(x);
           break;
      case LOW_LIMIT:
           show_application_data(x);
           break;
      case MANIPULATED_VARIABLE_REFERENCE:
           show_bac_obj_prop_ref();
           break;
      case MAXIMUM_OUTPUT: /* Real  */
           show_application_data(x);
           break;
      case MAX_APDU_LENGTH_ACCEPTED:  /* Unsigned  */
           show_application_data(x);
           break;
      case MAX_PRES_VALUE:  /* Real  */
           show_application_data(x);
           break;
      case MINIMUM_OFF_TIME: /* Unsigned 32 */
           show_application_data(x);
           break;
      case MINIMUM_ON_TIME: /* Unsigned 32 */
           show_application_data(x);
           break;
      case MINIMUM_OUTPUT: /* Real  */
           show_application_data(x);
           break;
      case MIN_PRES_VALUE: /*  Real  */
           show_application_data(x);
           break;
      case MODEL_NAME:  /* Character String  */
           show_application_data(x);
           break;
      case MODIFICATION_DATE:  /*  BACnetDateTime  */
           show_application_data(x);
           show_application_data(pif_get_byte(0));
           break;
      case NOTIFICATION_CLASS:
           show_application_data(x);
           break;
      case NOTIFY_TYPE:
           show_application_tag(x);
           bac_show_byte(BACnetNotifyType[pif_get_byte(0)],"%u");
           break;
      case NUMBER_OF_APDU_RETRIES:  /*  Unsigned  */
           show_application_data(x);
           break;
      case NUMBER_OF_STATES: /*  Unsigned  */
           show_application_data(x);
           break;
      case OBJECT_ID: /*  BACnetObjectIdentifier  */
           show_application_tag(x);
           show_bac_object_identifier();
           break;
      case OBJECT_LIST:
           if (prop_idx == 0) {
             show_application_tag(x);
             bac_show_byte("Array Size","%u");
             break;
             };
           while ((x & 0x0f) != 0x0f) {
             len = show_application_tag(x);
             if(x != 0xc4){ /* invalid tag */
               pif_show_space();
               pif_show_ascii(0, "Error: Invalid Tag!");
               };
             show_bac_object_identifier();
             x = pif_get_byte(0);
             };
           break;
      case OBJECT_NAME:  /* Character String  */
           show_application_data(x);
           break;
      case OBJECT_PROPERTY_REFERENCE: /*  BACnetObjectPropertyReference  */
           show_bac_obj_prop_ref();
           break;
      case OBJECT_TYPE: /*  BACnetObjectType  */
           show_application_tag(x);
           bac_show_byte(BACnetObjectType[pif_get_byte(0)],"%u");
           break;
      case xOPTIONAL : break;  /* No data!  */
      case OUT_OF_SERVICE: /* Boolean  */
           show_application_data(x);
           break;
      case OUTPUT_UNITS: /* BACnetEngineeringUnits  */
           show_application_tag(x);
           bac_show_byte(BACnetEngineeringUnits[pif_get_byte(0)],"%u");
           break;
      case POLARITY: /* BACnetPolarity  */
           show_application_tag(x);
           bac_show_byte(BACnetPolarity[pif_get_byte(0)],"%u");
           break;
      case PRESENT_VALUE: /*  Data Type depends on Object Type! */
           switch (obj_type) {
               case 0:  /* Analog_Input - Real */
               case 1:  /* Analog_Output - Real */
               case 2:  /* Analog_Value - Real */
               case 8:  /* Command - Unsigned */
               case 13: /* Loop - Real */
               case 14: /* Multistate_Input - Unsigned */
               case 15: /* Multistate_Output - Unsigned */
               case 17: /* Schedule - ANY Primitive Type */
                        show_application_data(x);
                        break;
               case 3:  /* Binary_Input - BACnetBinaryPV */
               case 4:  /* Binary_Output - BACnetBinaryPV */
               case 5:  /* Binary_Value - BACnetBinaryPV */
               case 6:  /* Calendar - BACnetBinaryPV */
                        show_application_tag(x);
                        if(x != 0x00){ /* not a NULL */
                          bac_show_byte(BACnetBinaryPV[pif_get_byte(0)],"%u");
                          };
                        break;
               case 12: /* Group - List of Read Access Result */
                       show_bac_read_access_result();
                       break;
               default: pif_show_ascii(0,
                  "Unknown Data - Object Type does not have Present Value Property!");
               }
           break;
      case PRIORITY:
           if (prop_idx == 0) {
             show_application_tag(x);
             bac_show_byte("Array Size","%u");
             break;
             };
           while ((x & 0x0f) != 0x0f) {
             show_application_data(x);
             x = pif_get_byte(0);
             };
           break;
      case PRIORITY_ARRAY: /* BACnetPriorityArray  */
           if (prop_idx == 0) {
             show_application_tag(x);
             bac_show_byte("Array Size","%u");
             x = pif_get_byte(0);
             break;
             };
           while ((x & 0x0f) != 0x0f) {
             /* handles single element or entire array */
             if((obj_type == 4) || (obj_type == 5)){ /* BO or BV */
               if(x == 0x00) /* null */
                  show_application_data(x);
               else{
                  show_application_tag(x);
                  bac_show_byte(BACnetBinaryPV[pif_get_byte(0)],"%u");
                  }
               }
             else
               show_application_data(x);
             x = pif_get_byte(0);
             };
           break;
      case PRIORITY_FOR_WRITING: /* Unsigned  */
           show_application_data(x);
           break;
      case PROCESS_IDENTIFIER:
           show_application_data(x);
           break;
      case PROGRAM_CHANGE: /*  BACnetProgramRequest  */
           show_application_tag(x);
           bac_show_byte(BACnetProgramRequest[pif_get_byte(0)],"%u");
           break;
      case PROGRAM_LOCATION: /* Character String  */
           show_application_data(x);
           break;
      case PROGRAM_STATE: /*  BACnetProgramState  */
           show_application_tag(x);
           bac_show_byte(BACnetProgramState[pif_get_byte(0)],"%u");
           break;
      case PROPORTIONAL_CONSTANT: /*  Real  */
           show_application_data(x);
           break;
      case PROPORTIONAL_CONSTANT_UNITS: /* BACnetEngineeringUnits  */
           show_application_tag(x);
           bac_show_byte(BACnetEngineeringUnits[pif_get_byte(0)],"%u");
           break;
      case PROTOCOL_CONFORMANCE_CLASS: /*  Unsigned  */
           show_application_data(x);
           break;
      case PROTOCOL_OBJECT_TYPES_SUPPORTED: /* BACnetObjectTypesSupported  */
           len1 = show_application_tag(x);
           unused = pif_get_byte(0);
           bac_show_byte("Unused Bits in Last Octet","%u");
           j=0;
           for(i=0;i<((len1-1)*8-unused);i++) {
             if(!(i%8)) {
               x = pif_get_byte(0);
               sprintf(outstr,"Bit String Octet [%u]",j++);
               bac_show_byte(outstr,"X'%02X'");
               }
             sprintf(outstr,"   %s",BACnetObjectTypesSupported[i]);
             pif_offset--;
             if(x&0x80)
               show_str_eq_str(outstr,"Supported",1);
             else
               show_str_eq_str(outstr,"---------",1);
             pif_offset++;
             x <<= 1;
             }
           break;
      case PROTOCOL_SERVICES_SUPPORTED: /* BACnetServicesSupported  */
           len1 = show_application_tag(x);
           unused = pif_get_byte(0);
           bac_show_byte("Unused Bits in Last Octet","%u");
           j=0;
           for(i=0;i<((len1-1)*8-unused);i++) {
             if(!(i%8)) {
               x = pif_get_byte(0);
               sprintf(outstr,"Bit String Octet [%u]",j++);
               bac_show_byte(outstr,"X'%02X'");
               }
             sprintf(outstr,"   %s",BACnetServicesSupported[i]);
             pif_offset--;
             if(x&0x80)
               show_str_eq_str(outstr,"Supported",1);
             else
               show_str_eq_str(outstr,"---------",1);
             pif_offset++;
             x <<= 1;
             }
           break;
      case PROTOCOL_VERSION: /*  Character String  */
           show_application_data(x);
           break;
      case READ_ONLY: /*  Boolean  */
           show_application_data(x);
           break;
      case REASON_FOR_HALT: /*  BACnetProgramError  */
           show_application_tag(x);
           bac_show_byte(BACnetProgramError[pif_get_byte(0)],"%u");
           break;
      case RECIPIENT:
           show_bac_recipient();
           break;
      case RECIPIENT_LIST:
           break;
      case RELIABILITY:
           show_application_tag(x);
           bac_show_byte(BACnetReliability[pif_get_byte(0)],"%u");
           break;
      case RELINQUISH_DEFAULT:  /* Data Type depends of Object Type! */
           switch (obj_type) {
              case 1:  /* Analog_Out */
              case 2:  /* Analog_Value */
              case 14: /* Multistate_Output */
                       show_application_data(x);
                       break;
              case 4:  /* Binary_Output */
              case 5:  /* Binary_Value */
                       show_application_tag(x);
                       bac_show_byte(BACnetBinaryPV[pif_get_byte(0)],"%u");
                       break;
              default: pif_show_ascii(0,
                "Unknown Data - Object Type has no Standard Commandable Properties!");
              }
           break;
      case REQUIRED:  break;  /*  No data! */
      case RESOLUTION:  /* Real */
           show_application_data(x);
           break;
      case SEGMENTATION_SUPPORTED:  /* BACnetSegmentation */
           show_application_tag(x);
           bac_show_byte(BACnetSegmentation[pif_get_byte(0)],"%u");
           break;
      case SETPOINT:  /* Real */
           show_application_data(x);
           break;
      case SETPOINT_REFERENCE:  /*  BACnetObjectPropertyReference */
           show_bac_obj_prop_ref();
           break;
      case STATE_TEXT :  /* Array of Character Strings */
           if (prop_idx == 0) {
             show_application_tag(x);
             bac_show_byte("Array Size","%u");
             }
           else
             while ((x & 0x0f) != 0x0f) {
                show_application_data(x);
                x = pif_get_byte(0);
                };
           break;
      case STATUS_FLAGS:  /*  Bit String */
           len = show_application_tag(x);
           show_bac_status_flags(len);
           break;
      case SYSTEM_STATUS:  /* BACnetDeviceStatus */
           show_application_tag(x);
           bac_show_byte(BACnetDeviceStatus[pif_get_byte(0)],"%u");
           break;
      case TIME_DELAY:
           show_application_data(x);
           break;
      case TIME_OF_ACTIVE_TIME_RESET:  /* BACnetDateTime   */
           show_application_data(x);
           show_application_data(pif_get_byte(0));
           break;
      case TIME_OF_STATE_COUNT_RESET:  /* BACnetDateTime   */
           show_application_data(x);
           show_application_data(pif_get_byte(0));
           break;
      case UNITS:  /* BACnetEngineering Units */
           show_application_tag(x);
           bac_show_byte(BACnetEngineeringUnits[pif_get_byte(0)],"%u");
           break;
      case UPDATE_INTERVAL:  /*  Unsigned */
           show_application_data(x);
           break;
      case UTC_OFFSET:  /*  Real  */
           show_application_data(x);
           break;
      case VENDOR_IDENTIFIER:
           show_application_data(x);
           break;
      case VENDOR_NAME:  /* Character String   */
           show_application_data(x);
           break;
      case VT_CLASSES_SUPPORTED:  /*  List of BACnet VT Classes */
           while ((pif_get_byte(0) & 0x0f) != 0x0f)
             show_application_data(pif_get_byte(0));
           break;
      case WEEKLY_SCHEDULE:  /* ARRAY of BACnetTimeValue */
           if (prop_idx == 0) {
             show_application_tag(x);
             bac_show_byte("Array Size","%u");
             };
           while ((pif_get_byte(0) & 0x0f) != 0x0f) {
             show_bac_time_value();
             };
           break;
      default:
           bac_show_byte("Error: Unknown Property Identifier","%u");

      }
}

/**************************************************************************/
int bac_extract_obj_type( void )
/**************************************************************************/
 /* This function extracts the object type information from an object
    identifer and returns the integer value of the enumeration. */
{ 
#if 0
  union {
    struct {
      unsigned char lo;
      unsigned char hi;
      } byte;
      int word;
  } object_type;

  object_type.byte.hi = (pif_get_byte(0) & 0x40)>>6;
  object_type.byte.lo = pif_get_byte(1)>>6 | pif_get_byte(0)<<2;

  return (object_type.word);
#else
	long	obj_id;
	
	for (int i = 0; i < 4; i++)
		obj_id = (obj_id << 8) | (unsigned char)pif_get_byte( i );
	
	return (obj_id >> 22) & 0x000003FF;
#endif
}