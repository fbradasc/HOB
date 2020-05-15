#if !defined(__HOB_VLIB_HPP__)
#define __HOB_VLIB_HPP__

#include <limits>
#include <climits>
#include "hob/codec/flib.hpp"

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
    namespace vlib
    {
        //========================================================================
        //
        // Variable Lenght Integer Binary (VLIB) are packed in 1 to 9 bytes
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

        class encoder
            : public flib::encoder
        {
        public:
            encoder(hobio::writer &os)
                : flib::encoder(os)
            {
            }

            virtual ~encoder()
            {
            }

            virtual inline size_t field_size(const uint8_t &v)
            {
                uint64_t rv;
                return get_varint_unzigzaged_size<uint8_t>(v,rv);
            }

            virtual inline size_t field_size(const uint16_t &v)
            {
                uint64_t rv;

                return get_varint_unzigzaged_size<uint16_t>(v,rv);
            }

            virtual inline size_t field_size(const uint32_t &v)
            {
                uint64_t rv;

                return get_varint_unzigzaged_size<uint32_t>(v,rv);
            }

            virtual inline size_t field_size(const uint64_t &v)
            {
                uint64_t rv;

                return get_varint_unzigzaged_size<uint64_t>(v,rv);
            }

            virtual inline size_t field_size(const int8_t &v)
            {
                uint64_t rv;

                return get_varint_unzigzaged_size<int8_t>(v,rv);
            }

            virtual inline size_t field_size(const int16_t &v)
            {
                uint64_t rv;

                return get_varint_unzigzaged_size<int16_t>(v,rv);
            }

            virtual inline size_t field_size(const int32_t &v)
            {
                uint64_t rv;

                return get_varint_unzigzaged_size<int32_t>(v,rv);
            }

            virtual inline size_t field_size(const int64_t &v)
            {
                uint64_t rv;

                return get_varint_unzigzaged_size<int64_t>(v,rv);
            }

            virtual inline size_t field_size(const bool &v)
            {
                return flib::encoder::field_size(v);
            }

            virtual inline size_t field_size(const float &v)
            {
                return flib::encoder::field_size(v);
            }

            virtual inline size_t field_size(const double &v)
            {
                return flib::encoder::field_size(v);
            }

            virtual inline size_t field_size(const long double &v)
            {
                return flib::encoder::field_size(v);
            }

        protected:
            template <class T>
            bool encode_integer(const T &v)
            {
                uint8_t  d[9];
                uint64_t rv;
                uint8_t  b = get_varint_unzigzaged_size<T>(v, rv);
                uint8_t  m = 0xff       >> b;
                uint32_t c = 0xfffffe00 >> b;

                *reinterpret_cast<uint64_t*>(&d[1]) = HOB_BIG_ENDIAN_ENCODE_64(rv);

                d[9-b] &= static_cast<uint8_t>(m & 0xff);
                d[9-b] |= static_cast<uint8_t>(c & 0xff);

                return writer().write(&d[9-b],b);
            }

            virtual inline bool encode(const uint8_t &v)
            {
                return encode_integer(v);
            }

            virtual inline bool encode(const uint16_t &v)
            {
                return encode_integer(v);
            }

            virtual inline bool encode(const uint32_t &v)
            {
                return encode_integer(v);
            }

            virtual inline bool encode(const uint64_t &v)
            {
                return encode_integer(v);
            }

            virtual inline bool encode(const int8_t &v)
            {
                return encode_integer(v);
            }

            virtual inline bool encode(const int16_t &v)
            {
                return encode_integer(v);
            }

            virtual inline bool encode(const int32_t &v)
            {
                return encode_integer(v);
            }

            virtual inline bool encode(const int64_t &v)
            {
                return encode_integer(v);
            }

            virtual inline bool encode(const bool &v)
            {
                return flib::encoder::encode(v);
            }

            virtual inline bool encode(const float &v)
            {
                return flib::encoder::encode(v);
            }

            virtual inline bool encode(const double &v)
            {
                return flib::encoder::encode(v);
            }

            virtual inline bool encode(const long double &v)
            {
                return flib::encoder::encode(v);
            }

            virtual inline bool encode(const string &v)
            {
                return flib::encoder::encode(v);
            }

            virtual inline bool encode(const hob &v)
            {
                return flib::encoder::encode(v);
            }

            virtual bool encode(const vector<bool> &v)
            {
                return flib::encoder::encode(v);
            }

        private:
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
        };

        class decoder
            : public flib::decoder
        {
        public:
            decoder(hobio::reader &is)
                : flib::decoder(is)
            {
            }

            virtual ~decoder()
            {
            }

            virtual inline bool decode_field(uint8_t &v, bool *changed = NULL)
            {
                return decode_integer<uint8_t>(v,changed);
            }

            virtual inline bool decode_field(uint16_t &v, bool *changed = NULL)
            {
                return decode_integer<uint16_t>(v,changed);
            }

            virtual inline bool decode_field(uint32_t &v, bool *changed = NULL)
            {
                return decode_integer<uint32_t>(v,changed);
            }

            virtual inline bool decode_field(uint64_t &v, bool *changed = NULL)
            {
                return decode_integer<uint64_t>(v, changed);
            }

            virtual inline bool decode_field(int8_t &v, bool *changed = NULL)
            {
                return decode_integer<int8_t>(v,changed);
            }

            virtual inline bool decode_field(int16_t &v, bool *changed = NULL)
            {
                return decode_integer<int16_t>(v,changed);
            }

            virtual inline bool decode_field(int32_t &v, bool *changed = NULL)
            {
                return decode_integer<int32_t>(v,changed);
            }

            virtual inline bool decode_field(int64_t &v, bool *changed = NULL)
            {
                return decode_integer<int64_t>(v,changed);
            }

        protected:
            template <class T>
            inline bool decode_integer(T &v, bool *changed = NULL)
            {
                uint8_t d[16] = { 0 };
                uint8_t b = 0;
                uint8_t c;
                uint8_t m;

                if (!reader().read(&d[7], sizeof(uint8_t)))
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
                    if (!reader().read(&d[8], b-1))
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
    } // namespace vlib
} // namespace hobio

#endif // __HOB_VLIB_HPP__
