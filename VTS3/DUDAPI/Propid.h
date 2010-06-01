/* ----- Property Identifiers for Standard Object Types ----- */

#ifndef _BACnetPropertyIdentifier
#define _BACnetPropertyIdentifier

enum BACnetPropertyIdentifier
	{	ACKED_TRANSITIONS,						//0
		ACK_REQUIRED,							//1
		ACTION,									//2
		ACTION_TEXT,							//3
		ACTIVE_TEXT,							//4
		ACTIVE_VT_SESSIONS,						//5
		ALARM_VALUE,							//6
		ALARM_VALUES,							//7
		ALL,   									//8
		ALL_WRITES_SUCCESSFUL,				    //9							
		APDU_SEGMENT_TIMEOUT,				    //10
		APDU_TIMEOUT,							//11
		APPLICATION_SOFTWARE_VERSION,			//12
		ARCHIVE,								//13
		BIAS,									//14
		CHANGE_OF_STATE_COUNT,					//15
		CHANGE_OF_STATE_TIME,					//16
		NOTIFICATION_CLASS,						//17
		blank1,									//18
		CONTROLLED_VARIABLE_REFERENCE,			//19
		CONTROLLED_VARIABLE_UNITS,				//20
		CONTROLLED_VARIABLE_VALUE,				//21
		COV_INCREMENT,							//22
		DATE_LIST,								//23
		DAYLIGHT_SAVINGS_STATUS,				//24
		DEADBAND,								//25
		DERIVATIVE_CONSTANT,					//26
		DERIVATIVE_CONSTANT_UNITS,				//27
		DESCRIPTION,							//28
		DESCRIPTION_OF_HALT,					//29
		DEVICE_ADDRESS_BINDING,					//30
		DEVICE_TYPE,							//31
		EFFECTIVE_PERIOD,						//32
		ELAPSED_ACTIVE_TIME,					//33
		ERROR_LIMIT,							//34
		EVENT_ENABLE,							//35
		EVENT_STATE,							//36
		EVENT_TYPE,								//37
		EXCEPTION_SCHEDULE,						//38
		FAULT_VALUES,							//39
		FEEDBACK_VALUE,							//40
		FILE_ACCESS_METHOD,						//41
		FILE_SIZE,								//42
		FILE_TYPE,								//43
		FIRMWARE_REVISION,						//44
		HIGH_LIMIT,								//45
		INACTIVE_TEXT,							//46
		IN_PROCESS,								//47
		INSTANCE_OF,							//48
		INTEGRAL_CONSTANT,						//49
		INTEGRAL_CONSTANT_UNITS,				//50
		ISSUE_CONFIRMED_NOTIFICATIONS,			//51
		LIMIT_ENABLE,							//52
		LIST_OF_GROUP_MEMBERS,					//53
		LIST_OF_OBJECT_PROPERTY_REFERENCES,		//54
		LIST_OF_SESSION_KEYS,					//55
		LOCAL_DATE,								//56
		LOCAL_TIME,								//57
		LOCATION,								//58
		LOW_LIMIT,								//59
		MANIPULATED_VARIABLE_REFERENCE,			//60
		MAXIMUM_OUTPUT,							//61
		MAX_APDU_LENGTH_ACCEPTED,				//62
		MAX_INFO_FRAMES,						//63
		MAX_MASTER,								//64
		MAX_PRES_VALUE,							//65
		MINIMUM_OFF_TIME,						//66
		MINIMUM_ON_TIME,						//67
		MINIMUM_OUTPUT,							//68
		MIN_PRES_VALUE,							//69
		MODEL_NAME,								//70
		MODIFICATION_DATE,						//71
		NOTIFY_TYPE,							//72
		NUMBER_OF_APDU_RETRIES,					//73
		NUMBER_OF_STATES,						//74
		OBJECT_IDENTIFIER,						//75
		OBJECT_LIST,							//76
		OBJECT_NAME,							//77
		OBJECT_PROPERTY_REFERENCE,				//78
		OBJECT_TYPE,							//79
		ISOPTIONAL,								//80	must hack this for VC++5.0
		OUT_OF_SERVICE,							//81
		OUTPUT_UNITS,							//82
		EVENT_PARAMETERS,						//83
		POLARITY,								//84
		PRESENT_VALUE,							//85
		PRIORITY,								//86
		PRIORITY_ARRAY,							//87
		PRIORITY_FOR_WRITING,					//88
		PROCESS_IDENTIFIER,						//89
		PROGRAM_CHANGE,							//90
		PROGRAM_LOCATION,						//91
		PROGRAM_STATE,							//92
		PROPORTIONAL_CONSTANT,					//93
		PROPORTIONAL_CONSTANT_UNITS,			//94
		PROTOCOL_CONFORMANCE_CLASS,				//95
		PROTOCOL_OBJECT_TYPES_SUPPORTED,		//96
		PROTOCOL_SERVICES_SUPPORTED,			//97
		PROTOCOL_VERSION,						//98
		READ_ONLY,								//99
		REASON_FOR_HALT,						//100
		RECIPIENT,								//101
		RECIPIENT_LIST,							//102
		RELIABILITY,							//103
		RELINQUISH_DEFAULT,						//104
		REQUIRED,								//105
		RESOLUTION,								//106
		SEGMENTATION_SUPPORTED,					//107
		SETPOINT,								//108
		SETPOINT_REFERENCE,						//109
		STATE_TEXT,								//110
		STATUS_FLAGS,							//111
		SYSTEM_STATUS,							//112
		TIME_DELAY,								//113
		TIME_OF_ACTIVE_TIME_RESET,				//114
		TIME_OF_STATE_COUNT_RESET,				//115
		TIME_SYNCHRONIZATION_RECIPIENTS,		//116
		UNITS,									//117
		UPDATE_INTERVAL,						//118
		UTC_OFFSET,								//119
		VENDOR_IDENTIFIER,						//120
		VENDOR_NAME,							//121
		VT_CLASSES_SUPPORTED,					//122
		WEEKLY_SCHEDULE,							//123
        ATTEMPTED_SAMPLES, 		                //124
        AVERAGE_VALUE, 			                //125
        BUFFER_SIZE,                             //126
        CLIENT_COV_INCREMENT,                    //127
        COV_RESUBSCRIPTION_INTERVAL,             //128
//        CURRENT_NOTIFY_TIME,                     //129		//Modified by Zhu Zhenhua, 2004-5-11
        EVENT_TIME_STAMPS = 130,                       //130
        LOG_BUFFER,                              //131
        LOG_DEVICE_OBJECT_PROPERTY,              //132
        LOG_ENABLE,                              //133
        LOG_INTERVAL,                            //134
        MAXIMUM_VALUE, 			                //135
        MINIMUM_VALUE, 			                //136
        NOTIFICATION_THRESHOLD,                  //137
 //       PREVIOUS_NOTIFY_TIME,                    //138       //Modified by Zhu Zhenhua, 2004-5-11
        PROTOCOL_REVISION = 139,                       //139
        RECORDS_SINCE_NOTIFICATION,              //140
        RECORD_COUNT,                            //141
        START_TIME,                              //142
        STOP_TIME,                               //143
        STOP_WHEN_FULL,                          //144
        TOTAL_RECORD_COUNT,                      //145            
        VALID_SAMPLES, 			                //146
        WINDOW_INTERVAL, 		                //147
        WINDOW_SAMPLES, 			                //148
        MAXIMUM_VALUE_TIMESTAMP,                 //149
        MINIMUM_VALUE_TIMESTAMP,                 //150
        VARIANCE_VALUE, 			                //151
		ACTIVE_COV_SUBSCRIPTIONS,           // 152 Zhu Zhenhua  2003-7-21 
		BACKUP_FAILURE_TIMEOUT,            // 153 Zhu Zhenhua  2003-7-21 
		CONFIGURATION_FILES,               // 154 Zhu Zhenhua  2003-7-21 
		DATABASE_REVISION,                 // 155 Zhu Zhenhua  2003-7-21 
		DIRECT_READING,                    // 156 Zhu Zhenhua  2003-7-21 
		LAST_RESTORE_TIME, 				  // 157 Zhu Zhenhua  2003-7-21 
		MAINTENANCE_REQUIRED, 			  // 158 Zhu Zhenhua  2003-7-21 
		MEMBER_OF, 						  // 159 Zhu Zhenhua  2003-7-21 
		MODE, 							  // 160 Zhu Zhenhua  2003-7-21 
		OPERATION_EXPECTED, 				  // 161 Zhu Zhenhua  2003-7-21 
		SETTING, 						  // 162 Zhu Zhenhua  2003-7-21 
		SILENCED,    					  // 163 Zhu Zhenhua  2003-7-21 
		TRACKING_VALUE, 					  // 164 Zhu Zhenhua  2003-7-21 
		ZONE_MEMBERS, 					  // 165 Zhu Zhenhua  2003-7-21 
		LIFE_SAFETY_ALARM_VALUES, 		  // 166 Zhu Zhenhua  2003-7-21 
		MAX_SEGMENTS_ACCEPTED, 			  // 167 Zhu Zhenhua  2003-7-21 
		PROFILE_NAME,					  // 168 Zhu Zhenhua  2003-7-21
		AUTO_SLAVE_DISCOVERY,             // 169 Shiyuan Xiao 7/15/2005
		MANUAL_SLAVE_ADDRESS_BINDING,     // 170 Shiyuan Xiao 7/15/2005
		SLAVE_ADDRESS_BINDING,            // 171 Shiyuan Xiao 7/15/2005
		SLAVE_PROXY_ENABLE,               // 172 Shiyuan Xiao 7/15/2005		
		LAST_NOTIFY_RECORD,   		      // 173 Zhu Zhenhua  2004-5-11
		SCHEDULE_DEFAULT,                 // 174 Shiyuan Xiao 7/15/2005
		ACCEPTED_MODES,                   // 175 Shiyuan Xiao 7/15/2005
		ADJUST_VALUE,                     // 176 Shiyuan Xiao 7/15/2005
		COUNT,                            // 177 Shiyuan Xiao 7/15/2005 
		COUNT_BEFORE_CHANGE,              // 178 Shiyuan Xiao 7/15/2005
		COUNT_CHANGE_TIME,                // 179 Shiyuan Xiao 7/15/2005		
		COV_PERIOD,                       // 180 Shiyuan Xiao 7/15/2005
		INPUT_REFERENCE,                  // 181 Shiyuan Xiao 7/15/2005
		LIMIT_MONITORING_INTERVAL,        // 182 Shiyuan Xiao 7/15/2005
		LOGGING_DEVICE,                   // 183 Shiyuan Xiao 7/15/2005
		LOGGING_RECORD,                   // 184 Shiyuan Xiao 7/15/2005  
		PRESCALE,                         // 185 Shiyuan Xiao 7/15/2005  
		PULSE_RATE,                       // 186 Shiyuan Xiao 7/15/2005
		SCALE,                            // 187 Shiyuan Xiao 7/15/2005
		SCALE_FACTOR,                     // 188 Shiyuan Xiao 7/15/2005  
		UPDATE_TIME,                      // 189 Shiyuan Xiao 7/15/2005 
		VALUE_BEFORE_CHANGE,              // 190 Shiyuan Xiao 7/15/2005
		VALUE_SET,                        // 191 Shiyuan Xiao 7/15/2005
		VALUE_CHANGE_TIME,                // 192 Shiyuan Xiao 7/15/2005
	   // added Addendum B (135-2004)
		ALIGN_INTERVALS,					// 193
		GROUP_MEMBERS_NAMES,				// 194
		INTERVAL_OFFSET,					// 195
		LAST_RESTART_REASON,				// 196
		LOGGING_TYPE,						// 197
		MEMBER_STATUS_FLAGS,				// 198
		NOTIFICATION_PERIOD,				// 199
		PREVIOUS_NOTIFY_RECORD,				// 200
		REQUESTED_UPDATE_INTERVAL,			// 201
		RESTART_NOTIFICATION_RECIPIENTS,	// 202
		TIME_OF_DEVICE_RESTART,				// 203
		TIME_SYNCHRONIZATION_INTERVAL,		// 204
		TRIGGER,							// 205
		UTC_TIME_SYNCHRONIZATION_RECIPIENTS,  // 206
		// Added by addenda D
		NODE_SUBTYPE,					// 207
		NODE_TYPE,						// 208
		STRUCTURED_OBJECT_LIST,			// 209
		SUBORDINATE_ANNOTATIONS,		// 210
		SUBORDINATE_LIST,				// 211
		// Added by addenda E 135-2004
		ACTUAL_SHED_LEVEL,				// 212
		DUTY_WINDOW,					// 213
		EXPECTED_SHED_LEVEL,			// 214
		FULL_DUTY_BASELINE,				// 215
		UNKNOWN_216,					// 216
		UNKNWON_217,					// 217
		REQUESTED_SHED_LEVEL,			// 218
		SHED_DURATION,					// 219
		SHED_LEVEL_DESCRIPTIONS,		// 220
		SHED_LEVELS,					// 221
		STATE_DESCRIPTION,				// 222
		P223,
		P224,
		P225,
		DOOR_ALARM_STATE,				// 226
		DOOR_EXTENDED_PULSE_TIME,		// 227
		DOOR_MEMBERS,					// 228
		DOOR_OPEN_TOO_LONG_TIME,		// 229
		DOOR_PULSE_TIME,				// 230
		DOOR_STATUS,					// 231
		DOOR_UNLOCK_DELAY_TIME,			// 232
		LOCK_STATUS,					// 233
		MASKED_ALARM_VALUES,			// 234
		SECURED_STATUS,					// 235
		// Addendum 2008-w
		BIT_MASK = 342,
		BIT_TEXT,						// 343
		IS_UTC							// 344
	};
// If you add more properties here, increase
// MAX_PROP_ID to one more than the last property
#endif
