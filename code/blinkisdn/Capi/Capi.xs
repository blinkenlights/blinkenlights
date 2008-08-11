#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <capi20.h>

static const _cstruct es = "\x0";


MODULE = Capi		PACKAGE = Capi		

unsigned
lib_CAPI20_ISINSTALLED()
    CODE:
	RETVAL = (CAPI20_ISINSTALLED() == CapiNoError);
    OUTPUT:
	RETVAL

unsigned
lib_CAPI20_REGISTER(MaxB3Connection, MaxB3Blks, MaxSizeB3)
	unsigned	MaxB3Connection
	unsigned	MaxB3Blks
	unsigned	MaxSizeB3
    CODE:
	if(capi20_register(MaxB3Connection, MaxB3Blks, MaxSizeB3, &RETVAL) != CapiNoError)
	    XSRETURN_UNDEF;
    OUTPUT:
	RETVAL

unsigned
lib_CAPI20_RELEASE(Appl_Id)
	unsigned	Appl_Id
    CODE:
	if(CAPI20_RELEASE(Appl_Id) != CapiNoError)
	    XSRETURN_UNDEF;
	RETVAL = 1;
    OUTPUT:
	RETVAL

unsigned
ALERT_REQ(ApplId=0, Messagenumber=0, adr=0, BChannelinformation=es, Keypadfacility=es, Useruserdata=es, Facilitydataarray=es)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	_cstruct	BChannelinformation
	_cstruct	Keypadfacility
	_cstruct	Useruserdata
	_cstruct	Facilitydataarray
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, BChannelinformation, Keypadfacility, Useruserdata, Facilitydataarray

unsigned
CONNECT_ACTIVE_RESP(ApplId=0, Messagenumber=0, adr=0)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr

unsigned
CONNECT_B3_ACTIVE_RESP(ApplId=0, Messagenumber=0, adr=0)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr

unsigned
CONNECT_B3_REQ(ApplId=0, Messagenumber=0, adr=0, NCPI=es)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	_cstruct	NCPI
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, NCPI

unsigned
CONNECT_B3_RESP(ApplId=0, Messagenumber=0, adr=0, Reject=0, NCPI=es)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	_cword	Reject
	_cstruct	NCPI
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, Reject, NCPI

unsigned
CONNECT_B3_T90_ACTIVE_RESP(ApplId, Messagenumber, adr)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr

unsigned
CONNECT_REQ(ApplId=0, Messagenumber=0, adr=0, CIPValue=0, CalledPartyNumber=es, CallingPartyNumber=es, CalledPartySubaddress=es, CallingPartySubaddress=es, B1protocol=0, B2protocol=0, B3protocol=0, B1configuration=es, B2configuration=es, B3configuration=es, BC=es, LLC=es, HLC=es, BChannelinformation=es, Keypadfacility=es, Useruserdata=es, Facilitydataarray=es)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	_cword	CIPValue
	_cstruct	CalledPartyNumber
	_cstruct	CallingPartyNumber
	_cstruct	CalledPartySubaddress
	_cstruct	CallingPartySubaddress
	_cword	B1protocol
	_cword	B2protocol
	_cword	B3protocol
	_cstruct	B1configuration
	_cstruct	B2configuration
	_cstruct	B3configuration
	_cstruct	BC
	_cstruct	LLC
	_cstruct	HLC
	_cstruct	BChannelinformation
	_cstruct	Keypadfacility
	_cstruct	Useruserdata
	_cstruct	Facilitydataarray
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, CIPValue, CalledPartyNumber, CallingPartyNumber, CalledPartySubaddress, CallingPartySubaddress, B1protocol, B2protocol, B3protocol, B1configuration, B2configuration, B3configuration, BC, LLC, HLC, BChannelinformation, Keypadfacility, Useruserdata, Facilitydataarray

