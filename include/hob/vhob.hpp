#if !defined(__VHOB_HPP__)
#define __VHOB_HPP__
#include "hob/hob.hpp"
#include "hob/std/type_traits.hpp"
#include <typeinfo>

class vhob : public hob
{
public:
    ///////////////////////////////////////////////////////////////////////////
    //                                                                       //
    //                              HOB interface                            //
    //                                                                       //
    ///////////////////////////////////////////////////////////////////////////

    vhob(): hob() { }

    vhob(const hobio::UID &id_): hob(id_) { }
    vhob(const char       *id_): hob(id_) { }
    vhob(const string     &id_): hob(id_) { }
    vhob(const hob        &ref): hob(ref) { *this = ref; }
    vhob(const vhob       &ref): hob(ref) { *this = ref; }

    virtual ~vhob()
    {
    }

    virtual vhob * clone() const
    {
        return new vhob(*this);
    }

    vhob & operator=(const hob & ref)
    {
        *static_cast<hob *>(this) = static_cast<const hob &>(ref);

        return *this;
    }

    vhob & operator=(const vhob & ref)
    {
        *static_cast<hob *>(this) = static_cast<const hob &>(ref);

        _df = ref._df;

        return *this;
    }

    bool operator==(const vhob &ref) const
    {
        return ((*static_cast<const hob*>(this) == static_cast<const hob &>(ref))
                 &&
                 (_df == ref._df));
    }

    bool operator==(const hob &ref) const
    {
        return !__ne(ref.__get_id());
    }

    virtual bool operator!=(const hob &ref) const
    {
        return __ne(ref.__get_id());
    }

    inline bool operator<(const vhob &ref) const
    {
        return __lt(ref.__get_id());
    }

    virtual bool operator>>(hobio::encoder &os) const
    {
        // M_LOG("{");

        if (hobio::UNDEFINED == __get_id())
        {
            // M_LOG("} - true");

            return true;
        }

        if (!os.encode_header(static_cast<const char *>(NULL),
                              static_cast<const char *>(NULL),
                              ( __get_id() & ~1 ) | __has_payload(),
                              __payload(os)))
        {
            // M_LOG("} - true");

            return false;
        }

        if ( __has_payload() && !os.encode_field(_df, "variant") )
        {
            // M_LOG("} - false");

            return false;
        }

        if (!os.encode_footer())
        {
            // M_LOG("} - false");

            return false;
        }

        // M_LOG("} - true");

        return true;

/*
        return
        (
            (hobio::UNDEFINED == __get_id())
            ||
            (
                os.encode_header(static_cast<const char *>(NULL),
                                 static_cast<const char *>(NULL),
                                 ( __get_id() & ~1 ) | __has_payload(),
                                 __payload(os))
                &&
                ( !__has_payload() || os.encode_field(_df, "variant") )
                &&
                os.encode_footer()
            )
        );
*/
    }

    inline bool operator<(const hobio::UID &id) const
    {
        return __lt(id);
    }

    inline bool operator!=(const hobio::UID &id) const
    {
        return __ne(id);
    }

    inline operator bool()
    {
        return true;
    }

    inline void operator~()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    //                                                                       //
    //                     Dynamic fields interface                          //
    //                                                                       //
    ///////////////////////////////////////////////////////////////////////////

    class variant : public hobio::codec
    {
    public:
        enum var_t
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

        variant(): _id(hobio::UNDEFINED), _t(_t_unknown) { _v.pd = NULL; }

        variant(const hobio::UID & in): _t(_t_unknown) { _v.pd = NULL; _id = in    ; }
        variant(const char       * in): _t(_t_unknown) { _v.pd = NULL; _id = id(in); }
        variant(const string     & in): _t(_t_unknown) { _v.pd = NULL; _id = id(in); }

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
            // M_LOG("{");

            __clear();

            _t = __type(__whois<K>(), __whois<V>());

            // M_LOG("- %d", _t);

            if (v_type() != _t_unknown)
            {
                // M_LOG("-");

                _v.pd = new Map<K,V>(m);
            }

            // M_LOG("}");

            return *this;
        }

