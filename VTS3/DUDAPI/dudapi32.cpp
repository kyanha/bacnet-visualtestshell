/*	29-Jan-01 [003] JJB revise for C++, added PICS namespace and typecasts
	09-Jun-98 [002] JN  fix to CheckFunctionalGroup
						fix integer->short
    23-Mar-98 [001] JN  32-bit version (not all marked) 
						 eg. far pascal __export --> APIENTRY
*/

#include "stdafx.h"									//			***001 Begin
//#include <afx.h>			
//#include <afxwin.h>								//			***001 End

#include <windows.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

namespace PICS {															// ***003

#define _DoStaticPropDescriptors 1

#include "db.h" 
#include "stdobj.h"
#include "stdobjpr.h"
#include "vtsapi.h" 
#include "bacprim.h"

// - jjb #include "resource.h"   // resource identifiers
// - jjb #include "dudclass.h" // derived classes 
#include "dudapi.h"   // export interface
#include "dudtool.h"  // tool functions

#include "props.h"                 
#include "propid.h"
#include "VTS.h"

const char* gThisDLLName= "DUDAPI32.DLL"; // literal name of this dll			***001
static HMODULE hMod= 0; // instance handle of this module						***001

//int		far pascal _WEP(int);					//			***001        
int     CheckTableProp(generic_object far* pObj, TApplServ far* pApplServ);
//int     CheckClass(int n, TApplServ far* pApplServ, char far ApplServ[MAX_SERVS_SUPP], char far Result[35], 
//                  generic_object far* root, TObjProp far resObj[64], short far* eol);	//***002




// local variables -------------------------------------------------------------------------------------------

static char gBuffer[256];
static int  gInitServer= 0;



// ===========================================================================================================
//
// export functions
//
// ===========================================================================================================
           


// This function is used to paint a standard icon on a window. There are no standard icons
// in Visual Basic. Look up "windows.h" for the ID of a specific standard icon.
//
// in:   x, y   logical position of the icon.
//       hw     Windows handle
//       id     id of standard icon (IDI_ASTERISK, IDI_ ...)
extern "C"
void  APIENTRY DrawStdIcon (int x, int y, HWND hw, int id )
{ 

  HICON icon;
  HDC dc;
  int oldmode;
  
  dc = GetDC(hw);    
  oldmode= GetMapMode(dc);
  SetMapMode(dc, MM_TEXT);
  icon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_ASTERISK));
  DrawIcon(dc, x, y, icon);
  SetMapMode(dc,oldmode);
  ReleaseDC(hw, dc);
  
}


// This function is used to copy an generic_object. Need for visual basic.
extern "C"
void APIENTRY CpyGenObj(generic_object far* godst, generic_object far* gosrc)
{ 
  if ((godst==NULL)||(gosrc==NULL)) return;
  memcpy(godst,gosrc,sizeof(generic_object));			//			***001
}
                                          

// This function is used to step through the generic_object list. 
//
// in:   obj   generic_object 
// out:  obj   next generic_object
//
// returns: -1 end of list, else 0
extern "C"
short APIENTRY GetNextGenObj(generic_object far* obj)	//		***002
{ 
  if ((obj==NULL)||(obj->next==NULL)) return(-1);
  memcpy(obj,obj->next,sizeof(generic_object));		//			***001
  return(0);
}
   
   
// This function is used to step through the standard object list. The function retrieves 
// the pointer to a specified object in the object list. Use FindPicsObj() and GetPropValue()
// to read or write values.
//
// in:       root   points to the root of the object list
//           index  zerobased index for the object you want to obtain the pointer
// returns:  a long pointer to the indexed object.
extern "C"
generic_object far* APIENTRY FindGenObj(generic_object far* root, int index)
{ 
  while ((index--) && (root!=NULL)) root= (generic_object far*) root->next;
  return(root);
}

                                          
// This function is used to obtain the name of a Property. 
//
// in:   PropName     points to a buffer to contain the property name
//       objtype      property for this BACnetObjectType
//       i            zero-based index in property table
//       propFlags    bitstream to determine if property is supported/not supported   
// out:  PropId       property ID
//
// returns: >0 if property supported, -1 if invalid property index, 0 if not supported 
extern "C"
short APIENTRY GetPropNameSupported(char far* PropName, word i, word objtype, octet far propFlags[64], dword far* PropId) //***002
{ 
  if ( (propFlags[i] & 1) == 1 )
    {                                   
      *PropId= VTSAPIgetpropinfo(objtype,i,PropName,NULL,NULL,NULL,NULL);
      if (*PropId == -1) return(-1); // invalid property index
      else return( strlen(PropName) );			//						***001
    }  
  else return(0); // not supported
}



