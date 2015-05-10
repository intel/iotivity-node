#ifndef __CALLBACK_INFO_H__
#define __CALLBACK_INFO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ffi.h>

typedef void ( *Callback )();
typedef void ( *Marshaller )( ffi_cif*, void*, void**, void* );

typedef struct _callback_info {
	ffi_cif cif;
	Marshaller resultingFunction;
	ffi_closure *closure;
	ffi_type *argTypes[];
} callback_info;

// Free a callback_info structure. Always returns NULL
callback_info *callback_info_free( callback_info *info );

// Create a new callback_info.
callback_info *callback_info_new(
	void *user_data,
	ffi_type *return_type,
	Marshaller default_handler,
	int argumentCount, ... );

#ifdef __cplusplus
}
#endif

#endif /* __CALLBACK_INFO_H__ */
