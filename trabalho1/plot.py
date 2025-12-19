import csv
import os
from collections import defaultdict
import matplotlib.pyplot as plt

RESULTS_PATH = os.path.join("results", "task_d.csv")
PLOTS_DIR = "plots"


def load_results(path):
    data = defaultdict(dict)
    with open(path, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            n = int(row["n"])
            threads = int(row["threads"])
            variant = row["variant"].strip()
            time = float(row["avg_time_sec"])
            data[n][threads] = data[n].get(threads, {})
            data[n][threads][variant] = time
    return data


def plot_times(data):
    os.makedirs(PLOTS_DIR, exist_ok=True)
    for n, by_threads in data.items():
        threads = sorted(by_threads.keys())
        naive = [by_threads[t].get("naive") for t in threads]
        tidy = [by_threads[t].get("tidy") for t in threads]

        plt.figure(figsize=(6, 4))
        plt.plot(threads, naive, marker="o", label="naive: 2 parallel for")
        plt.plot(threads, tidy, marker="s", label="arrumada: 1 parallel + 2 for")
        plt.xlabel("threads")
        plt.ylabel("tempo medio (s)")
        plt.title(f"Task D: tempo vs threads (N={n})")
        plt.grid(True, linestyle=":", linewidth=0.5)
        plt.legend()
        plt.tight_layout()
        plt.savefig(os.path.join(PLOTS_DIR, f"task_d_time_N{n}.png"), dpi=150)
        plt.close()

        ratios = []
        for t in threads:
            naive_t = by_threads[t].get("naive")
            tidy_t = by_threads[t].get("tidy")
            ratios.append(tidy_t / naive_t if naive_t else float("nan"))
        plt.figure(figsize=(6, 4))
        plt.plot(threads, ratios, marker="d", color="purple")
        plt.xlabel("threads")
        plt.ylabel("arrumada / ingenua")
        plt.title(f"Task D: razao de tempo (N={n})")
        plt.axhline(1.0, color="gray", linestyle="--", linewidth=1)
        plt.grid(True, linestyle=":", linewidth=0.5)
        plt.tight_layout()
        plt.savefig(os.path.join(PLOTS_DIR, f"task_d_ratio_N{n}.png"), dpi=150)
        plt.close()


def main():
    if not os.path.exists(RESULTS_PATH):
        raise SystemExit(f"Nao encontrei {RESULTS_PATH}; gere o CSV antes.")
    data = load_results(RESULTS_PATH)
    plot_times(data)
    print(f"Plots salvos em {PLOTS_DIR}/ (por N: tempo e razao arrumada/ingenua)")


if __name__ == "__main__":
    main()
