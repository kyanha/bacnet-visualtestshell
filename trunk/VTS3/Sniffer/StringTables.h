/*  ------ BACnet string table s--------------- */

#ifndef BACNET_STRING_TABLE_INCLUDED
#define BACNET_STRING_TABLE_INCLUDED

class CComboBox;

//#include "BACnetStringTables.h"
namespace NetworkSniffer {

// Return a test buffer for short term use, such as as an argument
// for sprintf.  Buffers are allocated from a small rotary set, so
// long-term usage will result in the buffer being re-used.
char* TempTextBuffer();


// To safely export string tables, we export them as structs
// containing a pointer to the table, and a count of elements
struct BACnetStringTable
{
	const char* const *m_pStrings;
	const int		  m_nStrings;	// number of strings in the table
	const int		  m_nReserved;	// max+1 value reserved for ASHRAE
	const int		  m_nMax;		// max+1 value allowed for proprietary extension

	// Constructors
	BACnetStringTable( const char* const *pStrings,
					   const int		  nStrings );
	
	BACnetStringTable( const char* const *pStrings,
					   const int		  nStrings,
					   const int		  nReserved,
					   const int		  nMax );

	// Return a string containing text for the specified enumerated value.
	// If the value is undefined, the string will show the pUndefined title and the numeric value
	const char* EnumString( int theIndex, const char *pUndefined = NULL ) const;

	// Fill a CComboBox with the contents of the string table
	void FillCombo( CComboBox &theCombo ) const;
};

#define STRING_TABLE const char* const

// Define a BACnetStringTable, and declare the table itself
#define BAC_STRINGTABLE(name) BACnetStringTable BAC_STRTAB_##name(name, sizeof(name)/sizeof(name[0]))

// Define a BACnetStringTable specifying limits for ASHRAE and proprietary extension
#define BAC_STRINGTABLE_EX(name, nReserved, nMax) BACnetStringTable BAC_STRTAB_##name(name, sizeof(name)/sizeof(name[0]), nReserved, nMax)


// Export a BACnetStringTable, and the table itself
#define EXPORT_STRINGTABLE(name) \
	extern BACnetStringTable BAC_STRTAB_##name

EXPORT_STRINGTABLE(FalseTrue);
EXPORT_STRINGTABLE(ApplicationTypes);
EXPORT_STRINGTABLE(BACnetAccumulatorStatus);
EXPORT_STRINGTABLE(BACnetAction);
EXPORT_STRINGTABLE(BACnetBinaryPV);
EXPORT_STRINGTABLE(BACnetDeviceStatus);
EXPORT_STRINGTABLE(BACnetDoorAlarmState);
EXPORT_STRINGTABLE(BACnetDoorSecuredStatus);
EXPORT_STRINGTABLE(BACnetDoorStatus);
EXPORT_STRINGTABLE(BACnetDoorValue);
EXPORT_STRINGTABLE(BACnetEngineeringUnits);
EXPORT_STRINGTABLE(BACnetError);
EXPORT_STRINGTABLE(BACnetErrorClass);
EXPORT_STRINGTABLE(BACnetErrorCode);
EXPORT_STRINGTABLE(BACnetEventState);
EXPORT_STRINGTABLE(BACnetEventTransitionBits);
EXPORT_STRINGTABLE(BACnetEventType);
EXPORT_STRINGTABLE(Acknowledgement_Filter);
EXPORT_STRINGTABLE(EventState_Filter);
EXPORT_STRINGTABLE(BACnetFileAccessMethod);
EXPORT_STRINGTABLE(BACnetLifeSafetyMode);
EXPORT_STRINGTABLE(BACnetLifeSafetyOperation);
EXPORT_STRINGTABLE(BACnetLifeSafetyState);
EXPORT_STRINGTABLE(BACnetLimitEnable);
EXPORT_STRINGTABLE(BACnetLockStatus);
EXPORT_STRINGTABLE(BACnetLoggingType);
EXPORT_STRINGTABLE(BACnetLogStatus);
EXPORT_STRINGTABLE(BACnetMaintenance);
EXPORT_STRINGTABLE(BACnetNodeType);
EXPORT_STRINGTABLE(BACnetNotifyType);
EXPORT_STRINGTABLE(BACnetObjectType);
EXPORT_STRINGTABLE(BACnetPolarity);
EXPORT_STRINGTABLE(BACnetProgramError);
EXPORT_STRINGTABLE(BACnetProgramRequest);
EXPORT_STRINGTABLE(BACnetProgramState);
EXPORT_STRINGTABLE(BACnetPropertyIdentifier);
EXPORT_STRINGTABLE(BACnetPropertyStates);
EXPORT_STRINGTABLE(BACnetReliability);
EXPORT_STRINGTABLE(BACnetRestartReason);
EXPORT_STRINGTABLE(BACnetResultFlags);
EXPORT_STRINGTABLE(BACnetShedState);
EXPORT_STRINGTABLE(BACnetSegmentation);
EXPORT_STRINGTABLE(BACnetServicesSupported);
EXPORT_STRINGTABLE(BACnetSilencedState);
EXPORT_STRINGTABLE(BACnetStatusFlags);
EXPORT_STRINGTABLE(BACnetVendorID);
EXPORT_STRINGTABLE(BACnetVTClass);
EXPORT_STRINGTABLE(BACnetDaysOfWeek);
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
EXPORT_STRINGTABLE(BACnetBackupState);
} // end namespace NetworkSniffer

#endif
