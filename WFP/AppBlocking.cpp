#include "ntddk.h"
#include <guiddef.h>
#include <fwpmk.h>
#include <fwpsk.h>
#include <fwpmu.h>
#define INITGUID

// {7C86B394-5527-45A2-93D9-44EB85E5EC95}
DEFINE_GUID(WFP_ESTABLISHED_CALLOUT_V4_GUID ,
	0x7c86b394, 0x5527, 0x45a2, 0x93, 0xd9, 0x44, 0xeb, 0x85, 0xe5, 0xec, 0x95);
// {45D8C1AE-112F-4150-9265-78F2936864B8}
DEFINE_GUID(WFP_SUB_LAYER_GUID ,
	0x45d8c1ae, 0x112f, 0x4150, 0x92, 0x65, 0x78, 0xf2, 0x93, 0x68, 0x64, 0xb8);

PDEVICE_OBJECT DeviceObject = NULL;
HANDLE EngineHandle = NULL;
UINT32 RegCalloutId = 0, AddCalloutId;
UINT64 filterId = 0;

VOID UnInitWfp() {
	if (EngineHandle != NULL) {

		if (filterId != 0) {
			FwpmFilterDeleteById(EngineHandle, filterId);
			FwpmSubLayerDeleteByKey(EngineHandle, &WFP_SUB_LAYER_GUID);
		}

		if (AddCalloutId != 0) {
			FwpmCalloutDeleteById(EngineHandle, AddCalloutId);
		}

		if (RegCalloutId != 0) {
			FwpsCalloutUnregisterById(RegCalloutId);
		}

		FwpmEngineClose(EngineHandle);
	}
}

VOID Unload(PDRIVER_OBJECT DriverObject) {
	UnInitWfp();
	IoDeleteDevice(DeviceObject);
	KdPrint(("unload\r\n"));
}

NTSTATUS NotifyCallback(FWPS_CALLOUT_NOTIFY_TYPE type, const GUID* filterkey, const FWPS_FILTER* filter) {
	return STATUS_SUCCESS;
}

VOID FlowDeleteCallback(UINT16 layerid, UINT32 calloutid, UINT64 flowcontext) {

}

/*here is where we can see which data is transfered from the driver and to the driver and take any actions needed*/

VOID FilterCallback(const FWPS_INCOMING_VALUE0* Values, const FWPS_INCOMING_METADATA_VALUES0 MetaData, const PVOID layerdata, const void* context, const FWPS_FILTER* filter, UINT64 flowcontext, FWPS_CLASSIFY_OUT* classifyout) {

	FWPS_STREAM_CALLOUT_IO_PACKET* packet;
	FWPS_STREAM_DATA* streamdata;
	UCHAR string[201] = { 0 };
	ULONG lenght = 0;
	SIZE_T bytes;
	UCHAR string searchApp;
	UCHAR targetApp = "Spotify.exe"; //Change this to the name of the Applications you want to block.

	/*KdPrint(("data is here\r\n")); defult print just to example*/

	packet = (FWPS_STREAM_CALLOUT_IO_PACKET*)layerdata;
	searchApp = Values->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_APP_ADDRESS].value.uint32;

	RtlZeroMemory(classifyout, sizeof(FWPS_CLASSIFY_OUT));

	/*if ((streamdata->flags & FWPS_STREAM_FLAG_SEND) || (streamdata->flags & FWPS_STREAM_FLAG_SEND_EXPEDITED) ||
		(streamdata->flags & FWPS_STREAM_FLAG_RECEIVE_EXPEDITED)) {
		goto end;
	}*/
	if (!(classifyout->rights & FWPS_RIGHT_ACTION_WRITE)) {
		goto end;
	}

	if (targetApp == searchApp) {
		KdPrint(("block \r\n"));
		classifyout->actionType = FWP_ACTION_BLOCK;
		classifyout->rights &= ~FWPS_RIGHT_ACTION_WRITE;

		return;
	}
	else {
		classifyout->actionType = FWP_ACTION_PERMIT;
	}
