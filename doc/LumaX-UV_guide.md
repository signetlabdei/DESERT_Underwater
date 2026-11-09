# LumaXUV tests

These examples test DESERT’s LumaX-UV modem driver through an application TCP
socket. The modem data interface and the application socket are separate:
DESERT exchanges modem frames through UDP, while `nc` is used only to inject
and receive application payloads.

The directory contains three tests:

- `test_uwlumaxuvmodem.tcl`: single-node hardware test. Run one instance per
  modem, typically on separate machines.
- `test_uwlumaxuvmodem_multihop.tcl`: three-node hardware test with a relay.
- `test_uwlumaxuvmodem_multihop_sim.tcl`: local three-node simulation without
  modem hardware.

The multihop tests use the same logical topology:

```text
node 0 <-> node 1 <-> node 2
```

Node 1 is a transparent relay. Node 0 and node 2 are the application
endpoints. The nodes are stationary.

## Prerequisites

1. Build DESERT and make sure the `ns` executable and DESERT libraries are
   available in the environment.
2. Run commands from the DESERT workspace or use the absolute path to `ns`.
3. Install `netcat` or `ncat` on the machine where the application socket is
   running.
4. For the hardware test, connect and configure three LumaX-UV modems.

The application TCP ports and the LumaX-UV modem data ports are different:

| Port type | Purpose |
| --- | --- |
| Modem data port | UDP traffic between DESERT and a modem |
| Application TCP port | TCP connection used by `netcat` |

Never connect `netcat` directly to a modem data port.

## Single-node hardware test

Use `test_uwlumaxuvmodem.tcl` to connect two or more physical LumaX-UV modems.
Run one script instance for each modem. Each instance uses a distinct node ID
and application TCP port.

### Two machines, one modem per machine

Run one instance on each machine. The local data endpoint is the machine
interface connected to the modem; the configuration address is the modem HTTP
address.

Machine A:

```bash
source DESERT_buildCopy_LOCAL/environment
ns DESERT_Framework/DESERT/samples/desert_samples/PHY/LumaXUV/test_uwlumaxuvmodem.tcl \
  1 1 300 0 192.168.102.50 55555 192.168.102.101 44440
```

Machine B:

```bash
source DESERT_buildCopy_LOCAL/environment
ns DESERT_Framework/DESERT/samples/desert_samples/PHY/LumaXUV/test_uwlumaxuvmodem.tcl \
  2 1 300 0 192.168.102.51 55555 192.168.102.102 44442
```

After both scripts are running, connect to the application sockets:

```bash
nc 192.168.102.50 44440   # Machine A
nc 192.168.102.51 44442   # Machine B
```

Use the application ports (`44440`, `44442`) with `nc`, never the modem data
port (`55555`). Ensure TCP application ports and UDP multicast traffic are
allowed by the firewalls.

## Hardware test

Use this test when three LumaX-UV modems are available. The script uses the
LumaX-UV driver, real-time scheduling, and the modem configuration API.

### Arguments

The hardware multihop script requires nine arguments:

1. Simulation stop time.
2. Node 0 local modem data endpoint in `IP:port` format.
3. Node 0 modem configuration IP address.
4. Node 1 local modem data endpoint in `IP:port` format.
5. Node 1 modem configuration IP address.
6. Node 2 local modem data endpoint in `IP:port` format.
7. Node 2 modem configuration IP address.
8. Node 0 application TCP port.
9. Node 2 application TCP port.

The local data endpoints must be reachable from the host running NS-Miracle.
The configuration addresses must be reachable through HTTP by the LumaX-UV
driver. Pass the configuration address without `http://`.

### Start the hardware test

From the workspace root:

```bash
ns DESERT_Framework/DESERT/samples/desert_samples/PHY/LumaXUV/test_uwlumaxuvmodem_multihop.tcl \
  100 \
  192.168.102.50:55555 192.168.102.101 \
  192.168.102.51:55555 192.168.102.102 \
  192.168.102.52:55555 192.168.102.103 \
  44440 44442
```

The command starts three DESERT nodes in one NS-Miracle process. Make sure
all three modem endpoints are available before starting it.

### Test node 0 to node 2