unsigned
CONNECT_RESP(ApplId=0, Messagenumber=0, adr=0, Reject=0, B1protocol=0, B2protocol=0, B3protocol=0, B1configuration=0, B2configuration=0, B3configuration=0, ConnectedNumber=es, ConnectedSubaddress=es, LLC=es, BChannelinformation=es, Keypadfacility=es, Useruserdata=es, Facilitydataarray=es)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	_cword	Reject
	_cword	B1protocol
	_cword	B2protocol
	_cword	B3protocol
	_cstruct	B1configuration
	_cstruct	B2configuration
	_cstruct	B3configuration
	_cstruct	ConnectedNumber
	_cstruct	ConnectedSubaddress
	_cstruct	LLC
	_cstruct	BChannelinformation
	_cstruct	Keypadfacility
	_cstruct	Useruserdata
	_cstruct	Facilitydataarray
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, Reject, B1protocol, B2protocol, B3protocol, B1configuration, B2configuration, B3configuration, ConnectedNumber, ConnectedSubaddress, LLC, BChannelinformation, Keypadfacility, Useruserdata, Facilitydataarray

unsigned
DATA_B3_REQ(ApplId=0, Messagenumber=0, adr=0, Data=es, DataLength=0, DataHandle=0, Flags=0)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	unsigned char *	Data
	_cword	DataLength
	_cword	DataHandle
	_cword	Flags
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, Data, DataLength, DataHandle, Flags

unsigned
DATA_B3_RESP(ApplId=0, Messagenumber=0, adr=0, DataHandle=0)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	_cword	DataHandle
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, DataHandle

unsigned
DISCONNECT_B3_REQ(ApplId=0, Messagenumber=0, adr=0, NCPI=es)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	_cstruct	NCPI
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, NCPI

unsigned
DISCONNECT_B3_RESP(ApplId=0, Messagenumber=0, adr=0)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr

unsigned
DISCONNECT_REQ(ApplId=0, Messagenumber=0, adr=0, BChannelinformation=es, Keypadfacility=es, Useruserdata=es, Facilitydataarray=es)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	_cstruct	BChannelinformation
	_cstruct	Keypadfacility
	_cstruct	Useruserdata
	_cstruct	Facilitydataarray
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, BChannelinformation, Keypadfacility, Useruserdata, Facilitydataarray

unsigned
DISCONNECT_RESP(ApplId=0, Messagenumber=0, adr=0)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr

unsigned
FACILITY_REQ(ApplId=0, Messagenumber=0, adr=0, FacilitySelector=0, FacilityRequestParameter=es)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	_cword	FacilitySelector
	_cstruct	FacilityRequestParameter
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, FacilitySelector, FacilityRequestParameter

unsigned
FACILITY_RESP(ApplId=0, Messagenumber=0, adr=0, FacilitySelector=0, FacilityResponseParameters=es)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	_cword	FacilitySelector
	_cstruct	FacilityResponseParameters
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, FacilitySelector, FacilityResponseParameters

unsigned
INFO_REQ(ApplId, Messagenumber, adr, CalledPartyNumber, BChannelinformation, Keypadfacility, Useruserdata, Facilitydataarray)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	_cstruct	CalledPartyNumber
	_cstruct	BChannelinformation
	_cstruct	Keypadfacility
	_cstruct	Useruserdata
	_cstruct	Facilitydataarray
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, CalledPartyNumber, BChannelinformation, Keypadfacility, Useruserdata, Facilitydataarray

unsigned
INFO_RESP(ApplId=0, Messagenumber=0, adr=0)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr

unsigned
LISTEN_REQ(ApplId=0, Messagenumber=0, adr=0, InfoMask=0, CIPmask=0, CIPmask2=0, CallingPartyNumber=es, CallingPartySubaddress=es)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	_cdword	InfoMask
	_cdword	CIPmask
	_cdword	CIPmask2
	_cstruct	CallingPartyNumber
	_cstruct	CallingPartySubaddress
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, InfoMask, CIPmask, CIPmask2, CallingPartyNumber, CallingPartySubaddress

unsigned
MANUFACTURER_REQ(ApplId, Messagenumber, adr, ManuID, Class, Function, ManuData)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	_cdword	ManuID
	_cdword	Class
	_cdword	Function
	_cstruct	ManuData
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, ManuID, Class, Function, ManuData

unsigned
MANUFACTURER_RESP(ApplId, Messagenumber, adr, ManuID, Class, Function, ManuData)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	_cdword	ManuID
	_cdword	Class
	_cdword	Function
	_cstruct	ManuData
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, ManuID, Class, Function, ManuData

