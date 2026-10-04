# Project instructions

This is the independently maintained `whiskywater/llama.cpp-hawaii` downstream of [llama.cpp](https://github.com/ggml-org/llama.cpp). Keep upstream attribution and licensing intact. The downstream adds and validates legacy GPU paths, especially AMD Hawaii (GCN2, gfx701) through OpenCL, and investigates HIP support separately.

AI-assisted implementation, autonomous repository maintenance, and validation are permitted in this downstream. The project owner is responsible for reviewing and maintaining changes. Keep changes scoped, architecture-aware, and tested. Never claim Hawaii HIP support without actual validation.

Do not publish private logs, model files, build artifacts, machine names, private paths, addresses, credentials, or unrelated work. Do not push to `ggml-org/llama.cpp`, open upstream pull requests, or imply upstream endorsement. Preserve the `upstream` remote and document the upstream base for downstream syncs.

Before publishing, review the complete diff and new commits for secrets and private infrastructure details. Preserve all applicable upstream and third-party notices.
