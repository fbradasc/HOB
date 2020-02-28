#if !defined(__MESSAGE_DECLARATION__)
#define __MESSAGE_DECLARATION__
#include <stdint.h>
#include <vector>
#include <map>
#include <bitset>
#include <string>
#include <sstream>
#include <cstring>
#include <climits>
#include <iostream>
#include <iomanip>
#include "optional.hpp"
#include "cpp_magic.h"

#define M_LOG(...)            printf("%s:%s:%d: ",        \
                                     __FILE__,            \
                                     __PRETTY_FUNCTION__, \
                                     __LINE__);           \
                              printf(__VA_ARGS__);        \
                              printf("\n");

#define ASSERT_SREAD(v,s,f)   deserialize(v,s,f)

#if defined(DEBUG_WRITE)
#define ASSERT_SWRITE(v,s,f)  Message::_w(reinterpret_cast<const char *>(v),s,f)
#else // DEBUG_WRITE
#define ASSERT_SWRITE(v,s,f)  (f.write(reinterpret_cast<const char *>(v),s)\
                                .good())
#endif // DEBUG_WRITE

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

#define ZIGZAG_DECODE(value)  static_cast<uint64_t>(              \
                                  ((value) >> 1) ^ -((value) & 1) \
                              )

#define ZIGZAG_ENCODE(value)  static_cast<uint64_t>(              \
                                  (                               \
                                      static_cast<int64_t>(value) \
                                      <<                          \
                                      1                           \
                                  )                               \
                                  ^                               \
                                  (                               \
                                      static_cast<int64_t>(value) \
                                      >>                          \
                                      ((sizeof(int64_t)*8)-1)     \
                                  )                               \
                              )

// |--------------------------------|
// |0..............................31|
//
// |----------------------------------------------------------------|
// |0..............................................................63|
//
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

class Message
{
public:
    typedef uint64_t UID;

    static const UID UNDEFINED = ULLONG_MAX;

    enum Dump
    {
        JSON,
        TEXT,
    };

    Message()
        : _id(UNDEFINED)
        , _is(     NULL)
        , _sp(        0)
        , _ep(        0)
        , _np(       -1)
    { }

    Message(const UID &id_)
        : _id(   0)
        , _is(NULL)
        , _sp(   0)
        , _ep(   0)
        , _np(  -1)
    {
        update_id(id_);
    }

    Message(const char *id_)
        : _id(   0)
        , _is(NULL)
        , _sp(   0)
        , _ep(   0)
        , _np(  -1)
    {
        update_id(id_);
    }

    Message(const string &id_)
        : _id(   0)
        , _is(NULL)
        , _sp(   0)
        , _ep(   0)
        , _np(  -1)
    {
        update_id(id_);
    }

    Message(const Message &ref)
    {
        *this = ref;
    }

    virtual ~Message()
    {
    }

    Message & operator=(const Message & ref)
    {
        _id = ref._id;
        _ss.str(ref._ss.str()); // TODO: needed ?
        _is = ref._is;
        _sp = ref._sp;
        _ep = ref._ep;

        return *this;
    }

    bool operator<<(istream &is_)
    {
        flush_pending();

        return _r(is_);
    }

    bool operator<<(Message & ref)
    {
        return ((_id == ref._id) && _r(ref));
    }

    bool operator>>(ostream &os) const
    {
        if (UNDEFINED == _id)
        {
            return true;
        }

        if (!_w(os, _id))
        {
            return false;
        }

#if 0
        std::ostringstream ss;

        if (!_w((has_payload(_id)) ? ss : os))
        {
            return false;
        }

        if (has_payload(_id) && !_w(os, ss.str()))
        {
            return false;
        }
#else
        if (has_payload(_id))
        {
            std::ostringstream ss;

            if (!_w(ss) || !_w(os, ss.str()))
            {
                return false;
            }
        }
#endif

        return true;
    }

    bool operator==(const Message &ref) const
    {
        bool rv = (_id == ref._id);

        return rv;
    }

    bool operator!=(const Message &ref) const
    {
        bool rv = (_id != ref._id);

        return rv;
    }

    operator size_t() const
    {
        return ( _ep - _sp );
    }

    virtual bool rewind()
    {
        istream & s_ = static_cast<istream&>(*this);

        s_.clear();

        return s_.seekg(_sp,s_.beg).good();
    }

    operator istream &() { return (NULL != _is) ? *_is : _ss; }

    operator bool() const
    {
        return is_changed();
    }

    void operator~()
    {
        reset_changes();
    }

    const string operator()(const Dump &dump = TEXT) const
    {
        return _s((JSON == dump) ? 0 : -1);
    }

    class Inporter: public istream
    {
    public:
        Inporter(istream & is)
            : istream(NULL)
            , _sb(is)
        {
            init(&_sb);
        }

    private:
        class Streambuf: public std::streambuf
        {
        public:
            Streambuf(istream & is)
                : m_is(is)
            {
            };

            virtual ~Streambuf() {};

