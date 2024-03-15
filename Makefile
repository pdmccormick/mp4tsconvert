GO              ?= go
GO_LDFLAGS      =
GO_TAGS         ?= mp4tsconvert,netgo
GO_BUILD_FLAGS  = -trimpath -tags=$(GO_TAGS) -ldflags=$(GO_LDFLAGS)

ifdef REBUILD
	GO_BUILD_FLAGS += -a
endif

ifdef FFMPEG_PATH
	export PKG_CONFIG_LIBAVCODEC_FFMPEG_PATH    := $(FFMPEG_PATH)
	export PKG_CONFIG_LIBAVFORMAT_FFMPEG_PATH   := $(FFMPEG_PATH)
	export PKG_CONFIG_LIBAVUTIL_FFMPEG_PATH     := $(FFMPEG_PATH)
	export PKG_CONFIG_PATH                      := $(CURDIR)/support/ffmpeg/pkgconfig
	export PKG_CONFIG_DISABLE_UNINSTALLED       := 1

	GO_LDFLAGS  := "-s -extldflags='-static'"
endif

ifdef V
	ifeq ("$(origin V)", "command line")
		VERBOSE=$(V)
	endif
endif

ifndef VERBOSE
	VERBOSE=0
endif

ifeq ($(VERBOSE),1)
	Q=
else
	Q=@
endif

.PHONY: all
all: mp4tsconvert

.PHONY: mp4tsconvert
mp4tsconvert:
	@echo "  BUILD      $@"
	$(Q)$(GO) build -o $@ $(GO_BUILD_FLAGS) .

.PHONY: clean
clean:
	@echo "  CLEAN"
	$(Q)rm -f mp4tsconvert

print-%: ; @echo $*=$($*)
