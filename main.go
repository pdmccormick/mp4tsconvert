package main

import (
	"flag"
	"fmt"
	"log"
	"time"

	"mp4tsconvert/internal/clibav"
)

var (
	logf   = log.Printf
	fatalf = log.Fatalf
)

func main() {
	var (
		inFlag  = flag.String("i", "", "input `filename`")
		outFlag = flag.String("o", "", "output `filename`")
	)

	flag.Parse()

	var (
		input  = *inFlag
		output = *outFlag
	)

	if input == "" {
		fatalf("missing `-i` input file")
	}

	if output == "" {
		fatalf("missing `-o` input file")
	}

	if err := clibav.Init(); err != nil {
		fatalf("clibav: %s", err)
	}

	logf("reading from %s", input)

	fmt.Println("")

	var start = time.Now()

	if err := clibav.Convert(input, output); err != nil {
		fatalf("convert: %s", err)
	}

	var dur = time.Since(start)

	fmt.Println("")

	logf("took %s to write %s", dur.Truncate(1*time.Millisecond), output)
}
