# AMD Hawaii support

This page tracks downstream changes for AMD Hawaii / Sea Islands (GCN2, `gfx701`), including FirePro W8100-class devices. Hawaii is distinct from gfx803/GCN4.

## Status

- **OpenCL:** Hawaii path builds and has completed current inference validation on FirePro W8100 hardware. Supported operations are intentionally limited; unsupported graph nodes remain on the CPU.
- **HIP/ROCm gfx701:** Experimental investigation only. No HIP implementation, gfx701 compile, or HIP runtime result is claimed.

## Build

Configure and build with CMake (example for Windows):

```powershell
cmake -B build -DGGML_OPENCL=ON -DGGML_OPENCL_TARGET_VERSION=200 -DGGML_OPENCL_USE_ADRENO_KERNELS=OFF
cmake --build build --config Release
```

The backend uses the OpenCL ICD installed for the target device. The validated environment used Windows, AMD APP 1800.5 (OpenCL 2.0), and GCC 16.2.0 via MSYS2. Other drivers and operating systems have not been validated. OpenCL 2.0 targeting avoids importing APIs unavailable in the AMD APP runtime; Adreno-specific kernels must be disabled for Hawaii.

## Validation record

Current validation used an AMD FirePro W8100 (Hawaii, 8 GiB) with AMD APP 1800.5 OpenCL 2.0 on Windows. Device enumeration succeeded. `llama-cli` loaded and ran both F32 and Q4_K_M synthetic `stories260K` models with all six model layers offloaded; Q4_K_M generation repeated at about 81 tokens/s. `llama-bench` on the same small Q4_K_M model measured 1,613 ± 21 prompt tokens/s and 85.0 ± 1.3 generation tokens/s (three repetitions). This is a smoke benchmark, not a performance claim for larger models. Flash Attention ran on the CPU. No suitable MoE or recurrent model was available for end-to-end validation.

The OpenCL build completed. Of 57 CTest cases (excluding two model-download tests), 50 passed and 7 failed: `test-recurrent-state-rollback`, `test-save-load-state`, and `test-gguf` failed in the OpenCL backend; `test-jinja-py` lacked the Python Jinja dependency; and `test-thread-safety`, `test-state-restore-fragmented`, and `test-eval-callback` lacked model fixtures. The `test-gguf` failure exposed an unsupported Q1_0 conversion path in the backend test's generic all-types allocation; it is not counted as passing. These limitations remain open.

A separate CPU-only build completed, with 53 of 54 selected CTest cases passing. `test-thread-safety` could not open its absent model fixture. Build warnings came from existing upstream code and dependencies (including `u8path`, enum conversions, and subprocess); no Hawaii-specific compiler warning was observed in the OpenCL build.

The current session did not have a HIP/ROCm toolchain or gfx701 HIP runtime available. The official [ROCm 5.7 HIP porting guide](https://rocm.docs.amd.com/projects/HIP/en/docs-5.7.0/user_guide/hip_porting_guide.html) and [ROCm 6.3 HIP porting guide](https://rocm.docs.amd.com/projects/HIP/en/docs-6.3.1/how-to/hip_porting_guide.html) listed `gfx701` as a supported HIP compiler target. The current [ROCm 7.1 compatibility matrix](https://rocm.docs.amd.com/en/docs-7.1.0/compatibility/compatibility-matrix.html) does not list it among supported GPUs. [LLVM's AMDGPU target guide](https://llvm.org/docs/AMDGPUUsage.html) still defines `gfx701`, but compiler code generation alone does not establish a working HIP runtime/driver stack.

llama.cpp currently does not classify Hawaii in its HIP path. The code assumes GCN4 for some capability decisions, reports 32 lanes for GFX7 despite Hawaii's wave64 execution, and has half-precision and kernel feature assumptions that need architecture-specific review. Since this machine had no HIP toolchain and no gfx701 HIP runtime, there was no way to validate a safe HIP patch here; no HIP changes are included. HIP support remains unsupported/in development.

Historical logs record earlier Hawaii/OpenCL inference and Q4_K tests. They were reviewed as provenance only and are not included because they contain local paths and diagnostics.

## Upstream synchronization

The Git remote named `upstream` tracks `https://github.com/ggml-org/llama.cpp.git`. The initial source base is documented in [UPSTREAM-BASE.md](UPSTREAM-BASE.md). Keep downstream support commits separate from upstream history and record the exact upstream base used for each sync.
