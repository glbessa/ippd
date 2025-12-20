import csv
import os
from collections import defaultdict

try:
	import matplotlib.pyplot as plt
except ImportError:  # pragma: no cover - mensagem amigável em ambiente sem matplotlib
	print("[ERRO] matplotlib não está instalado. Instale com 'pip install matplotlib' e rode novamente.")
	raise


RESULTS_CSV = os.path.join("results", "results.csv")
PLOTS_DIR = "plots"


def load_results(path: str):
	"""Lê o CSV gerado pelo run.sh e devolve uma lista de dicionários."""
	rows = []
	with open(path, newline="") as f:
		reader = csv.DictReader(f)
		for row in reader:
			# Conversão de tipos numéricos para facilitar o plot
			row["N"] = int(row["N"])
			row["threads"] = int(row["threads"])
			row["mean_total_s"] = float(row["mean_total_s"])
			row["std_total_s"] = float(row["std_total_s"])
			row["mean_kernel_s"] = float(row["mean_kernel_s"])
			row["std_kernel_s"] = float(row["std_kernel_s"])
			rows.append(row)
	return rows


def ensure_outdir():
	os.makedirs(PLOTS_DIR, exist_ok=True)


def plot_task_A(data):
	"""Tarefa A — impacto de schedule e chunk, escalabilidade por threads."""
	data_A = [r for r in data if r["task"] == "A"]
	if not data_A:
		return

	# Agrupar por (N, K)
	by_NK = defaultdict(list)
	for r in data_A:
		key = (r["N"], r["param_value"])  # param_name == "K"
		by_NK[key].append(r)

	for (N, K), rows in by_NK.items():
		variants = sorted({r["variant"] for r in rows})

		# Gráfico 1: tempo total vs threads
		fig, ax = plt.subplots(figsize=(8, 5))
		for variant in variants:
			subset = [r for r in rows if r["variant"] == variant]
			subset = sorted(subset, key=lambda r: r["threads"])
			threads = [r["threads"] for r in subset]
			means = [r["mean_total_s"] for r in subset]
			errs = [r["std_total_s"] for r in subset]
			if not threads:
				continue
			ax.errorbar(
				threads,
				means,
				yerr=errs,
				marker="o",
				label=variant,
				capsize=3,
			)
		ax.set_title(f"Tarefa A – N={N}, K={K}: tempo total vs threads")
		ax.set_xlabel("Threads")
		ax.set_ylabel("Tempo total (s)")
		ax.grid(True, linestyle=":", alpha=0.5)
		ax.legend(title="Variante (schedule)")
		fig.tight_layout()
		fig.savefig(os.path.join(PLOTS_DIR, f"taskA_N{N}_K{K}_total.png"))
		plt.close(fig)

		# Gráfico 2: tempo de kernel vs threads
		fig_k, ax_k = plt.subplots(figsize=(8, 5))
		for variant in variants:
			subset = [r for r in rows if r["variant"] == variant]
			subset = sorted(subset, key=lambda r: r["threads"])
			threads = [r["threads"] for r in subset]
			means = [r["mean_kernel_s"] for r in subset]
			errs = [r["std_kernel_s"] for r in subset]
			if not threads:
				continue
			ax_k.errorbar(
				threads,
				means,
				yerr=errs,
				marker="o",
				label=variant,
				capsize=3,
			)
		ax_k.set_title(f"Tarefa A – N={N}, K={K}: tempo de kernel vs threads")
		ax_k.set_xlabel("Threads")
		ax_k.set_ylabel("Tempo de kernel (s)")
		ax_k.grid(True, linestyle=":", alpha=0.5)
		ax_k.legend(title="Variante (schedule)")
		fig_k.tight_layout()
		fig_k.savefig(os.path.join(PLOTS_DIR, f"taskA_N{N}_K{K}_kernel.png"))
		plt.close(fig_k)

		# Gráfico 3: speedup vs threads (base: threads=1 por variante)
		fig_s, ax_s = plt.subplots(figsize=(8, 5))
		for variant in variants:
			subset = [r for r in rows if r["variant"] == variant]
			base_rows = [r for r in subset if r["threads"] == 1]
			if not base_rows:
				continue
			base_time = base_rows[0]["mean_total_s"]
			subset = sorted(subset, key=lambda r: r["threads"])
			threads = [r["threads"] for r in subset]
			speedups = [base_time / r["mean_total_s"] if r["mean_total_s"] > 0 else 0.0 for r in subset]
			ax_s.plot(threads, speedups, marker="o", label=variant)
		ax_s.set_title(f"Tarefa A – N={N}, K={K}: speedup vs threads")
		ax_s.set_xlabel("Threads")
		ax_s.set_ylabel("Speedup (T1 / Tt)")
		ax_s.grid(True, linestyle=":", alpha=0.5)
		ax_s.legend(title="Variante (schedule)")
		fig_s.tight_layout()
		fig_s.savefig(os.path.join(PLOTS_DIR, f"taskA_N{N}_K{K}_speedup.png"))
		plt.close(fig_s)


