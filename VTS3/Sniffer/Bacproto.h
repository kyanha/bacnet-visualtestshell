/*  This version of BACPROTO.H is compatible with BACSN015.C  */
// Revision: Sep 18 2001 added new properties published in Addendum 135b to ANSI/ASHRAE Standard 135-1995

/* ---- primitive data type constants ---- */
/*#define NULL             0 */
#define BOOLEAN            1   
#define UNSIGNED           2
#define SIGNED             3
#define REAL               4
#define DOUBLE             5
#define OCTET_STRING       6
#define CHARACTER_STRING   7
#define BIT_STRING         8
#define ENUMERATED         9
#define DATE              10
#define TIME              11
#define OBJECT_IDENTIFIER 12

/* ----  property identifiers ---*/
#define ACKED_TRANSITIONS                     0
#define ACK_REQUIRED                          1
#define ACTION                                2
#define ACTION_TEXT                           3
#define ACTIVE_TEXT                           4
#define ACTIVE_VT_SESSIONS                    5
#define ALARM_VALUE                           6
#define ALARM_VALUES                          7
#define ALL                                   8
#define ALL_WRITES_SUCCESSFUL                 9
#define APDU_SEGMENT_TIMEOUT                  10
#define APDU_TIMEOUT                          11
#define APPLICATION_SOFTWARE_VERSION          12
#define ARCHIVE                               13
#define BIAS                                  14
#define CHANGE_OF_STATE_COUNT                 15
#define CHANGE_OF_STATE_TIME                  16
#define NOTIFICATION_CLASS                    17  /* renamed in 2nd public review */
/* #define CONFIRMED_RECIPIENT_LIST           18  removed */
#define CONTROLLED_VARIABLE_REFERENCE         19
#define CONTROLLED_VARIABLE_UNITS             20
#define CONTROLLED_VARIABLE_VALUE             21
#define COV_INCREMENT                         22
#define DATELIST                              23
#define DAYLIGHT_SAVINGS_STATUS               24
#define DEADBAND                              25
#define DERIVATIVE_CONSTANT                   26
#define DERIVATIVE_CONSTANT_UNITS             27
#define DESCRIPTION                           28
#define DESCRIPTION_OF_HALT                   29
#define DEVICE_ADDRESS_BINDING                30
#define DEVICE_TYPE                           31
#define EFFECTIVE_PERIOD                      32
#define ELAPSED_ACTIVE_TIME                   33
#define ERROR_LIMIT                           34
#define EVENT_ENABLE                          35
#define EVENT_STATE                           36
#define EVENT_TYPE                            37
#define EXCEPTION_SCHEDULE                    38
#define FAULT_VALUES                          39
#define FEEDBACK_VALUE                        40
#define FILE_ACCESS_METHOD                    41
#define FILE_SIZE                             42
#define FILE_TYPE                             43
#define FIRMWARE_REVISION                     44
#define HIGH_LIMIT                            45
#define INACTIVE_TEXT                         46
#define IN_PROCESS                            47
#define INSTANCE_OF                           48
#define INTEGRAL_CONSTANT                     49
#define INTEGRAL_CONSTANT_UNITS               50
#define ISSUE_CONFIRMED_NOTIFICATIONS         51
#define LIMIT_ENABLE                          52
#define LIST_OF_GROUP_MEMBERS                 53
#define LIST_OF_OBJ_PROP_REFERENCES           54
#define LIST_OF_SESSION_KEYS                  55
#define LOCAL_DATE                            56
#define LOCAL_TIME                            57
#define LOCATION                              58
#define LOW_LIMIT                             59
#define MANIPULATED_VARIABLE_REFERENCE        60
#define MAXIMUM_OUTPUT                        61
#define MAX_APDU_LENGTH_ACCEPTED              62
#define MAX_INFO_FRAMES                       63
#define MAX_MASTER                            64
#define MAX_PRES_VALUE                        65
#define MINIMUM_OFF_TIME                      66
#define MINIMUM_ON_TIME                       67
#define MINIMUM_OUTPUT                        68
#define MIN_PRES_VALUE                        69
#define MODEL_NAME                            70
#define MODIFICATION_DATE                     71
#define NOTIFY_TYPE                           72
#define NUMBER_OF_APDU_RETRIES                73
#define NUMBER_OF_STATES                      74
#define OBJECT_ID                             75
#define OBJECT_LIST                           76
#define OBJECT_NAME                           77
#define OBJECT_PROPERTY_REFERENCE             78
#define OBJECT_TYPE                           79
// #define OPTIONAL                              80	// name collision with bogus MFC define
#define xOPTIONAL                             80
#define OUT_OF_SERVICE                        81
#define OUTPUT_UNITS                          82
#define EVENT_PARAMETERS                      83
#define POLARITY                              84
#define PRESENT_VALUE                         85
#define PRIORITY                              86
#define PRIORITY_ARRAY                        87
#define PRIORITY_FOR_WRITING                  88
#define PROCESS_IDENTIFIER                    89
#define PROGRAM_CHANGE                        90
#define PROGRAM_LOCATION                      91
#define PROGRAM_STATE                         92
#define PROPORTIONAL_CONSTANT                 93
#define PROPORTIONAL_CONSTANT_UNITS           94
#define PROTOCOL_CONFORMANCE_CLASS            95
#define PROTOCOL_OBJECT_TYPES_SUPPORTED       96
#define PROTOCOL_SERVICES_SUPPORTED           97
#define PROTOCOL_VERSION                      98
#define READ_ONLY                             99
#define REASON_FOR_HALT                       100
#define RECIPIENT                             101
#define RECIPIENT_LIST                        102
#define RELIABILITY                           103
#define RELINQUISH_DEFAULT                    104
#define REQUIRED                              105
#define RESOLUTION                            106
#define SEGMENTATION_SUPPORTED                107
#define SETPOINT                              108
#define SETPOINT_REFERENCE                    109
#define STATE_TEXT                            110
#define STATUS_FLAGS                          111
#define SYSTEM_STATUS                         112
#define TIME_DELAY                            113
#define TIME_OF_ACTIVE_TIME_RESET             114
#define TIME_OF_STATE_COUNT_RESET             115
#define TIME_SYNCHRONIZATION_RECIPIENTS       116
#define UNITS                                 117
#define UPDATE_INTERVAL                       118
#define UTC_OFFSET                            119
#define VENDOR_IDENTIFIER                     120
#define VENDOR_NAME                           121
#define VT_CLASSES_SUPPORTED                  122
#define WEEKLY_SCHEDULE                       123
#define ATTEMPTED_SAMPLES 		              124
#define AVERAGE_VALUE 			              125
#define BUFFER_SIZE                           126
#define CLIENT_COV_INCREMENT                  127
#define COV_RESUBSCRIPTION_INTERVAL           128
//#define CURRENT_NOTIFY_TIME                   129
#define EVENT_TIME_STAMPS                     130
#define LOG_BUFFER                            131
#define LOG_DEVICE_OBJECT_PROPERTY            132
#define LOG_ENABLE                            133
#define LOG_INTERVAL                          134
#define MAXIMUM_VALUE 			              135
#define MINIMUM_VALUE 			              136
#define NOTIFICATION_THRESHOLD                137
//#define PREVIOUS_NOTIFY_TIME                  138
#define PROTOCOL_REVISION                     139
#define RECORDS_SINCE_NOTIFICATION            140
#define RECORD_COUNT                          141
#define START_TIME                            142
#define STOP_TIME                             143
#define STOP_WHEN_FULL                        144
#define TOTAL_RECORD_COUNT                    145            
#define VALID_SAMPLES 			              146
#define WINDOW_INTERVAL 		              147
#define WINDOW_SAMPLES 			              148
#define MAXIMUM_VALUE_TIMESTAMP               149
#define MINIMUM_VALUE_TIMESTAMP               150
#define VARIANCE_VALUE 			              151
#define ACTIVE_COV_SUBSCRIPTIONS	          152  //Xiao Shiyuan 2002-7-23
#define	BACKUP_FAILURE_TIMEOUT				  153 // Zhu Zhenhua  2003-11-21 
#define	CONFIGURATION_FILES                   154 // Zhu Zhenhua  2003-11-21 
#define	DATABASE_REVISION                     155 // Zhu Zhenhua  2003-11-21 
#define	DIRECT_READING                        156 // Zhu Zhenhua  2003-11-21 
#define	LAST_RESTORE_TIME  				      157 // Zhu Zhenhua  2003-11-21 
#define	MAINTENANCE_REQUIRED 			      158 // Zhu Zhenhua  2003-11-21 
#define	MEMBER_OF 						      159 // Zhu Zhenhua  2003-11-21 
#define	MODE 							      160 // Zhu Zhenhua  2003-11-21 
#define	OPERATION_EXPECTED  				  161 // Zhu Zhenhua  2003-11-21 
#define	SETTING 						      162 // Zhu Zhenhua  2003-11-21 
#define	SILENCED    					      163 // Zhu Zhenhua  2003-11-21 
#define	TRACKING_VALUE  				      164 // Zhu Zhenhua  2003-11-21 
#define	ZONE_MEMBERS 					      165 // Zhu Zhenhua  2003-11-21 
#define	LIFE_SAFETY_ALARM_VALUES 		      166 // Zhu Zhenhua  2003-11-21 
#define	MAX_SEGMENTS_ACCEPTED 			      167 // Zhu Zhenhua  2003-11-21 
#define PROFILE_NAME                          168 //Xiao Shiyuan 2002-7-23
#define AUTO_SLAVE_DISCOVERY                  169
#define MANUAL_SLAVE_ADDRESS_BINDING          170
#define SLAVE_ADDRESS_BINDING				  171
#define SLAVE_PROXY_ENABLE					  172
#define LAST_NOTIFY_RECORD					  173 //Zhu Zhenhua  2004-5-11
#define SCHEDULE_DEFAULT					174
#define ACCEPTED_MODES						175
#define	ADJUST_VALUE						176
#define	COUNT				177
#define COUNT_BEFORE_CHANGE					178
#define COUNT_CHANGE_TIME					179
#define	COV_PERIOD							180
#define INPUT_REFERENCE						181
#define LIMIT_MONITORING_INTERVAL			182
#define LOGGING_DEVICE						183
#define LOGGING_RECORD						184
#define PRESCALE							185
#define PULSE_RATE							186
#define SCALE								187
#define	SCALE_FACTOR						188
#define UPDATE_TIME							189
#define VALUE_BEFORE_CHANGE					190
#define VALUE_SET							191
#define VALUE_CHANGE_TIME					192
#define ALIGN_INTERVALS						193
#define INTERVAL_OFFSET						195
#define LAST_RESTART_REASON					196
#define LOGGING_TYPE						197
#define RESTART_NOTIFICATION_RECIPIENTS		202
#define TIME_OF_DEVICE_RESTART				203
#define TIME_SYNCHRONIZATION_INTERVAL		204
#define TRIGGER								205
#define UTC_TIME_SYNCHRONIZATION_RECIPIENTS 206
#define NODE_SUBTYPE						207
#define NODE_TYPE							208
#define STRUCTURED_OBJECT_LIST				209
#define SUBORDINATE_LIST					211
#define SUBORDINATE_ANNOTATIONS				210
#define ACTUAL_SHED_LEVEL					212
#define DUTY_WINDOW							213
#define EXPECTED_SHED_LEVEL					214
#define FULL_DUTY_BASELINE					215
#define REQUESTED_SHED_LEVEL				218
#define SHED_DURATION						219
#define	SHED_LEVEL_DESCRIPTIONS				220
#define	SHED_LEVELS							221
#define	STATE_DESCRIPTION					222
#define DOOR_ALARM_STATE					226
#define DOOR_EXTENDED_PULSE_TIME			227
#define DOOR_MEMBERS						228
#define	DOOR_OPEN_TOO_LONG_TIME				229
#define	DOOR_PULSE_TIME						230
#define DOOR_STATUS							231
#define DOOR_UNLOCK_DELAY_TIME				232
#define LOCK_STATUS							233
#define	MASKED_ALARM_VALUES					234
#define SECURED_STATUS						235