        protected:
            virtual int underflow()
            {
                // If something is left in the get area by chance, return it
                // (this shouldn't normally happen, as underflow is only supposed
                // to be called when gptr >= egptr, but it serves as error check)
                //
                if (this->gptr() && (this->gptr() < this->egptr()))
                {
                    return traits_type::to_int_type(*(this->gptr()));
                }

                // If the file hasn't been opened for reading, produce error
                if (!m_is.good())
                {
                    return traits_type::eof();
                }

                m_inbuf.str("");

                parse('\0');

                char* gbeg  = const_cast<char_type*>(m_inbuf.str().data());
                char* gnext = gbeg;
                char* gend  = gbeg + m_inbuf.str().size();

                this->setg(gbeg, gnext, gend);

                if (gbeg == gend)
                {
                    // Indicates error or EOF
                    //
                    return traits_type::eof();
                }

                // Return next character in get area
                //
                return traits_type::to_int_type(*(this->gptr()));
            }

        private:
            istream         &m_is;
            stringstream     m_inbuf;

#define ASSERT_DUMP(...) if (!Message::_w(m_inbuf, __VA_ARGS__)) { return false; }

            bool parse(char t)
            {
                char c;
                bool group = ('}' == t);
                bool array = (']' == t);
                bool kword = ('"' == t);
                bool skip  = false;

                string token;

                while (m_is.get(c).good() && ((t != c) || skip))
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
                            case '{' : parse('}'); break;
                            case '[' : parse(']'); break;
                            case '"' : parse('"'); break;
                            default  :                 break;
                        }
                    }
                }

                if (!token.empty())
                {
                    while (m_is.get(c).good())
                    {
                        if (':' == c)
                        {
                            bool   dump      = false;
                            string value     = "";
                            bool   has_value = true;
                            size_t count_pos = 0;

                            if ("U" == token)
                            {
                                dump = true;
                            }
                            else
                            if ("S" == token)
                            {
                                dump = true;
                            }
                            else
                            if ("F" == token)
                            {
                                dump = true;
                            }
                            else
                            if ("D" == token)
                            {
                                dump = true;
                            }
                            else
                            if ("Q" == token)
                            {
                                dump = true;
                            }
                            else
                            if ("B" == token)
                            {
                                dump = true;
                            }
                            else
                            if ("T" == token)
                            {
                                dump = true;
                            }
                            else
                            if (token.find("V(") == 0)
                            {
                                dump      = true;
                                has_value = false;
                                count_pos = strlen("V(");
                            }
                            else
                            if (token.find("M(") == 0)
                            {
                                dump      = true;
                                has_value = false;
                                count_pos = strlen("M(");
                            }
                            else
                            if (token.find("O(") == 0)
                            {
                                dump      = true;
                                has_value = false;
                                count_pos = strlen("O(");
                            }
                            else
                            if (token.find("B(") == 0)
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

                                    while (m_is.get(c).good())
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
                                                    m_is.unget();
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

                                    uint64_t v_count = stoull(count);

                                    ASSERT_DUMP(v_count);
                                }

                                if (has_value)
                                {
                                    if ("U" == token)
                                    {
                                        uint64_t v_value = stoull(value);

                                        ASSERT_DUMP(v_value);
                                    }
                                    else
                                    if ("S" == token)
                                    {
                                        int64_t v_value = stoll(value);

                                        ASSERT_DUMP(v_value);
                                    }
                                    else
                                    if ("F" == token)
                                    {
                                        ASSERT_DUMP(hex_to_val<float>(value));
                                    }
                                    else
                                    if ("D" == token)
                                    {
                                        ASSERT_DUMP(hex_to_val<double>(value));
                                    }
                                    else
                                    if ("Q" == token)
                                    {
                                        ASSERT_DUMP(hex_to_val<long double>(value));
                                    }
                                    else
                                    if ("T" == token)
                                    {
                                        ASSERT_DUMP(value);
                                    }
                                    else
                                    if (("B" == token) || (token.find("B(") == 0))
                                    {
                                        if (("true" == value) || ("false" == value))
                                        {
                                            bool v_value = ("true" == value);

                                            ASSERT_DUMP(v_value);
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

                                            ASSERT_DUMP(v_value, false);
                                        }
                                    }
                                }
                            }

                            break;
                        }
                        else
                        if (!isspace(c))
                        {
                            m_is.unget();

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

        Streambuf _sb;
    };

protected:
    //========================================================================
    //
    // Variable integer (VARINT) are packed in 1 to 9 bytes
    //
    // The MSb bits in the MSB byte identify how many bytes are required
    // to store the numeric value.
    //
    // x: single bit
    // X: single byte (8 bits)
    //
    // 1 byte  [numeric value encoded in  7 bits]: 0xxxxxxx
    // 2 bytes [numeric value encoded in 14 bits]: 10xxxxxx.X
    // 3 bytes [numeric value encoded in 21 bits]: 110xxxxx.X.X
    // 4 bytes [numeric value encoded in 28 bits]: 1110xxxx.X.X.X
    // 5 bytes [numeric value encoded in 35 bits]: 11110xxx.X.X.X.X
    // 6 bytes [numeric value encoded in 42 bits]: 111110xx.X.X.X.X.X
    // 7 bytes [numeric value encoded in 49 bits]: 1111110x.X.X.X.X.X.X
    // 8 bytes [numeric value encoded in 56 bits]: 11111110.X.X.X.X.X.X.X
    // 9 bytes [numeric value encoded in 64 bits]: 11111111.X.X.X.X.X.X.X.X
    //
    // Signed Integer values are zigzag encoded by mapping negative values
    // to positive values while going back and forth:
    //
    // (0=0, -1=1, 1=2, -2=3, 2=4, -3=5, 3=6 ...)
    //
    //========================================================================

