# CS294 Fuzzing, Day 2: fuzzing in practice

Today you'll use a real fuzzer, **AFL++**, on a small C library, and compare what you see with what 18 experienced fuzzer users said in an interview study (Zhao et al., CCS '25).

Open this repository in a **GitHub Codespace** (Code → Codespaces → Create codespace on `day2`). The first start takes a few minutes. Every command below runs from the repository's top folder, in the terminal.

## What's here

| Path | What |
|---|---|
| `target/minilib.c` | the library under test |
| `target/harness_*.c` | the harnesses: small `main()`s that turn the fuzzer's bytes into a call to the library |
| `seeds/` | starting inputs for the fuzzer |
| `build/fuzz_*` | the harnesses compiled with AFL++'s compiler (for coverage) and AddressSanitizer (so memory errors crash loudly). See `Makefile`. |
| `results/` | runs we recorded earlier, longer than we have time for in class |
| `~/fuzzing_tcpdump/` | a real program, tcpdump 4.9.2, set up as in [Fuzzing101 exercise 3](https://github.com/antonio-morales/Fuzzing101/tree/main/Exercise%203) |
| `scripts/` | **our helpers, not part of AFL++** |

### The library: `target/minilib.c`

| Function | What it does | Harness |
|---|---|---|
| `parse_ini` | parses `[section]` and `key=value` config text | `ini`, `ini_trunc`, `ini_buggy` |
| `checksum` | Adler-32 checksum of a byte buffer | `checksum` |
| `url_decode` | turns `a%2Cb+c` into `a,b c` | none |
| `format_cents` | `1234` → `"$12.34"` | none |
| `lookup_user` | looks a name up in a fixed table | none |

## Commands

**AFL++ (the real tool):**

| What | Command |
|---|---|
| Fuzz (Ctrl-C to stop) | `afl-fuzz -i seeds/ini -o runs/live -- build/fuzz_ini` |
| Shrink a crashing input | `afl-tmin -i <crash file> -o min.ini -- build/fuzz_ini` |

The status screen needs a terminal of at least 80×25: drag the terminal panel bigger if AFL++ says it is too small. To fuzz another harness, change the binary (and use `seeds/ini_small` for `ini_buggy`, `seeds/checksum` for `checksum`). Each run needs its own `-o` folder.

**Ordinary tools:**

| What | Command |
|---|---|
| Re-run one input by hand | `build/fuzz_ini < <file>` |
| Look at an input's bytes | `xxd <file>` |
| Debug one input | `gdb -q --args build/fuzz_ini`, then `break minilib.c:92`, `run < <file>`, `bt`, `print klen` |
| Rebuild after editing `target/` | `make` |

**Our helpers (`scripts/`):**

| What | Command |
|---|---|
| Re-run every crash, show what went wrong and where | `scripts/triage.sh ini results/ini_run_a/default/crashes` |
| Coverage over time, as a picture | `scripts/plot.py coverage.png results/ini_run_a results/ini_trunc ...` |

`plot.py` replays each run's `queue/` through AFL++'s `afl-showmap` to count edges second by second. AFL++'s own `plot_data` is too coarse to show the first minute. The plot is already drawn in `results/coverage.png`.

## What AFL++ writes (`runs/<name>/default/`)

| Path | What it is |
|---|---|
| `queue/` | the population: every input that reached new coverage. The name records where it came from: `id:000012,src:000003,time:840,execs:1200,op:havoc,rep:4,+cov` |
| `crashes/` | inputs that crashed the target (one per new crash *path*, not per bug) |
| `hangs/` | inputs that ran past the timeout |
| `fuzzer_stats` | the numbers on the status screen, as `name : value` lines |
| `plot_data` | the same numbers over time |

## The recorded runs (`results/`)

Each ran for 10 minutes on 2 cores.

| Run | Harness | Seeds |
|---|---|---|
| `ini_run_a`, `ini_run_b` | `ini` (the same command twice) | `seeds/ini` |
| `ini_trunc` | `ini_trunc` | `seeds/ini` |
| `ini_buggy` | `ini_buggy` | `seeds/ini_small` |
| `checksum` | `checksum` | `seeds/checksum` |
## Activity 1: What should we fuzz, and how?

1. On the slide: which functions would you spend fuzzing time on? For each, what bytes would you feed it, how would you know it failed, and what gets better if the fuzzer finds something?
2. `scripts/triage.sh ini_buggy results/ini_buggy/default/crashes`. One line blames `harness_ini_buggy.c`. Is that a bug in `parse_ini`?
3. Open `target/harness_ini.c`, `harness_ini_trunc.c` and `harness_ini_buggy.c` side by side. They all call the same `parse_ini`. What does each one get wrong, or limit?

## Activity 2: What did the fuzzer actually find?

1. `ls results/ini_run_a/default/crashes`. How many crash files? How many bugs do you predict?
2. `scripts/triage.sh ini results/ini_run_a/default/crashes`. How many **different bugs** are there? What rule did you use to group them?
3. Rank the bugs from most to least severe. Is an `assert` failure a bug? Is a hang (`results/ini_run_a/default/hangs`)?
4. Pick the heap overflow. Shrink it with `afl-tmin`, then find the root cause with ASAN's report and `gdb`. What's the one-line fix? Make it, run `make`, and re-run the crash.

## Activity 3: When would you stop?

1. Look at the status screen of your own run. Which numbers can you explain? Which can't you?
2. Open `results/coverage.png`. For each line: keep going, stop, or fix the harness?
3. `ini_run_a` and `ini_run_b` are the same command. Why do they differ?
4. What one thing could the status screen show that would make you confident enough to stop?
