#ifndef CAN_DEVICE_H
#define CAN_DEVICE_H

// C Standard Library
#include <stdint.h>

typedef struct
{
	uint32_t id;
	uint8_t data [8];
	uint8_t dlc;
} canFrame_t;

typedef int canTransmit_t (void* device, canFrame_t* frame);

typedef int canReceive_t (void* device, canFrame_t* frame);

typedef int canFlushRx_t (void* device);

typedef int canSetTimeout_t (void* device, unsigned long timeoutMs);

typedef struct
{
	canTransmit_t*		transmit;
	canReceive_t*		receive;
	canFlushRx_t*		flushRx;
	canSetTimeout_t*	setTimeout;
} canDeviceVmt_t;

typedef struct
{
	canDeviceVmt_t vmt;
} canDevice_t;

#define canTransmit(device, frame)						\
	(device)->vmt.transmit (device, frame)

#define canReceive(device, frame)						\
	(device)->vmt.receive (device, frame)

#define canFlushRx(device)								\
	(device)->vmt.flushRx (device)

#define canSetTimeout(device, timeoutMs)				\
	(device)->vmt.setTimeout (device, timeoutMs)

canDevice_t* canInit (const char* name);

#endif // CAN_DEVICE_H