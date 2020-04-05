#if !defined(__HOB_HPP__)
#define __HOB_HPP__
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <limits>
#include <bitset>
#include <string>
#include <sstream>
#include <cstring>
#include <climits>
#include <iostream>
#include <iomanip>
#include "cpp_magic.h"
#include "optional.hpp"
#include "hobio.hpp"

#define M_LOG(...)            printf("%s:%s:%d: ",        \
                                     __FILE__,            \
                                     __PRETTY_FUNCTION__, \
                                     __LINE__);           \
                              printf(__VA_ARGS__);        \
                              printf("\n");

#define ASSERT_DUMP(os, ...)  if (!os.write(__VA_ARGS__)) { return false; }

#define INDENT(l)             string(((indent >= 0)?(indent + (l)):0)*4,' ')

#define HSEED                 65599llu
// #define HSEED                 11400714819323198485llu
#define HLEN(s)	              ((sizeof(s)/sizeof(s[0])) - 1)
#define H1(s,i,x)             (static_cast<uint64_t>(x)*(((i)<HLEN(s))?HSEED:1)\
                               +                                               \
                               static_cast<uint8_t>(s[((i)<HLEN(s))            \
                                                      ?HLEN(s)-1-(i)           \
                                                      :HLEN(s)]))

#define H4(s,i,x)             H1(s,(i)    , \
                              H1(s,(i)+  1, \
                              H1(s,(i)+  2, \
                              H1(s,(i)+  3, (x)))))

#define H16(s,i,x)            H4(s,(i)    , \
                              H4(s,(i)+  4, \
                              H4(s,(i)+  8, \
                              H4(s,(i)+ 12, (x)))))

#define H64(s,i,x)            H16(s,(i)    , \
                              H16(s,(i)+ 16, \
                              H16(s,(i)+ 32, \
                              H16(s,(i)+ 48, (x)))))

#define H256(s,i,x)           H64(s,(i)    , \
                              H64(s,(i)+ 64, \
                              H64(s,(i)+128, \
                              H64(s,(i)+192, (x)))))

#define HASH(x)               (static_cast<uint64_t>((x)^((x)>>32)))

#if defined(ENABLE_CPP_HASH)
#if defined(ENABLE_CPP_MAX_SIZE_HASH)
#define HUPDATE(s)   update_id(static_cast<uint64_t>( \
                               (HLEN(s)>0) ? H256(s,0,get_id()) \
                                           : get_id()))
#else // !ENABLE_CPP_MAX_SIZE_HASH
#define HUPDATE(s)   update_id(static_cast<uint64_t>( \
                               (HLEN(s)>0) ? H64(s,0,get_id()) \
                                           : get_id()))
#endif // !ENABLE_CPP_MAX_SIZE_HASH
#else // !ENABLE_CPP_HASH
#define HUPDATE(s)   update_id(s)
#endif // !ENABLE_CPP_HASH

using namespace std;
using namespace nonstd;

