.DEFAULT_GOAL := help

FAST ?= 1
SAVE ?= tamapoke.nvs

.PHONY: help clean emu-setup emu-build emu emu-fast emu-shot test test-agent-tools ai-docs-check web-check validate

help: ## List the documented targets
	@awk 'BEGIN {FS = ":[^#]*## "} /^[a-zA-Z_-]+:[^#]*## / {printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

emu-setup: ## Unpack SDL2 into tools/emu/.sdl without root (skip when libsdl2-dev is installed)
	@bash tools/emu/setup-sdl.sh

emu-build: ## Build the desktop emulator from the real firmware sources
	@bash tools/emu/build.sh

emu: emu-build ## Run the emulator in a window (FAST=<n> speeds up time, SAVE=<file> picks the save)
	@tools/emu/tamapoke-emu --scale 2 --fast $(FAST) --save $(SAVE)

emu-fast: ## Run the emulator at 60x time speed
	@$(MAKE) emu FAST=60

emu-shot: emu-build ## Headless screenshot to .dev/tmp/shot.ppm (TAPS="x,y x,y" MS=<ms> FAST=<n> optional)
	@mkdir -p .dev/tmp
	@tools/emu/tamapoke-emu --wipe --save .dev/tmp/shot.nvs --fast $(FAST) $(foreach t,$(TAPS),--tap $(t)) --shot .dev/tmp/shot.ppm --shot-ms $(or $(MS),3000)

clean: ## Remove native test and emulator build output
	@cd tests && make clean
	@rm -f tools/emu/tamapoke-emu tools/emu/proto.h tools/emu/sketch.cpp

test: ## Run the native firmware logic tests and the Web Serial tests
	@cd tests && make test

test-agent-tools: ## Test the agent hook
	@PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s .dev/tests -v

ai-docs-check: ## Check the agent docs for stale globs, Make targets, paths and skills
	@python3 .dev/ai-docs-check.py

web-check: ## Validate the installer builds without replacing artifacts
	@bash tools/build_web.sh --check
	@bash tools/build_web_local.sh --check

validate: test test-agent-tools ai-docs-check ## Fast checks that need no toolchain (use web-check for the Arduino build)