        template<class T>
        inline variant & operator=(const optional<T> & v)
        {
            // M_LOG("{");

            __clear();

            _t = __type(__whois<T>(), _t_special);

            // M_LOG("- %d", _t);

            if (v_type() != _t_unknown)
            {
                // M_LOG("-");

                _v.pd = new Optional<T>(v);
            }

            // M_LOG("}");

            return *this;
        }

        template<class T>
        inline variant & operator=(const vector<T> & v)
        {
            // M_LOG("{");

            __clear();

            _t = __type(_t_special, __whois<T>());

            // M_LOG("- %d", _t);

            if (v_type() != _t_unknown)
            {
                // M_LOG("-");

                _v.pd = new Vector<T>(v);
            }

            // M_LOG("}");

            return *this;
        }

        inline variant & operator=(const string & v)
        {
            // M_LOG("{");

            __clear();

            _t = __type(_t_unknown, __whois<string>());

            // M_LOG("- %d", _t);

            if (v_type() != _t_unknown)
            {
                // M_LOG("-");

                _v.pd = new String(v);
            }

            // M_LOG("}");

            return *this;
        }

        inline variant & operator=(const vhob & v)
        {
            // M_LOG("{");

            __clear();

            _t = __type(_t_unknown, __whois<vhob>());

            // M_LOG("- %d", _t);

            if (v_type() != _t_unknown)
            {
                // M_LOG("-");

                _v.pd = new Hob(v);
            }

            // M_LOG("}");

            return *this;
        }

        template<typename T>
        inline variant & operator=(const T & v)
        {
            // M_LOG("{");

            const void *p = &v;

            __clear();

            _t = __type(_t_unknown, __whois<T>());

            // M_LOG("- %d", _t);

            switch (_t)
            {
                case _t_u8    : _v.uc = *static_cast<const uint8_t     *>(p); break;
                case _t_u16   : _v.us = *static_cast<const uint16_t    *>(p); break;
                case _t_u32   : _v.ui = *static_cast<const uint32_t    *>(p); break;
                case _t_u64   : _v.ul = *static_cast<const uint64_t    *>(p); break;
                case _t_i8    : _v.sc = *static_cast<const int8_t      *>(p); break;
                case _t_i16   : _v.ss = *static_cast<const int16_t     *>(p); break;
                case _t_i32   : _v.si = *static_cast<const int32_t     *>(p); break;
                case _t_i64   : _v.sl = *static_cast<const int64_t     *>(p); break;
                case _t_bool  : _v.bd = *static_cast<const bool        *>(p); break;
                case _t_f32   : _v.fd = *static_cast<const float       *>(p); break;
                case _t_f64   : _v.dd = *static_cast<const double      *>(p); break;
                case _t_f128  : _v.qd = *static_cast<const long double *>(p); break;
                case _t_hob   : _v.pd = new Hob                          (v); break;
                default: break;
            }

            // M_LOG("}");

            return *this;
        }

        inline const hobio::UID & id  () const { return _id; }

        inline uint8_t            type() const { return _t ; }

        inline var_t v_type() const
        {
            return (__l_type() == _t_special) ? __h_type() : __l_type();
        }

        inline var_t k_type() const
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

        static hobio::UID id(const string &in)
        {
            return id(in.c_str());
        }

