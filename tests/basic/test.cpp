/******************************************************************************
//
// This build command generate a test executable which writes and read messages
// to and from a file:

g++ -std=c++98 \
    -DOUTPUT_ON_FILE \
    -O3 \
    test_message.cpp \
    -o test_message_on_file

//
// Usage to write messages on the outpud.dat file:
//
//    ./test_message_on_file w
//
// Usage to read messages from the outpud.dat file:
//
//    ./test_message r
//
******************************************************************************/

/******************************************************************************
//
// This build command generate a test executable which writes messages on a
// ostringstream and read messases from a istringstream initialized by the
// previous ostringstream pbject:

g++ -std=c++98 \
    -DSEPARATE_IN_AND_OUT_STRING_STREAMS \
    -O3 \
    test_message.cpp \
    -o test_message_on_separate_strstream

//
// Usage:
//
//    ./test_message_on_separate_strstream
//
******************************************************************************/

/******************************************************************************
//
// This build command generate a test executable which writes and reads
// messages on the same stringstream for both output and input:

g++ -std=c++98 \
    -O3 \
    test_message.cpp \
    -o test_message_on_iostream

//
// Usage:
//
//    ./test_message_on_iostream
//
******************************************************************************/

/******************************************************************************
//
// Test all together:

./test_message_on_file w             >  f.txt
./test_message_on_file r             >> f.txt
./test_message_on_separate_strstream >  s.txt
./test_message_on_iostream           >  i.txt

diff3 f.txt i.txt s.txt

// The three files (f.txt, s.txt and i.txt) shall be equals
//
******************************************************************************/
#include <stdio.h>
#include <fstream>
#include "message.hpp"
#include <inttypes.h>
#include <limits.h>

#define LOG(message_,type_) printf("%s\n\n", message_.json().c_str())


#if defined(MINIMAL)
DECLARE_MESSAGE(NumericNoParamMessage1, 42)
DECLARE_MESSAGE(NumericNoParamMessage2, 42)

NumericNoParamMessage1 m_NumericNoParamMessage1;
NumericNoParamMessage2 m_NumericNoParamMessage2;

int main(int argc, char *argv[])
{
    LOG(m_NumericNoParamMessage1, NumericNoParamMessage1);
    LOG(m_NumericNoParamMessage2, NumericNoParamMessage2);
    return 0;
}
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
    (vector<FlagsT>, iperbits)
)

DECLARE_MESSAGE(NoParamMessage, "NO_PARAMS")

DECLARE_MESSAGE(NumericNoParamMessage, Message::UID(42))

