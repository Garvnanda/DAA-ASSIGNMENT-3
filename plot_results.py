"""Draws all graphs for the DAA assignment from the CSV files written by the C++ programs.
Usage:  python plot_results.py <folder with results_*.csv> <output folder>
Needs:  matplotlib (open source)
"""
import csv, math, os, sys
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

src = sys.argv[1] if len(sys.argv) > 1 else "."
out = sys.argv[2] if len(sys.argv) > 2 else "graphs"
os.makedirs(out, exist_ok=True)

BLUE, ORANGE, AQUA, INK, MUTED = "#2a78d6", "#eb6834", "#1baf7a", "#0b0b0b", "#8a8984"
STYLES = [dict(color=BLUE, marker="o", ls="-"), dict(color=ORANGE, marker="s", ls="--"),
          dict(color=AQUA, marker="^", ls="-.")]

plt.rcParams.update({
    "font.family": "DejaVu Sans", "font.size": 9, "axes.edgecolor": MUTED, "axes.linewidth": 0.8,
    "axes.grid": True, "grid.color": "#e4e3df", "grid.linewidth": 0.6, "axes.spines.top": False,
    "axes.spines.right": False, "xtick.color": "#52514e", "ytick.color": "#52514e",
    "axes.labelcolor": INK, "axes.titlesize": 10, "axes.titleweight": "bold", "legend.frameon": False,
    "lines.linewidth": 2, "lines.markersize": 6,
})


def load(name):
    rows = {}
    with open(os.path.join(src, name)) as f:
        for r in csv.DictReader(f):
            rows.setdefault(r["case"], []).append((int(r["n"]), float(r["avg_ms"])))
    return rows


def fit_curve(points, f):
    """least-squares constant c so that time ~ c * f(n)"""
    c = sum(t * f(n) for n, t in points) / sum(f(n) ** 2 for n, t in points)
    return c


def plot_cases(ax, rows, cases, labels, theory=None, theory_label=None, theory_case=None):
    for i, (c, lab) in enumerate(zip(cases, labels)):
        xs, ys = zip(*rows[c])
        ax.plot(xs, ys, label=lab, markeredgecolor="white", markeredgewidth=1, **STYLES[i])
    if theory:
        pts = rows[theory_case]
        c = fit_curve(pts, theory)
        lo, hi = pts[0][0], pts[-1][0]
        xs = [lo + (hi - lo) * k / 100 for k in range(101)]
        ax.plot(xs, [c * theory(x) for x in xs], color=MUTED, lw=1.2, ls=":", label=theory_label)
    ax.set_ylim(bottom=0)
    ax.legend(loc="upper left")


def save(fig, name):
    fig.tight_layout()
    fig.savefig(os.path.join(out, name), dpi=220)
    plt.close(fig)


nlogn = lambda n: n * math.log2(n)
KFMT = matplotlib.ticker.FuncFormatter(lambda x, p: f"{x/1000:g}k" if x else "0")

# 1. Insertion sort
r = load("results_insertion.csv")
fig, ax = plt.subplots(figsize=(6.4, 3.6))
plot_cases(ax, r, ["random", "sorted", "reverse"],
           ["Random input (average case)", "Sorted input (best case)", "Reverse-sorted input (worst case)"],
           lambda n: n * n, "Fitted c·n² (random)", "random")
ax.set(title="Insertion Sort: running time vs n", xlabel="Input size n", ylabel="Time (ms)")
save(fig, "fig1_insertion.png")

# 2. Merge sort
r = load("results_merge.csv")
fig, ax = plt.subplots(figsize=(6.4, 3.6))
plot_cases(ax, r, ["random", "sorted", "reverse"],
           ["Random input", "Sorted input", "Reverse-sorted input"],
           nlogn, "Fitted c·n log n (random)", "random")
ax.set(title="Merge Sort: running time vs n", xlabel="Input size n", ylabel="Time (ms)")
ax.xaxis.set_major_formatter(KFMT)
save(fig, "fig2_merge.png")

# 3. Quick sort (two panels: average case, and average vs worst on the same n)
r = load("results_quick.csv")
fig, (a1, a2) = plt.subplots(1, 2, figsize=(6.8, 3.4))
plot_cases(a1, r, ["random"], ["Random input (average case)"], nlogn, "Fitted c·n log n", "random")
a1.set(title="Average case (random input)", xlabel="Input size n", ylabel="Time (ms)")
small = {"random": [p for p in r["random"] if p[0] <= 16000], "sorted": r["sorted"]}
plot_cases(a2, small, ["random", "sorted"], ["Random input", "Sorted input (worst case)"],
           lambda n: n * n, "Fitted c·n² (sorted)", "sorted")
