# Day 2 plan (instructor)

**Paper:** Zhao et al., "A Qualitative Analysis of Fuzzer Usability and Challenges", CCS '25. 18 semi-structured interviews with experienced fuzzer users, thematic analysis.

**Homework after today:** "In your own words, describe some lessons that this week's tool can teach us about how to design future tools."

**The rule for every segment:** students try it first, you collect their answers, *then* you show the study's quote ("the experts said the same thing"). The quotes are the reveal, not the lecture. Every activity ends in a vote or a share-out.

## Learning objectives

By the end of class, students will be able to:

1. **Decide what to fuzz and explain how the harness shapes the result.** Judge whether code is worth fuzzing, and explain how a harness limits what the fuzzer can reach and can even cause false crashes.
2. **Interpret fuzzer output critically.** Turn a folder of crash files into a short list of distinct bugs, and explain why "crashes", "unique crashes" and "bugs" are different numbers.
3. **Judge a campaign's progress and propose a stopping rule.** Read coverage over time, tell "nothing left to find" apart from "the harness is stuck", and explain why no fuzzer can say "you're done".

## Timing (75 minutes, about 60 hands-on or discussion)

| Min | Segment | Format |
|---|---|---|
| 0–2 | One person per group creates a Codespace | hands-on |
| 2–6 | The study in 4 minutes, while the Codespaces build | lecture |
| 6–9 | Tour of the Codespace's files | follow along |
| 9–13 | Run the fuzzer, read the live screen | hands-on |
| 13–20 | Reproduce a crash by hand: what ASAN is and how to read it | hands-on |
| 20–37 | Objective 1: where to fuzz, and harness bugs | groups + pairs |
| 37–54 | Objective 2: from a folder of crashes to bugs, to a fix | pairs + demo |
| 54–65 | Objective 3: is it progressing, and when do you stop? | vote + pairs |
| 65–75 | Debate: fuzzing vs. other ways to verify code, then homework | two teams |

## 0–2 · Create the Codespaces

One person per group: github.com/ian-chuang/cs294fuzz → Code → Codespaces → ⋯ → New with options → branch `day2` → Create. The first start takes a few minutes (it compiles tcpdump); `README.md` opens when it's ready. It is free for them (personal accounts get 120 core-hours a month; this uses 2 per hour).

## 2–6 · The study (while they build)

- 18 interviews; participants from industry and academia, with varied experience and depth of understanding.
- Fuzzing is uniquely valuable, but a great fit for only a few domains: "file formats, network-based formats, parsing, decoding, deserializing" (P12).
- Users understand how fuzzers *should* work, but get little feedback on what happens inside: unpredictable and opaque. There is no ground truth for judging a fuzzer or a campaign.
- Limits (§3.3): self-selected, experienced users; interviews, not observation.

## 6–9 · Tour of the files

Three groups: what **we wrote**, what **`make`** built from it, and what **AFL++** wrote.

```
target/            WE WROTE: the code under test
  minilib.c          the "library", with planted bugs (parse_ini, checksum, ...)
  harness_*.c        tiny main()s: read stdin, call one library function
seeds/             WE WROTE: one small valid starting input per harness
Makefile           WE WROTE: how to compile target/ into build/
build/             MAKE BUILT (when the Codespace started): fuzz_ini, fuzz_ini_trunc, ...
runs/              AFL++ WRITES: your own runs (empty at first)
results/           AFL++ WROTE, we saved: 10-minute runs recorded before class
scripts/           WE WROTE, helpers only: triage.sh, plot.py
```

Open `target/harness_ini.c` (read stdin, copy it, call `parse_ini`) and `seeds/ini/basic.ini`. Say: `build/fuzz_ini` is an ordinary program, and `build/fuzz_ini < seeds/ini/basic.ini` runs it once. The fuzzer does exactly that thousands of times a second with mutated files. One line on the `Makefile`: it compiles with `afl-clang-fast` (adds coverage tracking) and `AFL_USE_ASAN=1` (next segment). That is the study's "custom instrumentation compilers" point.

## 9–13 · Run the fuzzer

Make the terminal panel big (AFL++ needs 80×25), then:

```
afl-fuzz -i seeds/ini -o runs/live -- build/fuzz_ini
```

Leave it running; you come back to it in objective 3.

