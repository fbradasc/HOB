#if !defined(__HOB_HPP__)
#define __HOB_HPP__
#include "hob/types/hobject.hpp"
#include "hob/dynamhob.hpp"
#include "hob/utils/cpp_magic.h"

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
//  - Fixed length array of bits:
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
//      - variable length array:
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
#define HLEN(s)     ((sizeof(s)/sizeof(s[0])) - 1)
#define H1(s,i,x)   (static_cast<hobio::uid_t>(x)*(((i)<HLEN(s))?HSEED:1) \
                     +                                                    \
                     static_cast<uint8_t>(s[((i)<HLEN(s))                 \
                                            ?HLEN(s)-1-(i)                \
                                            :HLEN(s)]))

#define H4(s,i,x)   H1(s,(i)    , \
                    H1(s,(i)+  1, \
                    H1(s,(i)+  2, \
                    H1(s,(i)+  3, (x)))))

#define H16(s,i,x)  H4(s,(i)    , \
                    H4(s,(i)+  4, \
                    H4(s,(i)+  8, \
                    H4(s,(i)+ 12, (x)))))

#define H64(s,i,x)  H16(s,(i)    , \
                    H16(s,(i)+ 16, \
                    H16(s,(i)+ 32, \
                    H16(s,(i)+ 48, (x)))))

#define H256(s,i,x) H64(s,(i)    , \
                    H64(s,(i)+ 64, \
                    H64(s,(i)+128, \
                    H64(s,(i)+192, (x)))))

#define HASH(x)     (static_cast<hobio::uid_t>((x)^((x)>>32)))

#if defined(ENABLE_CPP_HASH)
#if defined(ENABLE_CPP_MAX_SIZE_HASH)
#define HUPDATE(s)   _id.update(static_cast<hobio::uid_t>(    \
                           (HLEN(s)>0) ? H256(s,0,_id) : _id))
#else // !ENABLE_CPP_MAX_SIZE_HASH
#define HUPDATE(s)   _id.update(static_cast<hobio::uid_t>(   \
                           (HLEN(s)>0) ? H64(s,0,_id) : _id))
#endif // !ENABLE_CPP_MAX_SIZE_HASH
#else // !ENABLE_CPP_HASH
#define HUPDATE(s)   _id.update(s)
#endif // !ENABLE_CPP_HASH

class hob
    : public dynamhob
{
public:
    ///////////////////////////////////////////////////////////////////////////
    //                                                                       //
    //                          HOB implementation                           //
    //                                                                       //
    ///////////////////////////////////////////////////////////////////////////

    hob(): dynamhob()
    {
    }

    hob(const hobio::UID &id_): dynamhob(id_)
    {
    }

    virtual ~hob()
    {
    }

    virtual bool operator>>(hobio::encoder &os) const
    {
        M_LOG("-");

        return
        (
            (hobio::UNDEFINED == _id)
            ||
            (
                os.encode_header(static_cast<const char *>(NULL),
                                 static_cast<const char *>(NULL),
                                 _id.with_dynamic_fields(!empty()),
                                 __get_payload_size(os))
                &&
                __encode_dynamic_fields(os)
                &&
                os.encode_footer()
            )
        );
    }

    inline operator bool()
    {
        return __is_changed();
    }

    inline void operator~()
    {
        __reset_changes();
    }

    virtual size_t size(hobio::encoder &os) const
    {
        size_t sz = __get_payload_size(os);

        return os.field_size(_id.with_dynamic_fields(!empty()))
               +
               ((sz > 0) ? (os.field_size(sz) + sz) : 0);
    }

    virtual bool encode(hobio::encoder &os) const
    {
        return (*this >> os);
    }

protected:
    virtual bool __decode(hobio::hobject &ref)
    {
        (void)ref;

        if (!this->hobio::hobject::__decode(ref))
        {
            return false;
        }

        return __decode_dynamic_fields(ref);
    }

    virtual bool __decode(hobio::decoder &is, bool update=true)
    {
        return this->hobio::hobject::__decode(is,update);
    }

    virtual size_t __get_static_fields_size(hobio::encoder &os) const
    {
        (void)os;

        return 0;
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

    size_t __get_payload_size(hobio::encoder &os) const
    {
        (void)os;

        return __get_static_fields_size(os) + __get_dynamic_fields_size(os);
    }

    // ----> dynamic fields implementation <----
    //
    size_t __get_dynamic_fields_size(hobio::encoder &os) const
    {
        (void)os;

        return (empty()) ? 0 : os.field_size(_df);
    }

    inline bool __encode_dynamic_fields(hobio::encoder &os) const
    {
        (void)os;

        return ( empty() || os.encode_field(_df, "dynamhob") );
    }

    inline bool __decode_dynamic_fields(hobio::hobject &ref)
    {
        if (!ref.__has_dynamic_fields())
        {
            return true;
        }

        hobio::decoder *is = static_cast<hobio::decoder*>(ref);

        if (NULL == is)
        {
            return false;
        }

        return is->decode_field(_df);
    }
    //
    // ----> dynamic fields implementation <----
};

inline bool operator<<(hobio::encoder &e, hob &h) { return h >> e; }
inline bool operator>>(hobio::decoder &d, hob &h) { return h << d; }
inline bool operator>>(hob            &f, hob &t) { return t << f; }

template<class T>
struct TypeInfo;

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
#define UPDATE_NP(t, n, ...)     _id.update("");
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
        _id.update(static_cast<const char *>(NULL)); /* finalize hob ID */        \
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
    name_ & operator=(const hobject & ref)                                        \
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
    virtual bool operator>>(hobio::encoder &os) const                             \
    {                                                                             \
        size_t payload_size = __get_payload_size(os);                             \
                                                                                  \
        return                                                                    \
        (                                                                         \
            (hobio::UNDEFINED == _id)                                             \
            ||                                                                    \
            (                                                                     \
                os.encode_header(PP_STR(name_),                                   \
                                 value_,                                          \
                                 _id.with_dynamic_fields(!empty()),               \
                                 payload_size)                                    \
                &&                                                                \
                __encode_dynamic_fields(os)                                       \
                SCAN_FIELDS(ENCODE_FIELD, PP_FIRST(__VA_ARGS__))                  \
                SCAN_FIELDS(ENCODE_FIELD, PP_REMAIN(__VA_ARGS__))                 \
                &&                                                                \
                os.encode_footer()                                                \
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
    virtual bool __decode(hobio::hobject &ref)                                    \
    {                                                                             \
        hobio::decoder *is = static_cast<hobio::decoder*>(ref);                   \
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
        if (!__decode_dynamic_fields(ref))                                        \
        {                                                                         \
            return false;                                                         \
        }                                                                         \
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
    virtual bool __decode(hobio::decoder &is, bool update=true)                   \
    {                                                                             \
        (void)is;                                                                 \
        (void)update;                                                             \
                                                                                  \
        bool c = false;                                                           \
                                                                                  \
        (void)c;                                                                  \
                                                                                  \
        /* Read mandatory fields : fail on error */                               \
                                                                                  \
        if (true SCAN_FIELDS(DECODE_FIELD, PP_FIRST(__VA_ARGS__)))                \
        {                                                                         \
            /* Read optional fields : ignore errors */                            \
                                                                                  \
            if ((true SCAN_FIELDS(DECODE_FIELD, PP_REMAIN(__VA_ARGS__))) || true) \
            {                                                                     \
                hob::__flush_pending();                                           \
                                                                                  \
                return true;                                                      \
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
    virtual size_t __get_static_fields_size(hobio::encoder &os) const             \
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
};                                                                                \

#endif // __HOB_HPP__
