#if !defined(__HOB_JSON_HPP__)
#define __HOB_JSON_HPP__

#include <stdlib.h>
#include <ctype.h>
#include <sstream>
#include <cstring>
#include <iomanip>
#include "hob/codec/flat.hpp"

using namespace std;

namespace hobio
{
    static const char * CODE_TAG   = "\"id\"    : ";
    static const char * FIELDS_TAG = "\"fields\": ";
    static const char * TYPE_TAG   = "\"type\"  : ";
    static const char * SIZE_TAG   = "\"size\"  : ";
    static const char * NAME_TAG   = "\"name\"  : ";
    static const char * DESC_TAG   = "\"desc\"  : ";
    static const char * VALUE_TAG  = "\"value\" : ";
    static const char * KEY_TAG    = "\"key\"   : ";

    namespace json
    {
        enum Format
        {
            VERBOSE,
            COMPACT
        };

        class encoder
            : public flat::encoder
        {
        public:
            encoder(hobio::writer &os)
                : flat::encoder(     os)
                , _os          (os.os())
                , _format      (COMPACT)
            {
            }

            virtual ~encoder()
            {
            }

            virtual encoder & operator << (json::Format format)
            {
                _format = format;

                return *this;
            }

            virtual flat::encoder & operator << (flat::Encoding format)
            {
                return (*static_cast<flat::encoder*>(this)) << format;
            }

            virtual bool encode_header(const char     *name ,
                                       const string   &value,
                                       const hob::UID &id   ,
                                       const size_t   &payload)
            {
                return encode_header(name, value.c_str(), id, payload);
            }

            virtual bool encode_header(const char     *name ,
                                       const hob::UID &value,
                                       const hob::UID &id   ,
                                       const size_t   &payload)
            {
                stringstream ss;

                ss << value;

                return encode_header(name, ss.str().c_str(), id, payload);
            }

            virtual bool encode_header(const char     *name ,
                                       const char     *value,
                                       const hob::UID &id   ,
                                       const size_t   &payload)
            {
                level(+1);

                _has_payload[level()] = (payload > 0);

                _os << noshowpos << "{";

                if (_format == VERBOSE)
                {
                    if (NULL != name)
                    {
                        _os << endl
                            << padding(1)
                            << TYPE_TAG
                            << "\""
                            << name
                            << "\",";
                    }

                    _os << endl
                        << padding(1)
                        << CODE_TAG;
                }

                indentation(+1);

                encode(id);

                indentation(-1);

                if ((_format == VERBOSE) && (NULL != value))
                {
                    _os << ","
                        << endl
                        << padding(1)
                        << DESC_TAG
                        << "\""
                        << value
                        << "\"";
                }

                if (_has_payload[level()])
                {
                    _os << ",";

                    if (_format == VERBOSE)
                    {
                        _os << endl
                            << padding(1)
                            << SIZE_TAG;
                    }

                    indentation(+1);

                    encode(payload);

                    indentation(-1);

                    if (_format == VERBOSE)
                    {
                        _os << ","
                            << endl
                            << padding(1)
                            << FIELDS_TAG
                            << "["
                            << endl;
                    }
                }

                return true;
            }

            virtual bool encode_field_pre(const char *type = NULL,
                                          const char *name = NULL)
            {
                (void)type;
                (void)name;

                if (_format == VERBOSE)
                {
                    _os << padding(2)
                        << "{"
                        << endl;

                    if (NULL != type)
                    {
                        _os << padding(3)
                            << TYPE_TAG
                            << "\""
                            << type
                            << "\","
                            << endl;
                    }

                    if (NULL != name)
                    {
                        _os << padding(3)
                            << NAME_TAG
                            << "\""
                            << name
                            << "\","
                            << endl;
                    }

                    _os << padding(3)
                        << VALUE_TAG;
                }

                indentation(+3);

                return true;
            }

            virtual bool encode_field_post(const char *type = NULL,
                                           const char *name = NULL)
            {
                (void)type;
                (void)name;

                indentation(-3);

                if (_format == VERBOSE)
                {
                    _os << endl
                        << padding(2)
                        << "},"
                        << endl;
                }

                return true;
            }


            virtual inline bool encode_footer()
            {
                if (_has_payload[level()] && (_format == VERBOSE))
                {
                    _os << padding(2)
                        << "null"
                        << endl
                        << padding(1)
                        << "]";
                }

                if (_format == VERBOSE)
                {
                   _os << endl;
                }

                _os << padding(0) << "}";

                level(-1);

                return (level() > 0) && _os.good();
            }

        protected:
            virtual bool encode(const uint8_t &v)
            {
                _os << noshowpos
                    << "{\"C\":"
                    << static_cast<unsigned int>(v)
                    << "}";

                return true;
            }

            virtual bool encode(const uint16_t &v)
            {
                _os << noshowpos
                    << "{\"S\":"
                    << v
                    << "}";

                return true;
            }

            virtual bool encode(const uint32_t &v)
            {
                _os << noshowpos
                    << "{\"I\":"
                    << v
                    << "}";

                return true;
            }

            virtual bool encode(const uint64_t &v)
            {
                _os << noshowpos
                    << "{\"L\":"
                    << v
                    << "}";

                return true;
            }

            virtual bool encode(const int8_t &v)
            {
                _os << "{\"C\":"
                    << showpos
                    << static_cast<int>(v)
                    << noshowpos
                    << "}";

                return true;
            }

            virtual bool encode(const int16_t &v)
            {
                _os << "{\"S\":"
                    << showpos
                    << v
                    << noshowpos
                    << "}";

                return true;
            }

            virtual bool encode(const int32_t &v)
            {
                _os << "{\"I\":"
                    << showpos
                    << v
                    << noshowpos
                    << "}";

                return true;
            }

            virtual bool encode(const int64_t &v)
            {
                _os << "{\"L\":"
                    << showpos
                    << v
                    << noshowpos
                    << "}";

                return true;
            }

            virtual bool encode(const float &v)
            {
                _os << noshowpos
                    << "{\"F\":\"";

                encode(&v, sizeof(v));

                _os << "\"}";

                return true;
            }

            virtual bool encode(const double &v)
            {
                _os << noshowpos
                    << "{\"D\":\"";

                encode(&v, sizeof(v));

                _os << "\"}";

                return true;
            }

            virtual bool encode(const long double &v)
            {
                _os << noshowpos
                    << "{\"Q\":\"";

                encode(&v, sizeof(v));

                _os << "\"}";

                return true;
            }

            virtual bool encode(const char *v)
            {
                _os << noshowpos
                    << "{\"T\":\""
                    << v
                    << "\"}";

                return true;
            }

            virtual bool encode(const string &v)
            {
                _os << noshowpos
                    << "{\"T\":\""
                    << v
                    << "\"}";

                return true;
            }

            virtual bool encode(const bool &v)
            {
                _os << (v ? "{\"B\":true}" : "{\"B\":false}");

                return true;
            }

            virtual bool encode(const hob &v)
            {
                return (v >> *this);
            }

            virtual bool encode_bitset(size_t size_, const uint8_t *bits)
            {
                if (NULL == bits)
                {
                    return false;
                }

                _os << noshowpos
                    << "{\"B\":\"";

                for (ssize_t i=size_-1; i>=0; i--)
                {
                    _os << static_cast<unsigned int>((bits[i>>3]>>(i%8)) & 1);
                }

                _os << "\"}";

                return true;
            }

            virtual bool encode_vector_begin(size_t len)
            {
                _os << noshowpos
                    << "{";

                if (_format == VERBOSE)
                {
                    _os << endl;
                }

                _os << padding(1)
                    << "\"V("
                    << len
                    << ")\":[";

                if (_format == VERBOSE)
                {
                    _os << endl;
                }

                return true;
            }

            virtual void encode_vector_item_pre(size_t i, size_t len)
            {
                (void)i;
                (void)len;

                _os << padding(2);

                indentation(+2);
            }

            virtual void encode_vector_item_post(size_t i, size_t len)
            {
                indentation(-2);

                if ((_format == VERBOSE) && (i < (len-1)))
                {
                    _os << ",";
                }

                if (_format == VERBOSE)
                {
                    _os << endl;
                }
            }

            virtual bool encode_vector_end()
            {
                _os << padding(1) << "]";

                if (_format == VERBOSE)
                {
                    _os << endl;
                }

                _os << padding(0) << "}";

                return true;
            }

            virtual bool encode(const vector<bool> &v)
            {
                _os << noshowpos
                    << "{\"B("
                    << v.size()
                    << ")\":\"";

                for (size_t i=0; i<v.size(); i++)
                {
                    _os << v[i];
                }

                _os << "\"}";

                return true;
            }

            virtual bool encode_variant_begin(hob::UID id, uint8_t type)
            {
                _os << noshowpos
                    << "{";

                if (_format == VERBOSE)
                {
                    _os << endl
                        << padding(1)
                        << CODE_TAG;

                }

                indentation(+1);

                encode(id);

                indentation(-1);

                if (_format == VERBOSE)
                {
                    _os << endl
                        << padding(1)
                        << TYPE_TAG;

                }

                indentation(+1);

                encode(type);

                indentation(-1);

                if (_format == VERBOSE)
                {
                    _os << ","
                        << endl
                        << padding(1)
                        << FIELDS_TAG;
                }

                indentation(+1);

                return true;
            }

            virtual bool encode_variant_end()
            {
                indentation(-1);

                if (_format == VERBOSE)
                {
                    _os << endl;
                }

                _os << padding(0) << "}";

                return true;
            }

            virtual bool encode_optional_begin(const bool &has_value)
            {
                _os << noshowpos
                    << "{";

                if (_format == VERBOSE)
                {
                    _os << endl;
                }

                _os << padding(1)
                    << "\"O("
                    << has_value
                    << ")\":";

                return true;
            }

            virtual void encode_optional_item_pre()
            {
                indentation(+1);
            }

            virtual void encode_optional_item_post()
            {
                indentation(-1);
            }

            virtual bool encode_optional_end(const bool &has_value)
            {
                if (!has_value && (_format == VERBOSE))
                {
                    _os << "null";
                }

                if (_format == VERBOSE)
                {
                    _os << endl;
                }

                _os << padding(0) << "}";

                return true;
            }

            virtual bool encode_map_begin(const size_t &len)
            {
                _os << noshowpos
                    << "{";

                if (_format == VERBOSE)
                {
                    _os << endl;
                }

                _os << padding(1)
                    << "\"M("
                    << len
                    << ")\":[";

                if (_format == VERBOSE)
                {
                    _os << endl;
                }

                return true;
            }

            virtual void encode_map_item_key_pre()
            {
                _os << padding(2)
                    << "{";

                if (_format == VERBOSE)
                {
                    _os << endl;
                    _os << padding(3)
                        << KEY_TAG;
                }

                indentation(+1);
            }

            virtual void encode_map_item_key_post()
            {
                indentation(-1);

                _os << ",";
            }

            virtual void encode_map_item_value_pre()
            {
                if (_format == VERBOSE)
                {
                    _os << endl
                        << padding(3)
                        << VALUE_TAG;
                }

                indentation(+3);
            }

            virtual void encode_map_item_value_post(const size_t &remaining)
            {
                indentation(-3);

                if (_format == VERBOSE)
                {
                    _os << endl
                        << padding(2);
                }

                _os << "}";

                if (_format == VERBOSE)
                {
                    if (remaining > 1)
                    {
                        _os << ",";
                    }

                    _os << endl;
                }
            }

            virtual bool encode_map_end()
            {
                _os << padding(1) << "]";

                if (_format == VERBOSE)
                {
                    _os << endl;
                }

                _os << padding(0) << "}";

                return true;
            }

            virtual bool encode(const void *v, size_t s)
            {
                for (size_t i = 0; i<s; i++)
                {
                      _os << hex
                          << setw(2)
                          << setfill('0')
                          << static_cast<int>(reinterpret_cast<const char*>(v)[i] & 0xff);
                }

                _os << dec
                    << setw(0);

                return true;
            }

        private:
            ostream          &_os;
            Format            _format;
            map<size_t,bool>  _has_payload;

            inline int level(int count=0)
            {
                static int _level = 0;

                _level += count;

                return _level;
            }

            inline int indentation(int count=0)
            {
                static int _indentation = 0;

                _indentation += count;

                return _indentation;
            }

            inline string padding(size_t p)
            {
                return string(((_format == VERBOSE)?(indentation() + (p)):0)*4,' ');
            }
        };

