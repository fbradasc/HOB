#if !defined(__HOB_HPP__)
#define __HOB_HPP__
// #include <stdint.h>
#include <stdio.h>
// #include <stdlib.h>
// #include <limits>
// #include <sstream>
#include <cstring>
#include <climits>
// #include <iostream>
// #include <iomanip>
#include "utils/cpp_magic.h"

//========================================================================
//
//  hob's fields data types:
//
//  - Unsigned integer:
//
//      uint8_t, uint16_t, uint32_t, uint64_t,
//
//  - Signed integer:
//
//      int8_t, int16_t, int32_t, int64_t,
//
//  - Floating point numbers:
//
//      float, double, long double
//
//  - Boolean values:
//
//      bool, bitset<N>
//
//  - Fixed lenght array of bits:
//
//      bitset<N>
//
//  - Strings:
//
//      string
//
//  - Itself:
//
//      hob
//
//  - Containers for all the above data types:
//
//      - variable lenght array:
//
//          vector<T>
//
//      - map (both key and value can be any of the above data type):
//
//          map<K,V>
//
//      - a field can be optional:
//
//          optional<T>
//
//========================================================================
//
#include <inttypes.h>
#include <bitset>
#include <string>
#include <vector>
#include <map>
#include "hob/std/optional.hpp"

#define M_LOG(...)            printf("%s:%s:%d: ",        \
                                     __FILE__,            \
                                     __PRETTY_FUNCTION__, \
                                     __LINE__);           \
                              printf(__VA_ARGS__);        \
                              printf("\n");

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
#define HUPDATE(s)   __update_id(static_cast<uint64_t>( \
                                 (HLEN(s)>0) ? H256(s,0,__get_id()) \
                                             : __get_id()))
#else // !ENABLE_CPP_MAX_SIZE_HASH
#define HUPDATE(s)   __update_id(static_cast<uint64_t>( \
                                 (HLEN(s)>0) ? H64(s,0,__get_id()) \
                                             : __get_id()))
#endif // !ENABLE_CPP_MAX_SIZE_HASH
#else // !ENABLE_CPP_HASH
#define HUPDATE(s)   __update_id(s)
#endif // !ENABLE_CPP_HASH

namespace hobio
{
    namespace json
    {
        class decoder;
    }
}

using namespace std;
using namespace nonstd;

class hob
{
public:
    typedef uint64_t UID;

    static const UID UNDEFINED = ULLONG_MAX;

    class encoder
    {
    public:
        virtual ~encoder()
        {
        }

        virtual bool encode_header(const char     *name ,
                                   const string   &value,
                                   const UID      &id   ,
                                   const size_t   &payload) = 0;

        virtual bool encode_header(const char     *name ,
                                   const UID      &value,
                                   const UID      &id   ,
                                   const size_t   &payload) = 0;

        virtual bool encode_header(const char     *name ,
                                   const char     *value,
                                   const UID      &id   ,
                                   const size_t   &payload) = 0;

        template<class T>
        bool encode_field(const T &v,
                          const char *type = NULL,
                          const char *name = NULL)
        {
            return encode_field_pre (type, name) &&
                   encode           (         v) &&
                   encode_field_post(type, name);
        }

        virtual bool encode_field_pre(const char *type = NULL,
                                      const char *name = NULL) = 0;

        virtual bool encode_field_post(const char *type = NULL,
                                       const char *name = NULL) = 0;

        virtual bool encode_footer() = 0;

        virtual inline size_t field_size(const uint8_t     &v) = 0;
        virtual inline size_t field_size(const uint16_t    &v) = 0;
        virtual inline size_t field_size(const uint32_t    &v) = 0;
        virtual inline size_t field_size(const uint64_t    &v) = 0;
        virtual inline size_t field_size(const int8_t      &v) = 0;
        virtual inline size_t field_size(const int16_t     &v) = 0;
        virtual inline size_t field_size(const int32_t     &v) = 0;
        virtual inline size_t field_size(const int64_t     &v) = 0;
        virtual inline size_t field_size(const bool        &v) = 0;
        virtual inline size_t field_size(const float       &v) = 0;
        virtual inline size_t field_size(const double      &v) = 0;
        virtual inline size_t field_size(const long double &v) = 0;

        inline size_t field_size(const string &v)
        {
            return field_size(v.size()) + v.size();
        }

        inline size_t field_size(size_t size_, const void *v)
        {
            return field_size(size_) + size_;
        }

        size_t field_size(const hob &v)
        {
            return v.size(*this);
        }

