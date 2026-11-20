from .desert_utils import *
from tqdm import trange
import tqdm
import numpy as np
from pathlib import Path
import pandas as pd
import json

def log_on_wandb(csv_path: Path):
    logged = evaluate_performance_metrics(csv_path)
    logged = logged.mean()
    logged = logged.to_dict()
    wandb.log(logged, commit=True)

def log_on_file(csv_path: Path, out_path: Path):
    logged = evaluate_performance_metrics(csv_path)
    logged = logged.mean()
    logged = logged.to_dict()
    with open(out_path, "w") as f:
        json.dump(logged, f, indent=4)


def evaluate_performance_metrics(csv_path):
    outs = []
    df = pd.read_csv(csv_path)
    seeds = df["evaluate id"].unique()
    for seed in seeds:
        agent_delay = df.loc[df["Traffic Type"] == 1, :]
        agent_delay_received = agent_delay[agent_delay["Received"] == True]
        avg_delay = agent_delay_received["Delay"].sum() / len(agent_delay_received.index)

        agent_delay_not_waited = agent_delay[agent_delay["Wait"] == 0]
        agent_delay_not_waited["Instantaneous Throughput"] = agent_delay_not_waited["Received"] * agent_delay_not_waited["Packet Size"] / agent_delay_not_waited["Delay"]
        average_istantaneous_throughput = agent_delay_not_waited["Instantaneous Throughput"].mean()

        agent_jitter = df.loc[df["Traffic Type"] == 2, :]
        jitter_delay_received = agent_jitter[agent_jitter["Received"] == True]
        delay_diff = jitter_delay_received["Delay"].diff().abs()
        avg_jitter = delay_diff.mean()

        agent_reliability = df.loc[df["Traffic Type"] == 3, :]
        avg_prr = agent_reliability["Received"].mean()
        out = {"Average Delay": avg_delay, "Average Instantaneous Throughput": average_istantaneous_throughput, "Average Jitter": avg_jitter, "Average PRR": avg_prr, "Seed": seed}
        outs.append(out)
    logged = pd.DataFrame(outs)
    return logged

def evaluate_epoch(config:dict, campaign_folder:Path, epoch, seeds=[0]):
    """Evaluates the simulation using a dictionary."""

    config["eval_flag"] = 1

    results_rl_dfs = []

    linspace_num = np.sqrt(seeds.size)
    assert linspace_num.is_integer(), "The number of seeds must be a square number."
    linspace_num = int(linspace_num)

    wind_means = np.linspace(5, 15, linspace_num)
    c_means = np.linspace(0.2, 0.35, linspace_num)

    combinations = np.array(np.meshgrid(wind_means, c_means)).T.reshape(-1, 2)

    pbar = tqdm.tqdm(total=len(seeds), desc=f"Evaluating epoch {epoch}", leave=False)

    for i, seed in enumerate(seeds):
        pbar.update(1)
        config["rng_stream"] = seed
        config["in_model_filename_1"] = campaign_folder.joinpath(f"{epoch}_1.pth")
        config["out_model_filename_1"] = campaign_folder.joinpath(f"garbage_1.pth")
        config["in_model_filename_2"] = campaign_folder.joinpath(f"{epoch}_2.pth")
        config["out_model_filename_2"] = campaign_folder.joinpath(f"garbage_2.pth")
        config["in_model_filename_3"] = campaign_folder.joinpath(f"{epoch}_3.pth")
        config["out_model_filename_3"] = campaign_folder.joinpath(f"garbage_3.pth")
        config["epsilon"] = 0.0
        comb = combinations[i]
        config["windspeed_mean"] = comb[0]
        config["c_mean"] = comb[1]

        command = init_simulation(config, "../test_uwmultimodal_rl.tcl")
        results_rl = run_simulation(command, config)

        results_rl["seed"] = seed
        results_rl["evaluate id"] = i
        results_rl_dfs.append(results_rl)
       
    results_rl_df = pd.concat(results_rl_dfs)
    
    results = results_rl_df
    results["epoch"] = epoch
    return results

def train_epoch(config, campaign_folder:Path, epoch):
    """Trains the simulation using a dictionary."""

    config["eval_flag"] = 0
    config["in_model_filename_1"] = campaign_folder.joinpath(f"{epoch-1}_1.pth")
    config["out_model_filename_1"] = campaign_folder.joinpath(f"{epoch}_1.pth")
    config["in_model_filename_2"] = campaign_folder.joinpath(f"{epoch-1}_2.pth")
    config["out_model_filename_2"] = campaign_folder.joinpath(f"{epoch}_2.pth")
    config["in_model_filename_3"] = campaign_folder.joinpath(f"{epoch-1}_3.pth")
    config["out_model_filename_3"] = campaign_folder.joinpath(f"{epoch}_3.pth")
    command = init_simulation(config, "../test_uwmultimodal_rl.tcl")
    results = run_simulation(command, config)
    results["epoch"] = epoch
    results["seed"] = config["rng_stream"]
    results["epsilon"] = config["epsilon"]
    results["alpha"] = config["alpha"]
    return results

