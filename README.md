<div align="center">

  ![REDasm open source disassembler logo](https://github.com/user-attachments/assets/5a7005f7-43de-45d7-97ba-e14996dc0ed0)

  ![License GPL-3.0](https://img.shields.io/badge/license-GPL--3.0-critical?style=flat-square)
  ![Version 4.0.0](https://img.shields.io/badge/version-4.0.0-blue?style=flat-square)
  [![Website redasm.dev](https://img.shields.io/badge/website-redasm.dev-informational?style=flat-square)](https://redasm.dev)
  [![X @re_dasm](https://img.shields.io/badge/@re__dasm-black?style=flat-square&logo=x)](https://twitter.com/re_dasm)

  [![CI](https://github.com/redasm-dev/workspace/actions/workflows/ci.yml/badge.svg)](https://github.com/redasm-dev/workspace/actions/workflows/ci.yml)
  [![Nightly](https://github.com/redasm-dev/workspace/actions/workflows/nightly.yml/badge.svg)](https://github.com/redasm-dev/workspace/actions/workflows/nightly.yml)

</div>

REDasm is a disassembler and binary analysis tool for Windows and Linux,
built for both hobbyists and professional reverse engineers.  
It supports various CPU architectures and executable formats
(see [Supported Formats and Architectures](#supported-formats-and-architectures)).  
It also offers an interactive listing, control flow graph view, cross-references, string detection and automatic
function recovery.

<div align="center">

  ![REDasm disassembler user interface showing the disassembly listing and graph view](https://github.com/user-attachments/assets/46eb03e4-1791-4216-85d8-82fef8056fc0)

</div>

---

## Table of Contents

- [Design Principles](#design-principles)
- [Features](#features)
- [Supported Formats and Architectures](#supported-formats-and-architectures)
- [Download](#download)
- [Contributing](#contributing)
- [FAQ](#faq)
- [License](#license)

---

## Design Principles

- **Open by default**: GPL core & plugins, public C API. Everything can be read,
  modified and rebuilt.
- **Native, self-contained core**: [libredasm](https://github.com/redasm-dev/core) is written in C17 and 
  doesn't depends on VM, runtime or interpreter. 
- **No platform lock-in**: Windows and Linux are what's actively developed and tested;
  support for other platforms is open to anyone willing to contribute the changes they need.
- **Extendable**: plugins are shared libraries loaded at runtime. Adding a CPU architecture or a file format 
  never requires touching the core.
- **First class native GUI**: written with Qt6 and sits entirely above core API. The interactive listing, 
  graph view and navigation are designed together with the engine, not layered on afterwards.
- **First class support for retro and legacy formats**: DOS, Win16, OS/2 era and console binaries are part of REDasm experience.
  This is what makes software preservation, recovery and porting possible for binaries whose source code is long gone.

## Features

- Interactive **disassembly listing** with renaming, commenting and typed data.
- **Control flow graph** view at function granularity, synchronized with the listing.
- **Cross-references** for code and data, with navigable history.
- **RDIL**, a minimal, architecture-neutral, intermediate language used for analysis and lifting.
- Automatic **string detection** (ASCII, UTF-16 and Latin-1 wide strings).
- **Type system** with structs, unions, enums and typedefs, applied directly to the listing.
- Integrated **hex view**.
- **Segments**, imports/exports, symbols, strings and problems panels.
- **Project save/load** so analysis, renames and comments survive between sessions.
- **Patching** and export of analysis data.
- Light and dark **themes**.

## Supported Formats and Architectures

Maturity levels: **S** production ready · **A** highly stable · **B** functional · **C** basic / stub · **D** wip / experimental

### Loaders (executable formats)

| Format | Description | Status |
|---|---|:---:|
| **PE / PE+** | Windows executables and DLLs, 32-bit and 64-bit | **S** |
| **ELF / ELF64** | Linux, BSD and Unix executables and shared objects | **A** |
| **MZ (DOS)** | MS-DOS executables and COM programs | **A** |
| **NE (Win16)** | 16-bit Windows and OS/2 New Executable | **B** |
| **LE / LX (OS/2)** | Linear Executable, OS/2 and DOS extenders (VxD is supported too) | **B** |
| **PSX EXE (PS1)** | PlayStation 1 executables | **B** |
| **PSX BIOS (PS1)** | PlayStation 1 BIOS | **B** |
| **XBE (Xbox)** | Original Xbox executables | **B** |

### Processors (CPU architectures)

| Architecture | Description | Status |
|---|---|:---:|
| **x86 / x86_64** | 16-bit, 32-bit and 64-bit Intel/AMD | **S** |
| **MIPS** | MIPS32, big and little endian, delay slot aware | **A** |
| **ARM / Thumb** | ARM32 and Thumb instruction sets | **B** |
| **ARM64** | AArch64 | **A** |

### Analyzers

| Analyzer | Description | Status |
|---|---|:---:|
| **Visual Basic** | Recover VB5/VB6 events and project information | **A** |
| **MSVC RTTI** | MSVC run-time type information and vtable recovery | **D** |

## Download

Pre-built binaries are published on the [Releases page](https://github.com/redasm-dev/redasm/releases):

- **Linux**: AppImage, runs on any reasonably recent distribution.
- **Windows**: portable build, no installation required.

Nightly builds are produced automatically from `master` and are GPG signed.  
They track development closely and **may be unstable**.

## Building from Source

REDasm is split across several repositories: the [workspace](https://github.com/redasm-dev/workspace) repo fetches and builds all of them together.  
Requirements, build steps and version pinning are documented in the
[workspace README](https://github.com/redasm-dev/workspace).

## Contributing

Bug reports, feature requests and pull requests are welcome.   
All reports are now centralized at [in a dedicated repo](https://github.com/redasm-dev/bugs), so report there regardless of which component the problem
belongs to, whether it's the GUI, the core or a plugin.

## FAQ

**Is REDasm free?**  
Yes. The whole project is GPL-3.0, including every bundled plugin.  
You can use it, study it, modify it
and redistribute it under the terms of the license.

**Does REDasm have a decompiler?**  
Not yet. The foundations like RDIL, function boundaries, basic blocks, the type system are already in
place, but decompilation is a long-term goal, some important features are still missing.

**Which platforms are supported?**  
Windows and Linux.  
Those are the platforms that can be actively tested, so they are the ones that get support.  
Nothing in the codebase is tied to them, though, if you need REDasm on another platform, 
open an issue or send the changes required, and support can be extended from there.

**Can I script REDasm?**  
A Python plugin API is on the roadmap.  
Today, extensions are written in C.

**Can I run REDasm headless as library?**  
Yes, this repo contains only the GUI, it's still the main repo for historical reasons.  
REDasm engine lives in [core](https://github.com/redasm-dev/core) repo and contains what's needed
to analyze binaries and load plugins.

**What happened to version 3?**  
Retired because of technical debt and engine limitations, version 4 is a complete plain C rewrite 
with a more powerful and fast analysis engine.

## License

REDasm is released under the [GNU General Public License v3.0](LICENSE).
