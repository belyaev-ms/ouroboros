/**
 * @file    sharedorderdlock.h
 */

#ifndef OUROBOROS_SHARED_ORDERED_LOCK_H
#define OUROBOROS_SHARED_ORDERED_LOCK_H

#ifndef OUROBOROS_BOOST_ENABLED
//#error Can not use shared_ordered_lock (OUROBOROS_BOOST_ENABLED)
#endif

#include <limits.h>
#include <thread>
#include <boost/thread/thread_time.hpp>
#ifdef OUROBOROS_SPINLOCK_ENABLED
#include <boost/smart_ptr/detail/spinlock.hpp>
#else
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#endif
#include "ouroboros/error.h"

namespace ouroboros
{

/**
 * Simple class for ordered RW lock which is placed in a shared memory
 */
class shared_ordered_lock
{
public:
    shared_ordered_lock();
    void lock();
    bool timed_lock(const boost::posix_time::ptime& abs_time);
    bool try_lock();
    void unlock();
    void lock_sharable();
    bool timed_lock_sharable(const boost::posix_time::ptime& abs_time);
    bool try_lock_sharable();
    void unlock_sharable();
protected:
    enum lock_state
    {
        LS_SCOPED,
        LS_SHARABLE,
        LS_NONE
    };
    struct ticket_type
    {
        ticket_type() :
            number(UINT_MAX),
            current_number(UINT_MAX),
            counter(0)
        {}
        bool valid() const
        {
            return number != UINT_MAX;
        }
        unsigned int number;
        unsigned int current_number;
        unsigned int counter;
    };
    lock_state try_book_lock(ticket_type& ticket);
    lock_state try_book_lock_sharable(ticket_type& ticket);
    inline unsigned int sharable_counter();
    void register_ticket(ticket_type& ticket);
    void update_ticket(ticket_type& ticket);
    void remove_ticket(ticket_type& ticket);
    void refresh_queue();
    inline void delay() const;
private:
    shared_ordered_lock(const shared_ordered_lock& );
    shared_ordered_lock& operator=(const shared_ordered_lock& );
private:
#ifdef OUROBOROS_SPINLOCK_ENABLED
    typedef boost::detail::spinlock lock_type;
    typedef boost::detail::spinlock::scoped_lock guard_type;
#else
    typedef boost::interprocess::interprocess_mutex lock_type;
    typedef boost::interprocess::scoped_lock<lock_type> guard_type;
#endif
    lock_type m_lock;
    unsigned int m_scoped_counter;
    unsigned int m_sharable_counter;
    unsigned int m_waiting_counter;
    unsigned int m_current_number;
    unsigned int m_booked_number;
};

/**
 * Constructor
 */
shared_ordered_lock::shared_ordered_lock() :
    m_scoped_counter(0),
    m_sharable_counter(0),
    m_waiting_counter(0),
    m_current_number(0),
    m_booked_number(0)
{
}

/**
 * Delay
 */
inline void shared_ordered_lock::delay() const
{
    usleep(10);
}

/**
 * Refresh the queue
 */
void shared_ordered_lock::refresh_queue()
{
    OUROBOROS_STANDART_ASSERT(m_waiting_counter > 0);
    --m_waiting_counter;
    if (0 == m_waiting_counter)
    {
        m_booked_number = m_current_number = 0;
    }
    else if (++m_current_number > m_booked_number)
    {
        m_booked_number = m_current_number;
    }
}

/**
 * Update the ticket
 * @param ticket the ticket
 */
void shared_ordered_lock::update_ticket(ticket_type& ticket)
{
    if (0 == m_scoped_counter && m_sharable_counter < UINT_MAX)
    {
        if (++ticket.counter > m_waiting_counter + 1)
        {
            ticket.current_number = ++m_current_number;
            ticket.counter = 0;
        }
    }
}

/**
 * Register the ticket
 * @param ticket the ticket
 */
void shared_ordered_lock::register_ticket(ticket_type& ticket)
{
    ticket.number = m_booked_number++;
    ticket.current_number = m_current_number;
    ++m_waiting_counter;
}

/**
 * Try to book the exclusive lock
 * @param ticket booked ticket
 * @return the type of the current lock
 */
shared_ordered_lock::lock_state shared_ordered_lock::try_book_lock(ticket_type& ticket)
{
    guard_type guard(m_lock);
    OUROBOROS_STANDART_ASSERT(ticket.number >= m_current_number);
    if (0 == m_scoped_counter)
    {
        m_scoped_counter = 1;
        if (m_current_number == m_booked_number)
        {
            return 0 == m_sharable_counter ? LS_NONE : LS_SHARABLE;
        }
        if (ticket.number == m_current_number)
        {
            refresh_queue();
            return 0 == m_sharable_counter ? LS_NONE : LS_SHARABLE;
        }
    }
    if (!ticket.valid())
    {
        register_ticket(ticket);
    }
    else if (1 == m_waiting_counter)
    {
        if (0 == m_scoped_counter)
        {
            m_booked_number = m_current_number = m_waiting_counter = 0;
            m_scoped_counter = 1;
            return 0 == m_sharable_counter ? LS_NONE : LS_SHARABLE;
        }
    }
    else if (ticket.current_number == m_current_number)
    {
        update_ticket(ticket);
    }
    else
    {
        ticket.current_number = m_current_number;
    }
    return LS_SCOPED;
}

/**
 * Get the count of the sharable lock
 * @return the count of the sharable lock
 */
inline unsigned int shared_ordered_lock::sharable_counter()
{
    guard_type guard(m_lock);
    return m_sharable_counter;
}

/**
 * Try to set the exclusive lock
 * @return the result of the setting
 */
bool shared_ordered_lock::try_lock()
{
    guard_type guard(m_lock);
    if (0 == m_scoped_counter && 0 == m_sharable_counter &&
        m_current_number == m_booked_number)
    {
        m_scoped_counter = 1;
        return true;
    }
    return false;
}

/**
 * Set the exclusive lock
 */
void shared_ordered_lock::lock()
{
    ticket_type ticket;
    while (1)
    {
        switch (try_book_lock(ticket))
        {
            case LS_NONE:
                return;
            case LS_SCOPED:
                break;
            case LS_SHARABLE:
                while (sharable_counter() > 0)
                {
                    delay();
                }
                return;
        }
        delay();
    }
}

/**
 * Remove the ticket
 * @param ticket the ticket
 */
void shared_ordered_lock::remove_ticket(ticket_type& ticket)
{
    if (ticket.valid())
    {
        guard_type guard(m_lock);
        OUROBOROS_STANDART_ASSERT(m_waiting_counter > 0);
        if (0 == --m_waiting_counter)
        {
            m_booked_number = m_current_number = 0;
        }
    }
}

/**
 * Try to set the exclusive lock until the time comes
 * @param abs_time the time
 * @return the result of the setting
 */
bool shared_ordered_lock::timed_lock(const boost::posix_time::ptime& abs_time)
{
    ticket_type ticket;
    while (boost::get_system_time() < abs_time)
    {
        switch (try_book_lock(ticket))
        {
            case LS_NONE:
                return true;
            case LS_SCOPED:
                break;
            case LS_SHARABLE:
                while (boost::get_system_time() < abs_time)
                {
                    if (0 == sharable_counter())
                    {
                        return true;
                    }
                    delay();
                }
                unlock();
                return false;
        }
        delay();
    }
    remove_ticket(ticket);
    return false;
}

/**
 * Remove the exclusive lock
 */
void shared_ordered_lock::unlock()
{
    guard_type guard(m_lock);
    OUROBOROS_STANDART_ASSERT(1 == m_scoped_counter);
    m_scoped_counter = 0;
}

/**
 * Try to book the sharable lock
 * @param ticket booked ticket
 * @return the type of the current lock
 */
shared_ordered_lock::lock_state shared_ordered_lock::try_book_lock_sharable(ticket_type& ticket)
{
    guard_type guard(m_lock);
    OUROBOROS_STANDART_ASSERT(ticket.number >= m_current_number);
    if (0 == m_scoped_counter && m_sharable_counter < UINT_MAX)
    {
        if (m_current_number == m_booked_number)
        {
            return 0 == m_sharable_counter++ ? LS_NONE : LS_SHARABLE;
        }
        if (ticket.number == m_current_number)
        {
            refresh_queue();
            return 0 == m_sharable_counter++ ? LS_NONE : LS_SHARABLE;
        }
    }
    if (!ticket.valid())
    {
        register_ticket(ticket);
    }
    else if (1 == m_waiting_counter)
    {
        if (0 == m_scoped_counter && m_sharable_counter < UINT_MAX)
        {
            m_booked_number = m_current_number = m_waiting_counter = 0;
            return 0 == m_sharable_counter++ ? LS_NONE : LS_SHARABLE;
        }
    }
    else if (ticket.current_number == m_current_number)
    {
        update_ticket(ticket);
    }
    else
    {
        ticket.current_number = m_current_number;
    }
    return LS_SCOPED;
}

/**
 * Try to set the sharable lock
 * @return the result of the setting
 */
bool shared_ordered_lock::try_lock_sharable()
{
    guard_type guard(m_lock);
    if (0 == m_scoped_counter && m_sharable_counter < UINT_MAX &&
        m_current_number == m_booked_number)
    {
        ++m_sharable_counter;
        return true;
    }
    return false;
}

/**
 * Set the sharable lock
 */
void shared_ordered_lock::lock_sharable()
{
    ticket_type ticket;
    while (1)
    {
        switch (try_book_lock_sharable(ticket))
        {
            case LS_NONE:
            case LS_SHARABLE:
                return;
            case LS_SCOPED:
                break;
        }
        delay();
    }
}

/**
 * Try to set the sharable lock until the time comes
 * @param abs_time the time
 * @return the result of the setting
 */
bool shared_ordered_lock::timed_lock_sharable(const boost::posix_time::ptime& abs_time)
{
    ticket_type ticket;
    while (boost::get_system_time() < abs_time)
    {
        switch (try_book_lock_sharable(ticket))
        {
            case LS_NONE:
            case LS_SHARABLE:
                return true;
            case LS_SCOPED:
                break;
        }
        delay();
    }
    remove_ticket(ticket);
    return false;
}

/**
 * Remove the sharable lock
 */
void shared_ordered_lock::unlock_sharable()
{
    guard_type guard(m_lock);
    OUROBOROS_STANDART_ASSERT(m_sharable_counter > 0);
    --m_sharable_counter;
}

} // namespace ouroboros

#endif /* OUROBOROS_SHARED_ORDERED_LOCK_H */

