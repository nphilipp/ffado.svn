/*
 * Copyright (C) 2005-2007 by Pieter Palmers
 *
 * This file is part of FFADO
 * FFADO = Free Firewire (pro-)audio drivers for linux
 *
 * FFADO is based upon FreeBoB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "libutil/Thread.h"
#include "cycletimer.h"

/**
 * Implements a DLL based mechanism to track the cycle timer
 * register of the Ieee1394Service pointed to by the 'parent'.
 *
 * A DLL mechanism is performance-wise better suited, since it
 * does not require an OS call. Hence we run a thread to update
 * the DLL at regular time intervals, and then use the DLL to
 * generate a cycle timer estimate for the parent to pass on
 * to it's clients.
 *
 * The idea is to make reading the cycle timer real-time safe,
 * which isn't (necessarily)the case for the direct raw1394 call,
 * since it's a kernel call that could block (although the current
 * implementation is RT safe).
 *
 * This also allows us to run on systems not having the
 * raw1394_read_cycle_timer kernel call. We can always do a normal
 * read of our own cycle timer register, but that's not very accurate.
 * The accuracy is improved by this DLL mechanism. Still not as good
 * as when using the raw1394_read_cycle_timer call, but anyway.
 *
 * On the long run this code will also allow us to map system time
 * on to 1394 time for the current host controller, hence enabling
 * different clock domains to operate together.
 */
#ifndef __CYCLETIMERTHREAD_H__
#define __CYCLETIMERTHREAD_H__

#include "debugmodule/debugmodule.h"

class Ieee1394Service;
namespace Util {
    class TimeSource;
    class Thread;
}

class CycleTimerHelper : public Util::RunnableInterface
{
public:
    CycleTimerHelper(Ieee1394Service &, unsigned int);
    CycleTimerHelper(Ieee1394Service &, unsigned int, bool rt, int prio);
    ~CycleTimerHelper();

    virtual bool Init();
    virtual bool Execute();

    bool setThreadParameters(bool rt, int priority);
    bool Start();

    /**
     * @brief get the current cycle timer value (in ticks)
     * @note thread safe
     */
    uint32_t getCycleTimerTicks();

    /**
     * @brief get the cycle timer value for a specific time instant (in ticks)
     * @note thread safe
     */
    uint32_t getCycleTimerTicks(uint64_t now);

    /**
     * @brief get the current cycle timer value (in CTR format)
     * @note thread safe
     */
    uint32_t getCycleTimer();

    /**
     * @brief get the cycle timer value for a specific time instant (in CTR format)
     * @note thread safe
     */
    uint32_t getCycleTimer(uint64_t now);

    float getRate();
    float getNominalRate();

    void setVerboseLevel(int l);

private:
    Ieee1394Service &m_Parent;
    // parameters
    uint32_t m_ticks_per_update;
    uint32_t m_usecs_per_update;

    float               m_avg_wakeup_delay; 

    // state variables
    double m_dll_e2;

    double m_current_time_usecs;
    double m_next_time_usecs;
    double m_current_time_ticks;
    double m_next_time_ticks;
    bool m_first_run;

    // cached vars used for computation
    struct compute_vars {
        double usecs;
        double ticks;
        double rate;
    };

    struct compute_vars m_current_vars;
    pthread_mutex_t m_compute_vars_lock;

    // Threading
    Util::Thread *  m_Thread;
    bool            m_realtime;
    unsigned int    m_priority;

    // debug stuff
    DECLARE_DEBUG_MODULE;
};
#endif