//STDOBJ.H

/* 
   Revision: Sep 18 2001
   Added the following objects: averaging, multi-state-value, and trendlog,
   published in Addendum 135b to ANSI/ASHRAE Standard 135-1995
*/

//BACnet Standard Object Structures
#ifndef __STDOBJ_H_INCLUDED
#define __STDOBJ_H_INCLUDED

//equates for priority arrays
#define	fpaNULL				FLT_MIN				//too bad we can't use NAN
#define bpaNULL				2					//binary present values are 0 and 1
#define dvaNULL				4					// DoorValue Enum NULL value
#define upaNULL				0xFFFF				//can't use more than 65534 enumerations, too bad

//equates for standard properties
//these are always in this order for every object type:
#define	idProp				0					//object_id property
#define	nameProp			1					//object_name property
#define	typeProp			2					//object_type property

//equates for propflags bits
#define	PropIsPresent		1					//0=not present, 1=present in this instance
#define	PropIsWritable		2					//0=R, 1=W
#define	ValueUnknown		4					//0=have a value, 1=value is ?

//Shiyuan Xiao 8/1/2005
typedef struct
{
//	BACnetAnyValue minValue;
//	BACnetAnyValue maxValue;
//	BACnetAnyValue resolution;
	int            maxLengthStr;
	int            maxLengthList;  
	int            maxLengthArray;
	int            allowedValues;
} ObjPropValueLimit; //Default property value restriction

#define MAX_TEXT_STRING 132		// our commonly used value

//Generic Object type structure common to all Std Objects
//-------------------------------------------------------
typedef struct {
    void far				*next;
    dword					object_id;
    char					object_name[32];    
    word					object_type;
//enum BACnetObjectType		object_type;
    char 					description[132];
//	octet					std_props[8];		//bitmap of std properties present in this instance, 1=present
    octet					propflags[64];		//up to 64 properties of parser flags
	char					profile_name[132];  //Added by Jingbo Gao, 2003-9-1
    } generic_object;

//Analog Input Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    float 					pv;
    char 					device_type[64];
    octet 					status_flags;
enum BACnetEventState		state;
enum BACnetReliability		reliability;
    boolean					out_of_service;
    word					update_interval;
enum BACnetEngineeringUnits	units;
    float					min_pres_value;
    float					max_pres_value;
    float					resolution;
    float					cov_increment;
    word					time_delay;
    word					notification_class;
    float					high_limit;
    float					low_limit;
    float					deadband;
    octet					limit_enable;
    octet					event_enable;
    octet					acked_transitions;
enum BACnetNotifyType		notify_type;
    BACnetTimeStamp	       far *event_time_stamps[3];  //madanner 6/03, added
   } ai_obj_type;

//Analog Output Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    float					pv;
    char					device_type[64];
    octet					status_flags;
enum BACnetEventState		state;
enum BACnetReliability		reliability;
    boolean					out_of_service;
enum BACnetEngineeringUnits units;
    float					min_pres_value;
    float					max_pres_value;
    float					resolution;
    float					priority_array[16];
    float					relinquish_default;
    float					cov_increment;
    word					time_delay;
    word					notification_class;
    float					high_limit;
    float					low_limit;
    float					deadband;
    octet					limit_enable;
    octet					event_enable;
    octet					acked_transitions;
enum BACnetNotifyType		notify_type;
    BACnetTimeStamp	        far *event_time_stamps[3];  //madanner 6/03, added
   } ao_obj_type;

//Analog Value Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    float					pv;
    octet					status_flags;
enum BACnetEventState		state;
enum BACnetReliability		reliability;
    boolean					out_of_service;
enum BACnetEngineeringUnits units;
    float					priority_array[16];
    float					relinquish_default;
    float					cov_increment;
    word					time_delay;
    word					notification_class;
    float					high_limit;
    float					low_limit;
    float					deadband;
    octet					limit_enable;
    octet					event_enable;
    octet					acked_transitions;
enum BACnetNotifyType		notify_type;
     BACnetTimeStamp	    far *event_time_stamps[3];  //Added by xuyiping 2002-8-29
   } av_obj_type;

//Binary Input Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
enum BACnetBinaryPV			pv;
    char					device_type[64];
    octet					status_flags;
enum BACnetEventState		state;
enum BACnetReliability		reliability;
    boolean					out_of_service;