    static bool _w(ostream &os, const uint8_t &v)
    {
        return _w(os, static_cast<const uint64_t &>(v));
    }

    static bool _w(ostream &os, const uint16_t &v)
    {
        return _w(os, static_cast<const uint64_t &>(v));
    }

    static bool _w(ostream &os, const uint32_t &v)
    {
        return _w(os, static_cast<const uint64_t &>(v));
    }

    // VARINT packing
    //
    static bool _w(ostream &os, const uint64_t &v)
    {
        uint8_t d[9];

        uint8_t b = (v <= 0x000000000000007fllu) ? 1 :
                    (v <= 0x0000000000003fffllu) ? 2 :
                    (v <= 0x00000000001fffffllu) ? 3 :
                    (v <= 0x000000000fffffffllu) ? 4 :
                    (v <= 0x00000007ffffffffllu) ? 5 :
                    (v <= 0x000003ffffffffffllu) ? 6 :
                    (v <= 0x0001ffffffffffffllu) ? 7 :
                    (v <= 0x00ffffffffffffffllu) ? 8 : 9;

        uint8_t  m = 0xff;
        uint32_t c = 0xfffffe00;

        for (uint8_t i=0; i<b; i++)
        {
            d[b-i-1] = static_cast<uint8_t>( ( v >> (8*i) ) & 0xff );

            c >>= 1;
            m >>= 1;
        }

        d[0] &= static_cast<uint8_t>(m & 0xff);
        d[0] |= static_cast<uint8_t>(c & 0xff);

        return ASSERT_SWRITE(d, b, os);
    }

    static bool _w(ostream &os, const int8_t &v)
    {
        return _w(os, static_cast<int64_t>(v));
    }

    static bool _w(ostream &os, const int16_t &v)
    {
        return _w(os, static_cast<int64_t>(v));
    }

    static bool _w(ostream &os, const int32_t &v)
    {
        return _w(os, static_cast<int64_t>(v));
    }

    // ZIGZAG encoding
    //
    static bool _w(ostream &os, const int64_t &v)
    {
        return _w(os, ZIGZAG_ENCODE(v));
    }

    static bool _w(ostream &os, const bool &v)
    {
        return ASSERT_SWRITE(&v, sizeof(v), os);
    }

    static bool _w(ostream &os, const float &v)
    {
        return ASSERT_SWRITE(&v, sizeof(v), os);
    }

    static bool _w(ostream &os, const double &v)
    {
        return ASSERT_SWRITE(&v, sizeof(v), os);
    }

    static bool _w(ostream &os, const long double &v)
    {
        return ASSERT_SWRITE(&v, sizeof(v), os);
    }

    static bool _w(ostream &os, const string &v)
    {
        if (!_w(os, v.size()))
        {
            return false;
        }

        return ASSERT_SWRITE(v.data(), v.size(), os);
    }

    static bool _w(ostream &os, uint8_t size_, const void *v)
    {
        if (!_w(os, size_))
        {
            return false;
        }

        return ASSERT_SWRITE(v, size_, os);
    }

    static bool _w(ostream &os, const Message &v)
    {
        return (v >> os);
    }

    template<size_t N>
    static bool _w(ostream &os, const std::bitset<N>& v)
    {
        uint8_t bits[(N+7)>>3];

        memset(bits, 0, ((N+7)>>3));

        for (size_t j=0; j<size_t(N); j++)
        {
            bits[j>>3] |= (v[j]<<(j&7));
        }

        return _w(os, ((N+7)>>3), bits);
    }

    template<class T>
    static bool _w(ostream &os, const vector<T> &v)
    {
        if (!_w(os, v.size()))
        {
            return false;
        }

        for (size_t i=0; i<v.size(); i++)
        {
            if (!_w(os, static_cast<const T &>(v[i])))
            {
                return false;
            }
        }

        return true;
    }

    static bool _w(ostream &os, const vector<bool> &v, bool dump_size=true)
    {
        if (dump_size && !_w(os, v.size()))
        {
            return false;
        }

        uint8_t bits[(v.size()+7)>>3];

        memset(bits, 0, ((v.size()+7)>>3));

        for (size_t j=0; j<v.size(); j++)
        {
            bits[j>>3] |= (v[j]<<(j&7));
        }

        return _w(os, ((v.size()+7)>>3), bits);
    } 

