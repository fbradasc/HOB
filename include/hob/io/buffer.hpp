#if !defined(__HOB_BUFFER_HPP__)
#define __HOB_BUFFER_HPP__

#include <stdlib.h>
#include "hob/io/common.hpp"
#include "hob/hob.hpp"

namespace hobio
{
    class obuffer
        : public common
        , public writer
    {
    public:
        obuffer(size_t reserve_=0)
            : common()
            , writer()
            , _buffer  (NULL)
            , _size    (0)
            , _capacity(0)
        {
            if (reserve_ > 0)
            {
                obuffer::alloc(reserve_);
            }
        }

        virtual ~obuffer()
        {
            free(_buffer);

            _buffer = NULL;
        }

        inline const uint8_t *data    () const { return _buffer  ; }
        inline size_t         size    () const { return _size    ; }
        inline size_t         capacity() const { return _capacity; }

        inline bool alloc(const size_t &s)
        {
            if ( s == 0 )
            {
                return true;
            }

            if ((_size + s) > _capacity)
            {
                size_t new_capacity = _size + s;

                _buffer = reinterpret_cast<uint8_t*> (
                    realloc(_buffer,new_capacity)
                );

                if (NULL != _buffer)
                {
                    _capacity = new_capacity;
                }
            }

            return set_good( NULL != _buffer );
        }

        inline virtual void clear()
        {
            _size = 0;
        }

        inline virtual bool write(const void *data_, size_t size_)
        {
            if ((NULL == data_) || (0 == size_))
            {
                return true;
            }

            if (!alloc(size_))
            {
                return set_good( false );
            }

            switch (size_)
            {
            case 1:
                *reinterpret_cast<uint8_t*>(&_buffer[_size]) = *reinterpret_cast<const uint8_t *>(data_);
                break;
            case 2:
                *reinterpret_cast<uint16_t*>(&_buffer[_size]) = *reinterpret_cast<const uint16_t *>(data_);
                break;
            case 4:
                *reinterpret_cast<uint32_t*>(&_buffer[_size]) = *reinterpret_cast<const uint32_t *>(data_);
                break;
            case 8:
                *reinterpret_cast<uint64_t*>(&_buffer[_size]) = *reinterpret_cast<const uint64_t *>(data_);
                break;
            case 16:
                *reinterpret_cast<long double*>(&_buffer[_size]) = *reinterpret_cast<const long double *>(data_);
                break;
            default:
                memcpy(&_buffer[_size], data_, size_);
                break;
            }

            _size += size_;

            return true;
        }

    private:
        uint8_t *_buffer;
        size_t   _size;
        size_t   _capacity;
    };

    class ibuffer
        : public common
        , public reader
    {
    public:
        ibuffer()
            : common()
            , reader()
            , _buffer       (NULL)
            , _size         (0)
            , _capacity     (0)
            , _pos          (0)
            , _reader       (NULL)
        {
        }

        ibuffer(const ibuffer &ref)
            : common()
            , reader()
            , _buffer       (NULL)
            , _size         (0)
            , _capacity     (0)
            , _pos          (0)
            , _reader       (NULL)
        {
            *this = ref;
        }

        ibuffer(const obuffer &ref)
            : common()
            , reader()
            , _buffer       (NULL)
            , _size         (0)
            , _capacity     (0)
            , _pos          (0)
            , _reader       (NULL)
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

        ibuffer(hobio::reader &ref, size_t size_)
            : common()
            , reader()
            , _buffer       (NULL)
            , _size         (0)
            , _capacity     (0)
            , _pos          (0)
            , _reader       (&ref)
        {
            ibuffer::load(size_);
        }

        virtual ~ibuffer()
        {
            free(_buffer);

            _buffer = NULL;
        }

        ibuffer & operator=(const ibuffer &ref)
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

            _reader = ref._reader;

            set_good( NULL != _buffer );

            return *this;
        }

        inline const uint8_t *data    () const { return _buffer  ; }
        inline size_t         size    () const { return _size    ; }
        inline size_t         capacity() const { return _capacity; }

        inline bool alloc(const size_t &s)
        {
            if ( s == 0 )
            {
                return true;
            }

            size_t new_capacity = _size + s;

            if (new_capacity > _capacity)
            {
                _buffer = reinterpret_cast<uint8_t*>
                (
                    realloc(_buffer,new_capacity)
                );

                if (NULL != _buffer)
                {
                    _capacity = new_capacity;
                }
            }

            return set_good( NULL != _buffer );
        }

        inline virtual void clear()
        {
            _size -= _pos;
            memcpy(_buffer,&_buffer[_pos],_size);
            _pos  = 0;
        }

        inline virtual bool load(size_t size_)
        {
            if (size_ > (_size-_pos))
            {
                size_ -= (_size-_pos);

                if ((NULL == _reader)
                    ||
                    ((size_ > 0) && !alloc(size_))
                    ||
                    !_reader->read(&_buffer[_size], size_))
                {
                    return set_good(false);
                }

                _size += size_;
            }

            return true;
        }

        inline virtual bool read(void *data_, size_t size_)
        {
            if (!good() || (NULL == _buffer))
            {
                return set_good(false);
            }

            if ((NULL == data_) || (size_ < 1))
            {
                return false;
            }

            if (!load(size_))
            {
                return false;
            }

            memcpy(data_, &_buffer[_pos], size_);

            _pos += size_;

            return true;
        }