        static hobio::UID id(const char *in)
        {
            hobio::UID id_ = 0;

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

        virtual size_t size(hobio::encoder &e) const
        {
            if (type() == _t_unknown)
            {
                return 0;
            }

            size_t retval = e.field_size(id())
                            +
                            e.field_size(type());

            if (!is_basic()
                ||
                (v_type() == _t_string)
                ||
                (v_type() == _t_hob))
            {
                if (NULL == _v.pd)
                {
                    return 0;
                }

                retval += _v.pd->field_size(e);
            }
            else switch (v_type())
            {
                case _t_u8  : retval += e.field_size(_v.uc); break;
                case _t_u16 : retval += e.field_size(_v.us); break;
                case _t_u32 : retval += e.field_size(_v.ui); break;
                case _t_u64 : retval += e.field_size(_v.ul); break;
                case _t_i8  : retval += e.field_size(_v.sc); break;
                case _t_i16 : retval += e.field_size(_v.ss); break;
                case _t_i32 : retval += e.field_size(_v.si); break;
                case _t_i64 : retval += e.field_size(_v.sl); break;
                case _t_bool: retval += e.field_size(_v.bd); break;
                case _t_f32 : retval += e.field_size(_v.fd); break;
                case _t_f64 : retval += e.field_size(_v.dd); break;
                case _t_f128: retval += e.field_size(_v.qd); break;

                default:
                {
                    return 0;
                }
                break;
            }

            return retval;
        }

    protected:
        virtual bool encode(hobio::encoder &e) const
        {
            // M_LOG("{");

            if (type() == _t_unknown)
            {
                // M_LOG("} - false");

                return false;
            }

            if (!e.encode_variant_begin(id(), type()))
            {
                // M_LOG("} - false");

                return false;
            }

            bool encoded = false;

            if (!is_basic()
                ||
                (v_type() == _t_string)
                ||
                (v_type() == _t_hob))
            {
                // M_LOG("{ - %s",
                      // (v_type() == _t_string) ? "string"   :
                      // (v_type() == _t_hob   ) ? "vhob"      :
                      // is_vector  ()           ? "vector"   :
                      // is_optional()           ? "optional" :
                      // is_map     ()           ? "map"      :
                                                // "unknown"  );

                if (NULL != _v.pd)
                {
                    // M_LOG("{");

                    encoded = _v.pd->encode(e);

                    // M_LOG("} - encoded: %s", encoded ? "true" : "false");
                }

                // M_LOG("}");
            }
            else switch (v_type())
            {
                case _t_u8  : encoded = e.encode(_v.uc); break;
                case _t_u16 : encoded = e.encode(_v.us); break;
                case _t_u32 : encoded = e.encode(_v.ui); break;
                case _t_u64 : encoded = e.encode(_v.ul); break;
                case _t_i8  : encoded = e.encode(_v.sc); break;
                case _t_i16 : encoded = e.encode(_v.ss); break;
                case _t_i32 : encoded = e.encode(_v.si); break;
                case _t_i64 : encoded = e.encode(_v.sl); break;
                case _t_bool: encoded = e.encode(_v.bd); break;
                case _t_f32 : encoded = e.encode(_v.fd); break;
                case _t_f64 : encoded = e.encode(_v.dd); break;
                case _t_f128: encoded = e.encode(_v.qd); break;
                default:
                    break;
            }

            if (!encoded)
            {
                // M_LOG("} - false");

                return false;
            }

            if (!e.encode_variant_end())
            {
                // M_LOG("} - false");

                return false;
            }

            // M_LOG("} - true");

            return true;
        }

        template<class K, class V>
        bool decode_map(hobio::decoder &d, bool *changed = NULL)
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
        bool decode_map(hobio::decoder &d, bool *changed = NULL)
        {
            bool decoded = false;

            M_LOG("{");

            switch (v_type())
            {
                case _t_u8    : decoded = decode_map<K, uint8_t    >(d, changed); break;
                case _t_u16   : decoded = decode_map<K, uint16_t   >(d, changed); break;
                case _t_u32   : decoded = decode_map<K, uint32_t   >(d, changed); break;
                case _t_u64   : decoded = decode_map<K, uint64_t   >(d, changed); break;
                case _t_i8    : decoded = decode_map<K, int8_t     >(d, changed); break;
                case _t_i16   : decoded = decode_map<K, int16_t    >(d, changed); break;
                case _t_i32   : decoded = decode_map<K, int32_t    >(d, changed); break;
                case _t_i64   : decoded = decode_map<K, int64_t    >(d, changed); break;
                case _t_bool  : decoded = decode_map<K, bool       >(d, changed); break;
                case _t_f32   : decoded = decode_map<K, float      >(d, changed); break;
                case _t_f64   : decoded = decode_map<K, double     >(d, changed); break;
                case _t_f128  : decoded = decode_map<K, long double>(d, changed); break;
                case _t_string: decoded = decode_map<K, string     >(d, changed); break;
                case _t_hob   : decoded = decode_map<K, vhob        >(d, changed); break;
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
        bool decode_vector(hobio::decoder &d, bool *changed = NULL)
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
        bool decode_optional(hobio::decoder &d, bool *changed = NULL)
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

        virtual bool decode(hobio::decoder &d, bool *changed = NULL)
        {
            hobio::UID id_   = hobio::UNDEFINED;
            uint8_t    type_ = 0;

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
                    case _t_u8    : decoded = decode_vector<uint8_t    >(d, changed); break;
                    case _t_u16   : decoded = decode_vector<uint16_t   >(d, changed); break;
                    case _t_u32   : decoded = decode_vector<uint32_t   >(d, changed); break;
                    case _t_u64   : decoded = decode_vector<uint64_t   >(d, changed); break;
                    case _t_i8    : decoded = decode_vector<int8_t     >(d, changed); break;
                    case _t_i16   : decoded = decode_vector<int16_t    >(d, changed); break;
                    case _t_i32   : decoded = decode_vector<int32_t    >(d, changed); break;
                    case _t_i64   : decoded = decode_vector<int64_t    >(d, changed); break;
                    case _t_bool  : decoded = decode_vector<bool       >(d, changed); break;
                    case _t_f32   : decoded = decode_vector<float      >(d, changed); break;
                    case _t_f64   : decoded = decode_vector<double     >(d, changed); break;
                    case _t_f128  : decoded = decode_vector<long double>(d, changed); break;
                    case _t_string: decoded = decode_vector<string     >(d, changed); break;
                    case _t_hob   : decoded = decode_vector<vhob        >(d, changed); break;
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
                    case _t_u8    : decoded = decode_optional<uint8_t    >(d, changed); break;
                    case _t_u16   : decoded = decode_optional<uint16_t   >(d, changed); break;
                    case _t_u32   : decoded = decode_optional<uint32_t   >(d, changed); break;
                    case _t_u64   : decoded = decode_optional<uint64_t   >(d, changed); break;
                    case _t_i8    : decoded = decode_optional<int8_t     >(d, changed); break;
                    case _t_i16   : decoded = decode_optional<int16_t    >(d, changed); break;
                    case _t_i32   : decoded = decode_optional<int32_t    >(d, changed); break;
                    case _t_i64   : decoded = decode_optional<int64_t    >(d, changed); break;
                    case _t_bool  : decoded = decode_optional<bool       >(d, changed); break;
                    case _t_f32   : decoded = decode_optional<float      >(d, changed); break;
                    case _t_f64   : decoded = decode_optional<double     >(d, changed); break;
                    case _t_f128  : decoded = decode_optional<long double>(d, changed); break;
                    case _t_string: decoded = decode_optional<string     >(d, changed); break;
                    case _t_hob   : decoded = decode_optional<vhob        >(d, changed); break;
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
                    case _t_u8    : decoded = decode_map<uint8_t    >(d, changed); break;
                    case _t_u16   : decoded = decode_map<uint16_t   >(d, changed); break;
                    case _t_u32   : decoded = decode_map<uint32_t   >(d, changed); break;
                    case _t_u64   : decoded = decode_map<uint64_t   >(d, changed); break;
                    case _t_i8    : decoded = decode_map<int8_t     >(d, changed); break;
                    case _t_i16   : decoded = decode_map<int16_t    >(d, changed); break;
                    case _t_i32   : decoded = decode_map<int32_t    >(d, changed); break;
                    case _t_i64   : decoded = decode_map<int64_t    >(d, changed); break;
                    case _t_bool  : decoded = decode_map<bool       >(d, changed); break;
                    case _t_f32   : decoded = decode_map<float      >(d, changed); break;
                    case _t_f64   : decoded = decode_map<double     >(d, changed); break;
                    case _t_f128  : decoded = decode_map<long double>(d, changed); break;
                    case _t_string: decoded = decode_map<string     >(d, changed); break;
                    case _t_hob   : decoded = decode_map<vhob        >(d, changed); break;
                    default:
                        {
                            M_LOG("Unknown map key type: ID=%lu - type=%d", id_, k_type());
                        }
                        break;
                }
            }
            else if (v_type() == _t_string)
            {
                string s;

                decoded = d.decode_field(s, changed);

                if (decoded)
                {
                    *this = s;
                }
            }
            else if (v_type() == _t_hob)
            {
                M_LOG("Decoding vhob");

                vhob h;

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
                    case _t_u8  : decoded = d.decode_field(_v.uc, changed); break;
                    case _t_u16 : decoded = d.decode_field(_v.us, changed); break;
                    case _t_u32 : decoded = d.decode_field(_v.ui, changed); break;
                    case _t_u64 : decoded = d.decode_field(_v.ul, changed); break;
                    case _t_i8  : decoded = d.decode_field(_v.sc, changed); break;
                    case _t_i16 : decoded = d.decode_field(_v.ss, changed); break;
                    case _t_i32 : decoded = d.decode_field(_v.si, changed); break;
                    case _t_i64 : decoded = d.decode_field(_v.sl, changed); break;
                    case _t_bool: decoded = d.decode_field(_v.bd, changed); break;
                    case _t_f32 : decoded = d.decode_field(_v.fd, changed); break;
                    case _t_f64 : decoded = d.decode_field(_v.dd, changed); break;
                    case _t_f128: decoded = d.decode_field(_v.qd, changed); break;
                    default:
                        {
                            M_LOG("Unknown variant type: ID=%lu - type=%d", id_, type_);
                        }
                        break;
                }
            }

            M_LOG("} - %s", decoded ? "true" : "false");

            return decoded;
        }

