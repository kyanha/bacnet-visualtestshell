/*--------------------------------------------------------------------------------

module:		EPICSConsCheck.h
desc:		used for EPICS consistency check
authors:	Liangping,Xu(xlp)
last edit:	3-Nov-02 [016] Liangping,Xu
----------------------------------------------------------------------------------*/
///////////////////////////////////////////////////////////////////////////////////
//
//#include "vts.h"

///////////////////////////////////////////////////////////////////////////////////
//
#define Max_Instance_Num 100
#define Max_ObjServ_Num  50 
#define Protocol_Serv_Sup 15        

///////////////////////////////////////////////////////////////////////////////////
/// Describe object id for each object type
typedef struct tagTObjConsCHK
  { word                  object_type;   //0...MAX_DEFINED_OBJ
    dword                 ObjInstanceSum; //HASH code cout value
    dword                 ObjInstance[Max_Instance_Num];//array of obj instance of each Obj Type
    octet                 ObjInstanceNum; //the number of Obj instance for each Obj Type
    octet                 ObjIDSupported; //indicate how this Obj type is supported 
   }TObjConsCHK;

TObjConsCHK  ObjInTestDB[MAX_DEFINED_OBJ];
TObjConsCHK  DevObjList[MAX_DEFINED_OBJ];

///////////////////////////////////////////////////////////////////////////////////
/// Describe object type listed by protocol_object_supported property
typedef struct tagDevProtocolSup{
	octet *PropSupValue;            
    //octet  ProtocolSup[Max_ObjServ_Num]; 
	word   ObjServNum;
}DevProtocolSup;

DevProtocolSup  ProtocolObjSup;
DevProtocolSup  ProtocolServSup;









  


