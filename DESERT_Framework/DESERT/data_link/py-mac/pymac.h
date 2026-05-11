#ifndef PYMAC_H
#define PYMAC_H

#include <mmac.h>
#include <queue>
#include <pybind11/embed.h>

namespace py = pybind11;

class PyMac;

/**
 * Timer handler for periodically invoking Python MAC logic.
 */
class PyMacTimer : public TimerHandler {
public:
    PyMacTimer(PyMac* mac) : TimerHandler(), mac_(mac) {}
    void expire(Event* e) override;
    
private:
    PyMac* mac_;
};

/**
 * @brief MAC protocol with logic driven by Python.
 *
 * Most of the protocol logic is in Python (mac_logic.py), with C++ providing
 * the NS-2 interface, packet handling, and scheduler access.
 */
class PyMac : public MMac
{
public:
    PyMac();
    virtual ~PyMac();

    /**
     * Tcl command interpreter.  Delegates to the base class implementation.
     */
    virtual int command(int argc, const char*const* argv) override;

    /**
     * Called when a packet arrives from the upper layer.
     */
    virtual void recvFromUpperLayers(Packet* p) override;
    
    /**
     * Called by the timer to invoke Python MAC logic.
     */
    void on_timer();

private:
    std::queue<Packet*> packet_queue_;
    PyMacTimer timer_;
    py::object py_mac_state_;  // Python object holding MAC state
    bool timer_scheduled_;
};

#endif // PYMAC_H