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
#include "optional.hpp"
#include "cpp_magic.h"

#define M_LOG(...)            printf("%s:%s:%d: ",        \
                                     __FILE__,            \
                                     __PRETTY_FUNCTION__, \
                                     __LINE__);           \
                              printf(__VA_ARGS__);        \
                              printf("\n");

/*
#define ASSERT_SREAD(v,s,f)   (f.good()                   && \
                               f.read((char *)v,s).good() && \
                               read(s))
*/
#define ASSERT_SREAD(v,s,f)   read(v,s,f)

#define ASSERT_SWRITE(v,s,f)  (f.write((char *)v,s).good())

#define INDENT(l)             string((indent + (l)) * 4, ' ')

#define HSEED                 65599llu
// #define HSEED                 11400714819323198485llu
#define HLEN(s)	              ((sizeof(s)/sizeof(s[0])) - 1)
#define H1(s,i,x)             ((uint64_t)(x)*(((i)<HLEN(s))?HSEED:1) + \
                               (uint8_t)s[((i)<HLEN(s))?HLEN(s)-1-(i):HLEN(s)])

#define H4(s,i,x)             H1(s,(i),H1(s,(i)+1,H1(s,(i)+2,H1(s,(i)+3,(x)))))
#define H16(s,i,x)            H4(s,(i),H4(s,(i)+4,H4(s,(i)+8,H4(s,(i)+12,(x)))))
#define H64(s,i,x)            H16(s,(i),H16(s,(i)+16,H16(s,(i)+32,H16(s,(i)+48,(x)))))
#define H256(s,i,x)           H64(s,(i),H64(s,(i)+64,H64(s,(i)+128,H64(s,(i)+192,(x)))))

#define HASH(x)               ((uint64_t)((x)^((x)>>32)))

#define ZIGZAG_DECODE(value)  (uint64_t)(((value) >> 1) ^ -((value) & 1))
#define ZIGZAG_ENCODE(value)  (uint64_t)(((int64_t)(value) << 1) ^ \
                                         ((int64_t)(value) >> ((sizeof(int64_t)*8)-1)))

// |--------------------------------|
// |0..............................31|
//
// |----------------------------------------------------------------|
// |0..............................................................63|
//
#if defined(ENABLE_CPP_HASH)
#if defined(ENABLE_CPP_MAX_SIZE_HASH)
#define HUPDATE(s)   id((uint64_t)((HLEN(s)>0)?H256(s,0,id()):id()))
#else // !ENABLE_CPP_MAX_SIZE_HASH
#define HUPDATE(s)   id((uint64_t)((HLEN(s)>0)?H64(s,0,id()):id()))
#endif // !ENABLE_CPP_MAX_SIZE_HASH
#else // !ENABLE_CPP_HASH
#define HUPDATE(s)   id(s)
#endif // !ENABLE_CPP_HASH

using namespace std;
using namespace nonstd;

class Message
{
public:
    static const uint64_t UNDEFINED = ULLONG_MAX;

    Message()
        : _id(UNDEFINED)
        , _is(     NULL)
        , _sp(        0)
        , _ep(        0)
        , _np(       -1)
    { }

    Message(const uint64_t &id_)
        : _id(   0)
        , _is(NULL)
        , _sp(   0)
        , _ep(   0)
        , _np(  -1)
    {
        id(id_);
    }

    Message(const char *id_)
        : _id(   0)
        , _is(NULL)
        , _sp(   0)
        , _ep(   0)
        , _np(  -1)
    {
        id(id_);
    }

