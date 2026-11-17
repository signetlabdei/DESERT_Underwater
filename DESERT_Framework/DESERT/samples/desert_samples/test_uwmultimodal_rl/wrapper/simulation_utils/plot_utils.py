import seaborn as sns
import matplotlib.pyplot as plt

def setup():
    sns.set_theme(style = "whitegrid")
    sns.set_palette("colorblind")

def plot_results(results):
    sns.lineplot(data = results, x = "epoch", y = "Reward/Reward", label = "Reward")
    plt.show()