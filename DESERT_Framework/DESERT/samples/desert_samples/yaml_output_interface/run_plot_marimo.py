# /// script
# dependencies = [
#     "marimo",
#     "matplotlib==3.10.9",
#     "numpy==2.4.6",
#     "polars==1.41.1",
# ]
# requires-python = ">=3.14"
# ///

import marimo

__generated_with = "0.23.4"
app = marimo.App(width="medium")


@app.cell(hide_code=True)
def _():
    import marimo as mo
    import os
    import yaml
    import subprocess
    import polars as pl
    import matplotlib.pyplot as plt

    return mo, os, pl, plt, subprocess, yaml


@app.cell(hide_code=True)
def _(os):
    input_config = "input_config.yaml"
    if not os.access(input_config, os.F_OK):
        raise ValueError("Input config file doesn't exists")

    tcl_file = "test_io_tdma_yaml.tcl"
    if not os.access(tcl_file, os.F_OK):
        raise ValueError("Simulation file doesn't exists")
    return input_config, tcl_file


@app.cell(hide_code=True)
def _(mo):
    simulation_form = (
        mo.md(
            """
            ### ⚙️ Simulation Setup
            Configure `rngstream` values and number of runs:

            * {start}
            * {step}
            * {count}
            """
        )
        .batch(
            start=mo.ui.number(start=1, stop=10000, value=1, label="Initial Value"),
            step=mo.ui.number(start=1, stop=1000, value=2, label="Step Size"),
            count=mo.ui.number(start=1, stop=100, value=5, label="Number of runs"),
        )
        .form(submit_button_label="Run simulations")
    )

    simulation_form
    return (simulation_form,)


@app.cell(hide_code=True)
def _(input_config, simulation_form, subprocess, tcl_file, yaml):
    if simulation_form.value is not None:
        inputs = simulation_form.value
        start_val = inputs["start"]
        step_val = inputs["step"]
        count_val = inputs["count"]

        rng_values = [start_val + (i * step_val) for i in range(count_val)]

        for current_rng in rng_values:
            # Update yaml file with current rng
            with open(input_config, "r") as file:
                data = yaml.safe_load(file) or {}

            data["opt"]["rngstream"] = current_rng

            with open(input_config, "w") as file:
                yaml.dump(data, file, default_flow_style=False)

            # Run simulation
            print(f"Running simulation with rngstream = {current_rng}...")
            result = subprocess.run(["ns", tcl_file], capture_output=True, text=True)
    return


@app.cell(hide_code=True)
def _(mo, pl, plt, simulation_form):
    if simulation_form.value is not None:
        df = pl.read_csv("Module_UW_CBR.csv")
        fig, ax = plt.subplots(1, 2, figsize=(10, 5))

        # pdr
        ax[0].boxplot(df["PDR"])
        ax[0].set_ylabel("PDR")
        ax[0].set_xticks([1])

        # throughput
        ax[1].boxplot(df["throughput"])
        ax[1].set_ylabel("Throughput [bps]")
        ax[1].set_xticks([1])

        # title and layout
        plt.suptitle("application layer performance")
        plt.tight_layout()

        # Embed the plot inside a markdown component using mo.as_html()
        plot_markdown = mo.md(
            f"""
            ### 📊 Simulation Results

            {mo.as_html(fig)}
            """
        )
    else:
        plot_markdown = None

    plot_markdown
    return (plot_markdown,)


if __name__ == "__main__":
    app.run()