def plot_task_B(data):
	"""Tarefa B — critical vs atomic vs local, por B e threads."""
	data_B = [r for r in data if r["task"] == "B"]
	if not data_B:
		return

	# Agrupar por (N, B)
	by_NB = defaultdict(list)
	for r in data_B:
		key = (r["N"], r["param_value"])  # param_name == "B"
		by_NB[key].append(r)

	for (N, B), rows in by_NB.items():
		variants = sorted({r["variant"] for r in rows})

		# Gráfico 1: tempo total vs threads
		fig, ax = plt.subplots(figsize=(8, 5))
		for variant in variants:
			subset = [r for r in rows if r["variant"] == variant]
			subset = sorted(subset, key=lambda r: r["threads"])
			threads = [r["threads"] for r in subset]
			means = [r["mean_total_s"] for r in subset]
			errs = [r["std_total_s"] for r in subset]
			if not threads:
				continue
			ax.errorbar(
				threads,
				means,
				yerr=errs,
				marker="o",
				label=variant,
				capsize=3,
			)
		ax.set_title(f"Tarefa B – N={N}, B={B}: tempo total vs threads")
		ax.set_xlabel("Threads")
		ax.set_ylabel("Tempo total (s)")
		ax.grid(True, linestyle=":", alpha=0.5)
		ax.legend(title="Variante (contenção)")
		fig.tight_layout()
		fig.savefig(os.path.join(PLOTS_DIR, f"taskB_N{N}_B{B}_total.png"))
		plt.close(fig)

		# Gráfico 2: tempo de kernel vs threads
		fig_k, ax_k = plt.subplots(figsize=(8, 5))
		for variant in variants:
			subset = [r for r in rows if r["variant"] == variant]
			subset = sorted(subset, key=lambda r: r["threads"])
			threads = [r["threads"] for r in subset]
			means = [r["mean_kernel_s"] for r in subset]
			errs = [r["std_kernel_s"] for r in subset]
			if not threads:
				continue
			ax_k.errorbar(
				threads,
				means,
				yerr=errs,
				marker="o",
				label=variant,
				capsize=3,
			)
		ax_k.set_title(f"Tarefa B – N={N}, B={B}: tempo de kernel vs threads")
		ax_k.set_xlabel("Threads")
		ax_k.set_ylabel("Tempo de kernel (s)")
		ax_k.grid(True, linestyle=":", alpha=0.5)
		ax_k.legend(title="Variante (contenção)")
		fig_k.tight_layout()
		fig_k.savefig(os.path.join(PLOTS_DIR, f"taskB_N{N}_B{B}_kernel.png"))
		plt.close(fig_k)

		# Gráfico 3: speedup vs threads (base: threads=1 por variante)
		fig_s, ax_s = plt.subplots(figsize=(8, 5))
		for variant in variants:
			subset = [r for r in rows if r["variant"] == variant]
			base_rows = [r for r in subset if r["threads"] == 1]
			if not base_rows:
				continue
			base_time = base_rows[0]["mean_total_s"]
			subset = sorted(subset, key=lambda r: r["threads"])
			threads = [r["threads"] for r in subset]
			speedups = [base_time / r["mean_total_s"] if r["mean_total_s"] > 0 else 0.0 for r in subset]
			ax_s.plot(threads, speedups, marker="o", label=variant)
		ax_s.set_title(f"Tarefa B – N={N}, B={B}: speedup vs threads")
		ax_s.set_xlabel("Threads")
		ax_s.set_ylabel("Speedup (T1 / Tt)")
		ax_s.grid(True, linestyle=":", alpha=0.5)
		ax_s.legend(title="Variante (contenção)")
		fig_s.tight_layout()
		fig_s.savefig(os.path.join(PLOTS_DIR, f"taskB_N{N}_B{B}_speedup.png"))
		plt.close(fig_s)


