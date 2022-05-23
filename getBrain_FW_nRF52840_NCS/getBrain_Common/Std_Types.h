#ifndef __STD_TYPES_H__
#define __STD_TYPES_H__

#ifdef __cplusplus
extern "C"{
#endif

#define E_OK			0x00
#define E_NOT_OK		0x01

#define ST_OFF          0x00
#define ST_ON           0x01

typedef uint8_t Std_ReturnType;
	
typedef uint8_t Std_SwitchType;

#define BIT_CLEAR8(value, mask)     ((value) &= (~(mask)))
#define BIT_CLEAR16(value, mask)    ((value) &= (~(mask)))
#define BIT_CLEAR32(value, mask)    ((value) &= (~(mask)))


#define BIT_GET8(value, mask)       (((uint8) value) & (mask))
#define BIT_GET16(value, mask)      (((uint16) value) & (mask))
#define BIT_GET32(value, mask)      (((uint32) value) & (mask))


#define BIT_SET8(value, mask)       ((value) |= (mask))
#define BIT_SET16(value, mask)      ((value) |= (mask))
#define BIT_SET32(value, mask)      ((value) |= (mask))

#define BIT_IS_SET8(value, mask)       ((value & mask) == mask)
#define BIT_IS_SET16(value, mask)      ((value & mask) == mask)
#define BIT_IS_SET32(value, mask)      ((value & mask) == mask)

#ifdef __cplusplus
}
#endif


#endif /* __STD_TYPES_H__ */
