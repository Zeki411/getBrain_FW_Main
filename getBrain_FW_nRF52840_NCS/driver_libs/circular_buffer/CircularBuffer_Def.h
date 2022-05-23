#ifndef CIRCULAR_BUFFER_DEF_H
#define CIRCULAR_BUFFER_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "../../getBrain_Common/Std_Types.h"

#define CIRCULAR_BUFFER_MAX_SLOTS                7000

#define CIRCULAR_BUFFER_MAX_SLOT_DATA_LENGTH     30
	
typedef enum
{
	CIRCULAR_BUFFER_STATE_IDLE = 0x00, \
	CIRCULAR_BUFFER_STATE_FULL_BUFFER, \
	CIRCULAR_BUFFER_STATE_EMPTY_BUFFER, \
}CircularBuffer_Stt;

typedef struct 
{
	CircularBuffer_Stt  buffer_status;
	uint32_t            write_index;
	uint32_t            read_index;
	uint32_t            buffer_length;
	const uint32_t      max_slots;
	uint8_t             buffer[CIRCULAR_BUFFER_MAX_SLOTS][CIRCULAR_BUFFER_MAX_SLOT_DATA_LENGTH];
}CircularBuffer_t;

#ifdef __cplusplus
}
#endif

#endif /* CIRCULARBUFFER_DEF_H */
