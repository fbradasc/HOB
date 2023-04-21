#if !defined(__HOB_HPP__)
#define __HOB_HPP__
#include <stdio.h>
#include <cstring>
#include <climits>
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
#if defined(ENABLE_DYNAMIC_FIELDS)
#include <typeinfo>
#endif
#include "hob/std/optional.hpp"

#if defined(HOB_DEBUG)
#define M_LOG(...)            printf(__VA_ARGS__); \
                              printf(" @ %s:%d", __PRETTY_FUNCTION__, __LINE__); \
                              printf("\n");
#else
#define M_LOG(...)
#endif

#define _PAYLOAD_FLAG_POS_    0
#if defined(ENABLE_DYNAMIC_FIELDS)
#define _DYNFLDS_FLAG_POS_    1
#endif

#define HSEED                 65599llu
// #define HSEED                 11400714819323198485llu
#define HLEN(s)               ((sizeof(s)/sizeof(s[0])) - 1)
#define H1(s,i,x)             (static_cast<UID>(x)*(((i)<HLEN(s))?HSEED:1) \
                               +                                           \
                               static_cast<uint8_t>(s[((i)<HLEN(s))        \
                                                      ?HLEN(s)-1-(i)       \
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

#define HASH(x)               (static_cast<UID>((x)^((x)>>32)))

#if defined(ENABLE_CPP_HASH)
#if defined(ENABLE_CPP_MAX_SIZE_HASH)
#define HUPDATE(s)   __update_id(static_cast<UID>(                  \
                                 (HLEN(s)>0) ? H256(s,0,__get_id()) \
                                             : __get_id()))
#else // !ENABLE_CPP_MAX_SIZE_HASH
#define HUPDATE(s)   __update_id(static_cast<UID>(                 \
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
    typedef uint64_t    UID;

#if defined(ENABLE_DYNAMIC_FIELDS)
    enum hob_t
    {
        _t_unknown  = 0x00,
        _t_u8             , // 00000001b
        _t_u16            , // 00000010b
        _t_u32            , // 00000011b
        _t_u64            , // 00000100b
        _t_i8             , // 00000101b
        _t_i16            , // 00000110b
        _t_i32            , // 00000111b
        _t_i64            , // 00001000b
        _t_bool           , // 00001001b
        _t_f32            , // 00001010b
        _t_f64            , // 00001011b
        _t_f128           , // 00001100b
        _t_string         , // 00001101b
        _t_hob            , // 00001110b
        _t_special  = 0x0f, // 00001111b
    };

    class encoder; // forward declaration
    class decoder; // forward declaration
#endif // ENABLE_DYNAMIC_FIELDS

    static const UID UNDEFINED = ULLONG_MAX;