enum BACnetPolarity			polarity;
    char					inactive_text[64];
    char					active_text[64];
    BACnetDateTime			cos_time;
    word					cos_count;
    BACnetDateTime			time_of_state_count_reset;
    dword					elapsed_active_time;
    BACnetDateTime			time_of_active_time_reset;
    word					time_delay;
    word					notification_class;
enum BACnetBinaryPV			alarm_value;
    octet					event_enable;
    octet					acked_transitions;
enum BACnetNotifyType		notify_type;
    BACnetTimeStamp	        far *event_time_stamps[3];  //madanner 6/03, added
   } bi_obj_type;

//Binary Output Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
enum BACnetBinaryPV			pv;
    char					device_type[64];
    octet					status_flags;
enum BACnetEventState		state;
enum BACnetReliability		reliability;
    boolean					out_of_service;
enum BACnetPolarity			polarity;
    char					inactive_text[64];
    char					active_text[64];
    BACnetDateTime			cos_time;
    word					cos_count;
    BACnetDateTime			time_of_state_count_reset;
    dword					elapsed_active_time;
    BACnetDateTime			time_of_active_time_reset;
    dword					min_off_time;
    dword					min_on_time;
enum BACnetBinaryPV			priority_array[16];
enum BACnetBinaryPV			relinquish_default;
    word					time_delay;
    word					notification_class;
enum BACnetBinaryPV			feedback_value;
    octet					event_enable;
    octet					acked_transitions;
enum BACnetNotifyType		notify_type;
    BACnetTimeStamp	        far *event_time_stamps[3];  //madanner 6/03, added
   } bo_obj_type;

//Binary Value Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
enum BACnetBinaryPV			pv;
    octet					status_flags;
enum BACnetEventState		state;
enum BACnetReliability		reliability;
    boolean					out_of_service;
    char					inactive_text[64];
    char					active_text[64];
    BACnetDateTime			cos_time;
    word					cos_count;
    BACnetDateTime			time_of_state_count_reset;
    dword					elapsed_active_time;
    BACnetDateTime			time_of_active_time_reset;
    dword					min_off_time;
    dword					min_on_time;
enum BACnetBinaryPV 		priority_array[16];
enum BACnetBinaryPV			relinquish_default;
    word					time_delay;
    word					notification_class;
enum BACnetBinaryPV			alarm_value;
    octet					event_enable;
    octet					acked_transitions;
enum BACnetNotifyType		notify_type;
	BACnetTimeStamp	        far *event_time_stamps[3];  //Added by xuyiping 2002-8-29
   } bv_obj_type;

//Calendar Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    boolean					pv;
    BACnetCalendarEntry	far	*date_list;
   } calendar_obj_type;

//Notification Class Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    word					notification_class;
    word					priority[3];
    octet					ack_required;
    BACnetDestination	far	*recipient_list;
   } nc_obj_type;

#define MAX_ACTION_TEXTS 32		
//Command Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    word 					pv;
    boolean					in_process;
    boolean					all_writes_successful;
    word					num_actions;
    BACnetActionCommand	far	*action[MAX_ACTION_TEXTS];
    char				far	*action_text[MAX_ACTION_TEXTS];
   } command_obj_type;

//Device Object
//-------------------------------------------------------
// msdanner 9/2004: Define maximum possible lengths
// for protocol_services_supported and object_types_supported.
// Still some work to do to prevent breakage if these bitstrings
// are longer than expected.
#define MAX_BITSTRING   80    

