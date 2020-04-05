#if !defined(__HOB_IO_HPP__)
#define __HOB_IO_HPP__

#include <unistd.h>
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
#include "optional.hpp"

using namespace std;
using namespace nonstd;

class HOB;

#define USE_VARINT

/*
 * Define some byte ordering macros
 */
#if defined(WIN32) || defined(_WIN32)
    #define HOB_BIG_ENDIAN_ENCODE_16(v) _byteswap_ushort(v)
    #define HOB_BIG_ENDIAN_ENCODE_32(v) _byteswap_ulong(v)
    #define HOB_BIG_ENDIAN_ENCODE_64(v) _byteswap_uint64(v)
    #define HOB_LITTLE_ENDIAN_ENCODE_16(v) (v)
    #define HOB_LITTLE_ENDIAN_ENCODE_32(v) (v)
    #define HOB_LITTLE_ENDIAN_ENCODE_64(v) (v)
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define HOB_BIG_ENDIAN_ENCODE_16(v) __builtin_bswap16(v)
    #define HOB_BIG_ENDIAN_ENCODE_32(v) __builtin_bswap32(v)
    #define HOB_BIG_ENDIAN_ENCODE_64(v) __builtin_bswap64(v)
    #define HOB_LITTLE_ENDIAN_ENCODE_16(v) (v)
    #define HOB_LITTLE_ENDIAN_ENCODE_32(v) (v)
    #define HOB_LITTLE_ENDIAN_ENCODE_64(v) (v)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define HOB_LITTLE_ENDIAN_ENCODE_16(v) __builtin_bswap16(v)
    #define HOB_LITTLE_ENDIAN_ENCODE_32(v) __builtin_bswap32(v)
    #define HOB_LITTLE_ENDIAN_ENCODE_64(v) __builtin_bswap64(v)
    #define HOB_BIG_ENDIAN_ENCODE_16(v) (v)
    #define HOB_BIG_ENDIAN_ENCODE_32(v) (v)
    #define HOB_BIG_ENDIAN_ENCODE_64(v) (v)
#else
    #error "Byte Ordering of platform not determined. Set __BYTE_ORDER__ manually before including this file."
#endif

namespace HOBIO
{
    class AbstractReader;
    class AbstractWriter;

    class Serializeable
    {
    public:
        virtual bool operator<<(AbstractReader &is)             = 0;
        virtual bool operator>>(AbstractWriter &os) const       = 0;
        virtual operator size_t() const                         = 0;
        virtual operator bool() const                           = 0;
        virtual void __reset_changes()                          = 0;
        virtual bool __is_changed() const                       = 0;
        virtual void __set_changed(ssize_t f, bool v)           = 0;
        virtual bool __deserialize(HOBIO::AbstractReader &is_,
                                   Serializeable &v,
                                   ssize_t field=-1)            = 0;

        static inline size_t __size_of(const uint8_t &v)
        {
            return __get_varint_size(v);
        }

        static inline size_t __size_of(const uint16_t &v)
        {
            return __get_varint_size(v);
        }

        static inline size_t __size_of(const uint32_t &v)
        {
            return __get_varint_size(v);
        }

        static inline size_t __size_of(const uint64_t &v)
        {
            return __get_varint_size(v);
        }

        static inline size_t __size_of(const int8_t &v)
        {
            return __get_varint_size(v);
        }

        static inline size_t __size_of(const int16_t &v)
        {
            return __get_varint_size(v);
        }

        static inline size_t __size_of(const int32_t &v)
        {
            return __get_varint_size(v);
        }

        static inline size_t __size_of(const int64_t &v)
        {
            return __get_varint_size(v);
        }

        static inline size_t __size_of(const bool &v)
        {
            return sizeof(v);
        }

        static inline size_t __size_of(const float &v)
        {
            return sizeof(v);
        }

        static inline size_t __size_of(const double &v)
        {
            return sizeof(v);
        }

        static inline size_t __size_of(const long double &v)
        {
            return sizeof(v);
        }

        static inline size_t __size_of(const string &v)
        {
            return __size_of(v.size()) + v.size();
        }

        static inline size_t __size_of(size_t size_, const void *v)
        {
            return __size_of(size_) + size_;
        }

        static size_t __size_of(const Serializeable &v)
        {
            return v;
        }

        template<size_t N>
        static size_t __size_of(const std::bitset<N>& v)
        {
            return __size_of((N+7)>>3) + ((N+7)>>3);
        }

        template<class T>
        static size_t __size_of(const vector<T> &v)
        {
            size_t retval = __size_of(v.size());

            for (size_t i=0; i<v.size(); i++)
            {
                retval += __size_of(static_cast<const T &>(v[i]));
            }

            return retval;
        }

        static size_t __size_of(const vector<bool> &v, bool dump_size=true)
        {
            size_t sz = v.size();
            return ((dump_size) ? __size_of(sz) : 0) + __size_of((sz+7)>>3) + ((sz+7)>>3);
        }