def plot_task_C(data):
	"""Tarefa C — ganho de SIMD sobre versão base (seq)."""
	data_C = [r for r in data if r["task"] == "C"]
	if not data_C:
		return

	# Agrupar por (N, K)
	by_NK = defaultdict(list)
	for r in data_C:
		key = (r["N"], r["param_value"])  # param_name == "K"
		by_NK[key].append(r)

	for (N, K), rows in by_NK.items():
		# Encontrar tempos da versão seq (threads == 1, variant == seq)
		base_rows = [r for r in rows if r["variant"] == "seq" and r["threads"] == 1]
		if not base_rows:
			continue
		base_time = base_rows[0]["mean_total_s"]

		# Variantes para comparar: seq, simd, parallel_simd (em threads "típicos")
		# Vamos usar threads=1 para seq e simd, e threads=max para parallel_simd.
		variants = ["seq", "simd", "parallel_simd"]
		labels = ["seq", "simd", "parallel simd"]
		mean_times = []
		std_times = []
		speedups = []
		for v in variants:
			cand = [r for r in rows if r["variant"] == v]
			if not cand:
				mean_times.append(None)
				std_times.append(0.0)
				speedups.append(None)
				continue
			# escolha simples: menor número de threads disponível para cada variante
			cand = sorted(cand, key=lambda r: r["threads"])
			row = cand[0]
			mean = row["mean_total_s"]
			std = row["std_total_s"]
			mean_times.append(mean)
			std_times.append(std)
			speedups.append(base_time / mean if mean > 0 else None)

		# Gráfico 1: tempos absolutos por variante
		x = list(range(len(variants)))
		fig1, ax1 = plt.subplots(figsize=(6, 4))
		ax1.bar(x, mean_times, yerr=std_times, capsize=3)
		ax1.set_xticks(x)
		ax1.set_xticklabels(labels)
		ax1.set_ylabel("Tempo total (s)")
		ax1.set_title(f"Tarefa C – N={N}, K={K}: tempo por variante")
		ax1.grid(True, axis="y", linestyle=":", alpha=0.5)
		fig1.tight_layout()
		fname1 = os.path.join(PLOTS_DIR, f"taskC_N{N}_K{K}_tempos.png")
		fig1.savefig(fname1)
		plt.close(fig1)

		# Gráfico 2: speedup em relação à versão seq
		fig2, ax2 = plt.subplots(figsize=(6, 4))
		ax2.bar(x, speedups)
		ax2.set_xticks(x)
		ax2.set_xticklabels(labels)
		ax2.set_ylabel("Speedup vs seq")
		ax2.set_title(f"Tarefa C – N={N}, K={K}: ganho de SIMD")
		ax2.grid(True, axis="y", linestyle=":", alpha=0.5)
		fig2.tight_layout()
		fname2 = os.path.join(PLOTS_DIR, f"taskC_N{N}_K{K}_speedup.png")
		fig2.savefig(fname2)
		plt.close(fig2)


def plot_task_D(data):
	"""Tarefa D — comparação de overhead de organização da região paralela."""
	data_D = [r for r in data if r["task"] == "D"]
	if not data_D:
		return

	# Agrupar por N (não há parâmetro K/B)
	by_N = defaultdict(list)
	for r in data_D:
		key = r["N"]
		by_N[key].append(r)

	for N, rows in by_N.items():
		variants = sorted({r["variant"] for r in rows})
		# Duas subplots: total e kernel
		fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(8, 7), sharex=True)

		for variant in variants:
			subset = [r for r in rows if r["variant"] == variant]
			subset = sorted(subset, key=lambda r: r["threads"])
			threads = [r["threads"] for r in subset]
			mean_total = [r["mean_total_s"] for r in subset]
			std_total = [r["std_total_s"] for r in subset]
			mean_kernel = [r["mean_kernel_s"] for r in subset]
			std_kernel = [r["std_kernel_s"] for r in subset]
			if not threads:
				continue

			ax1.errorbar(
				threads,
				mean_total,
				yerr=std_total,
				marker="o",
				label=variant,
				capsize=3,
			)
			ax2.errorbar(
				threads,
				mean_kernel,
				yerr=std_kernel,
				marker="o",
				label=variant,
				capsize=3,
			)

		ax1.set_ylabel("Tempo total (s)")
		ax1.set_title(f"Tarefa D – N={N}: organização da região paralela")
		ax1.grid(True, linestyle=":", alpha=0.5)
		ax1.legend(title="Variante")

		ax2.set_xlabel("Threads")
		ax2.set_ylabel("Tempo do kernel (s)")
		ax2.grid(True, linestyle=":", alpha=0.5)

		fig.tight_layout()
		fig.savefig(os.path.join(PLOTS_DIR, f"taskD_N{N}_overhead.png"))
		plt.close(fig)

		# Gráfico adicional: speedup vs threads (base: threads=1 por variante)
		fig_s, ax_s = plt.subplots(figsize=(8, 5))
		for variant in variants:
			subset = [r for r in rows if r["variant"] == variant]
			base_rows = [r for r in subset if r["threads"] == 1]
			if not base_rows:
				continue
			base_time = base_rows[0]["mean_total_s"]
			subset = sorted(subset, key=lambda r: r["threads"])
			threads = [r["threads"] for r in subset]
			speedups = [base_time / r["mean_total_s"] if r["mean_total_s"] > 0 else 0.0 for r in subset]
			ax_s.plot(threads, speedups, marker="o", label=variant)
		ax_s.set_title(f"Tarefa D – N={N}: speedup vs threads")
		ax_s.set_xlabel("Threads")
		ax_s.set_ylabel("Speedup (T1 / Tt)")
		ax_s.grid(True, linestyle=":", alpha=0.5)
		ax_s.legend(title="Variante")
		fig_s.tight_layout()
		fig_s.savefig(os.path.join(PLOTS_DIR, f"taskD_N{N}_speedup.png"))
		plt.close(fig_s)


def main():
	if not os.path.exists(RESULTS_CSV):
		print(f"[ERRO] Arquivo de resultados não encontrado: {RESULTS_CSV}")
		print("Execute primeiro ./run.sh para gerar o CSV.")
		return

	ensure_outdir()
	data = load_results(RESULTS_CSV)

	plot_task_A(data)
	plot_task_B(data)
	plot_task_C(data)
	plot_task_D(data)

	print(f"Gráficos gerados em '{PLOTS_DIR}/'. Inclua-os em RESULTADOS.md na sua análise.")


if __name__ == "__main__":
	main()
