# 🌌 Zyphryon Engine
*Built from the ground up. Zero compromises.*

**Zyphryon** is a high-performance, cross-platform **game engine** built entirely from scratch — no heavy dependencies, no bloated middleware, no compromises.

Every system, from the foundation library to the rendering pipeline, is custom-engineered for maximum performance and complete control.

---

## 🎯 Philosophy

Zyphryon rejects the "dependency hell" approach of modern game engines. Instead, it embraces:

- **From-scratch development** — Complete control over every subsystem
- **Zero STL dependency** — Custom foundation library optimized for game development
- **Minimal external dependencies** — Only what's absolutely necessary (platform APIs, OpenGL/D3D)
- **Data-oriented design** — Cache-friendly, SIMD-accelerated, built for modern hardware
- **Clear ownership** — Every line of code serves a purpose, nothing is hidden in black boxes

This isn't just another engine wrapper. This is **raw metal programming** with modern C++ elegance.

---

## 🛠️ Building

### Prerequisites
- **CMake** 3.20+
- **C++20** compliant compiler (MSVC 2019+, GCC 11+, Clang 13+)
- **Platform SDKs**:
  - Windows: Windows SDK 10.0+
  - Web: Emscripten SDK

### Quick Start

```bash
# Clone the repository
git clone https://github.com/yourusername/Zyphryon.git
cd Zyphryon/Engine

# Generate build files
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release
```

---

## 🎓 Design Principles

1. **Explicit is better than implicit** — No hidden allocations, no surprise virtual calls
2. **Cache locality matters** — Data-oriented layouts, minimal pointer chasing
3. **Profile, don't guess** — Optimization guided by real measurements
4. **Zero-cost abstractions** — High-level convenience without runtime penalty
5. **Fail fast** — Assertions in debug, optimized checks in release

---

## 📄 License

Zyphryon is licensed under the **MIT License** – see the [LICENSE](LICENSE) file for details.

---

## 🤝 Contributing

Contributions are welcome! This is a from-scratch project, so expect:

- **Code reviews** focused on performance and design
- **Zero external dependencies** unless absolutely justified
- **Documentation** for all public APIs
- **Tests** for critical systems

Feel free to submit Pull Requests, open Issues, or discuss new features in Discussions.

---

## 🌟 Why Zyphryon?

> *"Most engines are built on layers of abstraction that hide complexity. Zyphryon embraces complexity and controls it. This is an engine for developers who want to understand every cycle, every cache miss, every memory allocation."*

---

*Built with ❤️ and an unhealthy obsession with cache lines.*