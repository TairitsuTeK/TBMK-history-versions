# Build every TBMK version, then pack release archives.
#
#   make            build every version into dist/<version>/
#   make sources    pack all tracked sources of HEAD -> dist/tbmk-sources.zip
#   make release    build all + dist/tbmk-sources.zip + dist/tbmk-images.zip
#   make list       list discovered versions
#   make clean      remove dist/ and any local build/ directory
#
# Each version directory owns a Makefile; pass OUT=... to choose its output.

VERSION_FIND := find . -mindepth 2 -type f -name Makefile \
	-not -path './build/*' -not -path './dist/*'
DIST ?= $(CURDIR)/dist
CC ?= gcc

.PHONY: all sources release list clean

all:
	@rm -rf $(DIST)
	@mkdir -p $(DIST)
	@$(VERSION_FIND) | sort | while IFS= read -r mk; do \
	  dir=$$(dirname "$$mk"); \
	  slug=$$(printf '%s' "$$dir" | sed 's|^\./||; s|/|__|g; s| |_|g; s|[][()]||g'); \
	  printf '==> %s\n' "$$dir"; \
	  $(MAKE) -C "$$dir" OUT="$(DIST)/$$slug" || exit 1; \
	done

sources:
	@mkdir -p $(DIST)
	@git rev-parse --is-inside-work-tree >/dev/null 2>&1 || { \
	  echo "make sources needs a git checkout; commit the tree and retry" >&2; exit 1; }
	@git archive --format=zip --output=$(DIST)/tbmk-sources.zip HEAD
	@printf 'source archive: %s\n' "$(DIST)/tbmk-sources.zip"

release: all
	@$(MAKE) sources
	@printf 'TBMK history versions\ncommit: %s\nbuilt: %s\ncompiler: %s\n' \
	  "$$(git rev-parse --short HEAD 2>/dev/null || echo unknown)" \
	  "$$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
	  "$$($(CC) --version 2>/dev/null | head -n 1)" > $(DIST)/BUILD-INFO.txt
	@cd $(DIST) && rm -f tbmk-images.zip && zip -q tbmk-images.zip BUILD-INFO.txt
	@cd $(DIST) && find . -mindepth 2 -maxdepth 2 -type f \
	  \( -name '*.img' -o -name '*.iso' -o -name 'kernel.elf' \) -print \
	  | sort | zip -q tbmk-images.zip -@
	@printf 'image archive: %s\n' "$(DIST)/tbmk-images.zip"

list:
	@$(VERSION_FIND) | sort | while IFS= read -r mk; do dirname "$$mk"; done

clean:
	@rm -rf $(DIST)
	@find . -type d -name build -prune -exec rm -rf {} +
