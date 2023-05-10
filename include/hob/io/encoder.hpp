#ifndef __HOB_ENCODER_HPP__
#define __HOB_ENCODER_HPP__

#include "hob/io/common.hpp"

namespace hobio
{
#if !defined(EMBED_VARIANT)
    class variant;
#endif // EMBED_VARIANT

    namespace json
    {
        class decoder;
    }

    class encoder
    {
    public:
        virtual ~encoder()
        {
        }

        virtual bool encode_header(const char         *name ,
                                   const string       &value,
                                   const hobio::uid_t &id   ,
                                   const size_t       &payload) = 0;

        virtual bool encode_header(const char         *name ,
                                   const hobio::uid_t &value,
                                   const hobio::uid_t &id   ,
                                   const size_t       &payload) = 0;

        virtual bool encode_header(const char         *name ,
                                   const char         *value,
                                   const hobio::uid_t &id   ,
                                   const size_t       &payload) = 0;

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

        inline size_t field_size(const hobio::codec &c)
        {
            return c.size(*this);
        }

        inline size_t field_size(const string &v)
        {
            return field_size(v.size()) + v.size();
        }

        inline size_t field_size(size_t size_, const void *v)
        {
            (void)v;

            return field_size(size_) + size_;
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

#if !defined(EMBED_VARIANT)
    protected:
        friend class hobio::json::decoder;
        friend class hobio::variant;
#endif // EMBED_VARIANT

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
        virtual bool encode(const vector<bool> &v) = 0;

        bool encode(const hobio::codec &c)
        {
            return c.encode(*this);
        }

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

        virtual bool encode_variant_begin(const hobio::uid_t &id, uint8_t type) = 0;
        virtual bool encode_variant_end() = 0;
    };
}

#endif // __HOB_ENCODER_HPP__
