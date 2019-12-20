#include "ouroboros/transaction.h"

namespace ouroboros
{

//==============================================================================
//  global_locker
//==============================================================================
//static
count_type base_global_locker::s_scoped_count = 0;

//static
count_type base_global_locker::s_sharable_count = 0;

/**
 * Get the count of the locks for read data
 * @return the count of the locks for read data
 */
//static
count_type base_global_locker::sharable_count()
{
    return s_sharable_count;
}

/**
 * Get the count of the locks for write data
 * @return the count of the locks for write data
 */
//static
count_type base_global_locker::scoped_count()
{
    return s_scoped_count;
}

//==============================================================================
//  base_transaction
//==============================================================================
/**
 * Constructor
 */
base_transaction::base_transaction() :
    m_enabled(false)
{

}

/**
 * Destructor
 */
//virtual
base_transaction::~base_transaction()
{

}

/**
 * Start the transaction
 */
//virtual
void base_transaction::start()
{
    m_enabled = true;
}

/**
 * Stop the transaction
 */
//virtual
void base_transaction::stop()
{
    m_enabled = false;
}

/**
 * Cancel the transaction
 */
//virtual
void base_transaction::cancel()
{
    m_enabled = false;
}

/**
 * Check the transaction is enabled
 * @return the result is checking
 */
bool base_transaction::enabled() const
{
    return m_enabled;
}

/**
 * Attach a transaction
 * @param transact the transaction
 */
//virtual
void base_transaction::attach(base_transaction *transact)
{
    OUROBOROS_THROW_BUG("method not supported");
}

}   // namespace ouroboros
