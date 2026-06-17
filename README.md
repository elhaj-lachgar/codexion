*This project has been created as part of the 42 curriculum by elachgar.*

---

# Codexion

## Description

Codexion is a multithreaded simulation of the classic dining philosophers problem, reimagined in a developer context. A set of **coders** sit around a table, each needing two **USB dongles** (shared with adjacent coders) to compile their code. Each coder alternates between compiling, debugging, and refactoring. If a coder goes too long without compiling, they **burn out** and the simulation ends. The goal is to orchestrate shared resource access across concurrent threads with no deadlocks, no starvation, and precise burnout detection.

The project exercises core concepts of concurrent programming in C using POSIX threads: mutual exclusion, condition variables, priority scheduling, and careful memory lifecycle management.

**Simulation flow:**
- Each coder runs in its own `pthread` and must acquire both adjacent dongles before compiling.
- A dedicated watcher thread monitors all coders for burnout and for overall completion.
- All threads synchronize on a startup barrier, ensuring a simultaneous and fair launch.
- The simulation stops cleanly when all coders reach the required compile count, or any coder burns out.

---

## Instructions

### Requirements

- GCC or compatible C compiler
- POSIX threads support (`-pthread`)
- GNU Make
- Linux or macOS

### Compilation

```bash
make
```

Produces the `codexion` executable. Additional targets:
```bash
make all      # complie project
make clean    # remove object files
make fclean   # remove objects and binary
make re       # full rebuild
```

### Execution

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <required_compiles> <dongle_cooldown> <scheduler>
```

| Argument | Type | Description |
|---|---|---|
| `number_of_coders` | int  | Number of coder threads |
| `time_to_burnout` | int  | Max ms a coder can go without compiling (0 = instant burnout) |
| `time_to_compile` | int  | Duration of a compile in milliseconds |
| `time_to_debug` | int  | Duration of a debug session in milliseconds |
| `time_to_refactor` | int  | Duration of a refactor session in milliseconds |
| `required_compiles` | int  | Number of compiles each coder must complete |
| `dongle_cooldown` | int  | Ms a dongle must rest after being released |
| `scheduler` | string | Dongle acquisition policy: `fifo` or `edf` (case-sensitive) |

All integer arguments are validated with overflow and non-digit detection. The scheduler argument must be exactly `fifo` or `edf` — descriptive error messages are printed on invalid input.

### Examples

```bash
# 5 coders, 800ms burnout, 200ms compile, 100ms debug, 100ms refactor, 7 compiles, no cooldown, FIFO
./codexion 5 800 200 100 100 7 0 fifo

# Same but with EDF scheduling and a 50ms cooldown
./codexion 5 800 200 100 100 7 50 edf

# Edge case: single coder (acquires one dongle, then blocks and burns out)
./codexion 1 400 200 100 100 3 0 fifo

