#if !defined(__HOB_FLIB_HPP__)
#define __HOB_FLIB_HPP__

#include <unistd.h>
#include <limits>
#include <climits>
#include "hob/io/buffer.hpp"
#include "hob/hob.hpp"

using namespace std;
using namespace nonstd;

//========================================================================
//
// Variable Lenght Integer (VARINT) are packed in 1 to 9 bytes
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

namespace hobio
{
    namespace flat
    {
        enum Encoding
        {
            NATIVE,
            VARINT
        };

        class encoder
            : public hob::encoder
        {
        public:
            encoder(hobio::writer &os_, Encoding encoding_=VARINT)
                : hob::encoder()
                , _encoding(encoding_)
                , _os(os_)
            {
            }

            virtual ~encoder()
            {
            }

            encoder & operator << (Encoding encoding_)
            {
                _encoding = encoding_;

                return *this;
            }

            Encoding encoding()
            {
                return _encoding;
            }

            virtual bool encode_header(const char     *name ,
                                       const string   &value,
                                       const hob::UID &id   ,
                                       const size_t   &payload)
            {
                (void)name;
                (void)value;

                return encode_header(static_cast<const char*>(NULL),
                                     static_cast<const char*>(NULL),
                                     id,payload);
            }

            virtual bool encode_header(const char     *name ,
                                       const hob::UID &value,
                                       const hob::UID &id   ,
                                       const size_t   &payload)
            {
                (void)name;
                (void)value;

                return encode_header(static_cast<const char*>(NULL),
                                     static_cast<const char*>(NULL),
                                     id,payload);
            }

            virtual bool encode_header(const char     *name ,
                                       const char     *value,
                                       const hob::UID &id   ,
                                       const size_t   &payload)
            {
                (void)name;
                (void)value;

                if (!_os.alloc(field_size(id)
                               +
                               ((payload>0)
                                   ? (field_size(payload) + payload)
                                   : 0)))
                {
                    return false;
                }

                if (!encode(id))
                {
                    return false;
                }

                if ((payload > 0) && !encode(payload))
                {
                    return false;
                }

                return true;
            }

            virtual inline bool encode_field_pre(const char *type = NULL,
                                                 const char *name = NULL)
            {
                (void)type;
                (void)name;

                return true;
            }

            virtual inline bool encode_field_post(const char *type = NULL,
                                                  const char *name = NULL)
            {
                (void)type;
                (void)name;

                return true;
            }

            virtual inline bool encode_footer()
            {
                return true;
            }

            virtual inline size_t field_size(const uint8_t &v)
            {
                return get_size<uint8_t>(v);
            }

            virtual inline size_t field_size(const uint16_t &v)
            {
                return get_size<uint16_t>(v);
            }

            virtual inline size_t field_size(const uint32_t &v)
            {
                return get_size<uint32_t>(v);
            }

            virtual inline size_t field_size(const uint64_t &v)
            {
                return get_size<uint64_t>(v);
            }

            virtual inline size_t field_size(const int8_t &v)
            {
                return get_size<int8_t>(v);
            }

            virtual inline size_t field_size(const int16_t &v)
            {
                return get_size<int16_t>(v);
            }

            virtual inline size_t field_size(const int32_t &v)
            {
                return get_size<int32_t>(v);
            }

            virtual inline size_t field_size(const int64_t &v)
            {
                return get_size<int64_t>(v);
            }

            virtual inline size_t field_size(const bool &v)
            {
                return sizeof(v);
            }

            virtual inline size_t field_size(const float &v)
            {
                return sizeof(v);
            }

            virtual inline size_t field_size(const double &v)
            {
                return sizeof(v);
            }

            virtual inline size_t field_size(const long double &v)
            {
                return sizeof(v);
            }

        protected:
            inline hobio::writer &writer()
            {
                return _os;
            }

            virtual inline bool encode(const uint8_t &v)
            {
                return encode_integer<uint8_t>(v);
            }

            virtual inline bool encode(const uint16_t &v)
            {
                return encode_integer<uint16_t>(v);
            }

            virtual inline bool encode(const uint32_t &v)
            {
                return encode_integer<uint32_t>(v);
            }
            virtual inline bool encode(const uint64_t &v)
            {
                return encode_integer<uint64_t>(v);
            }

            virtual inline bool encode(const int8_t &v)
            {
                return encode_integer<int8_t>(v);
            }

            virtual inline bool encode(const int16_t &v)
            {
                return encode_integer<int16_t>(v);
            }

            virtual inline bool encode(const int32_t &v)
            {
                return encode_integer<int32_t>(v);
            }

            virtual inline bool encode(const int64_t &v)
            {
                return encode_integer<int64_t>(v);
            }

            virtual inline bool encode(const bool &v)
            {
                return _os.write(&v,sizeof(v));
            }

            virtual inline bool encode(const float &v)
            {
                return _os.write(&v,sizeof(v));
            }

            virtual inline bool encode(const double &v)
            {
                return _os.write(&v,sizeof(v));
            }

            virtual inline bool encode(const long double &v)
            {
                return _os.write(&v,sizeof(v));
            }

            virtual inline bool encode(const string &v)
            {
                return encode(v.size(),v.data());
            }

            virtual inline bool encode(const hob &v)
            {
                return (v >> *this);
            }

            virtual bool encode(const vector<bool> &v)
            {
                if (!encode(v.size()))
                {
                    return false;
                }

                uint8_t bits[(v.size()+7)>>3];

                memset(bits, 0, ((v.size()+7)>>3));

                for (size_t j=0; j<v.size(); j++)
                {
                    bits[j>>3] |= (v[j]<<(j%8));
                }

                return encode(((v.size()+7)>>3), bits);
            }

            virtual inline bool encode_bitset(size_t size_, const uint8_t *bits)
            {
                return (NULL != bits) && encode(((size_+7)>>3),bits);
            }

            virtual bool encode_variant_begin(hob::UID id, hob::hob_t type)
            {
                return encode(id) && encode(type);
            }

            virtual bool encode_variant_end()
            {
                return true;
            }

            virtual bool encode_vector_begin(size_t len)
            {
                return encode(len);
            }

            virtual void encode_vector_item_pre(size_t i, size_t len)
            {
                (void)i;
                (void)len;
            }

            virtual void encode_vector_item_post(size_t i, size_t len)
            {
                (void)i;
                (void)len;
            }

            virtual bool encode_vector_end()
            {
                return true;
            }

            virtual bool encode_optional_begin(const bool &has_value)
            {
                 return encode(has_value);
            }

            virtual void encode_optional_item_pre()
            {
            }

            virtual void encode_optional_item_post()
            {
            }

            virtual bool encode_optional_end(const bool &has_value)
            {
                (void)has_value;

                return true;
            }

            virtual bool encode_map_begin(const size_t &len)
            {
                return encode(len);
            }

            virtual void encode_map_item_key_pre()
            {
            }

            virtual void encode_map_item_key_post()
            {
            }

            virtual void encode_map_item_value_pre()
            {
            }

            virtual void encode_map_item_value_post(const size_t &remaining)
            {
                (void)remaining;
            }

            virtual bool encode_map_end()
            {
                return true;
            }

        private:
            Encoding       _encoding;
            hobio::writer &_os;

            inline bool encode(size_t size_, const void *v)
            {
                if (!encode(size_))
                {
                    return false;
                }

                return _os.write(v,size_);
            }

            template <class T>
            inline size_t get_size(const T &v)
            {
                if (NATIVE == _encoding)
                {
                    return sizeof(T);
                }

                uint64_t rv;

                return get_varint_unzigzaged_size<T>(v,rv);
            }

            template <class T>
            bool encode_integer(const T &v)
            {
                if (NATIVE == _encoding)
                {
                    return _os.write(&v,sizeof(v));
                }

                return encode_varint<T>(v);
            }

            // VARINT encoding
            //
            template <class T>
            inline size_t get_varint_unzigzaged_size(const T &v,
                                                     uint64_t &rv)
            {
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
            }

            template <class T>
            inline bool encode_varint(const T &v)
            {
                uint8_t  d[9] = { 0 };
                uint64_t rv;
                uint8_t  b = get_varint_unzigzaged_size<T>(v, rv);
                uint8_t  m = 0xff       >> b;
                uint32_t c = 0xfffffe00 >> b;

                *reinterpret_cast<uint64_t*>(&d[1]) = HOB_BIG_ENDIAN_ENCODE_64(rv);

                d[9-b] &= static_cast<uint8_t>(m & 0xff);
                d[9-b] |= static_cast<uint8_t>(c & 0xff);

                return _os.write(&d[9-b],b);
            }
        };

