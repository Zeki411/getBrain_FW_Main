#ifndef __CIRCULAR_BUFFER_H__
#define __CIRCULAR_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "CircularBuffer_Def.h"

Std_ReturnType CircularBuffer_Push(CircularBuffer_t *cirbuf, uint8_t * data, uint16_t len);

Std_ReturnType CircularBuffer_Pop(CircularBuffer_t *cirbuf, uint8_t * data, uint16_t len);


#ifdef __cplusplus
}
#endif

#endif /* CIRCULAR_BUFFER_H */