// This function is used to find a generic_object identified by its object id.
// in:		root		root of object list
//			ObjectId	object being sought
// returns: pointer to matching object, or NULL
extern "C"
generic_object far* APIENTRY GetpObj(generic_object far* root, dword ObjectId)  
{ 
  while (root!=NULL)
    { 
      if (root->object_id == ObjectId) // gotcha!
        break;
      root= (generic_object far*) root->next;
    }
  return root;
}



/* no longer used
   mandanner 10/02   
// This function is used to retrieve the property value of an object.
//
// in:
//       Buffer     points to a buffer to contain the string value of the Property
//       msg		PVMessage
//
// returns: 0 if ok, else !=0
extern "C"
short APIENTRY GetPropValue(char far* Buffer, PVMessage far* msg)	//***002
{ 
	ASSERT(Buffer != NULL && msg != NULL);

	if ( msg->Obj == NULL )
		return(-1); // no generic_object

	LoadObjectTypedData(msg);
	return (short) EncodeFromDatabase(Buffer, msg);
}
*/



// This Function is used to obtain the literal name of a BACnetObjectType.
// in:    objtype   enumerated BACnetObjectType
//        Buffer    points to a buffer to contain name
// returns: >0 if ok, else 0
extern "C"
short APIENTRY GetObjType(word objtype, char far* Buffer)	//			***002
{ word pet;
  VTSAPIgetpropinfo(objtype,2,Buffer,NULL,NULL,NULL,&pet); // 2 == ObjTypeProperty
  VTSAPIgetenumtable(pet,objtype,NULL,NULL,Buffer);
  return( strlen(Buffer) );									//			***001
}                                                         


// This function is used to obtain the name of an enumerated property value.
// in:    Name     buffer to contain name of enumeration
//        objtype  property is in this object
//        PropId   ID of the property as defined in propid.h
//        EnumVal  enumerated value for which you want to retrieve name
// returns: >0 if ok, 0 else
extern "C"
short APIENTRY GetEnumName(char far* Name, word objtype, dword PropId, word EnumVal) //***002
{ word pet,i; dword pid, pcount; 

  pcount= VTSAPIgetpropinfo(objtype,0xFFFF,Name,NULL,NULL,NULL,&pet); // how many properties?

  if (pcount==0xFFFFFFFF) { Name[0]= 0; return(0); } // objtype invalid
  i= (word)pcount;
  while ( (--i) >=0 ) // iterate all properties
    { pid= VTSAPIgetpropinfo(objtype,i,Name,NULL,NULL,NULL,&pet);
      if (pid==PropId) break; // check if PropId was found
    }
  if (i<0) return(0); // PropId not found 
  Name[0]= 0; // reset buffer
  i= VTSAPIgetenumtable(pet,EnumVal,NULL,NULL,Name); // get enumeration name
  
  return ( strlen(Name) );							//			***001
}                                                   



// This function is used to obtain the name of a property PropID.
// in:    PropId   ID of the property as defined in propid.h
//        PropName buffer to contain name of property
// returns: >0 if ok, 0 else
extern "C"
short APIENTRY GetPropName(dword PropId, char far* PropName)	//	***002
{ 
  dword count = 0;

  while (stPropIDs[count]){count++;}
  PropName[0]= 0;
  if ((PropId>=0)&&(PropId<count)) 
    { strcpy(PropName,stPropIDs[PropId]);		//			***001
      return(strlen(PropName));							//			***001
    }
  else return(0);  
}                                                   



