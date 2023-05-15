#if !defined(__DYNAMHOB__)
#define __DYNAMHOB__

#include "hob/types/variant.hpp"

class dynamhob
    : public hobio::hobject
{
public:
    typedef vector<hobio::variant> dynamic_fields_t;

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
    inline dynamhob & set(const hobio::UID & id, const T &v)
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

protected:
    dynamic_fields_t _df;

    dynamhob(): hobio::hobject()
    {
    }

    dynamhob(const hobio::UID &id_): hobio::hobject(id_)
    {
    }
};

#endif // __DYNAMHOB__


