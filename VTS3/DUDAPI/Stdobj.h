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
    BACnetTimeStamp	        event_time_stamps[3];  //madanner 6/03, added
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
    BACnetTimeStamp	        event_time_stamps[3];  //madanner 6/03, added
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
     BACnetTimeStamp	    event_time_stamps[3];  //Added by xuyiping 2002-8-29
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
    BACnetTimeStamp	        event_time_stamps[3];  //madanner 6/03, added
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
    BACnetTimeStamp	        event_time_stamps[3];  //madanner 6/03, added
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
	BACnetTimeStamp	        event_time_stamps[3];  //Added by xuyiping 2002-8-29
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

//Command Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
    word 					pv;
    boolean					in_process;
    boolean					all_writes_successful;
    word					num_actions;
    BACnetActionCommand	far	*action[32];
    char				far	*action_text[32];
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
	BACnetDateTime			last_restore_time;
	word					backup_failure_timeout;
	BACnetCOVSubscription   far	*active_cov_subscriptions;

   } device_obj_type;

//Event Enrollment Object
//-------------------------------------------------------
typedef struct {
    generic_object			go;
enum BACnetNotifyType		notify_type;
//Note that event_type is part of parameter_list structure in this implementation!
    BACnetEventParameter	parameter_list;
    BACnetObjectPropertyReference obj_prop_ref;
enum BACnetEventState 		state;
    octet					event_enable;
    octet					acked_transitions;
    word					notification_class;
    BACnetRecipient			recipient;
    word					process_id;
    word					priority;
    boolean					issue_conf_notifications;
    BACnetTimeStamp	        event_time_stamps[3];  //madanner 6/03, added
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
    BACnetTimeStamp	        event_time_stamps[3];  //madanner 6/03, added
   } loop_obj_type;

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
    char				far	*state_text[32];
    word					time_delay;
    word					notification_class;
    octet					alarm_values[32];	//note that 0 marks the end of each "list"
    octet					fault_values[32];
    octet					event_enable;
    octet					acked_transitions;
enum BACnetNotifyType		notify_type;
    BACnetTimeStamp	        event_time_stamps[3];  //madanner 6/03, added
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
    char				far	*state_text[32];
    word					priority_array[16];
    word					relinquish_default;
    word					time_delay;
    word					notification_class;
    octet					feedback_value;
    octet					event_enable;
    octet					acked_transitions;
enum BACnetNotifyType		notify_type;
    BACnetTimeStamp	        event_time_stamps[3];  //madanner 6/03, added
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
    BACnetObjectPropertyReference far *list_obj_prop_ref;
    word					priority_for_writing;
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
    char				far	*state_text[32];
    word				    priority_array[16];                        
    word				    relinquish_default;
    word				    time_delay;
    word				    notification_class;
    word				    alarm_values[10];
    word				    fault_values[10];
    octet					event_enable;
    octet					acked_transitions;
    enum BACnetNotifyType	notify_type;
    BACnetTimeStamp		event_time_stamps[3];
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
     BACnetTimeStamp			            event_time_stamps[3];
	 dword									last_notify_record;		//Added by Zhu Zhenhua, 2004-5-11
   } trend_obj_type;

#endif //__STDOBJ_H_INCLUDED
