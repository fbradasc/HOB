#if !defined(__HOB_COMMON_HPP__)
#define __HOB_COMMON_HPP__

#include <unistd.h>

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

        virtual ~writer()
        {
        }

        virtual bool alloc(const size_t &s)
        {
            return true;
        }

        virtual bool write(const void *data, size_t size) = 0;
    };

    class reader
    {
    public:
        virtual ~reader()
        {
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
