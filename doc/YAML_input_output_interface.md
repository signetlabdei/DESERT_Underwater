# YAML input and output for DESERT simulations

Normally, a DESERT simulation script gets its settings either from values hard-coded in the Tcl script or from command-line arguments in a fixed order.
For example, [the TDMA sample](../DESERT_Framework/DESERT/samples/desert_samples/MAC/test_uwtdma.tcl) expects a random seed, a CBR period and a packet size, in that order, when its `opt(bash_parameters)` flag is on, otherwise it will use default values.

We provide a system for configuring simulations from YAML files, so values can be changed by name without editing Tcl or remembering argument order.
The example explained in this document is available in the [YAML input/output sample](../DESERT_Framework/DESERT/samples/desert_samples/yaml_input_output_interface/).

## User guide

### Configuring the inputs

Simulation inputs can be set in `input_config.yaml`, which is organized into sections.

- **`opt`** fills the Tcl `opt` associative array. It holds general settings such as the number of nodes (`nn`), the simulation end time (`stoptime`) and the random number generator seed (`rngstream`).
- **Modules sections** group the parameters of DESERT modules. Each section is named after a generic layer or role (`APPLICATION`, `MAC`, `PHYSICAL`, and so on) and must contain a `tag` with the real DESERT module name. Other keys in the section are parameters of that module.

```yaml
opt:
  nn: 3

APPLICATION:
  tag: Module/UW/CBR
  packetSize_: 128

MAC:
  tag: Module/UW/TDMA
  tot_slots: "expr: ${opt.nn}"
  frame_duration: "expr: ${opt.nn} * (${APPLICATION.packetSize_} * 8) / ${PHYSICAL.BitRate_}"

PHYSICAL:
  tag: Module/UW/PHYSICAL
  BitRate_: 4000.0
```

Note that, a parameter value can be a numeric expression if you prefix it with `expr:`.
Inside an expression, `${SECTION.parameter}` refers to another value in the same file.

A separate file, `input_positions.yaml`, defines the position of each node in Cartesian space.
For every node ID from `0` to `opt.nn - 1`, it gives the `x`, `y` and `z` coordinates.

### Choosing the outputs

`output_config.yaml` controls which metrics of a module are saved after a run.
As in the input file, each section has a generic name and a module `tag`.
Set a metric to `1` to record it, or `0` to skip it:

```yaml
APPLICATION:
  tag: Module/UW/CBR
  PDR: 1
  throughput: 1
  sent_packets: 0
```

Results are appended to CSV files in the current directory, for example `Module_UW_CBR.csv` for the application layer module.
Each row contains the `rngstream` value plus a node or link identifier.

> **Note:** a CSV's header reflects the metrics selected when it was created. If you change the selection, rename the existing file before the next run, otherwise the columns won't match.

### Batch runs and plots

The example also provide a [marimo notebook](../DESERT_Framework/DESERT/samples/desert_samples/yaml_input_output_interface/run_plot_marimo.py) that is a minimal tool for running the simulation several times with different seeds and plotting application layer packet delivery ration and throughput from the CSV output.

1. From the sample directory, run `marimo run run_plot_marimo.py`.
2. Enter the initial `rngstream` value, the step and the number of runs, then submit the form.
3. Before each run, the notebook updates `opt.rngstream` in `input_config.yaml`, keeping the order of sections and keys intact. After the last run, the final value stays in the file.

You'll need marimo, which can be installed as follows:

```
pip install marimo
```

## Developer guide

The YAML parser lives in a single Tcl file, `get-config.tcl`.
It lets a simulation keep its topology and behavior in Tcl while exposing frequently changed values in YAML.
The file uses the Tcllib `yaml` package (which is included in the tcl version used by DESERT), to read YAML files into Tcl dictionaries.

### Input side

Three procedures handle the inputs:

- **`load-input-config <file>`** reads `input_config.yaml`.
  - Parameters in the `opt` section are stored in the global `opt` array.
  - For every other section, the `tag` is read as the DESERT module name, and each remaining key is applied with `<tag> set <param> <value>`. This sets the class-level default, so it must run **before** the module objects are created.
  - A section without a `tag` stops the simulation with an error.
- **`resolve-input-expression <expression> <config>`** evaluates values prefixed with `expr:`.
  - Each `${SECTION.parameter}` reference is replaced with the corresponding numeric value from the same YAML file.
- **`load-input-positions <file>`** reads `input_positions.yaml` and stores each coordinate in the global `positions` array, indexed as `positions(<node_id>,<x|y|z>)`.

### Output side

The output side is built around a registry of the modules used in the simulation.
Three helpers fill a dictionary in the caller's scope, grouped by module class (found with `$rx info class`).
Each entry holds a receiver (`rx`) and, where relevant, a transmitter (`tx`):

| Helper | Use case | Dictionary key |
| --- | --- | --- |
| `write-input-node-based-modules` | Metrics for a single node | node ID |
| `write-input-link-based-modules` | Metrics for a link between two nodes | `rx_id,tx_id` |
| `write-sink-modules` | Links from each node to a common sink | `sink,tx_id` |

Once the registry is built, `print-metrics-csv <rngstream> <input_modules> <output_config>` does the rest:

1. `load-output-config` reads `output_config.yaml` and returns a dictionary mapping each module `tag` to its list of enabled metrics (`1`). Metrics set to `0` are dropped, and any other value is ignored with a message.
2. For each module that has both requested metrics and registered objects, `write-group-csv` appends rows to a CSV file named after the module, with `/` replaced by `_` (for example `Module_UW_CBR.csv`). Sink-based results go to a separate `*_sink.csv` file.
3. Each row starts with the `rngstream` value, followed by the node, link or sink identifier and the metric values.
4. If the file already exists, its header is compared with the requested metrics. If they differ, nothing is written and a message is printed. This is why an existing CSV must be renamed after changing the selected metrics.

`get-metrics` is where metrics are read from the modules.
It matches the module class and the metric name, then calls the corresponding getter.
A metric that is not recognized is written as `nan`.
Currently supported metrics are:

| Module | Metrics |
| --- | --- |
| `Module/UW/CBR` | `PDR`, `throughput`, `sent_packets`, `received_packets` |
| `Module/UW/TDMA` | `sent_packets`, `received_packets` |
| `Module/UW/CSMA_ALOHA` | `sent_packets`, `received_packets` |
| `Module/UW/PHYSICAL` | `packets_lost` |

### Adding a new module or metric

1. **New metric for an existing module:** add a case for it in the matching block of `get-metrics`.
2. **New module:** define a `MOD_<NAME>` variable at the top of the file, add a block for it in `get-metrics` with the getters you need, and register its objects with one of the `write-*-modules` helpers.
3. **New section in the YAML files:** give it a `tag` matching the DESERT module name. For inputs, every other key must be a valid parameter of that module. For outputs, every other key is a metric name set to `1` or `0`.