typedef struct {
    generic_object			go;
enum BACnetDeviceStatus		system_status;
    char					vendor_name[64];
    word					vendor_id;
    char					model_name[32];
    char					firmware_rev[100];
    char					application_software_ver[32];
    char					location[64];
    word					protocol_ver;
    word					protocol_rev;                     // msdanner 9/2004 - added
    word					protocol_conf_class;
    // msdanner 9/2004 - Lengthened these bitstrings (and these only) so we could
    // handle unexpectedly long bitstrings for these.
    octet            protocol_services_supported[MAX_BITSTRING/8]; //bit string, one bit per service
    octet            object_types_supported[MAX_BITSTRING/8];    	//bit string, one bit per object type
    word					num_objects;
    BACnetObjectIdentifier	far	*object_list;				//points to our object list
    word					max_apdu_length_accepted;
enum BACnetSegmentation		segmentation_supported;
	dword					max_segments_accepted;         //added by Jingbo Gao, 2003-9-1
    BACnetVTClassList	far	*vt_classes_supported;
    BACnetVTSession		far	*active_vt_sessions;
    BACnetDate				local_date;
    BACnetTime				local_time;						//time and date are read from system clock
    //float					utc_offset;
	int 					utc_offset;				//modified by xlp 2002-9-4

    boolean					day_savings_status;
    dword					apdu_segment_timeout;
    dword					apdu_timeout;
    word					number_apdu_retries;
    BACnetSessionKey	far	*list_session_keys;
    BACnetRecipient		far	*time_synch_recipients;
    word					max_master;
    word					max_info_frames;
    BACnetAddressBinding far *device_add_binding;
	// added by Jingbo Gao, 2003-9-1
	dword					database_revision;
	BACnetObjectIdentifier	far	*configuration_files;		// points to configuration files
	BACnetTimeStamp			last_restore_time;
	word					backup_failure_timeout;
	BACnetCOVSubscription   far	*active_cov_subscriptions;
	// added LJT 10/12/2005
	BACnetAddressBinding far *manual_slave_add_bind;
	BACnetAddressBinding far *slave_add_bind;
	BooleanList          far *auto_slave_disc;    // SEQ of boolean
	BooleanList          far *slave_proxy_enable; // SEQ of boolean
	// added by Tom Brennan, 1-Apr-2010
    BACnetObjectIdentifier	far	*structured_object_list;	//object list of Structured Views

   } device_obj_type;

//Event Enrollment Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
enum BACnetNotifyType		notify_type;
//Note that event_type is part of parameter_list structure in this implementation!
    BACnetEventParameter	parameter_list;
    BACnetDeviceObjectPropertyReference obj_prop_ref;
enum BACnetEventState 		state;
    octet					event_enable;
    octet					acked_transitions;
    word					notification_class;
    BACnetRecipient			recipient;          // removed for revision 4
    word					process_id;         // removed for revision 4
    word					priority;           // removed for revision 4
    boolean					issue_conf_notifications; // removed for revision 4
    BACnetTimeStamp	        far *event_time_stamps[3];  //madanner 6/03, added
   } ee_obj_type;

//File Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    char					file_type[32];
    dword					file_size;
    BACnetDateTime			mod_date;
    boolean					archive;
    boolean					read_only;
	dword                   record_count;
enum BACnetFileAccessMethod access_method;
   } file_obj_type;

//Group Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    BACnetReadAccessSpecification far *list_of_group_members;
   //get present value by reading the relevant objects
   } group_obj_type;

//Loop Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    float					pv;
    octet					status_flags;
enum BACnetEventState		state;
enum BACnetReliability		reliability;
    boolean					out_of_service;
    word					update_interval;
enum BACnetEngineeringUnits output_units; 
    int						output_value;
    BACnetObjectPropertyReference man_var_ref;
    BACnetObjectPropertyReference cont_var_ref;
    float					cont_var_value;  
enum BACnetEngineeringUnits cont_var_units;
    BACnetObjectPropertyReference far *setpoint_ref;
    float					setpoint;
enum BACnetAction			action;
    float					proportional_const;
enum BACnetEngineeringUnits proportional_const_units;
    float					integral_const;
enum BACnetEngineeringUnits integral_const_units;
    float					derivative_const;
enum BACnetEngineeringUnits derivative_const_units;
    float					bias;
    float					max_output;
    float					min_output;
    word					priority_for_writing;
    float					cov_increment;
    word					time_delay;
    word					notification_class;
    float					error_limit;
    octet					event_enable;
    octet					acked_transitions;
enum BACnetNotifyType		notify_type;
    BACnetTimeStamp	        far *event_time_stamps[3];  //madanner 6/03, added
   } loop_obj_type;

#define MAX_STATE_TEXTS 128		/* Array size; 32 was too small */
//Multi-state Input Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    octet					pv;
    char					device_type[64];
    octet					status_flags;
enum BACnetEventState		state;
enum BACnetReliability		reliability;
    boolean					out_of_service;
    octet					num_of_states;
    char				far	*state_text[MAX_STATE_TEXTS];
    word					time_delay;
    word					notification_class;
    UnsignedList		far *alarm_values;
    UnsignedList		far	*fault_values;
    octet					event_enable;
    octet					acked_transitions;