#if defined(ENABLE_DYNAMIC_FIELDS)
    ///////////////////////////////////////////////////////////////////////////
    //                                                                       //
    //                     Dynamic fields interface                          //
    //                                                                       //
    ///////////////////////////////////////////////////////////////////////////

    class variant
    {
    public:
        variant(): _id(UNDEFINED), _t(_t_unknown) { _v.pd = NULL; }

        variant(const UID    & in): _t(_t_unknown) { _v.pd = NULL; _id = in    ; }
        variant(const char   * in): _t(_t_unknown) { _v.pd = NULL; _id = id(in); }
        variant(const string & in): _t(_t_unknown) { _v.pd = NULL; _id = id(in); }

        variant(const variant & ref) { *this = ref; }

        inline variant & operator=(const variant &ref)
        {
            _id = ref._id;
            _t  = ref._t;

            memcpy(_v._d, ref._v._d, sizeof(_v._d));

            return *this;
        }

        inline bool operator==(const variant &ref) const
        {
            return _id == ref._id;
        }

        inline bool operator!=(const variant &ref) const
        {
            return _id != ref._id;
        }

        inline bool operator>(const variant &ref) const
        {
            return _id > ref._id;
        }

        inline bool operator<(const variant &ref) const
        {
            return _id < ref._id;
        }

        template<class K, class V>
        inline variant & operator=(const map<K,V> & m)
        {
            __clear();

            _t = __type(__whois<K>(), __whois<V>());

            if (v_type() != _t_unknown)
            {
                _v.pd = new Map<K,V>(m);
            }

            return *this;
        }

        template<class T>
        inline variant & operator=(const optional<T> & v)
        {
            __clear();

            _t = __type(__whois<T>(), _t_special);

            if (v_type() != _t_unknown)
            {
                _v.pd = new Optional<T>(v);
            }

            return *this;
        }

        template<class T>
        inline variant & operator=(const vector<T> & v)
        {
            __clear();

            _t = __type(_t_special, __whois<T>());

            if (v_type() != _t_unknown)
            {
                _v.pd = new Vector<T>(v);
            }

            return *this;
        }

        inline variant & operator=(const string & v)
        {
            __clear();

            _t = __type(_t_unknown, __whois<string>());

            if (v_type() != _t_unknown)
            {
                _v.pd = new String(v);
            }

            return *this;
        }

        inline variant & operator=(const hob & v)
        {
            __clear();

            _t = __type(_t_unknown, __whois<hob>());

            if (v_type() != _t_unknown)
            {
                _v.pd = new Hob(v);
            }

            return *this;
        }

        template<typename T>
        inline variant & operator=(const T & v)
        {
            const void *p = &v;

            __clear();

            _t = __type(_t_unknown, __whois<T>());

            switch (_t)
            {
                case _t_u8  : _v.uc = *static_cast<const uint8_t     *>(p); break;
                case _t_u16 : _v.us = *static_cast<const uint16_t    *>(p); break;
                case _t_u32 : _v.ui = *static_cast<const uint32_t    *>(p); break;
                case _t_u64 : _v.ul = *static_cast<const uint64_t    *>(p); break;
                case _t_i8  : _v.sc = *static_cast<const int8_t      *>(p); break;
                case _t_i16 : _v.ss = *static_cast<const int16_t     *>(p); break;
                case _t_i32 : _v.si = *static_cast<const int32_t     *>(p); break;
                case _t_i64 : _v.sl = *static_cast<const int64_t     *>(p); break;
                case _t_bool: _v.bd = *static_cast<const bool        *>(p); break;
                case _t_f32 : _v.fd = *static_cast<const float       *>(p); break;
                case _t_f64 : _v.dd = *static_cast<const double      *>(p); break;
                case _t_f128: _v.qd = *static_cast<const long double *>(p); break;
                default     :                                           break;
            }

            return *this;
        }

        inline UID     id  () const { return _id; }
        inline uint8_t type() const { return _t ; }

        inline hob_t v_type() const
        {
            return (__l_type() == _t_special) ? __h_type() : __l_type();
        }

        inline hob_t k_type() const
        {
            return __h_type();
        }

        inline bool is_basic() const
        {
            return ((__h_type() == _t_unknown)
                    &&
                    (__l_type() != _t_special)
                    &&
                    (__l_type() != _t_unknown));
        }

        inline bool is_vector() const
        {
            return ((__h_type() == _t_special)
                    &&
                    (__l_type() != _t_special)
                    &&
                    (__l_type() != _t_unknown));
        }

        inline bool is_optional() const
        {
            return ((__l_type() == _t_special)
                    &&
                    (__h_type() != _t_special)
                    &&
                    (__h_type() != _t_unknown));
        }

        inline bool is_map() const
        {
            return ((__l_type() != _t_special)
                    &&
                    (__l_type() != _t_unknown)
                    &&
                    (__h_type() != _t_special)
                    &&
                    (__h_type() != _t_unknown));
        }

        template<class T>
        operator const vector<T> *() const
        {
            if ((NULL == _v.pd)
                ||
                (_t_unknown == _t)
                ||
                !is_vector()
                ||
                (__whois<T>() != v_type()))
            {
                return NULL;
            }

            return static_cast<const vector<T> *>(_v.pd->data());
        }

        template<class T>
        operator const optional<T> *() const
        {
            if ((NULL == _v.pd)
                ||
                (_t_unknown == _t)
                ||
                !is_optional()
                ||
                (__whois<T>() != v_type()))
            {
                return NULL;
            }

            return static_cast<const optional<T> *>(_v.pd->data());
        }

        template<class K, class V>
        operator const map<K,V> *() const
        {
            if ((NULL == _v.pd)
                ||
                (_t_unknown == _t)
                ||
                !is_map()
                ||
                (__whois<K>() != k_type())
                ||
                (__whois<V>() != v_type()))
            {
                return NULL;
            }

            return static_cast< const map<K,V> * >(_v.pd->data());
        }

        template<class T>
        operator const T *() const
        {
            if ((_t_unknown == _t)
                ||
                !is_basic()
                ||
                (__whois<T>() != v_type()))
            {
                return NULL;
            }

            if ((v_type() == _t_string) || (v_type() == _t_hob))
            {
                if (NULL == _v.pd)
                {
                    return NULL;
                }

                return static_cast<const T *>(_v.pd->data());
            }

            const void *p = &_v._d;

            return static_cast<const T *>(p);
        }

        static UID id(const string &in)
        {
            return id(in.c_str());
        }

        static UID id(const char *in)
        {
            UID id_ = 0;

            //
            // in != NULL -> update ID calculation
            //
            if (NULL != in)
            {
                for (size_t i = 0; in[i]; ++i)
                {
                    id_ = HSEED * id_ + in[i];
                }
            }

            return id_;
        }

        size_t field_size(hob::encoder &e) const
        {
            if (type() == hob::_t_unknown)
            {
                return 0;
            }

            size_t retval = e.field_size(id())
                            +
                            e.field_size(type());

            if (!is_basic()
                ||
                (v_type() == hob::_t_string)
                ||
                (v_type() == hob::_t_hob))
            {
                if (NULL == _v.pd)
                {
                    return 0;
                }

                retval += _v.pd->field_size(e);
            }
            else switch (v_type())
            {
                case hob::_t_u8  : retval += e.field_size(_v.uc); break;
                case hob::_t_u16 : retval += e.field_size(_v.us); break;
                case hob::_t_u32 : retval += e.field_size(_v.ui); break;
                case hob::_t_u64 : retval += e.field_size(_v.ul); break;
                case hob::_t_i8  : retval += e.field_size(_v.sc); break;
                case hob::_t_i16 : retval += e.field_size(_v.ss); break;
                case hob::_t_i32 : retval += e.field_size(_v.si); break;
                case hob::_t_i64 : retval += e.field_size(_v.sl); break;
                case hob::_t_bool: retval += e.field_size(_v.bd); break;
                case hob::_t_f32 : retval += e.field_size(_v.fd); break;
                case hob::_t_f64 : retval += e.field_size(_v.dd); break;
                case hob::_t_f128: retval += e.field_size(_v.qd); break;

                default:
                {
                    return 0;
                }
                break;
            }

            return retval;
        }

        bool encode(hob::encoder &e) const
        {
            if (type() == hob::_t_unknown)
            {
                return false;
            }

            if (!e.encode_variant_begin(id(), type()))
            {
                return false;
            }

            bool encoded = false;

            if (!is_basic()
                ||
                (v_type() == hob::_t_string)
                ||
                (v_type() == hob::_t_hob))
            {
                if (NULL != _v.pd)
                {
                    encoded = _v.pd->encode(e);
                }
            }
            else switch (v_type())
            {
                case hob::_t_u8  : encoded = e.encode(_v.uc); break;
                case hob::_t_u16 : encoded = e.encode(_v.us); break;
                case hob::_t_u32 : encoded = e.encode(_v.ui); break;
                case hob::_t_u64 : encoded = e.encode(_v.ul); break;
                case hob::_t_i8  : encoded = e.encode(_v.sc); break;
                case hob::_t_i16 : encoded = e.encode(_v.ss); break;
                case hob::_t_i32 : encoded = e.encode(_v.si); break;
                case hob::_t_i64 : encoded = e.encode(_v.sl); break;
                case hob::_t_bool: encoded = e.encode(_v.bd); break;
                case hob::_t_f32 : encoded = e.encode(_v.fd); break;
                case hob::_t_f64 : encoded = e.encode(_v.dd); break;
                case hob::_t_f128: encoded = e.encode(_v.qd); break;
                default:
                    break;
            }

            return encoded && e.encode_variant_end();
        }

        template<class K, class V>
        bool decode_map(hob::decoder &d, bool *changed = NULL)
        {
            map<K, V> m;

            M_LOG("{");

            bool decoded = d.decode_field(m, changed);

            if (decoded)
            {
                *this = m;
            }

            M_LOG("} - %d", decoded);

            return decoded;
        }

        template<class K>
        bool decode_map(hob::decoder &d, bool *changed = NULL)
        {
            bool decoded = false;

            M_LOG("{");

            switch (v_type())
            {
                case hob::_t_u8    : decoded = decode_map<K, uint8_t    >(d, changed); break;
                case hob::_t_u16   : decoded = decode_map<K, uint16_t   >(d, changed); break;
                case hob::_t_u32   : decoded = decode_map<K, uint32_t   >(d, changed); break;
                case hob::_t_u64   : decoded = decode_map<K, uint64_t   >(d, changed); break;
                case hob::_t_i8    : decoded = decode_map<K, int8_t     >(d, changed); break;
                case hob::_t_i16   : decoded = decode_map<K, int16_t    >(d, changed); break;
                case hob::_t_i32   : decoded = decode_map<K, int32_t    >(d, changed); break;
                case hob::_t_i64   : decoded = decode_map<K, int64_t    >(d, changed); break;
                case hob::_t_bool  : decoded = decode_map<K, bool       >(d, changed); break;
                case hob::_t_f32   : decoded = decode_map<K, float      >(d, changed); break;
                case hob::_t_f64   : decoded = decode_map<K, double     >(d, changed); break;
                case hob::_t_f128  : decoded = decode_map<K, long double>(d, changed); break;
                case hob::_t_string: decoded = decode_map<K, string     >(d, changed); break;
                case hob::_t_hob   : decoded = decode_map<K, hob        >(d, changed); break;
                default:
                    {
                        M_LOG("Unknown map value type ID=%lu - type=%d", id(), v_type());
                    }
                    break;
            }

            M_LOG("} - %d", decoded);

            return decoded;
        }

        template<class T>
        bool decode_vector(hob::decoder &d, bool *changed = NULL)
        {
            vector<T> v;

            M_LOG("{");

            bool decoded = d.decode_field(v, changed);

            if (decoded)
            {
                *this = v;
            }

            M_LOG("} - %d", decoded);

            return decoded;
        }

        template<class T>
        bool decode_optional(hob::decoder &d, bool *changed = NULL)
        {
            optional<T> o;

            M_LOG("{");

            bool decoded = d.decode_field(o, changed);

            if (decoded)
            {
                *this = o;
            }

            M_LOG("} - %d", decoded);

            return decoded;
        }

        bool decode(hob::decoder &d, bool *changed = NULL)
        {
            UID     id_   = UNDEFINED;
            uint8_t type_ = 0;

            M_LOG("{");

            if (!d.decode_field(id_) || !d.decode_field(type_))
            {
                M_LOG("} - Invalid ID(%lu) or type(%u)", id_, type_);

                return false;
            }

            M_LOG("Decoding variant: ID=%lu - type=%d", id_, type_);

            _id = id_;
            _t  = type_;

            bool decoded = false;

            if (is_vector())
            {
                switch (v_type())
                {
                    case hob::_t_u8    : decoded = decode_vector<uint8_t    >(d, changed); break;
                    case hob::_t_u16   : decoded = decode_vector<uint16_t   >(d, changed); break;
                    case hob::_t_u32   : decoded = decode_vector<uint32_t   >(d, changed); break;
                    case hob::_t_u64   : decoded = decode_vector<uint64_t   >(d, changed); break;
                    case hob::_t_i8    : decoded = decode_vector<int8_t     >(d, changed); break;
                    case hob::_t_i16   : decoded = decode_vector<int16_t    >(d, changed); break;
                    case hob::_t_i32   : decoded = decode_vector<int32_t    >(d, changed); break;
                    case hob::_t_i64   : decoded = decode_vector<int64_t    >(d, changed); break;
                    case hob::_t_bool  : decoded = decode_vector<bool       >(d, changed); break;
                    case hob::_t_f32   : decoded = decode_vector<float      >(d, changed); break;
                    case hob::_t_f64   : decoded = decode_vector<double     >(d, changed); break;
                    case hob::_t_f128  : decoded = decode_vector<long double>(d, changed); break;
                    case hob::_t_string: decoded = decode_vector<string     >(d, changed); break;
                    case hob::_t_hob   : decoded = decode_vector<hob        >(d, changed); break;
                    default:
                        {
                            M_LOG("Unknown vector type: ID=%lu - type=%d", id_, type_);
                        }
                        break;
                }
            }
            else if (is_optional())
            {
                switch (v_type())
                {
                    case hob::_t_u8    : decoded = decode_optional<uint8_t    >(d, changed); break;
                    case hob::_t_u16   : decoded = decode_optional<uint16_t   >(d, changed); break;
                    case hob::_t_u32   : decoded = decode_optional<uint32_t   >(d, changed); break;
                    case hob::_t_u64   : decoded = decode_optional<uint64_t   >(d, changed); break;
                    case hob::_t_i8    : decoded = decode_optional<int8_t     >(d, changed); break;
                    case hob::_t_i16   : decoded = decode_optional<int16_t    >(d, changed); break;
                    case hob::_t_i32   : decoded = decode_optional<int32_t    >(d, changed); break;
                    case hob::_t_i64   : decoded = decode_optional<int64_t    >(d, changed); break;
                    case hob::_t_bool  : decoded = decode_optional<bool       >(d, changed); break;
                    case hob::_t_f32   : decoded = decode_optional<float      >(d, changed); break;
                    case hob::_t_f64   : decoded = decode_optional<double     >(d, changed); break;
                    case hob::_t_f128  : decoded = decode_optional<long double>(d, changed); break;
                    case hob::_t_string: decoded = decode_optional<string     >(d, changed); break;
                    case hob::_t_hob   : decoded = decode_optional<hob        >(d, changed); break;
                    default:
                        {
                            M_LOG("Unknown optional type: ID=%lu - type=%d", id_, type_);
                        }
                        break;
                }
            }
            else if (is_map())
            {
                switch (k_type())
                {
                    case hob::_t_u8    : decoded = decode_map<uint8_t    >(d, changed); break;
                    case hob::_t_u16   : decoded = decode_map<uint16_t   >(d, changed); break;
                    case hob::_t_u32   : decoded = decode_map<uint32_t   >(d, changed); break;
                    case hob::_t_u64   : decoded = decode_map<uint64_t   >(d, changed); break;
                    case hob::_t_i8    : decoded = decode_map<int8_t     >(d, changed); break;
                    case hob::_t_i16   : decoded = decode_map<int16_t    >(d, changed); break;
                    case hob::_t_i32   : decoded = decode_map<int32_t    >(d, changed); break;
                    case hob::_t_i64   : decoded = decode_map<int64_t    >(d, changed); break;
                    case hob::_t_bool  : decoded = decode_map<bool       >(d, changed); break;
                    case hob::_t_f32   : decoded = decode_map<float      >(d, changed); break;
                    case hob::_t_f64   : decoded = decode_map<double     >(d, changed); break;
                    case hob::_t_f128  : decoded = decode_map<long double>(d, changed); break;
                    case hob::_t_string: decoded = decode_map<string     >(d, changed); break;
                    case hob::_t_hob   : decoded = decode_map<hob        >(d, changed); break;
                    default:
                        {
                            M_LOG("Unknown map key type: ID=%lu - type=%d", id_, k_type());
                        }
                        break;
                }
            }
            else if (v_type() == hob::_t_string)
            {
                string s;

                decoded = d.decode_field(s, changed);

                if (decoded)
                {
                    *this = s;
                }
            }
            else if (v_type() == hob::_t_hob)
            {
                M_LOG("Decoding hob");

                hob h;

                decoded = d.decode_field(h, changed);

                if (decoded)
                {
                    *this = h;
                }
            }
            else
            {
                M_LOG("Decoding type(%u) variant", v_type());

                switch (v_type())
                {
                    case hob::_t_u8  : decoded = d.decode_field(_v.uc, changed); break;
                    case hob::_t_u16 : decoded = d.decode_field(_v.us, changed); break;
                    case hob::_t_u32 : decoded = d.decode_field(_v.ui, changed); break;
                    case hob::_t_u64 : decoded = d.decode_field(_v.ul, changed); break;
                    case hob::_t_i8  : decoded = d.decode_field(_v.sc, changed); break;
                    case hob::_t_i16 : decoded = d.decode_field(_v.ss, changed); break;
                    case hob::_t_i32 : decoded = d.decode_field(_v.si, changed); break;
                    case hob::_t_i64 : decoded = d.decode_field(_v.sl, changed); break;
                    case hob::_t_bool: decoded = d.decode_field(_v.bd, changed); break;
                    case hob::_t_f32 : decoded = d.decode_field(_v.fd, changed); break;
                    case hob::_t_f64 : decoded = d.decode_field(_v.dd, changed); break;
                    case hob::_t_f128: decoded = d.decode_field(_v.qd, changed); break;
                    default:
                        {
                            M_LOG("Unknown variant type: ID=%lu - type=%d", id_, type_);
                        }
                        break;
                }
            }

            M_LOG("} - %d", decoded);

            return decoded;
        }

    private:
        class IPointer
        {
        public:
            virtual ~IPointer() {}
            virtual size_t field_size(hob::encoder &e) = 0;
            virtual bool   encode    (hob::encoder &e) = 0;
            virtual const void *data() = 0;
        };

        class Hob : public IPointer
        {
        public:
            Hob(const hob & v): _d(v.clone()) {}
            virtual size_t field_size(hob::encoder &e) { return e.field_size(*_d); }
            virtual bool   encode    (hob::encoder &e) { return e.encode    (*_d); }
            virtual const void *data() { return static_cast<const void *>(_d); }
        private:
            hob *_d;
        };

        class String : public IPointer
        {
        public:
            String(const string & v): _d(new string(v)) {}
            virtual size_t field_size(hob::encoder &e) { return e.field_size(*_d); }
            virtual bool   encode    (hob::encoder &e) { return e.encode    (*_d); }
            virtual const void *data() { return static_cast<const void *>(_d); }
        private:
            string *_d;
        };

        template <typename T>
        class Optional: public IPointer
        {
        public:
            Optional(const optional<T> & v): _d(new optional<T>(v)) {}
            virtual size_t field_size(hob::encoder &e) { return e.field_size(*_d); }
            virtual bool   encode    (hob::encoder &e) { return e.encode    (*_d); }
            virtual const void *data() { return static_cast<const void *>(_d); }
        private:
            optional<T> *_d;
        };

        template <typename T>
        class Vector: public IPointer
        {
        public:
            Vector(const vector<T> & v): _d(new vector<T>(v)) {}
            virtual size_t field_size(hob::encoder &e) { return e.field_size(*_d); }
            virtual bool   encode    (hob::encoder &e) { return e.encode    (*_d); }
            virtual const void *data() { return static_cast<const void *>(_d); }
        private:
            vector<T> *_d;
        };

        template <typename K, typename V>
        class Map: public IPointer
        {
        public:
            Map(const map<K,V> & v): _d(new map<K,V>(v)) {}
            virtual size_t field_size(hob::encoder &e) { return e.field_size(*_d); }
            virtual bool   encode    (hob::encoder &e) { return e.encode    (*_d); }
            virtual const void *data() { return static_cast<const void *>(_d); }
        private:
            map<K,V> *_d;
        };

        union data_t
        {
            uint8_t      uc;
            uint16_t     us;
            uint32_t     ui;
            uint64_t     ul;
            int8_t       sc;
            int16_t      ss;
            int32_t      si;
            int64_t      sl;
            bool         bd;
            float        fd;
            double       dd;
            long double  qd;
            IPointer    *pd;

            char _d[sizeof(long double)];
        };

        UID     _id;
        uint8_t _t ;
        data_t  _v ;

        template<typename T>
        inline hob_t __whois() const
        {
            return (typeid(T) == typeid(uint8_t    )) ? _t_u8     :
                   (typeid(T) == typeid(uint16_t   )) ? _t_u16    :
                   (typeid(T) == typeid(uint32_t   )) ? _t_u32    :
                   (typeid(T) == typeid(uint64_t   )) ? _t_u64    :
                   (typeid(T) == typeid(int8_t     )) ? _t_i8     :
                   (typeid(T) == typeid(int16_t    )) ? _t_i16    :
                   (typeid(T) == typeid(int32_t    )) ? _t_i32    :
                   (typeid(T) == typeid(int64_t    )) ? _t_i64    :
                   (typeid(T) == typeid(bool       )) ? _t_bool   :
                   (typeid(T) == typeid(float      )) ? _t_f32    :
                   (typeid(T) == typeid(double     )) ? _t_f64    :
                   (typeid(T) == typeid(long double)) ? _t_f128   :
                   (typeid(T) == typeid(string     )) ? _t_string :
                   (typeid(T) == typeid(hob        )) ? _t_hob    :
                                                        _t_unknown;
        }

        inline uint8_t __type(hob_t h, hob_t l)
        {
            return (((static_cast<uint8_t>(h) & 0x0f) << 4)
                    |
                    ((static_cast<uint8_t>(l) & 0x0f) << 0));
        }

        inline hob_t __l_type() const
        {
            return static_cast<hob_t>((_t >> 0) & 0x0f);
        }

        inline hob_t __h_type() const
        {
            return static_cast<hob_t>((_t >> 4) & 0x0f);
        }

        inline void __clear()
        {
            if (!is_basic() && (NULL != _v.pd))
            {
                delete _v.pd;
            }

            _v.pd = NULL;
        }
    };

    typedef vector<variant> dynamic_fields_t;

    typedef dynamic_fields_t::iterator               iterator;
    typedef dynamic_fields_t::const_iterator         const_iterator;
    typedef dynamic_fields_t::reverse_iterator       reverse_iterator;
    typedef dynamic_fields_t::const_reverse_iterator const_reverse_iterator;
    typedef dynamic_fields_t::size_type              size_type;

    inline iterator               begin   ()       { return _df.begin   (); }
    inline const_iterator         begin   () const { return _df.begin   (); }
    inline iterator               end     ()       { return _df.end     (); }
    inline const_iterator         end     () const { return _df.end     (); }
    inline reverse_iterator       rbegin  ()       { return _df.rbegin  (); }
    inline const_reverse_iterator rbegin  () const { return _df.rbegin  (); }
    inline reverse_iterator       rend    ()       { return _df.rend    (); }
    inline const_reverse_iterator rend    () const { return _df.rend    (); }
    inline bool                   empty   () const { return _df.empty   (); }
    inline size_type              size    () const { return _df.size    (); }
    inline size_type              max_size() const { return _df.max_size(); }

    inline iterator find(const UID &id)
    {
        iterator it = begin();

        for (; (it != end()) && ((*it) != id); ++it);

        return it;
    }

    inline iterator find(const string &n)
    {
        iterator it = begin();

        for (; (it != end()) && ((*it) != n); ++it);

        return it;
    }

    inline const_iterator find(const UID &id) const
    {
        const_iterator cit = begin();

        for (; (cit != end()) && ((*cit) != id); ++cit);

        return cit;
    }

    inline const_iterator find(const string &n) const
    {
        const_iterator cit = begin();

        for (; (cit != end()) && ((*cit) != n); ++cit);

        return cit;
    }

    template <typename T>
    inline iterator find(const UID &id)
    {
        iterator it = find(id);

        return ((it != end()) && (static_cast<T *>(*it) != NULL)) ? it : end();
    }

    template <typename T>
    inline iterator find(const string &n)
    {
        iterator it = find(n);

        return ((it != end()) && (static_cast<T *>(*it) != NULL)) ? it : end();
    }

    template <typename T>
    inline const_iterator find(const UID &id) const
    {
        const_iterator cit = find(id);

        return ((cit != end()) && (static_cast<T *>(*cit) != NULL)) ? cit : end();
    }

    template <typename T>
    inline const_iterator find(const string &n) const
    {
        const_iterator cit = find(n);

        return ((cit != end()) && (static_cast<T *>(*cit) != NULL)) ? cit : end();
    }

    inline void clear(                             ) { _df.clear(          ); }
    inline void erase(const char   *n              ) { _df.erase(find( n)  ); }
    inline void erase(const UID    &id             ) { _df.erase(find(id)  ); }
    inline void erase(const string &n              ) { _df.erase(find( n)  ); }
    inline void erase(iterator pos                 ) { _df.erase(pos       ); }
    inline void erase(iterator first, iterator last) { _df.erase(first,last); }

    inline variant& operator[](const UID &id)
    {
        return get_or_create(id);
    }

    inline variant& operator[](const char *n)
    {
        return get_or_create(n);
    }

    inline variant& operator[](const string &n)
    {
        return get_or_create(n);
    }

    inline variant& get_or_create(const UID id)
    {
        iterator it = find(id);

        if (it == end())
        {
            variant v(id);

            it = _df.insert(end(),v);
        }

        return *it;
    }

    inline variant& get_or_create(const char *n)
    {
        return get_or_create(variant::id(n));
    }

    inline variant& get_or_create(const string &n)
    {
        return get_or_create(variant::id(n));
    }

    template<typename T>
    inline hob & set(const string &n, const T &v)
    {
        variant & v_ = get_or_create(n);

        v_ = v;

        return *this;
    }

    template<typename T>
    inline hob & set(const UID &id, const T &v)
    {
        variant & v_ = get_or_create(id);

        v_ = v;

        return *this;
    }

    template<typename T>
    inline hob & set(const char *n, const T &v)
    {
        variant & v_ = get_or_create(n);

        v_ = v;

        return *this;
    }

    inline bool has(const string & n ) const { return (find( n) != end()); }
    inline bool has(const UID    & id) const { return (find(id) != end()); }
    inline bool has(const char   * n ) const { return (find( n) != end()); }

    template<typename T>
    inline bool has(const string & n ) const { return (find<T>( n) != end()); }

    template<typename T>
    inline bool has(const UID    & id) const { return (find<T>(id) != end()); }

    template<typename T>
    inline bool has(const char   * n ) const { return (find<T>( n) != end()); }

    template<typename T>
    inline const T * get(const string & n) const
    {
        const const_iterator cit = find(n);

        if (cit == end())
        {
            return NULL;
        }

        return (*cit);
    }

    template<typename T>
    inline const T * get(const UID & id) const
    {
        const const_iterator cit = find(id);

        if (cit == end())
        {
            return NULL;
        }

        return (*cit);
    }

    template<typename T>
    inline const T * get(const char * n ) const
    {
        const const_iterator cit = find(n);

        if (cit == end())
        {
            return NULL;
        }

        return (*cit);
    }

    template<typename T>
    inline bool get(const string & n, T & v) const
    {
        const const_iterator cit = find(n);

        if (cit == end())
        {
            return false;
        }

        const T *rv = (*cit);

        if (NULL == rv)
        {
            return false;
        }

        v = *rv;

        return true;
    }

    template<typename T>
    inline bool get(const UID & id, T & v) const
    {
        const const_iterator cit = find(id);

        if (cit == end())
        {
            return false;
        }

        const T *rv = (*cit);

        if (NULL == rv)
        {
            return false;
        }

        v = *rv;

        return true;
    }

    template<typename T>
    inline bool get(const char * n, T & v) const
    {
        const const_iterator cit = find(n);

        if (cit == end())
        {
            return false;
        }

        const T *rv = (*cit);

        if (NULL == rv)
        {
            return false;
        }

        v = *rv;

        return true;
    }
