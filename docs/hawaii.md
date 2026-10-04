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

The current session did not have a HIP/ROCm toolchain or gfx701 HIP runtime available. LLVM's AMDGPU target definitions still include `gfx701`, but that alone does not establish a usable HIP/ROCm runtime. Hawaii is not classified as GCN4 in upstream's HIP code; wave size and feature assumptions require dedicated review before a safe HIP path can be claimed. HIP support remains unsupported/in development.

Historical logs record earlier Hawaii/OpenCL inference and Q4_K tests. They were reviewed as provenance only and are not included because they contain local paths and diagnostics.

## Upstream synchronization

The Git remote named `upstream` tracks `https://github.com/ggml-org/llama.cpp.git`. The initial source base is documented in [UPSTREAM-BASE.md](UPSTREAM-BASE.md). Keep downstream support commits separate from upstream history and record the exact upstream base used for each sync.
