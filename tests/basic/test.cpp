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
#include <fstream>
#include "HOB.hpp"
#include <inttypes.h>
#include <limits.h>
#include <getopt.h>
#include "../HOBs.h"

HOB::Dump dump_mode = HOB::JSON;

MyStruct               m_MyStruct              ;
AnotherStruct          m_AnotherStruct         ;
NoParamMessage         m_NoParamMessage        ;
NumericNoParamMessage  m_NumericNoParamMessage ;
NumericMessage         m_NumericMessage        ;
NumericExtraParameters m_NumericExtraParameters;
ComplexStruct          m_ComplexStruct         ;

static struct option const long_options[] =
{
    // These options set a flag
    {"write"     , required_argument, 0, 'w'},
    {"read"      , optional_argument, 0, 'r'},
    {"inport"    , optional_argument, 0, 'i'},
    {"dump"      , optional_argument, 0, 'd'},
    {"help"      , no_argument      , 0, 'h'},

    {0           , 0                , 0,   0}
};

char * dump_modes[] =
{
    [HOB::JSON] = (char []){ 'j', 's', 'o', 'n', '\0' },
    [HOB::TEXT] = (char []){ 't', 'e', 'x', 't', '\0' },
    NULL
};

void help(const char *my_name)
{
    static const char * const descr_options[] =
    {
        "<output_file>\n\n\twrite HOBs to binary <output_file>",
        "<input_file|->\n\n\tread HOBs from binary <input_file> or from standard input <->",
        "<input_file|->\n\n\tinport HOBs from text/json <input_file> or from standard input <->",
        "<text|json>\n\n\tdump HOBs in plain <text> or <json> format",
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

bool handle_message(HOB &m)
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

    if (m >> m_MyStruct)
    {
        LOG(m_MyStruct);
    }
    else
    if (m >> m_AnotherStruct)
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
    else
    if (m >> m_NoParamMessage)
    {
        LOG(m_NoParamMessage);
    }
    else
    if (m >> m_NumericNoParamMessage)
    {
        LOG(m_NumericNoParamMessage);
    }
    else
    if (m >> m_NumericMessage)
    {
        LOG(m_NumericMessage);
    }
    else
    if (m >> m_ComplexStruct)
    {
        LOG(m_ComplexStruct);
    }
/*
    else
    if (m_NumericExtraParameters << m)
    {
        LOG(m_NumericExtraParameters);
    }
*/
    else
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

#if defined(OUTPUT_ON_FILE)
    bool do_read   = false;
    bool do_write  = false;
    bool do_inport = false;
    bool from_file = false;
    string in_file;

    int  file_arg  = 0;

    int  option_index = 0;
    int  c;

    char *subopts;
    char *value;
    int   errfnd = 0;


    while (true)
    {
        c = getopt_long(argc,
                        argv,
                        "w:r:i:d:h",
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
                    
                    if (NULL != optarg)
                    {
                        in_file  = optarg;
                    }
                    else
                    {
                        help(argv[0]);
                    }
                }
                break;

            case 'r':
                {
                    do_read = true;

                    if (NULL != optarg)
                    {
                        in_file = optarg;

                        if (!in_file.empty() && in_file != "-")
                        {
                            from_file = true;
                        }
                    }
                }
                break;

            case 'i':
                {
                    do_inport = true;

                    if (NULL != optarg)
                    {
                        in_file = optarg;

                        if (!in_file.empty() && in_file != "-")
                        {
                            from_file = true;
                        }
                    }
                }
                break;

            case 'h':
                {
                    help(argv[0]);
                }
                break;

            case 'd':
                {
                    subopts = optarg;

                    while ((NULL != subopts) && (*subopts != '\0') && !errfnd)
                    {
                        switch (getsubopt(&subopts, dump_modes, &value))
                        {
                            case HOB::JSON:
                                dump_mode = HOB::JSON;
                                break;

                            case HOB::TEXT:
                                dump_mode = HOB::TEXT;
                                break;

                            default:
                                errfnd = 1;
                                break;
                        }
                    }
                }
                break;

            default:
                break;
        }
    }

    if (!do_read && !do_write && !do_inport)
    {
        help(argv[0]);
    }

    if (do_write)
    {
        std::ofstream ofs(in_file.c_str(), std::ios::binary);
#else // !OUTPUT_ON_FILE
#if defined(SEPARATE_IN_AND_OUT_STRING_STREAMS)
        std::ostringstream ofs;
#else // SEPARATE_IN_AND_OUT_STRING_STREAMS
        std::stringstream ofs;
#endif // SEPARATE_IN_AND_OUT_STRING_STREAMS
#endif // OUTPUT_ON_FILE

        printf("------------------[ WRITING HOBS ]------------------\n\n");

        { MyStruct               m; m >> ofs; LOG(m); }
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

            m >> ofs;

            LOG(m);

            m.bnil       = 2;
            m.bone       = 3;
            m.bar        = 1.21;
            m.dat.anEnum = static_cast<uint32_t>(enThree);

            m >> ofs;

            LOG(m);
        }
        { NoParamMessage         m; m >> ofs; LOG(m); }
        { NumericNoParamMessage  m; m >> ofs; LOG(m); }
        { NumericExtraParameters m; m >> ofs; LOG(m); }
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

            m >> ofs;

            LOG(m);
        }

#if defined(OUTPUT_ON_FILE)
        ofs.close();
    }

    if (do_read || do_inport)
    {
        std::ifstream ifile;
        std::istream *is = &cin;
        std::stringstream os;

        if (from_file)
        {
            if (do_inport)
            {
                ifile.open(in_file.c_str());
            }
            else
            {
                ifile.open(in_file.c_str(), std::ios::binary);
            }

            is = &ifile;
        }

        HOB::Src src(*is);
        HOB::Snk snk(os);

        if (do_inport && (src >> snk)) // same of HOB::parse(src, snk)
        {
            is = &os;
        }

#else // OUTPUT_ON_FILE
#if defined(SEPARATE_IN_AND_OUT_STRING_STREAMS)
        std::istringstream ifs(ofs.str());
#else // SEPARATE_IN_AND_OUT_STRING_STREAMS
        std::stringstream & ifs = ofs;
#endif // SEPARATE_IN_AND_OUT_STRING_STREAMS
        std::istream *is = &ifs;
#endif // OUTPUT_ON_FILE

        printf("------------------[ READING HOBS ]------------------\n\n");

        HOB m;

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
            std::ifstream *pifs = (do_inport) ? &ifile
                                              : static_cast<std::ifstream *>(is);

            pifs->close();
        }
    }
#endif // OUTPUT_ON_FILE
}
#endif // MINIMAL
