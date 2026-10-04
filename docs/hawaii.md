# AMD Hawaii support

This page tracks downstream changes for AMD Hawaii / Sea Islands (GCN2, `gfx701`), including FirePro W8100-class devices. Hawaii is distinct from gfx803/GCN4.

## Status

- **OpenCL:** The historical implementation is being ported from the recovered `hawaii-opencl-experiment` work to the current upstream base. Runtime results below will distinguish current validation from historical logs.
- **HIP/ROCm gfx701:** Experimental investigation only. Current upstream does not classify gfx701 in its HIP path. No gfx701 HIP build or runtime result is claimed.

## Build

The current OpenCL build instructions will be finalized with the port. The backend uses the OpenCL ICD installed for the target device; AMD APP 1800.5 is the recovered Windows test environment. Other OpenCL platforms and drivers may behave differently.

## Validation record

Historical local evidence records an AMD FirePro W8100-class device on AMD APP OpenCL 2.0 and successful OpenCL inference/Q4_K tests. These historical logs are not included in this repository because they contain private paths and diagnostic data. Current downstream validation is pending.

## Upstream synchronization

The Git remote named `upstream` tracks `https://github.com/ggml-org/llama.cpp.git`. The initial source base is documented in [UPSTREAM-BASE.md](UPSTREAM-BASE.md). Keep downstream support commits separate from upstream history and record the exact upstream base used for each sync.
