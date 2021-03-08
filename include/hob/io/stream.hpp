#if !defined(__HOB_STREAM_HPP__)
#define __HOB_STREAM_HPP__

#include <stdio.h>
#include <string.h>
#include "hob/io/common.hpp"

namespace hobio
{
    class ostream
        : public common
        , public writer
    {
    public:
        ostream()
            : common()
            , writer()
            , _pf(stdout)
            , _can_close(false)
        {
        }

        ostream(FILE *pf)
            : common()
            , writer()
            , _pf(pf)
            , _can_close(false)
        {
        }

        ostream(const char *pathname, bool append = false)
            : common()
            , writer()
            , _pf(NULL)
            , _can_close(false)
        {
            ostream::open(pathname, append);
        }

        virtual ~ostream()
        {
            ostream::close();
        }

        bool open(const char *pathname, bool append = false)
        {
            ostream::close();

            if ((NULL == pathname) || (strcmp(pathname,"-") == 0))
            {
                _pf = stdout;
            }
            else
            {
                _pf = ::fopen(pathname, (append) ? "ab" : "wb");

                _can_close = true;
            }

            return set_good(NULL != _pf);
        }

        virtual bool write(const void *data, size_t size)
        {
            if ((NULL == data) || (0 == size))
            {
                return true;
            }

            if (!good() || (NULL == _pf))
            {
                return false;
            }

            return set_good( ::fwrite(data,1,size,_pf) == size );
        }

        void close()
        {
            if (NULL != _pf)
            {
                ::fflush(_pf);

                if (_can_close)
                {
                    ::fclose(_pf);
                }
            }

            _pf = NULL;

            _can_close = false;
        }

    private:
        FILE *_pf;
        bool  _can_close;
    };

    class istream
        : public common
        , public reader
    {
    public:
        istream()
            : common()
            , reader()
            , _pf(stdin)
            , _can_close(false)
        {
        }

        istream(FILE *pf)
            : common()
            , reader()
            , _pf(pf)
            , _can_close(false)
        {
        }

        istream(const char *pathname)
            : common()
            , reader()
            , _can_close(false)
        {
            istream::open(pathname);
        }

        virtual ~istream()
        {
            istream::close();
        }

        bool open(const char *pathname, bool binary=true)
        {
            istream::close();

            if ((NULL == pathname) || (strcmp(pathname,"-") == 0))
            {
                _pf = stdin;
            }
            else
            {
                _pf = ::fopen(pathname, (binary) ? "rb" : "r");

                _can_close = true;
            }

            return set_good(NULL != _pf);
        }

        virtual bool read(void *data, size_t size)
        {
            if (!good() || (NULL == _pf) || ::feof(_pf))
            {
                return set_good(false);
            }

            if ((NULL == data) || (size < 1))
            {
                return false;
            }

            return set_good(::fread(data,1,size,_pf) == size);
        }

        virtual bool get(uint8_t &v)
        {
            if (!good() || (NULL == _pf) || ::feof(_pf))
            {
                return set_good(false);
            }

            if (!good() || (NULL == _pf) || ::feof(_pf))
            {
                return set_good(false);
            }

            int rv = fgetc(_pf);

            if (EOF == rv)
            {
                return set_good(false);
            }

            v = static_cast<uint8_t>(rv);

            return true;
        }

        virtual bool unget(const uint8_t &v)
        {
            if (!good() || (NULL == _pf))
            {
                return set_good(false);
            }

            return set_good( ungetc(static_cast<int>(v), _pf) != EOF );
        }

        virtual bool seek(long offset, int whence)
        {
            if (!good() || (NULL == _pf) || ::feof(_pf))
            {
                return set_good(false);
            }

            return ( ::fseek(_pf,offset,whence) >= 0 );
        }

        virtual ssize_t tell()
        {
            if (NULL == _pf)
            {
                return -1;
            }

            return ::ftell(_pf);
        }

        void close()
        {
            if (NULL != _pf)
            {
                // ::fflush(_pf);

                if (_can_close)
                {
                    ::fclose(_pf);
                }
            }

            _pf = NULL;

            _can_close = false;
        }

    private:
        FILE *_pf;
        bool  _can_close;
    };
} // namespace hobio

#endif // _HOB_IO_HPP__