**How to present the screen: don't explain all of it.** The screen is the point P12 makes later ("famously inscrutable"), so say so up front: "Most of this you can ignore. Four questions matter, and each has one or two numbers." Point at those, in this order, then give the prompt. A sample screen 2 seconds in:

```
american fuzzy lop ++4.09c {default} (build/fuzz_ini) [fast]
┌─ process timing ────────────────────────────────────┬─ overall results ────┐
│        run time : 0 days, 0 hrs, 0 min, 2 sec       │  cycles done : 0     │
│   last new find : 0 days, 0 hrs, 0 min, 0 sec       │ corpus count : 35    │
│last saved crash : 0 days, 0 hrs, 0 min, 1 sec       │saved crashes : 3     │
│ last saved hang : none seen yet                     │  saved hangs : 0     │
├─ cycle progress ─────────────────────┬─ map coverage┴──────────────────────┤
│  now processing : 0.0 (0.0%)         │    map density : 25.49% / 37.25%    │
│  runs timed out : 0 (0.00%)          │ count coverage : 7.97 bits/tuple    │
├─ stage progress ─────────────────────┼─ findings in depth ─────────────────┤
│  now trying : splice 8               │ favored items : 1 (2.86%)           │
│ stage execs : 14/24 (58.33%)         │  new edges on : 7 (20.00%)          │
│ total execs : 1347                   │ total crashes : 3 (3 saved)         │
│  exec speed : 714.4/sec              │  total tmouts : 11 (0 saved)        │
├─ fuzzing strategy yields ────────────┴─────────────┬─ item geometry ───────┤
│   bit flips : disabled (default, enable with -D)   │    levels : 2         │
│  byte flips : disabled (default, enable with -D)   │   pending : 35        │
│ arithmetics : disabled (default, enable with -D)   │  pend fav : 1         │
│  known ints : disabled (default, enable with -D)   │ own finds : 34        │
│  dictionary : n/a                                  │  imported : 0         │
│havoc/splice : 33/0, 3/120                          │ stability : 100.00%   │
│py/custom/rq : unused, unused, unused, unused       ├───────────────────────┘
│    trim/eff : 22.39%/21, disabled                  │             [cpu:150%]
└─ strategy: explore ────────── state: started :-) ──┘
```

| Question | Where to point | What to say |
|---|---|---|
| 1. Is it running fast enough? | `exec speed : 714.4/sec`, `total execs : 1347` | It runs the program about 700 times a second: start, feed one input, exit. Tuesday's notebook managed far fewer. Speed is the one number every user watches. |
| 2. What has it kept? | `corpus count : 35` | **Tuesday's population.** Started with 1 seed; 34 mutated inputs were kept because each reached code no earlier input had (that's `own finds : 34`). They are files in `runs/live/default/queue/`. |
| 3. How much of the program has it reached? | `map density : 25.49% / 37.25%` | The "map" is AFL++'s record of which **edges** (jumps between blocks of code, Tuesday's branch coverage) were taken. This program has 102 edges (`total_edges` in `fuzzer_stats`). The right-hand number is the important one: all inputs together have reached 37% of them. (The left is just the input being mutated right now.) |
| 4. Has it found anything, and is it still finding things? | `saved crashes : 3`, `last new find`, `last saved crash` | 3 crashing inputs saved in `crashes/`. *Saved* crashes are one per new path, not one per bug (objective 2). `last new find` is how long since it last kept a new input: when that keeps growing, it has stopped making progress (objective 3). |

Then connect two more to Tuesday, briefly:

- **`[fast]` in the title bar is the power schedule** from Tuesday's AFLFast paper: AFL++ uses AFLFast's FAST schedule by default, giving more mutations to inputs on rarely-taken paths.
- **`levels : 2` is the family tree.** The seed is level 1, its mutants level 2, their mutants level 3. The queue file names record each input's parent (`src:`) and mutation (`op:`); you'll read those later.

