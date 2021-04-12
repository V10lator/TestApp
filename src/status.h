#ifndef TEST_STATUS_H
#define TEST_STATUS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum
{
	APP_STATE_STOPPING = 0,
	APP_STATE_STOPPED,
	APP_STATE_RUNNING,
	APP_STATE_BACKGROUND,
	APP_STATE_RETURNING,
	
} APP_STATE;

extern volatile APP_STATE app;

void initStatus();
void enableShutdown();
void disableShutdown();
bool isAroma();
bool isChannel();
bool AppRunning();
uint32_t homeButtonCallback(void *dummy);

#ifdef __cplusplus
	}
#endif

#endif // ifndef TEST_STATUS_H
