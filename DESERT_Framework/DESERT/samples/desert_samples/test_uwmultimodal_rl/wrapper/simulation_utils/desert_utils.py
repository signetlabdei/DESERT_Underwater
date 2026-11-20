import pandas as pd
from io import StringIO
from random import random
import subprocess
import os
from .const import *
import numpy as np

KEY_ORDER = [
    "rng_stream",
    "windspeed_mean",
    "windspeed_variance",
    "c_mean",
    "c_variance",
    "epsilon",
    "alpha",
    "first_run",
    "mode",
    "cbr_period_1",
    "pktsize_1",
    "cbr_period_2",
    "pktsize_2",
    "cbr_period_3",
    "pktsize_3",
    "expire_time",
    "queues_max_size",
    "probing_period",
    "mask_type",
    "range_based_distance",
    "campaign_path",
    "epoch",
    "agent_type",
    "eval_flag"
]

def init_simulation(config: dict, filename) -> str:
    """Initializes the simulation using a dictionary."""
    command = f"ns {filename} "
    for key in KEY_ORDER:
        command += f"{config[key]} "
    return command

def run_simulation(command:str, config) -> str:
    """Executes the simulation. Returning the log"""
    # Run the simulation
    actual_command = f'source {base_folder.joinpath("DESERT_buildCopy_LOCAL/environment").absolute()}; ' + command
    p = subprocess.Popen(actual_command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE, shell=True, executable='/bin/bash') # Security hazard
    out, err = p.communicate("")
    
    extention = "txt" if config["agent_type"] == "dsgsarsa" or config["agent_type"] == "dsgqlearning" else "pt"
    log_file_path = f"{config["campaign_path"]}/logs/log_{config["epoch"]}_0_1_1.csv"
    if not os.path.isfile(log_file_path):
        raise Exception("Diverging solution " + log_file_path + " not found")

    log_files = os.listdir(f"{config["campaign_path"]}/logs/")
    dfs = []
    for f in log_files:
        path = f"{config["campaign_path"]}/logs/{f}"
        df = pd.read_csv(path)
        df["Sender"] = int(f.split("_")[2])
        df["Receiver"] =  int(f.split("_")[3])
        df["Traffic Type"] = int(f.split("_")[4].removesuffix(".csv"))
        df.sort_values("Timestamp/Action Time", inplace = True)
        df["step"] = np.arange(len(df.index))
        dfs.append(df)
        Path(path).unlink()
    return pd.concat(dfs)