    Message(const string &id_)
        : _id(   0)
        , _is(NULL)
        , _sp(   0)
        , _ep(   0)
        , _np(  -1)
    {
        id(id_);
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

    bool operator<<(istream &is)
    {
        flush_pending();

        return _r(is);
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

        std::ostringstream ss;

        if (!_w((has_payload(_id)) ? ss : os))
        {
            return false;
        }

        if (has_payload(_id) && !_w(os, ss.str()))
        {
            return false;
        }

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

    string json(int indent = 0) const
    {
#if !defined(BINARY_ONLY)
        stringstream ss;

        ss << _j(indent);
        ss << INDENT(1) << "\"ID\": " << _id << endl;
        ss << INDENT(0) << "}";

        return(ss.str());
#else
        return "{}";
#endif
    }

    size_t size() const
    {
        return ( _ep - _sp );
    }

    virtual bool rewind()
    {
        is().clear();

        return is().seekg(_sp,is().beg).good();
    }

    virtual void flush_pending()
    {
        streampos cp = is().tellg();

        if ((cp >= 0) && (cp < _ep))
        {
            is().ignore(_ep - cp);
        }
    }

    istream &is() { return (NULL != _is) ? *_is : _ss; }

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

    bool _w(ostream &os, const uint8_t &v) const
    {
        return _w(os, (const uint64_t &)v);
    }

    bool _w(ostream &os, const uint16_t &v) const
    {
        return _w(os, (const uint64_t &)v);
    }

    bool _w(ostream &os, const uint32_t &v) const
    {
        return _w(os, (const uint64_t &)v);
    }

    // VARINT packing
    //
    bool _w(ostream &os, const uint64_t &v) const
    {
        uint8_t d[9];

        uint8_t b = (v <= 0x000000000000007f) ? 1 :
                    (v <= 0x0000000000003fff) ? 2 :
                    (v <= 0x00000000001fffff) ? 3 :
                    (v <= 0x000000000fffffff) ? 4 :
                    (v <= 0x00000007ffffffff) ? 5 :
                    (v <= 0x000003ffffffffff) ? 6 :
                    (v <= 0x0001ffffffffffff) ? 7 :
                    (v <= 0x00ffffffffffffff) ? 8 : 9;

        uint8_t  m = 0xff;
        uint32_t c = 0xfffffe00;

        for (uint8_t i=0; i<b; i++)
        {
            d[b-i-1] = (uint8_t)( ( v >> (8*i) ) & 0xff );

            c >>= 1;
            m >>= 1;
        }

        d[0] &= (uint8_t)(m & 0xff);
        d[0] |= (uint8_t)(c & 0xff);

        return ASSERT_SWRITE(d, b, os);
    }

    bool _w(ostream &os, const int8_t &v) const
    {
        return _w(os, (int64_t)v);
    }

    bool _w(ostream &os, const int16_t &v) const
    {
        return _w(os, (int64_t)v);
    }

    bool _w(ostream &os, const int32_t &v) const
    {
        return _w(os, (int64_t)v);
    }

    // ZIGZAG encoding
    //
    bool _w(ostream &os, const int64_t &v) const
    {
        return _w(os, ZIGZAG_ENCODE(v));
    }

    bool _w(ostream &os, const bool &v) const
    {
        return ASSERT_SWRITE(&v, sizeof(v), os);
    }

    bool _w(ostream &os, const float &v) const
    {
        return ASSERT_SWRITE(&v, sizeof(v), os);
    }

    bool _w(ostream &os, const double &v) const
    {
        return ASSERT_SWRITE(&v, sizeof(v), os);
    }

    bool _w(ostream &os, const string &v) const
    {
        if (!_w(os, v.size()))
        {
            return false;
        }

        return ASSERT_SWRITE(v.data(), v.size(), os);
    }

    bool _w(ostream &os, const Message &v) const
    {
        return (v >> os);
    }

    template<size_t N>
    bool _w(ostream &os, const std::bitset<N>& v) const
    {
        vector<uint8_t> bits((N + 7) >> 3);

        for (size_t j=0; j<size_t(N); j++)
        {
            bits[j>>3] |= (v[j] << (j & 7));
        }

        return _w(os, bits);
    }

    template<class T>
    bool _w(ostream &os, const vector<T> &v) const
    {
        if (!_w(os, v.size()))
        {
            return false;
        }

        for (size_t i=0; i<v.size(); i++)
        {
            if (!_w(os, (const T &)v[i]))
            {
                return false;
            }
        }

        return true;
    }

    template<class T>
    bool _w(ostream &os, const optional<T> &v) const
    {
        if (!_w(os, (bool)v))
        {
            return false;
        }

        if ((bool)v)
        {
            if (!_w(os, (const T &)*v))
            {
                return false;
            }
        }

        return true;
    }

    template<class K, class V>
    bool _w(ostream &os, const map<K,V> &v) const
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
            if (!_w(os, (const K &)(*ci).first))
            {
                return false;
            }

            if (!_w(os, (const V &)(*ci).second))
            {
                return false;
            }

            len--;
        }

        return true;
    }

    bool _r(istream &is, uint8_t &v)
    {
        uint64_t r;

        if (!_r(is, r))
        {
            return false;
        }

        v = (uint8_t)(r & 0xff);

        return true;
    }

    bool _r(istream &is, uint16_t &v)
    {
        uint64_t r;

        if (!_r(is, r))
        {
            return false;
        }

        v = (uint16_t)(r & 0xffff);

        return true;
    }