The application modules create TCP listeners on nodes 0 and 2. `netcat`
connects to those listeners; it does not listen on the application ports.

1. Start the hardware test.
1. From the host running node 2, connect to the node 2 application port:

```bash
nc 192.168.102.52 44442
```

1. From the host running node 0, connect to the node 0 application port:

```bash
nc 192.168.102.50 44440
```

1. Type a short message in the node 0 terminal and press Enter.
1. The message should appear in the node 2 terminal.

For a test where all application sockets are on the same host, use:

```bash
nc 127.0.0.1 44442
nc 127.0.0.1 44440
```

Use the first command in one terminal and the second command in another.

### Test node 2 to node 0

Keep the simulation running and connect the opposite way:

```bash
nc 192.168.102.50 44440
nc 192.168.102.52 44442
```

Type the message in the node 2 terminal. It should appear at node 0.

### Verify the relay

Node 1 does not have an external application socket. It forwards packets
below the application layer. To verify the relay:

1. Enable routing, MLL, MAC, and application debug logs in the Tcl file.
2. Send a short message from node 0 to node 2.
3. Confirm that logs show node 0 using node 1 as its next hop.
4. Confirm that node 1 receives the frame addressed to its MAC address.
5. Confirm that node 1 forwards the packet toward node 2.
6. Confirm that node 2 receives the application payload.

The configured routes, shown as node IDs and IP addresses, are:

```text
node 0 (IP 1): destination IP 3 via node 1 (next-hop IP 2)
node 1 (IP 2): destination IP 1 via node 0 (next-hop IP 1)
node 1 (IP 2): destination IP 3 via node 2 (next-hop IP 3)
node 2 (IP 3): destination IP 1 via node 1 (next-hop IP 2)
```

The IP addresses are one-based: node 0 has IP 1, node 1 has IP 2, and node 2
has IP 3. If node 1 is stopped or disconnected, node 0 and node 2 must no
longer be able to exchange messages through this topology.

## Local simulation

Use this test without any LumaX-UV hardware. It replaces the modem driver
with `Module/MPhy/BPSK`, `UnderwaterChannel`, and underwater propagation.
The application, UDP, IP, MLL, MAC, routes, and fixed topology remain the
same as in the hardware test.

### Start the simulation

Run with the default 60-second duration and application ports:

```bash
ns DESERT_Framework/DESERT/samples/desert_samples/PHY/LumaXUV/test_uwlumaxuvmodem_multihop_sim.tcl
```

Or choose a duration and different application ports:

```bash
ns DESERT_Framework/DESERT/samples/desert_samples/PHY/LumaXUV/test_uwlumaxuvmodem_multihop_sim.tcl \
  60 44440 44442
```

The simulated nodes are positioned at 0 m, 100 m, and 200 m. The simulation
uses the real-time scheduler so that `netcat` can connect while it runs.

### Test the simulated path

1. Start the simulation.
2. In one terminal, connect to the node 2 application socket:

```bash
nc 127.0.0.1 44442
```

1. In another terminal, connect to the node 0 application socket:

```bash
nc 127.0.0.1 44440
```

1. Type a short message in the node 0 terminal and press Enter.
1. Verify that the message appears in the node 2 terminal.
1. Repeat in the opposite direction by typing in the node 2 terminal.

Do not use `nc -l` for this test. The DESERT application modules already own
ports 44440 and 44442 and act as TCP servers.

### Simulation result

At shutdown, the simulation prints application counters for nodes 0 and 2.
Successful tests should show transmitted and received packets for both leaf
applications. Node 1 is a relay and should not be expected to increment an
application receive counter.

To collect a trace, set `opt(trace_files)` to `1` in the simulation Tcl file.
The trace is written to `uwlumaxuvmodem_multihop_sim.tr`.

## Troubleshooting

- `Address already in use`: another process already owns the application
  port. Stop it or select different application ports.
- `Connection refused`: start the NS-Miracle script first and wait until the
  application modules have started at simulation time 1 second.
- No payload at the destination: check that the sender and receiver use the
  application ports, not the modem data ports. Then inspect routing, MLL,
  MAC, and application logs.
- Hardware configuration errors: check the modem configuration IP addresses
  and confirm that the LumaX-UV HTTP API is reachable.