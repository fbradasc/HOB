#if !defined(__HOB_TYPES_HOBJECT__)
#define __HOB_TYPES_HOBJECT__

#include "hob/io/common.hpp"
#include "hob/io/encoder.hpp"
#include "hob/io/decoder.hpp"

namespace hobio
{
    class hobject
        : public codec
    {
    public:
        hobject()
            : _is(NULL)
            , _sp(   0)
            , _ep(   0)
        { }

        hobject(const hobio::UID &id_)
            : _id( id_)
            , _is(NULL)
            , _sp(   0)
            , _ep(   0)
        {
        }

        hobject(const hobject &ref)
        {
            *this = ref;
        }

        hobject & operator=(const hobject & ref)
        {
            _id = ref._id;
            _is = ref._is;
            _sp = ref._sp;
            _ep = ref._ep;

            return *this;
        }

        inline bool operator<<(hobio::decoder *is)
        {
            if (NULL == is)
            {
                return false;
            }

            return *this << *is;
        }

        inline bool operator<<(hobio::decoder &is)
        {
            __flush_pending();

            M_LOG("{");

            bool retval = __decode(is);

            M_LOG("} - %s", retval ? "true" : "false");

            return retval;
        }

        inline bool operator<<(hobject & ref)
        {
            M_LOG("{");

            bool retval = ((_id == ref._id)
                           &&
                           __decode(ref));

            M_LOG("} - %s", retval ? "true" : "false");

            return retval;
        }

        inline /**/ bool operator==(const hobject &ref) const
        {
            return (_id == ref._id);
        }

        inline /*virtual*/ bool operator!=(const hobject &ref) const
        {
            return (_id != ref._id);
        }

        inline bool operator!=(const hobio::UID &id) const
        {
            return (_id != id);
        }

        inline bool operator<(const hobio::UID &id) const
        {
            return (_id < id);
        }

        inline bool operator<(const hobject &ref) const
        {
            return (_id < ref._id);
        }

        inline operator hobio::decoder *()
        {
            return _is;
        }

        virtual hobject * clone() const
        {
            return new hobject(*this);
        }

        virtual size_t size(hobio::encoder &os) const
        {
            (void)os;

            return 0;
        }

        virtual bool encode(hobio::encoder &os) const
        {
            (void)os;

            return false;
        }

        virtual bool decode(hobio::decoder &d, bool *changed = NULL)
        {
            hobject h;

            M_LOG("{");

            if (h.__decode(d,false))
            {
                *this = h;

                M_LOG("Decoded hob: %lu", _id);

                if (*this << static_cast<hobio::decoder*>(h))
                {
                    if (changed)
                    {
                        *changed = static_cast<bool>(*this);
                    }

                    M_LOG("} - true");

                    return true;
                }
            }

            M_LOG("} - false");

            return false;
        }

        virtual void __flush_pending()
        {
            if (NULL == _is)
            {
                return;
            }

            ssize_t cp = _is->tell();

            if ((cp >= 0) && (cp < _ep))
            {
                _is->skip(_ep - cp);
            }
        }

        inline bool __rewind()
        {
            return (NULL != _is) && _is->seek(_sp,SEEK_SET);
        }

        bool __has_dynamic_fields()
        {
            return _id.has_dynamic_fields();
        }

    protected:
        hobio::UID _id;

        virtual bool __decode(hobject &ref)
        {
            (void)ref;

            ref.__rewind();

            return true;
        }

        virtual bool __decode(hobio::decoder &is, bool update=true)
        {
            hobio::uid_t id_;

            _is = NULL;
            _sp = 0;
            _ep = 0;

            M_LOG("{");

            if (!is.load(update))
            {
                M_LOG("} - false");

                return false;
            }

            if (!is.decode_field(id_))
            {
                M_LOG("} - false");

                return false;
            }

            size_t sz_ = 0;

            if (hobio::UID::has_payload(id_) && !is.decode_field(sz_))
            {
                M_LOG("} - false");

                return false;
            }

            if (update && !is.bufferize(sz_))
            {
                M_LOG("} - false");

                return false;
            }

            _is = &is;
            _sp = is.tell();
            _ep = _sp + sz_;
            _id = id_;

            return true;
        }

    private:
        hobio::decoder *_is;
        ssize_t         _sp;
        ssize_t         _ep;
    };
};

#endif // __HOB_TYPES_HOBJECT__