    bool _r(istream &is, uint32_t &v)
    {
        uint64_t r;

        if (!_r(is, r))
        {
            return false;
        }

        v = (uint32_t)(r & 0xffffffff);

        return true;
    }

    // VARINT unpacking
    //
    bool _r(istream &is, uint64_t &v)
    {
        if (is.eof())
        {
            return false;
        }

        uint8_t d[9];
        uint8_t b = 0;
        uint8_t c;
        uint8_t m;

        if (!ASSERT_SREAD(&d[0], sizeof(uint8_t), is))
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
            if (!ASSERT_SREAD(&d[1], b-1, is))
            {
                return false;
            }
        }

        v = (b < 9) ? d[0] & ~m : 0;

        for (uint8_t i=1; i<b; i++)
        {
            v <<= 8;
            v |= ((uint64_t)(d[i] & 0xff));
        }

        return true;
    }

    bool _r(istream &is, int8_t &v)
    {
        return _z<int8_t>(is,v);
    }

    bool _r(istream &is, int16_t &v)
    {
        return _z<int16_t>(is,v);
    }

    bool _r(istream &is, int32_t &v)
    {
        return _z<int32_t>(is,v);
    }

    bool _r(istream &is, int64_t &v)
    {
        return _z<int64_t>(is,v);
    }

    bool _r(istream &is, bool &v)
    {
        return ASSERT_SREAD(&v, sizeof(v), is);
    }

    bool _r(istream &is, float &v)
    {
        return ASSERT_SREAD(&v, sizeof(v), is);
    }

    bool _r(istream &is, double &v)
    {
        return ASSERT_SREAD(&v, sizeof(v), is);
    }

    bool _r(istream &is, string &v)
    {
        uint64_t len;

        if (!_r(is, len))
        {
            return false;
        }

        vector<char> tmp(len);

        if (!ASSERT_SREAD(tmp.data(), len, is))
        {
            return false;
        }

        v.assign(tmp.data(),len);

        return true;
    }

    bool _r(istream &is, Message &v)
    {
        if (!is.eof())
        {
            Message m;

            if (m << is)
            {
                v = m;

                return ( v << m.is() );
            }
        }

        return false;
    }

    template<size_t N>
    bool _r(istream &is, std::bitset<N> &v)
    {
        vector<uint8_t> buf;

        if (!_r(is, buf))
        {
            return false;
        }

        if (buf.size() != ((N + 7) >> 3))
        {
            return false;
        }

        for (size_t j=0; j<size_t(N); j++)
        {
            v[j] = ((buf[j>>3] >> (j & 7)) & 1);
        }

        return true;
    }

    template<class T>
    bool _r(istream &is, vector<T> &v)
    {
        size_t len;

        if (!_r(is, len))
        {
            return false;
        }

        if (len > 0)
        {
            vector<T> tmp(len);

            for (size_t i=0; i<len; i++)
            {
                if (!_r(is, (T&)tmp[i]))
                {
                    return false;
                }
            }

            v = tmp;
        }
        else
        {
            v = vector<T>();
        }

        return true;
    }

    template<class T>
    bool _r(istream &is, optional<T> &v)
    {
        bool has_field;

        if (!_r(is, has_field))
        {
            return false;
        }

        if (has_field)
        {
            T tmp;

            if (!_r(is, (T&)tmp))
            {
                return false;
            }

            v = tmp;
        }
        else
        {
            v = optional<T>();
        }

        return true;
    }

    template<class K, class V>
    bool _r(istream &is, map<K,V> &v)
    {
        size_t len;

        if (!_r(is, len))
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

                if (!_r(is, (K&)k))
                {
                    return false;
                }

                if (!_r(is, (V&)v))
                {
                    return false;
                }

                tmp[k] = v;
            }

            v = tmp;
        }
        else
        {
            v = map<K,V>();
        }

        return true;
    }

    string _t(const uint8_t &v, int indent = 0) const
    {
        stringstream o;
        o << (int)v;
        return(o.str());
    }

    string _t(const uint16_t &v, int indent = 0) const
    {
        stringstream o;
        o << v;
        return(o.str());
    }

    string _t(const uint32_t &v, int indent = 0) const
    {
        stringstream o;
        o << v;
        return(o.str());
    }

    string _t(const uint64_t &v, int indent = 0) const
    {
        stringstream o;
        o << v;
        return(o.str());
    }

    string _t(const int8_t &v, int indent = 0) const
    {
        stringstream o;
        o << (int)v;
        return(o.str());
    }

    string _t(const int16_t &v, int indent = 0) const
    {
        stringstream o;
        o << v;
        return(o.str());
    }

    string _t(const int32_t &v, int indent = 0) const
    {
        stringstream o;
        o << v;
        return(o.str());
    }

    string _t(const int64_t &v, int indent = 0) const
    {
        stringstream o;
        o << v;
        return(o.str());
    }

    string _t(const float &v, int indent = 0) const
    {
        stringstream o;
        o << v;
        return(o.str());
    }

    string _t(const double &v, int indent = 0) const
    {
        stringstream o;
        o << v;
        return(o.str());
    }

    string _t(const char *v, int indent = 0) const
    {
        stringstream o;
        o << "\"" << v << "\"";
        return(o.str());
    }

    string _t(const string &v, int indent = 0) const
    {
        stringstream o;
        o << "\"" << v << "\"";
        return(o.str());
    }

    string _t(const bool &v, int indent = 0) const
    {
        return( v ? "true" : "false");
    }

    string _t(const Message &v, int indent = 0) const
    {
        return( v.json(indent) );
    }

    template<size_t N>
    string _t(const bitset<N> &v, int indent = 0) const
    {
        stringstream o;
        o << "\"" << v << "\"";
        return(o.str());
    }

    template<class T>
    string _t(const vector<T> &v, int indent = 0) const
    {
#if !defined(BINARY_ONLY)
        stringstream o;

        o << "[" << endl;

        size_t len = v.size();

        for (size_t i=0; i<len; i++)
        {
            o << INDENT(1) << _t((const T &)v[i], indent+1);

            if (i < (len-1))
            {
                o << ",";
            }

            o << endl;
        }

        o << INDENT(0) << "]";

        return(o.str());
#else
        return "[]";
#endif
    }

    template<class T>
    string _t(const optional<T> &v, int indent = 0) const
    {
#if !defined(BINARY_ONLY)
        if ((bool)v)
        {
            stringstream o;

            o << _t((const T &)*v, indent+1);

            return(o.str());
        }
#endif
        return "null";
    }

    template<class K, class V>
    string _t(const map<K,V> &v, int indent = 0) const
    {
#if !defined(BINARY_ONLY)
        stringstream o;

        o << "[" << endl;

        size_t len = v.size();

        for (typename map<K,V>::const_iterator ci = v.begin();
             (len > 0) && (ci != v.end());
             ++ci)
        {
            o << INDENT(1) << "{" << endl;
            o << INDENT(2) << "\"k\": " << _t((const K&)(*ci).first , indent+1);
            o << "," << endl;
            o << INDENT(2) << "\"v\": " << _t((const V&)(*ci).second, indent+1);
            o << endl;
            o << INDENT(1) << "}";

            if (len > 1)
            {
                o << ",";
            }

            o << endl;

            len--;
        }

        o << INDENT(0) << "]";

        return(o.str());
#else
        return "[]";
#endif
    }

    virtual bool _r(Message &ref) { return true; }

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

        uint64_t sz_ = 0;

        if (has_payload(id_) && !_r(is_, sz_))
        {
            return false;
        }

        bool success = true;

        _is = &is_;

        if ((sz_ > 0) && (is().tellg() < 0))
        {
            success = false;

            char *buffer = new char[sz_];

            if (NULL != buffer)
            {
                if (read(buffer,sz_,is_))
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
            _sp = is().tellg();
            _ep = _sp + (streamoff)sz_;
            _id = id_;
        }

        return success;
    }

    virtual bool _w(ostream &os) const { return true; }

