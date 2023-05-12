/******************************************************************************
//
// This build command generate a test executable which writes and read HOBs
// to and from a file:

g++ -std=c++98 \
    -DOUTPUT_ON_FILE \
    -O3 \
    test_message.cpp \
    -o test_message_on_file

******************************************************************************/

/******************************************************************************
//
// This build command generate a test executable which writes HOBs on a
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
// HOBs on the same stringstream for both output and input:

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
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <inttypes.h>
#include <limits.h>
#include <getopt.h>
#include "../hobs.h"
#include "hob/codec/flat.hpp"
#include "hob/codec/json.hpp"
#include "hob/io/buffer.hpp"
#include "hob/io/stream.hpp"

#if defined(TEST_VARIANT_HOBS) || defined(TEST_LONG_DOUBLE)
hob dh("DYNAMIC_FIELDS");
#if !defined(TEST_LONG_DOUBLE)
hob dh1("DEEP_NESTED_DYNAMIC_FIELDS");
#endif
#endif // TEST_VARIANT_HOBS

#if defined(__ANDROID__)
#include <stdlib.h>
#include <string.h>

/* Parse comma separated suboption from *OPTIONP and match against
 * strings in TOKENS.  If found return index and set *VALUEP to
 * optional value introduced by an equal sign.  If the suboption is
 * not part of TOKENS return in *VALUEP beginning of unknown
 * suboption.  On exit *OPTIONP is set to the beginning of the next
 * token or at the terminating NUL character.
 */
int getsubopt(char **optionp, char *const *tokens, char **valuep)
{
    char *endp, *vstart;
    int cnt;

    if (**optionp == '\0')
    {
        return -1;
    }

    /* Find end of next token.
     */
    endp = strchr(*optionp, ',');

    if (!endp)
    {
        endp = static_cast<char *>(*optionp) + strlen(*optionp);
    }

    /* Find start of value.
     */
    vstart = static_cast<char *>(memchr(*optionp, '=', endp - *optionp));

    if (vstart == NULL)
    {
        vstart = endp;
    }

    /* Try to match the characters between *OPTIONP and VSTART against
     * one of the TOKENS.
     */
    for (cnt = 0; tokens[cnt] != NULL; ++cnt)
    {
        if (!strncmp(*optionp, tokens[cnt], vstart - *optionp)
            &&
            tokens[cnt][vstart - *optionp] == '\0')
        {
            /* We found the current option in TOKENS.  */
            *valuep = vstart != endp ? vstart + 1 : NULL;

            if (*endp != '\0')
            {
                *endp++ = '\0';
            }

            *optionp = endp;

            return cnt;
        }
    }

    /* The current suboption does not match any option.  */
    *valuep = *optionp;

    if (*endp != '\0')
    {
        *endp++ = '\0';
    }

    *optionp = endp;

    return -1;
}
#endif

#if defined(TEST_FIXED_HOBS)
MyStruct               m_MyStruct              ;
AnotherStruct          m_AnotherStruct         ;
NoParamMessage         m_NoParamMessage        ;
NumericNoParamMessage  m_NumericNoParamMessage ;
NumericMessage         m_NumericMessage        ;
NumericExtraParameters m_NumericExtraParameters;
ComplexStruct          m_ComplexStruct         ;
#endif // TEST_FIXED_HOBS

static struct option const long_options[] =
{
    // These options set a flag
    {"write"     , required_argument, 0, 'w'},
    {"read"      , optional_argument, 0, 'r'},
    {"format"    , optional_argument, 0, 'f'},
    {"encoding"  , optional_argument, 0, 'e'},
    {"help"      , no_argument      , 0, 'h'},

    {0           , 0                , 0,   0}
};

char * dump_modes[] =
{
    (char []){ 'j', 's', 'o', 'n', '\0' },
    (char []){ 't', 'e', 'x', 't', '\0' },
    (char []){ 'f', 'l', 'a', 't', '\0' },
    NULL
};

