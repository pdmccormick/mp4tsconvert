package main

import (
	"flag"

	"mp4tsconvert/internal/clibav"
)

func main() {
	var (
		inFlag  = flag.String("i", "", "input `filename`")
		outFlag = flag.String("o", "", "output `filename`")
	)

	flag.Parse()

	if err := clibav.Convert(*inFlag, *outFlag); err != nil {
		panic(err)
	}
}