/* ----- prototypes for interpreter functions ----- */

int     interp_bacnet_IP( char *header, int length);
int     interp_bacnet_ETHERNET( char *header, int length);
int     interp_bacnet_ARCNET( char *header, int length);
int     interp_bacnet_MSTP( char *header, int length);
int     interp_bacnet_PTP( char *header, int length);
int     interp_bacnet_BVLL( char *header, int length);
int     interp_bacnet_NL( char *header, int length);
int     interp_bacnet_AL( char *header, int length );
int     interp_Message( char *header, int length);
int     interp_BakRestoreMessage( char *header, int length);	// Added by Jingbo Gao, Sep 20 2004

/* ----- prototypes for interpreters of the 8 PDU types ----- */

void	show_confirmed(unsigned char);
void    show_unconfirmed(unsigned char);
void    show_simple_ack(unsigned char);
void    show_complex_ack(unsigned char);
void    show_segment_ack(unsigned char);
void    show_error(unsigned char);
void    show_reject(unsigned char);
void    show_abort(unsigned char);

/* ----- prototypes for confirmed service interpreter functions ----- */

void    show_acknowledgeAlarm(void);
void    show_confirmedCOVNotification(void);
void    show_confirmedEventNotification(void);
void    show_getAlarmSummary(void);
void    show_getEnrollmentSummary(void);
void    show_getEventInformation(void); //Added by Zhu Zhenhua, 2004-5-25
void    show_getEventInformationACK(void); //Added by Zhu Zhenhua, 2004-5-25
void	show_event_summary(void); //Added by Zhu Zhenhua, 2004-5-25
void    show_subscribeCOV(void);
void    show_atomicReadFile(void);
void    show_atomicWriteFile(void);
void    show_addListElement(void);
void    show_removeListElement(void);
void    show_createObject(void);
void    show_deleteObject(void);
void    show_readProperty(void);
void    show_readPropertyConditional(void);
void    show_readPropertyMultiple(void);
void    show_writeProperty(void);
void    show_writePropertyMultiple(void);
void    show_deviceCommunicationControl(void);
void    show_privateTransfer(void);
void    show_confirmedTextMessage(void);
void    show_reinitializeDevice(void);
void    show_vtOpen(void);
void    show_vtClose(void);
void    show_vtData(void);
void    show_authenticate(void);
void    show_requestKey(void);
void    show_ReadRange(void);
/* ----- prototypes for unconfirmed service interpreter functions ----- */

