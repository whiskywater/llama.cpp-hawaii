#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <string.h>

#include "hawaii-low-address.h"

typedef LPVOID (WINAPI * virtual_alloc_fn)(LPVOID, SIZE_T, DWORD, DWORD);

static virtual_alloc_fn real_virtual_alloc;
static SRWLOCK install_lock = SRWLOCK_INIT;
static int installed;

static LPVOID WINAPI low_virtual_alloc(LPVOID address, SIZE_T size, DWORD type, DWORD protect) {
    if (real_virtual_alloc == NULL) {
        return NULL;
    }
    if (address != NULL || size == 0 || type != MEM_RESERVE || protect != PAGE_NOACCESS) {
        return real_virtual_alloc(address, size, type, protect);
    }

    const uintptr_t limit = UINT64_C(0xffffffffff);
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    const uintptr_t align = si.dwAllocationGranularity;
    uintptr_t cursor = UINT64_C(0x100000000);

    while (cursor < limit && size <= limit - cursor) {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery((void *) cursor, &mbi, sizeof(mbi)) == 0) {
            break;
        }

        const uintptr_t base = (uintptr_t) mbi.BaseAddress;
        if (mbi.RegionSize > UINTPTR_MAX - base) {
            break;
        }
        const uintptr_t end = base + mbi.RegionSize;
        const uintptr_t candidate = (cursor + align - 1) & ~(align - 1);
        if (mbi.State == MEM_FREE && candidate < end && size <= end - candidate && size <= limit - candidate) {
            LPVOID result = real_virtual_alloc((void *) candidate, size, type, protect);
            if (result != NULL) {
                return result;
            }
        }
        if (end <= cursor) {
            break;
        }
        cursor = end;
    }

    return real_virtual_alloc(address, size, type, protect);
}

static int install_hook_impl(void) {
    HMODULE module = GetModuleHandleA("amdocl64.dll");
    if (module == NULL) {
        return 0;
    }

    unsigned char * base = (unsigned char *) module;
    IMAGE_DOS_HEADER * dos = (IMAGE_DOS_HEADER *) base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0) {
        return 0;
    }

    IMAGE_NT_HEADERS64 * nt = (IMAGE_NT_HEADERS64 *) (base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE || nt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
        return 0;
    }

    const DWORD image_size = nt->OptionalHeader.SizeOfImage;
    const IMAGE_DATA_DIRECTORY imports = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (image_size == 0 || imports.VirtualAddress == 0 || imports.Size < sizeof(IMAGE_IMPORT_DESCRIPTOR) ||
            imports.VirtualAddress > image_size || imports.Size > image_size - imports.VirtualAddress) {
        return 0;
    }

    const DWORD import_end = imports.VirtualAddress + imports.Size;
    IMAGE_IMPORT_DESCRIPTOR * desc = (IMAGE_IMPORT_DESCRIPTOR *) (base + imports.VirtualAddress);
    for (; (DWORD) ((unsigned char *) (desc + 1) - base) <= import_end; ++desc) {
        if (desc->Name == 0) {
            break;
        }
        if (desc->OriginalFirstThunk == 0 || desc->FirstThunk == 0 ||
                desc->OriginalFirstThunk >= image_size || desc->FirstThunk >= image_size) {
            continue;
        }

        IMAGE_THUNK_DATA64 * names = (IMAGE_THUNK_DATA64 *) (base + desc->OriginalFirstThunk);
        IMAGE_THUNK_DATA64 * iat = (IMAGE_THUNK_DATA64 *) (base + desc->FirstThunk);
        for (DWORD i = 0; ; ++i) {
            const uintptr_t name_rva = (uintptr_t) desc->OriginalFirstThunk + (uintptr_t) i * sizeof(*names);
            const uintptr_t iat_rva = (uintptr_t) desc->FirstThunk + (uintptr_t) i * sizeof(*iat);
            if (name_rva > image_size - sizeof(*names) || iat_rva > image_size - sizeof(*iat) ||
                    names[i].u1.AddressOfData == 0) {
                break;
            }
            if (IMAGE_SNAP_BY_ORDINAL64(names[i].u1.Ordinal)) {
                continue;
            }

            const DWORD symbol_rva = (DWORD) names[i].u1.AddressOfData;
            if (symbol_rva > image_size - sizeof(IMAGE_IMPORT_BY_NAME) - sizeof("VirtualAlloc")) {
                break;
            }
            IMAGE_IMPORT_BY_NAME * symbol = (IMAGE_IMPORT_BY_NAME *) (base + symbol_rva);
            if (memcmp(symbol->Name, "VirtualAlloc", sizeof("VirtualAlloc")) != 0) {
                continue;
            }

            memcpy(&real_virtual_alloc, &iat[i].u1.Function, sizeof(real_virtual_alloc));
            if (real_virtual_alloc == NULL) {
                return 0;
            }

            DWORD old_protect;
            if (!VirtualProtect(&iat[i].u1.Function, sizeof(iat[i].u1.Function), PAGE_READWRITE, &old_protect)) {
                return 0;
            }

            virtual_alloc_fn replacement = low_virtual_alloc;
            PVOID replacement_ptr = NULL;
            memcpy(&replacement_ptr, &replacement, sizeof(replacement_ptr));
            InterlockedExchangePointer((PVOID volatile *) &iat[i].u1.Function, replacement_ptr);

            DWORD unused;
            if (!VirtualProtect(&iat[i].u1.Function, sizeof(iat[i].u1.Function), old_protect, &unused)) {
                PVOID original_ptr = NULL;
                memcpy(&original_ptr, &real_virtual_alloc, sizeof(original_ptr));
                InterlockedExchangePointer((PVOID volatile *) &iat[i].u1.Function, original_ptr);
                VirtualProtect(&iat[i].u1.Function, sizeof(iat[i].u1.Function), old_protect, &unused);
                real_virtual_alloc = NULL;
                return 0;
            }

            return 1;
        }
    }

    return 0;
}

int ggml_opencl_install_hawaii_low_address_hook(void) {
    AcquireSRWLockExclusive(&install_lock);
    if (!installed) {
        installed = install_hook_impl();
    }
    const int result = installed;
    ReleaseSRWLockExclusive(&install_lock);
    return result;
}