char * flat_modes[] =
{
    (char []){ 'n', 'a', 't', 'i', 'v', 'e', '\0' },
    (char []){ 'v', 'a', 'r', 'i', 'n', 't', '\0' },
    NULL
};

void help(const char *my_name)
{
    static const char * const descr_options[] =
    {
        "<output_file>\n\n\twrite HOBs to <output_file>",
        "<input_file|->\n\n\tread HOBs from <input_file> or from standard input <->",
        "<text|json|flat>\n\n"
            "\tHOBs format:\n"
            "\t\tplain <text>\n"
            "\t\t<json> text\n"
            "\t\t<flat> binary\n",
        "<native|varint>\n\n"
            "\tBinary encoding format:\n"
            "\t\t<native> encoding\n"
            "\t\t<varint> encoding\n",
        "\n\n\tthis help",
    };

    cerr << endl << "Usage:" << endl << endl;

    char short_option[2];

    short_option[1] = '\0';

    for (size_t i = 0; i < sizeof(descr_options) / sizeof(char *); i++)
    {
        short_option[0] = long_options[i].val;

        cerr << my_name
             << " --"
             << long_options[i].name
             << " | "
             << "-"
             << short_option
             << " "
             << descr_options[i]
             << endl << endl;
    }

    cerr << endl;

    exit(0);
}

#if defined(MINIMAL)
int main(int argc, char *argv[])
{
    LOG(m_NumericNoParamMessage1);
    LOG(m_NumericNoParamMessage2);
    return 0;
}
#else // MINIMAL

