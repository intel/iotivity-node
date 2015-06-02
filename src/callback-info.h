#ifndef __CALLBACK_INFO_H__
#define __CALLBACK_INFO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ffi.h>

typedef void (*Callback)();
typedef void (*Marshaller)(ffi_cif *, void *, void **, void *);
typedef void (*UserDataRemover)(void *);

typedef struct _callback_info {
  ffi_cif cif;
  Marshaller resultingFunction;
  ffi_closure *closure;
  void *user_data;
  UserDataRemover remover;
  ffi_type *argTypes[];
} callback_info;

// Free a callback_info structure. Always returns NULL
callback_info *callback_info_free(callback_info *info);

// Create a new callback_info.
callback_info *callback_info_new(

    // Extra parameter
    void *user_data,

    // Function that frees the memory for the extra parameter
    UserDataRemover remover,

    // The signature of the original function:
    // Return type
    ffi_type *return_type,

    // Location
    Marshaller default_handler,

    // Argument count followed by list of argument types
    int argumentCount, ...);

#ifdef __cplusplus
}
#endif

#endif /* __CALLBACK_INFO_H__ */