// This function is used to read a string resource from the resource file. The Function 
// uses the literal name "DUDAPI32.DLL" to retrieve the instance handle of this module.
//                                                                                   
// in:		id			ID of the string resource
//    		Buffer		to contain the string resource
//			Size		size of the buffer >0
extern "C"
short APIENTRY LoadStringRes(int id, char far* Buffer, int Size)	//	***002
/*{																		***001 Begin
  TRY
    { 
      CString s; int slen, max;
	  s.LoadString(id);
      slen= s.GetLength();
      (Size>slen)?(max= slen):(max= Size);
      strncpy(Buffer, (const char*)s , max);				//			***001
      Buffer[max]= 0;
      return(max); 
    }
  CATCH_ALL(e)
    {
      // a failure caused an exception.
      Buffer[0]= 0;
      return 0;
    }
  END_CATCH_ALL 
                       
}*/
{ short c;													//			***002
  if (hMod==0) // first call to function
    hMod= GetModuleHandle(gThisDLLName);  // retrieve instance handle
  c= (short)LoadString(hMod,id,Buffer,Size);     // load string into buffer ***002
  return(c);
}											//							***001 End

// This function is used to get the property index of a property.        
extern "C"
short APIENTRY GetPropIndex(word object_type, dword PropId)
{ dword c; dword id;
  c= VTSAPIgetpropinfo(object_type,0xFFFF,NULL,NULL,NULL,NULL,NULL);
  for (short i=0; (dword)i<c; i++)
    { id= VTSAPIgetpropinfo(object_type,i,gBuffer,NULL,NULL,NULL,NULL);
      if (id==PropId) return(i);
    }
  return -1;
}        
 
//Marked by xlp,2002-11          
           
// This function is used to check a conformance class or a functional group.
// in:		n				number of lines in table
//			pApplServ		Table containing class requirements
//			ApplServ		appl services supported by the given device
//		    root			object list (objects supported)
// out:     Result  		appl services missing to complete the class
//			resObj  		objects and properties missing (int ObjType, int PropIndex...)
/*int CheckClass(int n, TApplServ far* pApplServ, char far ApplServ[MAX_SERVS_SUPP], char far Result[35], 
                           generic_object far* root, TObjProp far resObj[64], short far* eol)
{
  int line; enum BACnetApplServ serv; 
  int allChecked, obj_supported;
  char InitExec;

  int rval= 1; // default: class supported

  for (line= 0; line<n; line++) // for all lines in table:
    { 
      serv= pApplServ[line].ApplServ; // required application service 0..34
      if (serv>=0)
        { InitExec= pApplServ[line].InitExec;
          if ( (ApplServ[serv] & InitExec) != pApplServ[line].InitExec )
            { Result[serv]= Result[serv] | ( (InitExec ^ ApplServ[serv]) & InitExec ); 
              rval= 0;
            } 
        }
    } // application services checked!           
      
  int iProp;       // property index
  generic_object far* pObj= root;
  while (pObj!=NULL) // step through object list, check required objects/properties
    { 
      obj_supported= 1;
      for (line= 0; line<n; line++)
	  {
          if (pApplServ[line].object_type == (word)pObj->object_type) // check this object:
            { 
              if (pApplServ[line].property_id!=-0xFFFF) // property required	***002
                {
                  iProp= GetPropIndex(pObj->object_type,pApplServ[line].property_id);
                  if ((pObj->propflags[iProp] & 1)==1) // property supported
                    pApplServ[line].object_type= 0xFFFE;  // temporary mark		***002
                  else
                    obj_supported= 0; // not supported  
                }
              else // object supported
                pApplServ[line].object_type= 0xFFFE; // temporary mark			***002
            }
        }
      
      allChecked= 1;  
      for (line= 0; line<n; line++)   
        { 
          if (pApplServ[line].object_type==0xFFFE)					//			***002
            if (obj_supported) pApplServ[line].object_type= 0xFFFF; // obj supports requirement!***002
            else pApplServ[line].object_type= pObj->object_type;	// write back object type
            
          if (pApplServ[line].object_type!=0xFFFF) allChecked= 0;	//			***002
        }
      if (allChecked) break; // all lines in table checked 
      pObj= (generic_object far*)pObj->next;
      
    } // while        
  
  short iObjProp= *eol; // index in result array					//			***002
    
  for (line= 0; line<n; line++) // collect all lines not supported:
    {
      if (pApplServ[line].object_type!=0xFFFF) // not supported		//			***002
        { iObjProp++;
          if (iObjProp>63) break;
          resObj[iObjProp].object_type= pApplServ[line].object_type;
          resObj[iObjProp].property_id= pApplServ[line].property_id;
        }
    }
      
  *eol= iObjProp;
             
  if (iObjProp>0) rval= 0; // class not supported
  return(rval);  
    
}     */     



