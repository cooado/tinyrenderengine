#ifndef BOOST_THREAD_CONDITION_VARIABLE_PTHREAD_HPP
#define BOOST_THREAD_CONDITION_VARIABLE_PTHREAD_HPP
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2007-8 Anthony Williams

#include "timespec.hpp"
#include "pthread_mutex_scoped_lock.hpp"
#include "thread_data.hpp"
#include "condition_variable_fwd.hpp"

#include <boost/config/abi_prefix.hpp>

namespace boost
{
    inline void condition_variable::wait(unique_lock<mutex>& m)
    {
        detail::interruption_checker check_for_interruption(&cond);
        BOOST_VERIFY(!pthread_cond_wait(&cond,m.mutex()->native_handle()));
    }

    inline bool condition_variable::timed_wait(unique_lock<mutex>& m,boost::system_time const& wait_until)
    {
        detail::interruption_checker check_for_interruption(&cond);
        struct timespec const timeout=detail::get_timespec(wait_until);
        int const cond_res=pthread_cond_timedwait(&cond,m.mutex()->native_handle(),&timeout);
        if(cond_res==ETIMEDOUT)
        {
            return false;
        }
        BOOST_ASSERT(!cond_res);
        return true;
    }

    inline void condition_variable::notify_one()
    {
        BOOST_VERIFY(!pthread_cond_signal(&cond));
    }
        
    inline void condition_variable::notify_all()
    {
        BOOST_VERIFY(!pthread_cond_broadcast(&cond));
    }
    
    class condition_variable_any
    {
        pthread_mutex_t internal_mutex;
        pthread_cond_t cond;

        condition_variable_any(condition_variable&);
        condition_variable_any& operator=(condition_variable&);

    public:
        condition_variable_any()
        {
            int const res=pthread_mutex_init(&internal_mutex,NULL);
            if(res)
            {
                boost::throw_exception(thread_resource_error());
            }
            int const res2=pthread_cond_init(&cond,NULL);
            if(res2)
            {
                BOOST_VERIFY(!pthread_mutex_destroy(&internal_mutex));
                boost::throw_exception(thread_resource_error());
            }
        }
        ~condition_variable_any()
        {
            BOOST_VERIFY(!pthread_mutex_destroy(&internal_mutex));
            BOOST_VERIFY(!pthread_cond_destroy(&cond));
        }
        
        template<typename lock_type>
        void wait(lock_type& m)
        {
            int res=0;
            {
                detail::interruption_checker check_for_interruption(&cond);
                {
                    boost::pthread::pthread_mutex_scoped_lock internal_lock(&internal_mutex);
                    m.unlock();
                    res=pthread_cond_wait(&cond,&internal_mutex);
                }
                m.lock();
            }
            if(res)
            {
                boost::throw_exception(condition_error());
            }
        }

        template<typename lock_type,typename predicate_type>
        void wait(lock_type& m,predicate_type pred)
        {
            while(!pred()) wait(m);
        }
        
        template<typename lock_type>
        bool timed_wait(lock_type& m,boost::system_time const& wait_until)
        {
            struct timespec const timeout=detail::get_timespec(wait_until);
            int res=0;
            {
                detail::interruption_checker check_for_interruption(&cond);
                {
                    boost::pthread::pthread_mutex_scoped_lock internal_lock(&internal_mutex);
                    m.unlock();
                    res=pthread_cond_timedwait(&cond,&internal_mutex,&timeout);
                }
                m.lock();
            }
            if(res==ETIMEDOUT)
            {
                return false;
            }
            if(res)
            {
                boost::throw_exception(condition_error());
            }
            return true;
        }
        template<typename lock_type>
        bool timed_wait(lock_type& m,xtime const& wait_until)
        {
            return timed_wait(m,system_time(wait_until));
        }

        template<typename lock_type,typename duration_type>
        bool timed_wait(lock_type& m,duration_type const& wait_duration)
        {
            return timed_wait(m,get_system_time()+wait_duration);
        }

        template<typename lock_type,typename predicate_type>
        bool timed_wait(lock_type& m,boost::system_time const& wait_until,predicate_type pred)
        {
            while (!pred())
            {
                if(!timed_wait(m, wait_until))
                    return pred();
            }
            return true;
        }

        template<typename lock_type,typename predicate_type>
        bool timed_wait(lock_type& m,xtime const& wait_until,predicate_type pred)
        {
            return timed_wait(m,system_time(wait_until),pred);
        }

        template<typename lock_type,typename duration_type,typename predicate_type>
        bool timed_wait(lock_type& m,duration_type const& wait_duration,predicate_type pred)
        {
            return timed_wait(m,get_system_time()+wait_duration,pred);
        }

        void notify_one()
        {
            boost::pthread::pthread_mutex_scoped_lock internal_lock(&internal_mutex);
            BOOST_VERIFY(!pthread_cond_signal(&cond));
        }
        
        void notify_all()
        {
            boost::pthread::pthread_mutex_scoped_lock internal_lock(&internal_mutex);
            BOOST_VERIFY(!pthread_cond_broadcast(&cond));
        }
    };

}

#include <boost/config/abi_suffix.hpp>

#endif
