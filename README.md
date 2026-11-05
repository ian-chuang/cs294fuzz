# CS294 Fuzzing, Day 2: fuzzing in practice

Today you'll use a real fuzzer, **AFL++**, on a small C library. Run every command below in the Codespace's terminal, from the top folder (`/workspaces/cs294fuzz`).

## What's here

| Path | What |
|---|---|
| `target/minilib.c` | the library being tested: `parse_ini` (reads `[section]` / `key=value` config text) and `join_path` (`"docs"` + `"a.txt"` → `"docs/a.txt"`) |
| `target/harness_*.c` | the **harnesses**: small programs that read the fuzzer's bytes and call one library function |
| `seeds/` | starting inputs for the fuzzer |
| `Makefile` | builds each harness + the library into `build/`, with AFL++'s compiler and AddressSanitizer (ASAN) |
| `build/` | the built programs |
| `runs/` | your fuzzing runs go here |
| `results/` | 10-minute runs recorded before class |
| `scripts/` | our helpers (not part of AFL++) |

The helpers in `scripts/` take the harness name first (`ini`, `ini2` or `join`).

## Tour

**Run a program yourself**

```
build/fuzz_ini < seeds/ini/basic.ini     # run once on one input
echo $?                                  # exit code: 0 means no crash
```

**Fuzz** (make the terminal panel big first: AFL++ needs 80×25; Ctrl-C to stop)

```
afl-fuzz -i seeds/ini -o runs/live -- build/fuzz_ini
```

`-i` is the seed folder, `-o` where to write the results, and after `--` comes the program. Each run needs a new `-o` folder. Leave this one running, and open a second terminal (the **+** in the terminal panel) to keep working.

**What AFL++ writes** (in `runs/<name>/default/`, and the same in `results/<name>/default/`)

| Path | What |
|---|---|
| `queue/` | every input it kept because it reached new code |
| `crashes/` | inputs that crashed the program |
| `hangs/` | inputs that ran too long |
| `fuzzer_stats` | the status screen's numbers, as text |

**Look at a crash**

```
cat  <file>                   # the input as text
xxd  <file>                   # the input as bytes: hex on the left, text on the right
build/fuzz_ini < <file>       # re-run it: ASAN reports what went wrong, and where
```

In ASAN's report, read the `ERROR:` line, then the stack (`#0`, `#1`, …): the first frame in `minilib.c` or a harness is where it went wrong.

**Re-run crashes with our helper**

```
scripts/triage.sh ini <folder or files>    # re-run each input, show the error and where
```

**Debug it**

```
gdb -q --args build/fuzz_ini
(gdb) break <file>:<line>        # stop at a line (add "if <condition>" to stop only sometimes)
(gdb) run < <file>               # start, with the file as input (Tab completes file names)
(gdb) print <variable>
(gdb) bt                         # the call stack
(gdb) continue
(gdb) quit
```

## Objective 1: write a harness

Fill in the TODOs in `target/harness_join.c`, then build and fuzz it:

```
make build/fuzz_join                                   # rebuild after every edit
build/fuzz_join < seeds/join/basic.txt                 # does it run on the seed?
afl-fuzz -i seeds/join -o runs/join -- build/fuzz_join
```

Each new try needs a new `-o` folder (`runs/join2`, …), or delete the old one first with `rm -rf runs/join`.

Re-run a crash it finds, and read the stack: is the bug in `join_path`, or in your harness?

```
build/fuzz_join < runs/join/default/crashes/<file>
scripts/triage.sh join runs/join/default/crashes
```

## Objective 2: from crashes to bugs, and from one crash to a fix

**Triage and group** the recorded crashes

```
ls results/ini/default/crashes
scripts/triage.sh ini results/ini/default/crashes
scripts/group.sh  ini results/ini/default/crashes results/ini/default/hangs   # same error at the same place = one group
```

**Shrink** a crash to a smaller input that still crashes, then re-run it

```
afl-tmin -i <file> -o min.ini -- build/fuzz_ini
cat min.ini
build/fuzz_ini < min.ini
```

**Fix** it in `target/minilib.c`, rebuild, and check

```
make
build/fuzz_ini < min.ini
scripts/triage.sh ini results/ini/default/crashes    # which crashes are gone now?
```

To undo your edits: `git checkout target/minilib.c && make`

## Objective 3: is it still making progress?

**Your live run**: switch back to the terminal running `runs/live`, or read its numbers as text

```
cat runs/live/default/fuzzer_stats
```

**Coverage over time** for the two recorded runs: open `results/coverage.png`, or redraw it

```
scripts/plot.py out.png results/ini results/ini2     # also works on your own runs/<name>
```

The two harnesses in that plot are `target/harness_ini.c` and `target/harness_ini2.c`.

## Rebuild

After you edit anything in `target/`:

```
make                         # everything
make build/fuzz_join         # one program
```