enum BACnetNotifyType		notify_type;
    BACnetTimeStamp	        far *event_time_stamps[3];  //madanner 6/03, added
   } mi_obj_type;

//Multi-state Output Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    word					pv;
    char					device_type[64];
    octet					status_flags;
enum BACnetEventState		state;
enum BACnetReliability		reliability;
    boolean					out_of_service;
    word					num_of_states;
    char				far	*state_text[MAX_STATE_TEXTS];
    word					priority_array[16];
    word					relinquish_default;
    word					time_delay;
    word					notification_class;
    octet					feedback_value;
    octet					event_enable;
    octet					acked_transitions;
enum BACnetNotifyType		notify_type;
    BACnetTimeStamp	        far *event_time_stamps[3];  //madanner 6/03, added
   } mo_obj_type;

//Program Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
enum BACnetProgramState		prog_state;
enum BACnetProgramRequest	prog_change;
enum BACnetProgramError		reason_for_halt;
    char					description_of_halt[64];
    char					prog_location[64];
    char					instance_of[64];
    octet					status_flags;
enum BACnetEventState		state;
enum BACnetReliability		reliability;
    boolean					out_of_service;
   } program_obj_type;

//Schedule Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
//enum ActionValueType		value_type;
	dword					value_type;   //modified by xyp
    union {
        enum BACnetBinaryPV	bproperty_value; 	//binary value
        word				uproperty_value;	//unsigned word
        float				fproperty_value;	//float
        }					pv;
    BACnetDateRange			effective_period;
    BACnetTimeValue		far	*weekly_schedule[7];
    BACnetExceptionSchedule	exception_schedule;
    BACnetDeviceObjectPropertyReference far *list_obj_prop_ref;
    word					priority_for_writing;

    octet					status_flags;
	enum BACnetReliability	reliability;
    boolean					out_of_service;

    union {
        enum BACnetBinaryPV	bproperty_value; 	//binary value
        word				uproperty_value;	//unsigned word
        float				fproperty_value;	//float
        }					schedule_default;

   } schedule_obj_type;

//Averaging Object
//-------------------------------------------------------
typedef struct 
    {
    generic_object		            	go;
    float				            	minimum_value;
    BACnetDateTime		            	minimum_value_timestamp;
    float				            	average_value;
    float				            	variance_value;
    float				            	maximum_value;
    BACnetDateTime		            	maximum_value_timestamp;
    word		       	                attempted_samples;
    word		       	                valid_samples;
    BACnetDeviceObjectPropertyReference	obj_prop_ref;
    word			                    window_interval;
    word			                    window_samples;
   } avg_obj_type;

// --------------------------------------------------------
//Multi-state Value Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    word				    present_value;
    octet					status_flags;
    enum BACnetEventState	event_state;
    enum BACnetReliability	reliability;
    bool					out_of_service;
    word				    number_of_states;
	// msdanner 9/2004
    //char					state_text[64];
    char				far	*state_text[MAX_STATE_TEXTS];
    word				    priority_array[16];                        
    word				    relinquish_default;
    word				    time_delay;
    word				    notification_class;
    UnsignedList	    far *alarm_values;
    UnsignedList	    far *fault_values;
    octet					event_enable;
    octet					acked_transitions;
    enum BACnetNotifyType	notify_type;
    BACnetTimeStamp		far *event_time_stamps[3];
   } msv_obj_type;
// --------------------------------------------------------
// TrendLog Object
// --------------------------------------------------------
typedef struct
 {
    generic_object			                go;
     boolean							    log_enable;                   
     BACnetDateTime						    start_time;
     BACnetDateTime						    stop_time;
     BACnetDeviceObjectPropertyReference    log_device_object_property;
     word							        log_interval;
     word							        cov_resubscription_interval;
     float								    client_cov_increment;
     BOOLEAN							    stop_when_full;
     word							        buffer_size;
     BACnetLogRecord					    log_buffer;
     word						    	    record_count;
//     word						    	    total_record_count;
     dword						    	    total_record_count;			//should be Unsigned32, GJB 01-MAR-2004
     word						    	    notification_threshold;
     word						    	    records_since_notification;
 //    BACnetDateTime				    		previous_notify_time;
//     BACnetDateTime				    		current_notify_time;
     enum BACnetEventState		    		event_state;
     word						    	   	notification_class;
     word                                   alarm_value;
     word                                   fault_values;
     octet						    		event_enable;
     octet						    		acked_transitions;
     enum BACnetNotifyType		    		notify_type;
     BACnetTimeStamp			            far *event_time_stamps[3];
	 dword									last_notify_record;		//Added by Zhu Zhenhua, 2004-5-11
   } trend_obj_type;