        template<size_t N>
        size_t field_size(const std::bitset<N>& v)
        {
            return field_size((N+7)>>3) + ((N+7)>>3);
        }

        template<class T>
        size_t field_size(const vector<T> &v)
        {
            size_t retval = field_size(v.size());

            for (size_t i=0; i<v.size(); i++)
            {
                retval += field_size(static_cast<const T &>(v[i]));
            }

            return retval;
        }

        size_t field_size(const vector<bool> &v)
        {
            size_t sz = v.size();
            return (field_size(sz) + field_size((sz+7)>>3) + ((sz+7)>>3));
        }

        template<class T>
        size_t field_size(const optional<T> &v)
        {
            size_t retval = field_size(static_cast<bool>(v));

            if (static_cast<bool>(v))
            {
                retval += field_size(static_cast<const T &>(*v));
            }

            return retval;
        }

        template<class K, class V>
        size_t field_size(const map<K,V> &v)
        {
            size_t len = v.size();
            size_t retval = field_size(len);

            for (typename map<K,V>::const_iterator ci = v.begin();
                 (len > 0) && (ci != v.end());
                 ++ci)
            {
                retval += field_size(static_cast<const K &>((*ci).first ));
                retval += field_size(static_cast<const V &>((*ci).second));

                len--;
            }

            return retval;
        }

    protected:
        friend class hobio::json::decoder;

        virtual bool encode(const uint8_t      &v) = 0;
        virtual bool encode(const uint16_t     &v) = 0;
        virtual bool encode(const uint32_t     &v) = 0;
        virtual bool encode(const uint64_t     &v) = 0;
        virtual bool encode(const int8_t       &v) = 0;
        virtual bool encode(const int16_t      &v) = 0;
        virtual bool encode(const int32_t      &v) = 0;
        virtual bool encode(const int64_t      &v) = 0;
        virtual bool encode(const bool         &v) = 0;
        virtual bool encode(const float        &v) = 0;
        virtual bool encode(const double       &v) = 0;
        virtual bool encode(const long double  &v) = 0;
        virtual bool encode(const string       &v) = 0;
        virtual bool encode(const hob          &v) = 0;
        virtual bool encode(const vector<bool> &v) = 0;

        template<size_t N>
        bool encode(const std::bitset<N>& v)
        {
            uint8_t bits[(N+7)>>3];

            memset(bits, 0, ((N+7)>>3));

            for (size_t j=0; j<size_t(N); j++)
            {
                bits[j>>3] |= (v[j]<<(j%8));
            }

            return encode_bitset(N, bits);
        }

        virtual bool encode_bitset(size_t size, const uint8_t *bits) = 0;

        template<class T>
        bool encode(const vector<T> &v)
        {
            size_t len = v.size();

            if (!encode_vector_begin(len))
            {
                return false;
            }

            for (size_t i=0; i<len; i++)
            {
                encode_vector_item_pre(i,len);

                if (!encode(static_cast<const T &>(v[i])))
                {
                    return false;
                }

                encode_vector_item_post(i,len);
            }

            return encode_vector_end();
        }

        virtual bool encode_vector_begin(size_t len) = 0;
        virtual void encode_vector_item_pre(size_t i, size_t len) = 0;
        virtual void encode_vector_item_post(size_t i, size_t len) = 0;
        virtual bool encode_vector_end() = 0;

        template<class T>
        bool encode(const optional<T> &v)
        {
            bool has_value = static_cast<bool>(v);

            if (!encode_optional_begin(has_value))
            {
                return false;
            }

            if (has_value)
            {
                encode_optional_item_pre();

                if (!encode(static_cast<const T &>(*v)))
                {
                    return false;
                }

                encode_optional_item_post();
            }

            return encode_optional_end(has_value);
        }

        virtual bool encode_optional_begin(const bool &has_value) = 0;
        virtual void encode_optional_item_pre() = 0;
        virtual void encode_optional_item_post() = 0;
        virtual bool encode_optional_end(const bool &has_value) = 0;

        template<class K, class V>
        bool encode(const map<K,V> &v)
        {
            size_t len = v.size();

            if (!encode_map_begin(len))
            {
                return false;
            }

            for (typename map<K,V>::const_iterator ci = v.begin();
                 (len > 0) && (ci != v.end());
                 ++ci)
            {
                encode_map_item_key_pre();

                if (!encode(static_cast<const K &>((*ci).first)))
                {
                    return false;
                }

                encode_map_item_key_post();

                encode_map_item_value_pre();

                if (!encode(static_cast<const V &>((*ci).second)))
                {
                    return false;
                }

                encode_map_item_value_post(len);

                len--;
            }

            return encode_map_end();
        }