class HOB
    : public HOBIO::Serializeable
{
public:
    typedef uint64_t UID;

    static const UID UNDEFINED = ULLONG_MAX;

    enum Dump
    {
        JSON,
        TEXT,
    };

    HOB()
        : HOBIO::Serializeable()
        , _id(UNDEFINED)
        , _is(     NULL)
        , _sp(        0)
        , _ep(        0)
        , _np(       -1)
    { }

    HOB(const UID &id_)
        : HOBIO::Serializeable()
        , _id(   0)
        , _is(NULL)
        , _sp(   0)
        , _ep(   0)
        , _np(  -1)
    {
        update_id(id_);
    }

    HOB(const char *id_)
        : HOBIO::Serializeable()
        , _id(   0)
        , _is(NULL)
        , _sp(   0)
        , _ep(   0)
        , _np(  -1)
    {
        update_id(id_);
    }

    HOB(const string &id_)
        : HOBIO::Serializeable()
        , _id(   0)
        , _is(NULL)
        , _sp(   0)
        , _ep(   0)
        , _np(  -1)
    {
        update_id(id_);
    }

    HOB(const HOB &ref)
        : HOBIO::Serializeable()
    {
        *this = ref;
    }

    virtual ~HOB()
    {
    }

    HOB & operator=(const HOB & ref)
    {
        _id = ref._id;
        // _ss = ref._ss; // TODO: needed ?
        _is = ref._is;
        _sp = ref._sp;
        _ep = ref._ep;

        return *this;
    }

    inline bool operator<<(HOBIO::AbstractReader &is_)
    {
        flush_pending();

        return __read(is_);
    }

    inline bool operator<<(HOB & ref)
    {
        return ((_id == ref._id) && __read(ref));
    }

    virtual bool operator>>(HOBIO::AbstractWriter &os) const
    {
        if (UNDEFINED == _id)
        {
            return true;
        }

        os.alloc(__size_of(_id));

        if (!os.write(_id))
        {
            return false;
        }

        return true;
    }

    inline bool operator==(const HOB &ref) const
    {
        bool rv = (_id == ref._id);

        return rv;
    }

    inline bool operator!=(const HOB &ref) const
    {
        bool rv = (_id != ref._id);

        return rv;
    }

    inline virtual operator size_t() const
    {
        return __size_of(_id);
    }

    inline virtual bool rewind()
    {
        return static_cast<HOBIO::AbstractReader&>(*this).seek(_sp,SEEK_SET);
    }

    inline operator HOBIO::AbstractReader &() { return (NULL != _is) ? *_is : _ss; }

    inline virtual operator bool() const
    {
        return __is_changed();
    }

    inline void operator~()
    {
        __reset_changes();
    }

    const string operator()(const Dump &dump = TEXT) const
    {
        stringstream ss;

        ss.str(string());

        _s(ss, (JSON == dump) ? 0 : -1);

        return ss.str();
    }

    virtual bool __deserialize(HOBIO::AbstractReader &is_,
                               Serializeable &v,
                               ssize_t field=-1)
    {
        HOB m;

        if (m << is_)
        {
            v = m;

            if ( v << static_cast<HOBIO::AbstractReader&>(m) )
            {
                __set_changed(field, v.__is_changed());

                return true;
            }
        }

        return false;
    }

    static bool parse(HOBIO::AbstractReader &is, HOBIO::AbstractWriter &os)
    {
        return parse(is,os,'\0');
    }

protected:
#if !defined(BINARY_ONLY)
    static void _t(ostream &o, const uint8_t &v, int indent = -1)
    {
        (void)indent;

        o << noshowpos << "{\"C\":" << static_cast<unsigned int>(v) << "}";
    }

    static void _t(ostream &o, const uint16_t &v, int indent = -1)
    {
        (void)indent;

        o << noshowpos << "{\"S\":" << v << "}";
    }

    static void _t(ostream &o, const uint32_t &v, int indent = -1)
    {
        (void)indent;

        o << noshowpos << "{\"I\":" << v << "}";
    }

    static void _t(ostream &o, const uint64_t &v, int indent = -1)
    {
        (void)indent;

        o << noshowpos << "{\"L\":" << v << "}";
    }

    static void _t(ostream &o, const int8_t &v, int indent = -1)
    {
        (void)indent;

        o << "{\"C\":"
          << showpos
          << static_cast<int>(v)
          << noshowpos
          << "}";
    }

    static void _t(ostream &o, const int16_t &v, int indent = -1)
    {
        (void)indent;

        o << "{\"S\":"
          << showpos
          << v
          << noshowpos
          << "}";
    }

    static void _t(ostream &o, const int32_t &v, int indent = -1)
    {
        (void)indent;

        o << "{\"I\":"
          << showpos
          << v
          << noshowpos
          << "}";
    }

    static void _t(ostream &o, const int64_t &v, int indent = -1)
    {
        (void)indent;

        o << "{\"L\":"
          << showpos
          << v
          << noshowpos
          << "}";
    }

    static void _t(ostream &o, const float &v, int indent = -1)
    {
        (void)indent;

        o << noshowpos << "{\"F\":\"";

        _t(o, &v, sizeof(v));

        o << "\"}";
    }

    static void _t(ostream &o, const double &v, int indent = -1)
    {
        (void)indent;

        o << noshowpos << "{\"D\":\"";

        _t(o, &v, sizeof(v));

        o << "\"}";
    }

    static void _t(ostream &o, const long double &v, int indent = -1)
    {
        (void)indent;

        o << noshowpos << "{\"Q\":\"";

        _t(o, &v, sizeof(v));

        o << "\"}";
    }

    static void _t(ostream &o, const char *v, int indent = -1)
    {
        (void)indent;

        o << noshowpos << "{\"T\":\"" << v << "\"}";
    }

    static void _t(ostream &o, const string &v, int indent = -1)
    {
        (void)indent;

        o << noshowpos << "{\"T\":\"" << v << "\"}";
    }

    static void _t(ostream &o, const bool &v, int indent = -1)
    {
        (void)indent;

        o << (v ? "{\"B\":true}" : "{\"B\":false}");
    }

    static void _t(ostream &o, const HOB &v, int indent = -1)
    {
        v._s(o, indent);
    }

    template<size_t N>
    static void _t(ostream &o, const bitset<N> &v, int indent = -1)
    {
        (void)indent;

        o << noshowpos << "{\"B\":\"" << v << "\"}";
    }

    template<class T>
    static void _t(ostream &o, const vector<T> &v, int indent = -1)
    {
        (void)indent;

        o << noshowpos << "{";

        if (indent >= 0)
        {
            o << endl;
        }

        size_t len = v.size();

        o << INDENT(1) << "\"V(" << len << ")\":[";

        if (indent >= 0)
        {
            o << endl;
        }

        if (len > 0)
        {
            for (size_t i=0; i<len; i++)
            {
                o << INDENT(2);

                _t(o, static_cast<const T &>(v[i]),
                      (indent >= 0) ? (indent+2) : -1);

                if ((indent >=0) && (i < (len-1)))
                {
                    o << ",";
                }

                if (indent >= 0)
                {
                    o << endl;
                }
            }
        }

        o << INDENT(1) << "]";

        if (indent >= 0)
        {
            o << endl;
        }

        o << INDENT(0) << "}";
    }

    static void _t(ostream &o, const vector<bool> &v, int indent = -1)
    {
        (void)indent;

        o << noshowpos << "{\"B(" << v.size() << ")\":\"";

        for (size_t i=0; i<v.size(); i++)
        {
            o << v[i];
        }

        o << "\"}";
    }

    template<class T>
    static void _t(ostream &o, const optional<T> &v, int indent = -1)
    {
        (void)indent;

        o << noshowpos << "{";

        if (indent >= 0)
        {
            o << endl;
        }

        o << INDENT(1) << "\"O(" << v.has_value() << ")\":";

        if (v.has_value())
        {
            _t(o, static_cast<const T &>(*v),
                  (indent >= 0) ? (indent+1) : -1);
        }
        else
        if (indent >= 0)
        {
            o << "null";
        }

        if (indent >= 0)
        {
            o << endl;
        }

        o << INDENT(0) << "}";
    }

    template<class K, class V>
    static void _t(ostream &o, const map<K,V> &v, int indent = -1)
    {
        (void)indent;

        o << noshowpos << "{";

        if (indent >= 0)
        {
            o << endl;
        }

        size_t len = v.size();

        o << INDENT(1) << "\"M(" << len << ")\":[";

        if (indent >= 0)
        {
            o << endl;
        }

        if (len > 0)
        {
            for (typename map<K,V>::const_iterator ci = v.begin();
                (len > 0) && (ci != v.end());
                ++ci)
            {
                o << INDENT(2) << "{";

                if (indent >= 0)
                {
                    o << endl;
                    o << INDENT(3) << "\"k\": ";
                }

                _t(o, static_cast<const K&>((*ci).first),
                      (indent >= 0) ? (indent+1) : -1);
                o << ",";

                if (indent >= 0)
                {
                    o << endl
                      << INDENT(3)
                      << "\"v\": ";
                }

                _t(o, static_cast<const V&>((*ci).second),
                      (indent >= 0) ? (indent+1) : -1);

                if (indent >= 0)
                {
                    o << endl
                      << INDENT(2);
                }

                o << "}";

                if (indent >= 0)
                {
                    if (len > 1)
                    {
                        o << ",";
                    }

                    o << endl;
                }

                len--;
            }
        }

        o << INDENT(1) << "]";

        if (indent >= 0)
        {
            o << endl;
        }

        o << INDENT(0) << "}";
    }
#endif // !BINARY_ONLY

    static void _t(ostream &o, const void *v, size_t s)
    {
        for (size_t i = 0; i<s; i++)
        {
              o << hex
                << setw(2)
                << setfill('0')
                << static_cast<int>(reinterpret_cast<const char*>(v)[i] & 0xff);
        }

        o << dec << setw(0);
    }

    virtual bool __read(HOB &ref) { (void)ref; return true; }

    virtual bool __read(HOBIO::AbstractReader &is_)
    {
        uint64_t id_;

        _ss.clear();
        _is = NULL;
        _sp = 0;
        _ep = 0;

        if (!is_.read(*this, id_))
        {
            return false;
        }

        size_t sz_ = 0;

        if (has_payload(id_) && !is_.read(*this, sz_))
        {
            return false;
        }

        bool success = true;

        _is = &is_;

        if ((sz_ > 0) && (static_cast<HOBIO::AbstractReader&>(*this).tell() < 0))
        {
            success = false;

            if (_ss.load(is_,sz_))
            {
                _is     = NULL;
                success = true;
            }
        }

        if (success)
        {
            // Now it's mandatory to re-obtain the AbstractReader reference:
            //
            _sp = static_cast<HOBIO::AbstractReader&>(*this).tell();
            _ep = _sp + sz_;
            _id = id_;
        }

        return success;
    }

    virtual size_t get_payload_size() const { return 0; }

    inline const uint64_t& get_id() const
    {
        return _id;
    }

    inline void update_id(const uint64_t &in)
    {
        _np++;

        _id = in;
    }

    inline void update_id(const string &in)
    {
        update_id(in.c_str());
    }

    void update_id(const char *in)
    {
        //
        // in != NULL -> update ID calculation
        //
        if (NULL != in)
        {
            //
            // count HOB parameters
            //
            _np++;

            for (size_t i = 0; in[i]; ++i)
            {
                _id = HSEED * _id + in[i];
            }
        }
        else
        //
        // in == NULL -> finalize ID calculation
        //
        if (_np >= 0)
        {
            // HOBs with    parameters have an odd  ID
            // HOBs without parameters have an even ID

            _id <<= 1;
            _id |= (_np > 0);

            // Avoid parameters check multiple appliance
            //
            _np = -1;
        }
    }

    static inline bool has_payload(uint64_t id)
    {
        // An odd ID means attached payload
        //
        return ( id & 1 );
    }

    virtual void __reset_changes()
    {
        return;
    }

    virtual bool __is_changed() const
    {
        return false;
    }

    virtual void __set_changed(ssize_t f, bool v)
    {
        (void)f;
        (void)v;
    }

    virtual void _s(ostream &o, int indent) const
    {
        (void)indent;

        o << noshowpos << "{";

#if !defined(BINARY_ONLY)
        if (indent >= 0)
        {
            o << endl
              << INDENT(1) << "\"i\": ";
        }

        _t(o, get_id(), (indent >= 0) ? (indent+1) : -1);

        if (indent >= 0)
        {
            o << endl;
        }
#endif // !BINARY_ONLY

        o << "}";
    }

    virtual void flush_pending()
    {
        HOBIO::AbstractReader & s_ = static_cast<HOBIO::AbstractReader&>(*this);

        ssize_t cp = s_.tell();

        if ((cp >= 0) && (cp < _ep))
        {
            s_.ignore(_ep - cp);
        }
    }

private:
    UID                    _id;
    HOBIO::BufferReader    _ss;
    HOBIO::AbstractReader *_is;
    ssize_t                _sp;
    ssize_t                _ep;
    ssize_t                _np;

    static bool parse(HOBIO::AbstractReader &is, HOBIO::AbstractWriter &os, uint8_t t)
    {
        uint8_t c;
        bool group = ('}' == t);
        bool array = (']' == t);
        bool kword = ('"' == t);
        bool skip  = false;

        string token;

        while (is.get(c) && ((t != c) || skip))
        {
            if (kword)
            {
                if (!skip && ('\\' == c))
                {
                    skip = true;
                }
                else
                {
                    skip = false;
                    token += c;
                }
            }
            else
            if (!isspace(c))
            {
                skip = false;

                switch (c)
                {
                    case '{' : parse(is, os, '}'); break;
                    case '[' : parse(is, os, ']'); break;
                    case '"' : parse(is, os, '"'); break;
                    default  :                     break;
                }
            }
        }

        if (!token.empty())
        {
            while (is.get(c))
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
                    else
                    if ("F" == token) // float
                    {
                        dump = true;
                    }
                    else
                    if ("D" == token) // double
                    {
                        dump = true;
                    }
                    else
                    if ("Q" == token) // long double
                    {
                        dump = true;
                    }
                    else
                    if ("B" == token) // bool
                    {
                        dump = true;
                    }
                    else
                    if ("T" == token) // string / char*
                    {
                        dump = true;
                    }
                    else
                    if (token.find("V(") == 0) // vector<>
                    {
                        dump      = true;
                        has_value = false;
                        count_pos = strlen("V(");
                    }
                    else
                    if (token.find("M(") == 0) // map<>
                    {
                        dump      = true;
                        has_value = false;
                        count_pos = strlen("M(");
                    }
                    else
                    if (token.find("O(") == 0) // optional<>
                    {
                        dump        = true;
                        has_value   = false;
                        count_pos   = strlen("O(");
                        is_optional = true;
                    }
                    else
                    if (token.find("B(") == 0) // bitset<> / vector<bool>
                    {
                        dump      = true;
                        has_value = true;
                        count_pos = strlen("B(");
                    }

                    if (dump)
                    {
                        if (has_value)
                        {
                            bool can_be_spaced = false;

                            skip = false;

                            while (is.get(c))
                            {
                                if (can_be_spaced || !isspace(c))
                                {
                                    if (!skip && ('"' == c))
                                    {
                                        can_be_spaced = !can_be_spaced;
                                    }
                                    else
                                    if (!skip && ('\\' == c))
                                    {
                                        skip = true;
                                    }
                                    else
                                    {
                                        skip = false;

                                        if ('}' == c)
                                        {
                                            is.unget(c);
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
                                ASSERT_DUMP(os, v_count);
                            }
                            else
                            {
                                uint64_t v_count = strtoull(count.c_str(),NULL,10);
                                ASSERT_DUMP(os, v_count);
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

                                    if ("C" == token) { ASSERT_DUMP(os, static_cast<int8_t >(v)); }
                                    else
                                    if ("S" == token) { ASSERT_DUMP(os, static_cast<int16_t>(v)); }
                                    else
                                    if ("I" == token) { ASSERT_DUMP(os, static_cast<int32_t>(v)); }
                                    else
                                    if ("L" == token) { ASSERT_DUMP(os, static_cast<int64_t>(v)); }
                                }
                                else
                                {
                                    // unsigned integer values

                                    uint64_t v = strtoull(value.c_str(),NULL,10);

                                    if ("C" == token) { ASSERT_DUMP(os, static_cast<uint8_t >(v)); }
                                    else
                                    if ("S" == token) { ASSERT_DUMP(os, static_cast<uint16_t>(v)); }
                                    else
                                    if ("I" == token) { ASSERT_DUMP(os, static_cast<uint32_t>(v)); }
                                    else
                                    if ("L" == token) { ASSERT_DUMP(os, static_cast<uint64_t>(v)); }
                                }
                            }
                            else
                            if ("F" == token) // float
                            {
                                ASSERT_DUMP(os, hex_to_val<float>(value));
                            }
                            else
                            if ("D" == token) // double
                            {
                                ASSERT_DUMP(os, hex_to_val<double>(value));
                            }
                            else
                            if ("Q" == token) // long double
                            {
                                ASSERT_DUMP(os, hex_to_val<long double>(value));
                            }
                            else
                            if ("T" == token) // string / char*
                            {
                                ASSERT_DUMP(os, value);
                            }
                            else
                            if (("B" == token) ||        // bool
                                (token.find("B(") == 0)) // bitset<> / vector<bool>
                            {
                                if (("true" == value) || ("false" == value))
                                {
                                    bool v_value = ("true" == value);

                                    ASSERT_DUMP(os, v_value);
                                }
                                else
                                {
                                    vector<bool> v_value;

                                    for (ssize_t i=value.size()-1; i>=0; i--)
                                    {
                                        if (('1' == value[i]) || ('0' == value[i]))
                                        {
                                            v_value.push_back(value[i] == '1');
                                        }
                                    }

                                    ASSERT_DUMP(os, v_value, false);
                                }
                            }
                        }
                    }

                    break;
                }
                else
                if (!isspace(c))
                {
                    is.unget(c);

                    break;
                }
            }
        }

        return true;
    }

    static inline uint8_t hex_to_int(const char &c)
    {
        if (c >= '0' && c <= '9') { return c - '0' +  0; }
        if (c >= 'A' && c <= 'F') { return c - 'A' + 10; }
        if (c >= 'a' && c <= 'f') { return c - 'a' + 10; }
        return 0xff;
    }

    template<class T>
    static inline T hex_to_val(const string &s)
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

inline bool operator<<(HOBIO::AbstractWriter &o, HOB &m) { return m >> o; }
inline bool operator>>(HOBIO::AbstractReader &i, HOB &m) { return m << i; }
inline bool operator>>(HOB                   &i, HOB &m) { return m << i; }

inline bool operator>>(HOBIO::AbstractReader &i, HOBIO::AbstractWriter &o)
{
    return HOB::parse(i,o);
}

#define SCAN_FIELDS(m, ...) \
    SCAN_FIELDS_I(m CAT(FOR_EACH_FIELD_0 __VA_ARGS__, _END))

#define FOR_EACH_FIELD_0(...) \
    DEFER1(COMMA)()(__VA_ARGS__) FOR_EACH_FIELD_1

#define FOR_EACH_FIELD_1(...) \
    DEFER1(COMMA)()(__VA_ARGS__) FOR_EACH_FIELD_0

#define FOR_EACH_FIELD_0_END
#define FOR_EACH_FIELD_1_END

#define SCAN_FIELDS_I(...) \
    SCAN_FIELDS_II(__VA_ARGS__)

#define SCAN_FIELDS_II(m, ...) \
    IF(HAS_ARGS(__VA_ARGS__))(EVAL(SCAN_FIELDS_INNER(m, __VA_ARGS__)))

#define SCAN_FIELDS_INNER(op, cur_val, ...) \
    op cur_val                              \
    IF(HAS_ARGS(__VA_ARGS__))(DEFER2(SCAN_FIELDS_INNER_I)()(op, ## __VA_ARGS__))

#define SCAN_FIELDS_INNER_I() SCAN_FIELDS_INNER

#define DECLARE_ENUM(t, n, ...)  _ ## n,
#define DECLARE_FIELD(t, n, ...) t n;
#define INIT_FIELD(t, n, ...)    IF(HAS_ARGS(__VA_ARGS__) )(n = __VA_ARGS__;)
#define READ_FIELD(t, n, ...)    && is_.read(*this, n, _ ## n)
#define WRITE_FIELD(t, n, ...)   && os.write(n)
#define FIELD_SIZE(t, n, ...)    + HOB::__size_of(n)
#define COMPARE_FIELD(t, n, ...) && (n == ref.n)
#define CLONE_FIELD(t, n, ...)   n = ref.n;
#define UPDATE_NP(t, n, ...)     update_id("");
#define UPDATE_ID(t, n, ...)     HUPDATE(STR(name_)); \
                                 HUPDATE(STR(t    )); \
                                 HUPDATE(STR(n    ));

#if !defined(BINARY_ONLY)
#define WTEXT_FIELD(t, n, ...)                                                 \
        if (indent >=0)                                                        \
        {                                                                      \
            o << INDENT(2)                                                     \
              << "{"                                                           \
              << endl                                                          \
              << INDENT(3)                                                     \
              << "\"t\": \"" << STR(t) << "\","                                \
              << endl                                                          \
              << INDENT(3)                                                     \
              << "\"n\": \"" << STR(n) << "\","                                \
              << endl                                                          \
              << INDENT(3)                                                     \
              << "\"v\": ";                                                    \
        }                                                                      \
                                                                               \
        _t(o, static_cast<const t&>(n), (indent >= 0) ? (indent+3): -1);       \
                                                                               \
        if (indent >=0)                                                        \
        {                                                                      \
            o << endl                                                          \
              << INDENT(2)                                                     \
              << "},"                                                          \
              << endl;                                                         \
        }

#define ASCII_DUMP(name_,value_,...)                                           \
        size_t sz = get_payload_size(); /* 0; */                               \
                                                                               \
        o << noshowpos << "{";                                                 \
                                                                               \
        if (indent >= 0)                                                       \
        {                                                                      \
            o << endl                                                          \
              << INDENT(1) << "\"t\": \"" << STR(name_) << "\","               \
              << endl                                                          \
              << INDENT(1) << "\"i\": ";                                       \
        }                                                                      \
                                                                               \
        _t(o, get_id(), (indent >= 0) ? (indent+1) : -1);                      \
                                                                               \
        if (indent >= 0)                                                       \
        {                                                                      \
            o << ","                                                           \
              << endl                                                          \
              << INDENT(1) << "\"s\": \"" << value_ << "\"";                   \
        }                                                                      \
                                                                               \
        if (has_payload(get_id()))                                             \
        {                                                                      \
            o << ",";                                                          \
                                                                               \
            if (indent >= 0)                                                   \
            {                                                                  \
                o << endl << INDENT(1) << "\"b\": ";                           \
            }                                                                  \
                                                                               \
            _t(o, sz, (indent >= 0) ? (indent+1) : -1);                        \
        }                                                                      \
                                                                               \
        if (sz > 0)                                                            \
        {                                                                      \
            if (indent >= 0)                                                   \
            {                                                                  \
                o << "," << endl                                               \
                  << INDENT(1) << "\"f\": ["                                   \
                  << endl;                                                     \
            }                                                                  \
                                                                               \
            SCAN_FIELDS(WTEXT_FIELD, FIRST(__VA_ARGS__))                       \
            SCAN_FIELDS(WTEXT_FIELD, REMAIN(__VA_ARGS__))                      \
                                                                               \
            if (indent >= 0)                                                   \
            {                                                                  \
                o << INDENT(2) << "null"                                       \
                  << endl << INDENT(1) << "]";                                 \
            }                                                                  \
        }                                                                      \
                                                                               \
        if (indent >= 0)                                                       \
        {                                                                      \
           o << endl;                                                          \
        }                                                                      \
                                                                               \
        o << INDENT(0) << "}";
#else // !BINARY_ONLY
#define ASCII_DUMP(n,v,...)                                                    \
        ss << "{}";
#endif // !BINARY_ONLY

#define CHANGED_FIELDS(n, ...) \
    IF(HAS_ARGS(__VA_ARGS__))(EVAL(CHANGED_FIELDS_INNER(n, __VA_ARGS__)))

#define CHANGED_FIELDS_INNER(n, ...)                                           \
    bitset<_FIELDS_COUNT_> __ ## n ## __changed__fields__;                     \

#define RESET_CHANGES(n,f)                                                     \
        if (f == _FIELDS_COUNT_)                                               \
        {                                                                      \
            __ ## n ## __changed__fields__.reset();                            \
        }                                                                      \
        else                                                                   \
        if (f < _FIELDS_COUNT_)                                                \
        {                                                                      \
            __ ## n ## __changed__fields__.reset(f);                           \
        }                                                                      \

#define CHECK_CHANGES(n,f)                                                     \
        if (f == _FIELDS_COUNT_)                                               \
        {                                                                      \
            return __ ## n ## __changed__fields__.any();                       \
        }                                                                      \
        else                                                                   \
        if (f < _FIELDS_COUNT_)                                                \
        {                                                                      \
            return __ ## n ## __changed__fields__.test(f);                     \
        }                                                                      \

#define SET_CHANGED(n,f,v)                                                     \
        if (f < _FIELDS_COUNT_)                                                \
        {                                                                      \
            __ ## n ## __changed__fields__[f] = v;                             \
        }                                                                      \

#define HOBSTRUCT(name_, value_, ...)                                          \
class name_ : public HOB                                                       \
{                                                                              \
public:                                                                        \
    enum Fields                                                                \
    {                                                                          \
        SCAN_FIELDS(DECLARE_ENUM, FIRST(__VA_ARGS__))                          \
        SCAN_FIELDS(DECLARE_ENUM, REMAIN(__VA_ARGS__))                         \
        _FIELDS_COUNT_                                                         \
    };                                                                         \
                                                                               \
    SCAN_FIELDS(DECLARE_FIELD, FIRST(__VA_ARGS__))                             \
    SCAN_FIELDS(DECLARE_FIELD, REMAIN(__VA_ARGS__))                            \
                                                                               \
    name_() : HOB(value_)                                                      \
    {                                                                          \
        SCAN_FIELDS(INIT_FIELD, FIRST(__VA_ARGS__))                            \
        SCAN_FIELDS(INIT_FIELD, REMAIN(__VA_ARGS__))                           \
                                                                               \
        /* ID is evaluated on mandatory fields only */                         \
        SCAN_FIELDS(UPDATE_ID, FIRST(__VA_ARGS__))                             \
                                                                               \
        /* Extra fields update parameters count */                             \
        SCAN_FIELDS(UPDATE_NP, REMAIN(__VA_ARGS__))                            \
                                                                               \
        update_id(static_cast<const char *>(NULL)); /* finalize HOB ID */      \
    }                                                                          \
                                                                               \
    name_(const HOB & ref): HOB(ref)                                           \
    {                                                                          \
        *this = ref;                                                           \
    }                                                                          \
                                                                               \
    name_(const name_ & ref): HOB(ref)                                         \
    {                                                                          \
        *this = ref;                                                           \
    }                                                                          \
                                                                               \
    ~name_()                                                                   \
    {                                                                          \
    }                                                                          \
                                                                               \
    name_ & operator=(const HOB & ref)                                         \
    {                                                                          \
        *static_cast<HOB *>(this) = static_cast<const HOB &>(ref);             \
                                                                               \
        return *this;                                                          \
    }                                                                          \
                                                                               \
    name_ & operator=(const name_ & ref)                                       \
    {                                                                          \
        *static_cast<HOB *>(this) = static_cast<const HOB &>(ref);             \
                                                                               \
        SCAN_FIELDS(CLONE_FIELD, FIRST(__VA_ARGS__))                           \
        SCAN_FIELDS(CLONE_FIELD, REMAIN(__VA_ARGS__))                          \
                                                                               \
        return *this;                                                          \
    }                                                                          \
                                                                               \
    bool operator==(const name_ &ref) const                                    \
    {                                                                          \
        bool rv;                                                               \
                                                                               \
        rv = ((*static_cast<const HOB*>(this) ==                               \
                static_cast<const HOB &>(ref))                                 \
               SCAN_FIELDS(COMPARE_FIELD, FIRST(__VA_ARGS__))                  \
               SCAN_FIELDS(COMPARE_FIELD, REMAIN(__VA_ARGS__)));               \
                                                                               \
        return rv;                                                             \
    }                                                                          \
                                                                               \
    void operator-=(const Fields & f)                                          \
    {                                                                          \
        __reset_changes(f);                                                    \
    }                                                                          \
                                                                               \
    bool operator&(const Fields &f) const                                      \
    {                                                                          \
        return __is_changed(f);                                                \
    }                                                                          \
                                                                               \
    virtual bool operator>>(HOBIO::AbstractWriter &os) const                   \
    {                                                                          \
        if (UNDEFINED == get_id())                                             \
        {                                                                      \
            return true;                                                       \
        }                                                                      \
                                                                               \
        size_t payload = get_payload_size();                                   \
                                                                               \
        os.alloc(__size_of(get_id())                                           \
                 +                                                             \
                 (payload>0) ? __size_of(payload) + payload : 0);              \
                                                                               \
        if (!os.write(get_id()))                                               \
        {                                                                      \
            return false;                                                      \
        }                                                                      \
                                                                               \
        if ((payload > 0) && !os.write(payload))                               \
        {                                                                      \
            return false;                                                      \
        }                                                                      \
                                                                               \
        return (true                                                           \
                SCAN_FIELDS(WRITE_FIELD, FIRST(__VA_ARGS__))                   \
                SCAN_FIELDS(WRITE_FIELD, REMAIN(__VA_ARGS__)));                \
    }                                                                          \
                                                                               \
    inline virtual operator size_t() const                                     \
    {                                                                          \
        size_t payload = get_payload_size();                                   \
                                                                               \
        return (__size_of(get_id())                                            \
                +                                                              \
                ((payload>0) ? __size_of(payload) + payload : 0));             \
    }                                                                          \
                                                                               \
    inline virtual operator bool() const                                       \
    {                                                                          \
        return __is_changed();                                                 \
    }                                                                          \
                                                                               \
protected:                                                                     \
    void __reset_changes(const Fields & f)                                     \
    {                                                                          \
        (void)f;                                                               \
                                                                               \
        IF(HAS_ARGS(__VA_ARGS__))(RESET_CHANGES(name_,f))                      \
    }                                                                          \
                                                                               \
    bool __is_changed(const Fields &f) const                                   \
    {                                                                          \
        (void)f;                                                               \
                                                                               \
        IF(HAS_ARGS(__VA_ARGS__))(CHECK_CHANGES(name_,f))                      \
                                                                               \
        return false;                                                          \
    }                                                                          \
                                                                               \
    virtual void __reset_changes()                                             \
    {                                                                          \
        __reset_changes(_FIELDS_COUNT_);                                       \
    }                                                                          \
                                                                               \
    virtual bool __is_changed() const                                          \
    {                                                                          \
        return __is_changed(_FIELDS_COUNT_);                                   \
    }                                                                          \
                                                                               \
    bool __read(HOB &ref)                                                      \
    {                                                                          \
        __reset_changes();                                                     \
                                                                               \
        ref.rewind();                                                          \
                                                                               \
        if (!__read(static_cast<HOBIO::AbstractReader&>(ref)))                 \
        {                                                                      \
            return false;                                                      \
        }                                                                      \
                                                                               \
        *this = ref;                                                           \
                                                                               \
        return true;                                                           \
    }                                                                          \
                                                                               \
    bool __read(HOBIO::AbstractReader &is_)                                    \
    {                                                                          \
        (void)is_;                                                             \
                                                                               \
        /* Read mandatory fields : fail on error */                            \
                                                                               \
        if (true SCAN_FIELDS(READ_FIELD, FIRST(__VA_ARGS__)))                  \
        {                                                                      \
            /* Read optional fields : ignore errors */                         \
                                                                               \
            if ((true SCAN_FIELDS(READ_FIELD, REMAIN(__VA_ARGS__))) || true)   \
            {                                                                  \
                HOB::flush_pending();                                          \
                                                                               \
                return true;                                                   \
            }                                                                  \
        }                                                                      \
                                                                               \
        return false;                                                          \
    }                                                                          \
                                                                               \
    size_t get_payload_size() const                                            \
    {                                                                          \
        return (0                                                              \
                SCAN_FIELDS(FIELD_SIZE, FIRST(__VA_ARGS__))                    \
                SCAN_FIELDS(FIELD_SIZE, REMAIN(__VA_ARGS__)));                 \
    }                                                                          \
                                                                               \
    virtual void __set_changed(ssize_t f, bool v)                              \
    {                                                                          \
        (void)f;                                                               \
        (void)v;                                                               \
                                                                               \
        if (f < 0)                                                             \
        {                                                                      \
            return;                                                            \
        }                                                                      \
                                                                               \
        IF(HAS_ARGS(__VA_ARGS__))(SET_CHANGED(name_,f,v))                      \
    }                                                                          \
                                                                               \
    virtual void _s(ostream &o, int indent) const                              \
    {                                                                          \
        (void)indent;                                                          \
                                                                               \
        ASCII_DUMP(name_,value_,__VA_ARGS__)                                   \
    }                                                                          \
                                                                               \
    virtual void flush_pending()                                               \
    {                                                                          \
    }                                                                          \
                                                                               \
private:                                                                       \
    CHANGED_FIELDS(name_, __VA_ARGS__)                                         \
};

#endif // __HOB_HPP__