#endif // ENABLE_DYNAMIC_FIELDS

    ///////////////////////////////////////////////////////////////////////////
    //                                                                       //
    //                          HOB implementation                           //
    //                                                                       //
    ///////////////////////////////////////////////////////////////////////////

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

        virtual size_t field_size(const uint8_t     &v) = 0;
        virtual size_t field_size(const uint16_t    &v) = 0;
        virtual size_t field_size(const uint32_t    &v) = 0;
        virtual size_t field_size(const uint64_t    &v) = 0;
        virtual size_t field_size(const int8_t      &v) = 0;
        virtual size_t field_size(const int16_t     &v) = 0;
        virtual size_t field_size(const int32_t     &v) = 0;
        virtual size_t field_size(const int64_t     &v) = 0;
        virtual size_t field_size(const bool        &v) = 0;
        virtual size_t field_size(const float       &v) = 0;
        virtual size_t field_size(const double      &v) = 0;
        virtual size_t field_size(const long double &v) = 0;

#if defined(ENABLE_DYNAMIC_FIELDS)
        inline size_t field_size(const variant &v)
        {
            return v.field_size(*this);
        }
#endif // ENABLE_DYNAMIC_FIELDS

        inline size_t field_size(const string &v)
        {
            return field_size(v.size()) + v.size();
        }

        inline size_t field_size(size_t size_, const void *v)
        {
            (void)v;

            return field_size(size_) + size_;
        }

        size_t field_size(const hob &v)
        {
            return v.size(*this);
        }

        template<size_t N>
        size_t field_size(const std::bitset<N>& v)
        {
            (void)v;

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
#if defined(ENABLE_DYNAMIC_FIELDS)
        friend class variant;
#endif // ENABLE_DYNAMIC_FIELDS

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

#if defined(ENABLE_DYNAMIC_FIELDS)
        bool encode(const variant &v)
        {
            return v.encode(*this);
        }

        virtual bool encode_variant_begin(UID id, uint8_t type) = 0;
        virtual bool encode_variant_end() = 0;
#endif // ENABLE_DYNAMIC_FIELDS

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
            (void)enc;

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

#if defined(ENABLE_DYNAMIC_FIELDS)
        bool decode_field(variant &v, bool *changed = NULL)
        {
            M_LOG("{");

            bool retval = v.decode(*this, changed);

            M_LOG("} - %d", retval);

            return retval;
        }
#endif // ENABLE_DYNAMIC_FIELDS

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

    virtual hob * clone() const
    {
        return new hob(*this);
    }

    hob & operator=(const hob & ref)
    {
        _id = ref._id;
        _is = ref._is;
        _sp = ref._sp;
        _ep = ref._ep;
#if defined(ENABLE_DYNAMIC_FIELDS)
        _df = ref._df;
#endif // ENABLE_DYNAMIC_FIELDS

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
                os.encode_header(static_cast<const char *>(NULL),
                                 static_cast<const char *>(NULL),
                                 __get_id(),
                                 __payload(os))
                &&
                __encode_head(os)
                &&
                __encode_tail(os)
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

#if defined(ENABLE_DYNAMIC_FIELDS)
    inline bool operator<(const hob &ref) const
    {
        bool rv = (_id < ref._id);

        return rv;
    }
#endif

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

        return os.field_size(__get_id())
               +
               ((sz > 0) ? (os.field_size(sz) + sz) : 0);
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
        UID id_;

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

        if (update && !is.bufferize(sz_))
        {
            return false;
        }

        _is = &is;
        _sp = is.tell();
        _ep = _sp + sz_;
        _id = id_;

        return (__decode_head() && __decode_tail(false));
    }

#if defined(ENABLE_DYNAMIC_FIELDS)
    inline UID __get_id() const
    {
        return _id | ( !_df.empty() << _DYNFLDS_FLAG_POS_ );
    }
#else // !ENABLE_DYNAMIC_FIELDS
    inline const UID & __get_id() const
    {
        return _id;
    }
#endif // ENABLE_DYNAMIC_FIELDS

    inline void __update_id(const UID &in)
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
#if defined(ENABLE_DYNAMIC_FIELDS)
            // Reserve bit for dynamic fields flag
            //
            _id <<= 2;
#else // !ENABLE_DYNAMIC_FIELDS
            _id <<= 1;
#endif // ENABLE_DYNAMIC_FIELDS

            // HOBs with    parameters have an odd  ID
            // HOBs without parameters have an even ID
            //
            _id |= (_np > 0);

            // Avoid parameters check multiple appliance
            //
            _np = -1;
        }
    }

    size_t __payload(hob::encoder &os) const
    {
#if defined(ENABLE_DYNAMIC_FIELDS)
        return __static_payload(os) + __dynamic_payload(os);
#else // !ENABLE_DYNAMIC_FIELDS
        return __static_payload(os);
#endif // ENABLE_DYNAMIC_FIELDS
    }

    virtual size_t __static_payload(hob::encoder &os) const
    {
        (void)os;

        return 0;
    }

    inline bool __has_payload(const UID &id)
    {
        return __has_static_payload(id)
#if defined(ENABLE_DYNAMIC_FIELDS)
               ||
               __has_dynamic_payload(id)
#endif // ENABLE_DYNAMIC_FIELDS
               ;
    }

    inline bool __has_static_payload(const UID &id)
    {
        return ( ( id >> _PAYLOAD_FLAG_POS_ ) & 1 );
    }