// This Function is used to check, whether a BACnet device supports all required application services
// in order to belong to a particular conformance class.  
// in:		ConfClass		conformance class (1..5)
//			ApplServ		appl services supported by the given device
//		    root			object list supported
// out:     Result  		appl services missing to complete the conformance class
//			resObj  		objects and properties missing (int ObjType, int PropIndex...)
// retunrs  1 if supported, 0 else			
/*extern "C"
short APIENTRY CheckConfClass(word ConfClass, char far ApplServ[35], char far Result[35], 
                                   generic_object far* root, TObjProp far resObj[64], short far* eol)
{  short rval= 1; // default: ConfClass supported					***002
  
  memset(Result, 0, MAX_SERVS_SUPP);						//					***001
  memset(Result, 0, 35);						//					***001
  memset(resObj,0,64*sizeof(TObjProp));			//					***001
  *eol= -1;
  
  switch (ConfClass)
    { 
      case 6:  
        //if ( CheckClass(nFgClock,gFgClock,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
        //if ( CheckClass(nFgPCWS,gFgPCWS,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
        //if ( CheckClass(nFgEventInit,gFgEventInit,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
        //if ( CheckClass(nFgEventResponse,gFgEventResponse,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
        //if ( CheckClass(nFgFiles,gFgFiles,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
        break;
      case 5:  
        if ( CheckClass(nCC5,gCC5_Table,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
        break;
      case 4:
        if ( CheckClass(nCC4,gCC4_Table,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
        break;
      case 3:
        if ( CheckClass(nCC3,gCC3_Table,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
        break;
      case 2:
        if ( CheckClass(nCC2,gCC2_Table,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
        break;
      case 1:
      default:
        if ( CheckClass(nCC1,gCC1_Table,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
        break;
    }    
    
  return(rval);  
}*/
  

/*extern "C"
short APIENTRY CheckFunctionalGroup(dword FuncGroup, char far ApplServ[35], char far Result[35], 
                                          generic_object far* root, TObjProp far resObj[64], short far* eol)
{ 
   short rval= 1; // default: ConfClass supported					***002
  
  memset(Result, 0, 35);						//					***001
  memset(resObj,0,64*sizeof(TObjProp));			//					***001
  *eol= -1;
  
  switch (FuncGroup)
    { 
   		case fgHHWS:				
          if ( CheckClass(nFgHHWS,gFgHHWS,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
   		  break;
   		case fgPCWS:				
          if ( CheckClass(nFgPCWS,gFgPCWS,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
   		  break;
   		case fgCOVEventInitiation:	
          if ( CheckClass(nFgCOVInit,gFgCOVInit,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
   		  break;
   		case fgCOVEventResponse:	
          if ( CheckClass(nFgCOVResponse,gFgCOVResponse,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
   		  break;
   		case fgEventInitiation:		
          if ( CheckClass(nFgEventInit,gFgEventInit,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
   		  break;
   		case fgEventResponse:		
          if ( CheckClass(nFgEventResponse,gFgEventResponse,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
   		  break;
   		case fgClock:				
          if ( CheckClass(nFgClock,gFgClock,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
   		  break;
   		case fgDeviceCommunications:
          if ( CheckClass(nFgDevCom,gFgDevCom,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
   		  break;
   		case fgFiles:		
          if ( CheckClass(nFgFiles,gFgFiles,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
   		  break;
   		case fgTimeMaster:	
          if ( CheckClass(nFgTimeMaster,gFgTimeMaster,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
   		  break;
   		case fgVirtualOPI:	
          if ( CheckClass(nFgVO,gFgVO,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
   		  break;
   		case fgReinitialize:
          if ( CheckClass(nFgReinitialize,gFgReinitialize,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
   		  break;
   		case fgVirtualTerminal:
          if ( CheckClass(nFgVT,gFgVT,ApplServ,Result,root,resObj,eol)== 0 ) rval= 0;
   		  break;
    }
    
  return (rval);  
}*/



