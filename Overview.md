**QuarterLang (Pyrus Edition)** is a general-purpose, systems-level programming language designed with a hyper-readable English-like syntax. It fuses **functional**, **imperative**, and **itemized** paradigms, enabling both high-level abstraction and low-level control. QuarterLang compiles **Ahead-Of-Time (AOT)** into native binaries, supports a **toggleable JIT**, and embeds **inline NASM** for ultimate control.

It introduces **Capsules**—self-contained execution bundles that combine source code, metadata, and **Dodecagram (DG)** base-12 annotations—for high-performance, traceable execution across **bare-metal**, **FPGA**, **WebAssembly**, and **modular OS environments**.

---

## 🔹 Language Design Paradigms

QuarterLang integrates five core paradigms:

* **ITO**: *Itemized Transformative Objective*
* **FD**: *Functional Directive*
* **SII**: *Sequential Iterative Imperative*
* **LP**: *Lateral Procedural*
* **LSFS**: *Longitudinal Scope Full Stack*

These ensure structural rigor from high-level scripting to machine-level optimization.

---

## 🔹 Program Structure

* All programs begin with `star` and end with `end`.
* Blocks must be explicitly closed.
* Virtual indentation (zoomed spacing) replaces mandatory whitespace.

---

## 🔹 Core Syntax

### Keywords

| Keyword     | Purpose                       |
| ----------- | ----------------------------- |
| `star`      | Begin a program/capsule block |
| `end`       | End a block                   |
| `val`       | Declare an immutable variable |
| `var`       | Declare a mutable variable    |
| `derive`    | Transformative assignment     |
| `dg`        | Annotate Dodecagram base-12   |
| `say`       | Print to standard output      |
| `loop`      | Iterate over ranges           |
| `when`      | Conditional branch            |
| `else`      | Else branch                   |
| `match`     | Pattern matching block        |
| `case`      | Pattern branch inside match   |
| `define`    | Function declaration          |
| `procedure` | Alternative to define         |
| `return`    | Return from a function        |
| `yield`     | Yield value (generators)      |
| `fn`        | Anonymous function            |
| `thread`    | Spawn parallel thread         |
| `pipe`      | Pipe output to stream/file    |
| `nest`      | Inline block encapsulation    |
| `asm`       | Inline NASM                   |
| `stop`      | Runtime halt                  |

---

### Data Types

| Type     | Description                 |
| -------- | --------------------------- |
| `int`    | 32-bit signed integer       |
| `float`  | 32-bit floating-point       |
| `bool`   | Boolean (`true` or `false`) |
| `string` | UTF-8 encoded string        |
| `dg`     | Base-12 Dodecagram literal  |

---

### Example: Variables & Derivatives

```quarterlang
val x as int: 10
var y as float: 2.5
say "Value is {x} and {y}"

val base as int: 20
derive speed from base by 5
say speed  # Outputs: 100
```

---

## 🔹 Dodecagram (DG) Base-12 System

* Digits: `0–9`, `A`=10, `B`=11
* Use `dg:` prefix or `as dg` for DG literals

| Function       | Signature     | Description        |
| -------------- | ------------- | ------------------ |
| `to_dg(n)`     | `Int → DG`    | Convert int to DG  |
| `from_dg(d)`   | `DG → Int`    | Convert DG to int  |
| `dg_add(a, b)` | `DG, DG → DG` | Add DG values      |
| `dg_mul(a, b)` | `DG, DG → DG` | Multiply DG values |

---

## 🔹 Control Flow

```quarterlang
loop from 1 to 5:
    say "Count {i}"

when x > 0:
    say "Positive"
else:
    say "Non-positive"

match value:
    case 1: say "One"
    case (a, b): say "Pair {a},{b}"
```

---

## 🔹 Functions & Procedures

```quarterlang
define add (a as int, b as int): int
    return a + b

let square = fn(x: int): int
    return x * x
```

---

## 🔹 Concurrency & I/O

