from matplotlib import pyplot as plt
import pandas as pd
import numpy as np

def find_positions(agv_2=False):
    dt = 2
    fw_time = 3490 if agv_2 else 3500
    slow = 3000 if agv_2 else 2000
    n_loops = 10
    last_position = 0
    positions = []
    for t in np.arange(0, fw_time, dt):
        speed = float(np.tanh((t - 3.3*slow)/ slow) +1)
        last_position += speed * dt
        positions.append(last_position)
    back = positions[::-1]
    positions = positions + back
    positions = positions * n_loops
    return positions

agv_0 = find_positions()
agv_2 = find_positions(True)
agv_0 = agv_0[:len(agv_2)]
distance = np.array(agv_0) + np.array(agv_2)
print("Optical link is usable for " + str(np.mean(distance < 20)*100) + " % of the time")
plt.plot(np.arange(0,len(agv_0) * 2,2),agv_0)
plt.plot(np.arange(0,len(agv_2) * 2,2),agv_2)
plt.show()

waypoints = pd.DataFrame({
    "time": np.arange(0,len(agv_0) * 2,2),
    "x": np.array(agv_0),
    "y": np.ones(len(agv_0)),
    "z": np.full(len(agv_0), -100.0)
})

waypoints.to_csv("node_0_path.csv", index=False, header=False)

waypoints = pd.DataFrame({
    "time": np.arange(0,len(agv_2) * 2,2),
    "x": -np.array(agv_2),
    "y": -np.ones(len(agv_2)),
    "z": np.full(len(agv_2), -100.0)
})

waypoints.to_csv("node_2_path.csv", index=False, header=False)

node_0 = pd.read_csv("node_0_path.csv", header=None, names=["time", "x", "y", "z"])
node_2 = pd.read_csv("node_2_path.csv", header=None, names=["time", "x", "y", "z"])

fig, ax = plt.subplots(figsize=(12, 4))

ax.scatter(0, 1, color="blue", marker="o", s=100, edgecolor="black", label="Mobile Node 0")
ax.scatter(0, 0, color="white", marker="o", s=100, edgecolor="black", label="Static Node 1")
ax.scatter(0, -1, color="green", marker="o", s=100, edgecolor="black", label="Mobile Node 2")

ax.plot(
    node_0["x"],
    node_0["y"],
    # label="Mobile Node 0 Path",
    color="blue",
    alpha=0.7,
    linestyle="-",
    linewidth=2,
)
ax.annotate(
    "",
    xy=(node_0["x"].iloc[-1], node_0["y"].iloc[-1]),
    xytext=(node_0["x"].iloc[-2], node_0["y"].iloc[-2]),
    arrowprops=dict(arrowstyle="->", color="blue", lw=2),
)

ax.annotate(
    "",
    xy=(85.325, 1),
    xytext=(88.5, 1),
    arrowprops=dict(arrowstyle="<-", color="blue", lw=2),
)

ax.plot(
    node_2["x"],
    node_2["y"],
    # label="Mobile Node 2 Path",
    color="green",
    alpha=0.7,
    linestyle="-",
    linewidth=2,
)
ax.annotate(
    "",
    xy=(node_2["x"].iloc[-1], node_2["y"].iloc[-1]),
    xytext=(node_2["x"].iloc[-2], node_2["y"].iloc[-2]),
    arrowprops=dict(arrowstyle="->", color="green", lw=2),
)

ax.annotate(
    "",
    xy=(-37.4, -1),
    xytext=(-40.5, -1),
    arrowprops=dict(arrowstyle="<-", color="green", lw=2),
)

ax.set_ylim(-3, 3)
ax.set_yticks(np.arange(-3, 4, 1))
ax.set_xlabel("X [m]", fontsize=12, labelpad=10)
ax.set_ylabel("Y [m]", fontsize=12)
ax.grid(True, linestyle="--", alpha=0.6)
ax.legend(loc="lower right")
plt.tight_layout()
plt.show()