unsigned
RESET_B3_REQ(ApplId, Messagenumber, adr, NCPI)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	_cstruct	NCPI
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, NCPI

unsigned
RESET_B3_RESP(ApplId, Messagenumber, adr)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr

unsigned
SELECT_B_PROTOCOL_REQ(ApplId, Messagenumber, adr, B1protocol, B2protocol, B3protocol, B1configuration, B2configuration, B3configuration)
	_cmsg	cmsg = {};
	_cword	ApplId
	_cword	Messagenumber
	_cdword	adr
	_cword	B1protocol
	_cword	B2protocol
	_cword	B3protocol
	_cstruct	B1configuration
	_cstruct	B2configuration
	_cstruct	B3configuration
    C_ARGS:
	&cmsg, ApplId, Messagenumber, adr, B1protocol, B2protocol, B3protocol, B1configuration, B2configuration, B3configuration

unsigned char *
capi20_get_manufacturer(Ctrl, Buf)
	unsigned	Ctrl
	unsigned char *	Buf

unsigned
capi20_get_message(ApplID, Buf)
	unsigned	ApplID
	unsigned char **	Buf

SV *
capi20_get_profile(Controller=0)
	unsigned	Controller
    PREINIT:
	unsigned char	Buf[64];
    CODE:
	ST(0) = sv_newmortal();
	if(capi20_get_profile(Controller, Buf) == CapiNoError)
		sv_setpvn(ST(0), Buf, 64);

unsigned char *
capi20_get_serial_number(Ctrl, Buf)
	unsigned	Ctrl
	unsigned char *	Buf

unsigned char *
capi20_get_version(Ctrl, Buf)
	unsigned	Ctrl
	unsigned char *	Buf

unsigned
capi20_isinstalled()

unsigned
capi20_put_message(ApplID, Msg)
	unsigned	ApplID
	unsigned char *	Msg

_cmsg
capi_get_cmsg(applid)
	unsigned	applid
    CODE:
        if(capi_get_cmsg(&RETVAL, applid) != CapiNoError)
            XSRETURN_UNDEF;
    OUTPUT:
	RETVAL

unsigned
capi_put_cmsg(cmsg)
	_cmsg *	cmsg

MODULE = Capi		PACKAGE = _cmsg		

_cmsg *
_to_ptr(THIS)
	_cmsg THIS = NO_INIT
    PROTOTYPE: $
    CODE:
	if (sv_derived_from(ST(0), "_cmsg")) {
	    STRLEN len;
	    char *s = SvPV((SV*)SvRV(ST(0)), len);
	    if (len != sizeof(THIS))
		croak("Size %d of packed data != expected %d",
			len, sizeof(THIS));
	    New(0, RETVAL, 1, _cmsg);
	    Copy(s, RETVAL, 1, _cmsg);
	}   
	else
	    croak("THIS is not of type _cmsg");
    OUTPUT:
	RETVAL

_cmsg
new(CLASS)
	char *CLASS = NO_INIT
    PROTOTYPE: $
    CODE:
	Zero((void*)&RETVAL, sizeof(RETVAL), char);
    OUTPUT:
	RETVAL

MODULE = Capi		PACKAGE = _cmsgPtr

void
DESTROY(THIS)
	_cmsg *	THIS
    CODE:
	Safefree(THIS);

