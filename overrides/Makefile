MUPDF_JS := libmupdf.js
MUPDF_WASM := libmupdf.wasm

SAMPLE_PDF := pdfref13.pdf

all: $(MUPDF_JS)

EMSDK_DIR := /opt/emsdk
MUPDF_CORE := ../../build/wasm/libmupdf.a ../../build/wasm/libmupdf-third.a
$(MUPDF_CORE) : .FORCE
	$(MAKE) -j4 -C ../.. \
		OUT=wasm build=release \
		XCFLAGS='-DTOFU -DTOFU_CJK -DFZ_ENABLE_SVG=0 -DFZ_ENABLE_HTML=0 -DFZ_ENABLE_EPUB=0 -DFZ_ENABLE_JS=0' \
		generate
	BASH_SOURCE=$(EMSDK_DIR)/emsdk_env.sh; \
	. $(EMSDK_DIR)/emsdk_env.sh; \
	$(MAKE) -j4 -C ../.. \
		OS=wasm build=release \
		XCFLAGS='-DTOFU -DTOFU_CJK -DFZ_ENABLE_SVG=0 -DFZ_ENABLE_HTML=0 -DFZ_ENABLE_EPUB=0 -DFZ_ENABLE_JS=0' \
		libs

wasm: $(MUPDF_JS) $(MUPDF_WASM)
$(MUPDF_JS) $(MUPDF_WASM) : $(MUPDF_CORE) wrap.c wrap.js
	BASH_SOURCE=$(EMSDK_DIR)/emsdk_env.sh; . $(EMSDK_DIR)/emsdk_env.sh; \
	emcc -Wall -O3 -o $@ \
		-s ASSERTIONS=1 \
		-s WASM=1 \
		-s VERBOSE=0 \
		-s INITIAL_MEMORY=750MB \
		-s MAXIMUM_MEMORY=4096MB \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall","cwrap", "FS"]' \
		-s MODULARIZE=1 \
		-s 'EXPORT_NAME="mupdf"' \
		-I ../../include \
		--pre-js wrap.js \
		wrap.c \
		../../build/wasm/release/libmupdf.a \
		../../build/wasm/release/libmupdf-third.a

run: $(SAMPLE_PDF) $(MUPDF_JS) $(MUDPF_WASM)
	(sleep 3; xdg-open http://127.0.0.1:8000/view-page.html) &
	python3 -m http.server 8000

clean:
	rm -f $(MUPDF_JS) $(MUPDF_WASM)

nuke: clean
	$(MAKE) -C ../../ OS=wasm build=release clean

.PHONY: .FORCE clean