#if defined(ENABLE_DYNAMIC_FIELDS)
    inline bool __has_dynamic_payload(const UID &id)
    {
        return ( ( id >> _DYNFLDS_FLAG_POS_ ) & 1 );
    }

    size_t __dynamic_payload(hob::encoder &os) const
    {
        return (_df.empty()) ? 0 : os.field_size(_df);
    }

    bool __encode_dynamic_fields(hob::encoder &os) const
    {
        return (_df.empty()) ? true : os.encode_field(_df, "variant");
    }

    bool __decode_dynamic_fields()
    {
        M_LOG("{");

        bool retval = ( !__has_dynamic_payload(_id)
                        ||
                        (NULL == _is)
                        ||
                        _is->decode_field(_df) );

        M_LOG("} - %d", retval);

        return retval;
    }
#endif // ENABLE_DYNAMIC_FIELDS

    bool __decode_head()
    {
        return true;
    }

    bool __decode_tail(bool force_flush_pending=false)
    {
        (void)force_flush_pending;

#if defined(ENABLE_DYNAMIC_FIELDS)
        if (!__decode_dynamic_fields())
        {
            return false;
        }
#else // ENABLE_DYNAMIC_FIELDS
        if (force_flush_pending)
        {
            __flush_pending();
        }
#endif // ENABLE_DYNAMIC_FIELDS

        return true;
    }

    bool __encode_head(encoder &os) const
    {
        return true;
    }

    bool __encode_tail(encoder &os) const
    {
#if defined(ENABLE_DYNAMIC_FIELDS)
        if (!__encode_dynamic_fields(os))
        {
            return false;
        }
#endif // ENABLE_DYNAMIC_FIELDS

        return os.encode_footer();
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

#if defined(ENABLE_DYNAMIC_FIELDS)
    dynamic_fields_t _df;
#endif // ENABLE_DYNAMIC_FIELDS
};