    private:
        class IPointer
        {
        public:
            virtual ~IPointer() {}
            virtual size_t field_size(hobio::encoder &e) = 0;
            virtual bool   encode    (hobio::encoder &e) = 0;
            virtual const void *data() = 0;
        };

        class Hob : public IPointer
        {
        public:
            Hob(const vhob & v): _d(v.clone()) {}
            virtual size_t field_size(hobio::encoder &e) { return e.field_size(*_d); }
            virtual bool   encode    (hobio::encoder &e) { /*M_LOG("vhob");*/ return e.encode    (*_d); }
            virtual const void *data() { return static_cast<const void *>(_d); }
        private:
            vhob *_d;
        };

        class String : public IPointer
        {
        public:
            String(const string & v): _d(new string(v)) {}
            virtual size_t field_size(hobio::encoder &e) { return e.field_size(*_d); }
            virtual bool   encode    (hobio::encoder &e) { /*M_LOG("string");*/ return e.encode    (*_d); }
            virtual const void *data() { return static_cast<const void *>(_d); }
        private:
            string *_d;
        };

        template <typename T>
        class Optional: public IPointer
        {
        public:
            Optional(const optional<T> & v): _d(new optional<T>(v)) {}
            virtual size_t field_size(hobio::encoder &e) { return e.field_size(*_d); }
            virtual bool   encode    (hobio::encoder &e) { /*M_LOG("optional");*/ return e.encode    (*_d); }
            virtual const void *data() { return static_cast<const void *>(_d); }
        private:
            optional<T> *_d;
        };

