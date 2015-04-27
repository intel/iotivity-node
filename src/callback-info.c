#ifdef __cplusplus
extern "C" {
#endif

#include <ffi.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

#include "callback-info.h"

callback_info *callback_info_free( callback_info *info ) {
	if ( info->closure ) {
		ffi_closure_free( info->closure );
	}
	if ( info->argTypes ) {
		free( info->argTypes );
	}
	free( info );

	return 0;
}

callback_info *callback_info_new(
		int uuid,
		ffi_type *return_type,
		Marshaller default_handler,
		int argumentCount, ... ) {
	callback_info *info = ( callback_info * )malloc( sizeof( callback_info ) );
	ffi_cif cif;
	va_list va;
	int index;

	// Allocate info structure
	if ( info ) {
		memset( info, 0, sizeof( callback_info ) );

		// Allocate closure
		info->closure = ( ffi_closure * )ffi_closure_alloc(
			sizeof( ffi_closure ),
			(void **)&( info->resultingFunction ) );
		if ( info->closure ) {

			// Allocate argument type list
			info->argTypes = ( ffi_type ** )malloc( argumentCount * sizeof( ffi_type * ) );
			if ( info->argTypes ) {
				memset( info->argTypes, 0, argumentCount * sizeof( ffi_type * ) );

				// Populate argument type list
				va_start( va, argumentCount );
				for ( index = 0 ; index < argumentCount ; index++ ) {
					info->argTypes[ index ] = va_arg( va, ffi_type * );
				}
				va_end( va );

				// Set up argument description
				if ( ffi_prep_cif(
						&cif,
						FFI_DEFAULT_ABI,
						argumentCount,
						return_type,
						info->argTypes ) == FFI_OK ) {

					// Set up closure
					if ( ffi_prep_closure_loc(
							info->closure,
							&cif,
							default_handler,
							(void *)( uintptr_t )uuid,
							(void *)( info->resultingFunction ) ) != FFI_OK ) {

						info = callback_info_free( info );
					}
				} else {
					info = callback_info_free( info );
				}
			} else {
				info = callback_info_free( info );
			}
		} else {
			info = callback_info_free( info );
		}
	}

	return info;
}

#ifdef __cplusplus
}
#endif