a2.set(title="Average vs worst case", xlabel="Input size n", ylabel="Time (ms)")
a1.xaxis.set_major_formatter(KFMT)
a2.xaxis.set_major_formatter(KFMT)
fig.suptitle("Quick Sort (last element as pivot): running time vs n", fontweight="bold", fontsize=10)
save(fig, "fig3_quick.png")

# 4. Floyd-Warshall
r = load("results_floyd.csv")
case = list(r.keys())[0]
fig, ax = plt.subplots(figsize=(6.4, 3.6))
plot_cases(ax, r, [case], ["Dense random graph (adjacency matrix)"], lambda n: n ** 3, "Fitted c·V³", case)
ax.set(title="Floyd-Warshall: running time vs number of vertices V", xlabel="Number of vertices V",
       ylabel="Time (ms)")
save(fig, "fig4_floyd.png")

# 5. KMP
r = load("results_kmp.csv")
fig, ax = plt.subplots(figsize=(6.4, 3.6))
plot_cases(ax, r, ["random text", "worst-case text"],
           ["Random text, m = 8", "Text aaa...a, pattern aaa...ab, m = 1000"],
           lambda n: n, "Fitted c·n (random text)", "random text")
ax.set(title="KMP string matching: running time vs text length n", xlabel="Text length n (characters)",
       ylabel="Time (ms)")
ax.xaxis.set_major_formatter(matplotlib.ticker.FuncFormatter(lambda x, p: f"{x/1e6:g}M"))
save(fig, "fig5_kmp.png")

# 6. Comparison: Merge vs Quick (random input)
m, q = load("results_merge.csv")["random"], load("results_quick.csv")["random"]
fig, ax = plt.subplots(figsize=(6.4, 3.6))
plot_cases(ax, {"m": m, "q": q}, ["m", "q"], ["Merge Sort", "Quick Sort"])
ax.set(title="Comparison: Merge Sort vs Quick Sort (random input)", xlabel="Input size n", ylabel="Time (ms)")
ax.xaxis.set_major_formatter(KFMT)
save(fig, "fig6_merge_vs_quick.png")

# 7. Comparison on small n: insertion vs merge vs quick
cr = {}
with open(os.path.join(src, "results_crossover.csv")) as f:
    for row in csv.DictReader(f):
        cr.setdefault(row["algorithm"], []).append((int(row["n"]), float(row["avg_us"])))
fig, ax = plt.subplots(figsize=(6.4, 3.6))
for i, name in enumerate(["Insertion Sort", "Merge Sort", "Quick Sort"]):
    xs, ys = zip(*cr[name])
    ax.plot(xs, ys, label=name, markeredgecolor="white", markeredgewidth=1, **STYLES[i])
ax.set_xscale("log", base=2)
ax.set_yscale("log")
ax.set_xticks([4, 8, 16, 32, 64, 128, 256, 512])
ax.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())
ax.legend(loc="upper left")
ax.set(title="Small inputs: Insertion vs Merge vs Quick (log-log scale)", xlabel="Input size n",
       ylabel="Time per sort (µs)")
save(fig, "fig7_small_n.png")

# 8. Summary of measured T(2n)/T(n) ratios against theory
series = [
    ("KMP (random)", load("results_kmp.csv")["random text"], 2),
    ("Merge (random)", load("results_merge.csv")["random"], 2.1),
    ("Quick (random)", load("results_quick.csv")["random"], 2.1),
    ("Insertion (random)", load("results_insertion.csv")["random"], 4),
    ("Quick (sorted)", load("results_quick.csv")["sorted"], 4),
    ("Floyd-Warshall", load("results_floyd.csv")[case], 8),
]
fig, ax = plt.subplots(figsize=(6.4, 3.4))
for i, (name, pts, th) in enumerate(series):
    ratios = [pts[k][1] / pts[k - 1][1] for k in range(1, len(pts))]
    ax.scatter([i] * len(ratios), ratios, s=28, color=BLUE, edgecolor="white", linewidth=1, zorder=3,
               label="Measured T(2n)/T(n)" if i == 0 else None)
    ax.plot([i - 0.3, i + 0.3], [th, th], color=ORANGE, lw=2, zorder=2,
            label="Theoretical ratio" if i == 0 else None)
ax.set_xticks(range(len(series)))
ax.set_xticklabels([s[0] for s in series], rotation=20, ha="right")
ax.set_ylim(0, 10)
ax.set(title="Measured growth ratio T(2n)/T(n) vs theory", ylabel="T(2n) / T(n)")
ax.legend(loc="upper left")
save(fig, "fig8_ratios.png")
print("graphs written to", out)
