#include "status.h"

#include <coreinit/core.h>
#include <coreinit/dynload.h>
#include <coreinit/energysaver.h>
#include <coreinit/foreground.h>
#include <coreinit/systeminfo.h>
#include <coreinit/title.h>
#include <proc_ui/procui.h>

#include <stdbool.h>
#include <stdint.h>

volatile APP_STATE app = APP_STATE_RUNNING;
volatile bool shutdownEnabled = true;
volatile bool shutdownRequested = false;
bool channel;
bool aroma;

void enableShutdown()
{
	IMEnableAPD();
	shutdownEnabled = true;
}
void disableShutdown()
{
	IMDisableAPD();
	shutdownEnabled = false;
}

bool isAroma()
{
	return aroma;
}

bool isChannel()
{
	return channel;
}

uint32_t homeButtonCallback(void *dummy)
{
	if(shutdownEnabled)
		shutdownRequested = true;
	
	return 0;
}

void initStatus()
{
	ProcUIInit(&OSSavesDone_ReadyToRelease);
	ProcUIRegisterCallback(PROCUI_CALLBACK_HOME_BUTTON_DENIED, &homeButtonCallback, NULL, 100);
	OSEnableHomeButtonMenu(false);
	
	OSDynLoad_Module mod;
	aroma = OSDynLoad_Acquire("homebrew_kernel", &mod) == OS_DYNLOAD_OK;
	if(aroma)
		OSDynLoad_Release(mod);
	channel = OSGetTitleID() == 0x000500004E555373;
	
}

bool AppRunning()
{
	if(OSIsMainCore())
	{
		if(app != APP_STATE_STOPPED)
		{
			if(shutdownRequested)
				app = APP_STATE_STOPPING;
			
			switch(ProcUIProcessMessages(true))
			{
				case PROCUI_STATUS_EXITING:
					// Being closed, deinit, free, and prepare to exit
					app = APP_STATE_STOPPED;
					break;
				case PROCUI_STATUS_RELEASE_FOREGROUND:
					if(shutdownRequested)
						break;
					// Free up MEM1 to next foreground app, deinit screen, etc.
					app = APP_STATE_BACKGROUND;
					ProcUIDrawDoneRelease();
					break;
				case PROCUI_STATUS_IN_FOREGROUND:
					if(shutdownRequested)
						break;
					// Executed while app is in foreground
					app = app == APP_STATE_BACKGROUND ? APP_STATE_RETURNING : APP_STATE_RUNNING;
					break;
				case PROCUI_STATUS_IN_BACKGROUND:
					if(!shutdownRequested)
						app = APP_STATE_BACKGROUND;
					break;
			}
		}
	}
	
	return app;
}