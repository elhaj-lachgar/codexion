*This project has been created as part of the 42 curriculum by [elachgar].*

---

# Codexion

## Description

Codexion is a multithreaded problem reimagined in a developer context. A set of **coders** sit around a table, each needing two **USB dongles** (shared between adjacent coders) to compile their code. Each coder alternates between compiling, debugging, and refactoring. If a coder goes too long without compiling, they **burn out** and the simulation ends. The goal is to orchestrate shared resource access across concurrent threads with no deadlocks, no starvation, and precise burnout detection.

The project exercises core concepts of concurrent programming: mutual exclusion, condition variables, scheduling policies, and careful memory lifecycle management in C using POSIX threads.

**Simulation flow:**
- Each coder runs in its own thread and must acquire both adjacent dongles before compiling.
- A dedicated watcher thread monitors all coders for burnout and completion.
- All threads synchronize on a barrier before starting, ensuring a fair simultaneous launch.
- The simulation stops cleanly when either all coders reach the required compile count, or any coder burns out.

---

## Instructions

### Requirements

- CC
- POSIX threads support (`-pthread`)
- GNU Make

### Compilation

```bash
make
```

or 
```
make all
```
This produces the `codexion` executable. Use `make clean` to remove object files, `make fclean` to remove everything, and `make re` to rebuild from scratch.

### Execution

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <required_compiles> <dongle_cooldown> <scheduler>
```

| Argument | Description |
|---|---|
| `number_of_coders` | Number of coder threads|
| `time_to_burnout` | Max milliseconds a coder can go without compiling|
| `time_to_compile` | Duration of a compile in milliseconds |
| `time_to_debug` | Duration of a debug session in milliseconds |
| `time_to_refactor` | Duration of a refactor session in milliseconds |
| `required_compiles` | Number of compiles each coder must complete |
| `dongle_cooldown` | Milliseconds a dongle must rest after being released |
| `scheduler` | Dongle acquisition scheduling policy: `fifo` or `edf` |

### Examples

```bash
./codexion 5 800 200 100 100 7 0 fifo

./codexion 5 800 200 100 100 7 50 edf

