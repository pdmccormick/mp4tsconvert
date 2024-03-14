package clibav

// #cgo pkg-config: libavcodec libavformat libavutil
// #include <stdlib.h>
// #include "convert.h"
import "C"
import (
	"errors"
	"unsafe"
)

func Init() error {
	if rc := C.init_convert(); rc < 0 {
		return errors.New("unable to initialize clibav")
	}

	return nil
}

func Convert(input, output string) error {
	var (
		c       C.struct_convert
		pInput  = C.CString(input)
		pOutput = C.CString(output)
	)

	defer C.free(unsafe.Pointer(pInput))
	defer C.free(unsafe.Pointer(pOutput))

	if _, err := C.convert_open(&c, pInput, pOutput); err != nil {
		return err
	}

	if _, err := C.convert_process_all(&c); err != nil {
		return err
	}

	defer C.convert_close(&c)

	return nil
}
