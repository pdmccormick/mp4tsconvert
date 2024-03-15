package clibav

// #cgo pkg-config: libavcodec libavformat libavutil
// #include <stdlib.h>
// #include "convert.h"
import "C"
import (
	"errors"
	"fmt"
	"os"
	"unsafe"
)

func Init() error {
	if rc := C.convert_init(); rc < 0 {
		return errors.New("unable to initialize clibav")
	}

	return nil
}

func toError(rc C.int) error {
	var (
		p = C.convert_err2str(rc)
		s = C.GoString(p)
	)

	if p != nil {
		defer C.free(unsafe.Pointer(p))
	}

	if s == "" {
		s = fmt.Sprintf("rc %d", rc)
	}

	return errors.New(s)
}

func Convert(input, output string) error {
	inf, err := os.Open(input)
	if err != nil {
		return err
	}

	defer inf.Close()

	var (
		c       C.struct_convert
		inputFd = C.int(inf.Fd())
		pOutput = C.CString(output)
	)

	defer C.free(unsafe.Pointer(pOutput))

	if rc := C.convert_open(&c, inputFd, pOutput); rc < 0 {
		return toError(rc)
	}

	defer C.convert_close(&c)

	if rc := C.convert_process_all(&c); rc < 0 {
		return toError(rc)
	}

	return nil
}