inline bool operator<<(hob::encoder &e, hob &h) { return h >> e; }
inline bool operator>>(hob::decoder &d, hob &h) { return h << d; }
inline bool operator>>(hob          &f, hob &t) { return t << f; }

#define SCAN_FIELDS(m, ...) \
    SCAN_FIELDS_I(m PP_CAT(FOR_EACH_FIELD_0 __VA_ARGS__, _END))

#define FOR_EACH_FIELD_0(...) \
    PP_DEFER1(PP_COMMA)()(__VA_ARGS__) FOR_EACH_FIELD_1

#define FOR_EACH_FIELD_1(...) \
    PP_DEFER1(PP_COMMA)()(__VA_ARGS__) FOR_EACH_FIELD_0

#define FOR_EACH_FIELD_0_END
#define FOR_EACH_FIELD_1_END

#define SCAN_FIELDS_I(...) \
    SCAN_FIELDS_II(__VA_ARGS__)

#define SCAN_FIELDS_II(m, ...) \
    PP_IF(PP_HAS_ARGS(__VA_ARGS__))(PP_EVAL(SCAN_FIELDS_INNER(m, __VA_ARGS__)))

#define SCAN_FIELDS_INNER(op, cur_val, ...) \
    op cur_val                              \
    PP_IF(PP_HAS_ARGS(__VA_ARGS__))(PP_DEFER2(SCAN_FIELDS_INNER_I)()(op, ## __VA_ARGS__))

#define SCAN_FIELDS_INNER_I() SCAN_FIELDS_INNER

#define DECLARE_ENUM(t, n, ...)  _ ## n,
#define DECLARE_FIELD(t, n, ...) t n;
#define INIT_FIELD(t, n, ...)    PP_IF(PP_HAS_ARGS(__VA_ARGS__) )(n = __VA_ARGS__;)
#define DECODE_FIELD(t, n, ...)  && is.decode_field(n, &c) \
                                 && __set_changed(_ ## n, c)
#define ENCODE_FIELD(t, n, ...)  && os.encode_field(n, PP_STR(t), PP_STR(n))
#define FIELD_SIZE(t, n, ...)    + os.field_size(n)
#define COMPARE_FIELD(t, n, ...) && (n == ref.n)
#define CLONE_FIELD(t, n, ...)   n = ref.n;
#define UPDATE_NP(t, n, ...)     __update_id("");
#define UPDATE_ID(t, n, ...)     HUPDATE(PP_STR(name_)); \
                                 HUPDATE(PP_STR(t    )); \
                                 HUPDATE(PP_STR(n    ));

#define CHANGED_FIELDS(n, ...) \
    PP_IF(PP_HAS_ARGS(__VA_ARGS__))(PP_EVAL(CHANGED_FIELDS_INNER(n, __VA_ARGS__)))

#define CHANGED_FIELDS_INNER(n, ...)                                              \
    bitset<_FIELDS_COUNT_> __ ## n ## __changed__fields__;                        \

#define RESET_CHANGES(n,f)                                                        \
    if (f == _FIELDS_COUNT_)                                                      \
    {                                                                             \
        __ ## n ## __changed__fields__.reset();                                   \
    }                                                                             \
    else                                                                          \
    if (f < _FIELDS_COUNT_)                                                       \
    {                                                                             \
        __ ## n ## __changed__fields__.reset(f);                                  \
    }                                                                             \

#define CHECK_CHANGES(n,f)                                                        \
    if (f == _FIELDS_COUNT_)                                                      \
    {                                                                             \
        return __ ## n ## __changed__fields__.any();                              \
    }                                                                             \
    else                                                                          \
    if (f < _FIELDS_COUNT_)                                                       \
    {                                                                             \
        return __ ## n ## __changed__fields__.test(f);                            \
    }                                                                             \

#define SET_CHANGED(n,f,v)                                                        \
    if (f < _FIELDS_COUNT_)                                                       \
    {                                                                             \
        __ ## n ## __changed__fields__[f] = v;                                    \
    }                                                                             \

#define HOBSTRUCT(name_, value_, ...)                                             \
class name_                                                                       \
    : public hob                                                                  \
{                                                                                 \
public:                                                                           \
    enum Fields                                                                   \
    {                                                                             \
        SCAN_FIELDS(DECLARE_ENUM, PP_FIRST(__VA_ARGS__))                          \
        SCAN_FIELDS(DECLARE_ENUM, PP_REMAIN(__VA_ARGS__))                         \
        _FIELDS_COUNT_                                                            \
    };                                                                            \
                                                                                  \
    SCAN_FIELDS(DECLARE_FIELD, PP_FIRST(__VA_ARGS__))                             \
    SCAN_FIELDS(DECLARE_FIELD, PP_REMAIN(__VA_ARGS__))                            \
                                                                                  \
    name_() : hob(value_)                                                         \
    {                                                                             \
        SCAN_FIELDS(INIT_FIELD, PP_FIRST(__VA_ARGS__))                            \
        SCAN_FIELDS(INIT_FIELD, PP_REMAIN(__VA_ARGS__))                           \
                                                                                  \
        /* ID is evaluated on mandatory fields only */                            \
        SCAN_FIELDS(UPDATE_ID, PP_FIRST(__VA_ARGS__))                             \
                                                                                  \
        /* Extra fields update parameters count */                                \
        SCAN_FIELDS(UPDATE_NP, PP_REMAIN(__VA_ARGS__))                            \
                                                                                  \
        __update_id(static_cast<const char *>(NULL)); /* finalize hob ID */       \
    }                                                                             \
                                                                                  \
    name_(const hob & ref): hob(ref)                                              \
    {                                                                             \
        *this = ref;                                                              \
    }                                                                             \
                                                                                  \
    name_(const name_ & ref): hob(ref)                                            \
    {                                                                             \
        *this = ref;                                                              \
    }                                                                             \
                                                                                  \
    ~name_()                                                                      \
    {                                                                             \
    }                                                                             \
                                                                                  \
    virtual name_ * clone() const                                                 \
    {                                                                             \
        return new name_(*this);                                                  \
    }                                                                             \
                                                                                  \
    name_ & operator=(const hob & ref)                                            \
    {                                                                             \
        *static_cast<hob *>(this) = static_cast<const hob &>(ref);                \
                                                                                  \
        return *this;                                                             \
    }                                                                             \
                                                                                  \
    name_ & operator=(const name_ & ref)                                          \
    {                                                                             \
        *static_cast<hob *>(this) = static_cast<const hob &>(ref);                \
                                                                                  \
        SCAN_FIELDS(CLONE_FIELD, PP_FIRST(__VA_ARGS__))                           \
        SCAN_FIELDS(CLONE_FIELD, PP_REMAIN(__VA_ARGS__))                          \
                                                                                  \
        return *this;                                                             \
    }                                                                             \
                                                                                  \
    bool operator==(const name_ &ref) const                                       \
    {                                                                             \
        return ((*static_cast<const hob*>(this) == static_cast<const hob &>(ref)) \
                 SCAN_FIELDS(COMPARE_FIELD, PP_FIRST(__VA_ARGS__))                \
                 SCAN_FIELDS(COMPARE_FIELD, PP_REMAIN(__VA_ARGS__)));             \
    }                                                                             \
                                                                                  \
    void operator-=(const Fields & f)                                             \
    {                                                                             \
        __reset_changes(f);                                                       \
    }                                                                             \
                                                                                  \
    bool operator&(const Fields &f) const                                         \
    {                                                                             \
        return __is_changed(f);                                                   \
    }                                                                             \
                                                                                  \
    virtual bool operator>>(hob::encoder &os) const                               \
    {                                                                             \
        size_t payload = __payload(os);                                           \
                                                                                  \
        return                                                                    \
        (                                                                         \
            (UNDEFINED == __get_id())                                             \
            ||                                                                    \
            (                                                                     \
                os.encode_header(PP_STR(name_),                                   \
                                 value_,                                          \
                                 __get_id(),                                      \
                                 payload)                                         \
                &&                                                                \
                hob::__encode_head(os)                                            \
                SCAN_FIELDS(ENCODE_FIELD, PP_FIRST(__VA_ARGS__))                  \
                SCAN_FIELDS(ENCODE_FIELD, PP_REMAIN(__VA_ARGS__))                 \
                &&                                                                \
                hob::__encode_tail(os)                                            \
            )                                                                     \
        );                                                                        \
    }                                                                             \
                                                                                  \
protected:                                                                        \
    void __reset_changes(const Fields & f)                                        \
    {                                                                             \
        (void)f;                                                                  \
                                                                                  \
        PP_IF(PP_HAS_ARGS(__VA_ARGS__))(RESET_CHANGES(name_,f))                   \
    }                                                                             \
                                                                                  \
    bool __is_changed(const Fields &f) const                                      \
    {                                                                             \
        (void)f;                                                                  \
                                                                                  \
        PP_IF(PP_HAS_ARGS(__VA_ARGS__))(CHECK_CHANGES(name_,f))                   \
                                                                                  \
        return false;                                                             \
    }                                                                             \
                                                                                  \
    virtual void __reset_changes()                                                \
    {                                                                             \
        __reset_changes(_FIELDS_COUNT_);                                          \
    }                                                                             \
                                                                                  \
    virtual bool __is_changed() const                                             \
    {                                                                             \
        return __is_changed(_FIELDS_COUNT_);                                      \
    }                                                                             \
                                                                                  \
    bool __decode(hob &ref)                                                       \
    {                                                                             \
        hob::decoder *is = static_cast<hob::decoder*>(ref);                       \
                                                                                  \
        if (NULL == is)                                                           \
        {                                                                         \
            return false;                                                         \
        }                                                                         \
                                                                                  \
        __reset_changes();                                                        \
                                                                                  \
        ref.__rewind();                                                           \
                                                                                  \
        if (!__decode(*is))                                                       \
        {                                                                         \
            return false;                                                         \
        }                                                                         \
                                                                                  \
        *this = ref;                                                              \
                                                                                  \
        return true;                                                              \
    }                                                                             \
                                                                                  \
    bool __decode(hob::decoder &is, bool update=true)                             \
    {                                                                             \
        (void)is;                                                                 \
        (void)update;                                                             \
                                                                                  \
        bool c = false;                                                           \
                                                                                  \
        (void)c;                                                                  \
                                                                                  \
        /* Read non optional fields : fail on error */                            \
                                                                                  \
        if (hob::__decode_head() &&                                               \
            (true SCAN_FIELDS(DECODE_FIELD, PP_FIRST(__VA_ARGS__))))              \
        {                                                                         \
            /* Read optional fields : ignore errors */                            \
                                                                                  \
            if ((true SCAN_FIELDS(DECODE_FIELD, PP_REMAIN(__VA_ARGS__))) || true) \
            {                                                                     \
                return hob::__decode_tail(true);                                  \
            }                                                                     \
        }                                                                         \
                                                                                  \
        return false;                                                             \
    }                                                                             \
                                                                                  \
    virtual bool __set_changed(ssize_t f, bool v)                                 \
    {                                                                             \
        (void)f;                                                                  \
        (void)v;                                                                  \
                                                                                  \
        if (f < 0)                                                                \
        {                                                                         \
            return false;                                                         \
        }                                                                         \
                                                                                  \
        PP_IF(PP_HAS_ARGS(__VA_ARGS__))(SET_CHANGED(name_,f,v))                   \
                                                                                  \
        return true;                                                              \
    }                                                                             \
                                                                                  \
    virtual void __flush_pending()                                                \
    {                                                                             \
    }                                                                             \
                                                                                  \
    virtual size_t __static_payload(hob::encoder &os) const                       \
    {                                                                             \
        (void)os;                                                                 \
                                                                                  \
        return (0                                                                 \
                SCAN_FIELDS(FIELD_SIZE, PP_FIRST(__VA_ARGS__))                    \
                SCAN_FIELDS(FIELD_SIZE, PP_REMAIN(__VA_ARGS__)));                 \
    }                                                                             \
                                                                                  \
private:                                                                          \
    CHANGED_FIELDS(name_, __VA_ARGS__)                                            \
};

#endif // __HOB_HPP__
