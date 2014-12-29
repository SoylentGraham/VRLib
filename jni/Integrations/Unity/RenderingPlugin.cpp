/************************************************************************************

Filename    :   RenderingPlugin.cpp
Content     :   DLL Plugin; Expose rendering functionality to applications
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

#define EXPORT_API

extern "C" void EXPORT_API EnableTimeWarp(bool isEnabled)
{
}

extern "C" void EXPORT_API GetFloatv(int id, float buffer[])
{
	memset(buffer, 0, 16 * sizeof(float));
}