* `thread name():` launches a lightweight thread
* `pipe write: "out.txt"` redirects capsule output
* Asynchronous runtime I/O supported natively

---

## 🔹 Inline Assembly

```quarterlang
asm {
    mov rax, 1
    inc rax
}
```

Supports symbolic DG comments and direct instruction injection.

---

## 🔹 Capsules & Modules

* Files compile into `.qtrcapsule`
* Structure:

  * Magic Header (`QTRC`)
  * Payload: source + inline NASM + DG annotations
  * Metadata: author, time, integrity hash
* Supports: **rewind, snapshot, health trace, JIT fallback**

---

## 🔹 Runtime Built-ins

| Function                           | Description                      |
| ---------------------------------- | -------------------------------- |
| `say(...)`                         | Print formatted output           |
| `to_dg(...)`                       | Convert int to DG                |
| `from_dg(...)`                     | Convert DG to int                |
| `dg_add(...)`                      | Add DG values                    |
| `dg_mul(...)`                      | Multiply DG values               |
| `read_file(...)`/`write_file(...)` | File I/O                         |
| `show_health()`                    | Capsule health (entropy, memory) |

---

## 🔹 Execution Model

* **AOT compilation** with optional **JIT toggle**
* Custom pipeline OR LLVM IR bridge
* Optimizations:

  * Peephole & loop unrolling
  * Constant folding, propagation
  * Register scheduling
  * Instruction timeline mutation

---

## 🔹 Compilation Pipeline

```
┌───────────┐   ┌─────────┐   ┌─────────┐   ┌─────────┐   ┌─────────┐
│  Source   │ → │  Lexer  │ → │ Parser  │ → │  DG IR   │ → │ NASM/IR │
└───────────┘   └─────────┘   └─────────┘   └─────────┘   └─────────┘
                                         ↓
                                     Optimizer
                                         ↓
                                   Bytecode/Hex
                                         ↓
                                    AOT Emitter
                                         ↓
                                  Native Binary
```

---

## 🔹 Tooling & Developer Experience

* Multi-user REPL with live opcode tracing
* Visual debugger with capsule graphing
* Plugin hooks for:

  * Diagnostics
  * AI modeling
  * IDE extensions
* Hot-swapping modules
* FFI to WebAssembly, .NET, Vulkan

---

## 🔹 Interoperability & Targets

* Transpile between **DCIL ↔ LLVM**
* Target WebAssembly, sandboxed browsers
* Deploy to:

  * x86-64 / ARM / RISC-V
  * FPGAs, Bare-metal
  * Quantum-assisted VM (experimental)

---

## 🔹 Security & Validation

* Pre-execution verification
* Opcode sanitation & DG integrity check
* Permission maps in metadata
* Capsule signature & hash seal

---

## 🔹 Extensibility & Future Features

* DG floating-point (base-12) support
* Quantum speculative entropy maps
* GPU offloading with shader tracing overlays
* Neural opcode pruning & tuning
* Live training persistence
* Predictive tensor execution graphs
* Capsule-memetic instruction lattice
* Holographic introspection overlays
* Temporal inference feedback per opcode

---

## 🔹 Sample Program

```quarterlang
star
val sum as int: 0
loop from 1 to 10:
    derive sum from sum by i
say "Total: {sum}"
end
```

### Execution Trace

```
OP_LOOP 0x22 → OP_DERIVE 0x38 → OP_SAY 0x4A → OP_END 0xFF
```

---

## 🔹 Glossary

| Term       | Definition                                              |
| ---------- | ------------------------------------------------------- |
| Capsule    | Executable unit: source + metadata + DG + compiled code |
| Dodecagram | Base-12 symbolic numeric format                         |
| Derivative | Transformative operation for new variable assignments   |
| ITO        | Itemized Transformative Objective paradigm              |
| LSFS       | Longitudinal Scope Full Stack paradigm                  |

---

This constitutes the complete specification and overview for **QuarterLang (Pyrus Edition)**.