    template<class T>
    static bool _w(ostream &os, const optional<T> &v)
    {
        if (!_w(os, static_cast<bool>(v)))
        {
            return false;
        }

        if (static_cast<bool>(v))
        {
            if (!_w(os, static_cast<const T &>(*v)))
            {
                return false;
            }
        }

        return true;
    }

    template<class K, class V>
    static bool _w(ostream &os, const map<K,V> &v)
    {
        size_t len = v.size();

        if (!_w(os, len))
        {
            return false;
        }

        for (typename map<K,V>::const_iterator ci = v.begin();
             (len > 0) && (ci != v.end());
             ++ci)
        {
            if (!_w(os, static_cast<const K &>((*ci).first)))
            {
                return false;
            }

            if (!_w(os, static_cast<const V &>((*ci).second)))
            {
                return false;
            }

            len--;
        }

        return true;
    }

    bool _r(istream &is_, uint8_t &v, ssize_t field=-1)
    {
        uint64_t r;

        if (!_r(is_, r))
        {
            return false;
        }

        uint8_t rv = static_cast<uint8_t>(r & 0xff);

        set_changed(field, v != rv);

        v = rv;

        return true;
    }

    bool _r(istream &is_, uint16_t &v, ssize_t field=-1)
    {
        uint64_t r;

        if (!_r(is_, r))
        {
            return false;
        }

        uint16_t rv = static_cast<uint16_t>(r & 0xffff);

        set_changed(field, v != rv);

        v = rv;

        return true;
    }

    bool _r(istream &is_, uint32_t &v, ssize_t field=-1)
    {
        uint64_t r;

        if (!_r(is_, r))
        {
            return false;
        }

        uint32_t rv = static_cast<uint32_t>(r & 0xffffffff);

        set_changed(field, v != rv);

        v = rv;

        return true;
    }

    // VARINT unpacking
    //
    bool _r(istream &is_, uint64_t &v, ssize_t field=-1)
    {
        if (is_.eof())
        {
            return false;
        }

        uint8_t d[9];
        uint8_t b = 0;
        uint8_t c;
        uint8_t m;

        if (!ASSERT_SREAD(&d[0], sizeof(uint8_t), is_))
        {
            return false;
        }

        for (b=1; b<=8; b++)
        {
            m = (0xff << (8-b));
            c = (0x01 << (8-b));

            if ((d[0] & c) == 0)
            {
                break;
            }
        }

        if (b > 1)
        {
            if (!ASSERT_SREAD(&d[1], b-1, is_))
            {
                return false;
            }
        }

        uint64_t rv = (b < 9) ? d[0] & ~m : 0;

        for (uint8_t i=1; i<b; i++)
        {
            rv <<= 8;
            rv |= static_cast<uint64_t>(d[i] & 0xff);
        }

        set_changed(field, v != rv);

        v = rv;

        return true;
    }

    bool _r(istream &is_, int8_t &v, ssize_t field=-1)
    {
        return _z<int8_t>(is_,v,field);
    }

    bool _r(istream &is_, int16_t &v, ssize_t field=-1)
    {
        return _z<int16_t>(is_,v,field);
    }

    bool _r(istream &is_, int32_t &v, ssize_t field=-1)
    {
        return _z<int32_t>(is_,v,field);
    }

    bool _r(istream &is_, int64_t &v, ssize_t field=-1)
    {
        return _z<int64_t>(is_,v,field);
    }

    bool _r(istream &is_, bool &v, ssize_t field=-1)
    {
        bool rv;

        if (!ASSERT_SREAD(&rv, sizeof(v), is_))
        {
            return false;
        }

        set_changed(field, v != rv);

        v = rv;

        return true;
    }

    bool _r(istream &is_, float &v, ssize_t field=-1)
    {
        float rv;

        if (!ASSERT_SREAD(&rv, sizeof(v), is_))
        {
            return false;
        }

        set_changed(field, v != rv);

        v = rv;

        return true;
    }

    bool _r(istream &is_, double &v, ssize_t field=-1)
    {
        double rv;

        if (!ASSERT_SREAD(&rv, sizeof(v), is_))
        {
            return false;
        }

        set_changed(field, v != rv);

        v = rv;

        return true;
    }

    bool _r(istream &is_, long double &v, ssize_t field=-1)
    {
        long double rv;

        if (!ASSERT_SREAD(&rv, sizeof(v), is_))
        {
            return false;
        }

        set_changed(field, v != rv);

        v = rv;

        return true;
    }

    bool _r(istream &is_, string &v, ssize_t field=-1)
    {
        uint64_t len;

        if (!_r(is_, len))
        {
            return false;
        }

        vector<char> tmp(len);

        if (!ASSERT_SREAD(tmp.data(), len, is_))
        {
            return false;
        }

        string rv;

        rv.assign(tmp.data(),len);

        set_changed(field, v != rv);

        v = rv;

        return true;
    }

