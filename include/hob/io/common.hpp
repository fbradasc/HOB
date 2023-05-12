#if !defined(__HOB_COMMON_HPP__)
#define __HOB_COMMON_HPP__

#include <unistd.h>
#include <iostream>
#include <cstdio>

#include <stdio.h>
#include <cstring>
#include <climits>

#include <inttypes.h>
#include <bitset>
#include <string>
#include <vector>
#include <map>
#include "hob/std/optional.hpp"

#if defined(HOB_DEBUG)
#define M_LOG(...)            printf(__VA_ARGS__); \
                              printf(" @ %s:%s:%d", __FUNCTION__, __FILE__, __LINE__); \
                              printf("\n");
#else
#define M_LOG(...)
#endif

#define HSEED                 65599llu
// #define HSEED                 11400714819323198485llu

using namespace std;
using namespace nonstd;

namespace hobio
{
    class encoder;
    class decoder;

    typedef uint64_t uid_t;

    static const uid_t UNDEFINED = static_cast<uid_t>(-1); // ULLONG_MAX;

    class UID
    {
    public:
        UID(): _id(hobio::UNDEFINED), _np(-1) {}

        UID(const hobio::uid_t & in): _id(0), _np(-1) { update(in); }
        UID(const char         * in): _id(0), _np(-1) { update(in); }
        UID(const string       & in): _id(0), _np(-1) { update(in); }

        UID(const UID & ref) { *this = ref; }

        inline operator const hobio::uid_t &() const { return _id; }

        static inline bool has_static_fields(hobio::uid_t id)
        {
            return ( ( id & SF_MASK ) != 0 );
        }

        static inline bool has_dynamic_fields(hobio::uid_t id)
        {
            return ( ( id & DF_MASK ) != 0 );
        }

        static inline bool has_payload(hobio::uid_t id)
        {
            return ( ( id & ( SF_MASK | DF_MASK ) ) != 0 );
        }

        inline bool has_payload() const
        {
            return ( ( _id & SF_MASK ) != 0 );
        }

        inline bool has_dynamic_fields()
        {
            return has_dynamic_fields(_id);
        }

        inline hobio::uid_t with_dynamic_fields(bool has_dynamic_fields) const
        {
            (void)has_dynamic_fields;

            return ( _id | ( DF_MASK * has_dynamic_fields ) );
        }

        inline UID & operator=(const uid_t & ref)
        {
#if 0
            // preserve static fields flag bit, discard dynamic fields flag bit
            //
            _id = ref & ( ID_MASK | SF_MASK );
#else
            _id = ref;
#endif

            _np = -1;

            return *this;
        }

        inline UID & operator=(const UID & ref)
        {
            _id = ref._id;
            _np = ref._np;

            return *this;
        }

        inline bool operator==(const UID & ref) const
        {
            return _id == ref._id;
        }

        inline bool operator!=(const UID & ref) const
        {
            return _id != ref._id;
        }

        inline bool operator<(const UID & ref) const
        {
            return (_id < ref._id);
        }
/*
        inline UID & operator<<=(const uid_t & ref)
        {
            _id <<= ref;

            return *this;
        }

        inline UID & operator|=(const uid_t & ref)
        {
            _id |= ref;

            return *this;
        }
*/
        inline void update(const hobio::uid_t & in)
        {
            _np++;

            _id = ( in << ID_POS );
        }

        inline void update(const string & in)
        {
            update(in.c_str());
        }

        void update(const char * in)
        {
            //
            // in != NULL -> update ID calculation
            //
            if (NULL != in)
            {
                //
                // count hob parameters
                //
                _np++;

                _id >>= ID_POS;

                for (size_t i = 0; in[i]; ++i)
                {
                    _id = ( HSEED * _id + in[i] );
                }

                _id <<= ID_POS;
            }
            else
            //
            // in == NULL -> finalize ID calculation
            //
            if (_np >= 0)
            {
                if (_np > 0)
                {
                    _id |= SF_MASK;
                }

                // Avoid parameters check multiple appliance
                //
                _np = -1;
            }
        }

    private:
        //======================================================//
        //                                                      //
        //             hobio::uid_t _id [64 bits]               //
        //                                                      //
        // +--------+---+---+                                   //
        // | 63...2 | 1 | 0 |                                   //
        // +--------+---+---+                                   //
        //        ^   ^   ^                                     //
        //        |   |   |                                     //
        //        |   |   `---  1 bit : Has static  fields flag //
        //        |   `-------  1 bit : Has dynamic fields flag //
        //        `----------- 62 bits: HOB hash code           //
        //                                                      //
        //======================================================//

        enum FieldsPos
        {
            SF_POS = 0, // static fields flag's bit position
            DF_POS    , // dynamic fields flag's bit position
            ID_POS      // HOB hash code's first bit position
        };

        enum FieldsMasks
        {
            SF_MASK = ( static_cast<uid_t>( 1) << SF_POS ),
            DF_MASK = ( static_cast<uid_t>( 1) << DF_POS ),
            ID_MASK = ( static_cast<uid_t>(-1) << ID_POS )
        };

        hobio::uid_t _id;
        ssize_t      _np;

        static const uid_t C_SF_MASK = SF_MASK;
        static const uid_t C_DF_MASK = DF_MASK;
        static const uid_t C_ID_MASK = ID_MASK;
    };

    class codec
    {
    public:
        virtual size_t size  (hobio::encoder &e) const = 0;
        virtual bool   encode(hobio::encoder &e) const = 0;
        virtual bool   decode(hobio::decoder &d, bool *changed = NULL) = 0;
    };

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
