#include <string.h>
#include "CircularBuffer.h"

Std_ReturnType CircularBuffer_Push(CircularBuffer_t *cirbuf, uint8_t * data, uint16_t len)
{
	if(cirbuf->buffer_length == cirbuf->max_slots)
	{
		cirbuf->buffer_status = CIRCULAR_BUFFER_STATE_FULL_BUFFER;
		
		return E_NOT_OK;
	}
	
	//memcpy(cirbuf->buffer[cirbuf->write_index],data,sizeof((unsigned char *)data));
	for(uint16_t dtcnt = 0; dtcnt < len; dtcnt++)
	{
		cirbuf->buffer[cirbuf->write_index][dtcnt] = data[dtcnt];
	}
	
	cirbuf->write_index++;
	
	if(cirbuf->write_index == cirbuf->max_slots)
	{
		cirbuf->write_index = 0;
	}
	
	cirbuf->buffer_length++;
	
	cirbuf->buffer_status = CIRCULAR_BUFFER_STATE_IDLE;
	
	return E_OK;
}

Std_ReturnType CircularBuffer_Pop(CircularBuffer_t *cirbuf, uint8_t * data, uint16_t len)
{
	if(cirbuf->buffer_length == 0)
	{
		cirbuf->buffer_status = CIRCULAR_BUFFER_STATE_EMPTY_BUFFER;
		return E_NOT_OK;
	}
	
	// memcpy(data,cirbuf->buffer[cirbuf->read_index],sizeof(cirbuf->buffer[cirbuf->read_index]));
	for(uint16_t dtcnt = 0; dtcnt < len; dtcnt++)
	{
		data[dtcnt] = cirbuf->buffer[cirbuf->read_index][dtcnt];
	}
	
	memset(cirbuf->buffer[cirbuf->read_index],0,CIRCULAR_BUFFER_MAX_SLOT_DATA_LENGTH);
	
	cirbuf->read_index++;
	
	if(cirbuf->read_index == cirbuf->max_slots)
	{
		cirbuf->read_index = 0;
	}
	
	cirbuf->buffer_length--;
	
	cirbuf->buffer_status = CIRCULAR_BUFFER_STATE_IDLE;
	
	return E_OK;
}
