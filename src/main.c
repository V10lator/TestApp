#include "status.h"

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <coreinit/fastmutex.h>
#include <coreinit/memdefaultheap.h>
#include <coreinit/memory.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <coreinit/title.h>
#include <proc_ui/procui.h>
#include <sysapp/launch.h>

#include <whb/log.h>
#include <whb/log_udp.h>

char days[7][4] = {
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
};

char months[12][4] = {
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Nov",
	"Dez",
};

OSFastMutex debugMutex;

void debugInit()
{
	OSFastMutex_Init(&debugMutex, "NUSspli debug");
	OSFastMutex_Unlock(&debugMutex);
	WHBLogUdpInit();
}

void debugPrintf(const char *str, ...)
{
	va_list va;
	va_start(va, str);
	char newStr[2048];
	
	OSCalendarTime now;
    OSTicksToCalendarTime(OSGetTime(), &now);
	sprintf(newStr, "%s %02d %s %d %02d:%02d:%02d.%03d\t", days[now.tm_wday], now.tm_mday, months[now.tm_mon], now.tm_year, now.tm_hour, now.tm_min, now.tm_sec, now.tm_msec);

	size_t tss = strlen(newStr);
	
	vsnprintf(newStr + tss, 2048 - tss, str, va);
	va_end(va);
	
	while(!OSFastMutex_TryLock(&debugMutex))
		;
	WHBLogPrint(newStr);
	OSFastMutex_Unlock(&debugMutex);
}

int main()
{
	initStatus();
	
	debugInit();
	debugPrintf("main()");
	
	OSSleepTicks(OSSecondsToTicks(2));
	
	if(AppRunning())
	{
		debugPrintf("App still running, faking HOME button press...");
		homeButtonCallback(NULL);
	}
	
	debugPrintf("Sending SYS command...");
	if(isAroma() || isChannel())
		SYSLaunchMenu();
	else
		SYSRelaunchTitle(0, NULL);
	
	debugPrintf("Waiting for ProcUI....");
	while(AppRunning() != APP_STATE_STOPPED)
		;
	
	debugPrintf("Shutting down ProcUI...");
	ProcUIShutdown();
	debugPrintf("All done, returning!");
	return 0;
}
