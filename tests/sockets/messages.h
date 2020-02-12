#if !defined(__MESSAGES_H__)
#define __MESSAGES_H__

#include <inttypes.h>
#include "message.hpp"

DECLARE_MESSAGE(Hello, 0, (uint32_t, my_id))
DECLARE_MESSAGE(Put  , 1, (uint32_t, my_id) (string, data))
DECLARE_MESSAGE(Get  , 2, (uint32_t, my_id) (string, data))
DECLARE_MESSAGE(Bye  , 3, (uint32_t, my_id))

#endif // __MESSAGES_H__