// 5-23-2005 Shiyuan Xiao
// --------------------------------------------------------
// Proprietary Object
// --------------------------------------------------------
typedef struct
{
    generic_object go;
} proprietary_obj_type;

//Shiyuan Xiao. 7/14/2005.
typedef struct
{
    generic_object		           go;
enum BACnetLifeSafetyState 		   present_value;
enum BACnetLifeSafetyState         tracking_value;
    char					       device_type[64];
    octet					       status_flags;
enum BACnetEventState	           event_state;
enum BACnetReliability	           reliability;
    bool					       out_of_service;
enum BACnetLifeSafetyMode          mode;
    BACnetEnumList       far *accepted_modes; // List of BACnetLifeSafetyMode
    word        				   time_delay;
    word				           notification_class;
    BACnetEnumList      far *life_safety_alarm_values; // List of LifeSafetyState
    BACnetEnumList	   far *alarm_values; // List of LifeSafetyState
    BACnetEnumList	   far *fault_values; // List of LifeSafetyState
    octet					       event_enable;
    octet					       acked_transitions;
    enum BACnetNotifyType          notify_type;
    BACnetTimeStamp	 	           far *event_time_stamps[3];
enum BACnetSilencedState           silenced; 
enum BACnetLifeSafetyOperation     operation_expected;
enum BACnetMaintenance             maintenance_required;
    word                           setting;
    float                          direct_reading;
enum BACnetEngineeringUnits        units;
    BACnetDeviceObjectReference    far *member_of;  
} lifesafetypoint_obj_type;

//Shiyuan Xiao. 7/14/2005.
typedef struct
{
    generic_object		           go;
enum BACnetLifeSafetyState 		   present_value;
enum BACnetLifeSafetyState         tracking_value;
    char					       device_type[64];
    octet					       status_flags;
enum BACnetEventState	           event_state;
enum BACnetReliability	           reliability;
    bool					       out_of_service;
enum BACnetLifeSafetyMode          mode;
	BACnetEnumList       far *accepted_modes; // List of BACnetLifeSafetyMode
    word        				   time_delay;
    word				           notification_class;
    BACnetEnumList      far *life_safety_alarm_values; // List of LifeSafetyState
    BACnetEnumList	   far *alarm_values; // List of LifeSafetyState
    BACnetEnumList	   far *fault_values; // List of LifeSafetyState
    octet					       event_enable;
    octet					       acked_transitions;
    enum BACnetNotifyType          notify_type;
    BACnetTimeStamp	 	           far *event_time_stamps[3];
enum BACnetSilencedState           silenced; 
enum BACnetLifeSafetyOperation     operation_expected;
     bool                          maintenance_required;
     BACnetDeviceObjectReference   far *zone_members; 
     BACnetDeviceObjectReference   far *member_of;  
} lifesafetyzone_obj_type;

//Shiyuan Xiao. 7/14/2005.
typedef struct
{
    generic_object		           go;
	word 					       present_value;
    char					       device_type[64];
    octet					       status_flags;
enum BACnetEventState	           event_state;
enum BACnetReliability	           reliability;
    bool					       out_of_service;
	BACnetScale                    scale;
enum BACnetEngineeringUnits        units;
    BACnetPrescale                 prescale;
	word                           max_pres_value;
	BACnetDateTime                 value_change_time;
	word                           value_before_change;
	word                           value_set;
	BACnetAccumulatorRecord        logging_record;
	BACnetObjectIdentifier         logging_device;
	word                           pulse_rate;
	word                           high_limit;
	word                           low_limit;
	word                           limit_monitoring_interval;
	word                           notification_class;
	word        				   time_delay;
	octet                          limit_enable;
	octet					       event_enable;	
    octet					       acked_transitions;
	enum BACnetNotifyType          notify_type;
    BACnetTimeStamp	 	     far *event_time_stamps[3];
} accumulator_obj_type;