# Zero burnout — all coders burn out immediately at start
./codexion 5 0 200 100 100 7 0 fifo
```

### Output format

Each log line follows the format:

```
<timestamp_ms> <coder_id> <message>
```

Possible messages: `has taken a dongle`, `is compiling`, `is debuging`, `is refactoring`, `burned out`.

---

## Blocking Cases Handled

### Deadlock Prevention — Coffman's Conditions

The classical deadlock scenario in a resource-ring topology (circular wait, one of Coffman's four necessary conditions) is eliminated by enforcing a **consistent global dongle acquisition order** inside `take_dongles`. Both dongles are always registered and acquired by the coder itself in the order `f` then `l` as assigned, where odd-numbered coders hold `f > l` and even-numbered coders hold `f < l` — breaking any possible cycle. The condition check before sleeping is also atomic under the coder's own `wake_mtx`, preventing a coder from grabbing a dongle that another has already claimed.

### Starvation Prevention — Priority Scheduler (FIFO / EDF)

Each dongle maintains a **min-heap priority queue** of waiting coders (`t_scheduler` inside `t_dongle`). A coder registers itself in both heaps via `req_dongle` before sleeping. It may only proceed once it is at position 0 in **both** heaps simultaneously (`get_smaller` checks), guaranteeing no coder is indefinitely bypassed.

Two scheduling modes are supported:

- **FIFO** — priority is the coder's `req_time` (request timestamp). Coders are served in strict arrival order, bounding wait time to a fixed queue depth.
- **EDF (Earliest Deadline First)** — priority is `last_compile + time_to_burnout`. The coder whose deadline is nearest is served first, minimising burnout risk under contention.

A tie-breaking rule is applied in EDF mode via `is_valid_equal`: when two coders share the same deadline, the one with the lower ID wins, making priority fully deterministic and preventing livelock between equally-urgent coders.


### Cooldown Handling

After a dongle is released in `release_dongle`, it enters a cooldown period:

```c
dongle->available_at = get_time_ms() + config->dongle_cooldown;
```

`is_dongle_avaible` reads both `available_at` and `is_taken` atomically under `locker_d`, closing any TOCTOU window between availability check and acquisition. When `dongle_cooldown` is zero, `wake_up_all` is called immediately after release to avoid any unnecessary sleep cycles.

### Precise Burnout Detection

The watcher thread polls every 500 µs and compares each coder's `last_compile` (read under `coder->lock`) against the current wall-clock time. To guarantee that `burned out` is the last line printed even if other threads are mid-log, the watcher sets `stop_watcher = 1` under `lock_stop` before calling `log_hanlder`. Inside `log_hanlder`, `should_stop` is checked while holding `config->print`, so no compile/debug message from another thread can appear after the burnout line.

### Edge Case: Zero Burnout Time

When `time_to_burnout == 0`, the watcher immediately calls `bournout_null`: it logs burnout for coder 1, sets `stop_watcher`, broadcasts on `cond_stop`, and wakes all coders — before any compile cycle begins. Each coder also short-circuits at `!config->time_to_burnout` right after the startup barrier, returning immediately.

### Edge Case: Single Coder

With one coder there is only one dongle but the coder needs two. The deadlock is detected early: `number_coders == 1` routes directly to `take_dongle(coder->f, coder)`, which acquires the one dongle and blocks indefinitely — the watcher detects burnout and terminates the simulation without entering the normal loop.

---

## Thread Synchronization Mechanisms

### `pthread_mutex_t` — Mutual Exclusion

Every piece of shared state is protected by a dedicated mutex:

| Mutex | File | Protected data |
|---|---|---|
| `coder->lock` | `utils.c` | `last_compile` |
| `coder->c_lock` | `utils.c` | `compiles` counter |
| `coder->wake_mtx` | `routine_utils1.c` | paired with `wake_cond` |
| `dongle->locker_d` | `add_utils.c`, `routine_utils1.c`, `heap.c` | `is_taken`, `available_at`, scheduler heap |
| `config->print` | `utils.c` | stdout serialisation |
| `config->lock_stop` | `utils.c`, `watcher.c` | `stop_watcher` flag |
| `config->time_lock` | `utils.c` | `config->start` timestamp |
| `config->operation.op_lock` | `routine_utils1.c` | startup barrier state |

Example — race-free compile counter increment:

```c
pthread_mutex_lock(&coder->c_lock);
coder->compiles++;
pthread_mutex_unlock(&coder->c_lock);
```

### `pthread_cond_t` — Condition Variables

Three condition variables implement event-driven blocking, eliminating all busy-wait loops:

**`coder->wake_cond` / `coder->wake_mtx`** — each coder sleeps here while waiting for its two dongles to become free and for itself to be first in both heaps. Any thread releasing a dongle calls `wake_up_all`, which broadcasts to every coder:

```c
// Coder sleeps (routine_utils1.c — take_dongles):
pthread_mutex_lock(&coder->wake_mtx);
while (is_valid_cond(coder->f, coder->l, coder))
{
    if (should_stop(coder->config))
		{
			pthread_mutex_unlock(&coder->wake_mtx);
			return ;
		}
    pthread_cond_wait(&coder->wake_cond, &coder->wake_mtx);
}
pthread_mutex_unlock(&coder->wake_mtx);

