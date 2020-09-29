/**
 * @file   error.h
 * The base errors and the base exceptions
 */

#ifndef OUROBOROS_ERROR_H
#define	OUROBOROS_ERROR_H

#include <typeinfo>
#include <exception>
#include <sstream>
#include "ouroboros/log.h"

namespace ouroboros
{

/** throw the exception */
#define OUROBOROS_THROW(ex) throw ouroboros::where(ex, typeid(ex).name(), __FILE__, __FUNCTION__, __LINE__)

template <class E>
const E& where(const E& ex, const char *name, const char *file, const char *func, const int line)
{
    OUROBOROS_ERROR(ex.what());
    ex.where(name, file, func, line);
    return ex;
}

/** throw the exception with the information message */
#define OUROBOROS_THROW_ERROR(err, message) do { const err ex; ex << message; OUROBOROS_THROW(ex); } while(0)
#define OUROBOROS_THROW_BUG(message) OUROBOROS_THROW_ERROR(ouroboros::bug_error, message)

/** check the condition */
#define OUROBOROS_THROW_ERROR_IF(condition, err, message) do { if (condition) { OUROBOROS_THROW_ERROR(err, message); } } while(0)
#if (defined OUROBOROS_TEST_ENABLED || defined OUROBOROS_TEST_TOOLS_ENABLED || defined OUROBOROS_STRICT_ASSERT_ENABLED)
#define OUROBOROS_ASSERT(condition) OUROBOROS_THROW_ERROR_IF(!(condition), ouroboros::bug_error, "assert failed : " << #condition)
#define OUROBOROS_RANGE_ASSERT(condition) OUROBOROS_THROW_ERROR_IF(!(condition), ouroboros::range_error, "assert failed : " << #condition)
#else
#define OUROBOROS_ASSERT(condition)
#define OUROBOROS_RANGE_ASSERT(condition)
#endif

/**
 * The base class of exception
 * @attention inherit their own exceptions from class base_error
 */
class base_exception : public std::exception
{
public:
    virtual ~base_exception() throw ()
    {
    }
    virtual const char* what() const throw()
    {
        return m_what.c_str();
    }
    void where(const char *name, const char *file, const char *func, const int line) const
    {
        std::ostringstream s;
        s << name << "::" << file << "::" << func << "(" << line << ")" << " : " << m_what;
        m_what = s.str();
    }
    template <typename T>
    const base_exception& operator<< (const T& arg) const
    {
        std::ostringstream s;
        s << m_what << arg;
        m_what = s.str();
        return *this;
    }
protected:
    base_exception() : std::exception()
    {
    }
private:
    mutable std::string m_what;
};

/** classes of exceptions */
struct bug_error : public base_exception { bug_error() : base_exception() {} };
struct base_error : public base_exception { base_error() : base_exception() {} };
struct range_error : public base_error { range_error() : base_error() {} };
struct compatibility_error : public base_error { compatibility_error() : base_error() {} };
struct version_error : public compatibility_error { version_error() : compatibility_error() {} };
struct lock_error : public base_error { lock_error() : base_error() {} };
struct io_error : public base_error { io_error() : base_error() {} };

inline std::ostream& operator<< (std::ostream& s, const base_exception& e)
{
    s << PE(e.what());
    return s;
}

}   //namespace ouroboros

#endif	/* OUROBOROS_ERROR_H */

