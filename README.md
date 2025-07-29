# QuarterLang-Pyrus-Edition

## # QuarterLang (Pyrus Edition) Spec Sheet & Overview

---

## Overview

QuarterLang is a general-purpose, systems-level programming language that blends functional, imperative, and itemized paradigms into an English-inspired, highly readable syntax. It compiles Ahead-Of-Time (AOT) to native binaries, supports a split/toggled Just-In-Time (JIT) layer, and integrates inline NASM for low-level control. Capsules—self-contained execution bundles—carry code, metadata, and Dodecagram (DG) annotations for traceable, high-performance deployment across bare-metal, FPGA, WebAssembly, and hosted environments. 

---

## Language Design Paradigms

- Itemized Transformative Objective (ITO)  
- Functional Directive (FD)  
- Sequential Iterative Imperative (SII)  
- Lateral Procedural (LP)  
- Longitudinal Scope Full Stack (LSFS)  

These five paradigms govern structure, data flow, and scope resolution, ensuring consistency from high-level scripts down to machine instructions. 

---

## Program Structure

- Every program begins with `star` and ends with `end`.  
- Blocks opened by keywords (`define`, `loop`, `match`, etc.) must be explicitly closed.  
- Virtual indentation and zoomed spacing replace mandatory whitespace rules, while still enforcing clear nesting. 

---

## Core Syntax

### Keywords

| Keyword   | Purpose                                 |
|-----------|-----------------------------------------|
| star      | Begin program or capsule block          |
| end       | End program or capsule block            |
| val       | Declare immutable variable              |
| var       | Declare mutable variable                |
| derive    | Create a new value from an existing one |
| dg        | Annotate a base-12 Dodecagram literal   |
| say       | Print to standard output                |
| loop      | Iterate over ranges                     |
| when      | `if`-style conditional                  |
| else      | Alternate branch for `when`             |
| match     | Pattern matching on values              |
| case      | Single branch in a `match`              |
| define    | Named function declaration              |
| procedure | Alternative to `define`                 |
| return    | Return from a function                  |
| yield     | Yield from generator or coroutine       |
| fn        | Anonymous (lambda) function             |
| thread    | Spawn a parallel thread                 |
| pipe      | Pipe output to a stream or file         |
| nest      | Inline block encapsulation              |
| asm       | Embed inline NASM assembly              |
| stop      | Immediate runtime halt                  |

### Data Types

| Type    | Description                       |
|---------|-----------------------------------|
| int     | 32-bit signed integer             |
| float   | 32-bit floating-point             |
| bool    | `true` or `false`                 |
| string  | UTF-8 text                        |
| dg      | Base-12 Dodecagram numeric literal|

### Variables & Constants

```quarter
val   x as int:   10
var   y as float: 2.5
say   "Value is {x} and {y}"
```

### Derivatives

```quarter
val   base as int: 20
derive speed from base by 5    # speed = base * 5
say   speed                   # outputs 100
```

### DG (Dodecagram) Numeric System

- Digits: `0–9`, `A` = 10, `B` = 11  
- Base-12 literals: prefixed with `dg:` or suffixed as type  
- Conversion and arithmetic via built-in functions  

| Function         | Signature           | Description                        |
|------------------|---------------------|------------------------------------|
| `to_dg(n)`       | `Int → DG`          | Convert integer to DG literal      |
| `from_dg(d)`     | `DG → Int`          | Convert DG literal to integer      |
| `dg_add(a,b)`    | `DG, DG → DG`       | Add two DG values                  |
| `dg_mul(a,b)`    | `DG, DG → DG`       | Multiply two DG values             |

---

## Control Flow

| Construct       | Syntax                             |
|-----------------|------------------------------------|
| Loop            | `loop from <start> to <end>:`…     |
| Conditional     | `when <expr>:`… `else:`…          |
| Pattern Match   | `match <expr>:`<br>`case <pat>:`…  |