DECLARE_MESSAGE(NumericMessage, Message::UID(42),
     (bool              , valid, false)
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

MyStruct               m_MyStruct              ;
AnotherStruct          m_AnotherStruct         ;
NoParamMessage         m_NoParamMessage        ;
NumericNoParamMessage  m_NumericNoParamMessage ;
NumericMessage         m_NumericMessage        ;
NumericExtraParameters m_NumericExtraParameters;
ComplexStruct          m_ComplexStruct         ;

bool handle_message(Message &m)
{

    bool handled = true;

    if ((m == m_MyStruct              ) ||
        (m == m_AnotherStruct         ) ||
        (m == m_NoParamMessage        ) ||
        (m == m_NumericNoParamMessage ) ||
        (m == m_NumericMessage        ) ||
//        (m == m_NumericExtraParameters) ||
        (m == m_ComplexStruct         ))
    {
        printf("Known message\n");
    }

    if (m == m_AnotherStruct)
    {
        LOG(m_AnotherStruct, AnotherStruct);
    }

#if 0
    if ((m == m_AnotherStruct) || (m == m_NumericNoParamMessage))
    {
        printf("Skipping\n");

        LOG(m, Message);

        m.skip();

        printf("Skipped\n");

        return;
    }
#endif

    if (m >> m_MyStruct)
    {
        LOG(m_MyStruct, MyStruct);
    }
    else
    if (m >> m_AnotherStruct)
    {
        LOG(m_AnotherStruct, AnotherStruct);

        if (m_AnotherStruct)
        {
            if (m_AnotherStruct & AnotherStruct::_bnil)
            {
                printf("bnil changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_bone)
            {
                printf("bone changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_bmin)
            {
                printf("bmin changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_bmax)
            {
                printf("bmax changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_snil)
            {
                printf("snil changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_sone)
            {
                printf("sone changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_smin)
            {
                printf("smin changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_smax)
            {
                printf("smax changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_inil)
            {
                printf("inil changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_ione)
            {
                printf("ione changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_imin)
            {
                printf("imin changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_imax)
            {
                printf("imax changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_lnil)
            {
                printf("lnil changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_lone)
            {
                printf("lone changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_lmin)
            {
                printf("lmin changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_lmax)
            {
                printf("lmax changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_bar )
            {
                printf("bar  changed\n");
            }
            if (m_AnotherStruct & AnotherStruct::_dat )
            {
                printf("dat  changed\n");

                if (m_AnotherStruct.dat)
                {
                    if (m_AnotherStruct.dat & MyStruct::_anEnum)
                    {
                        printf("dat.anEnum changed\n");

                        m_AnotherStruct.dat -= MyStruct::_anEnum;
                    }
                    if (m_AnotherStruct.dat & MyStruct::_aChar)
                    {
                        printf("dat.aChar changed\n");

                        m_AnotherStruct.dat -= MyStruct::_aChar;
                    }
                    if (m_AnotherStruct.dat & MyStruct::_aFloat)
                    {
                        printf("dat.aFloat changed\n");

                        m_AnotherStruct.dat -= MyStruct::_aFloat;
                    }
                    if (m_AnotherStruct.dat & MyStruct::_optional)
                    {
                        printf("dat.optional changed\n");

                        m_AnotherStruct.dat -= MyStruct::_optional;
                    }

                    if (m_AnotherStruct.dat & MyStruct::_anEnum)
                    {
                        printf("dat.anEnum still changed\n");
                    }
                    if (m_AnotherStruct.dat & MyStruct::_aChar)
                    {
                        printf("dat.aChar still changed\n");
                    }
                    if (m_AnotherStruct.dat & MyStruct::_aFloat)
                    {
                        printf("dat.aFloat still changed\n");
                    }
                    if (m_AnotherStruct.dat & MyStruct::_optional)
                    {
                        printf("dat.optional still changed\n");
                    }
                }
            }
            if (m_AnotherStruct & AnotherStruct::_aMap)
            {
                printf("aMap changed\n");
            }

            printf("\n");

            ~m_AnotherStruct;
        }
    }
    else
    if (m >> m_NoParamMessage)
    {
        LOG(m_NoParamMessage, NoParamMessage);
    }
    else
    if (m >> m_NumericNoParamMessage)
    {
        LOG(m_NumericNoParamMessage, NumericNoParamMessage);
    }
    else
    if (m >> m_NumericMessage)
    {
        LOG(m_NumericMessage, NumericMessage);
    }
    else
    if (m >> m_ComplexStruct)
    {
        LOG(m_ComplexStruct, ComplexStruct);
    }
/*
    else
    if (m_NumericExtraParameters << m)
    {
        LOG(m_NumericExtraParameters, NumericExtraParameters);
    }
*/
    else
    {
        printf("Unknown message\n");
        LOG(m, Message);

        handled = false;
    }

    return handled;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

#if defined(OUTPUT_ON_FILE)
    bool do_read=true;
    bool do_write=true;
    bool from_file=false;

    if (argc>0)
    {
        do_read=false;
        do_write=false;

        if (argv[0][0] == 'r')
        {
            do_read = true;
        }

        if (argv[0][0] == 'w')
        {
            do_write = true;
        }

        if (argc>1)
        {
            if (argv[1][0] == '-')
            {
                do_read = true;
            }

            if (argv[1][0] == 'r')
            {
                do_read = true;
                from_file = true;
            }

            if (argv[1][0] == 'w')
            {
                do_write = true;
            }
        }
    }

    if (do_write)
    {
        std::ofstream ofs("output.dat", std::ios::binary);
#else // !OUTPUT_ON_FILE
#if defined(SEPARATE_IN_AND_OUT_STRING_STREAMS)
        std::ostringstream ofs;
#else // SEPARATE_IN_AND_OUT_STRING_STREAMS
        std::stringstream ofs;
#endif // SEPARATE_IN_AND_OUT_STRING_STREAMS
#endif // OUTPUT_ON_FILE

        printf("------------------[ WRITING MESSAGES ]------------------\n\n");

        { MyStruct               m; m >> ofs; LOG(m, MyStruct             ); }
        {
            AnotherStruct m;

            LOG(m, AnotherStruct);

            m.bmin = SCHAR_MIN;
            m.bmax = SCHAR_MAX;
            m.smin = SHRT_MIN;
            m.smax = SHRT_MAX;
            m.imin = INT_MIN;
            m.imax = INT_MAX;
            m.lmin = LLONG_MIN;
            m.lmax = LLONG_MAX;

            m.aMap[0] = "zero";
            m.aMap[1] = "uno";
            m.aMap[2] = "due";
            m.aMap[3] = "tre";
            m.aMap[7] = "sette";

            m >> ofs;

            LOG(m, AnotherStruct);

            m.bnil       = 2;
            m.bone       = 3;
            m.bar        = 1.21;
            m.dat.anEnum = static_cast<uint32_t>(enThree);

            m >> ofs;

            LOG(m, AnotherStruct);
        }
        { NoParamMessage         m; m >> ofs; LOG(m, NoParamMessage        ); }
        { NumericNoParamMessage  m; m >> ofs; LOG(m, NumericNoParamMessage ); }
        { NumericExtraParameters m; m >> ofs; LOG(m, NumericExtraParameters); }
        {
            NumericMessage m;

            m.bytes.push_back(0);
            m.bytes.push_back(1);
            m.bytes.push_back(-125);
            m.bytes.push_back(124);

            m.levels.push_back(0.123456789);
            m.levels.push_back(1.123456789);
            m.levels.push_back(2.123456789);

            m.opt_param = string("Valued optional parameter");

            m.numbers.push_back(3.14);
            m.numbers.push_back(2.81);

            m.names.push_back("Aldo");
            m.names.push_back("Giovanni");
            m.names.push_back("Giacomo");
            m.names.push_back("Francesco");

            MyStruct s;
            s.anEnum = static_cast<uint32_t>(enOne);
            s.aChar  = 'Q';
            s.aFloat = 6.28f;

            m.structs.push_back(s);

            s.anEnum = static_cast<uint32_t>(enTwo);
            s.aChar  = 'W';
            s.aFloat = 9.81f;

            m.structs.push_back(s);

            s.anEnum   = static_cast<uint32_t>(enThree);
            s.aChar    = 'R';
            s.aFloat   = 1.21f;
            s.optional = false;

            m.opt_struct = s;

            m >> ofs;

            LOG(m, NumericMessage);
        }
        {
            ComplexStruct m;

            size_t i, j;

            for (i=0; i<m.bits.size(); i++)
            {
                m.bits.set(i, (i % 2));
            }

            for (j=0; j<3; j++)
            {
                FlagsT bs;

                for (i=0; i<bs.size(); i++)
                {
                    bs.set(i, !(i % 2));
                }

                m.iperbits.push_back(bs);
            }

            m.root.bmin = SCHAR_MIN;
            m.root.bmax = SCHAR_MAX;
            m.root.smin = SHRT_MIN;
            m.root.smax = SHRT_MAX;
            m.root.imin = INT_MIN;
            m.root.imax = INT_MAX;
            m.root.lmin = LLONG_MIN;
            m.root.lmax = LLONG_MAX;

            m.root.aMap[1]  = "primo";
            m.root.aMap[2]  = "secondo";
            m.root.aMap[3]  = "terzo";
            m.root.aMap[5]  = "quarto";
            m.root.aMap[7]  = "quinto";
            m.root.aMap[11] = "sesto";

            m >> ofs;

            LOG(m, ComplexStruct);
        }

#if defined(OUTPUT_ON_FILE)
        ofs.close();
    }

    if (do_read)
    {
        std::ifstream ifile;
        std::istream *is = &cin;

        if (from_file)
        {
            ifile.open("output.dat", std::ios::binary);

            is = &ifile;
        }

#else // OUTPUT_ON_FILE
#if defined(SEPARATE_IN_AND_OUT_STRING_STREAMS)
        std::istringstream ifs(ofs.str());
#else // SEPARATE_IN_AND_OUT_STRING_STREAMS
        std::stringstream & ifs = ofs;
#endif // SEPARATE_IN_AND_OUT_STRING_STREAMS
        std::istream *is = &ifs;
#endif // OUTPUT_ON_FILE

        printf("------------------[ READING MESSAGES ]------------------\n\n");

        Message m;

        while (*is >> m) // same of (m << *is)
        {
            handle_message(m);

            if (m == m_AnotherStruct)
            {
                m_AnotherStruct.bnil         = 1;
                m_AnotherStruct.bone         = 0;
                m_AnotherStruct.bar          = 6.28;
                m_AnotherStruct.dat.optional = false;
            }

            handle_message(m);
        }

#if defined(OUTPUT_ON_FILE)
        if (from_file)
        {
            std::ifstream *pifs =static_cast<std::ifstream *>(is);

            pifs->close();
        }
    }
#endif // OUTPUT_ON_FILE
}
#endif // MINIMAL