    bool _r(istream &is_, Message &v, ssize_t field=-1)
    {
        if (!is_.eof())
        {
            Message m;

            if (m << is_)
            {
                v = m;

                if ( v << static_cast<istream&>(m) )
                {
                    set_changed(field, v);

                    return true;
                }
            }
        }

        return false;
    }

    template<size_t N>
    bool _r(istream &is_, bitset<N> &v, ssize_t field=-1)
    {
        size_t len;

        if (!_r(is_, len) || (len != ((N + 7) >> 3)))
        {
            return false;
        }

        uint8_t bits[len];

        memset(bits, 0, len);

        if (!ASSERT_SREAD(bits, len, is_))
        {
            return false;
        }

        bitset<N> rv;

        for (size_t j=0; j<size_t(N); j++)
        {
            rv[j] = ((bits[j>>3]>>(j&7)) & 1);
        }

        set_changed(field, v != rv);

        v = rv;

        return true;
    }

    template<class T>
    bool _r(istream &is_, vector<T> &v, ssize_t field=-1)
    {
        size_t len;

        if (!_r(is_, len))
        {
            return false;
        }

        if (len > 0)
        {
            vector<T> tmp(len);

            for (size_t i=0; i<len; i++)
            {
                if (!_r(is_, static_cast<T&>(tmp[i])))
                {
                    return false;
                }
            }

            set_changed(field, v != tmp);

            v = tmp;
        }
        else
        {
            set_changed(field, !v.empty());

            v = vector<T>();
        }

        return true;
    }

    bool _r(istream &is_, vector<bool> &v, ssize_t field=-1)
    {
        size_t count;

        if (!_r(is_, count))
        {
            return false;
        }

        if (count > 0)
        {
            size_t len;

            if (!_r(is_, len) || (len != ((count + 7) >> 3)))
            {
                return false;
            }

            uint8_t bits[len];

            memset(bits, 0, len);

            if (!ASSERT_SREAD(bits, len, is_))
            {
                return false;
            }

            vector<bool> rv;

            for (size_t j=0; j<count; j++)
            {
                rv.push_back((bits[j>>3]>>(j&7)) & 1);
            }

            set_changed(field, v != rv);

            v = rv;

            return true;
        }
        else
        {
            set_changed(field, !v.empty());

            v = vector<bool>();
        }

        return true;
    }

    template<class T>
    bool _r(istream &is_, optional<T> &v, ssize_t field=-1)
    {
        bool has_field;

        if (!_r(is_, has_field))
        {
            return false;
        }

        if (has_field)
        {
            T tmp;

            if (!_r(is_, static_cast<T&>(tmp)))
            {
                return false;
            }

            set_changed(field, v != tmp);

            v = tmp;
        }
        else
        {
            set_changed(field, v.has_value());

            v = optional<T>();
        }

        return true;
    }

    template<class K, class V>
    bool _r(istream &is_, map<K,V> &v_, ssize_t field=-1)
    {
        size_t len;

        if (!_r(is_, len))
        {
            return false;
        }

        if (len > 0)
        {
            map<K,V> tmp;

            for (size_t i=0; i<len; i++)
            {
                K k;
                V v;

                if (!_r(is_, static_cast<K&>(k)))
                {
                    return false;
                }

                if (!_r(is_, static_cast<V&>(v)))
                {
                    return false;
                }

                tmp[k] = v;
            }

            set_changed(field, v_ != tmp);

            v_ = tmp;
        }
        else
        {
            set_changed(field, !v_.empty());

            v_ = map<K,V>();
        }

        return true;
    }

#if !defined(BINARY_ONLY)
    static string _t(const uint8_t &v, int indent = -1)
    {
        (void)indent;

        stringstream o;
        o << "{\"U\":" << static_cast<int>(v) << "}";
        return(o.str());
    }

    static string _t(const uint16_t &v, int indent = -1)
    {
        (void)indent;

        stringstream o;
        o << "{\"U\":" << v << "}";
        return(o.str());
    }

    static string _t(const uint32_t &v, int indent = -1)
    {
        (void)indent;

        stringstream o;
        o << "{\"U\":" << v << "}";
        return(o.str());
    }

    static string _t(const uint64_t &v, int indent = -1)
    {
        (void)indent;

        stringstream o;
        o << "{\"U\":" << v << "}";
        return(o.str());
    }

    static string _t(const int8_t &v, int indent = -1)
    {
        (void)indent;

        stringstream o;
        o << "{\"S\":" << static_cast<int>(v) << "}";
        return(o.str());
    }

    static string _t(const int16_t &v, int indent = -1)
    {
        (void)indent;

        stringstream o;
        o << "{\"S\":" << v << "}";
        return(o.str());
    }

    static string _t(const int32_t &v, int indent = -1)
    {
        (void)indent;

        stringstream o;
        o << "{\"S\":" << v << "}";
        return(o.str());
    }

    static string _t(const int64_t &v, int indent = -1)
    {
        (void)indent;

        stringstream o;
        o << "{\"S\":" << v << "}";
        return(o.str());
    }