        template<class T>
        static size_t __size_of(const optional<T> &v)
        {
            size_t retval = __size_of(static_cast<bool>(v));

            if (static_cast<bool>(v))
            {
                retval += __size_of(static_cast<const T &>(*v));
            }

            return retval;
        }

        template<class K, class V>
        static size_t __size_of(const map<K,V> &v)
        {
            size_t len = v.size();
            size_t retval = __size_of(len);

            for (typename map<K,V>::const_iterator ci = v.begin();
                 (len > 0) && (ci != v.end());
                 ++ci)
            {
                retval += __size_of(static_cast<const K &>((*ci).first ));
                retval += __size_of(static_cast<const V &>((*ci).second));

                len--;
            }

            return retval;
        }

    private:
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

        template <class T>
        static inline size_t __get_varint_size(const T &v)
        {
            uint64_t rv;

            return __get_varint_unzigzaged_size<T>(v,rv);
        }

        friend class AbstractWriter;

        template <class T>
        static inline size_t __get_varint_unzigzaged_size(const T &v,
                                                          uint64_t &rv)
        {
#if defined(USE_VARINT)
            rv = static_cast<uint64_t>
                 (
                     (std::numeric_limits<T>::is_signed)
                     ?
                         // ZIGZAG encoding
                         //
                         (static_cast<int64_t>(v) << 1)
                         ^
                         (static_cast<int64_t>(v) >> 63)
                     :
                         v
                 );

            return (rv <= 0x000000000000007fllu) ? 1 :
                   (rv <= 0x0000000000003fffllu) ? 2 :
                   (rv <= 0x00000000001fffffllu) ? 3 :
                   (rv <= 0x000000000fffffffllu) ? 4 :
                   (rv <= 0x00000007ffffffffllu) ? 5 :
                   (rv <= 0x000003ffffffffffllu) ? 6 :
                   (rv <= 0x0001ffffffffffffllu) ? 7 :
                   (rv <= 0x00ffffffffffffffllu) ? 8 : 9;
#else
            rv = static_cast<uint64_t>(v);

            return sizeof(T);
#endif
        }
    };

    class BaseIO
    {
    public:
        BaseIO(): _good(true) {}

        inline bool good() { return _good; }

    protected:
        inline bool set_good(bool v) { _good = v; return _good; }

    private:
        bool _good;
    };

    class AbstractWriter
    {
    public:
        AbstractWriter()
        {
        }

        virtual ~AbstractWriter()
        {
        }

        inline bool write(const uint8_t &v)
        {
            return varint_pack(v);
        }

        inline bool write(const uint16_t &v)
        {
            return varint_pack(v);
        }

        inline bool write(const uint32_t &v)
        {
            return varint_pack(v);
        }
        inline bool write(const uint64_t &v)
        {
            return varint_pack(v);
        }

        inline bool write(const int8_t &v)
        {
            return varint_pack(v);
        }

        inline bool write(const int16_t &v)
        {
            return varint_pack(v);
        }

        inline bool write(const int32_t &v)
        {
            return varint_pack(v);
        }

        inline bool write(const int64_t &v)
        {
            return varint_pack(v);
        }

        inline bool write(const bool &v)
        {
            return write(&v,sizeof(v));
        }

        inline bool write(const float &v)
        {
            return write(&v,sizeof(v));
        }

        inline bool write(const double &v)
        {
            return write(&v,sizeof(v));
        }

        inline bool write(const long double &v)
        {
            return write(&v,sizeof(v));
        }

        inline bool write(const string &v)
        {
            return write(v.size(),v.data());
        }

        inline bool write(size_t size_, const void *v)
        {
            if (!write(size_))
            {
                return false;
            }

            return write(v,size_);
        }

        inline bool write(const Serializeable &v)
        {
            return (v >> *this);
        }

        template<size_t N>
        bool write(const std::bitset<N>& v)
        {
            uint8_t bits[(N+7)>>3];

            memset(bits, 0, ((N+7)>>3));

            for (size_t j=0; j<size_t(N); j++)
            {
                bits[j>>3] |= (v[j]<<(j&7));
            }

            return write(((N+7)>>3), bits);
        }

        template<class T>
        bool write(const vector<T> &v)
        {
            if (!write(v.size()))
            {
                return false;
            }

            for (size_t i=0; i<v.size(); i++)
            {
                if (!write(static_cast<const T &>(v[i])))
                {
                    return false;
                }
            }

            return true;
        }

        bool write(const vector<bool> &v, bool dump_size=true)
        {
            if (dump_size && !write(v.size()))
            {
                return false;
            }

            uint8_t bits[(v.size()+7)>>3];

            memset(bits, 0, ((v.size()+7)>>3));

            for (size_t j=0; j<v.size(); j++)
            {
                bits[j>>3] |= (v[j]<<(j&7));
            }

            return write(((v.size()+7)>>3), bits);
        }

