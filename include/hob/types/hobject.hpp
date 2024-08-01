#if !defined(__HOB_TYPES_HOBJECT__)
#define __HOB_TYPES_HOBJECT__

#include "hob/io/common.hpp"
#include "hob/io/encoder.hpp"
#include "hob/io/decoder.hpp"

// ----> dynamic fields implementation <----
//
#include "hob/types/variant.hpp"
//
// ----> dynamic fields implementation <----

namespace hobio
{
    class hobject
        : public codec
    {
    public:
        // ----> dynamic fields implementation <----
        //
        typedef hobio::fields::iterator               iterator;
        typedef hobio::fields::const_iterator         const_iterator;
        typedef hobio::fields::reverse_iterator       reverse_iterator;
        typedef hobio::fields::const_reverse_iterator const_reverse_iterator;
        typedef hobio::fields::size_type              size_type;

        inline iterator               begin   ()       { return _df.begin   (); }
        inline const_iterator         begin   () const { return _df.begin   (); }
        inline iterator               end     ()       { return _df.end     (); }
        inline const_iterator         end     () const { return _df.end     (); }
        inline reverse_iterator       rbegin  ()       { return _df.rbegin  (); }
        inline const_reverse_iterator rbegin  () const { return _df.rbegin  (); }
        inline reverse_iterator       rend    ()       { return _df.rend    (); }
        inline const_reverse_iterator rend    () const { return _df.rend    (); }
        inline bool                   empty   () const { return _df.empty   (); }
        inline size_type              count   () const { return _df.size    (); }
        inline size_type              max_size() const { return _df.max_size(); }

        inline iterator find(const hobio::UID & id)
        {
            iterator it = begin();

            for (; (it != end()) && ((*it) != id); ++it);

            return it;
        }

        inline const_iterator find(const hobio::UID & id) const
        {
            const_iterator cit = begin();

            for (; (cit != end()) && ((*cit) != id); ++cit);

            return cit;
        }

        template <typename T>
        inline iterator find(const hobio::UID & id)
        {
            iterator it = find(id);

            return ((it != end()) && (static_cast<T *>(*it) != NULL)) ? it : end();
        }

        template <typename T>
        inline const_iterator find(const hobio::UID & id) const
        {
            const_iterator cit = find(id);

            return ((cit != end()) && (static_cast<T *>(*cit) != NULL)) ? cit : end();
        }

        inline void clear(                             ) { _df.clear(          ); }
        inline void erase(const hobio::UID & id        ) { _df.erase(find(id)  ); }
        inline void erase(iterator pos                 ) { _df.erase(pos       ); }
        inline void erase(iterator first, iterator last) { _df.erase(first,last); }

        inline hobio::variant& operator[](const hobio::UID & id)
        {
            return get_or_create(id);
        }

        // avoid ambiguous overload to built-in operator[](long int, const char*)
        //
        operator hobio::fields &()
        {
            return _df;
        }

        inline hobio::variant& operator[](const char *n)
        {
            return get_or_create(n);
        }

        inline hobio::variant& get_or_create(const hobio::UID & id)
        {
            iterator it = find(id);

            if (it == end())
            {
                hobio::variant v(id);

                it = _df.insert(end(),v);
            }

            return *it;
        }

        template<typename T>
        inline hobject & set(const hobio::UID & id, const T &v)
        {
            hobio::variant & v_ = get_or_create(id);

            v_ = v;

            return *this;
        }

        inline bool has(const hobio::UID & id) const { return (find(id) != end()); }

        template<typename T>
        inline bool has(const hobio::UID & id) const { return (find<T>(id) != end()); }

        template<typename T>
        inline const T * get(const hobio::UID & id) const
        {
            const const_iterator cit = find(id);

            if (cit == end())
            {
                return NULL;
            }

            return (*cit);
        }

        template<typename T>
        inline bool get(const hobio::UID & id, T & v) const
        {
            const const_iterator cit = find(id);

            if (cit == end())
            {
                return false;
            }

            const T *rv = (*cit);

            if (NULL == rv)
            {
                return false;
            }

            v = *rv;

            return true;
        }

        virtual bool operator>>(hobio::encoder &os) const
        {
            return
            (
                (hobio::UNDEFINED == _id)
                ||
                (
                    os.encode_header(static_cast<const char *>(NULL),
                                     static_cast<const char *>(NULL),
                                     _id.with_dynamic_fields(!empty()),
                                     __get_payload_size(os))
                    &&
                    __encode_dynamic_fields(os)
                    &&
                    os.encode_footer()
                )
            );
        }
        //
        // ----> dynamic fields implementation <----

        hobject()
            : _is( NULL)
            , _sp(    0)
            , _ep(    0)
        {
        }

        hobject(const hobio::UID &id_)
            : _id(  id_)
            , _is( NULL)
            , _sp(    0)
            , _ep(    0)
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
            _df = ref._df;

            return *this;
        }

