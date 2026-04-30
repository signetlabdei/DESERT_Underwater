# mac_logic.py
"""
MAC protocol logic implemented in Python.

The protocol is driven by a C++ timer (PyMacTimer) that calls on_event()
at regular intervals. The MacState object tracks protocol state and makes
decisions about when to transmit.
"""

import random


class MacState:
    """Encapsulates the state of the MAC protocol."""
    
    def __init__(self):
        """Initialize MAC state."""
        self.backoff_counter = 0
        self.max_backoff = 10
        self.state = "IDLE"  # States: IDLE, BACKOFF, TX
        self.last_event_time = 0.0
    
    def __repr__(self):
        return (f"MacState(state={self.state}, backoff={self.backoff_counter}, "
                f"last_event_time={self.last_event_time})")


def on_event(mac_state, current_time, queue_size, packets):
    """
    Called periodically by C++ to run MAC protocol logic.
    
    Args:
        mac_state: MacState object (mutable)
        current_time: Current simulation time (float)
        queue_size: Number of packets in the queue (int)
        packets: List of Packet objects from the queue
    
    Returns:
        dict with:
            - 'action': 'transmit', 'wait', or 'idle'
            - 'next_event_delay': seconds until next event (0 = no reschedule)
    """
    
    print(f"[{current_time:.4f}] on_event: queue_size={queue_size}, state={mac_state}")
    
    # Example: inspect first packet if available
    if packets:
        pkt = packets[0]
        print(f"  First packet: {pkt}")  # Uses __repr__ to show packet info
        # Access fields from common header:
        # pkt.uid(), pkt.size(), pkt.ptype(), pkt.timestamp()
        # pkt.next_hop(), pkt.prev_hop(), pkt.error(), pkt.direction()
    
    if mac_state.state == "IDLE":
        if queue_size > 0:
            # Packets waiting, start channel check
            mac_state.state = "BACKOFF"
            mac_state.backoff_counter = 0
            # Check channel again soon
            return {
                'action': 'wait',
                'next_event_delay': 0.001  # Check in 1 ms
            }
        else:
            # No packets, stay idle
            return {
                'action': 'wait',
                'next_event_delay': 0.0  # Don't reschedule
            }
    
    elif mac_state.state == "BACKOFF":
        if queue_size == 0:
            # Queue emptied, go back to idle
            mac_state.state = "IDLE"
            return {
                'action': 'wait',
                'next_event_delay': 0.0
            }
        
        # Simulate channel sensing (in a real implementation this would 
        # come from physical layer feedback)
        is_channel_busy = random.random() < 0.1  # 10% chance busy
        
        if is_channel_busy:
            # Channel busy, increase backoff
            if mac_state.backoff_counter == 0:
                mac_state.backoff_counter = random.randint(1, mac_state.max_backoff)
            print(f"  Channel busy, backoff={mac_state.backoff_counter}")
            
            # Wait before next check
            return {
                'action': 'wait',
                'next_event_delay': 0.01  # Check in 10 ms
            }
        else:
            # Channel free, decrement backoff
            if mac_state.backoff_counter > 0:
                mac_state.backoff_counter -= 1
                print(f"  Channel free, backoff decremented to {mac_state.backoff_counter}")
                
                # Check again soon
                return {
                    'action': 'wait',
                    'next_event_delay': 0.001
                }
            else:
                # Backoff done and channel free, transmit!
                mac_state.state = "TX"
                print(f"  Transmitting packet")
                
                # After transmission, assume we go back to IDLE
                # (In reality, we'd wait for TX to complete)
                mac_state.state = "IDLE"
                
                return {
                    'action': 'transmit',
                    'next_event_delay': 0.0  # Will reschedule when next packet arrives
                }
    
    return {
        'action': 'wait',
        'next_event_delay': 0.0
    }