        virtual bool encode_map_begin(const size_t &size) = 0;
        virtual void encode_map_item_key_pre() = 0;
        virtual void encode_map_item_key_post() = 0;
        virtual void encode_map_item_value_pre() = 0;
        virtual void encode_map_item_value_post(const size_t &remaining) = 0;
        virtual bool encode_map_end() = 0;
    };

    class decoder
    {
    public:
        virtual ~decoder()
        {
        }

        virtual bool operator>>(encoder &enc)
        {
            return false;
        }

        virtual bool load(bool update=true) = 0;
        virtual bool bufferize(size_t size) = 0;
        virtual bool read_field(void *v, size_t size) = 0;

        virtual bool seek(ssize_t offset, int whence) = 0;
        virtual ssize_t tell() = 0;
        virtual void skip(size_t size) = 0;

        virtual bool decode_field(uint8_t  &v, bool *changed = NULL) = 0;
        virtual bool decode_field(uint16_t &v, bool *changed = NULL) = 0;
        virtual bool decode_field(uint32_t &v, bool *changed = NULL) = 0;
        virtual bool decode_field(uint64_t &v, bool *changed = NULL) = 0;
        virtual bool decode_field(int8_t   &v, bool *changed = NULL) = 0;
        virtual bool decode_field(int16_t  &v, bool *changed = NULL) = 0;
        virtual bool decode_field(int32_t  &v, bool *changed = NULL) = 0;
        virtual bool decode_field(int64_t  &v, bool *changed = NULL) = 0;

        bool decode_field(bool &v, bool *changed = NULL)
        {
            bool rv;

            if (!read_field(&rv, sizeof(v)))
            {
                return false;
            }

            if (changed)
            {
                *changed = ( v != rv );
            }

            v = rv;

            return true;
        }

        bool decode_field(float &v, bool *changed = NULL)
        {
            float rv;

            if (!read_field(&rv, sizeof(v)))
            {
                return false;
            }

            if (changed)
            {
                *changed = ( v != rv );
            }

            v = rv;

            return true;
        }

        bool decode_field(double &v, bool *changed = NULL)
        {
            double rv;

            if (!read_field(&rv, sizeof(v)))
            {
                return false;
            }

            if (changed)
            {
                *changed = ( v != rv );
            }

            v = rv;

            return true;
        }

        bool decode_field(long double &v, bool *changed = NULL)
        {
            long double rv;

            if (!read_field(&rv, sizeof(v)))
            {
                return false;
            }

            if (changed)
            {
                *changed = ( v != rv );
            }

            v = rv;

            return true;
        }

        bool decode_field(string &v, bool *changed = NULL)
        {
            size_t len;

            if (!decode_field(len))
            {
                return false;
            }

            vector<char> tmp(len);

            if (!read_field(tmp.data(), len))
            {
                return false;
            }

            string rv;

            rv.assign(tmp.data(),len);

            if (changed)
            {
                *changed = ( v != rv );
            }

            v = rv;

            return true;
        }

        bool decode_field(hob &v, bool *changed = NULL)
        {
            // return is_.__deserialize(*this,v,field);

            hob h;

            if (h.__decode(*this,false))
            {
                v = h;

                if (v << static_cast<hob::decoder*>(h))
                {
                    if (changed)
                    {
                        *changed = static_cast<bool>(v);
                    }

                    return true;
                }
            }

            return false;
        }

        template<size_t N>
        bool decode_field(bitset<N> &v, bool *changed = NULL)
        {
            size_t len;

            if (!decode_field(len) || (len != ((N + 7) >> 3)))
            {
                return false;
            }

            uint8_t bits[len];

            memset(bits, 0, len);

            if (!read_field(bits, len))
            {
                return false;
            }

            bitset<N> rv;

            for (size_t j=0; j<size_t(N); j++)
            {
                rv[j] = ((bits[j>>3]>>(j&7)) & 1);
            }

            if (changed)
            {
                *changed = ( v != rv );
            }

            v = rv;

            return true;
        }

