#if !defined(__MESSAGES_H__)
#define __MESSAGES_H__

#include <inttypes.h>
#include "message.hpp"

extern Message::Dump dump_mode;

#define LOG(message_) printf("%s\n", message_(dump_mode).c_str())

DECLARE_MESSAGE(Hello, Message::UID(0), (uint32_t, my_id))
DECLARE_MESSAGE(Put  , Message::UID(1), (uint32_t, my_id) (string, data))
DECLARE_MESSAGE(Get  , Message::UID(2), (uint32_t, my_id) (string, data))
DECLARE_MESSAGE(Bye  , Message::UID(3), (uint32_t, my_id))

#if defined(MINIMAL)

DECLARE_MESSAGE(NumericNoParamMessage1, 42)
DECLARE_MESSAGE(NumericNoParamMessage2, 42)

extern NumericNoParamMessage1 m_NumericNoParamMessage1;
extern NumericNoParamMessage2 m_NumericNoParamMessage2;

#else // MINIMAL

enum MyEnum {
    enOne,
    enTwo,
    enThree,
    invalidMax = 2147483647
};

typedef bitset<256> MegaFlagsT;
typedef bitset<4> FlagsT;

typedef map<uint8_t, string> MyMap;

DECLARE_MESSAGE(MyStruct, "MY_STRUCT",
    //
    // mandatory fields
    //
    (uint32_t, anEnum  , static_cast<uint32_t>(enTwo))
    (uint8_t , aChar   , 4              )
    (float   , aFloat  , 3.14f          ),
    //
    // optional fields
    //
    (bool    , optional, true           )
)

#if 0
DECLARE_MESSAGE(AnotherStruct, "ANOTHER_STRUCT",
    (int8_t  , bnil, 0        )
    (int8_t  , bone, 1        )
    (int8_t  , bmin, SCHAR_MIN)
    (int8_t  , bmax, SCHAR_MAX)
    (int16_t , snil, 0        )
    (int16_t , sone, 1        )
    (int16_t , smin, SHRT_MIN )
    (int16_t , smax, SHRT_MAX )
    (int32_t , inil, 0        )
    (int32_t , ione, 1        )
    (int32_t , imin, INT_MIN  )
    (int32_t , imax, INT_MAX  )
    (int64_t , lnil, 0        )
    (int64_t , lone, 1        )
    (int64_t , lmin, LLONG_MIN)
    (int64_t , lmax, LLONG_MAX)
    (double  , bar , 3.1415927)
    (MyStruct, dat            )
)
#else
DECLARE_MESSAGE(AnotherStruct, "ANOTHER_STRUCT",
    (int8_t  , bnil, 0)
    (int8_t  , bone, 0)
    (int8_t  , bmin, 0)
    (int8_t  , bmax, 0)
    (int16_t , snil, 0)
    (int16_t , sone, 0)
    (int16_t , smin, 0)
    (int16_t , smax, 0)
    (int32_t , inil, 0)
    (int32_t , ione, 0)
    (int32_t , imin, 0)
    (int32_t , imax, 0)
    (int64_t , lnil, 0)
    (int64_t , lone, 0)
    (int64_t , lmin, 0)
    (int64_t , lmax, 0)
    (double  , bar , 0)
    (MyStruct, dat    )
    (MyMap   , aMap   )
)
#endif

DECLARE_MESSAGE(ComplexStruct, "COMPLEX_STRUCT",
    (AnotherStruct , root)
    (MegaFlagsT    , bits)
    (vector<bool>  , var_bits)
    (vector<FlagsT>, iperbits)
)

DECLARE_MESSAGE(NoParamMessage, "NO_PARAMS")

DECLARE_MESSAGE(NumericNoParamMessage, Message::UID(42))

DECLARE_MESSAGE(NumericMessage, Message::UID(42),
     (bool              , valid, false)
     /*(long double       , bignum, 1.23456789123456789)*/
     (string            , text , "1Po'DiMaiuscoleMinuscole&Numeri")
     (vector<int8_t>    , bytes)
     (vector<double>    , levels)
     (optional<string>  , opt_param)
     (optional<MyStruct>, opt_struct)
     (vector<float>     , numbers)
     (vector<string>    , names)
     (vector<MyStruct>  , structs)
)

DECLARE_MESSAGE(NumericExtraParameters, Message::UID(43),
     ,
     (bool, extra, true)
)

#endif // MINIMAL

#endif // __MESSAGES_H__