        template<class T>
        bool write(const optional<T> &v)
        {
            if (!write(static_cast<bool>(v)))
            {
                return false;
            }

            if (static_cast<bool>(v))
            {
                if (!write(static_cast<const T &>(*v)))
                {
                    return false;
                }
            }

            return true;
        }

        template<class K, class V>
        bool write(const map<K,V> &v)
        {
            size_t len = v.size();

            if (!write(len))
            {
                return false;
            }

            for (typename map<K,V>::const_iterator ci = v.begin();
                 (len > 0) && (ci != v.end());
                 ++ci)
            {
                if (!write(static_cast<const K &>((*ci).first)))
                {
                    return false;
                }

                if (!write(static_cast<const V &>((*ci).second)))
                {
                    return false;
                }

                len--;
            }

            return true;
        }

        virtual bool alloc(const size_t &s)
        {
            (void)s;

            return true;
        }

    protected:
        virtual bool write(const void *data, size_t size)
        {
            (void)data;
            (void)size;

            return true;
        }

        template <class T>
        inline bool varint_pack(const T &v)
        {
#if defined(USE_VARINT)
            uint8_t  d[9];
            uint64_t rv;
            uint8_t  b = Serializeable::__get_varint_unzigzaged_size<T>(v, rv);
            uint8_t  m = 0xff       >> b;
            uint32_t c = 0xfffffe00 >> b;

            *reinterpret_cast<uint64_t*>(&d[1]) = HOB_BIG_ENDIAN_ENCODE_64(rv);

            d[9-b] &= static_cast<uint8_t>(m & 0xff);
            d[9-b] |= static_cast<uint8_t>(c & 0xff);

            return write(&d[9-b],b);
#else
            return write(&v,sizeof(T));
#endif
        }
    };

    class AbstractReader
    {
    public:
        AbstractReader() { }

        virtual ~AbstractReader()
        {
        }

        inline bool read(Serializeable &is_, uint8_t &v, ssize_t field=-1)
        {
            return varint_unpack<uint8_t>(is_,v,field);
        }

        inline bool read(Serializeable &is_, uint16_t &v, ssize_t field=-1)
        {
            return varint_unpack<uint16_t>(is_,v,field);
        }

        inline bool read(Serializeable &is_, uint32_t &v, ssize_t field=-1)
        {
            return varint_unpack<uint32_t>(is_,v,field);
        }

        inline bool read(Serializeable &is_, uint64_t &v, ssize_t field=-1)
        {
            return varint_unpack<uint64_t>(is_, v, field);
        }

        inline bool read(Serializeable &is_, int8_t &v, ssize_t field=-1)
        {
            return varint_unpack<int8_t>(is_,v,field);
        }

        inline bool read(Serializeable &is_, int16_t &v, ssize_t field=-1)
        {
            return varint_unpack<int16_t>(is_,v,field);
        }

        inline bool read(Serializeable &is_, int32_t &v, ssize_t field=-1)
        {
            return varint_unpack<int32_t>(is_,v,field);
        }

        inline bool read(Serializeable &is_, int64_t &v, ssize_t field=-1)
        {
            return varint_unpack<int64_t>(is_,v,field);
        }

        bool read(Serializeable &is_, bool &v, ssize_t field=-1)
        {
            bool rv;

            if (!read(&rv, sizeof(v)))
            {
                return false;
            }

            is_.__set_changed(field, v != rv);

            v = rv;

            return true;
        }

        bool read(Serializeable &is_, float &v, ssize_t field=-1)
        {
            float rv;

            if (!read(&rv, sizeof(v)))
            {
                return false;
            }

            is_.__set_changed(field, v != rv);

            v = rv;

            return true;
        }

        bool read(Serializeable &is_, double &v, ssize_t field=-1)
        {
            double rv;

            if (!read(&rv, sizeof(v)))
            {
                return false;
            }

            is_.__set_changed(field, v != rv);

            v = rv;

            return true;
        }

        bool read(Serializeable &is_, long double &v, ssize_t field=-1)
        {
            long double rv;

            if (!read(&rv, sizeof(v)))
            {
                return false;
            }

            is_.__set_changed(field, v != rv);

            v = rv;

            return true;
        }

        bool read(Serializeable &is_, string &v, ssize_t field=-1)
        {
            size_t len;

            if (!read(is_, len))
            {
                return false;
            }

            vector<char> tmp(len);

            if (!read(tmp.data(), len))
            {
                return false;
            }

            string rv;

            rv.assign(tmp.data(),len);

            is_.__set_changed(field, v != rv);

            v = rv;

            return true;
        }

        bool read(Serializeable &is_, Serializeable &v, ssize_t field=-1)
        {
            return is_.__deserialize(*this,v,field);
        }