```quarter
loop from 1 to 5:
  say "Count {i}"
when x > 0:
  say "Positive"
else:
  say "Non-positive"

match value:
  case 1: say "One"
  case (a,b): say "Pair {a},{b}"
```

---

## Functions & Procedures

```quarter
define add (a as int, b as int): int
  return a + b

let square = fn(x: int): int
  return x * x
```

- Named (`define`/`procedure`) and anonymous (`fn`) functions  
- Support for `return` and streaming `yield` semantics  

---

## Concurrency & I/O

- `thread <name>() … end` spawns lightweight threads  
- `pipe write: "file.txt"` directs capsule output to files or streams  
- Asynchronous I/O integrated into the runtime  

---

## Inline Assembly

```quarter
asm {
  mov rax, 1
  inc rax
}
```

- Embed raw NASM within capsules  
- Opcodes and DG comments interleaved for symbolic tracing  

---

## Capsules & Modules

- Source files compile into `.qtrcapsule` bundles  
- Structure:

  - Magic header (`QTRC` signature + version)  
  - Compressed payload: source, NASM, DG metadata  
  - Execution metadata: hash, author, timestamp  

- Capsules run via interpreter or direct loader, support snapshotting, rewinds, and health metrics  

---

## Runtime Built-ins

| Function       | Description                              |
|----------------|------------------------------------------|
| `say(...)`     | Print formatted text                     |
| `to_dg(...)`   | Integer → DG conversion                  |
| `from_dg(...)` | DG → Integer conversion                  |
| `dg_add(...)`  | DG addition                              |
| `dg_mul(...)`  | DG multiplication                        |
| `read_file(...)` / `write_file(...)` | File I/O         |
| `show_health()`| Emit capsule health metrics (entropy, memory) |

---

## Execution Model

- AOT compilation to native machine code  
- Split/toggled JIT for hotspots and dynamic linking  
- Hybrid pipeline allows direct LLVM IR bridge or custom backend for aggressive optimization  

---

## Compilation Pipeline

```
┌───────────┐   ┌─────────┐   ┌─────────┐   ┌─────────┐   ┌─────────┐
│ Source    │ → │ Lexer   │ → │ Parser  │ → │ DG IR   │ → │ NASM/IR │
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

## Tooling & Developer Experience

- Multi-user REPL with live opcode tracing  
- Visual capsule debugger with memory inspector  
- Plugin hooks for diagnostics, AI modeling, and IDE integrations  
- Hot-reloadable modules and FFI bridges (WASM, .NET)  

---

## Interoperability & Targets

- DCIL↔LLVM transpilation for hybrid optimization  
- WebAssembly and browser sandbox targets  
- FPGA-deployable ISA variant  
- Bare-metal executables across x86-64, ARM, and custom architectures  

---

## Security & Validation

- Pre-execution opcode verifier to catch malformed instructions  
- Capsule sandboxing: isolated memory, I/O, and threading  
- Symbolic permission maps embedded in metadata  
- Source hashing for authenticity and immutability  

---

## Extensibility & Versioning

- Versioned opcode sets for backward compatibility  
- Pluggable macros and capsule injectors  
- DG algebraic extensions (symbolic simplification, base-12 floats)  
- Capsule rewind/replay API for deterministic debugging  

---

## Example Program

```quarter
star
  val sum as int: 0
  loop from 1 to 10:
    derive sum from sum by i
  say "Total: {sum}"
end
```

Execution Trace (simplified):

```
OP_LOOP 0x22 → OP_DERIVE 0x38 → OP_SAY 0x4A → OP_END 0xFF
```

---

## Glossary

| Term         | Definition                                                 |
|--------------|------------------------------------------------------------|
| Capsule      | Self-contained executable bundle with code + metadata      |
| DG (Dodecagram) | Base-12 numeric system with symbolic tracing            |
| Derivative   | Transformative assignment deriving new values from existing|
| ITO          | Itemized Transformative Objective paradigm                |
| LSFS         | Longitudinal Scope Full Stack paradigm                    |

---