        template <typename T>
        class Vector: public IPointer
        {
        public:
            Vector(const vector<T> & v): _d(new vector<T>(v)) {}
            virtual size_t field_size(hobio::encoder &e) { return e.field_size(*_d); }
            virtual bool   encode    (hobio::encoder &e) { /*M_LOG("vector");*/ return e.encode    (*_d); }
            virtual const void *data() { return static_cast<const void *>(_d); }
        private:
            vector<T> *_d;
        };

        template <typename K, typename V>
        class Map: public IPointer
        {
        public:
            Map(const map<K,V> & v): _d(new map<K,V>(v)) {}
            virtual size_t field_size(hobio::encoder &e) { return e.field_size(*_d); }
            virtual bool   encode    (hobio::encoder &e) { /*M_LOG("map");*/ return e.encode    (*_d); }
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

        hobio::UID _id;
        uint8_t    _t ;
        data_t     _v ;

        template<typename T>
        inline var_t __whois() const
        {
            if (is_base_of<vhob, T>::value)
            {
                // M_LOG("%s", typeid(vhob).name());

                return _t_hob;
            }

            const std::type_info & tref = typeid(T);

            // M_LOG("%s", tref.name());

            return (tref == typeid(uint8_t    )) ? _t_u8     :
                   (tref == typeid(uint16_t   )) ? _t_u16    :
                   (tref == typeid(uint32_t   )) ? _t_u32    :
                   (tref == typeid(uint64_t   )) ? _t_u64    :
                   (tref == typeid(int8_t     )) ? _t_i8     :
                   (tref == typeid(int16_t    )) ? _t_i16    :
                   (tref == typeid(int32_t    )) ? _t_i32    :
                   (tref == typeid(int64_t    )) ? _t_i64    :
                   (tref == typeid(bool       )) ? _t_bool   :
                   (tref == typeid(float      )) ? _t_f32    :
                   (tref == typeid(double     )) ? _t_f64    :
                   (tref == typeid(long double)) ? _t_f128   :
                   (tref == typeid(string     )) ? _t_string :
                   (tref == typeid(vhob        )) ? _t_hob    :
                                                   _t_unknown;
        }

