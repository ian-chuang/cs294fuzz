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
| 0–10 | AFL++ walkthrough: everyone starts a live run | hands-on |
| 10–14 | The study in 4 minutes | lecture |
| 14–34 | Objective 1: where to fuzz, and harness bugs | groups + pairs |
| 34–54 | Objective 2: from a folder of crashes to bugs | pairs + demo |
| 54–65 | Objective 3: is it progressing, and when do you stop? | vote + pairs |
| 65–75 | Debate: fuzzing vs. other ways to verify code, then homework | two teams |

## 0–10 · AFL++ walkthrough

Everyone, in the Codespace terminal (make the terminal panel big: AFL++ needs 80×25):

```
afl-fuzz -i seeds/ini -o runs/live -- build/fuzz_ini
```

Leave it running; you come back to it in objective 3. Point out on the screen: **corpus count** (Tuesday's "population"), **saved crashes**, **exec speed**, **map density / edges** (coverage). Then 30 seconds on the `Makefile`: the library was rebuilt with `afl-clang-fast` (coverage) and ASAN (memory errors crash loudly). That is the study's "custom instrumentation compilers" point; no separate slide needed.

**Prompt:** "Write down two numbers on this screen you can't explain." Don't answer yet.

## 10–14 · The study

- 18 interviews; participants from industry and academia, with varied experience and depth of understanding.
- Fuzzing is uniquely valuable, but a great fit for only a few domains: "file formats, network-based formats, parsing, decoding, deserializing" (P12).
- Users understand how fuzzers *should* work, but get little feedback on what happens inside: unpredictable and opaque. There is no ground truth for judging a fuzzer or a campaign.
- Limits (§3.3): self-selected, experienced users; interviews, not observation.

## 14–34 · Objective 1: configuring a fuzzer is mostly choosing the target and writing the harness

**1a. "Where would you spend fuzzing time?" (9 min, groups of 3).** Slide with six functions from an unfamiliar C codebase, declaration plus one line each:

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

**1b. The buggy harness (8 min, pairs).**

1. Show only this line from `scripts/triage.sh ini_buggy results/ini_buggy/default/crashes`:
   `stack-buffer-overflow  harness_ini_buggy.c:12 in main`
2. Ask: "Is this a bug in `parse_ini`?" Show of hands.
3. Pairs open `target/harness_ini.c`, `harness_ini_trunc.c`, `harness_ini_buggy.c` and find the line at fault.
4. Reveal P17: "False positive crashes could be occurring from my harness itself."

Answers: `ini` passes an exact-size heap copy (so over-reads are caught). `ini_trunc` passes only the first 12 bytes: it can still find the short bugs, but it can never reach long values; its coverage stops lower (41 edges vs 48, objective 3). `ini_buggy` copies the whole input into a 64-byte stack array: every input over 64 bytes crashes *in the harness*. Only 1 of its 10 crash files shows this, because every long input takes the same path.

**1c. Seeds and suggestions (3 min, whole class).**

- "Where would you get starting inputs for tcpdump?" Its own `tests/` folder of capture files, which is what our tcpdump run used. P18 calls test-suite inputs the "best way", since developers "know their code the best".
- "The most requested feature was automatic harness generation. Just use AI?" Then: "Would you trust an AI-written harness? `ini_buggy` is exactly the harness an AI might write."
- Limited documentation and switching fuzzers: one slide, one quote, at most 30 seconds. P5: "reading blog posts and experimenting made it hard to tell if I'm doing it right."

## 34–54 · Objective 2: the output is a starting point, not a diagnosis

**2a. Triage (8 min, pairs).**

1. `ls results/ini_run_a/default/crashes`: 11 files. **"How many bugs?"** Write the guesses on the board.
2. `scripts/triage.sh ini results/ini_run_a/default/crashes`, then group.
3. Debrief: "What rule did you use to group them? Line number, whole stack trace, kind of error?"
4. Reveal P3: "Out of those thousand crashes, 900 of them can be redundant." Then P7 and P11: deduplication goes wrong in both directions (different bugs merged; one bug split).

Answer: 11 crash files are 3 bugs. 5 × SEGV at `minilib.c:120` (a `key=value` line before any `[section]`: `strlen(NULL)`), 5 × heap-buffer-overflow at `minilib.c:92` (an 8-character key writes one byte past `malloc(8)`), 1 × assert at `minilib.c:76` (sections nested deeper than `[a.b.c]`). Plus 1 hang in `hangs/` (a value ending in a backslash at the end of input loops forever). AFL++ saves a crash per new *coverage path*, not per bug.

**2b. Rank by severity (4 min, vote).** Heap overflow, NULL dereference, assert, hang. Expect disagreement; that is the point. Usual order: heap overflow (memory corruption) > NULL dereference (crash, denial of service) > assert (an intended check, but reachable from input) > hang (denial of service, or noise). Reveal P6: "Does a timeout mean there was a bug? Or could there be multiple bugs?" and the four meanings of "false positive": duplicates (P1), a real bug wrongly discarded (P3), not a real vulnerability (P13), caused by the harness (P17).

**2c. From output to root cause, then a fix (7 min).** Instructor demo, then pairs write the fix:

1. `afl-tmin -i <heap-overflow crash> -o min.ini -- build/fuzz_ini`, then `xxd min.ini`: a short input with an 8-character key.
2. `build/fuzz_ini < min.ini`: ASAN says `heap-buffer-overflow ... WRITE of size 1`, first frame in our code `minilib.c:92`.
3. `gdb -q --args build/fuzz_ini`, `break minilib.c:92`, `run < min.ini`, `print klen`: 8, so `key[klen]` writes one byte past `malloc(8)`.
4. Pairs: one-line fix (`malloc(INI_KEY_MAX + 1)` at line 90, or `malloc(klen + 1)`).
5. Reveal P9: "more of a starter. I take that input and use other tools to figure out where the bug is." Count how many tools you used that were not the fuzzer.

**Stretch for pairs who finish early: real code.** `scripts/triage.sh tcpdump tcpdump/crashes`: 19 capture files that crash tcpdump 4.9.2 (from tcpdump 4.9.3's regression tests, the release that fixed them). How many distinct bugs? All 19 are heap over-reads, at 17 different lines in 16 functions. The two `icmp-*` files and the two `smb_*` files crash on the same line. The two `bgp-*` files (`print-bgp.c:2345` and `:2354`) and the two `vrrp-*` files (`:146` and `:154`) are in the same function but on different lines: one bug or two? The file names help here, but a fuzzer's crash files have no such names.

## 54–65 · Objective 3: monitoring, and deciding when to stop

**3a. Back to the live run (2 min).** "What were your two unexplained numbers?" Collect a few. Reveal P12: "Runtime outputs are famously inscrutable ... it's all just abbreviations of things. Nobody knows what the heck they are until you read the source code." Students can stop their run now (Ctrl-C).

**3b. The coverage plots (5 min, vote with fingers).** Open `results/coverage.png` and `results/tcpdump.png`. For each line: 1 = keep going, 2 = stop, 3 = fix the harness.

| Line | What it shows | Reveal |
|---|---|---|
| `ini_run_a` vs `ini_run_b` | same command, different curves early, same end (48 edges) | P14: "Fuzzing results are inherently random." |
| `ini_trunc` | stops at 41 edges, below the others | a harness problem, not an exhausted program. P18: "If the coverage is not increasing anymore ... maybe there's a bug in our harness." |
| `checksum` | flat at 5 edges from the start | nothing to explore: stop |
| tcpdump | 5,480 → 7,274 edges, still climbing after 1 hour, 0 crashes | keep going. Yet the 19 crashing captures show bugs are there. |

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
| `results/` | pre-recorded 10-minute runs (Linux, AFL++ 4.09c), and a 1-hour tcpdump run (stats only) |
| `tcpdump/crashes/` | 19 captures that crash tcpdump 4.9.2 |
| `/opt/tcpdump/` | tcpdump 4.9.2 + libpcap 1.8.0 built with AFL++ and ASAN (in the image) |
| `scripts/` | our helpers, not AFL++: `triage.sh`, `plot.py`, `record_results.sh` |

## Before class

- Create a Codespace on this branch once, to check it builds (first start takes a few minutes: it compiles tcpdump).
- To re-record `results/`: `scripts/record_results.sh 600` in the Codespace (about an hour), then commit `results/`.
- Stop the Codespace afterwards (idle ones still count against the free hours until they time out).