        class decoder
            : public flat::decoder
        {
        public:
            decoder(hobio::reader &is)
                : flat::decoder(is)
                , _depth(-1)
            {
            }

            virtual ~decoder()
            {
            }

            bool operator>>(hob::encoder &enc)
            {
                return parse(enc,'\0');
            }

            virtual bool load(bool update=true)
            {
                if (NULL == buffer() || update)
                {
                    hobio::writer *os = writer(hobio::writer::CLEAR);

                    if (NULL == os)
                    {
                        return false;
                    }

                    _depth = -1;

                    flat::encoder enc(*os, encoding());

                    return parse(enc,'\0');
                }

                return true;
            }

        private:
            int _depth;

            bool parse(hob::encoder &os, uint8_t t)
            {
                uint8_t c  = 0;
                bool start = ('\0' == t);
                bool group = ('}'  == t);
                // bool array = (']'  == t);
                bool kword = ('"'  == t);
                bool skip_  = false;

                if (group)
                {
                    _depth++;
                }

                string token;

                while ((_depth >= -1) && stream().get(c) && ((t != c) || skip_))
                {
                    if (start && ('{' != c))
                    {
                        continue;
                    }

                    if (kword)
                    {
                        if (!skip_ && ('\\' == c))
                        {
                            skip_ = true;
                        }
                        else
                        {
                            skip_ = false;
                            token += c;
                        }
                    }
                    else
                    if (!isspace(c))
                    {
                        skip_ = false;

                        switch (c)
                        {
                            case '{' : parse(os,'}'); break;
                            case '[' : parse(os,']'); break;
                            case '"' : parse(os,'"'); break;
                            default  :             break;
                        }
                    }
                }

                if (group && (t == c))
                {
                    _depth--;

                    if (_depth == -1)
                    {
                        _depth--;

                        return true;
                    }
                }

                if ((_depth >= 0) && !token.empty())
                {
                    while (stream().get(c))
                    {
                        if (':' == c)
                        {
                            string value;
                            bool   dump        = false;
                            bool   has_value   = true;
                            bool   is_optional = false;
                            size_t count_pos   = 0;

                            if (("C" == token) || // [u]int8_t  (char)
                                ("S" == token) || // [u]int16_t (short)
                                ("I" == token) || // [u]int32_t (int)
                                ("L" == token))   // [u]int64_t (long)
                            {
                                dump = true;
                            }
                            else if ("F" == token) // float
                            {
                                dump = true;
                            }
                            else if ("D" == token) // double
                            {
                                dump = true;
                            }
                            else if ("Q" == token) // long double
                            {
                                dump = true;
                            }
                            else if ("B" == token) // bool / bitset<>
                            {
                                dump = true;
                            }
                            else if ("T" == token) // string / char*
                            {
                                dump = true;
                            }
                            else if (token.find("V(") == 0) // vector<>
                            {
                                dump      = true;
                                has_value = false;
                                count_pos = strlen("V(");
                            }
                            else if (token.find("M(") == 0) // map<>
                            {
                                dump      = true;
                                has_value = false;
                                count_pos = strlen("M(");
                            }
                            else if (token.find("O(") == 0) // optional<>
                            {
                                dump        = true;
                                has_value   = false;
                                count_pos   = strlen("O(");
                                is_optional = true;
                            }
                            else if (token.find("B(") == 0) // vector<bool>
                            {
                                dump      = true;
                                has_value = true;
                                // count_pos = strlen("B(");
                            }

                            if (dump)
                            {
                                if (has_value)
                                {
                                    bool can_be_spaced = false;

                                    skip_ = false;

                                    while (stream().get(c))
                                    {
                                        if (can_be_spaced || !isspace(c))
                                        {
                                            if (!skip_ && ('"' == c))
                                            {
                                                can_be_spaced = !can_be_spaced;
                                            }
                                            else
                                            if (!skip_ && ('\\' == c))
                                            {
                                                skip_ = true;
                                            }
                                            else
                                            {
                                                skip_ = false;

                                                if ('}' == c)
                                                {
                                                    stream().unget(c);

                                                    break;
                                                }
                                                else
                                                {
                                                    value += c;
                                                }
                                            }
                                        }
                                    }
                                }

                                if (count_pos > 0)
                                {
                                    string count = token.substr(count_pos,
                                                                token.size()-count_pos-1);

                                    if (is_optional)
                                    {
                                        bool v_count = ("1" == count);

                                        if (!os.encode_field(v_count))
                                        {
                                            return false;
                                        }
                                    }
                                    else
                                    {
                                        uint64_t v_count = strtoull(count.c_str(),NULL,10);

                                        if (!os.encode_field(v_count))
                                        {
                                            return false;
                                        }
                                    }
                                }

                                if (has_value)
                                {
                                    if (("C" == token) || // [u]int8_t  (char)
                                        ("S" == token) || // [u]int16_t (short)
                                        ("I" == token) || // [u]int32_t (int)
                                        ("L" == token))   // [u]int64_t (long)
                                    {
                                        if (value[0] == '+' || value[0] == '-')
                                        {
                                            // signed integer values

                                            int64_t v = strtoll(value.c_str(),NULL,10);

                                            if ("C" == token)
                                            {
                                                if (!os.encode_field(static_cast<int8_t>(v)))
                                                {
                                                    return false;
                                                }
                                            }
                                            else
                                            if ("S" == token)
                                            {
                                                if (!os.encode_field(static_cast<int16_t>(v)))
                                                {
                                                    return false;
                                                }
                                            }
                                            else
                                            if ("I" == token)
                                            {
                                                if (!os.encode_field(static_cast<int32_t>(v)))
                                                {
                                                    return false;
                                                }
                                            }
                                            else
                                            if ("L" == token)
                                            {
                                                if (!os.encode_field(static_cast<int64_t>(v)))
                                                {
                                                    return false;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            // unsigned integer values

                                            uint64_t v = strtoull(value.c_str(),NULL,10);

                                            if ("C" == token)
                                            {
                                                if (!os.encode_field(static_cast<uint8_t>(v)))
                                                {
                                                    return false;
                                                }
                                            }
                                            else
                                            if ("S" == token)
                                            {
                                                if (!os.encode_field(static_cast<uint16_t>(v)))
                                                {
                                                    return false;
                                                }
                                            }
                                            else
                                            if ("I" == token)
                                            {
                                                if (!os.encode_field(static_cast<uint32_t>(v)))
                                                {
                                                    return false;
                                                }
                                            }
                                            else
                                            if ("L" == token)
                                            {
                                                if (!os.encode_field(static_cast<uint64_t>(v)))
                                                {
                                                    return false;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    if ("F" == token) // float
                                    {
                                        if (!os.encode_field(hex_to_val<float>(value)))
                                        {
                                            return false;
                                        }
                                    }
                                    else
                                    if ("D" == token) // double
                                    {
                                        if (!os.encode_field(hex_to_val<double>(value)))
                                        {
                                            return false;
                                        }
                                    }
                                    else
                                    if ("Q" == token) // long double
                                    {
                                        if (!os.encode_field(hex_to_val<long double>(value)))
                                        {
                                            return false;
                                        }
                                    }
                                    else
                                    if ("T" == token) // string / char*
                                    {
                                        if (!os.encode_field(value))
                                        {
                                            return false;
                                        }
                                    }
                                    else
                                    if ("B" == token)        // bool // bitset<>
                                    {
                                        if (("true" == value) || ("false" == value))
                                        {
                                            bool v_value = ("true" == value);

                                            if (!os.encode_field(v_value)) { return false; };
                                        }
                                        else
                                        {
                                            const size_t N = value.size();

                                            uint8_t bits[(N+7)>>3];

                                            memset(bits, 0, ((N+7)>>3));

                                            for (size_t i=0; i<N; i++)
                                            {
                                                if (('1' == value[i]) || ('0' == value[i]))
                                                {
                                                    bits[i>>3] |= (('1' == value[N-i-1])<<(i%8));
                                                }
                                            }

                                            if (!os.encode_bitset(N, bits))
                                            {
                                                return false;
                                            }
                                        }
                                    }
                                    else
                                    if (token.find("B(") == 0) // vector<bool>
                                    {
                                        vector<bool> v_value;

                                        for (ssize_t i=value.size()-1; i>=0; i--)
                                        {
                                            if (('1' == value[i]) || ('0' == value[i]))
                                            {
                                                v_value.push_back('1' == value[i]);
                                            }
                                        }

                                        if (!os.encode_field(v_value))
                                        {
                                            return false;
                                        }
                                    }
                                }
                            }

                            break;
                        }
                        else
                        if (!isspace(c))
                        {
                            stream().unget(c);

                            break;
                        }
                    }
                }

                return true;
            }

            inline uint8_t hex_to_int(const char &c)
            {
                if (c >= '0' && c <= '9') { return c - '0' +  0; }
                if (c >= 'A' && c <= 'F') { return c - 'A' + 10; }
                if (c >= 'a' && c <= 'f') { return c - 'a' + 10; }
                return 0xff;
            }

            template<class T>
            inline T hex_to_val(const string &s)
            {
                union { T n; uint8_t x[sizeof(T)]; } x2n;

                for (size_t i = 0; i < sizeof(T); i++)
                {
                    x2n.x[i] = (hex_to_int(s[(2*i)+0]) & 0x0f) << 4 |
                               (hex_to_int(s[(2*i)+1]) & 0x0f);
                }

                return x2n.n;
            }
        };
    } // namespace json
} // namespace hobio

#endif // __HOB_JSON_HPP__