        bool decode_field(vector<bool> &v, bool *changed = NULL)
        {
            size_t count;

            if (!decode_field(count))
            {
                return false;
            }

            if (count > 0)
            {
                size_t len;

                if (!decode_field(len) || (len != ((count + 7) >> 3)))
                {
                    return false;
                }

                uint8_t bits[len];

                memset(bits, 0, len);

                if (!read_field(bits, len))
                {
                    return false;
                }

                vector<bool> rv;

                for (size_t j=0; j<count; j++)
                {
                    rv.push_back((bits[j>>3]>>(j&7)) & 1);
                }

                if (changed)
                {
                    *changed = ( v != rv );
                }

                v = rv;

                return true;
            }
            else
            {
                if (changed)
                {
                    *changed = !v.empty();
                }

                v = vector<bool>();
            }

            return true;
        }

        template<class T>
        bool decode_field(vector<T> &v, bool *changed = NULL)
        {
            size_t len;

            if (!decode_field(len))
            {
                return false;
            }

            if (len > 0)
            {
                vector<T> rv(len);

                for (size_t i=0; i<len; i++)
                {
                    if (!decode_field(static_cast<T&>(rv[i])))
                    {
                        return false;
                    }
                }

                if (changed)
                {
                    *changed = ( v != rv );
                }

                v = rv;
            }
            else
            {
                if (changed)
                {
                    *changed = !v.empty();
                }

                v = vector<T>();
            }

            return true;
        }

        template<class T>
        bool decode_field(optional<T> &v, bool *changed = NULL)
        {
            bool has_field;

            if (!decode_field(has_field))
            {
                return false;
            }

            if (has_field)
            {
                T rv;

                if (!decode_field(static_cast<T&>(rv)))
                {
                    return false;
                }

                if (changed)
                {
                    *changed = ( v != rv );
                }

                v = rv;
            }
            else
            {
                if (changed)
                {
                    *changed = v.has_value();
                }

                v = optional<T>();
            }

            return true;
        }

        template<class K, class V>
        bool decode_field(map<K,V> &v, bool *changed = NULL)
        {
            size_t len;

            if (!decode_field(len))
            {
                return false;
            }

            if (len > 0)
            {
                map<K,V> rv;

                for (size_t i=0; i<len; i++)
                {
                    K key;
                    V val;

                    if (!decode_field(static_cast<K&>(key)))
                    {
                        return false;
                    }

                    if (!decode_field(static_cast<V&>(val)))
                    {
                        return false;
                    }

                    rv[key] = val;
                }

                if (changed)
                {
                    *changed = ( v != rv );
                }

                v = rv;
            }
            else
            {
                if (changed)
                {
                    *changed = !v.empty();
                }

                v = map<K,V>();
            }

            return true;
        }
    };

    hob()
        : _id(UNDEFINED)
        , _is(     NULL)
        , _sp(        0)
        , _ep(        0)
        , _np(       -1)
    { }

    hob(const UID &id_)
        : _id(   0)
        , _is(NULL)
        , _sp(   0)
        , _ep(   0)
        , _np(  -1)
    {
        __update_id(id_);
    }

    hob(const char *id_)
        : _id(   0)
        , _is(NULL)
        , _sp(   0)
        , _ep(   0)
        , _np(  -1)
    {
        __update_id(id_);
    }

    hob(const string &id_)
        : _id(   0)
        , _is(NULL)
        , _sp(   0)
        , _ep(   0)
        , _np(  -1)
    {
        __update_id(id_);
    }

    hob(const hob &ref)
    {
        *this = ref;
    }

    virtual ~hob()
    {
    }

    hob & operator=(const hob & ref)
    {
        _id = ref._id;
        _is = ref._is;
        _sp = ref._sp;
        _ep = ref._ep;

        return *this;
    }

    inline bool operator<<(decoder *is)
    {
        if (NULL == is)
        {
            return false;
        }

        return *this << *is;
    }

    inline bool operator<<(decoder &is)
    {
        __flush_pending();

        return __decode(is);
    }

    inline bool operator<<(hob & ref)
    {
        return ((_id == ref._id) && __decode(ref));
    }

    virtual bool operator>>(encoder &os) const
    {
        return
        (
            (UNDEFINED == _id)
            ||
            (
                os.encode_header((const char *)NULL, (const char *)NULL, _id, 0)
                &&
                os.encode_footer()
            )
        );
    }

    inline bool operator==(const hob &ref) const
    {
        bool rv = (_id == ref._id);

        return rv;
    }

    inline bool operator!=(const hob &ref) const
    {
        bool rv = (_id != ref._id);

        return rv;
    }

    inline operator decoder *()
    {
        return _is;
    }

    inline operator bool()
    {
        return __is_changed();
    }

    inline void operator~()
    {
        __reset_changes();
    }

