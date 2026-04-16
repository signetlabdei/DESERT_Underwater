# mac_logic.py
import random

# Global state for backoff
backoff_counter = 0
max_backoff = 10

def should_transmit(is_busy, queue_size):
    """
    Simple CSMA with backoff MAC logic.
    Returns: 0 for Wait, 1 for Transmit
    """
    global backoff_counter
    print("is_busy={}, queue_size={}, backoff_counter={}".format(is_busy, queue_size, backoff_counter))
    if is_busy:
        # Channel busy, start or continue backoff
        if backoff_counter == 0:
            backoff_counter = random.randint(1, max_backoff)
        return 0
    else:
        # Channel free
        if backoff_counter > 0:
            backoff_counter -= 1
            return 0
        else:
            # Backoff done, transmit if we have packets
            if queue_size > 0:
                return 1
            return 0