bool handle_message(hob &m)
{

    bool handled = true;

#if defined(TEST_FIXED_HOBS)
    if ((m == m_MyStruct              ) ||
        (m == m_AnotherStruct         ) ||
        (m == m_NoParamMessage        ) ||
        (m == m_NumericNoParamMessage ) ||
        (m == m_NumericMessage        ) ||
        (m == m_NumericExtraParameters) ||
        (m == m_ComplexStruct         ))
    {
        printf("Known HOB\n");
    }

    if (m == m_AnotherStruct)
    {
        LOG(m_AnotherStruct);
    }

#if 0
    if ((m == m_AnotherStruct) || (m == m_NumericNoParamMessage))
    {
        printf("Skipping\n");

        LOG(m);

        m.skip();

        printf("Skipped\n");

        return;
    }
#endif
#endif

#if defined(TEST_VARIANT_HOBS) || defined(TEST_LONG_DOUBLE)
    if (m >> dh)
    {
        LOG(dh);
    }
    else
#if !defined(TEST_LONG_DOUBLE)
    if (m >> dh1)
    {
        LOG(dh1);
    }
    else
#endif // TEST_LONG_DOUBLE
#endif // TEST_VARIANT_HOBS
#if defined(TEST_FIXED_HOBS)
    if (m >> m_MyStruct)
    {
        LOG(m_MyStruct);
    }
    else if (m >> m_AnotherStruct)
    {
        LOG(m_AnotherStruct);

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
    else if (m >> m_NoParamMessage)
    {
        LOG(m_NoParamMessage);
    }
    else if (m >> m_NumericNoParamMessage)
    {
        LOG(m_NumericNoParamMessage);
    }
    else if (m >> m_NumericMessage)
    {
        LOG(m_NumericMessage);
    }
    else if (m >> m_ComplexStruct)
    {
        LOG(m_ComplexStruct);
    }
    else if (m_NumericExtraParameters << m)
    {
        LOG(m_NumericExtraParameters);
    }
    else
#endif // TEST_FIXED_HOBS
    {
        printf("Unknown HOB\n");
        LOG(m);

        handled = false;
    }

    return handled;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    hobio::iobuffer *io = NULL;
    hobio::writer   *os = NULL;
    hobio::reader   *is = NULL;

    bool do_read   = false;
    bool do_write  = false;
    int  dump_mode = -1;
    int  flat_mode = -1;
    string in_file;

    int  option_index = 0;
    int  c;

    char *subopts;
    char *value;
    int   errfnd = 0;

    while (true)
    {
        c = getopt_long(argc,
                        argv,
                        ":w::r::f:e:h",
                        long_options,
                        &option_index);

        if (c == -1)
        {
            break;
        }

        switch (c)
        {
            case 'w':
                {
                    do_write = true;

                    if (optarg)
                    {
                        os = new hobio::ostream(optarg);
                    }
                }
                break;

            case 'r':
                {
                    do_read = true;

                    if (optarg)
                    {
                        is = new hobio::istream(optarg);
                    }
                }
                break;

            case 'h':
                {
                    help(argv[0]);
                }
                break;

            case 'f':
                {
                    subopts = optarg;

                    while ((NULL != subopts) && (*subopts != '\0') && !errfnd)
                    {
                        dump_mode = getsubopt(&subopts, dump_modes, &value);

                        if (dump_mode > 2)
                        {
                            errfnd = 1;
                        }
                    }
                }
                break;

            case 'e':
                {
                    subopts = optarg;

                    while ((NULL != subopts) && (*subopts != '\0') && !errfnd)
                    {
                        flat_mode = getsubopt(&subopts, flat_modes, &value);

                        if (flat_mode > 1)
                        {
                            errfnd = 1;
                        }
                    }
                }
                break;

            case ':':
                switch (optopt)
                {
                    case 'w':
                        do_write = true;
                        break;

                    case 'r':
                        do_read = true;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    if (!do_read && !do_write && (NULL == os) && (NULL == is))
    {
        io = new hobio::iobuffer();
        os = static_cast<hobio::writer*>(io);
        is = static_cast<hobio::reader*>(io);

        do_read = do_write = true;
    }
    else if (do_write && !do_read && (NULL == os) && (NULL == is))
    {
        os = new hobio::ostream();
    }
    else if (!do_write && do_read && (NULL == os) && (NULL == is))
    {
        is = new hobio::istream();
    }

    if (do_write)
    {
        if (NULL == os)
        {
            os = new hobio::obuffer();
        }

        hobio::encoder *ps = NULL;

        switch (dump_mode)
        {
            case 0:
                {
                    hobio::json::encoder *js = new hobio::json::encoder(*os);

                    if (NULL != js)
                    {
                        *js << hobio::json::VERBOSE;

                        switch (flat_mode)
                        {
                            case 0:
                                *js << hobio::flat::NATIVE;

                                break;

                            default:
                                *js << hobio::flat::VARINT;

                                break;
                        }
                    }

                    ps = js;
                }
                break;

            case 1:
                {
                    hobio::json::encoder *js = new hobio::json::encoder(*os);

                    if (NULL != js)
                    {
                        *js << hobio::json::COMPACT;

                        switch (flat_mode)
                        {
                            case 0:
                                *js << hobio::flat::NATIVE;

                                break;

                            default:
                                *js << hobio::flat::VARINT;

                                break;
                        }
                    }

                    ps = js;
                }
                break;

            default:
                {
                    hobio::flat::encoder *fe = new hobio::flat::encoder(*os);

                    if (NULL != fe)
                    {
                        switch (flat_mode)
                        {
                            case 0:
                                *fe << hobio::flat::NATIVE;

                                break;

                            default:
                                *fe << hobio::flat::VARINT;

                                break;
                        }
                    }

                    ps = fe;
                }
                break;
        }

        printf("------------------[ WRITING HOBS ]------------------\n\n");

#if defined(TEST_LONG_DOUBLE)
        {
            vhob h("DYNAMIC_FIELDS");

            h.set<long double>("ld", 1.3131313131313131313);

            LOG(h);

            h >> *ps;

            cout << "-----------------------------------------------" << endl;
        }
#endif
#if defined(TEST_VARIANT_HOBS)
        {
            hob h("DYNAMIC_FIELDS");
            hob h1("DEEP_NESTED_DYNAMIC_FIELDS");

            vector<uint8_t> v;
            v.push_back(42);
            v.push_back(17);

            h.set< uint8_t         >("uint8_t"        ,  13    )
             .set< uint16_t        >("uint16_t"       ,  1313  )
             .set< uint32_t        >("uint32_t"       ,  131313)
             .set< int8_t          >("int8_t"         , -13    )
             .set< int16_t         >("int16_t"        , -1313  )
             .set< int32_t         >("int32_t"        , -131313)
             .set< vector<uint8_t> >("vector<uint8_t>", v      )
             .set< bool            >("bool"           , true   )
             .set< float           >("float"          , 1.3f   )
             .set< double          >("double"         , 1.313f )
#if defined(TEST_LONG_DOUBLE)
             .set< long double     >("quadle"         , 1.3131313131313131313)
#endif
             .set< string          >("string"         , "1Po'DiMaiuscoleMinuscole&Numeri")
/*
             .set<vhob>(
                "vhob",
                vhob("NESTED_DYNAMIC_FIELDS").set<string>("nested"  ,"deep")
                                             .set<vhob  >("MyHob",
                                                          h1.set<string>("nested2",
                                                                         "deepest")
                                                          )
                                             )
*/
            ;

            cout << "12 items expected:" << endl << endl;
/*
            {
                const vhob * nh = h.get<vhob>("vhob");

                if (NULL != nh)
                {
                    const string * n = (*nh).get<string>("nested");

                    (NULL != n) && cout << (*n) << endl;
                }
            }

            cout << endl << "0 items expected:" << endl << endl;
*/
            h.has("uint8_t" ) && cout << *h.get<uint8_t    >("uint8_t" ) << endl;
            h.has("uint16_t") && cout << *h.get<uint16_t   >("uint16_t") << endl;
            h.has("uint32_t") && cout << *h.get<uint32_t   >("uint32_t") << endl;
            h.has("int8_t"  ) && cout << *h.get<int8_t     >("int8_t"  ) << endl;
            h.has("int16_t" ) && cout << *h.get<int16_t    >("int16_t" ) << endl;
            h.has("int32_t" ) && cout << *h.get<int32_t    >("int32_t" ) << endl;
            h.has("bool"    ) && cout << *h.get<bool       >("bool"    ) << endl;
            h.has("float"   ) && cout << *h.get<float      >("float"   ) << endl;
            h.has("double"  ) && cout << *h.get<double     >("double"  ) << endl;
#if defined(TEST_LONG_DOUBLE)
            h.has("quadle"  ) && cout << *h.get<long double>("quadle"  ) << endl;
#endif
            h.has("string"  ) && cout << *h.get<string     >("string"  ) << endl;
/*
            if (h.has("vhob"))
            {
                const vhob * nh = h.get<vhob>("vhob");

                LOG((*nh));
            }
*/
            cout << endl << "Using operator[]:" << endl << endl;

            h.erase("uint16_t");
            h.erase("int16_t");
            h["bool"  ] = false;
            h["string"] = string("1Po'DiMinuscoleNumeri&Maiuscole");

            h.has("uint8_t" ) && cout << *h.get<uint8_t    >("uint8_t" ) << endl;
            h.has("uint16_t") && cout << *h.get<uint16_t   >("uint16_t") << endl;
            h.has("uint32_t") && cout << *h.get<uint32_t   >("uint32_t") << endl;
            h.has("int8_t"  ) && cout << *h.get<int8_t     >("int8_t"  ) << endl;
            h.has("int16_t" ) && cout << *h.get<int16_t    >("int16_t" ) << endl;
            h.has("int32_t" ) && cout << *h.get<int32_t    >("int32_t" ) << endl;
            h.has("bool"    ) && cout << *h.get<bool       >("bool"    ) << endl;
            h.has("float"   ) && cout << *h.get<float      >("float"   ) << endl;
            h.has("double"  ) && cout << *h.get<double     >("double"  ) << endl;
#if defined(TEST_LONG_DOUBLE)
            h.has("quadle"  ) && cout << *h.get<long double>("quadle"  ) << endl;
#endif
            h.has("string"  ) && cout << *h.get<string     >("string"  ) << endl;
/*
            vhob nh;
            string nested;

            if (h.get("vhob",nh))
            {
                if (nh.get("nested",nested))
                {
                    cout << "nested     : " << nested << endl;
                }
            }
*/
            LOG(h);

            h >> *ps;

            cout << "-----------------------------------------------" << endl;
        }
#endif // TEST_VARIANT_HOBS
#ifdef TEST_FIXED_HOBS
        { MyStruct               m; m >> *ps; LOG(m); }
        {
            AnotherStruct m;

            // LOG(m);

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

            m >> *ps;

            LOG(m);

            m.bnil       = 2;
            m.bone       = 3;
            m.bar        = 1.21;
            m.dat.anEnum = static_cast<uint32_t>(enThree);

            m >> *ps;

            LOG(m);
        }
        { NoParamMessage         m; m >> *ps; LOG(m); }
        { NumericNoParamMessage  m; m >> *ps; LOG(m); }
        { NumericExtraParameters m; m >> *ps; LOG(m); }
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

            m >> *ps;

            LOG(m);
        }
        {
            ComplexStruct m;

            size_t i, j;

            for (i=0; i<m.bits.size(); i++)
            {
                m.bits.set(i, (i % 2));
            }

            for (i=0; i<17; i++)
            {
                m.var_bits.push_back(i % 2);
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

            m >> *ps;

            LOG(m);
        }
#endif // TEST_FIXED_HOBS
        delete ps;
    }

    if (do_read)
    {
        if (NULL == is)
        {
            if (NULL == os)
            {
                help(argv[0]);
            }

            is = new hobio::ibuffer(static_cast<const hobio::obuffer&>(*os));
        }

        hobio::decoder *pp = NULL;

        switch (dump_mode)
        {
            case 0:
            case 1:
                {
                    hobio::json::decoder *js = new hobio::json::decoder(*is);

                    if (NULL != js)
                    {
                        switch (flat_mode)
                        {
                            case 0:
                                *js << hobio::flat::NATIVE;

                                break;

                            default:
                                *js << hobio::flat::VARINT;

                                break;
                        }
                    }

                    pp = js;
                }
                break;

            default:
                {
                    hobio::flat::decoder *fd = new hobio::flat::decoder(*is);

                    if (NULL != fd)
                    {
                        switch (flat_mode)
                        {
                            case 0:
                                *fd << hobio::flat::NATIVE;

                                break;

                            default:
                                *fd << hobio::flat::VARINT;

                                break;
                        }
                    }

                    pp = fd;
                }
                break;
        }

        printf("------------------[ READING HOBS ]------------------\n\n");

        hob m;

        while (*pp >> m) // same of (m << *is)
        {
            handle_message(m);

#if defined(TEST_FIXED_HOBS)
            if (m == m_AnotherStruct)
            {
                m_AnotherStruct.bnil         = 1;
                m_AnotherStruct.bone         = 0;
                m_AnotherStruct.bar          = 6.28;
                m_AnotherStruct.dat.optional = false;
            }

            handle_message(m);
#endif // TEST_FIXED_HOBS
        }

        delete pp;
    }

    if (NULL != io)
    {
        delete io;
        io = NULL;
        is = NULL;
        os = NULL;
    }

    if (NULL != is)
    {
         delete is;
         is = NULL;
    }

    if (NULL != os)
    {
         delete os;
         os = NULL;
    }
}
#endif // MINIMAL
