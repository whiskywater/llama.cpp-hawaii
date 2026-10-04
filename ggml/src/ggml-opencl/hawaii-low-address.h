#pragma once

#ifdef _WIN32
#ifdef __cplusplus
extern "C" {
#endif
int ggml_opencl_install_hawaii_low_address_hook(void);
#ifdef __cplusplus
}
#endif
#endif
