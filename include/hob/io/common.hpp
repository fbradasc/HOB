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
                              printf(" @ %s:%d", __PRETTY_FUNCTION__, __LINE__); \
                              printf("\n");
#else
#define M_LOG(...)
#endif

#define HSEED                 65599llu
// #define HSEED                 11400714819323198485llu

using namespace std;
using namespace nonstd;

namespace hobio
{
    typedef uint64_t UID;

    static const UID UNDEFINED = ULLONG_MAX;

    class encoder;
    class decoder;

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
