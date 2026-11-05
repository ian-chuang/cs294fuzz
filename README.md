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

## Commands

**Run a program yourself**

```
build/fuzz_ini < seeds/ini/basic.ini     # run once on one input
echo $?                                  # exit code: 0 means no crash
```

**Fuzz** (make the terminal panel big first: AFL++ needs 80×25; Ctrl-C to stop)

```
afl-fuzz -i seeds/ini -o runs/live -- build/fuzz_ini
```

`-i` is the seed folder, `-o` where to write the results, and after `--` comes the program. Each run needs a new `-o` folder. Open a second terminal (the **+** in the terminal panel) to keep working while it runs.

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

**Shrink a crash** to a smaller input that still crashes

```
afl-tmin -i <file> -o min.ini -- build/fuzz_ini
```

**Debug it**

```
gdb -q --args build/fuzz_ini
(gdb) break minilib.c:92         # stop at a line (add "if <condition>" to stop only sometimes)
(gdb) run < <file>               # start, with the file as input (Tab completes file names)
(gdb) print <variable>
(gdb) bt                         # the call stack
(gdb) continue
(gdb) quit
```

**Rebuild** after you edit anything in `target/`

```
make                         # everything
make build/fuzz_join         # one program
```

**Our helpers**

```
scripts/triage.sh ini <folder or files>    # re-run each input, show the error and where
scripts/group.sh  ini <folder or files>    # the same, grouped: inputs with the same error at the same place
scripts/plot.py out.png runs/a runs/b      # coverage over time, as a picture
```

The first argument is the harness (`ini`, `ini2` or `join`).
