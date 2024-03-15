package clibav

// #cgo pkg-config: libavcodec libavformat libavutil
// #include <stdlib.h>
// #include "convert.h"
import "C"
import (
	"errors"
	"fmt"
	"os"
	"strings"
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

var str2loglevel = map[string]C.enum_convert_loglevel{
	"quiet":   C.CONVERT_LOG_QUIET,
	"panic":   C.CONVERT_LOG_PANIC,
	"fatal":   C.CONVERT_LOG_FATAL,
	"error":   C.CONVERT_LOG_ERROR,
	"warning": C.CONVERT_LOG_WARNING,
	"info":    C.CONVERT_LOG_INFO,
	"verbose": C.CONVERT_LOG_VERBOSE,
	"debug":   C.CONVERT_LOG_DEBUG,
}

var loglevels []string = getLevels()

func getLevels() []string {
	var (
		keys = make([]string, len(str2loglevel))
		i    = 0
	)

	for key, _ := range str2loglevel {
		keys[i] = key
		i++
	}

	return keys
}

func SetLogLevel(level string) error {
	v, ok := str2loglevel[level]
	if !ok {
		return fmt.Errorf("unknown level `%s`, available options are: %s", level, strings.Join(loglevels, ", "))
	}

	C.convert_set_logging(v)
	return nil
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