void     show_iAm(void);
void     show_iHave(void);
void     show_unconfirmedCOVNotification(void);
void     show_unconfEventNotification(void);
void     show_unconfPrivateTransfer(void);
void     show_unconfTextMessage(void);
void     show_timeSynchronization(void);
void     show_whoHas(void);
void     show_whoIs(void);
void     show_UTCtimeSynchronization(void);
/* ----- prototypes for complex ACK interpreter functions ----- */

void     show_getAlarmSummaryACK(void);
void     show_getEnrollmentSummaryACK(void);
void     show_atomicReadFileACK(void);
void     show_atomicWriteFileACK(void);
void     show_createObjectACK(void);
void     show_readPropertyACK(void);
void     show_readPropertyConditionalACK(void);
void     show_readPropertyMultipleACK(void);
void     show_conf_PrivateTransferACK(void);
void     show_vtOpenACK(void);
void     show_vtDataACK(void);
void     show_authenticateACK(void);
void     show_ReadRangeACK(void);
/* ----- prototypes for error interpreter functions ----- */

void     show_error_codes(void);
void     show_createObjectError(void);
void     show_writePropertyMultipleError(void);
void     show_vtCloseError(void);

/*  ----- prototypes for general purpose pif routines ----- */

void     bac_show_byte(char *,char *);
void     bac_show_unsigned(char *,unsigned int);
void     bac_show_bipaddr(char *);
void     bac_show_flag(char *, unsigned char);
void     bac_show_ctag_flag(void);
void     bac_show_flagmask (unsigned char, char *);
void     bac_show_nbytes(unsigned int, char *);
void     bac_show_word_hl(char *, char *);
void     bac_show_long_hl(char *, char *);
void     float_to_ascii(double, char *);
void     show_str_eq_str(char *, char *, unsigned int);
unsigned long get_bac_unsigned(int, int);