        inline bool operator<<(hobio::decoder *is)
        {
            M_LOG("{");

            if (NULL == is)
            {
                M_LOG("} - false");

                return false;
            }

            bool retval = *this << *is;

            M_LOG("} - %s", retval ? "true" : "false");

            return retval;
        }

        inline bool operator<<(hobio::decoder &is)
        {
            M_LOG("{");

            __flush_pending();

            bool retval = __decode(is, !__decoding());

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
            hobject w;

            M_LOG("{");

            if (w.__decode(d,false))
            {
                *this = w;

                M_LOG("Decoded hobject: %lu", static_cast<uid_t>(_id));

                if (*this << static_cast<hobio::decoder*>(w))
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
            M_LOG("{");

            if (__decoding())
            {
                M_LOG("}");

                return;
            }

            if (NULL == _is)
            {
                M_LOG("}");

                return;
            }

            ssize_t cp = _is->tell();

            if ((cp >= 0) && (cp < _ep))
            {
                _is->skip(_ep - cp);
            }

            M_LOG("}");
        }

        inline bool __rewind()
        {
            M_LOG("{");

            if (NULL == _is)
            {
                M_LOG("} - false");

                return false;
            }

            if (!_is->seek(_sp,SEEK_SET))
            {
                M_LOG("} - false");

                return false;
            }

            M_LOG("} - true");

            return true;
        }

        bool __has_dynamic_fields()
        {
            return _id.has_dynamic_fields();
        }

    protected:
        hobio::UID    _id;
        hobio::fields _df;


        virtual bool __decode(hobio::hobject &ref)
        {
            (void)ref;

            M_LOG("{");

            ref.__rewind();

            if (!__decode_dynamic_fields(ref))
            {
                M_LOG("} - false");

                return false;
            }

            M_LOG("} - true");

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

            M_LOG("} - true");

            return true;
        }

        bool __decoding(bool set = false, bool decoding = false)
        {
            static size_t _dh = 0;

            if (set)
            {
                if (decoding)
                {
                    _dh++;
                }
                else
                if (0 < _dh)
                {
                    _dh--;
                }
            }

            return ( _dh > 0 );
        }

        // ----> dynamic fields implementation <----
        //
        size_t __get_dynamic_fields_size(hobio::encoder &os) const
        {
            (void)os;

            return (empty()) ? 0 : os.field_size(_df);
        }

        inline bool __encode_dynamic_fields(hobio::encoder &os) const
        {
            (void)os;

            return ( empty() || os.encode_field(_df, "variant") );
        }

        inline bool __decode_dynamic_fields(hobio::hobject &ref)
        {
            M_LOG("{");

            if (!ref.__has_dynamic_fields())
            {
                M_LOG("} - true");

                return true;
            }

            hobio::decoder *is = static_cast<hobio::decoder*>(ref);

            if (NULL == is)
            {
                M_LOG("} - false");

                return false;
            }

            __decoding(true, true);

            bool retval = is->decode_field(_df);

            __decoding(true, false);

            M_LOG("} - %s", retval ? "true" : "false");

            return retval;
        }
        //
        // ----> dynamic fields implementation <----

        virtual size_t __get_static_fields_size(hobio::encoder &os) const
        {
            (void)os;

            return 0;
        }

        size_t __get_payload_size(hobio::encoder &os) const
        {
            (void)os;

            return __get_static_fields_size(os) + __get_dynamic_fields_size(os);
        }

    private:
        hobio::decoder *_is;
        ssize_t         _sp;
        ssize_t         _ep;
    };
};

#endif // __HOB_TYPES_HOBJECT__