        inline uint8_t __type(var_t h, var_t l)
        {
            return (((static_cast<uint8_t>(h) & 0x0f) << 4)
                    |
                    ((static_cast<uint8_t>(l) & 0x0f) << 0));
        }

        inline var_t __l_type() const
        {
            return static_cast<var_t>((_t >> 0) & 0x0f);
        }

        inline var_t __h_type() const
        {
            return static_cast<var_t>((_t >> 4) & 0x0f);
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
    inline size_type              count   () const { return _df.size    (); }
    inline size_type              max_size() const { return _df.max_size(); }

    inline iterator find(const hobio::UID &id)
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

    inline const_iterator find(const hobio::UID &id) const
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
    inline iterator find(const hobio::UID &id)
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
    inline const_iterator find(const hobio::UID &id) const
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
    inline void erase(const hobio::UID &id         ) { _df.erase(find(id)  ); }
    inline void erase(const string &n              ) { _df.erase(find( n)  ); }
    inline void erase(iterator pos                 ) { _df.erase(pos       ); }
    inline void erase(iterator first, iterator last) { _df.erase(first,last); }

    inline variant& operator[](const hobio::UID &id)
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

    inline variant& get_or_create(const hobio::UID id)
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
    inline vhob & set(const string &n, const T &v)
    {
        variant & v_ = get_or_create(n);

        v_ = v;

        return *this;
    }

    template<typename T>
    inline vhob & set(const hobio::UID &id, const T &v)
    {
        variant & v_ = get_or_create(id);

        v_ = v;

        return *this;
    }

    template<typename T>
    inline vhob & set(const char *n, const T &v)
    {
        variant & v_ = get_or_create(n);

        v_ = v;

        return *this;
    }

    inline bool has(const string     & n ) const { return (find( n) != end()); }
    inline bool has(const hobio::UID & id) const { return (find(id) != end()); }
    inline bool has(const char       * n ) const { return (find( n) != end()); }

    template<typename T>
    inline bool has(const string     & n ) const { return (find<T>( n) != end()); }

    template<typename T>
    inline bool has(const hobio::UID & id) const { return (find<T>(id) != end()); }

    template<typename T>
    inline bool has(const char       * n ) const { return (find<T>( n) != end()); }

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
    inline const T * get(const hobio::UID & id) const
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
    inline bool get(const hobio::UID & id, T & v) const
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

protected:
    virtual bool __decode(hob &ref)
    {
        hobio::decoder *is = static_cast<hobio::decoder*>(ref);

        if (NULL == is)
        {
            return false;
        }

        ref.__rewind();

        if (!__decode(*is))
        {
            return false;
        }

        *this = ref;

        return true;
    }

    virtual bool __decode(hobio::decoder &is, bool update=true)
    {
        (void)is;
        (void)update;

        return is.decode_field(_df);
    }

    virtual size_t __payload(hobio::encoder &os) const
    {
        (void)os;

        return (__has_payload()) ? os.field_size(_df) : 0;
    }

    inline bool __has_payload() const
    {
        return !_df.empty();
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
    }

private:
    dynamic_fields_t _df;

    inline bool __lt(const hobio::UID &id) const
    {
        return ( ( __get_id() & ~1 ) < ( id & ~1 ) );
    }

    inline bool __ne(const hobio::UID &id) const
    {
        return ( ( ( __get_id() ^ id ) & ~1 ) != 0 );
    }
};

// inline bool operator<<(hobio::encoder &e, vhob &h) { M_LOG("-"); return h >> e; }
// inline bool operator>>(hobio::decoder &d, vhob &h) { M_LOG("-"); return h << d; }
// inline bool operator>>(vhob           &f, vhob &t) { M_LOG("-"); return t << f; }
// inline bool operator>>(hob            &f, vhob &t) { M_LOG("-"); return t << f; }

#endif // __VHOB_HPP__