#if !defined(BINARY_ONLY)
    virtual string _j(int indent = 0) const
    {
        return("");
    }
#endif // BINARY_ONLY

    inline uint64_t& id()
    {
        return _id;
    }

    inline void id(const uint64_t &in)
    {
        _np++;

        _id = in;
    }

    inline void id(const string &in)
    {
        id(in.c_str());
    }

    void id(const char *in)
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

private:
    uint64_t      _id;
    stringstream  _ss;
    istream      *_is;
    streampos     _sp;
    streampos     _ep;
    ssize_t       _np;

    // ZIGZAG decoding
    //
    template<class T>
    bool _z(istream &is, T &v)
    {
        uint64_t r;

        if (!_r(is, r))
        {
            return false;
        }

        v = (T)(ZIGZAG_DECODE(r) & ((T)-1));

        return true;
    }

    bool read(void *v, size_t s, istream &is)
    {
        if (is.fail())
        {
            is.clear();
        }

        return is.good() && is.read((char *)v,s).good();
    }
};

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

#define DECLARE_FIELD(t, n, ...) t n;
#define INIT_FIELD(t, n, ...)    IF(HAS_ARGS(__VA_ARGS__) )(n = __VA_ARGS__;)
#define READ_FIELD(t, n, ...)    && Message::_r(is, n)
#define WRITE_FIELD(t, n, ...)   && Message::_w(os, n)
#define COMPARE_FIELD(t, n, ...) && (n == ref.n)
#define CLONE_FIELD(t, n, ...)   n = ref.n;
#define UPDATE_NP(t, n, ...)     id("");
#define UPDATE_ID(t, n, ...)     HUPDATE(STR(name_)); \
                                 HUPDATE(STR(t    )); \
                                 HUPDATE(STR(n    ));
