#!/usr/bin/env python3
"""Plot coverage (edges found) over time for one or more AFL++ runs.
Our helper, not part of AFL++.

    scripts/plot.py out.png runs/run_a runs/run_b ...

Each run is the directory you passed to afl-fuzz -o.

AFL++'s own plot_data only gets a row about once a minute, which hides the fast
climb at the start. So instead this replays the run: every input AFL++ kept in
default/queue/ has its discovery time in its file name (time:<ms>), and
afl-showmap tells us which edges that input covers. Coverage at time t is the
number of distinct edges covered by all inputs kept up to t. Crashes (from
default/crashes/, same naming) are marked with an x.

A run saved without its queue/ is drawn from AFL++'s plot_data instead.

Needs afl-showmap (comes with AFL++) and matplotlib. Without matplotlib it
prints a text table instead.
"""
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent      # build/fuzz_* paths in cmdline are relative to this


def discovery_times(folder: Path):
    """[(seconds, file)] for every AFL++ input file in folder, oldest first."""
    found = []
    for f in folder.glob("id:*"):
        m = re.search(r"time:(\d+)", f.name)
        found.append((int(m.group(1)) / 1000 if m else 0.0, f))
    return sorted(found)


def edges_of(target: str, inp: Path) -> set:
    """The edge ids one input covers, according to afl-showmap (-e: ignore hit counts)."""
    with tempfile.NamedTemporaryFile() as trace, open(inp, "rb") as stdin:
        subprocess.run(["afl-showmap", "-q", "-e", "-m", "none", "-t", "1000",
                        "-o", trace.name, "--", target],
                       stdin=stdin, cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        return {line.split(":")[0] for line in Path(trace.name).read_text().split()}


def from_plot_data(run: Path):
    """Coverage curve from AFL++'s own plot_data (relative_time first, edges_found last).
    Its first row comes only after the seeds are loaded, so the curve starts there."""
    curve = []
    for line in (run / "default" / "plot_data").read_text().splitlines():
        if line.startswith("#"):
            continue
        cols = [c.strip() for c in line.split(",")]
        curve.append((float(cols[0]), int(cols[-1])))
    return curve, []


def replay(run: Path):
    """Coverage curve [(seconds, edges)] and crash points [(seconds, edges)] for one run."""
    if not (run / "default" / "queue").is_dir():
        return from_plot_data(run)
    target = (run / "default" / "cmdline").read_text().split("\n")[0]   # e.g. build/fuzz_ini
    covered = set()
    curve = [(0.0, 0)]
    for t, inp in discovery_times(run / "default" / "queue"):
        covered |= edges_of(target, inp)
        curve.append((t, len(covered)))

    def edges_at(t):
        return max(e for s, e in curve if s <= t)

    crashes = [(t, edges_at(t)) for t, _ in discovery_times(run / "default" / "crashes")]
    stats = (run / "default" / "fuzzer_stats").read_text()
    run_time = int(re.search(r"run_time\s*:\s*(\d+)", stats).group(1))
    curve.append((run_time, len(covered)))                               # extend the line to the end of the run
    return curve, crashes


def main():
    out, runs = sys.argv[1], [Path(r) for r in sys.argv[2:]]
    os.environ.setdefault("ASAN_OPTIONS", "abort_on_error=1:detect_leaks=0:symbolize=0")
    data = {r.name: replay(r) for r in runs}
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        for name, (curve, crashes) in data.items():
            print(f"{name}  ({len(crashes)} crash files)")
            for t, edges in curve:
                print(f"  {t:7.1f}s  {edges:4d} edges")
        return
    fig, ax = plt.subplots(figsize=(7, 4))
    for name, (curve, crashes) in data.items():
        line, = ax.step([t for t, _ in curve], [e for _, e in curve], where="post", label=name)
        if crashes:
            ax.plot([t for t, _ in crashes], [e for _, e in crashes], "x", color=line.get_color())
    ax.set_ylim(bottom=0)
    ax.set_xlim(left=0)
    ax.set_xlabel("seconds")
    ax.set_ylabel("edges covered")
    ax.set_title("Coverage over time (x = a saved crash)")
    ax.legend()
    ax.grid(alpha=0.3)
    fig.tight_layout()
    fig.savefig(out, dpi=120)
    print("wrote", out)


if __name__ == "__main__":
    main()