    static string _t(const float &v, int indent = -1)
    {
        (void)indent;

        stringstream o;

        o << "{\"F\":\"";
        
        _t(reinterpret_cast<const char *>(&v), sizeof(v), o);
        
        o << "\"}";

        return(o.str());
    }

    static string _t(const double &v, int indent = -1)
    {
        (void)indent;

        stringstream o;

        o << "{\"D\":\"";
        
        _t(reinterpret_cast<const char *>(&v), sizeof(v), o);
        
        o << "\"}";

        return(o.str());
    }

    static string _t(const long double &v, int indent = -1)
    {
        (void)indent;

        stringstream o;

        o << "{\"Q\":\"";
        
        _t(reinterpret_cast<const char *>(&v), sizeof(v), o);
        
        o << "\"}";

        return(o.str());
    }

    static string _t(const char *v, int indent = -1)
    {
        (void)indent;

        stringstream o;
        o << "{\"T\":\"" << v << "\"}";
        return(o.str());
    }

    static string _t(const string &v, int indent = -1)
    {
        (void)indent;

        stringstream o;
        o << "{\"T\":\"" << v << "\"}";
        return(o.str());
    }

    static string _t(const bool &v, int indent = -1)
    {
        (void)indent;

        return( v ? "{\"B\":true}" : "{\"B\":false}");
    }

    static string _t(const Message &v, int indent = -1)
    {
        return v._s(indent);
    }

    template<size_t N>
    static string _t(const bitset<N> &v, int indent = -1)
    {
        (void)indent;

        stringstream o;
        o << "{\"B\":\"" << v << "\"}";
        return(o.str());
    }