//Shiyuan Xiao. 7/14/2005.
typedef struct
{
    generic_object		           go;
	float 					       present_value;
	BACnetObjectPropertyReference  input_reference;
	octet					       status_flags;
enum BACnetEventState	           event_state;
enum BACnetReliability	           reliability;
    bool					       out_of_service;
	enum BACnetEngineeringUnits    units;
	float                          scale_factor;
	float                          adjust_value;
	word                           count;
	BACnetDateTime                 update_time;
	BACnetDateTime                 count_change_time;
	word                           count_before_change;
	float                          cov_increment;
	word                           cov_period;
	word                           notification_class;
	word        				   time_delay;
	float                          high_limit;
	float                          low_limit;
	float                          deadband;
	octet                          limit_enable;
	octet					       event_enable;	
    octet					       acked_transitions;
	enum BACnetNotifyType          notify_type;
    BACnetTimeStamp	 	           far *event_time_stamps[3];
} pulseconverter_obj_type;

#define MAX_SHED_LEVELS 255
typedef struct
{
    generic_object					go;
	octet							status_flags;
enum BACnetEventState				event_state;
enum BACnetReliability				reliability;
	BACnetShedLevel					requested_shed_level;
	DWORD							shed_duration;
	DWORD							duty_window;
    BACnetShedLevel					expected_shed_level;
	BACnetShedLevel					actual_shed_level;
	UnsignedList				far *shed_levels;
	char						far	*shed_level_descriptions[MAX_SHED_LEVELS];
    boolean							log_enable;                   
	BACnetDateTime					start_time;
    word						    notification_class;
    octet						    event_enable;
    octet						    acked_transitions;
    enum BACnetNotifyType		    notify_type;
    BACnetTimeStamp			        far *event_time_stamps[3];
    word							time_delay;
	enum BACnetShedState			present_value;
    char 							state_description[132];
	float							full_duty_baseline;

} lc_obj_type;

typedef struct
{
    generic_object		           go;
    octet					status_flags;
enum BACnetEventState		event_state;
enum BACnetReliability		reliability;
    boolean					out_of_service;
    word					notification_class;
    octet					event_enable;
    octet					acked_transitions;
    enum BACnetNotifyType	notify_type;
    BACnetTimeStamp		far *event_time_stamps[3];
    word				    time_delay;
    BACnetEnumList		far *alarm_values;
    BACnetEnumList		far	*fault_values;

	enum BACnetDoorValue	present_value;
	enum BACnetDoorValue	relinquish_default;
	enum BACnetDoorValue	priority_array[16];
	enum BACnetDoorStatus	door_status;
	enum BACnetLockStatus	lock_status;
	enum BACnetDoorSecuredStatus	secured_status;
	BACnetDeviceObjectReference far *door_members;
	word					door_pulse_time;
	word					door_extended_pulse_time;
	word					door_unlock_delay_time;
	word					door_open_too_long_time;
	enum BACnetDoorAlarmState	door_alarm_state;
	BACnetEnumList			far *masked_alarm_values; //enum BACnetDoorAlarmState	
	enum BACnetMaintenance	maintenance_required;
} ad_obj_type;

#define MAX_SV_ANNOTATIONS 255
// Structured-View
typedef struct
{
    generic_object		           go;
	enum BACnetNodeType				node_type;
	char							node_subtype[132];
	BACnetDeviceObjectReference far *subordinate_list;
	char						far *subordinate_annotations[MAX_SV_ANNOTATIONS];
} sv_obj_type;

typedef struct
{
    generic_object		           go;
    octet					status_flags;
enum BACnetEventState		event_state;
enum BACnetReliability		reliability;
     boolean							    log_enable;                   
     BACnetDateTime						    start_time;
     BACnetDateTime						    stop_time;
     word							        buffer_size;
     BACnetLogRecord					    log_buffer;
     word						    	    record_count;
     dword						    	    total_record_count;			//should be Unsigned32, GJB 01-MAR-2004
     word						    	    notification_threshold;
     word						    	    records_since_notification;
	 dword									last_notify_record;		//Added by Zhu Zhenhua, 2004-5-11
     word						    	   	notification_class;
     octet						    		event_enable;
     octet						    		acked_transitions;
     enum BACnetNotifyType		    		notify_type;
     BACnetTimeStamp			            far *event_time_stamps[3];
} el_obj_type;