#define WJSON_FIELD(t, n, ...)                                                 \
        ss << INDENT(2) << "{" << endl                                         \
           << INDENT(3) << "\"t\": \"" << STR(t) << "\","       << endl        \
           << INDENT(3) << "\"n\": \"" << STR(n) << "\","       << endl        \
           << INDENT(3) << "\"v\": "   << _t((t&)n, (indent+3)) << endl        \
           << INDENT(2) << "}," << endl;

#if !defined(BINARY_ONLY)
#define JSON_DUMP(name_,value_,...)                                            \
    string _j(int indent = 0) const                                            \
    {                                                                          \
        stringstream ss;                                                       \
                                                                               \
        ss << "{"                                                              \
           << endl                                                             \
           << INDENT(1) << "\"t\": \"" << STR(name_) << "\","                  \
           << endl                                                             \
           << INDENT(1) << "\"v\": " << _t(value_, indent+1) << ","            \
           << endl                                                             \
           << INDENT(1) << "\"f\": ["                                          \
           << endl;                                                            \
                                                                               \
        SCAN_FIELDS(WJSON_FIELD, FIRST(__VA_ARGS__))                           \
        SCAN_FIELDS(WJSON_FIELD, REMAIN(__VA_ARGS__))                          \
                                                                               \
        ss << INDENT(2) << "null"                                              \
           << endl                                                             \
           << INDENT(1) << "],"                                                \
           << endl;                                                            \
                                                                               \
        return ss.str();                                                       \
    }
#else
#define JSON_DUMP(n,v,...)
#endif

#define DECLARE_MESSAGE(name_, value_, ...)                                    \
class name_ : public Message                                                   \
{                                                                              \
public:                                                                        \
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
        id((const char *)NULL); /* finalize message ID */                      \
    }                                                                          \
                                                                               \
    name_(const Message & ref)                                                 \
    {                                                                          \
        *this = ref;                                                           \
    }                                                                          \
                                                                               \
    name_(const name_ & ref)                                                   \
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
        *(Message *)this = (Message &)ref;                                     \
                                                                               \
        return *this;                                                          \
    }                                                                          \
                                                                               \
    name_ & operator=(const name_ & ref)                                       \
    {                                                                          \
        *(Message *)this = (Message &)ref;                                     \
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
        rv = ((*((Message*)this) == (const Message &)ref)                      \
               SCAN_FIELDS(COMPARE_FIELD, FIRST(__VA_ARGS__))                  \
               SCAN_FIELDS(COMPARE_FIELD, REMAIN(__VA_ARGS__)));               \
                                                                               \
        return rv;                                                             \
    }                                                                          \
                                                                               \
protected:                                                                     \
    bool _r(Message &ref)                                                      \
    {                                                                          \
        ref.rewind();                                                          \
                                                                               \
        if (!_r(ref.is()))                                                     \
        {                                                                      \
            return false;                                                      \
        }                                                                      \
                                                                               \
        *this = ref;                                                           \
                                                                               \
        return true;                                                           \
    }                                                                          \
                                                                               \
    bool _r(istream &is)                                                       \
    {                                                                          \
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
        return (true                                                           \
                SCAN_FIELDS(WRITE_FIELD, FIRST(__VA_ARGS__))                   \
                SCAN_FIELDS(WRITE_FIELD, REMAIN(__VA_ARGS__)));                \
    }                                                                          \
                                                                               \
    JSON_DUMP(name_,value_,__VA_ARGS__)                                        \
                                                                               \
    virtual void flush_pending()                                               \
    {                                                                          \
    }                                                                          \
                                                                               \
    virtual bool rewind()                                                      \
    {                                                                          \
        return Message::rewind();                                              \
    }                                                                          \
};

#endif // __MESSAGE_DECLARATION__
