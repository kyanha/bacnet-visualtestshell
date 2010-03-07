/*  ------ BACnet string table s--------------- */

//#include "BACnetStringTables.h"
namespace NetworkSniffer {

// To safely export string tables, we export them as structs
// containing a pointer to the table, and a count of elements
struct BACnetStringTable
{
	const char*	 const* m_pStrings;
	int					m_nStrings;
};

// TODO: Eventually replace the macro by "const char* const"
//#define STRING_TABLE const char* const
#define STRING_TABLE char*

// Define a BACnetStringTable, and declare the table itself
#define BAC_STRINGTABLE(name) BACnetStringTable BAC_STRTAB_##name = {name, sizeof(name)/sizeof(name[0])}

// Export a BACnetStringTable, and the table itself
// TODO: eventually DON'T export the table, since it isn't safe without knowning its size
#define EXPORT_STRINGTABLE(name) \
	extern STRING_TABLE name[]; \
	extern BACnetStringTable BAC_STRTAB_##name

EXPORT_STRINGTABLE(ApplicationTypes);
EXPORT_STRINGTABLE(BACnetAction);
EXPORT_STRINGTABLE(BACnetActionList);
EXPORT_STRINGTABLE(BACnetActionCommand);
EXPORT_STRINGTABLE(BACnetAddress);
EXPORT_STRINGTABLE(BACnetAddressBinding);
EXPORT_STRINGTABLE(BACnetBinaryPV);
EXPORT_STRINGTABLE(BACnetCalendarEntry);
EXPORT_STRINGTABLE(BACnetClientCOV);
EXPORT_STRINGTABLE(BACnetScale);
EXPORT_STRINGTABLE(BACnetDateRange);
EXPORT_STRINGTABLE(BACnetDateTime);
EXPORT_STRINGTABLE(BACnetTimeStamp);
EXPORT_STRINGTABLE(BACnetDaysOfWeek);
EXPORT_STRINGTABLE(BACnetDestination);
EXPORT_STRINGTABLE(BACnetDeviceObjectReference);
EXPORT_STRINGTABLE(BACnetDeviceObjectPropertyReference);
EXPORT_STRINGTABLE(BACnetDeviceObjectPropertyValue);
EXPORT_STRINGTABLE(BACnetDeviceStatus);
EXPORT_STRINGTABLE(BACnetDoorAlarmState);
EXPORT_STRINGTABLE(BACnetDoorSecuredStatus);
EXPORT_STRINGTABLE(BACnetDoorStatus);
EXPORT_STRINGTABLE(BACnetDoorValue);
EXPORT_STRINGTABLE(BACnetEngineeringUnits);
EXPORT_STRINGTABLE(BACnetError);
EXPORT_STRINGTABLE(BACnetErrorClass);
EXPORT_STRINGTABLE(BACnetErrorCode);
EXPORT_STRINGTABLE(BACnetEventLogRecord);
EXPORT_STRINGTABLE(BACnetEventParameter);
EXPORT_STRINGTABLE(BACnetEventState);
EXPORT_STRINGTABLE(BACnetEventTransitionBits);
EXPORT_STRINGTABLE(BACnetEventType);
EXPORT_STRINGTABLE(Acknowledgement_Filter);
EXPORT_STRINGTABLE(EventState_Filter);
EXPORT_STRINGTABLE(BACnetFileAccessMethod);
EXPORT_STRINGTABLE(BACnetGetEventInfoACK);
EXPORT_STRINGTABLE(BACnetEventSummary);
EXPORT_STRINGTABLE(BACnetLifeSafetyMode);
EXPORT_STRINGTABLE(BACnetLifeSafetyOperation);
EXPORT_STRINGTABLE(BACnetLifeSafetyState);
EXPORT_STRINGTABLE(BACnetAccumulatorStatus);
EXPORT_STRINGTABLE(BACnetMaintenance);
EXPORT_STRINGTABLE(BACnetSilencedState);
EXPORT_STRINGTABLE(BACnetLimitEnable);
EXPORT_STRINGTABLE(BACnetLockStatus);
EXPORT_STRINGTABLE(BACnetLogData);
EXPORT_STRINGTABLE(BACnetLoggingType);
EXPORT_STRINGTABLE(BACnetLogMultipleRecord);
EXPORT_STRINGTABLE(BACnetLogRecord);
EXPORT_STRINGTABLE(BACnetLogStatus);
EXPORT_STRINGTABLE(BACnetNotifyType);
EXPORT_STRINGTABLE(BACnetObjectPropertyReference);
EXPORT_STRINGTABLE(BACnetPropertyAccessResult);
EXPORT_STRINGTABLE(BACnetShedLevel);
EXPORT_STRINGTABLE(BACnetShedState);
EXPORT_STRINGTABLE(BACnetReadRangeACK);
EXPORT_STRINGTABLE(BACnetReadRangeRequest);
EXPORT_STRINGTABLE(BACnetObjectPropertyValue);
EXPORT_STRINGTABLE(BACnetObjectType);
EXPORT_STRINGTABLE(BACnetObjectTypesSupported);
EXPORT_STRINGTABLE(BACnetPolarity);
EXPORT_STRINGTABLE(BACnetPrescale);
EXPORT_STRINGTABLE(BACnetProgramError);
EXPORT_STRINGTABLE(BACnetProgramRequest);
EXPORT_STRINGTABLE(BACnetProgramState);
EXPORT_STRINGTABLE(BACnetPropertyIdentifier);
EXPORT_STRINGTABLE(BACnetNodeType);
EXPORT_STRINGTABLE(BACnetPropertyReference);
EXPORT_STRINGTABLE(BACnetPropertyStates);
EXPORT_STRINGTABLE(BACnetPropertyValue);
EXPORT_STRINGTABLE(BACnetRecipient);
EXPORT_STRINGTABLE(BACnetRecipientProcess);
EXPORT_STRINGTABLE(BACnetReliability);
EXPORT_STRINGTABLE(BACnetRestartReason);
EXPORT_STRINGTABLE(BACnetSegmentation);
EXPORT_STRINGTABLE(BACnetServicesSupported);
EXPORT_STRINGTABLE(BACnetSessionKey);
EXPORT_STRINGTABLE(BACnetSpecialEvent);
EXPORT_STRINGTABLE(BACnetStatusFlags);
EXPORT_STRINGTABLE(BACnetResultFlags);
EXPORT_STRINGTABLE(BACnetVendorID);
EXPORT_STRINGTABLE(BACnetTimeValue);
EXPORT_STRINGTABLE(BACnetVTClass);
EXPORT_STRINGTABLE(BACnetVTSession);
EXPORT_STRINGTABLE(BACnetCOVSubscription);
EXPORT_STRINGTABLE(BACnetWeekNDay);
EXPORT_STRINGTABLE(day_of_week);
EXPORT_STRINGTABLE(month);
EXPORT_STRINGTABLE(PDU_types);
EXPORT_STRINGTABLE(PDU_typesENUM);
EXPORT_STRINGTABLE(NL_msgs);
EXPORT_STRINGTABLE(BACnetReject);
EXPORT_STRINGTABLE(BACnetAbort);
EXPORT_STRINGTABLE(Selection_Logic);
EXPORT_STRINGTABLE(Relation_Specifier);
EXPORT_STRINGTABLE(BVLL_Function);
EXPORT_STRINGTABLE(BACnetConfirmedServiceChoice);
EXPORT_STRINGTABLE(BACnetUnconfirmedServiceChoice);
EXPORT_STRINGTABLE(BACnetReinitializedStateOfDevice);
EXPORT_STRINGTABLE(DeviceCommControl_Command);
EXPORT_STRINGTABLE(TextMessage_Priority);

} // end namespace NetworkSniffer