        template<size_t N>
        bool read(Serializeable &is_, bitset<N> &v, ssize_t field=-1)
        {
            size_t len;

            if (!read(is_, len) || (len != ((N + 7) >> 3)))
            {
                return false;
            }

            uint8_t bits[len];

            memset(bits, 0, len);

            if (!read(bits, len))
            {
                return false;
            }

            bitset<N> rv;

            for (size_t j=0; j<size_t(N); j++)
            {
                rv[j] = ((bits[j>>3]>>(j&7)) & 1);
            }

            is_.__set_changed(field, v != rv);

            v = rv;

            return true;
        }

        template<class T>
        bool read(Serializeable &is_, vector<T> &v, ssize_t field=-1)
        {
            size_t len;

            if (!read(is_, len))
            {
                return false;
            }

            if (len > 0)
            {
                vector<T> tmp(len);

                for (size_t i=0; i<len; i++)
                {
                    if (!read(is_, static_cast<T&>(tmp[i])))
                    {
                        return false;
                    }
                }

                is_.__set_changed(field, v != tmp);

                v = tmp;
            }
            else
            {
                is_.__set_changed(field, !v.empty());

                v = vector<T>();
            }

            return true;
        }

        bool read(Serializeable &is_, vector<bool> &v, ssize_t field=-1)
        {
            size_t count;

            if (!read(is_, count))
            {
                return false;
            }

            if (count > 0)
            {
                size_t len;

                if (!read(is_, len) || (len != ((count + 7) >> 3)))
                {
                    return false;
                }

                uint8_t bits[len];

                memset(bits, 0, len);

                if (!read(bits, len))
                {
                    return false;
                }

                vector<bool> rv;

                for (size_t j=0; j<count; j++)
                {
                    rv.push_back((bits[j>>3]>>(j&7)) & 1);
                }

                is_.__set_changed(field, v != rv);

                v = rv;

                return true;
            }
            else
            {
                is_.__set_changed(field, !v.empty());

                v = vector<bool>();
            }

            return true;
        }

        template<class T>
        bool read(Serializeable &is_, optional<T> &v, ssize_t field=-1)
        {
            bool has_field;

            if (!read(is_, has_field))
            {
                return false;
            }

            if (has_field)
            {
                T tmp;

                if (!read(is_, static_cast<T&>(tmp)))
                {
                    return false;
                }

                is_.__set_changed(field, v != tmp);

                v = tmp;
            }
            else
            {
                is_.__set_changed(field, v.has_value());

                v = optional<T>();
            }

            return true;
        }

        template<class K, class V>
        bool read(Serializeable &is_, map<K,V> &v_, ssize_t field=-1)
        {
            size_t len;

            if (!read(is_, len))
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

                    if (!read(is_, static_cast<K&>(k)))
                    {
                        return false;
                    }

                    if (!read(is_, static_cast<V&>(v)))
                    {
                        return false;
                    }

                    tmp[k] = v;
                }

                is_.__set_changed(field, v_ != tmp);

                v_ = tmp;
            }
            else
            {
                is_.__set_changed(field, !v_.empty());

                v_ = map<K,V>();
            }

