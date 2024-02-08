# Desert UDP position provider

This addon provides a simple position setter that get the data via a UDP socket.

## Usage

- Create an instance of the provider and add it to the node object `$node_`:

   ```tcl
   set pos_provider [new "Position/UDP"]
   $node_ addPosition $pos_provider
   ```
- Set the UDP port to listen on and start the receive thread:
    
   ```tcl
   $pos_provider set udp_receive_port_ 1234
   $pos_provider start
   ```

- After the simulation, stop the receive thread:
   ```tcl
   $pos_provider stop
   ```

You may activate debug output for the addon with setting the debug_ variable to 1:

```tcl
$pos_provider set debug_ 1
```
   
## Data format

Since there are no specifications for the coordinate system used in DESERT, this should be done here. `x`, `y`, `z` are expected to be defined as local tangential plane coordinates, more precise as ENU (x=east, y=north, z=up). Depth values have to represented as negative z: `z =  -depth`.

See https://en.wikipedia.org/wiki/Local_tangent_plane_coordinates for more information.

## Example

```python
import struct

x = 1.2
y = 2.4
z = 3.6
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
packed_data = struct.pack("<ddd", x, y, z)
s.sendto(packed_data, ('127.0.0.1', 1234))
s.close()
```