        class decoder
            : public hob::decoder
        {
        public:
            decoder(hobio::reader &is_, Encoding encoding_=VARINT)
                : hob::decoder()
                , _encoding(encoding_)
                , _is(is_)
                , _bs(NULL)
            {
            }

            virtual decoder & operator << (Encoding encoding_)
            {
                _encoding = encoding_;

                return *this;
            }

            virtual ~decoder()
            {
                if (NULL != _bs)
                {
                    delete _bs;

                    _bs = NULL;
                }
            }

            Encoding encoding()
            {
                return _encoding;
            }

            virtual bool load(bool update=true)
            {
                (void)update;

                return true;
            }

            virtual inline bool bufferize(size_t size)
            {
                if ((size < 1) || (stream().tell() >= 0))
                {
                    return true;
                }

                if (NULL == _bs)
                {
                    _bs = new hobio::iobuffer(stream(), size);
                }
                else
                {
                    _bs->clear();

                    _bs->load(size);
                }

                return ( (NULL != _bs) && _bs->good() );
            }

            inline bool decode_field(uint8_t &v, bool *changed = NULL)
            {
                return decode_integer<uint8_t>(v,changed);
            }

            inline bool decode_field(uint16_t &v, bool *changed = NULL)
            {
                return decode_integer<uint16_t>(v,changed);
            }

            inline bool decode_field(uint32_t &v, bool *changed = NULL)
            {
                return decode_integer<uint32_t>(v,changed);
            }

            inline bool decode_field(uint64_t &v, bool *changed = NULL)
            {
                return decode_integer<uint64_t>(v, changed);
            }

            inline bool decode_field(int8_t &v, bool *changed = NULL)
            {
                return decode_integer<int8_t>(v,changed);
            }

            inline bool decode_field(int16_t &v, bool *changed = NULL)
            {
                return decode_integer<int16_t>(v,changed);
            }

            inline bool decode_field(int32_t &v, bool *changed = NULL)
            {
                return decode_integer<int32_t>(v,changed);
            }

            inline bool decode_field(int64_t &v, bool *changed = NULL)
            {
                return decode_integer<int64_t>(v,changed);
            }

            virtual bool seek(ssize_t offset, int whence)
            {
                return reader().seek(offset, whence);
            }

            virtual ssize_t tell()
            {
                return reader().tell();
            }

            virtual void skip(size_t size)
            {
                return reader().skip(size);
            }

        protected:
            inline virtual bool read_field(void *v, size_t size)
            {
                return reader().read(v, size);
            }

            hobio::writer *writer(bool clear=false)
            {
                if (NULL == _bs)
                {
                    _bs = new hobio::iobuffer();
                }

                if ((NULL != _bs) && _bs->good())
                {
                    if (clear)
                    {
                        _bs->clear();
                    }

                    return _bs;
                }

                return NULL;
            }

            hobio::reader &reader()
            {
                return (NULL != _bs) ? *_bs : _is;
            }

            hobio::reader &stream()
            {
                return _is;
            }

            hobio::iobuffer *buffer()
            {
                return _bs;
            }

        private:
            Encoding         _encoding;
            hobio::reader   &_is;
            hobio::iobuffer *_bs;

            template <class T>
            inline bool decode_integer(T &v, bool *changed = NULL)
            {
                if (NATIVE == _encoding)
                {
                    T rv = T();

                    if (!read_field(&rv,sizeof(T)))
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

                return decode_varint<T>(v, changed);
            }

            // VARINT decoding
            //
            template <class T>
            inline bool decode_varint(T &v, bool *changed = NULL)
            {
                uint8_t d[16] = { 0 };
                uint8_t b = 0;
                uint8_t c;
                uint8_t m;

                if (!read_field(&d[7], sizeof(uint8_t)))
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
                    if (!read_field(&d[8], b-1))
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

                if (changed)
                {
                    *changed = ( v != rv );
                }

                v = rv;

                return true;
            }
        };
    } // namespace flat
} // namespace hobio

#endif // __HOB_FLIB_HPP__
