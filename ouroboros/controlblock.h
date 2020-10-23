#ifndef CONTROLBLOCK_H
#define CONTROLBLOCK_H

#include "ouroboros/global.h"

namespace ouroboros
{

/**
 * The control block of a table
 */
template <typename Key, typename Interface>
class control_block
{
public:
    typedef Key skey_type;
    typedef Interface interface_type;
    typedef typename interface_type::locker_type::lock_type lock_type;
    control_block(skey_type& skey, lock_type& lock) :
        m_skey(skey),
        m_lock(lock)
    {}
    skey_type& get_skey() const
    {
        return m_skey;
    }
    lock_type& get_lock() const
    {
        return m_lock;
    }
private:
    control_block();
private:
    skey_type& m_skey;
    lock_type& m_lock;
};

} // namespace ouroboros

#endif /* CONTROLBLOCK_H */

