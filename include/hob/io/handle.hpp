#if !defined(__HOB_HANDLE_HPP__)
#define __HOB_HANDLE_HPP__

#include <unistd.h>
#include "hob/io/common.hpp"

namespace hobio
{
    class ohandle
        : public common
        , public writer
    {
    public:
        ohandle(int fd)
            : common()
            , writer()
            , _fd(fd)
        {
        }

        virtual ~ohandle()
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

    class ihandle
        : public common
        , public reader
    {
    public:
        ihandle(int fd)
            : common()
            , reader()
            , _fd(fd)
            , _ungetc(0)
        {
        }

        virtual ~ihandle()
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

    class iohandle
        : public common
        , public reader
        , public writer
    {
    public:
        iohandle(int fd)
            : common()
            , reader()
            , writer()
            , _fd(fd)
            , _ungetc(0)
        {
        }

        virtual ~iohandle()
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
} // namespace hobio

#endif // __HOB_HANDLE_HPP__