            return true;
        }

        virtual bool get(uint8_t &v)
        {
            return true;
        }

        virtual bool unget(const uint8_t &v)
        {
            return true;
        }

        virtual bool seek(ssize_t offset, int whence)
        {
            return true;
        }

        virtual ssize_t tell()
        {
            return -1;
        }

        virtual void ignore(size_t size)
        {
            if (!seek(size,SEEK_CUR))
            {
                for (uint8_t v; (size>0) && get(v); size--);
            }
        }

        virtual bool read(void *data, size_t size)
        {
            if ((NULL == data) || (size < 1))
            {
                return false;
            }

            return true;
        }

    protected:
        template <class T>
        inline bool varint_unpack(Serializeable &is_,
                                  T &v,
                                  ssize_t field=-1)
        {
#if defined(USE_VARINT)
            uint8_t d[16] = { 0 };
            uint8_t b = 0;
            uint8_t c;
            uint8_t m;

            if (!read(&d[7], sizeof(uint8_t)))
            {
                return false;
            }

            for (c=0x80, b=1; (b <= 8) && ((d[7] & c) != 0); b++)
            {
                c >>= 1;
            }

            m = (0xff << (8-b));

            if (b > 1)
            {
                if (!read(&d[8], b-1))
                {
                    return false;
                }
            }

            if (b < 9)
            {
                d[7] &= ~m;
            }

            uint64_t r = HOB_BIG_ENDIAN_ENCODE_64
            (
                 *reinterpret_cast<uint64_t*>(&d[b-1])
            );

            if (std::numeric_limits<T>::is_signed)
            {
                // ZIGZAG decoding
                //
                r = (r >> 1) ^ -(r & 1);
            }

            T rv = static_cast<T>(r & (static_cast<T>(-1)));
#else
            T rv;

            if (!read(&rv,sizeof(T)))
            {
                return false;
            }
#endif

            is_.__set_changed(field, v != rv);

            v = rv;

            return true;
        }
    };

    class FileWriter
        : public BaseIO
        , public AbstractWriter
    {
    public:
        FileWriter()
            : BaseIO()
            , AbstractWriter()
            , _pf(stdout)
            , _can_close(false)
        {
        }

        FileWriter(FILE *pf)
            : BaseIO()
            , AbstractWriter()
            , _pf(pf)
            , _can_close(false)
        {
        }

        FileWriter(const char *pathname, bool append = false)
            : BaseIO()
            , AbstractWriter()
            , _pf(NULL)
            , _can_close(false)
        {
            open(pathname, append);
        }

        ~FileWriter()
        {
            close();
        }

        bool open(const char *pathname, bool append = false)
        {
            close();

            _pf = ::fopen(pathname, (append) ? "ab" : "wb");

            return set_good(_can_close = (NULL != _pf));
        }

        virtual bool write(const void *data, size_t size)
        {
            if ((NULL == data) || (0 == size))
            {
                return true;
            }

            if (!good() || (NULL == _pf))
            {
                return false;
            }

            return set_good( ::fwrite(data,1,size,_pf) == size );
        }

        void close()
        {
            if (NULL != _pf)
            {
                ::fflush(_pf);

                if (_can_close)
                {
                    ::fclose(_pf);
                }
            }

            _pf = NULL;

            _can_close = false;
        }

    private:
        FILE *_pf;
        bool  _can_close;
    };

    class FileReader
        : public BaseIO
        , public AbstractReader
    {
    public:
        FileReader()
            : BaseIO()
            , AbstractReader()
            , _pf(stdin)
            , _can_close(false)
        {
        }

        FileReader(FILE *pf)
            : BaseIO()
            , AbstractReader()
            , _pf(pf)
            , _can_close(false)
        {
        }

        FileReader(const char *pathname)
            : BaseIO()
            , AbstractReader()
            , _can_close(false)
        {
            open(pathname);
        }

        ~FileReader()
        {
            close();
        }

        bool open(const char *pathname, bool binary=true)
        {
            close();

            _pf = ::fopen(pathname, (binary) ? "rb" : "r");

            return set_good( _can_close = (NULL != _pf) );
        }

        virtual bool read(void *data, size_t size)
        {
            if (!good() || (NULL == _pf) || ::feof(_pf))
            {
                return set_good(false);
            }

            if ((NULL == data) || (size < 1))
            {
                return false;
            }

            return set_good(::fread(data,1,size,_pf) == size);
        }

        virtual bool get(uint8_t &v)
        {
            if (!good() || (NULL == _pf) || ::feof(_pf))
            {
                return set_good(false);
            }

            int rv = fgetc(_pf);

            if (EOF == rv)
            {
                return set_good(false);
            }

            v = static_cast<uint8_t>(rv);

            return true;
        }

        virtual bool unget(const uint8_t &v)
        {
            if (!good() || (NULL == _pf))
            {
                return set_good(false);
            }

            return set_good( ungetc(static_cast<int>(v), _pf) != EOF );
        }

        virtual bool seek(long offset, int whence)
        {
            if (!good() || (NULL == _pf) || ::feof(_pf))
            {
                return set_good(false);
            }

            return ( ::fseek(_pf,offset,whence) >= 0 );
        }

        virtual ssize_t tell()
        {
            if (NULL == _pf)
            {
                return -1;
            }

            return ::ftell(_pf);
        }

        void close()
        {
            if (NULL != _pf)
            {
                // ::fflush(_pf);

                if (_can_close)
                {
                    ::fclose(_pf);
                }
            }

            _pf = NULL;

            _can_close = false;
        }

    private:
        FILE *_pf;
        bool  _can_close;
    };

    class FdWriter
        : public BaseIO
        , public AbstractWriter
    {
    public:
        FdWriter(int fd)
            : BaseIO()
            , AbstractWriter()
            , _fd(fd)
        {
        }

        ~FdWriter()
        {
            close();
        }

        virtual bool write(const void *data, size_t size)
        {
            if ((NULL == data) || (0 == size))
            {
                return true;
            }

            if (!good() || (_fd < 0))
            {
                return set_good(false);
            }

            return set_good(::write(_fd,data,size) == static_cast<ssize_t>(size));
        }

        void close()
        {
            _fd = -1;
        }

    private:
        int _fd;
    };

    class FdReader
        : public BaseIO
        , public AbstractReader
    {
    public:
        FdReader(int fd)
            : BaseIO()
            , AbstractReader()
            , _fd(fd)
            , _ungetc(0)
        {
        }

        ~FdReader()
        {
            close();
        }

        virtual bool read(void *data, size_t size)
        {
            if (!good() || (_fd < 0))
            {
                return set_good(false);
            }

            if ((NULL == data) || (size < 1))
            {
                return false;
            }

            uint8_t *ptr = static_cast<uint8_t*>(data);

            if ((size > 0) && (_ungetc != 0))
            {
                int tmp = _ungetc & 0x00ff;

                _ungetc = 0;

                ptr[0] = static_cast<uint8_t>(tmp);

                ptr++;

                size--;

                if (size == 0)
                {
                    return true;
                }
            }

            return set_good( ::read(_fd,ptr,size) == static_cast<ssize_t>(size) );
        }

        virtual bool get(uint8_t &v)
        {
            return read(&v,sizeof(v));
        }

        virtual bool unget(const uint8_t &v)
        {
            if (_ungetc)
            {
                return false;
            }

            _ungetc = 0xff00 | ( static_cast<int>(v) & 0x00ff );

            return true;
        }

        virtual bool seek(ssize_t offset, int whence)
        {
            if (!good() || (_fd < 0))
            {
                return set_good(false);
            }

            return ( ::lseek(_fd,static_cast<off_t>(offset),whence) >= 0 );
        }

        virtual ssize_t tell()
        {
            if (!good() || (_fd < 0))
            {
                set_good(false);

                return -1;
            }

            return ::lseek(_fd,0,SEEK_CUR);
        }

        void close()
        {
            _fd = -1;
        }

    private:
        int      _fd;
        uint16_t _ungetc;
    };

    class FdReaderWriter
        : public BaseIO
        , public AbstractReader
        , public AbstractWriter
    {
    public:
        FdReaderWriter(int fd)
            : BaseIO()
            , AbstractReader()
            , AbstractWriter()
            , _fd(fd)
            , _ungetc(0)
        {
        }

        ~FdReaderWriter()
        {
            close();
        }

        virtual bool write(const void *data, size_t size)
        {
            if ((NULL == data) || (0 == size))
            {
                return true;
            }

            if (!good() || (_fd < 0))
            {
                return set_good(false);
            }

            _ungetc = 0;

            return set_good( ::write(_fd,data,size) == static_cast<ssize_t>(size) );
        }

        virtual bool read(void *data, size_t size)
        {
            if (!good() || (_fd < 0))
            {
                return set_good(false);
            }

            if ((NULL == data) || (size < 1))
            {
                return false;
            }

            uint8_t *ptr = static_cast<uint8_t*>(data);

            if ((size > 0) && (_ungetc != 0))
            {
                int tmp = _ungetc & 0x00ff;

                _ungetc = 0;

                ptr[0] = static_cast<uint8_t>(tmp);

                ptr++;

                size--;

                if (size == 0)
                {
                    return true;
                }
            }

            return set_good( ::read(_fd,ptr,size) == static_cast<ssize_t>(size) );
        }

        virtual bool get(uint8_t &v)
        {
            return read(&v,sizeof(v));
        }

        virtual bool unget(const uint8_t &v)
        {
            if (!good() || (_fd < 0))
            {
                return set_good(false);
            }

            if (_ungetc)
            {
                return false;
            }

            _ungetc = 0xff00 | ( static_cast<int>(v) & 0x00ff );

            return true;
        }

        virtual bool seek(ssize_t offset, int whence)
        {
            if (!good() || (_fd < 0))
            {
                return set_good(false);
            }

            return ( ::lseek(_fd,static_cast<off_t>(offset),whence) >= 0 );
        }

        virtual ssize_t tell()
        {
            if (!good() || (_fd < 0))
            {
                set_good(false);

                return -1;
            }

            return ::lseek(_fd,0,SEEK_CUR);
        }

        void close()
        {
            _fd = -1;
        }

    private:
        int      _fd;
        uint16_t _ungetc;
    };

    class BufferWriter
        : public BaseIO
        , public AbstractWriter
    {
    public:
        BufferWriter(size_t reserve_=0)
            : BaseIO()
            , AbstractWriter()
            , _buffer  (NULL)
            , _size    (0)
            , _capacity(0)
        {
            if (reserve_ > 0)
            {
                alloc(reserve_);
            }
        }

        ~BufferWriter()
        {
            free(_buffer);

            _buffer = NULL;
        }

        inline const uint8_t *data    () const { return _buffer  ; }
        inline size_t         size    () const { return _size    ; }
        inline size_t         capacity() const { return _capacity; }

        inline bool alloc(const size_t &s)
        {
            if ( s == 0 )
            {
                return true;
            }

            if ((_size + s) > _capacity)
            {
                size_t new_capacity = _size + s;

                _buffer = reinterpret_cast<uint8_t*> (
                    realloc(_buffer,new_capacity)
                );

                if (NULL != _buffer)
                {
                    _capacity = new_capacity;
                }
            }

            return set_good( NULL != _buffer );
        }

        inline virtual void clear() { _size = 0; }

        inline virtual bool write(const void *data, size_t size)
        {
#if 1
            if ((NULL == data) || (0 == size))
            {
                return true;
            }

            if (!alloc(size))
            {
                return set_good( false );
            }

            memcpy(&_buffer[_size], data, size);

            _size += size;
#endif
            return true;
        }

    private:
        uint8_t *_buffer;
        size_t   _size;
        size_t   _capacity;
    };

    class BufferReader
        : public BaseIO
        , public AbstractReader
    {
    public:
        BufferReader()
            : BaseIO()
            , AbstractReader()
            , _buffer       (NULL)
            , _size         (0)
            , _capacity     (0)
            , _pos          (0)
        {
        }

        BufferReader(const BufferReader &ref)
            : BaseIO()
            , AbstractReader()
            , _buffer       (NULL)
            , _size         (0)
            , _capacity     (0)
            , _pos          (0)
        {
            *this = ref;
        }

        BufferReader(const BufferWriter &ref)
            : BaseIO()
            , AbstractReader()
            , _buffer       (NULL)
            , _size         (0)
            , _capacity     (0)
            , _pos          (0)
        {
            if (ref.capacity() > 0)
            {
                _buffer = reinterpret_cast<uint8_t*>
                (
                    malloc(ref.capacity())
                );

                if (NULL != _buffer)
                {
                    memcpy(_buffer,ref.data(),ref.capacity());

                    _size     = ref.size();

                    _capacity = ref.capacity();
                }
            }

            set_good( NULL != _buffer );
        }

        BufferReader(AbstractReader &ref, size_t size)
            : BaseIO()
            , AbstractReader()
            , _buffer       (NULL)
            , _size         (0)
            , _capacity     (0)
            , _pos          (0)
        {
            load(ref, size);
        }

        BufferReader & operator=(const BufferReader &ref)
        {
            if (ref.capacity() > 0)
            {
                _buffer = reinterpret_cast<uint8_t*>
                (
                    malloc(ref.capacity())
                );

                if (NULL != _buffer)
                {
                    memcpy(_buffer,ref.data(),ref.capacity());

                    _size     = ref.size();

                    _capacity = ref.capacity();
                }
            }

            set_good( NULL != _buffer );

            return *this;
        }

        ~BufferReader()
        {
            free(_buffer);

            _buffer = NULL;
        }

        inline const uint8_t *data    () const { return _buffer  ; }
        inline size_t         size    () const { return _size    ; }
        inline size_t         capacity() const { return _capacity; }

        inline bool alloc(const size_t &s)
        {
            if ( s == 0 )
            {
                return true;
            }

            size_t new_capacity = _size + s;

            if (new_capacity > _capacity)
            {
                _buffer = reinterpret_cast<uint8_t*>
                (
                    realloc(_buffer,new_capacity)
                );

                if (NULL != _buffer)
                {
                    _capacity = new_capacity;
                }
            }

            return set_good( NULL != _buffer );
        }

        inline virtual void rewind()
        {
            set_good(true);
            _pos = 0;
        }

        inline virtual void clear()
        {
            _size = 0;
            _pos  = 0;
        }

        inline virtual bool load(AbstractReader &ref, size_t size)
        {
            if ((size > 0) && (!alloc(size) || !ref.read(&_buffer[_size], size)))
            {
                return set_good(false);
            }

            _size += size;

            return true;
        }

        inline virtual bool read(void *data, size_t size)
        {
            if (!good() || (NULL == _buffer))
            {
                return set_good(false);
            }

            if ((NULL == data) || (size < 1))
            {
                return false;
            }

            if ((_pos + size) > _size)
            {
                return false;
            }

            memcpy(data, &_buffer[_pos], size);

            _pos += size;

            return true;
        }

        inline virtual bool get(uint8_t &v)
        {
            if (!good() || (NULL == _buffer))
            {
                return set_good(false);
            }

            if ((_pos + 1) > static_cast<ssize_t>(_size))
            {
                return false;
            }

            v = _buffer[_pos++];

            return true;
        }

        inline virtual bool unget(const uint8_t &v)
        {
            if (!good() || (NULL == _buffer))
            {
                return set_good(false);
            }

            if (_pos < 1)
            {
                return false;
            }

            _buffer[--_pos] = v;

            return true;
        }

        inline virtual bool seek(ssize_t offset, int whence)
        {
            if (!good() || (NULL == _buffer))
            {
                return set_good(false);
            }

            switch (whence)
            {
                case SEEK_SET:
                {
                    if (offset < 0)
                    {
                        return false;
                    }

                    _pos = offset;
                }
                break;

                case SEEK_CUR:
                {
                    if ((offset < 0) && (abs(offset) > _pos))
                    {
                        return false;
                    }

                    if ((offset+_pos) > static_cast<ssize_t>(_size))
                    {
                        return false;
                    }

                    _pos += offset;
                }
                break;

                case SEEK_END:
                {
                    if ((offset < 0) || (offset > static_cast<ssize_t>(_size)))
                    {
                        return false;
                    }

                    _pos = _size - offset;
                }
                break;

                default:
                    return false;
            }

            return true;
        }

        inline virtual ssize_t tell()
        {
            if (!good() || (NULL == _buffer))
            {
                set_good(false);

                return -1;
            }

            return _pos;
        }

    private:
        uint8_t *_buffer;
        size_t   _size;
        size_t   _capacity;
        ssize_t  _pos;
    };

    class BufferReaderWriter
        : BaseIO
        , public AbstractReader
        , public AbstractWriter
    {
    public:
        BufferReaderWriter(size_t reserve_=0)
            : BaseIO()
            , AbstractReader()
            , AbstractWriter()
            , _buffer  (NULL)
            , _size    (0)
            , _capacity(0)
            , _pos     (0)
        {
            if (reserve_ > 0)
            {
                alloc(reserve_);
            }
        }

        ~BufferReaderWriter()
        {
            free(_buffer);

            _buffer = NULL;
        }

        BufferReaderWriter & operator=(const BufferReaderWriter &ref)
        {
            if (_capacity != ref._capacity)
            {
                free(_buffer);

                _buffer = NULL;
            }

            if (write(ref._buffer,ref._capacity))
            {
                _size = ref._size;
                _pos  = ref._pos ;
            }

            return *this;
        }

        inline const uint8_t *data    () const { return _buffer  ; }
        inline size_t         size    () const { return _size    ; }
        inline size_t         capacity() const { return _capacity; }

        inline bool alloc(const size_t &s)
        {
            if ( s == 0 )
            {
                return true;
            }

            if ((_size + s) > _capacity)
            {
                size_t new_capacity = _size + s;

                _buffer = reinterpret_cast<uint8_t*> (
                    realloc(_buffer,new_capacity)
                );

                if (NULL != _buffer)
                {
                    _capacity = new_capacity;
                }
            }

            return set_good( NULL != _buffer );
        }

        inline virtual void rewind()
        {
            set_good(true);
            _pos = 0;
        }

        inline virtual void clear()
        {
            _size = 0;
            _pos  = 0;
        }

        inline virtual bool write(const void *data, size_t size)
        {
            if ((NULL == data) || (0 == size))
            {
                return true;
            }

            if (!alloc(size))
            {
                return set_good(false);
            }

            memcpy(&_buffer[_size], data, size);

            _size += size;

            return true;
        }

        inline virtual bool read(void *data, size_t size)
        {
            if (!good() || (NULL == _buffer))
            {
                return set_good(false);
            }

            if ((NULL == data) || (size < 1))
            {
                return false;
            }

            if ((_pos + size) > _size)
            {
                return false;
            }

            memcpy(data, &_buffer[_pos], size);

            _pos += size;

            return true;
        }

        inline virtual bool get(uint8_t &v)
        {
            if (!good() || (NULL == _buffer))
            {
                return set_good(false);
            }

            if ((_pos + 1) > static_cast<ssize_t>(_size))
            {
                return false;
            }

            v = _buffer[_pos++];

            return true;
        }

        inline virtual bool unget(const uint8_t &v)
        {
            if (!good() || (NULL == _buffer))
            {
                return set_good(false);
            }

            if (_pos < 1)
            {
                return false;
            }

            _buffer[--_pos] = v;

            return true;
        }

        inline virtual bool seek(ssize_t offset, int whence)
        {
            if (!good() || (NULL == _buffer))
            {
                return set_good(false);
            }

            switch (whence)
            {
                case SEEK_SET:
                {
                    if (offset < 0)
                    {
                        return false;
                    }

                    _pos = offset;
                }
                break;

                case SEEK_CUR:
                {
                    if ((offset < 0) && (abs(offset) > _pos))
                    {
                        return false;
                    }

                    if ((offset+_pos) > static_cast<ssize_t>(_size))
                    {
                        return false;
                    }

                    _pos += offset;
                }
                break;

                case SEEK_END:
                {
                    if ((offset < 0) || (offset > static_cast<ssize_t>(_size)))
                    {
                        return false;
                    }

                    _pos = _size - offset;
                }
                break;

                default:
                    return false;
            }

            return true;
        }

        inline virtual ssize_t tell()
        {
            if (!good() || (NULL == _buffer))
            {
                set_good(false);

                return -1;
            }

            return _pos;
        }

    private:
        uint8_t *_buffer;
        size_t   _size;
        size_t   _capacity;
        ssize_t  _pos;
    };

} // HOB

#endif // _HOB_IO_HPP__