        inline virtual bool get(uint8_t &v)
        {
            if (!good() || (NULL == _buffer))
            {
                return set_good(false);
            }

            if (!load(1))
            {
                return false;
            }

            v = _buffer[_pos++];

            return true;
        }

        inline virtual bool unget(const uint8_t &v)
        {
            if (!good() || (NULL == _buffer))
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

        inline virtual bool seek(ssize_t offset, int whence)
        {
            if (!good() || (NULL == _buffer))
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

        inline virtual ssize_t tell()
        {
            if (!good() || (NULL == _buffer))
            {
                set_good(false);

                return -1;
            }

            return _pos;
        }

    private:
        uint8_t       *_buffer;
        size_t         _size;
        size_t         _capacity;
        ssize_t        _pos;
        hobio::reader *_reader;
    };

    class iobuffer
        : public common
        , public reader
        , public writer
    {
    public:
        iobuffer(size_t reserve_=0)
            : common()
            , reader()
            , writer()
            , _buffer  (NULL)
            , _size    (0)
            , _capacity(0)
            , _pos     (0)
            , _reader  (NULL)
        {
            if (reserve_ > 0)
            {
                iobuffer::alloc(reserve_);
            }
        }

        iobuffer(hobio::reader &ref, size_t size_)
            : common()
            , reader()
            , writer()
            , _buffer       (NULL)
            , _size         (0)
            , _capacity     (0)
            , _pos          (0)
            , _reader       (&ref)
        {
            iobuffer::load(size_);
        }

        virtual ~iobuffer()
        {
            free(_buffer);

            _buffer = NULL;
        }

        iobuffer & operator=(const iobuffer &ref)
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

            _reader = ref._reader;

            return *this;
        }

        inline const uint8_t *data    () const { return _buffer  ; }
        inline size_t         size    () const { return _size    ; }
        inline size_t         capacity() const { return _capacity; }

        inline bool alloc(const size_t &s)
        {
            if ( s == 0 )
            {
                return true;
            }

            if ((_size + s) > _capacity)
            {
                size_t new_capacity = _size + s;

                _buffer = reinterpret_cast<uint8_t*> (
                    realloc(_buffer,new_capacity)
                );

                if (NULL != _buffer)
                {
                    _capacity = new_capacity;
                }
            }

            return set_good( NULL != _buffer );
        }

        inline virtual void clear()
        {
            // _size = 0;
            _size -= _pos;
            memcpy(_buffer,&_buffer[_pos],_size);
            _pos  = 0;
        }

        inline virtual bool write(const void *data_, size_t size_)
        {
            if ((NULL == data_) || (0 == size_))
            {
                return true;
            }

            if (!alloc(size_))
            {
                return set_good(false);
            }

            switch (size_)
            {
            case 1:
                *reinterpret_cast<uint8_t*>(&_buffer[_size]) = *reinterpret_cast<const uint8_t *>(data_);
                break;
            case 2:
                *reinterpret_cast<uint16_t*>(&_buffer[_size]) = *reinterpret_cast<const uint16_t *>(data_);
                break;
            case 4:
                *reinterpret_cast<uint32_t*>(&_buffer[_size]) = *reinterpret_cast<const uint32_t *>(data_);
                break;
            case 8:
                *reinterpret_cast<uint64_t*>(&_buffer[_size]) = *reinterpret_cast<const uint64_t *>(data_);
                break;
            case 16:
                *reinterpret_cast<long double*>(&_buffer[_size]) = *reinterpret_cast<const long double *>(data_);
                break;
            default:
                memcpy(&_buffer[_size], data_, size_);
                break;
            }

            _size += size_;

            return true;
        }

        inline virtual bool read(void *data_, size_t size_)
        {
            if (!good() || (NULL == _buffer))
            {
                return set_good(false);
            }

            if ((NULL == data_) || (size_ < 1))
            {
                return false;
            }

            if (!load(size_))
            {
                return false;
            }

            memcpy(data_, &_buffer[_pos], size_);

            _pos += size_;

            return true;
        }

        inline virtual bool get(uint8_t &v)
        {
            if (!good() || (NULL == _buffer))
            {
                return set_good(false);
            }

            if (!load(1))
            {
                return false;
            }

            v = _buffer[_pos++];

            return true;
        }

        inline virtual bool unget(const uint8_t &v)
        {
            if (!good() || (NULL == _buffer))
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

        inline virtual bool seek(ssize_t offset, int whence)
        {
            if (!good() || (NULL == _buffer))
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

        inline virtual ssize_t tell()
        {
            if (!good() || (NULL == _buffer))
            {
                set_good(false);

                return -1;
            }

            return _pos;
        }

        inline virtual bool load(size_t size_)
        {
            if (size_ > (_size-_pos))
            {
                size_ -= (_size-_pos);

                if ((NULL == _reader)
                    ||
                    ((size_ > 0) && !iobuffer::alloc(size_))
                    ||
                    !_reader->read(&_buffer[_size], size_))
                {
                    return set_good(false);
                }

                _size += size_;
            }

            return true;
        }

    private:
        uint8_t       *_buffer;
        size_t         _size;
        size_t         _capacity;
        ssize_t        _pos;
        hobio::reader *_reader;
    };
} // namespace hobio


#endif // _HOB_BUFFER_HPP__