**If students ask about the rest** (have these ready, but don't volunteer them):

| Field | Meaning |
|---|---|
| `cycles done` | complete passes over the whole queue. A "cycle" of 0 just means it hasn't finished the first pass. |
| `now processing : 0.0` | which queue entry is being mutated (entry 0, the seed), `.` if it is favored (`*` if not), then how many times it has been fuzzed before (0). |
| `now trying`, `stage execs` | the mutation strategy running right now (`havoc`: random stacked mutations; `splice`: glue two inputs together) and its progress. |
| `runs timed out`, `total tmouts : 11 (0 saved)` | runs that hit the time limit. They are saved as hangs only if they also time out on a longer limit. |
| `count coverage` | how varied the hit *counts* are per edge (AFL++ also rewards running a loop 4 times instead of 1). |
| `favored items` | the smallest set of inputs that together cover every edge seen; these get mutated most. |
| `new edges on` | how many kept inputs reached a brand-new edge (the rest only changed hit counts). |
| `bit flips` … `known ints` | the old deterministic mutation stages, off by default in AFL++ 4 (`-D` turns them on). |
| `havoc/splice : 33/0, 3/120` | new inputs found / executions, per strategy: splice found 3 in 120 runs. Executions are added when a stage finishes, so havoc's `/0` only means its first stage is still running. |
| `trim/eff` | AFL++ shortens each kept input before mutating it; 22% of the bytes were removable. |
| `pending`, `pend fav` | queue entries not fuzzed yet (all, and favored ones). |
| `imported` | inputs taken from other fuzzers running in parallel (none: we run one). |
| `stability` | does the same input always take the same path? 100% means yes. Lower means randomness or global state in the target. |
| `strategy: explore`, `[cpu:150%]` | AFL++'s mode (explore before exploit), and CPU use. |

**Show where the numbers live (1 min).** In a second terminal (the + in the terminal panel): `ls runs/live/default/` (`queue/`, `crashes/`, `hangs/`, `fuzzer_stats`), then `cat runs/live/default/fuzzer_stats`: the same numbers as `name : value` lines (`corpus_count`, `saved_crashes`, `execs_per_sec`, `edges_found`). That file is what `plot.py` and any custom script reads.

**Prompt:** "Write down two numbers on this screen you can't explain." Don't answer yet; it comes back in objective 3 with P12's quote. Expect them to pick the ones in the "if students ask" table.

## 13–20 · Reproduce a crash by hand: ASAN

**What ASAN is (1 minute).** In C, writing one byte past the end of an array usually does *not* crash: it silently corrupts memory, and the fuzzer would never notice. AddressSanitizer is a compiler option that adds a check before every memory access and a no-go zone around every block of memory. Touch a no-go byte and the program stops at once and reports what happened and where. Cost: about 2× slower.

**A crash you can read (2 min).** Use the recorded run, so everyone sees the same thing:

```
xxd results/ini_run_a/default/crashes/id:000004*        # 4 bytes: 4=lo
build/fuzz_ini < results/ini_run_a/default/crashes/id:000004*
```

ASAN prints `SEGV on unknown address 0x000000000000 ... caused by a READ`, then a stack, innermost first. Skip the libc and `strlen` frames to the first one in our code: `parse_ini target/minilib.c:120`. Line 120 is `strlen(section)`, and `section` is still NULL because `4=lo` is a `key=value` line with no `[section]` before it. Diagnosed from the input alone.

**The bug ASAN exists for (2 min).**

```
build/fuzz_ini < results/ini_run_a/default/crashes/id:000001*
```

Three lines tell the story: `heap-buffer-overflow ... WRITE of size 1` (what), `#0 in parse_ini target/minilib.c:92` (where), `located 0 bytes after 8-byte region ... allocated ... minilib.c:90` (whose memory). Ignore the "Shadow bytes" table: ASAN's internal map. Without ASAN this input does not crash at all. The input is 84 bytes of junk (`xxd` it): hold that thought for `afl-tmin` in objective 2.

**The same thing, automated (2 min).** `scripts/triage.sh` is our helper, not AFL++: it runs that same command for each file and keeps those lines.

```
scripts/triage.sh ini results/ini_run_a/default/crashes/id:000001*
```

Show it on one file only: running it on the whole folder is objective 2's question.

## 20–37 · Objective 1: configuring a fuzzer is mostly choosing the target and writing the harness

**1a. "Where would you spend fuzzing time?" (8 min, groups of 3).** Slide with six functions from an unfamiliar C codebase, declaration plus one line each:

| Function | What it does |
|---|---|
| `png_read_chunk(buf, len)` | reads one chunk of a PNG image |
| `parse_http_header(line, len)` | parses one HTTP header line |
| `decompress(in, inlen, out, outcap)` | decompresses a buffer |
| `compute_tax(double income)` | computes tax owed |
| `check_permission(user, resource)` | decides whether a user may access a resource |
| `on_button_click(Widget *w)` | handles a GUI click |

Each group picks its top 2 and writes, for each: (1) what bytes would the fuzzer feed it, (2) how would you know it failed (crash, ASAN error, or a wrong answer that needs a checker), (3) what gets better if the fuzzer finds something (security, robustness, nothing anyone cares about). Groups report.

Expected: the first three take untrusted bytes and fail by crashing: fuzz them. `compute_tax` and `check_permission` fail by giving *wrong answers*, which ASAN cannot see: you would need an oracle (that is property-based testing, the debate later). `on_button_click` has no byte input. Question 2 is what separates "worth fuzzing" from "fuzzable".

Reveal: P12's list, then P15: "Even with help ... the way we select functions is not scientific. It's intuition-based." P16 picks functions that "looked interesting".

**1b. The buggy harness (6 min, pairs).**

1. Show only this entry from `scripts/triage.sh ini_buggy results/ini_buggy/default/crashes`:
   `error: stack-buffer-overflow, WRITE of size 80` / `where: main  target/harness_ini_buggy.c:12`
2. Ask: "Is this a bug in `parse_ini`?" Show of hands.
3. Pairs open `target/harness_ini.c`, `harness_ini_trunc.c`, `harness_ini_buggy.c` and find the line at fault.
4. Reveal P17: "False positive crashes could be occurring from my harness itself."

Answers: `ini` passes an exact-size heap copy (so over-reads are caught). `ini_trunc` passes only the first 12 bytes: it can still find the short bugs, but it can never reach long values; its coverage stops lower (41 edges vs 48, objective 3). `ini_buggy` copies the whole input into a 64-byte stack array: every input over 64 bytes crashes *in the harness*. Only 1 of its 10 crash files shows this, because every long input takes the same path.

**1c. Seeds and suggestions (3 min, whole class).**

- "Where would you get starting inputs for tcpdump?" Its own `tests/` folder of capture files, which is what Fuzzing101 exercise 3 uses. P18 calls test-suite inputs the "best way", since developers "know their code the best".
- "The most requested feature was automatic harness generation. Just use AI?" Then: "Would you trust an AI-written harness? `ini_buggy` is exactly the harness an AI might write."
- Limited documentation and switching fuzzers: one slide, one quote, at most 30 seconds. P5: "reading blog posts and experimenting made it hard to tell if I'm doing it right."

## 37–54 · Objective 2: the output is a starting point, not a diagnosis

**2a. Triage (6 min, pairs).**

1. `ls results/ini_run_a/default/crashes`: 11 files. **"How many bugs?"** Write the guesses on the board.
2. `scripts/triage.sh ini results/ini_run_a/default/crashes`, then group.
3. Debrief: "What rule did you use to group them? Line number, whole stack trace, kind of error?"
4. Reveal P3: "Out of those thousand crashes, 900 of them can be redundant." Then P7 and P11: deduplication goes wrong in both directions (different bugs merged; one bug split).

Answer: 11 crash files are 3 bugs. 5 × SEGV at `minilib.c:120` (a `key=value` line before any `[section]`: `strlen(NULL)`), 5 × heap-buffer-overflow at `minilib.c:92` (an 8-character key writes one byte past `malloc(8)`), 1 × assert at `minilib.c:76` (sections nested deeper than `[a.b.c]`). Plus 1 hang in `hangs/` (a value ending in a backslash at the end of input loops forever). AFL++ saves a crash per new *coverage path*, not per bug.

**2b. Rank by severity (4 min, vote).** Heap overflow, NULL dereference, assert, hang. Expect disagreement; that is the point. Usual order: heap overflow (memory corruption) > NULL dereference (crash, denial of service) > assert (an intended check, but reachable from input) > hang (denial of service, or noise). Reveal P6: "Does a timeout mean there was a bug? Or could there be multiple bugs?" and the four meanings of "false positive": duplicates (P1), a real bug wrongly discarded (P3), not a real vulnerability (P13), caused by the harness (P17).

**2c. From output to root cause, to a fix (7 min).** Instructor demo of the whole chain on the heap overflow from the opening (`id:000001`), with pairs writing the fix at step 4:

1. **Shrink:** `afl-tmin -i results/ini_run_a/default/crashes/id:000001* -o min.ini -- build/fuzz_ini`, then `xxd min.ini`: 84 bytes of junk become `[0]`, newline, `00000000=` (a section and an 8-character key).
2. **Reproduce:** `build/fuzz_ini < min.ini`: the same `heap-buffer-overflow` at `minilib.c:92`.
3. **Inspect:** `gdb -q --args build/fuzz_ini`, `break minilib.c:92`, `run < min.ini`, `print klen`: 8, so `key[klen]` writes one byte past `malloc(8)`.
4. **Fix:** pairs propose it. Edit `target/minilib.c` line 90 to `malloc(INI_KEY_MAX + 1)`.
5. **Rebuild and rerun:** `make` (only `build/` changes), then `build/fuzz_ini < min.ini`: no report. Then `scripts/triage.sh ini results/ini_run_a/default/crashes`: the five `minilib.c:92` entries now say "no crash", the other bugs remain. Afterwards: `git checkout target/minilib.c && make`.
6. Reveal P9: "more of a starter. I take that input and use other tools to figure out where the bug is." Count the tools used that were not the fuzzer: `afl-tmin` is the only AFL++ one.

**Stretch for pairs who finish early: real code.** TODO after the Fuzzing101 exercise 3 run (tcpdump 4.9.2, several hours): triage its saved crashes with `scripts/triage.sh tcpdump <crashes folder>`.

## 54–65 · Objective 3: monitoring, and deciding when to stop

**3a. Back to the live run (2 min).** "What were your two unexplained numbers?" Collect a few. Reveal P12: "Runtime outputs are famously inscrutable ... it's all just abbreviations of things. Nobody knows what the heck they are until you read the source code." Students can stop their run now (Ctrl-C).

**3b. The coverage plots (5 min, vote with fingers).** Open `results/coverage.png`. For each line: 1 = keep going, 2 = stop, 3 = fix the harness.

| Line | What it shows | Reveal |
|---|---|---|
| `ini_run_a` vs `ini_run_b` | same command, different curves early, same end (48 edges) | P14: "Fuzzing results are inherently random." |
| `ini_trunc` | stops at 41 edges, below the others | a harness problem, not an exhausted program. P18: "If the coverage is not increasing anymore ... maybe there's a bug in our harness." |
| `checksum` | flat at 5 edges from the start | nothing to explore: stop |
| tcpdump | TODO: add the Fuzzing101 run's curve | |

**3c. Design the status screen (4 min, pairs).** "What one thing could the status screen show that would make you confident enough to stop?" Then compare with the study: run overnight and stop if nothing new (P15), a fixed 24 hours because benchmarks use it (10/18), P8's "the frequency with which the fuzzer hits each path" (Tuesday's AFLFast table), and the study's suggestion: detailed, actionable runtime output.