./codexion 1 400 200 100 100 3 0 fifo
```

### Output format

Each log line is printed as:

```
<timestamp_ms> <coder_id> <state>
```

States: `has taken a dongle`, `is compiling`, `is debuging`, `is refactoring`, `burned out`.

---

## Blocking Cases Handled

### Deadlock Prevention — Coffman's Conditions

The classical deadlock scenario in a resource-ring topology (circular wait) is eliminated by enforcing a **consistent global dongle acquisition order**. Before locking, the two dongles assigned to a coder are sorted by memory address:

```c
if (coder->l < coder->f) { first = coder->l; last = coder->f; }
else                      { first = coder->f; last = coder->l; }
```

Both `req_dongle` calls always go lower-address first, then higher-address. This breaks the circular-wait condition, the fourth of Coffman's four necessary conditions, making deadlock structurally impossible regardless of scheduling.

### Starvation Prevention — Scheduler (FIFO / EDF)

Each dongle maintains a **min-heap priority queue** of waiting coders. When a coder wants a dongle it registers itself in the heap before sleeping. A coder may only grab a dongle when it is at the top of that dongle's heap (i.e., it has the highest priority), ensuring no coder is indefinitely bypassed.

Two scheduling modes are supported:

- **FIFO** — priority is the coder's request timestamp. Coders are served in arrival order, bounding wait time.
- **EDF (Earliest Deadline First)** — priority is `last_compile_time + time_to_burnout`. The coder closest to burning out is served first, minimizing burnout risk under load.

A `dup_coder` check prevents a coder from being inserted into the same dongle's heap more than once per acquisition attempt.

### Cooldown Handling

After a dongle is released it enters a cooldown period (`dongle_cooldown` ms) during which `is_dongle_avaible` returns false:

```c
dongle->available_at = get_time_ms() + config->dongle_cooldown;
```

This cooldown is checked atomically under `locker_d` to avoid races between the availability check and the is-taken flag.

### Precise Burnout Detection

The watcher thread polls every 500 µs and compares each coder's `last_compile` timestamp (read under `coder->lock`) against the current time. Detection is sub-millisecond in practice. To guarantee that `burned out` is the last line printed even if other threads are mid-log, the watcher sets `stop_watcher = 1` under `lock_stop` before calling `log_hanlder`, and `log_hanlder` checks `should_stop` under `config->print` before writing — ensuring no later compile/debug lines can appear after burnout.

### Edge Case: Zero Burnout Time

When `time_to_burnout == 0`, the watcher immediately logs burnout for coder 1 and signals all threads to stop before any compile cycle begins. Each coder thread checks `should_stop` at the start of every action and exits cleanly.

### Edge Case: Single Coder

With one coder and one dongle, the coder acquires its single dongle and then blocks forever waiting for the second (which is itself). This is detected: `number_coders == 1` triggers a direct burnout path instead of the normal compile loop.

---

## Thread Synchronization Mechanisms

### `pthread_mutex_t` — Mutual Exclusion

Every shared field is protected by a dedicated mutex:

| Mutex | Protected data |
|---|---|
| `coder->lock` | `last_compile`, `is_waiting` |
| `coder->c_lock` | `compiles` counter |
| `coder->wake_mtx` | paired with `wake_cond` |
| `dongle->locker_d` | `is_taken`, `available_at`, scheduler heap |
| `config->print` | stdout (prevents interleaved log lines) |
| `config->lock_stop` | `stop_watcher` flag |
| `config->time_lock` | `config->start` timestamp |
| `config->operation.op_lock` | startup barrier state |

Example — race-free compile counter increment:

```c
pthread_mutex_lock(&coder->c_lock);
coder->compiles++;
pthread_mutex_unlock(&coder->c_lock);
```

### `pthread_cond_t` — Condition Variables

Three condition variables implement event-driven blocking, avoiding busy-wait loops:

**`coder->wake_cond` / `coder->wake_mtx`** — each coder sleeps here while waiting for its two dongles to become available and for itself to be at the top of both heaps. The watcher and any coder releasing a dongle broadcast to all `wake_cond` variables:

```c
// Coder sleeps:
pthread_mutex_lock(&coder->wake_mtx);
while (!is_dongle_avaible(first) || get_smaller(first) != coder->id ...)
    pthread_cond_wait(&coder->wake_cond, &coder->wake_mtx);

// Releaser wakes everyone:
pthread_cond_broadcast(&config->coders[i].wake_cond);
```

**`config->cond_stop` / `config->lock_stop`** — signals all threads that the simulation is over (burnout or completion).

**`config->operation.op_cond` / `config->operation.op_lock`** — implements a startup barrier. All coder threads and the watcher block here until the main thread has spawned every thread and set the precise start timestamp, ensuring simultaneous and fair launch:

```c
// All threads block:
pthread_cond_wait(&config->operation.op_cond, &config->operation.op_lock);

// Main thread releases everyone at once:
config->operation.is_finish = 1;
pthread_cond_broadcast(&config->operation.op_cond);
```

### Per-dongle Heap as a Synchronized Queue

The min-heap inside each `t_dongle` acts as a thread-safe priority queue. All reads and writes to the heap are performed under `dongle->locker_d`. A coder checks the heap top before taking the dongle:

```c
pthread_mutex_lock(&dongle->locker_d);
if (!dup_coder(dongle, coder->id))
    insert(dongle, coder);          // enqueue with priority
pthread_mutex_unlock(&dongle->locker_d);
// ...later, only take if get_smaller() == coder->id
```

This guarantees that at most one coder believes it is next in line for any given dongle at any point in time.

### How Race Conditions Are Prevented

- **Log serialization:** `config->print` mutex wraps every `printf`. The stop check inside `log_hanlder` is also done under this mutex, so `burned out` is atomically the last message.
- **Time reads:** `get_time_ms()` returns wall-clock time with no shared state; all derived timestamps are stored under per-field mutexes.
- **Dongle availability:** `is_dongle_avaible` locks `locker_d` to read both `available_at` and `is_taken` atomically, preventing a TOCTOU race between the check and the acquisition.
- **Even/odd stagger:** Even-numbered coders sleep for half a compile duration before entering their loop. This initial desynchronization reduces contention at startup without affecting correctness.

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