#if !defined(__HOB_COMMON_HPP__)
#define __HOB_COMMON_HPP__

#include <unistd.h>
#include <iostream>
#include <cstdio>

#include <stdio.h>
#include <cstring>
#include <climits>

#include <inttypes.h>
#include <bitset>
#include <string>
#include <vector>
#include <map>
#include "hob/std/optional.hpp"

#if defined(HOB_DEBUG)
#define M_LOG(...)            printf(__VA_ARGS__); \
                              printf(" @ %s:%s:%d", __FUNCTION__, __FILE__, __LINE__); \
                              printf("\n");
#else
#define M_LOG(...)
#endif

#define HSEED                 65599llu
// #define HSEED                 11400714819323198485llu

#define HAS_DYNAMIC_FIELDS_FLAG_BIT_POS 1

// #define DYNAMIC_MASK(v)  ((v) & ~(HAS_DYNAMIC_FIELDS_FLAG_BIT_POS << 1))
#define DYNAMIC_MASK(v)  (v)

using namespace std;
using namespace nonstd;

namespace hobio
{
    class encoder;
    class decoder;

    typedef uint64_t uid_t;

    static const uid_t UNDEFINED = ULLONG_MAX;

    class UID
    {
    public:
        UID(): _id(hobio::UNDEFINED), _np(-1) {}

        UID(const hobio::uid_t & in): _id(0), _np(-1) { update(in); }
        UID(const char         * in): _id(0), _np(-1) { update(in); }
        UID(const string       & in): _id(0), _np(-1) { update(in); }

        UID(const UID & ref) { *this = ref; }

        inline operator const hobio::uid_t &() const { return _id; }

        inline hobio::uid_t with_variants(bool has_variants) const
        {
            (void)has_variants;

            return _id /* | (HAS_DYNAMIC_FIELDS_FLAG_BIT_POS << has_variants) */;
        }

        inline UID & operator=(const uid_t & ref)
        {
            _id = DYNAMIC_MASK(ref);
            _np = -1;

            return *this;
        }

        inline UID & operator=(const UID & ref)
        {
            _id = DYNAMIC_MASK(ref._id);
            _np = ref._np;

            return *this;
        }

        inline bool operator==(const UID & ref) const
        {
            return DYNAMIC_MASK(_id) == DYNAMIC_MASK(ref._id);
        }

        inline bool operator!=(const UID & ref) const
        {
            return DYNAMIC_MASK(_id) != DYNAMIC_MASK(ref._id);
        }

        inline bool operator<(const UID & ref) const
        {
            return (DYNAMIC_MASK(_id) < DYNAMIC_MASK(ref._id));
        }
/*
        inline UID & operator<<=(const uid_t & ref)
        {
            _id <<= ref;

            return *this;
        }

        inline UID & operator|=(const uid_t & ref)
        {
            _id |= DYNAMIC_MASK(ref);

            return *this;
        }
*/
        inline void update(const hobio::uid_t & in)
        {
            _np++;

            _id = in;
        }

        inline void update(const string & in)
        {
            update(in.c_str());
        }

        void update(const char * in)
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
                //===========================================================//
                //                                                           //
                //             +----------+---+---+                          //
                // UID[64b] := | 63 ... 2 | 1 | 0 |                          //
                //             +----------+---+---+                          //
                //                   ^      ^   ^                            //
                //                   |      |   |                            //
                //                   |      |   `--- Has static  fields flag //
                //                   |      `------- Has dynamic fields flag //
                //                   `-------------- 62 bits HOB hash code   //
                //                                                           //
                //=============================================================

                _id <<= 1 /*2*/;

                // HOBs with    parameters have an odd  ID
                // HOBs without parameters have an even ID
                //
                _id |= (_np > 0);

                // Avoid parameters check multiple appliance
                //
                _np = -1;
            }
        }

    private:
        hobio::uid_t _id;
        ssize_t      _np;
    };

    class codec
    {
    public:
        virtual size_t size  (hobio::encoder &e) const = 0;
        virtual bool   encode(hobio::encoder &e) const = 0;
        virtual bool   decode(hobio::decoder &d, bool *changed = NULL) = 0;
    };

    class common
    {
    public:
        common()
            : _good(true)
        {
        }

        virtual ~common()
        {
        }

        inline bool good()
        {
            return _good;
        }

        inline virtual void clear()
        {
        }

        inline virtual void close()
        {
        }

    protected:
        inline bool set_good(bool v)
        {
            _good = v;
            return _good;
        }

    private:
        bool _good;
    };

    class writer
    {
    public:
        static const bool CLEAR=true;

        writer()
            : _obuffer(*this)
            , _ostream(&_obuffer)
        {
        }

        virtual ~writer()
        {
        }

        virtual bool alloc(const size_t &s)
        {
            (void)s;

            return true;
        }

        virtual bool write(const void *data, size_t size) = 0;

        ostream &os() { return _ostream; }

    protected:
        class buffer
            : public streambuf
        {
        public:
            buffer(writer &os): _os(os)
            {
            }

        protected:
            virtual int_type overflow(int_type c)
            {
                uint8_t out = (c & 0xff);

                return ((c == EOF) || !_os.write(&out,1)) ? EOF : c;
            }

        private:
            writer &_os;
        };

    private:
        buffer  _obuffer;
        ostream _ostream;
    };

    class reader
    {
    public:
        virtual ~reader()
        {
        }

        virtual bool get(uint8_t &v)
        {
            (void)v;

            return true;
        }

        virtual bool unget(const uint8_t &v)
        {
            (void)v;

            return true;
        }

        virtual bool seek(ssize_t offset, int whence)
        {
            (void)offset;
            (void)whence;

            return true;
        }

        virtual ssize_t tell()
        {
            return -1;
        }

        virtual void skip(size_t size)
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
    };
} // namespace hobio

#endif // __HOB_COMMON_HPP__
