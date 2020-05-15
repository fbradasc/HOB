#ifndef NONSTD_THREAD_BARE_HPP
#define NONSTD_THREAD_BARE_HPP

// thread-bare configuration:

#define thread_THREAD_DEFAULT  0
#define thread_THREAD_NONSTD   1
#define thread_THREAD_STD      2

#if !defined( thread_CONFIG_SELECT_THREAD )
# define thread_CONFIG_SELECT_THREAD  ( thread_HAVE_STD_THREAD ? thread_THREAD_STD : thread_THREAD_NONSTD )
#endif

// Control presence of exception handling (try and auto discover):

#ifndef thread_CONFIG_NO_EXCEPTIONS
# if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
#  define thread_CONFIG_NO_EXCEPTIONS  0
# else
#  define thread_CONFIG_NO_EXCEPTIONS  1
# endif
#endif

// C++ language version detection (C++20 is speculative):
// Note: VC14.0/1900 (VS2015) lacks too much from C++14.

#ifndef   thread_CPLUSPLUS
# if defined(_MSVC_LANG ) && !defined(__clang__)
#  define thread_CPLUSPLUS  (_MSC_VER == 1900 ? 201103L : _MSVC_LANG )
# else
#  define thread_CPLUSPLUS  __cplusplus
# endif
#endif

#define thread_CPP98_OR_GREATER  ( thread_CPLUSPLUS >= 199711L )
#define thread_CPP11_OR_GREATER  ( thread_CPLUSPLUS >= 201103L )
#define thread_CPP14_OR_GREATER  ( thread_CPLUSPLUS >= 201402L )
#define thread_CPP17_OR_GREATER  ( thread_CPLUSPLUS >= 201703L )
#define thread_CPP20_OR_GREATER  ( thread_CPLUSPLUS >= 202000L )

// C++ language version (represent 98 as 3):

#define thread_CPLUSPLUS_V  ( thread_CPLUSPLUS / 100 - (thread_CPLUSPLUS > 200000 ? 2000 : 1994) )

// Use C++11 std::thread if available and requested:

#if thread_CPP11_OR_GREATER && defined(__has_include)
# if __has_include( <thread> )
#  define thread_HAVE_STD_THREAD  1
# else
#  define thread_HAVE_STD_THREAD  0
# endif
#else
# define  thread_HAVE_STD_THREAD  0
#endif

#define thread_USES_STD_THREAD  ( (thread_CONFIG_SELECT_THREAD == thread_THREAD_STD) || ((thread_CONFIG_SELECT_THREAD == thread_THREAD_DEFAULT) && thread_HAVE_STD_THREAD) )

#if defined(HAVE_STD_THREAD)

#include <thread>

#else // !HAVE_STD_THREAD

namespace std
{
    template <typename TArg0>
    class thread
    {
    public:
        typedef void (*function_t)(TArg0);

        thread(function_t func, TArg0 arg0)
            : m_func(func)
        {
            m_arg0 = new TThunkArg(this, arg0);

            pthread_attr_init(&m_attr);

            if (0 != pthread_create(&m_thread, &m_attr, thunk, m_arg0))
            {
                delete m_arg0;

                throw new std::runtime_error("Could not create a thread");
            }
        }

        void join()
        {
            pthread_join(m_thread, NULL);
        }

        void detach()
        {
            pthread_detach(m_thread);
        }

    private:
        struct TThunkArg
        {
            thread<TArg0> *Self;
            TArg0          Arg0;

            TThunkArg(thread<TArg0> *self, TArg0 arg0)
                : Self(self)
                , Arg0(arg0)
            {}
        };

        static void *thunk(void *unsafe_args)
        {
            TThunkArg *args = reinterpret_cast<TThunkArg*>(unsafe_args);

            try
            {
                args->Self->m_func(args->Arg0);
            }
            catch(...)
            {
                delete args;

                throw;
            }

            delete args;

            return 0;
        }

        pthread_t       m_thread;
        pthread_attr_t  m_attr;
        function_t      m_func;
        TThunkArg      *m_arg0;
    };
};

#endif // !HAVE_STD_THREAD

#endif // NONSTD_THREAD_BARE_HPP