# Build every TBMK version into dist/<version>/.
#
#   make            build all versions
#   make list       list discovered versions
#   make clean      remove dist/ and any local build/ directory
#
# Each version directory owns a Makefile; pass OUT=... to choose its output.
# GitHub Actions publishes one Release per version (see releases.tsv).

VERSION_FIND := find . -mindepth 2 -type f -name Makefile \
	-not -path './build/*' -not -path './dist/*'
DIST ?= $(CURDIR)/dist

.PHONY: all list clean

all:
	@rm -rf $(DIST)
	@mkdir -p $(DIST)
	@$(VERSION_FIND) | sort | while IFS= read -r mk; do \
	  dir=$$(dirname "$$mk"); \
	  slug=$$(printf '%s' "$$dir" | sed 's|^\./||; s|/|__|g; s| |_|g; s|[][()]||g'); \
	  printf '==> %s\n' "$$dir"; \
	  $(MAKE) -C "$$dir" OUT="$(DIST)/$$slug" || exit 1; \
	done

list:
	@$(VERSION_FIND) | sort | while IFS= read -r mk; do dirname "$$mk"; done

clean:
	@rm -rf $(DIST)
	@find . -type d -name build -prune -exec rm -rf {} +
