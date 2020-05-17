#if !defined(__HOB_FLIB_HPP__)
#define __HOB_FLIB_HPP__

#include <unistd.h>
// #include <limits>
// #include <cstring>
// #include <iostream>
// #include <iomanip>
#include "hob/io/buffer.hpp"
#include "hob/hob.hpp"

using namespace std;
using namespace nonstd;

namespace hobio
{
    namespace flib
    {
        // Fixed Lenght Integer Binary encoder
        //
        class encoder
            : public hob::encoder
        {
        public:
            encoder(hobio::writer &os)
                : hob::encoder()
                , _os(os)
            {
            }

            virtual ~encoder()
            {
            }

            virtual bool encode_header(const char     *name ,
                                       const string   &value,
                                       const hob::UID &id   ,
                                       const size_t   &payload)
            {
                return encode_header(static_cast<const char*>(NULL),
                                     static_cast<const char*>(NULL),
                                     id,payload);
            }

            virtual bool encode_header(const char     *name ,
                                       const hob::UID &value,
                                       const hob::UID &id   ,
                                       const size_t   &payload)
            {
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
                               (payload>0)
                                   ? field_size(payload) + payload
                                   : 0))
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
                return sizeof(v);
            }

            virtual inline size_t field_size(const uint16_t &v)
            {
                return sizeof(v);
            }

            virtual inline size_t field_size(const uint32_t &v)
            {
                return sizeof(v);
            }

            virtual inline size_t field_size(const uint64_t &v)
            {
                return sizeof(v);
            }

            virtual inline size_t field_size(const int8_t &v)
            {
                return sizeof(v);
            }

            virtual inline size_t field_size(const int16_t &v)
            {
                return sizeof(v);
            }

            virtual inline size_t field_size(const int32_t &v)
            {
                return sizeof(v);
            }

            virtual inline size_t field_size(const int64_t &v)
            {
                return sizeof(v);
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
                return _os.write(&v,sizeof(v));
            }

            virtual inline bool encode(const uint16_t &v)
            {
                return _os.write(&v,sizeof(v));
            }

            virtual inline bool encode(const uint32_t &v)
            {
                return _os.write(&v,sizeof(v));
            }
            virtual inline bool encode(const uint64_t &v)
            {
                return _os.write(&v,sizeof(v));
            }

            virtual inline bool encode(const int8_t &v)
            {
                return _os.write(&v,sizeof(v));
            }

            virtual inline bool encode(const int16_t &v)
            {
                return _os.write(&v,sizeof(v));
            }

            virtual inline bool encode(const int32_t &v)
            {
                return _os.write(&v,sizeof(v));
            }

            virtual inline bool encode(const int64_t &v)
            {
                return _os.write(&v,sizeof(v));
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
            hobio::writer &_os;

            inline bool encode(size_t size_, const void *v)
            {
                if (!encode(size_))
                {
                    return false;
                }

                return _os.write(v,size_);
            }

        };

        class decoder
            : public hob::decoder
        {
        public:
            decoder(hobio::reader &is)
                : hob::decoder()
                , _is(is)
                , _bs(NULL)
            {
            }

            virtual ~decoder()
            {
                if (NULL != _bs)
                {
                    delete _bs;

                    _bs = NULL;
                }
            }

            virtual bool load(bool update=true)
            {
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

            template <class T>
            inline bool decode_integer(T &v, bool *changed = NULL)
            {
                T rv;

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
            hobio::reader   &_is;
            hobio::iobuffer *_bs;
        };
    } // namespace flib
} // namespace hobio

#endif // __HOB_FLIB_HPP__