// Releaser wakes all coders (watcher.c — wake_up_all):
pthread_cond_broadcast(&config->coders[i].wake_cond);
```

**`config->cond_stop` / `config->lock_stop`** — signals all threads that the simulation has ended (burnout or completion detected by the watcher).

**`config->operation.op_cond` / `config->operation.op_lock`** — implements the **startup barrier**. Every coder thread and the watcher call `wait_coders` and block here until `start_coders` has spawned every thread, recorded the precise start timestamp, and broadcast the signal. This ensures all threads start measuring time from the same moment:

```c
// All threads block (routine_utils1.c — wait_coders):
pthread_cond_wait(&config->operation.op_cond, &config->operation.op_lock);

// Main thread releases all at once (start_coders.c — wait):
config->operation.is_finish = 1;
config->operation.op_coders = i;           // records how many threads were created
config->start = get_time_ms();             // single authoritative start time
pthread_cond_broadcast(&config->operation.op_cond);
```

### Per-dongle Heap as a Synchronized Priority Queue

The `t_scheduler` inside each `t_dongle` is a min-heap serving as a thread-safe wait queue. All reads and writes are done under `dongle->locker_d`. A coder checks the heap top before acquiring:

```c
// Register interest (routine_utils2.c — assign_req → req_dongle):
pthread_mutex_lock(&dongle->locker_d);
insert(dongle, coder);
pthread_mutex_unlock(&dongle->locker_d);

// Only proceed when at position 0 in both heaps (routine_utils1.c — is_valid_cond):
get_smaller(first) != coder->id || get_smaller(last) != coder->id
```

This guarantees at most one coder per dongle believes it is next at any point in time.

In EDF mode the heap also applies tie-breaking by coder ID (`is_valid_equal` in `heap_utils.c`), making the priority order fully deterministic even when two coders share the same deadline.

### How Race Conditions Are Prevented

- **Log serialisation:** `config->print` mutex wraps every `printf` in `log_hanlder`. The `should_stop` check is performed while holding this mutex, so `burned out` is atomically the last line printed — no compile/refactor message can interleave with or follow it.
- **Time reads:** `get_last_compile` and `start_time` always lock their respective mutexes before reading, preventing torn reads on 64-bit `long` values.
- **Dongle availability:** `is_dongle_avaible` reads `available_at` and `is_taken` in the same critical section under `locker_d`, preventing a TOCTOU race between the check and the acquisition performed in `take_dongle`.
- **Even/odd stagger:** Even-numbered coders sleep for half a compile duration before entering their loop (`usleep(config->time_to_compile * 1000 / 2)`). This initial desynchronisation reduces contention at startup without affecting correctness or timing guarantees.
- **Cooldown optimisation:** `release_dongle` only calls `wake_up_all` when `dongle_cooldown` is zero, avoiding redundant broadcasts when released dongles are temporarily unavailable anyway.

---

## Resources

### POSIX Threads & Concurrency

- [Multithreading in CL](https://www.geeksforgeeks.org/c/multithreading-in-c/)
- [Thread Management Functions in C](https://www.geeksforgeeks.org/c/thread-functions-in-c-c/)

### Deadlock & Scheduling Theory

- [Everything you should know about deadlock in three minutes or less](https://youtu.be/oEbXlSH8hyE?si=8KRk1fGrHR5Z1Xbm)

### Priority Queues / Binary Heap

- [Heaps & Priority Queues - Heapify, Heap Sort, Heapq Library ](https://www.youtube.com/watch?v=E2v9hBgG6gE)
- [Heap - Heap Sort - Heapify - Priority Queues](https://www.youtube.com/watch?v=HqPJF2L5h9U)

### AI Usage

- **Explain Deadlock:** simplify explaination deadlock and what cause it.
- **README Structure:** global pattern of readme file.
- **implementation Min heap:** help with implementation of min heap