extern "C"
short APIENTRY DevApplServCheck(char far ApplServ[MAX_SERVS_SUPP], generic_object far* root, 
              					char far resApplServ[35])					//***002
{ device_obj_type far* pdev= NULL; 
  while (root!=NULL)
    { 
      if (root->object_type==DEVICE)
        { pdev= (device_obj_type far*)root;
          break;
        } 
      root= (generic_object far*)root->next;  
    }
  if (pdev==NULL) return(0);  
  
  int iAppl=  0x80;
  int iOctet= 0;
  short rval=   1;						//							***002
  memset(resApplServ, 0, 35);			//							***001

  for (int i= 0; i<35; i++)
    { 
      if (  ((ApplServ[i] & ssExecute) == ssExecute) &&
            ( (pdev->protocol_services_supported[iOctet] & iAppl) != iAppl ) // not supported!
         )   
        {
          resApplServ[i]= ssExecute | 0x04; // missing in protocol_services_supported
          rval= 0;
        }
      
      if (  ((pdev->protocol_services_supported[iOctet] & iAppl) == iAppl) &&
            ( (ApplServ[i] & ssExecute) != ssExecute ) // not supported!
         )
        {
          resApplServ[i]= ssExecute | 0x08;  // missing in application services supported (PICS)
          rval= 0;
        }
        
      iAppl/= 2;
      if (iAppl==0) { iAppl= 0x80; iOctet++; }
    }         
    
  return rval;  
  
}              													 


extern "C"
BACnetObjectIdentifier far* APIENTRY GetObjIdRoot(generic_object far* pdbRoot)
{ // find the device object
  device_obj_type far* pdev= NULL; 
  while (pdbRoot!=NULL)
    { if (pdbRoot->object_type==DEVICE)
        { pdev= (device_obj_type far*)pdbRoot;
          break;
        } 
      pdbRoot= (generic_object far*)pdbRoot->next;  
    }
  if (pdev==NULL) return(NULL);  
  return(pdev->object_list); // list of object IDs
 }


// This function is used to check, if an element of object_list 
// (BACnetObjectIdentifier far*) is present in the PICS Database. Afterwards,
// the pointer to the element of object_list will point to the next element in 
// object_list.
// in:			pdbRoot		root of PICS Database
//			pid		pointer to a long value. This long value is the 
//					  address of the object_list element to be checked.
// out:			pid		address of next element in list (or NULL= end of 
//					  list)
//			ObjId		object id of element 
// returns:	1 if element is in PICS Database, 0 else.
extern "C"
short APIENTRY pIDinList(generic_object far* pdbRoot, long far* pid, dword far* ObjId)	//***002
{ 
  BACnetObjectIdentifier far* p= (BACnetObjectIdentifier far*) *pid;
  if (p==NULL) { *ObjId= 0; return(0); }
  *ObjId= p->object_id;
  while (pdbRoot!=NULL)
    { if (pdbRoot->object_id==p->object_id) break;
      pdbRoot= (generic_object far*)pdbRoot->next;
    }

 *pid= (long)p->next; // next pid
 if (pdbRoot!=NULL) 
   return(1);         // pid in pdb list
 else 
   return(0);         // pid not in pdb list  
}


// This function is used to check, if a particular object of the PICS Database is present 
// in the list of property Object_List of the Device object.
// in:			pidRoot		points to Object_List
//			pdb		points to a long value. This long value contains the 
//					  address of the object to be checked.
// out:			pdb		address of next object
//			ObjId		object id of object
// returns:	1 if object is in Object_List, 0 else
extern "C"
short APIENTRY pDBinList(BACnetObjectIdentifier far* pidRoot, long far* pdb, dword far* ObjId) //***002
{ 
  generic_object far* p= (generic_object far*) *pdb;
  if (p==NULL) { *ObjId= 0; return(0); }
  *ObjId= p->object_id;
  while (pidRoot!=NULL)
    { if (pidRoot->object_id==p->object_id) break;
      pidRoot= (BACnetObjectIdentifier far*)pidRoot->next;
    }
  *pdb= (long)p->next; // next pdb 
  if (pidRoot!=NULL) 
    return(1);         // pid in pdb list
  else 
    return(0);         // pid not in pdb list  
}                                                                      