_cword
ApplId(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->ApplId = __value;
	RETVAL = THIS->ApplId;
    OUTPUT:
	RETVAL

_cbyte
Command(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cbyte __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->Command = __value;
	RETVAL = THIS->Command;
    OUTPUT:
	RETVAL

_cbyte
Subcommand(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cbyte __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->Subcommand = __value;
	RETVAL = THIS->Subcommand;
    OUTPUT:
	RETVAL

_cword
Messagenumber(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->Messagenumber = __value;
	RETVAL = THIS->Messagenumber;
    OUTPUT:
	RETVAL

_cmstruct
AdditionalInfo(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cmstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->AdditionalInfo = __value;
	RETVAL = THIS->AdditionalInfo;
    OUTPUT:
	RETVAL

_cstruct
B1configuration(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->B1configuration = __value;
	RETVAL = THIS->B1configuration;
    OUTPUT:
	RETVAL

_cword
B1protocol(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->B1protocol = __value;
	RETVAL = THIS->B1protocol;
    OUTPUT:
	RETVAL

_cstruct
B2configuration(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->B2configuration = __value;
	RETVAL = THIS->B2configuration;
    OUTPUT:
	RETVAL

_cword
B2protocol(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->B2protocol = __value;
	RETVAL = THIS->B2protocol;
    OUTPUT:
	RETVAL

_cstruct
B3configuration(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->B3configuration = __value;
	RETVAL = THIS->B3configuration;
    OUTPUT:
	RETVAL

_cword
B3protocol(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->B3protocol = __value;
	RETVAL = THIS->B3protocol;
    OUTPUT:
	RETVAL

_cstruct
BC(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->BC = __value;
	RETVAL = THIS->BC;
    OUTPUT:
	RETVAL

_cstruct
BChannelinformation(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->BChannelinformation = __value;
	RETVAL = THIS->BChannelinformation;
    OUTPUT:
	RETVAL

_cmstruct
BProtocol(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cmstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->BProtocol = __value;
	RETVAL = THIS->BProtocol;
    OUTPUT:
	RETVAL

_cstruct
CalledPartyNumber(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->CalledPartyNumber = __value;
	RETVAL = THIS->CalledPartyNumber;
    OUTPUT:
	RETVAL

_cstruct
CalledPartySubaddress(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->CalledPartySubaddress = __value;
	RETVAL = THIS->CalledPartySubaddress;
    OUTPUT:
	RETVAL

_cstruct
CallingPartyNumber(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->CallingPartyNumber = __value;
	RETVAL = THIS->CallingPartyNumber;
    OUTPUT:
	RETVAL

_cstruct
CallingPartySubaddress(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->CallingPartySubaddress = __value;
	RETVAL = THIS->CallingPartySubaddress;
    OUTPUT:
	RETVAL

_cdword
CIPmask(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cdword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->CIPmask = __value;
	RETVAL = THIS->CIPmask;
    OUTPUT:
	RETVAL

_cdword
CIPmask2(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cdword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->CIPmask2 = __value;
	RETVAL = THIS->CIPmask2;
    OUTPUT:
	RETVAL

_cword
CIPValue(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->CIPValue = __value;
	RETVAL = THIS->CIPValue;
    OUTPUT:
	RETVAL

_cdword
Class(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cdword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->Class = __value;
	RETVAL = THIS->Class;
    OUTPUT:
	RETVAL

_cstruct
ConnectedNumber(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->ConnectedNumber = __value;
	RETVAL = THIS->ConnectedNumber;
    OUTPUT:
	RETVAL

_cstruct
ConnectedSubaddress(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->ConnectedSubaddress = __value;
	RETVAL = THIS->ConnectedSubaddress;
    OUTPUT:
	RETVAL

_cdword
Data32(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cdword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->Data32 = __value;
	RETVAL = THIS->Data32;
    OUTPUT:
	RETVAL

_cword
DataHandle(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->DataHandle = __value;
	RETVAL = THIS->DataHandle;
    OUTPUT:
	RETVAL

_cword
DataLength(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->DataLength = __value;
	RETVAL = THIS->DataLength;
    OUTPUT:
	RETVAL

_cstruct
FacilityConfirmationParameter(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->FacilityConfirmationParameter = __value;
	RETVAL = THIS->FacilityConfirmationParameter;
    OUTPUT:
	RETVAL

_cstruct
Facilitydataarray(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->Facilitydataarray = __value;
	RETVAL = THIS->Facilitydataarray;
    OUTPUT:
	RETVAL

_cstruct
FacilityIndicationParameter(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->FacilityIndicationParameter = __value;
	RETVAL = THIS->FacilityIndicationParameter;
    OUTPUT:
	RETVAL

_cstruct
FacilityRequestParameter(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->FacilityRequestParameter = __value;
	RETVAL = THIS->FacilityRequestParameter;
    OUTPUT:
	RETVAL

_cstruct
FacilityResponseParameters(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->FacilityResponseParameters = __value;
	RETVAL = THIS->FacilityResponseParameters;
    OUTPUT:
	RETVAL

_cword
FacilitySelector(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->FacilitySelector = __value;
	RETVAL = THIS->FacilitySelector;
    OUTPUT:
	RETVAL

_cword
Flags(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->Flags = __value;
	RETVAL = THIS->Flags;
    OUTPUT:
	RETVAL

_cdword
Function(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cdword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->Function = __value;
	RETVAL = THIS->Function;
    OUTPUT:
	RETVAL

_cstruct
HLC(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->HLC = __value;
	RETVAL = THIS->HLC;
    OUTPUT:
	RETVAL

_cword
Info(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->Info = __value;
	RETVAL = THIS->Info;
    OUTPUT:
	RETVAL

_cstruct
InfoElement(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->InfoElement = __value;
	RETVAL = THIS->InfoElement;
    OUTPUT:
	RETVAL

_cdword
InfoMask(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cdword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->InfoMask = __value;
	RETVAL = THIS->InfoMask;
    OUTPUT:
	RETVAL

_cword
InfoNumber(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->InfoNumber = __value;
	RETVAL = THIS->InfoNumber;
    OUTPUT:
	RETVAL

_cstruct
Keypadfacility(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->Keypadfacility = __value;
	RETVAL = THIS->Keypadfacility;
    OUTPUT:
	RETVAL

_cstruct
LLC(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->LLC = __value;
	RETVAL = THIS->LLC;
    OUTPUT:
	RETVAL

_cstruct
ManuData(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->ManuData = __value;
	RETVAL = THIS->ManuData;
    OUTPUT:
	RETVAL

_cdword
ManuID(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cdword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->ManuID = __value;
	RETVAL = THIS->ManuID;
    OUTPUT:
	RETVAL

_cstruct
NCPI(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->NCPI = __value;
	RETVAL = THIS->NCPI;
    OUTPUT:
	RETVAL

_cword
Reason(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->Reason = __value;
	RETVAL = THIS->Reason;
    OUTPUT:
	RETVAL

_cword
Reason_B3(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->Reason_B3 = __value;
	RETVAL = THIS->Reason_B3;
    OUTPUT:
	RETVAL

_cword
Reject(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->Reject = __value;
	RETVAL = THIS->Reject;
    OUTPUT:
	RETVAL

_cstruct
Useruserdata(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cstruct __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->Useruserdata = __value;
	RETVAL = THIS->Useruserdata;
    OUTPUT:
	RETVAL

SV *
Data(THIS, __value = NO_INIT)
	_cmsg * THIS
	unsigned char * __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->Data = __value;
	ST(0) = sv_newmortal();
	sv_setpvn(ST(0), THIS->Data, THIS->DataLength);

unsigned
l(THIS, __value = NO_INIT)
	_cmsg * THIS
	unsigned __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->l = __value;
	RETVAL = THIS->l;
    OUTPUT:
	RETVAL

unsigned
p(THIS, __value = NO_INIT)
	_cmsg * THIS
	unsigned __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->p = __value;
	RETVAL = THIS->p;
    OUTPUT:
	RETVAL

unsigned char *
par(THIS, __value = NO_INIT)
	_cmsg * THIS
	unsigned char * __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->par = __value;
	RETVAL = THIS->par;
    OUTPUT:
	RETVAL

_cbyte *
m(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cbyte * __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->m = __value;
	RETVAL = THIS->m;
    OUTPUT:
	RETVAL

_cdword
Controller(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cdword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->adr.adrController = __value;
	RETVAL = THIS->adr.adrController;
    OUTPUT:
	RETVAL

_cdword
PLCI(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cdword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->adr.adrPLCI = __value;
	RETVAL = THIS->adr.adrPLCI;
    OUTPUT:
	RETVAL

_cdword
NCCI(THIS, __value = NO_INIT)
	_cmsg * THIS
	_cdword __value
    PROTOTYPE: $;$
    CODE:
	if (items > 1)
	    THIS->adr.adrNCCI = __value;
	RETVAL = THIS->adr.adrNCCI;
    OUTPUT:
	RETVAL