end:
	if (filter->flags & FWPS_FILTER_FLAG_CLEAR_ACTION_RIGHT) {
		classifyout->rights &= ~FWPS_RIGHT_ACTION_WRITE;
	}
}

NTSTATUS WfpOpenEngine() {
	FwpmEngineOpen(NULL, RPC_C_AUTHN_WINNT, NULL, NULL, &EngineHandle);
}

NTSTATUS WfpRegisterCallout() {
	FWPS_CALLOUT Callout = { 0 };

	Callout.calloutKey = WFP_ESTABLISHED_CALLOUT_V4_GUID;
	Callout.flags = 0;
	Callout.classifyFn = (FWPS_CALLOUT_CLASSIFY_FN2)FilterCallback;
	Callout.notifyFn = (FWPS_CALLOUT_NOTIFY_FN2)NotifyCallback;
	Callout.flowDeleteFn = (FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN0)FlowDeleteCallback;

	return FwpsCalloutRegister(DeviceObject, &Callout, &RegCalloutId);
}

NTSTATUS WfpAddCallout() {
	FWPM_CALLOUT callout = { 0 };

	callout.flags = 0;
	callout.displayData.name = L"EstablishedCalloutName";
	callout.displayData.description = L"EstablishedCalloutName";
	callout.calloutKey = WFP_ESTABLISHED_CALLOUT_V4_GUID; // change Id to Key
	callout.applicableLayer = FWPM_LAYER_STREAM_V4;

	return FwpmCalloutAdd(EngineHandle, &callout, NULL, &AddCalloutId);
}

NTSTATUS WfpAddSublayer() {
	FWPM_SUBLAYER sublayer = { 0 };

	sublayer.displayData.name = L"Establishedsublayername";
	sublayer.displayData.description = L"Establishedsublayername";
	sublayer.subLayerKey = WFP_SUB_LAYER_GUID;
	sublayer.weight = 65500;

	return FwpmSubLayerAdd(EngineHandle, &sublayer, NULL);
}

NTSTATUS InitializeWfp() {
	if (!NT_SUCCESS(WfpOpenEngine())) {
		goto end;
	}

	if (!NT_SUCCESS(WfpRegisterCallout())) {
		goto end;
	}

	if (!NT_SUCCESS(WfpAddCallout())) {
		goto end;
	}

	if (!NT_SUCCESS(WfpAddSublayer())) {
		goto end;
	}

	if (!NT_SUCCESS(WfpAddFilter())) {
		goto end;
	}

	return STATUS_SUCCESS;

end:
	UnInitWfp();
	return STATUS_UNSUCCESSFUL;

}

NTSTATUS WfpAddFilter() {
	FWPM_FILTER filter = { 0 };
	FWPM_FILTER_CONDITION condition[1] = { 0 };

	filter.displayData.name = L"filterCalloutName";
	filter.displayData.description = L"filterCalloutName";
	filter.layerKey = FWPM_LAYER_STREAM_V4;
	filter.subLayerKey = WFP_SUB_LAYER_GUID;
	filter.weight.type = FWP_EMPTY;
	filter.numFilterConditions = 1;
	filter.filterCondition = condition;
	filter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
	filter.action.calloutKey = WFP_ESTABLISHED_CALLOUT_V4_GUID;

	condition[0].fieldKey = FWPM_CONDITION_IP_LOCAL_PORT;
	condition[0].matchType = FWP_MATCH_LESS_OR_EQUAL;
	condition[0].conditionValue.type = FWP_UINT16;
	condition[0].conditionValue.uint16 = 65000;

	return FwpmFilterAdd(EngineHandle, &filter, NULL, &filterId);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriveObject, PUNICODE_STRING RegistryPath) {
	NTSTATUS status;

	DriverObject->DriverUnload = Unload;

	status = IoCreateDevice(DriverObject, 0, NULL, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);

	if (!NT_SUCCESS(status)) {
		return status;
	}

	status = InitializeWfp();

	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(DeviceObject);
	}

	return status;
}
