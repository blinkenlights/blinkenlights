/*
 Oolong Engine for the iPhone / iPod touch
 Copyright (c) 2007-2008 Wolfgang Engel  http://code.google.com/p/oolongengine/
 
 This software is provided 'as-is', without any express or implied warranty.
 In no event will the authors be held liable for any damages arising from the use of this software.
 Permission is granted to anyone to use this software for any purpose, 
 including commercial applications, and to alter it and redistribute it freely, 
 subject to the following restrictions:
 
 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.
*/
#include "Timer.h"
#include <sys/time.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <Mathematics.h>


/*
static CFTimeInterval	startTime = 0;
CFTimeInterval			TimeInterval;

// Absolute time is measured in seconds relative to the absolute reference date of Jan 1 2001 00:00:00 GMT. 
// A positive value represents a date after the reference date, a negative value represents a date before it. 
TimeInterval = CFAbsoluteTimeGetCurrent();
if(startTime == 0)
startTime = TimeInterval;

frames++;
if (TimeInterval - startTime) 
{
	m_fFPS = ((float)frames/(TimeInterval - startTime));
}
*/
// 
// GetTime
// Time in milliseconds since 1970
// this is probably not very stable .. check out CFAbsoluteTimeGetCurrent() instead
//
unsigned long GetTimeInMsSince1970()
{
	timeval tv;
	// The time is expressed in seconds and microseconds since midnight (0 hour), January 1, 1970.
	gettimeofday(&tv,NULL);
	// to receive milliseconds we transform the seconds to milliseconds and the microseconds to milliseconds
	// and then add them
	return (unsigned long)((tv.tv_sec*1000) + (tv.tv_usec/1000.0));
}

//
// This is a replacement for QueryPerformanceFrequency / QueryPerformanceCounter
// returns nanoseconds since system start
//
unsigned long GetTimeInNsSinceCPUStart()
{
	double time;
	
	time = mach_absolute_time();
	
	// this is the timebase info
    static mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    double nano = ( (double) info.numer) / ((double) info.denom);
	
	return nano * time / 1000000000.0;
}

//
// returns Ticks since system start
//
unsigned long GetTimeInTicksSinceCPUStart()
{
		// return value is nanoseconds
		//result = gethrtime();
		//result = _rdtsc();
		INT64BIT time;
		// This function returns its result in terms of the Mach absolute time unit. 
		// This unit is CPU dependent, so you can't just multiply it by a constant to get a real world value. 
		time = (INT64BIT) mach_absolute_time();
		
		return time;
}


