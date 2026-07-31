# 🌌 Zyphryon Engine
*Built from the ground up.*

**Zyphryon** is a high-performance, cross-platform **game engine** written from scratch in C++20.

---

## 🎯 Philosophy

- **From-scratch subsystems** — The entire stack is first-party, foundation to renderer.
- **Minimal standard library surface** — The STL appears only inside `Zyphryon.Base`,
  and only where the language leaves no alternative:
  - *compiler and language support* — `<type_traits>`, `<concepts>`, `<bit>`, `<new>`,
    `<cstdint>`, `<limits>`
  - *math* — `<cmath>`
  - *concurrency* — `<atomic>`, `<thread>`, `<mutex>`, `<condition_variable>`
- **Minimal dependencies** — The runtime carries two third-party libraries: the
  [flecs](https://github.com/SanderMertens/flecs) ECS and
  [libopus](https://opus-codec.org/) for audio decoding. Anything else lives in the
  tools, never ships with the engine.
- **Data-oriented design** — Cache-friendly layouts, SIMD math, structures shaped by
  access patterns rather than taxonomy.
- **Explicit ownership** — No hidden allocations, no surprise virtual calls, no work
  the caller didn't ask for.

---

## 📦 Dependencies

### What ships in your game

A game built on Zyphryon links **two** third-party libraries by default. Everything else below arrives only if
you enable the module that needs it.

| Library | Pulled in by | Used for |
|---------|--------------|----------|
| [flecs](https://github.com/SanderMertens/flecs) | always (core) | The ECS behind the `Scene` module |
| [libopus](https://opus-codec.org/) | always (core) | Opus decoding for the `.snd` container |
| [glad](https://github.com/Dav1dde/glad) | `ZY_GRAPHIC_DRIVER_GLES3` *(ON on Unix)* | GL entry-point loading |
| [Tracy](https://github.com/wolfpld/tracy) | `ZY_PROFILE_BACKEND_TRACY` *(OFF)* | Frame profiling |
| [stb](https://github.com/nothings/stb) | `ZY_GRAPHIC_LOADER_STB` *(OFF)* | Runtime image decode |

### What doesn't

The offline bakers under `Pipeline/Baker/` are standalone executables you run on your own machine. Nothing links
against them, and they only build with `ZY_BUILD_PIPELINE` *(OFF)*.

| Library | Used by | Used for |
|---------|---------|----------|
| [dr_libs](https://github.com/mackron/dr_libs) | Audio baker | WAV / MP3 decode |
| [msdfgen](https://github.com/Chlumsky/msdfgen) | Font baker | MSDF glyph generation |
| [stb](https://github.com/nothings/stb) | Font, Texture bakers | Image decode |

Ship the baked assets and these stay on your workstation.

---

## 🛠️ Building

### Prerequisites
- **CMake** 3.20+
- **C++20** compliant compiler (MSVC 2019+, GCC 11+, Clang 13+)
- **Platform SDKs**
    - Windows: Windows SDK 10.0+
    - Linux: `libasound2-dev`, `libx11-dev`, `libxrandr-dev` *(all required)*
    - Web: Emscripten SDK

---

## 🎓 Design Principles

1. **Explicit is better than implicit** — No hidden allocations, no surprise virtual calls
2. **Cache locality matters** — Data-oriented layouts, minimal pointer chasing
3. **Profile, don't guess** — Optimization guided by real measurements
4. **Zero-cost abstractions** — High-level convenience without runtime penalty
5. **Fail fast** — Assertions in debug, optimized checks in release

---

## 🚧 Status

Zyphryon is under active development and pre-1.0. Expect APIs to move.

Known gaps, so you don't have to discover them:

- **No automated test suite yet.** Correctness is currently verified by hand and by the sample project.
- **The 3D path is young** — skeletal animation and model rendering are still being built out.

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
