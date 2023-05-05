#ifndef __HOB_DECODER_HPP__
#define __HOB_DECODER_HPP__

#include "hob/io/common.hpp"

namespace hobio
{
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

        bool decode_field(hobio::codec &c, bool *changed = NULL)
        {
            bool retval = c.decode(*this, changed);

            return retval;
        }

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
}

#endif // __HOB_DECODER_HPP__
