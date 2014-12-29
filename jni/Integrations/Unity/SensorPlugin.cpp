/************************************************************************************

Filename    :   SensorPlugin.cpp
Content     :   DLL Plugin; Expose sensor functionality to applications
Created     :   January 2, 2013
Authors     :   Peter Giokaris

Copyright   :   Copyright 2014 Oculus, Inc. All Rights reserved.


*************************************************************************************/
#include <jni.h>
#include <unistd.h>						// usleep, etc
#include <sys/syscall.h>

#include "Log.h"

//---------------------------
// STUBS for Android
//---------------------------

#define OCULUS_EXPORT

extern "C"
{
//---------------------------
// Global functions

//---------------------------
OCULUS_EXPORT bool OVR_Initialize()
{
	return true;
}

//---------------------------
OCULUS_EXPORT bool OVR_Destroy()
{
	return true;
}

//---------------------------
struct MessageList
{
	char isHMDSensorAttached;
	char isHMDAttached;
	char isLatencyTesterAttached;

	MessageList() : isHMDSensorAttached(0),
		isHMDAttached(0),
		isLatencyTesterAttached(0) {}
};
OCULUS_EXPORT void OVR_Update(MessageList &messageList)
{
}


//---------------------------
// Sensor functions
//---------------------------

//---------------------------
OCULUS_EXPORT bool OVR_IsSensorPresent()
{
	return false;
}

//---------------------------
OCULUS_EXPORT bool OVR_GetMagnetometer(float &x, float &y, float &z)
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	return false;
}


//---------------------------
// Camera Functions

//---------------------------
OCULUS_EXPORT void OVR_SetHeadModel(float x, float y, float z)
{
}

//---------------------------
OCULUS_EXPORT void OVR_SetVisionEnabled(bool on)
{
}

//---------------------------
OCULUS_EXPORT void OVR_SetLowPersistenceMode(bool on)
{
}

//---------------------------
// Latency Functions

//---------------------------
OCULUS_EXPORT void OVR_ProcessLatencyInputs()
{
	//s_displayLatencyColor = ovrHmd_ProcessLatencyTest(OvrHmd, s_rgbLatencyColorOut);
}

//---------------------------
// MAG FUNCTIONS
//---------------------------

//---------------------------
OCULUS_EXPORT bool OVR_IsMagCalibrated()
{
	return false;
}

//---------------------------
OCULUS_EXPORT bool OVR_EnableMagYawCorrection(bool enable)
{
	return false;
}

//---------------------------
OCULUS_EXPORT bool OVR_IsYawCorrectionEnabled()
{
	return false;
}

}	// extern "C"
