/**
 * @file    gateway.h
 * The class gateway that helps to sinchronize some operations of several
 * processes
 */

#ifndef OUROBOROS_GATEWAY_H
#define OUROBOROS_GATEWAY_H

#include <sched.h>
#include "ouroboros/locker.h"

namespace ouroboros
{

/*
 *            P1      P2      P3      P4
 *            |       |       |       |
 *  door1   +---+   +---+   +---+   +---+
 *  room1   |   |   |   |   |   |   |   |
 *  door2   +---+   +---+   +---+   +---+
 *  room2   |   |   |   |   |   |   |   |
 *  door3   +---+   +---+   +---+   +---+
 *  room3   |   |   |   |   |   |   |   |
 *  door4   +---+   +---+   +---+   +---+
 *            |       |       |       |
 *
 *  The first door is opened when there are one or more processes in the middle
 *  room or when there aren't any processes in the last room:
 *            |       |
 *  door1   +\ /+   +\ /+
 *  room1   |   |   |   |
 *  door2   +---+   +---+
 *  room2   | 1 |   |   |
 *  door3   +---+   +---+
 *  room3   |   |   | 0 |
 *  door4   +---+   +---+
 *            |       |
 *
 *  The second door is opened when there are two or more processes in the middle
 *  room or when there aren't any processes in the last room:
 *            |       |
 *  door1   +---+   +---+
 *  room1   |   |   |   |
 *  door2   +\ /+   +\ /+
 *  room2   | 2 |   |   |
 *  door3   +---+   +---+
 *  room3   |   |   | 0 |
 *  door4   +---+   +---+
 *            |       |
 *
 *  The third door is allways opened:
 *            |
 *  door1   +---+
 *  room1   |   |
 *  door2   +---+
 *  room2   |   |
 *  door3   +\ /+
 *  room3   |   |
 *  door4   +---+
 *            |
 *
 *  The fourth is opened when there aren't any processes in the middle room:
 *            |
 *  door1   +---+
 *  room1   |   |
 *  door2   +---+
 *  room2   | 0 |
 *  door3   +---+
 *  room3   |   |
 *  door4   +\ /+
 *            |
 *
 */

/**
 * The gateway
 */
template <typename Locker>
class gateway
{
    enum
    {
        FIRST_ROOM,
        MIDDLE_ROOM,
        LAST_ROOM,
        ROOMS_COUNT
    };
public:
    typedef Locker locker_type;
    typedef guard<locker_type> guard_type;
    gateway();
    void go_first_room();
    size_t go_middle_room();
    void go_last_room();
    void leave_last_room();
private:
    bool try_go_first_room();
    size_t try_go_middle_room();
    void try_go_last_room();
    bool try_leave_last_room();
private:
    locker_type m_locker;
    size_t m_counters[ROOMS_COUNT];
};

template < >
class gateway<stub_locker>
{
public:
    gateway() {}
    void go_first_room() {}
    size_t go_middle_room() { return 1; }
    void go_last_room() {}
    void leave_last_room() {};
};

//==============================================================================
//  gateway
//==============================================================================
/**
 * Constructor
 */
template <typename Locker>
gateway<Locker>::gateway()
{
    memset(m_counters, 0, sizeof(m_counters));
}

/**
 * Go to the first room
 */
template <typename Locker>
void gateway<Locker>::go_first_room()
{
    while (!try_go_first_room())
    {
        sched_yield();
    }
}

/**
 * Go to the middle room
 * @return your number in this room
 */
template <typename Locker>
size_t gateway<Locker>::go_middle_room()
{
    while (true)
    {
        size_t result = try_go_middle_room();
        if (result > 0)
        {
            return result;
        }
        sched_yield();
    }
}

/**
 * Go to the last room
 */
template <typename Locker>
void gateway<Locker>::go_last_room()
{
    guard_type lock(m_locker);
    --m_counters[MIDDLE_ROOM];
    ++m_counters[LAST_ROOM];
}

/**
 * Leave the last room
 */
template <typename Locker>
void gateway<Locker>::leave_last_room()
{
    while (!try_leave_last_room())
    {
        sched_yield();
    }
}

/**
 * Try to go to the first room
 * @return result of this attempt
 */
template <typename Locker>
bool gateway<Locker>::try_go_first_room()
{
    guard_type lock(m_locker);
    if (0 == m_counters[LAST_ROOM] || m_counters[MIDDLE_ROOM] > 0)
    {
        ++m_counters[FIRST_ROOM];
        return true;
    }
    return false;
}

/**
 * Try to go to the middle room
 * @return result of this attempt
 */
template <typename Locker>
size_t gateway<Locker>::try_go_middle_room()
{
    guard_type lock(m_locker);
    if (m_counters[MIDDLE_ROOM] > 1 || 0 == m_counters[LAST_ROOM])
    {
        --m_counters[FIRST_ROOM];
        return ++m_counters[MIDDLE_ROOM];
    }
    return 0;
}

/**
 * Try to leave the last room
 * @return result of this attempt
 */
template <typename Locker>
bool gateway<Locker>::try_leave_last_room()
{
    guard_type lock(m_locker);
    if (0 == m_counters[MIDDLE_ROOM])
    {
        --m_counters[LAST_ROOM];
        return true;
    }
    return false;
}

}   //namespace ouroboros

#endif /* OUROBOROS_GATEWAY_H */