    inline size_t size(hob::encoder &os) const
    {
        size_t sz = __payload(os);

        return os.field_size(_id) + ((sz > 0) ? (os.field_size(sz) + sz) : 0);
    }

    inline bool __rewind()
    {
        return (NULL != _is) && _is->seek(_sp,SEEK_SET);
    }

protected:
    virtual bool __decode(hob &ref)
    {
        (void)ref;
        return true;
    }

    virtual bool __decode(decoder &is, bool update=true)
    {
        uint64_t id_;

        _is = NULL;
        _sp = 0;
        _ep = 0;

        if (!is.load(update))
        {
            return false;
        }

        if (!is.decode_field(id_))
        {
            return false;
        }

        size_t sz_ = 0;

        if (__has_payload(id_) && !is.decode_field(sz_))
        {
            return false;
        }

        if (!is.bufferize(sz_))
        {
            return false;
        }

        _is = &is;
        _sp = is.tell();
        _ep = _sp + sz_;
        _id = id_;

        return true;
    }

    inline const uint64_t& __get_id() const
    {
        return _id;
    }

    inline void __update_id(const uint64_t &in)
    {
        _np++;

        _id = in;
    }

    inline void __update_id(const string &in)
    {
        __update_id(in.c_str());
    }

    void __update_id(const char *in)
    {
        //
        // in != NULL -> update ID calculation
        //
        if (NULL != in)
        {
            //
            // count hob parameters
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

    virtual size_t __payload(hob::encoder &os) const
    {
        return 0;
    }

    inline bool __has_payload(uint64_t id)
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

    virtual bool __set_changed(ssize_t f, bool v)
    {
        (void)f;
        (void)v;

        return true;
    }

    virtual void __flush_pending()
    {
        if (NULL == _is)
        {
            return;
        }

        ssize_t cp = _is->tell();

        if ((cp >= 0) && (cp < _ep))
        {
            _is->skip(_ep - cp);
        }
    }

private:
    UID      _id;
    decoder *_is;
    ssize_t  _sp;
    ssize_t  _ep;
    ssize_t  _np;
};

inline bool operator<<(hob::encoder &e, hob          &h) { return h >> e; }
inline bool operator>>(hob::decoder &d, hob          &h) { return h << d; }
inline bool operator>>(hob          &f, hob          &t) { return t << f; }

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
#define DECODE_FIELD(t, n, ...)  && is.decode_field(n, &c) \
                                 && __set_changed(_ ## n, c)
#define ENCODE_FIELD(t, n, ...)  && os.encode_field(n, STR(t), STR(n))
#define FIELD_SIZE(t, n, ...)    + os.field_size(n)
#define COMPARE_FIELD(t, n, ...) && (n == ref.n)
#define CLONE_FIELD(t, n, ...)   n = ref.n;
#define UPDATE_NP(t, n, ...)     __update_id("");
#define UPDATE_ID(t, n, ...)     HUPDATE(STR(name_)); \
                                 HUPDATE(STR(t    )); \
                                 HUPDATE(STR(n    ));

#define CHANGED_FIELDS(n, ...) \
    IF(HAS_ARGS(__VA_ARGS__))(EVAL(CHANGED_FIELDS_INNER(n, __VA_ARGS__)))

#define CHANGED_FIELDS_INNER(n, ...)                                           \
    bitset<_FIELDS_COUNT_> __ ## n ## __changed__fields__;                     \

#define RESET_CHANGES(n,f)                                                     \
    if (f == _FIELDS_COUNT_)                                                   \
    {                                                                          \
        __ ## n ## __changed__fields__.reset();                                \
    }                                                                          \
    else                                                                       \
    if (f < _FIELDS_COUNT_)                                                    \
    {                                                                          \
        __ ## n ## __changed__fields__.reset(f);                               \
    }                                                                          \

#define CHECK_CHANGES(n,f)                                                     \
    if (f == _FIELDS_COUNT_)                                                   \
    {                                                                          \
        return __ ## n ## __changed__fields__.any();                           \
    }                                                                          \
    else                                                                       \
    if (f < _FIELDS_COUNT_)                                                    \
    {                                                                          \
        return __ ## n ## __changed__fields__.test(f);                         \
    }                                                                          \

#define SET_CHANGED(n,f,v)                                                     \
    if (f < _FIELDS_COUNT_)                                                    \
    {                                                                          \
        __ ## n ## __changed__fields__[f] = v;                                 \
    }                                                                          \

#define HOBSTRUCT(name_, value_, ...)                                          \
class name_                                                                    \
    : public hob                                                               \
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
    name_() : hob(value_)                                                      \
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
        __update_id(static_cast<const char *>(NULL)); /* finalize hob ID */    \
    }                                                                          \
                                                                               \
    name_(const hob & ref): hob(ref)                                           \
    {                                                                          \
        *this = ref;                                                           \
    }                                                                          \
                                                                               \
    name_(const name_ & ref): hob(ref)                                         \
    {                                                                          \
        *this = ref;                                                           \
    }                                                                          \
                                                                               \
    ~name_()                                                                   \
    {                                                                          \
    }                                                                          \
                                                                               \
    name_ & operator=(const hob & ref)                                         \
    {                                                                          \
        *static_cast<hob *>(this) = static_cast<const hob &>(ref);             \
                                                                               \
        return *this;                                                          \
    }                                                                          \
                                                                               \
    name_ & operator=(const name_ & ref)                                       \
    {                                                                          \
        *static_cast<hob *>(this) = static_cast<const hob &>(ref);             \
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
        rv = ((*static_cast<const hob*>(this) ==                               \
                static_cast<const hob &>(ref))                                 \
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
    virtual bool operator>>(hob::encoder &os) const                            \
    {                                                                          \
        size_t payload = __payload(os);                                        \
                                                                               \
        return                                                                 \
        (                                                                      \
            (UNDEFINED == __get_id())                                          \
            ||                                                                 \
            (                                                                  \
                os.encode_header(STR(name_),                                   \
                                 value_,                                       \
                                 __get_id(),                                   \
                                 payload)                                      \
                SCAN_FIELDS(ENCODE_FIELD, FIRST(__VA_ARGS__))                  \
                SCAN_FIELDS(ENCODE_FIELD, REMAIN(__VA_ARGS__))                 \
                &&                                                             \
                os.encode_footer()                                             \
            )                                                                  \
        );                                                                     \
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
    bool __decode(hob &ref)                                                    \
    {                                                                          \
        hob::decoder *is = static_cast<hob::decoder*>(ref);                    \
                                                                               \
        if (NULL == is)                                                        \
        {                                                                      \
            return false;                                                      \
        }                                                                      \
                                                                               \
        __reset_changes();                                                     \
                                                                               \
        ref.__rewind();                                                        \
                                                                               \
        if (!__decode(*is))                                                    \
        {                                                                      \
            return false;                                                      \
        }                                                                      \
                                                                               \
        *this = ref;                                                           \
                                                                               \
        return true;                                                           \
    }                                                                          \
                                                                               \
    bool __decode(hob::decoder &is, bool update=true)                          \
    {                                                                          \
        (void)is;                                                              \
                                                                               \
        bool c = false;                                                        \
                                                                               \
        /* Read mandatory fields : fail on error */                            \
                                                                               \
        if (true SCAN_FIELDS(DECODE_FIELD, FIRST(__VA_ARGS__)))                \
        {                                                                      \
            /* Read optional fields : ignore errors */                         \
                                                                               \
            if ((true SCAN_FIELDS(DECODE_FIELD, REMAIN(__VA_ARGS__))) || true) \
            {                                                                  \
                hob::__flush_pending();                                        \
                                                                               \
                return true;                                                   \
            }                                                                  \
        }                                                                      \
                                                                               \
        return false;                                                          \
    }                                                                          \
                                                                               \
    virtual bool __set_changed(ssize_t f, bool v)                              \
    {                                                                          \
        (void)f;                                                               \
        (void)v;                                                               \
                                                                               \
        if (f < 0)                                                             \
        {                                                                      \
            return false;                                                      \
        }                                                                      \
                                                                               \
        IF(HAS_ARGS(__VA_ARGS__))(SET_CHANGED(name_,f,v))                      \
                                                                               \
        return true;                                                           \
    }                                                                          \
                                                                               \
    virtual void __flush_pending()                                             \
    {                                                                          \
    }                                                                          \
                                                                               \
    virtual size_t __payload(hob::encoder &os) const                           \
    {                                                                          \
        return (0                                                              \
                SCAN_FIELDS(FIELD_SIZE, FIRST(__VA_ARGS__))                    \
                SCAN_FIELDS(FIELD_SIZE, REMAIN(__VA_ARGS__)));                 \
    }                                                                          \
                                                                               \
private:                                                                       \
    CHANGED_FIELDS(name_, __VA_ARGS__)                                         \
};

#endif // __HOB_HPP__