def train(config: dict,
          epochs: int,
          train_seeds: np.ndarray,
          evaluate_freq: int,
          evaluate_seeds:np.ndarray,
          epsilon: callable,
          campaign_folder = Path("."),
          resume = 0,
          evaluate_counter_start = 0):
    
    """Trains the simulation using a dictionary."""
    assert len(train_seeds) == epochs

    train_data_path = campaign_folder.joinpath("train_data.csv")
    evaluate_data_path = campaign_folder.joinpath("evaluate_data.csv")

    evaluate_counter = 0
    starting_alpha = config["alpha"]
    for epoch in trange(epochs):
        config["alpha"] = starting_alpha * (config["decaying_rate"]**epoch)
        config["epsilon"] = epsilon(epoch)
        config["rng_stream"] = train_seeds[epoch]

        linspace_num = int(np.floor(np.sqrt(len(evaluate_seeds))))
        config["windspeed_mean"] = np.random.choice(np.linspace(5, 15, linspace_num))
        config["c_mean"] = np.random.choice(np.linspace(0.2, 0.35, linspace_num))

        if epoch == 0:
            config["first_run"] = 1
        else:
            config["first_run"] = 0

        config["epoch"] = epoch

        if epoch < resume:
            continue
        df = train_epoch(config, campaign_folder, epoch)
        # df.to_csv(train_data_path, mode='a', header=not train_data_path.exists(), index=False) # DONEGA: uncomment if you want to save training data
        # save_results(df, train_data_path, train=True) # DONEGA: uncomment if you want to save some training data

        if epoch % evaluate_freq == 0 and epoch != 0:
            df = evaluate_epoch(config, campaign_folder, epoch, seeds=evaluate_seeds)
            evaluate_data_path = campaign_folder.joinpath(f"evaluate_data_{evaluate_counter}.csv")
            df = save_results(df, evaluate_data_path)
            log_on_file(evaluate_data_path, campaign_folder.joinpath(f"evaluate_data_{evaluate_counter}.json"))
            evaluate_counter += 1

    if epoch % evaluate_freq != 0:
        df = evaluate_epoch(config, campaign_folder, epoch, seeds=evaluate_seeds)
        evaluate_data_path = campaign_folder.joinpath(f"evaluate_data_{evaluate_counter}.csv")
        df = save_results(df, evaluate_data_path)
        log_on_file(evaluate_data_path, campaign_folder.joinpath(f"evaluate_data_{evaluate_counter}.json"))
        evaluate_counter += 1
    return(df)


def evaluate(config: dict,
             epochs: int,
             evaluation_seeds: np.ndarray,
             campaign_folder: Path):
    """Trains the simulation using a dictionary."""
    evaluate_data_path = campaign_folder.joinpath("evaluate_data.csv")
    config["first_run"] = 1
    df = evaluate_epoch(config, campaign_folder, 0, evaluation_seeds)
    df.sort_values("Timestamp/Action Time", inplace = True)
    df = save_results(df, evaluate_data_path)
    log_on_file(evaluate_data_path, campaign_folder.joinpath(f"evaluate_data_0.json"))
    


def save_results(results: pd.DataFrame, filename: str, train = False):
    """Saves partial results to a file."""
    useful_columns = ["Sender", "Receiver", "Traffic Type", "State/Distance", 
                      "State/Packet Size", "Action/Acoustic", "Action/Optical", 
                      "Action/Wait", "Action/Drop", "Reward/Reward", "Reward/Delay",
                      "Reward/Received", "Timestamp/Action Time", 
                      "Timestamp/Generation Time", "Packet_uid/Packet_uid", 
                      "Timestamp/Rx Time", "epoch", "seed"]
    
    if train:
        useful_columns.append("epsilon")
        useful_columns.append("alpha")
    else:
        useful_columns.append("evaluate id")
    
    partial_df = results.loc[:,useful_columns]

    partial_df.rename(columns={"State/Distance":"Distance",
                               "State/Packet Size":"Packet Size",
                               "Action/Acoustic":"Acoustic",
                               "Action/Optical":"Optical",
                               "Action/Wait":"Wait",
                               "Action/Drop":"Drop",
                               "Reward/Reward":"Reward",
                               "Reward/Delay":"Delay",
                               "Reward/Received":"Received",
                               "Timestamp/Action Time":"Action Time",
                               "Packet_uid/Packet_uid":"Packet UID",
                               "Timestamp/Rx Time":"Rx Time"}, inplace=True)
    
    received_bool = partial_df["Received"] == 1
    
    partial_df.loc[:, "Received"] = received_bool.astype(bool)
    
    if not Path(filename).exists():
        partial_df.to_csv(filename, mode='w', header=True, index=False)
    else:
        partial_df.to_csv(filename, mode='a', header=False, index=False)
    return partial_df