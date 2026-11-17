import simulation_utils as sim
from pathlib import Path
import numpy as np
import argparse
import shutil
import json
import os

default_config = {

    "rng_stream": 0,
    "windspeed_mean": 10.0,
    "windspeed_variance": 0.3,
    "c_mean": 0.30,
    "c_variance": 0.01,
    "alpha": 0.0001,
    "decaying_rate": 0.995,
    "mode": 2,
    "cbr_period_1": 10,
    "pktsize_1": 64,
    "cbr_period_2": 1,
    "pktsize_2": 12500,
    "cbr_period_3": 60,
    "pktsize_3": 64,
    "expire_time": 10,
    "queues_max_size": 1000,
    "probing_period": 1.0,
    "mask_type": 0,
    "range_based_distance": 0,
    "campaign_path": "",
    "epoch": 0,
    "agent_type": "dsgsarsa",
}

folder_base_name = "campaign"

def run(rng_seed=None, resume=False):

    resume_num = 0
    evaluate_counter_start = 0

    # Training and evaluation epochs
    training_epochs = 20
    evaluation_period = 4
    evaluation_epochs = 4

    # Set the output folder path
    out_folder = Path("out/")

    config = default_config.copy()

    if resume:
        parser = argparse.ArgumentParser(description='Resume a campaign')
        parser.add_argument('--campaign_folder', default='', type=str, help='Campaign folder')
        args = parser.parse_args()

        campaign_folder = Path(args.campaign_folder)
        training_epochs = 2000

        config = json.load(open(campaign_folder.joinpath("config.json"), "r"))
        files_old = os.listdir(campaign_folder)
        max_resumable = len(files_old)//3 + 1
        processed = [0,] * max_resumable

        for f in files_old:
            if f.endswith(".pth") and not f.startswith("garbage"):
                ep = int(f.split("_")[0])
                processed[ep] += 1
            if f.startswith("evaluate_data_"):
                evaluate_counter_start = max(evaluate_counter_start, int(f.split("_")[-1].split(".")[0]) + 1)
        for e, i in enumerate(processed):
            if i < 3:
                resume_num = e
                break

        config["train_agent_1"] = 1
        config["train_agent_2"] = 1
        config["train_agent_3"] = 1

        id_camp = int(campaign_folder.name.split("_")[-1])
        out_folder = campaign_folder.parent
        print(f"Resuming campaign {id_camp} from epoch {resume_num}")
    else:
        if config["mode"] == 0:
            out_folder = out_folder.joinpath("ground_truth/ac/")
        elif config["mode"] == 1:
            out_folder = out_folder.joinpath("ground_truth/op/")
        elif config["mode"] == 2:
            out_folder = out_folder.joinpath("deleteme/")
        elif config["mode"] == 3:
            out_folder = out_folder.joinpath("baseline/distance/")
        elif config["mode"] == 4:
            out_folder = out_folder.joinpath("baseline/probing/")
        os.makedirs(out_folder, exist_ok=True)
        id_camp = len(os.listdir(out_folder))
        campaign_folder = out_folder.joinpath(f"{folder_base_name}_{id_camp}/")
        os.mkdir(campaign_folder)
        os.mkdir(campaign_folder.joinpath("logs"))
        os.mkdir(campaign_folder.joinpath("models"))
        print(f"Running campaign {id_camp}")
        config["campaign_path"] = str(campaign_folder.absolute())

    
    if rng_seed is not None:
        np.random.seed(rng_seed)
        config["rng_stream"] = rng_seed
    else:
        np.random.seed(config["rng_stream"])
    
    if config["mode"] == 2:
        evaluation_seeds = np.random.randint(0, 2**16, evaluation_epochs)
        train_seeds = np.random.randint(0, 2**16, training_epochs)
    else:
        evaluation_seeds = np.random.randint(0, 2**16, evaluation_epochs)

    with open(campaign_folder.joinpath("config.json"), "w") as f:
        json.dump(config, f, indent=4)

    shutil.copy(sim.base_folder.joinpath("DESERT_Addons/uwmultimodal_rl/uwmulti-traffic-rl.cpp"), campaign_folder.joinpath("uwmulti-traffic-rl.cpp"))

    epsilon_func = lambda ep: 0.05 + (0.95 * np.exp(- ep / config["decaying_rate"]))

    assert config["agent_type"] in ["ddqn", "ppo", "dsgsarsa", "dsgqlearning"]

    if config["mode"] == 2:
        results = sim.train(config,
                            training_epochs,
                            train_seeds,
                            evaluation_period, 
                            evaluation_seeds,
                            epsilon_func, 
                            campaign_folder,
                            resume=resume_num,
                            evaluate_counter_start=evaluate_counter_start)
    else:
        results = sim.evaluate(config,
                               evaluation_epochs,
                               evaluation_seeds,
                               campaign_folder=campaign_folder)

if __name__ == "__main__":

    if "--campaign-folder" in os.sys.argv:
        run(resume=True)
    elif "--rng-seed" in os.sys.argv:
        run(rng_seed=int(os.sys.argv[2]))
    else:
        for arg in os.sys.argv:
            if arg.startswith("--"):
                if arg.replace("--", "") in default_config:
                    type_arg = type(default_config[arg.replace("--", "")])
                    default_config[arg.replace("--", "")] = type_arg (os.sys.argv[os.sys.argv.index(arg) + 1])
                else:
                    raise ValueError(f"Unknown argument {arg}")
        run()
        