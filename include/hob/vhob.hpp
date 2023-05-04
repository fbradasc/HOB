#if !defined(__VHOB_HPP__)
#define __VHOB_HPP__
#include "hob/hob.hpp"
#if defined(EMBED_VARIANT)
#include "hob/std/type_traits.hpp"
#include <typeinfo>
#define variant_t variant
#else // EMBED_VARIANT
#include "hob/types/variant.hpp"
#define variant_t hobio::variant
#endif // EMBED_VARIANT

class vhob : public hob
{
public:
    ///////////////////////////////////////////////////////////////////////////
    //                                                                       //
    //                              HOB interface                            //
    //                                                                       //
    ///////////////////////////////////////////////////////////////////////////

#if defined(EMBED_VARIANT)
    vhob(): hob() { }
#endif // EMBED_VARIANT

    vhob(const hobio::UID &id_): hob(id_) { }
    vhob(const char       *id_): hob(id_) { }
    vhob(const string     &id_): hob(id_) { }
    vhob(const hob        &ref): hob(ref) { *this = ref; }
    vhob(const vhob       &ref): hob(ref) { *this = ref; }

    virtual ~vhob()
    {
    }

    virtual vhob * clone() const
    {
        return new vhob(*this);
    }

    vhob & operator=(const hob & ref)
    {
        *static_cast<hob *>(this) = static_cast<const hob &>(ref);

        return *this;
    }

    vhob & operator=(const vhob & ref)
    {
        *static_cast<hob *>(this) = static_cast<const hob &>(ref);

        _df = ref._df;

        return *this;
    }

    bool operator==(const vhob &ref) const
    {
        return ((*static_cast<const hob*>(this) == static_cast<const hob &>(ref))
                 &&
                 (_df == ref._df));
    }

    bool operator==(const hob &ref) const
    {
        return !__ne(ref.__get_id());
    }

    virtual bool operator!=(const hob &ref) const
    {
        return __ne(ref.__get_id());
    }

    inline bool operator<(const vhob &ref) const
    {
        return __lt(ref.__get_id());
    }

    virtual bool operator>>(hobio::encoder &os) const
    {
        return
        (
            (hobio::UNDEFINED == __get_id())
            ||
            (
                os.encode_header(static_cast<const char *>(NULL),
                                 static_cast<const char *>(NULL),
                                 ( __get_id() & ~1 ) | __has_payload(),
                                 __payload(os))
                &&
                ( !__has_payload() || os.encode_field(_df, "variant") )
                &&
                os.encode_footer()
            )
        );
    }

    inline bool operator<(const hobio::UID &id) const
    {
        return __lt(id);
    }

    inline bool operator!=(const hobio::UID &id) const
    {
        return __ne(id);
    }

    inline operator bool()
    {
        return true;
    }

    inline void operator~()
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    //                                                                       //
    //                     Dynamic fields interface                          //
    //                                                                       //
    ///////////////////////////////////////////////////////////////////////////

#if defined(EMBED_VARIANT)
#include "hob/types/variant.hpp"
#endif // EMBED_VARIANT

    typedef vector<variant_t> dynamic_fields_t;

    typedef dynamic_fields_t::iterator               iterator;
    typedef dynamic_fields_t::const_iterator         const_iterator;
    typedef dynamic_fields_t::reverse_iterator       reverse_iterator;
    typedef dynamic_fields_t::const_reverse_iterator const_reverse_iterator;
    typedef dynamic_fields_t::size_type              size_type;

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

    inline iterator find(const hobio::UID &id)
    {
        iterator it = begin();

        for (; (it != end()) && ((*it) != id); ++it);

        return it;
    }

    inline iterator find(const string &n)
    {
        iterator it = begin();

        for (; (it != end()) && ((*it) != n); ++it);

        return it;
    }

    inline const_iterator find(const hobio::UID &id) const
    {
        const_iterator cit = begin();

        for (; (cit != end()) && ((*cit) != id); ++cit);

        return cit;
    }

    inline const_iterator find(const string &n) const
    {
        const_iterator cit = begin();

        for (; (cit != end()) && ((*cit) != n); ++cit);

        return cit;
    }

    template <typename T>
    inline iterator find(const hobio::UID &id)
    {
        iterator it = find(id);

        return ((it != end()) && (static_cast<T *>(*it) != NULL)) ? it : end();
    }

    template <typename T>
    inline iterator find(const string &n)
    {
        iterator it = find(n);

        return ((it != end()) && (static_cast<T *>(*it) != NULL)) ? it : end();
    }

    template <typename T>
    inline const_iterator find(const hobio::UID &id) const
    {
        const_iterator cit = find(id);

        return ((cit != end()) && (static_cast<T *>(*cit) != NULL)) ? cit : end();
    }