/*  ----- prototypes for displaying other PDU components ----- */

void     check_ctag_length(unsigned char, unsigned int, unsigned int);
unsigned int show_context_tag(char *);
unsigned int show_application_data(unsigned char);
unsigned int show_application_tag(unsigned char);

/*  ----- prototypes for displaying primitive data types ----- */

void     show_bac_ANY(int, unsigned int, int);
void     show_bac_bitstring(unsigned int);
void     show_bac_charstring(unsigned int);
void     show_bac_date(void);
void     show_bac_double(void);
void     show_bac_object_identifier(void);
void     show_bac_real(void);
void     show_bac_signed(unsigned int);
void     show_bac_time(void);
void     show_bac_unsigned(unsigned int);

/*  ----- prototypes for displaying BACnet base types ----- */

void	show_bac_action_list(void);		//Added by Zhu Zhenhua, 2004-6-14
void     show_bac_action_command(unsigned int);
void     show_bac_address(void);
void     show_bac_calendar_entry(void);
void     show_bac_event_parameters(void);
void     show_bac_event_transitions_ackd(void);
void     show_bac_obj_prop_ref(void);
void     show_bac_dev_obj_prop_ref(void); //Added by Zhu Zhenhua, 2004-6-14
void     show_bac_dev_obj_ref(void); //Added by Zhu Zhenhua, 2004-6-14
void     show_bac_obj_prop_value(void);
void     show_bac_object_type(void);
void     show_bac_property_identifier(unsigned int);
void     show_bac_property_ref(void);
void     show_bac_property_states(void);
void     show_bac_property_value(void);
void     show_bac_read_access_result(void);
void     show_bac_read_access_spec(void);
void     show_bac_recipient(void);
void     show_bac_recipient_process(void);
void     show_bac_special_event(void);
void     show_bac_status_flags(unsigned int);
void     show_bac_timestamp(void);
void     show_bac_time_value(void);
void     show_bac_VT_session(void);
void     show_bac_weeknday(void);
void     show_log_buffer( void );
void     show_logDatum_choice( void );
void     show_bac_result_flags( unsigned int);
void     show_bac_COV_Subscription(); //Xiao Shiyuan 2002-7-23
void     show_bac_destination(void); //Xu yiping 2002-9-28
void	 show_bac_bitstring_value(char**c);	//Xu yiping 2002-9-28
void	 show_bac_devobj_prop_ref(); //Added by Zhu Zhenhua, 2004-5-17
void	 show_bac_devobj_prop_val(); //Added by Zhu Zhenhua, 2004-5-17
void	 show_bac_scale(void);
void	show_bac_life_safety_mode();
void	show_bac_life_safety_state();
void	show_bac_life_safety_operation();

/*  ----- functions to add new tree node to the detail view ----- */

void	 show_head_obj_id( unsigned int , char* , int );		// Lei Chengxin 2003-7-25		
void	 show_head_unsigned( unsigned int , char* , int );		// Lei Chengxin 2003-7-30
void	 show_head_ascii( char* );								// Lei Chengxin 2003-7-31
void	 show_head_char_string( unsigned int , char* , int );	// Lei Chengxin 2003-7-31
void	 show_head_time( unsigned int , char* , int );			// Lei Chengxin 2003-7-31
void	 show_head_property_ID( unsigned int , char* , int );	// Lei Chengxin 2003-7-31
void     show_head_app_data( void );							// Lei Chengxin 2003-8-23
void	 show_head_octet_string( unsigned int , char* , int );	// Lei Chengxin 2003-8-23
void	 show_head_signed( unsigned int , char* , int );		// Lei Chengxin 2003-8-23
void	 show_head_real( unsigned int , char* , int );			// Lei Chengxin 2003-8-23
void     show_head_bit_string( unsigned int , char* , int );	// Lei Chengxin 2003-8-23

/*  ----- prototypes extracting information from object identifiers ----- */
int      bac_extract_obj_type(void);