// This function is used to check the object types supported in:
//   the PICS "object types supported"
//   property Object_Types_Supported of the Device object
// in:		StdObj			array 1..18 indicating supported object types
//			pdbRoot			root of PICS Database
// out:		resObjDev[18]	missing object types in Device object (indicated by 1)
//			resObjPICS[18]	missing object types in PICS (indicated by 1)
// returns: 0 if at least one object type is missing, 1 else
extern "C"
short APIENTRY CheckObjTypeDevPics(char far* StdObj, generic_object far* pdbRoot,
                                          octet far resObjDev[MAX_DEFINED_OBJ], octet far resObjPICS[MAX_DEFINED_OBJ])	//***002
{ // find the device object
  device_obj_type far* pdev= NULL; 
  while (pdbRoot!=NULL)
    { if (pdbRoot->object_type==DEVICE)
        { pdev= (device_obj_type far*)pdbRoot;
          break;
        } 
      pdbRoot= (generic_object far*)pdbRoot->next;  
    }
  if (pdev==NULL) return(0);  
  
  int iOctet= 0;
  int iBit= 0x80;
  short rval= 1; // default: no object type missing					***002

  memset(resObjDev, 0, MAX_DEFINED_OBJ);				//							***001
  memset(resObjPICS, 0, MAX_DEFINED_OBJ);			//							***001
  
  for (int i=0; i<MAX_DEFINED_OBJ; i++)
    { 
      if ( (StdObj[i]>0) && ((pdev->object_types_supported[iOctet] & iBit) == 0) )     // not in Device object
        { resObjDev[i]= 1; rval= 0; }
      if ( (StdObj[i]==0) && ((pdev->object_types_supported[iOctet] & iBit) == iBit) ) // not in PICS section
        { resObjPICS[i]= 1; rval= 0; }
      
      iBit/= 2;
      if (iBit==0) { iOctet++; iBit= 0x80; }
    }          
    
  return(rval);  
}                                          


// This function is used to extract object type and object instance from an object identifier.
// in: 		ObjId		object identifier
// out:		ObjType		buffer to contain object type
//			ObjInst		buffer to contain object instance
extern "C"
void APIENTRY SplitObjectId(dword ObjId, word far* ObjType, dword far* ObjInst)
{
  if (ObjType!=NULL) *ObjType= (word) (ObjId>>22); 
  if (ObjInst!=NULL) *ObjInst= ObjId & 0x3FFFFF;
}


// This function is used to convert 2 octets to an integer value. The first
// octet is the least significant byte of the integer value.
// in:		sInt		octet string (low byte, high byte)
// returns	integer value	
extern "C"
short APIENTRY CMyInt(unsigned char far* sInt)						//***002
{
  return ( sInt[0] + sInt[1]*256 );  // so easy! (you can't do this in VB)
}


// This function is used to convert 4 octets to an long value. The first
// octet is the most significant byte of the long value.
// in:		sLong		octet string 
// returns	long value	
extern "C"
long APIENTRY CMyBigLong(unsigned char far* sLong)
{ 
  long f=256; long l;
  
  l=  sLong[3]; 
  l+= sLong[2]* f; f= f*256;
  l+= sLong[1]* f; f= f*256;
  l+= sLong[0]* f; 
  
  return (l);
}


// This function is used to convert 2 octets to an integer value. The first
// octet is the most significant byte of the integer value.
// in:		sInt		octet string (high byte, low byte)
// returns	integer value	
extern "C"
short APIENTRY CMyBigInt(unsigned char far* sInt)
{ 
  return ( sInt[1] + sInt[0]*256 );  // so easy! (you can't do this in VB)
}


// This function is used to remove the complete database.
// in:		root		root of object list.
extern "C"
void APIENTRY MyDeletePICSObject(generic_object far* root)
{ generic_object far* p;
  while (root!=NULL)
	{ p= root;
	  root= (generic_object far*)root->next;
	  DeletePICSObject(p);
	}
}


// Every client of dudapi32.dll has to call this function first, in order to create
// test values (dynamic allocated memeory). The dynamic data will be created only
// once (first client)
extern "C"
void APIENTRY InitDudapi(void)
{
  if (gInitServer==0) 
    {
      CreateTestValues();
    }  

  gInitServer++;

}


// Every client of dudapi32.dll has to call this function, in order to release
// dynamic data. The data will be released only once (last client).
extern "C"
void APIENTRY CloseDudapi(void)
{
  if (gInitServer>0) gInitServer--;
  
  if (gInitServer==0) // no active clients
    { 
      DeleteTestValues();
    }  
}
								
}
