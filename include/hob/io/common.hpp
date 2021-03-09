#if !defined(__HOB_COMMON_HPP__)
#define __HOB_COMMON_HPP__

#include <unistd.h>
#include <inttypes.h>
#include <iostream>
#include <cstdio>

using namespace std;

namespace hobio
{
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