typedef struct
{
    generic_object		           go;
    octet					status_flags;
enum BACnetEventState		event_state;
enum BACnetReliability		reliability;
     boolean							    log_enable;                   
     BACnetDateTime						    start_time;
     BACnetDateTime						    stop_time;
     BACnetDeviceObjectPropertyReference    log_device_object_property;
	 enum BACnetLoggingType					logging_type;
	 boolean								align_intervals;
	 word									interval_offset;
	 boolean								trigger;
	 boolean								stop_when_full;
     word							        log_interval;
     word							        buffer_size;
     BACnetLogRecord					    log_buffer;
     word						    	    record_count;
     dword						    	    total_record_count;			//should be Unsigned32, GJB 01-MAR-2004
     word						    	    notification_threshold;
     word						    	    records_since_notification;
	 dword									last_notify_record;		//Added by Zhu Zhenhua, 2004-5-11
     word						    	   	notification_class;
     octet						    		event_enable;
     octet						    		acked_transitions;
     enum BACnetNotifyType		    		notify_type;
     BACnetTimeStamp			            far *event_time_stamps[3];
} tlm_obj_type;

// --------------------------------------------------------
// A placeholder until *someone* fleshs out the real  
// properties of a new Object Type.
//-------------------------------------------------------
typedef struct {
    generic_object			go;
} placeholder_obj_type;

#define MAX_FAULT_STRINGS 8
// --------------------------------------------------------
//Characterstring Value Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    char				    present_value[MAX_TEXT_STRING];
    octet					status_flags;
    enum BACnetEventState	event_state;
    enum BACnetReliability	reliability;
    bool					out_of_service;
    word				    priority_array[16];                        
    char				    relinquish_default[MAX_TEXT_STRING];
    word				    time_delay;
    word				    notification_class;
    char					*alarm_values[MAX_FAULT_STRINGS];
    char					*fault_values[MAX_FAULT_STRINGS];
    octet					event_enable;
    octet					acked_transitions;
    enum BACnetNotifyType	notify_type;
    BACnetTimeStamp		far *event_time_stamps[3];
   } charstring_obj_type;

//-------------------------------------------------------
//Integer Value Object INTEGER_VALUE
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    int						present_value;
    octet					status_flags;
enum BACnetEventState		state;
enum BACnetReliability		reliability;
    boolean					out_of_service;
enum BACnetEngineeringUnits units;
    int						priority_array[16];
    int						relinquish_default;
    int						cov_increment;
    word					time_delay;
    word					notification_class;
    int						high_limit;
    int						low_limit;
    unsigned int			deadband;
    octet					limit_enable;
    octet					event_enable;
    octet					acked_transitions;
enum BACnetNotifyType		notify_type;
     BACnetTimeStamp	    far *event_time_stamps[3];  //Added by xuyiping 2002-8-29
   } integer_obj_type;

//-------------------------------------------------------
//Positive Integer Value Object POSITIVE_INTEGER_VALUE
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    int						present_value;
    octet					status_flags;
enum BACnetEventState		state;
enum BACnetReliability		reliability;
    boolean					out_of_service;
enum BACnetEngineeringUnits units;
    int						priority_array[16];
    int						relinquish_default;
    int						cov_increment;
    word					time_delay;
    word					notification_class;
    int						high_limit;
    int						low_limit;
    unsigned int			deadband;
    octet					limit_enable;
    octet					event_enable;
    octet					acked_transitions;
enum BACnetNotifyType		notify_type;
     BACnetTimeStamp	    far *event_time_stamps[3];  //Added by xuyiping 2002-8-29
   } positive_integer_obj_type;

//-------------------------------------------------------
//DateTime Value Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    BACnetDateTime			present_value;
    octet					status_flags;
enum BACnetEventState		state;
enum BACnetReliability		reliability;
    boolean					out_of_service;
    BACnetDateTime			priority_array[16];
    BACnetDateTime			relinquish_default;
    bool					is_utc;
   } datetimevalue_obj_type;

#endif //__STDOBJ_H_INCLUDED