    template<class T>
    static string _t(const vector<T> &v, int indent = -1)
    {
        (void)indent;

        stringstream o;

        o << "{";

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
                o << INDENT(2) << _t(static_cast<const T &>(v[i]),
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

        return(o.str());
    }

    static string _t(const vector<bool> &v, int indent = -1)
    {
        (void)indent;

        stringstream o;

        o << "{\"B(" << v.size() << ")\":\"";

        for (size_t i=0; i<v.size(); i++)
        {
            o << v[i];
        }

        o << "\"}";

        return(o.str());
    }

    template<class T>
    static string _t(const optional<T> &v, int indent = -1)
    {
        (void)indent;

        stringstream o;

        o << "{";

        if (indent >= 0)
        {
            o << endl;
        }

        o << INDENT(1) << "\"O(" << v.has_value() << ")\":";

        if (v.has_value())
        {
            o << _t(static_cast<const T &>(*v),
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

        return(o.str());
    }

    template<class K, class V>
    static string _t(const map<K,V> &v, int indent = -1)
    {
        (void)indent;

        stringstream o;

        o << "{";

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

                o << _t(static_cast<const K&>((*ci).first),
                        (indent >= 0) ? (indent+1) : -1);
                o << ",";

                if (indent >= 0)
                {
                    o << endl
                      << INDENT(3)
                      << "\"v\": ";
                }

                o << _t(static_cast<const V&>((*ci).second),
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

        return(o.str());
    }
#endif // !BINARY_ONLY

    static void _t(const char *v, size_t s, ostream &o)
    {
        for (size_t i = 0; i<s; i++)
        {
            o << hex
              << setw(2)
              << setfill('0')
              << static_cast<int>(v[i] & 0xff);
        }
    }

    virtual bool _r(Message &ref) { (void)ref; return true; }

    virtual bool _r(istream &is_)
    {
        uint64_t id_;

        _ss.str("");
        _is = NULL;
        _sp = 0;
        _ep = 0;

        if (!_r(is_, id_))
        {
            return false;
        }

        size_t sz_ = 0;

        if (has_payload(id_) && !_r(is_, sz_))
        {
            return false;
        }

        bool success = true;

        _is = &is_;

        if ((sz_ > 0) && (static_cast<istream&>(*this).tellg() < 0))
        {
            success = false;

            char *buffer = new char[sz_];

            if (NULL != buffer)
            {
                if (deserialize(buffer,sz_,is_))
                {
                    if (_ss.write(buffer,sz_).good())
                    {
                        _is     = NULL;
                        success = true;
                    }
                }

                delete[] buffer;
            }
        }

        if (success)
        {
            _sp = static_cast<istream&>(*this).tellg();
            _ep = _sp + sz_;
            _id = id_;
        }

        return success;
    }

    virtual bool _w(ostream &os) const { (void)os; return true; }

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
            // count message parameters
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
            // Messages with    parameters have an odd  ID
            // Messages without parameters have an even ID

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

    virtual void reset_changes()
    {
        return;
    }

    virtual bool is_changed() const
    {
        return false;
    }

    virtual void set_changed(ssize_t f, bool v)
    {
        (void)f;
        (void)v;
    }

    virtual const string _s(int indent) const
    {
        (void)indent;

#if !defined(BINARY_ONLY)
        stringstream ss;

        ss << "{";

        if (indent >= 0)
        {
            ss << endl
               << INDENT(1) << "\"i\": ";
        }

        ss << _t(get_id(), (indent >= 0) ? (indent+1) : -1);

        if (indent >= 0)
        {
            ss << endl;
        }

        ss << "}";

        return ss.str();
#else // !BINARY_ONLY
        return "{}";
#endif // !BINARY_ONLY
    }

    virtual void flush_pending()
    {
        istream & s_ = static_cast<istream&>(*this);

        size_t cp = s_.tellg();

        if (cp < _ep)
        {
            s_.ignore(_ep - cp);
        }
    }

private:
    UID           _id;
    stringstream  _ss;
    istream      *_is;
    size_t        _sp;
    size_t        _ep;
    ssize_t       _np;

    // ZIGZAG decoding
    //
    template<class T>
    bool _z(istream &is_, T &v, ssize_t field=-1)
    {
        uint64_t r;

        if (!_r(is_, r))
        {
            return false;
        }

        T rv = static_cast<T>(ZIGZAG_DECODE(r) & (static_cast<const T>(-1)));

        set_changed(field, v != rv);

        v = rv;

        return true;
    }

    bool deserialize(void *v, size_t s, istream &is_)
    {
        // Check whether **only** an internal logic error occurred ...
        //
        if (is_.fail() && !is_.bad() && !is_.eof())
        {
            // ... and ignore it
            //
            is_.clear();
        }

        // Fail in case of any other error
        //
        return is_.good() && is_.read(static_cast<char *>(v),s).good();
    }

#if defined(DEBUG_WRITE)
    static bool _w(const char *v, const size_t s, ostream &os)
    {
        if (os.write(v,s).good())
        {
#if !defined(DUMP_ALL)
            if (dynamic_cast<ofstream*>(&os) != NULL)
#endif // DUMP_ALL
            {
                cout << "DEBUG_WRITE:";

                _t(v,s,cout);
                /*
                for (size_t i = 0; i<s; i++)
                {
                    cout << hex
                         << setw(2)
                         << setfill('0')
                         << static_cast<int>(v[i] & 0xff);
                }
                */

                cout << endl;
            }

            return true;
        }

        return false;
    }
#endif // DEBUG_WRITE
};

bool operator<<(ostream &is, Message &m) { return m >> is; } 
bool operator>>(istream &is, Message &m) { return m << is; } 
bool operator>>(Message &im, Message &m) { return m << im; }

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
#define READ_FIELD(t, n, ...)    && Message::_r(is_, n, _ ## n)
#define WRITE_FIELD(t, n, ...)   && Message::_w(os, n)
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
            ss << INDENT(2)                                                    \
               << "{"                                                          \
               << endl                                                         \
        /*     << INDENT(3)                                                    \
               << "\"t\": \"" << STR(t) << "\","                               \
               << endl                                                         \
        */     << INDENT(3)                                                    \
               << "\"n\": \"" << STR(n) << "\","                               \
               << endl                                                         \
               << INDENT(3)                                                    \
               << "\"v\": ";                                                   \
        }                                                                      \
                                                                               \
        ss << _t(static_cast<const t&>(n), (indent >= 0) ? (indent+3): -1);    \
                                                                               \
        if (indent >=0)                                                        \
        {                                                                      \
            ss << endl                                                         \
               << INDENT(2)                                                    \
               << "},"                                                         \
               << endl;                                                        \
        }

#define ASCII_DUMP(name_,value_,...)                                           \
        stringstream ss;                                                       \
        size_t sz = 0;                                                         \
                                                                               \
        if (has_payload(get_id()) && _w(ss))                                   \
        {                                                                      \
            sz = ss.str().size();                                              \
        }                                                                      \
                                                                               \
        ss.str("");                                                            \
                                                                               \
        ss << "{";                                                             \
                                                                               \
        if (indent >= 0)                                                       \
        {                                                                      \
            ss << endl                                                         \
               << INDENT(1) << "\"t\": \"" << STR(name_) << "\","              \
               << endl                                                         \
               << INDENT(1) << "\"i\": ";                                      \
        }                                                                      \
                                                                               \
        ss << _t(get_id(), (indent >= 0) ? (indent+1) : -1);                   \
                                                                               \
        if (indent >= 0)                                                       \
        {                                                                      \
            ss << ","                                                          \
               << endl                                                         \
               << INDENT(1) << "\"s\": \"" << value_ << "\","                  \
               << endl;                                                        \
        }                                                                      \
                                                                               \
        if (has_payload(get_id()))                                             \
        {                                                                      \
            if (indent >= 0)                                                   \
            {                                                                  \
                ss << INDENT(1) << "\"b\": ";                                  \
            }                                                                  \
                                                                               \
            ss << _t(sz, (indent >= 0) ? (indent+1) : -1);                     \
        }                                                                      \
                                                                               \
        if (sz > 0)                                                            \
        {                                                                      \
            if (indent >= 0)                                                   \
            {                                                                  \
                ss << "," << endl                                              \
                   << INDENT(1) << "\"f\": ["                                  \
                   << endl;                                                    \
            }                                                                  \
                                                                               \
            SCAN_FIELDS(WTEXT_FIELD, FIRST(__VA_ARGS__))                       \
            SCAN_FIELDS(WTEXT_FIELD, REMAIN(__VA_ARGS__))                      \
                                                                               \
            if (indent >= 0)                                                   \
            {                                                                  \
                ss << INDENT(2) << "null"                                      \
                   << endl << INDENT(1) << "]";                                \
            }                                                                  \
        }                                                                      \
                                                                               \
        if (indent >= 0)                                                       \
        {                                                                      \
           ss << endl;                                                         \
        }                                                                      \
                                                                               \
        ss << INDENT(0) << "}";                                                \
                                                                               \
        return ss.str();
#else // !BINARY_ONLY
#define ASCII_DUMP(n,v,...)                                                    \
        return "{}";
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

#define DECLARE_MESSAGE(name_, value_, ...)                                    \
class name_ : public Message                                                   \
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
    name_() : Message(value_)                                                  \
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
        update_id(static_cast<const char *>(NULL)); /* finalize message ID */  \
    }                                                                          \
                                                                               \
    name_(const Message & ref): Message(ref)                                   \
    {                                                                          \
        *this = ref;                                                           \
    }                                                                          \
                                                                               \
    name_(const name_ & ref): Message(ref)                                     \
    {                                                                          \
        *this = ref;                                                           \
    }                                                                          \
                                                                               \
    ~name_()                                                                   \
    {                                                                          \
    }                                                                          \
                                                                               \
    name_ & operator=(const Message & ref)                                     \
    {                                                                          \
        *static_cast<Message *>(this) = static_cast<const Message &>(ref);     \
                                                                               \
        return *this;                                                          \
    }                                                                          \
                                                                               \
    name_ & operator=(const name_ & ref)                                       \
    {                                                                          \
        *static_cast<Message *>(this) = static_cast<const Message &>(ref);     \
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
        rv = ((*static_cast<const Message*>(this) ==                           \
                static_cast<const Message &>(ref))                             \
               SCAN_FIELDS(COMPARE_FIELD, FIRST(__VA_ARGS__))                  \
               SCAN_FIELDS(COMPARE_FIELD, REMAIN(__VA_ARGS__)));               \
                                                                               \
        return rv;                                                             \
    }                                                                          \
                                                                               \
    void operator-=(const Fields & f)                                          \
    {                                                                          \
        reset_changes(f);                                                      \
    }                                                                          \
                                                                               \
    bool operator&(const Fields &f) const                                      \
    {                                                                          \
        return is_changed(f);                                                  \
    }                                                                          \
                                                                               \
protected:                                                                     \
    void reset_changes(const Fields & f)                                       \
    {                                                                          \
        (void)f;                                                               \
                                                                               \
        IF(HAS_ARGS(__VA_ARGS__))(RESET_CHANGES(name_,f))                      \
    }                                                                          \
                                                                               \
    bool is_changed(const Fields &f) const                                     \
    {                                                                          \
        (void)f;                                                               \
                                                                               \
        IF(HAS_ARGS(__VA_ARGS__))(CHECK_CHANGES(name_,f))                      \
                                                                               \
        return false;                                                          \
    }                                                                          \
                                                                               \
    virtual void reset_changes()                                               \
    {                                                                          \
        reset_changes(_FIELDS_COUNT_);                                         \
    }                                                                          \
                                                                               \
    virtual bool is_changed() const                                            \
    {                                                                          \
        return is_changed(_FIELDS_COUNT_);                                     \
    }                                                                          \
                                                                               \
    bool _r(Message &ref)                                                      \
    {                                                                          \
        reset_changes();                                                       \
                                                                               \
        ref.rewind();                                                          \
                                                                               \
        if (!_r(static_cast<istream&>(ref)))                                   \
        {                                                                      \
            return false;                                                      \
        }                                                                      \
                                                                               \
        *this = ref;                                                           \
                                                                               \
        return true;                                                           \
    }                                                                          \
                                                                               \
    bool _r(istream &is_)                                                      \
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
                Message::flush_pending();                                      \
                                                                               \
                return true;                                                   \
            }                                                                  \
        }                                                                      \
                                                                               \
        return false;                                                          \
    }                                                                          \
                                                                               \
    bool _w(ostream &os) const                                                 \
    {                                                                          \
        (void)os;                                                              \
                                                                               \
        return (true                                                           \
                SCAN_FIELDS(WRITE_FIELD, FIRST(__VA_ARGS__))                   \
                SCAN_FIELDS(WRITE_FIELD, REMAIN(__VA_ARGS__)));                \
    }                                                                          \
                                                                               \
    virtual bool rewind()                                                      \
    {                                                                          \
        return Message::rewind();                                              \
    }                                                                          \
                                                                               \
    virtual void set_changed(ssize_t f, bool v)                                \
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
    virtual const string _s(int indent) const                                  \
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

#endif // __MESSAGE_DECLARATION__