## 65–75 · Debate and homework

Scenario: "Your team has 2 weeks to harden a new PNG decoder written in C." Team A argues for fuzzing. Team B argues for an alternative: property-based testing (QuickCheck), rewriting in Rust, or proving it in Dafny.

- 2 min prepare, 2 min per side.
- 2 min: you pull out "which of today's challenges does each approach avoid, and which does it share?" (PBT still needs someone to choose targets and write the property; Rust removes the overflow class but not the hang or the assert; Dafny needs a specification.)
- 1 min: homework. Their raw material is the debate and their status-screen designs.

## What's in the Codespace

| Path | What |
|---|---|
| `target/minilib.c` | the library under test, with planted bugs |
| `target/harness_*.c` | `ini`, `ini_trunc`, `ini_buggy`, `checksum` |
| `seeds/` | starting inputs (`ini_small` is for `ini_buggy`) |
| `build/` | built by `make` when the Codespace is created |
| `results/` | pre-recorded 10-minute runs (Linux, AFL++ 4.09c) |
| `~/fuzzing_tcpdump/` | tcpdump 4.9.2 + libpcap 1.8.0, set up as in Fuzzing101 exercise 3 (in the image) |
| `scripts/` | our helpers, not AFL++: `triage.sh`, `plot.py`, `record_results.sh` |

## Before class

- Create a Codespace on this branch once, to check it builds (first start takes a few minutes: it compiles tcpdump).
- To re-record `results/`: `scripts/record_results.sh 600` in the Codespace (about an hour), then commit `results/`.
- Stop the Codespace afterwards (idle ones still count against the free hours until they time out).