    template <typename T>
    inline const_iterator find(const string &n) const
    {
        const_iterator cit = find(n);

        return ((cit != end()) && (static_cast<T *>(*cit) != NULL)) ? cit : end();
    }

    inline void clear(                             ) { _df.clear(          ); }
    inline void erase(const char   *n              ) { _df.erase(find( n)  ); }
    inline void erase(const hobio::UID &id         ) { _df.erase(find(id)  ); }
    inline void erase(const string &n              ) { _df.erase(find( n)  ); }
    inline void erase(iterator pos                 ) { _df.erase(pos       ); }
    inline void erase(iterator first, iterator last) { _df.erase(first,last); }

    inline variant_t& operator[](const hobio::UID &id)
    {
        return get_or_create(id);
    }

    inline variant_t& operator[](const char *n)
    {
        return get_or_create(n);
    }

    inline variant_t& operator[](const string &n)
    {
        return get_or_create(n);
    }

    inline variant_t& get_or_create(const hobio::UID id)
    {
        iterator it = find(id);

        if (it == end())
        {
            variant_t v(id);

            it = _df.insert(end(),v);
        }

        return *it;
    }

    inline variant_t& get_or_create(const char *n)
    {
        return get_or_create(variant_t::id(n));
    }

    inline variant_t& get_or_create(const string &n)
    {
        return get_or_create(variant_t::id(n));
    }

    template<typename T>
    inline vhob & set(const string &n, const T &v)
    {
        variant_t & v_ = get_or_create(n);

        v_ = v;

        return *this;
    }

    template<typename T>
    inline vhob & set(const hobio::UID &id, const T &v)
    {
        variant_t & v_ = get_or_create(id);

        v_ = v;

        return *this;
    }

    template<typename T>
    inline vhob & set(const char *n, const T &v)
    {
        variant_t & v_ = get_or_create(n);

        v_ = v;

        return *this;
    }

    inline bool has(const string     & n ) const { return (find( n) != end()); }
    inline bool has(const hobio::UID & id) const { return (find(id) != end()); }
    inline bool has(const char       * n ) const { return (find( n) != end()); }

    template<typename T>
    inline bool has(const string     & n ) const { return (find<T>( n) != end()); }

    template<typename T>
    inline bool has(const hobio::UID & id) const { return (find<T>(id) != end()); }

    template<typename T>
    inline bool has(const char       * n ) const { return (find<T>( n) != end()); }

    template<typename T>
    inline const T * get(const string & n) const
    {
        const const_iterator cit = find(n);

        if (cit == end())
        {
            return NULL;
        }

        return (*cit);
    }

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
    inline const T * get(const char * n ) const
    {
        const const_iterator cit = find(n);

        if (cit == end())
        {
            return NULL;
        }

        return (*cit);
    }

    template<typename T>
    inline bool get(const string & n, T & v) const
    {
        const const_iterator cit = find(n);

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

    template<typename T>
    inline bool get(const char * n, T & v) const
    {
        const const_iterator cit = find(n);

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

protected:
    virtual bool __decode(hob &ref)
    {
        hobio::decoder *is = static_cast<hobio::decoder*>(ref);

        if (NULL == is)
        {
            return false;
        }

        ref.__rewind();

        if (!__decode(*is))
        {
            return false;
        }

        *this = ref;

        return true;
    }

    virtual bool __decode(hobio::decoder &is, bool update=true)
    {
        (void)is;
        (void)update;

        return is.decode_field(_df);
    }

    virtual size_t __payload(hobio::encoder &os) const
    {
        (void)os;

        return (__has_payload()) ? os.field_size(_df) : 0;
    }

    inline bool __has_payload() const
    {
        return !_df.empty();
    }

    virtual void __reset_changes()
    {
        return;
    }

    virtual bool __is_changed() const
    {
        return false;
    }

    virtual bool __set_changed(ssize_t f, bool v)
    {
        (void)f;
        (void)v;

        return true;
    }

    virtual void __flush_pending()
    {
    }

private:
    dynamic_fields_t _df;

    inline bool __lt(const hobio::UID &id) const
    {
        return ( ( __get_id() & ~1 ) < ( id & ~1 ) );
    }

    inline bool __ne(const hobio::UID &id) const
    {
        return ( ( ( __get_id() ^ id ) & ~1 ) != 0 );
    }
};

// inline bool operator<<(hobio::encoder &e, vhob &h) { M_LOG("-"); return h >> e; }
// inline bool operator>>(hobio::decoder &d, vhob &h) { M_LOG("-"); return h << d; }
// inline bool operator>>(vhob           &f, vhob &t) { M_LOG("-"); return t << f; }
// inline bool operator>>(hob            &f, vhob &t) { M_LOG("-"); return t << f; }

#endif // __VHOB_HPP__
