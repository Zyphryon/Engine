# 🌌 Zyphryon Engine
*Fast. Tight. Built from the ground up.*

**Zyphryon** is a high-performance, cross-platform **game engine** written from scratch in C++23. Every
subsystem is first-party, from the containers to the renderer, and every one is measured.

---

## ⚡ Performance

The same workloads, run in each engine and checked against the same checksums, on one machine.

| Workload | Zyphryon | Bevy 0.20 | Godot 4.7 |
|----------|---------:|----------:|----------:|
| Iterate 1M entities | **0.84 ms** | 1.00 ms | 215 ms |
| Move and frustum-cull 1M entities, spread over every core | **2.47 ms** | 2.65 ms | 39.0 ms |
| Spawn and despawn 100k entities | **1.42 ms** | 2.48 ms | 127 ms |
| Propagate a 110k-node hierarchy | **0.23 ms** | 0.41 ms | 21.1 ms |
| Four systems over 100k entities, in parallel | **0.08 ms** | 0.16 ms | 14.9 ms |
| Save a 100k-entity world | **14.5 ms** | 390 ms | — |
| Draw 100k moving sprites | **1.09 ms** | 12.5 ms | 53.2 ms |
| Draw 10k text labels, rewritten every frame | **3.51 ms** | 62.8 ms | — |
| Cull 1M sprites | **4.79 ms** | 14.5 ms | — |
| Schedule and wait on 10k tiny jobs | **1.92 ms** | 5.98 ms | — |
| Start to the first presented frame | **201 ms** | 532 ms | — |

And it stays small while doing it:

| Peak memory | Zyphryon | Bevy 0.20 | Godot 4.7 |
|-------------|---------:|----------:|----------:|
| Iterate 1M entities | **61 MB** | 124 MB | 1467 MB |
| Draw 100k moving sprites | **87 MB** | 586 MB | 633 MB |

Median work per frame on a Ryzen 7 5700X, RTX 3070 and Windows 10, release builds, measured in October 2026.

---

## 🎯 Philosophy

- **Explicit over implicit** — No hidden allocations, no surprise virtual calls, no work the caller didn't ask for.
- **Cache locality first** — Data-oriented layouts, SIMD math, structures shaped by how they are read rather than
  by taxonomy.
- **Measure, don't guess** — A change that claims to be faster comes with numbers, and one that measures as noise
  is taken back out.
- **Zero-cost abstractions** — High-level convenience with no runtime penalty.
- **Fail fast** — Assertions catch misuse in debug builds, and loaders reject malformed data in every build.
- **A minimal standard library surface** — The STL appears only inside `Zyphryon.Base`, and only where the
  language leaves no alternative:
  - *compiler and language support* — `<type_traits>`, `<concepts>`, `<bit>`, `<new>`, `<cstdint>`, `<limits>`
  - *math* — `<cmath>`
  - *concurrency* — `<atomic>`; threads, mutexes and condition variables are the engine's own, over the OS.

---

## 🧩 What's inside

| Module | What it does |
|--------|--------------|
| **Base** | Containers, strings, smart pointers, reflection, binary and JSON marshalling, LZ4, threads and logging. |
| **Math** | SIMD vectors, matrices and quaternions, collision, curves, geometry, tweens and easing. |
| **Job** | A worker pool, one worker per core, with main-thread, compute and IO lanes. |
| **Scene** | An archetype ECS: queries, systems that spread over the workers when it pays, prefabs, hierarchies, pull-based change lists and world saves. |
| **Graphic** | A low-level GPU layer with a background submission thread, over Direct3D 11 or OpenGL 3.3-4.6 and WebGL 2. |
| **Render** | The renderer: render graph, sorted draw recording, sprites and sheets, MSDF fonts, models and skeletal animation. |
| **Audio** | An in-house mixer with spatial voices, filters and resampling, decoding Opus and PCM. |
| **Content** | Asynchronous asset loading from disk, memory or remote mounts, with dependency tracking. |
| **Platform**, **Input** | Windows, displays and timers per OS; keyboard, mouse, touch and named action mappings. |
| **Network** | TCP and UDP transport on its own thread, reporting to the main thread. |
| **AI** | Behaviour trees with a shared blackboard. |
| **Runtime** | The kernel that brings the services up, runs the frame loop and tears it all down. |

Assets are baked offline into native binary formats that load without any conversion at runtime.

---

## 📦 Dependencies

### What ships in your game

A game built on Zyphryon links **one** third-party library by default. Everything else below arrives only if
you enable the module that needs it.

| Library | Pulled in by | Used for |
|---------|--------------|----------|
| [libopus](https://opus-codec.org/) | always (core) | Opus decoding for the `.snd` container |
| [Tracy](https://github.com/wolfpld/tracy) | `ZY_PROFILE_BACKEND_TRACY` *(OFF)* | Frame profiling |

No graphics loader ships either: the OpenGL driver loads its own entry points at runtime.

---

## 🛠️ Building

### Prerequisites
- **CMake** 3.26+ and **Ninja**
- **C++23** compliant compiler (MSVC 2019+, GCC 11+, Clang 19+)
- **Platform SDKs**
    - Windows: Windows SDK 10.0+
    - Linux: `libasound2-dev`, `libx11-dev`, `libxrandr-dev` *(no GL development package needed)*
    - Web: Emscripten SDK
- **Target platforms**
    - Windows 10 version 1607 or later, Direct3D 11 or OpenGL
    - Linux on X11, OpenGL
    - Web, WebGL 2
    - Headless, for dedicated servers

### Quick start

```
cmake --preset release
cmake --build --preset release
```

| Preset | Builds |
|--------|--------|
| `debug` | Unoptimized and instrumented, with AddressSanitizer under GCC and Clang |
| `relwithdebinfo` | Optimized, without whole-program optimization, for quick iteration |
| `release` | Fully optimized and stripped |
| `headless` | No window, input, graphics or audio, for a dedicated server |
| `profile` | Lights up the profiling macros against a Tracy client |
| `web` | WebAssembly, with an activated emsdk |

---

## 🚧 Status

Zyphryon is under active development and pre-1.0. Expect APIs to move.

Known gaps, so you don't have to discover them:

- **No automated test suite yet.** Correctness is currently verified by hand, by the sample project and by the
  checksums of the benchmark suite.
- **The 3D path is young** — skeletal animation and model rendering are still being built out.
- **No macOS or mobile yet.**

---

## 📄 License

Zyphryon is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

## 🤝 Contributing

Contributions are welcome. Expect:

- **Code reviews** focused on performance and design
- **New dependencies to need justification** — not forbidden, but each one has to earn its place and gets
  listed in the table above
- **Documentation** for all public APIs
- **Measurements** rather than assertions when a change is claimed to be faster

Feel free to submit Pull Requests, open Issues, or discuss new features in Discussions.

---

*Built with ❤️ and an unhealthy obsession with cache lines.*
