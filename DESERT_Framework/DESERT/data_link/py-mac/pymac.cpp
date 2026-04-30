// Python headers should be the very first thing we pull in.  Some
// third-party headers (mmac, Tcl, etc.) may themselves include Python.h with
// limited-API macros or undefine symbols later, so we protect ourselves by
// defining the missing bits both before and after including them.
#include <iostream>
#include <iostream>
#include <Python.h>
#ifndef Py_REFCNT
// direct field access if the standard macro is absent
#    define Py_REFCNT(ob) (((PyObject*)(ob))->ob_refcnt)
#endif
#ifndef _Py_REFCNT
#    define _Py_REFCNT(ob) (((PyObject*)(ob))->ob_refcnt)
#endif
// ensure rich-compare is declared even if Python.h was brought in with
// Py_LIMITED_API or otherwise skipped it
#ifndef PyObject_RichCompareBool
extern "C" int PyObject_RichCompareBool(PyObject *a, PyObject *b, int op);
#endif

#include "pymac.h"

// some headers included by pymac.h (e.g. mmac.h) may undefine the macros above
// or re-include Python.h; redefine them again so pybind11 sees them.
#ifndef Py_REFCNT
#    define Py_REFCNT(ob) (((PyObject*)(ob))->ob_refcnt)
#endif
#ifndef _Py_REFCNT
#    define _Py_REFCNT(ob) (((PyObject*)(ob))->ob_refcnt)
#endif
#ifndef PyObject_RichCompareBool
extern "C" int PyObject_RichCompareBool(PyObject *a, PyObject *b, int op);
#endif

#include <tclcl.h>
#include <scheduler.h>
#include <pybind11/embed.h> // Everything needed for embedding
#include <pybind11/functional.h>
namespace py = pybind11;

/**
 * Pybind11 module exposing NS-2 scheduler and Packet to Python.
 * Allows Python code to schedule events, read simulation time, and access packet fields.
 */
PYBIND11_EMBEDDED_MODULE(ns2_scheduler, m) {
    m.def("current_time", []() {
        return Scheduler::instance().clock();
    }, "Get current simulation time");
}

/**
 * Pybind11 module exposing Packet interface to Python.
 */
PYBIND11_EMBEDDED_MODULE(ns2_packet, m) {
    py::class_<Packet, std::shared_ptr<Packet>>(m, "Packet")
        .def("uid", [](Packet* p) {
            return HDR_CMN(p)->uid_;
        }, "Get packet unique ID")
        .def("size", [](Packet* p) {
            return HDR_CMN(p)->size_;
        }, "Get packet size in bytes")
        .def("ptype", [](Packet* p) {
            return HDR_CMN(p)->ptype_;
        }, "Get packet type")
        .def("timestamp", [](Packet* p) {
            return HDR_CMN(p)->ts_;
        }, "Get packet timestamp")
        .def("next_hop", [](Packet* p) {
            return HDR_CMN(p)->next_hop_;
        }, "Get next hop address")
        .def("prev_hop", [](Packet* p) {
            return HDR_CMN(p)->prev_hop_;
        }, "Get previous hop address")
        .def("error", [](Packet* p) {
            return HDR_CMN(p)->error_;
        }, "Get error flag")
        .def("direction", [](Packet* p) {
            return static_cast<int>(HDR_CMN(p)->direction_);
        }, "Get direction (-1=down, 0=none, 1=up)")
        .def("__repr__", [](Packet* p) {
            char buf[256];
            sprintf(buf, "<Packet uid=%d size=%d ptype=%d next_hop=%d prev_hop=%d ts=%.6f>",
                    HDR_CMN(p)->uid_, HDR_CMN(p)->size_, HDR_CMN(p)->ptype_,
                    HDR_CMN(p)->next_hop_, HDR_CMN(p)->prev_hop_, HDR_CMN(p)->ts_);
            return std::string(buf);
        });
}

// Tcl registration so ns-2 can create the object from OTcl scripts.
static class PyMacClass : public TclClass {
public:
    PyMacClass() : TclClass("Module/UW/DataLink/PyMac") {}
    TclObject* create(int, const char*const*) override {
        return (new PyMac());
    }
} class_pymac;

PyMac::PyMac() : MMac(), timer_(this), timer_scheduled_(false)
{
    static py::scoped_interpreter guard{};
    
    // Add the lib directory to Python's search path so mac_logic.py can be imported
    py::module_ sys = py::module_::import("sys");
    py::list path = sys.attr("path");
    path.append(std::string(INSTALL_LIB_DIR));
    
    // Import mac_logic module and create state
    py::module_ mac_logic = py::module_::import("mac_logic");
    py_mac_state_ = mac_logic.attr("MacState")();
}

PyMac::~PyMac()
{
    // Clean up
}

int PyMac::command(int argc, const char*const* argv)
{
    return MMac::command(argc, argv);
}

void PyMacTimer::expire(Event* e)
{
    mac_->on_timer();
}

void PyMac::on_timer()
{
    // Invoke Python logic
    try {
        py::module_ mac_logic = py::module_::import("mac_logic");
        
        // Get current simulation time
        double now = Scheduler::instance().clock();
        int queue_size = packet_queue_.size();
        
        // Convert packet queue to Python list (for now, just pass queue size)
        // In future, could pass actual packet objects if needed
        py::list packets;
        std::queue<Packet*> temp_queue = packet_queue_;
        while (!temp_queue.empty()) {
            packets.append(temp_queue.front());
            temp_queue.pop();
        }
        
        // Call Python function
        py::dict result = mac_logic.attr("on_event")(
            py_mac_state_,
            now,
            queue_size,
            packets  // Pass actual packet objects to Python
        ).cast<py::dict>();
        
        // Parse result
        std::string action = py::cast<std::string>(result["action"]);
        double next_event_delay = py::cast<double>(result["next_event_delay"]);
        
        if (action == "transmit" && !packet_queue_.empty()) {
            Packet* pkt = packet_queue_.front();
            packet_queue_.pop();
            Mac2PhyStartTx(pkt);
        }
        
        // Reschedule if needed
        if (next_event_delay > 0) {
            timer_scheduled_ = true;
            timer_.resched(next_event_delay);
        } else {
            // Timer has fired and we're not rescheduling
            cout << "No further events scheduled by Python logic." << endl;
            timer_scheduled_ = false;
            // Don't call cancel() here—the timer just fired, it's not scheduled anymore
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in on_timer: " << e.what() << std::endl;
    }
}

void PyMac::recvFromUpperLayers(Packet* p)
{
    packet_queue_.push(p);
    
    // Schedule timer if not already scheduled
    if (!timer_scheduled_) {
        timer_scheduled_ = true;
        // Schedule for immediate processing (sched is for first-time scheduling)
        timer_.sched(0);
    }
}