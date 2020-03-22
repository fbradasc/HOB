#if !defined(__HOB_IO_HPP__)
#define __HOB_IO_HPP__

#include <unistd.h>

namespace HOBIO
{
    class BaseIO
    {
    public:
        BaseIO(): _good(true) {}

        inline bool good() { return _good; }

    protected:
        inline bool set_good(bool v) { _good = v; return _good; }

    private:
        bool _good;
    };

    class AbstractWriter
    {
    public:
        AbstractWriter() { }

        virtual      ~AbstractWriter()                     {}
        virtual bool  alloc(const size_t &s)               { return true; }
        virtual bool  write(const void *data, size_t size) { return true; }
    };

    class AbstractReader
    {
    public:
        AbstractReader() { }

        virtual ~AbstractReader()
        {
        }

        virtual bool read(void *data, size_t size)
        {
            return true;
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

        virtual void ignore(size_t size)
        {
            for (uint8_t v; (size>0) && get(v); size--);
        }
    };

    class FileWriter
        : public BaseIO
        , public AbstractWriter
    {
    public:
        FileWriter()
            : BaseIO()
            , AbstractWriter()
            , _pf(stdout)
            , _can_close(false)
        {
        }

        FileWriter(FILE *pf)
            : BaseIO()
            , AbstractWriter()
            , _pf(pf)
            , _can_close(false)
        {
        }

        FileWriter(const char *pathname, bool append = false)
            : BaseIO()
            , AbstractWriter()
            , _pf(NULL)
            , _can_close(false)
        {
            open(pathname, append);
        }

        ~FileWriter()
        {
            close();
        }

        bool open(const char *pathname, bool append = false)
        {
            if (NULL != _pf)
            {
                return set_good(false);
            }

            _pf = ::fopen(pathname, (append) ? "a" : "w");

            return set_good(_can_close = (NULL != _pf));
        }

        virtual bool write(const void *data, size_t size)
        {
            if ((NULL == data) || (0 == size))
            {
                return true;
            }

            if (NULL == _pf)
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

    class FileReader
        : public BaseIO
        , public AbstractReader
    {
    public:
        FileReader()
            : BaseIO()
            , AbstractReader()
            , _pf(stdin)
            , _can_close(false)
        {
        }

        FileReader(FILE *pf)
            : BaseIO()
            , AbstractReader()
            , _pf(pf)
            , _can_close(false)
        {
        }

        FileReader(const char *pathname)
            : BaseIO()
            , AbstractReader()
            , _can_close(false)
        {
            open(pathname);
        }

        ~FileReader()
        {
            close();
        }

        bool open(const char *pathname, bool binary=true)
        {
            if (NULL != _pf)
            {
                return set_good(false);
            }

            _pf = ::fopen(pathname, (binary) ? "rb" : "r");

            return set_good( _can_close = (NULL != _pf) );
        }

        virtual bool read(void *data, size_t size)
        {
            if (NULL == _pf)
            {
                return false;
            }

            return set_good(!::feof(_pf) && (::fread(data,1,size,_pf) == size));
        }

        virtual bool get(uint8_t &v)
        {
            if (NULL == _pf || ::feof(_pf))
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
            if (NULL == _pf)
            {
                return set_good(false);
            }

            return set_good( ungetc(static_cast<int>(v), _pf) != EOF );
        }

        virtual bool seek(long offset, int whence)
        {
            if (NULL == _pf)
            {
                return set_good(false);
            }

            return set_good( ::fseek(_pf,offset,whence) >= 0 );
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
            if (_can_close)
            {
                ::fclose(_pf);
            }

            _pf = NULL;

            _can_close = false;
        }

    private:
        FILE *_pf;
        bool  _can_close;
    };

    class FdWriter
        : public BaseIO
        , public AbstractWriter
    {
    public:
        FdWriter(int fd)
            : BaseIO()
            , AbstractWriter()
            , _fd(fd)
        {
        }

        ~FdWriter()
        {
            close();
        }

        virtual bool write(const void *data, size_t size)
        {
            if ((NULL == data) || (0 == size))
            {
                return true;
            }

            if (_fd < 0)
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

    class FdReader
        : public BaseIO
        , public AbstractReader
    {
    public:
        FdReader(int fd)
            : BaseIO()
            , AbstractReader()
            , _fd(fd)
            , _ungetc(0)
        {
        }

        ~FdReader()
        {
            close();
        }

        virtual bool read(void *data, size_t size)
        {
            if (_fd < 0)
            {
                return set_good(false);
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
            return set_good( ::lseek(_fd,static_cast<off_t>(offset),whence) >= 0 );
        }

        virtual ssize_t tell()
        {
            return static_cast<ssize_t>(::lseek(_fd,0,SEEK_CUR));
        }

        void close()
        {
            _fd = -1;
        }

    private:
        int      _fd;
        uint16_t _ungetc;
    };

    class FdReaderWriter
        : public BaseIO
        , public AbstractReader
        , public AbstractWriter
    {
    public:
        FdReaderWriter(int fd)
            : BaseIO()
            , AbstractReader()
            , AbstractWriter()
            , _fd(fd)
            , _ungetc(0)
        {
        }

        ~FdReaderWriter()
        {
            close();
        }

        virtual bool write(const void *data, size_t size)
        {
            if ((NULL == data) || (0 == size))
            {
                return true;
            }

            if (_fd < 0)
            {
                return set_good(false);
            }

            _ungetc = 0;

            return set_good( ::write(_fd,data,size) == static_cast<ssize_t>(size) );
        }

        virtual bool read(void *data, size_t size)
        {
            if (_fd < 0)
            {
                return set_good(false);
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
            if (_fd < 0)
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
            return set_good( ::lseek(_fd,static_cast<off_t>(offset),whence) >= 0 );
        }

        virtual ssize_t tell()
        {
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

    class BufferWriter
        : public BaseIO
        , public AbstractWriter
    {
    public:
        BufferWriter(size_t reserve_=0)
            : BaseIO()
            , AbstractWriter()
            , _buffer  (NULL)
            , _size    (0)
            , _capacity(0)
        {
            alloc(reserve_);
        }

        ~BufferWriter()
        {
            free(_buffer);

            _buffer = NULL;
        }

        const uint8_t *data    () const { return _buffer  ; }
        size_t         size    () const { return _size    ; }
        size_t         capacity() const { return _capacity; }

        inline bool alloc(const size_t &s)
        {
            if ((_size + s) > _capacity)
            {
                _capacity = _size + s;

                _buffer = reinterpret_cast<uint8_t*> (
                    realloc(_buffer,_capacity)
                );
            }

            return set_good( NULL != _buffer );
        }

        virtual void clear() { _size = 0; }

        virtual bool write(const void *data, size_t size)
        {
            if ((NULL == data) || (0 == size))
            {
                return true;
            }

            if (!alloc(size))
            {
                return set_good( false );
            }

            memcpy(&_buffer[_size], data, size);

            _size += size;

            return true;
        }

    private:
        uint8_t *_buffer;
        size_t   _size;
        size_t   _capacity;
    };

    class BufferReader
        : public BaseIO
        , public AbstractReader
    {
    public:
        BufferReader(const BufferWriter &ref)
            : BaseIO()
            , AbstractReader()
            , _buffer       (NULL)
            , _pos          (0)
        {
            if (ref.capacity() > 0)
            {
                _buffer = reinterpret_cast<uint8_t*>
                (
                    malloc(ref.capacity())
                );

                if (NULL != _buffer)
                {
                    memcpy(_buffer,ref.data(),ref.capacity());

                    _size     = ref.size();

                    _capacity = ref.capacity();
                }
            }

            set_good( NULL != _buffer );
        }

        ~BufferReader()
        {
            free(_buffer);

            _buffer = NULL;
        }

        const uint8_t *data    () const { return _buffer  ; }
        size_t         size    () const { return _size    ; }
        size_t         capacity() const { return _capacity; }

        virtual bool read(void *data, size_t size)
        {
            if (NULL == _buffer)
            {
                return set_good(false);
            }

            if ((_pos + size) > _size)
            {
                return false;
            }

            memcpy(data, &_buffer[_pos], size);

            _pos += size;

            return true;
        }

        virtual bool get(uint8_t &v)
        {
            if (NULL == _buffer)
            {
                return set_good(false);
            }

            if ((_pos + 1) > static_cast<ssize_t>(_size))
            {
                return false;
            }

            v = _buffer[_pos++];

            return true;
        }

        virtual bool unget(const uint8_t &v)
        {
            if (NULL == _buffer)
            {
                return set_good(false);
            }

            if (_pos < 1)
            {
                return false;
            }

            _buffer[--_pos] = v;

            return true;
        }

        virtual bool seek(ssize_t offset, int whence)
        {
            if (NULL == _buffer)
            {
                return set_good(false);
            }

            switch (whence)
            {
                case SEEK_SET:
                {
                    if (offset < 0)
                    {
                        return false;
                    }

                    _pos = offset;
                }
                break;

                case SEEK_CUR:
                {
                    if ((offset < 0) && (abs(offset) > _pos))
                    {
                        return false;
                    }

                    if ((offset+_pos) > static_cast<ssize_t>(_size))
                    {
                        return false;
                    }

                    _pos += offset;
                }
                break;

                case SEEK_END:
                {
                    if ((offset < 0) || (offset > static_cast<ssize_t>(_size)))
                    {
                        return false;
                    }

                    _pos = _size - offset;
                }
                break;

                default:
                    return false;
            }

            return true;
        }

        virtual ssize_t tell()
        {
            if (NULL == _buffer)
            {
                return -1;
            }

            return _pos;
        }

    private:
        uint8_t *_buffer;
        size_t   _size;
        size_t   _capacity;
        ssize_t  _pos;
    };

    class BufferReaderWriter
        : BaseIO
        , public AbstractReader
        , public AbstractWriter
    {
    public:
        BufferReaderWriter(size_t reserve_=0)
            : BaseIO()
            , AbstractReader()
            , AbstractWriter()
            , _buffer  (NULL)
            , _size    (0)
            , _capacity(0)
            , _pos     (0)
        {
            alloc(reserve_);
        }

        ~BufferReaderWriter()
        {
            free(_buffer);

            _buffer = NULL;
        }

        BufferReaderWriter & operator=(const BufferReaderWriter &ref)
        {
            if (_capacity != ref._capacity)
            {
                free(_buffer);

                _buffer = NULL;
            }

            if (write(ref._buffer,ref._capacity))
            {
                _size = ref._size;
                _pos  = ref._pos ;
            }

            return *this;
        }

        const uint8_t *data    () const { return _buffer  ; }
        size_t         size    () const { return _size    ; }
        size_t         capacity() const { return _capacity; }

        inline bool alloc(const size_t &s)
        {
            if ((_size + s) > _capacity)
            {
                _capacity = _size + s;

                _buffer = reinterpret_cast<uint8_t*>
                (
                    realloc(_buffer,_capacity)
                );
            }

            return set_good( NULL != _buffer );
        }

        virtual void clear()
        {
            _size = 0;
            _pos  = 0;
        }

        virtual bool write(const void *data, size_t size)
        {
            if ((NULL == data) || (0 == size))
            {
                return true;
            }

            if (!alloc(size))
            {
                return set_good(false);
            }

            memcpy(&_buffer[_size], data, size);

            _size += size;

            return true;
        }

        virtual bool read(void *data, size_t size)
        {
            if (NULL == _buffer)
            {
                return set_good(false);
            }

            if ((_pos + size) > _size)
            {
                return false;
            }

            memcpy(data, &_buffer[_pos], size);

            _pos += size;

            return true;
        }

        virtual bool get(uint8_t &v)
        {
            if (NULL == _buffer)
            {
                return set_good(false);
            }

            if ((_pos + 1) > static_cast<ssize_t>(_size))
            {
                return false;
            }

            v = _buffer[_pos++];

            return true;
        }

        virtual bool unget(const uint8_t &v)
        {
            if (NULL == _buffer)
            {
                return set_good(false);
            }

            if (_pos < 1)
            {
                return false;
            }

            _buffer[--_pos] = v;

            return true;
        }

        virtual bool seek(ssize_t offset, int whence)
        {
            if (NULL == _buffer)
            {
                return set_good(false);
            }

            switch (whence)
            {
                case SEEK_SET:
                {
                    if (offset < 0)
                    {
                        return false;
                    }

                    _pos = offset;
                }
                break;

                case SEEK_CUR:
                {
                    if ((offset < 0) && (abs(offset) > _pos))
                    {
                        return false;
                    }

                    if ((offset+_pos) > static_cast<ssize_t>(_size))
                    {
                        return false;
                    }

                    _pos += offset;
                }
                break;

                case SEEK_END:
                {
                    if ((offset < 0) || (offset > static_cast<ssize_t>(_size)))
                    {
                        return false;
                    }

                    _pos = _size - offset;
                }
                break;

                default:
                    return false;
            }

            return true;
        }

        virtual ssize_t tell()
        {
            if (NULL == _buffer)
            {
                return -1;
            }

            return _pos;
        }

    private:
        uint8_t *_buffer;
        size_t   _size;
        size_t   _capacity;
        ssize_t  _pos;
    };

} // HOB

#endif // _HOB_IO_HPP__
