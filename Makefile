# MelonOS Makefile

.PHONY: all clean run dev

all:
	@./scripts/build.sh

run: all
	@./scripts/run.sh

clean:
	@echo "Cleaning build directory..."
	@rm -rf build/*
	@echo "Clean completed!"

test: all
	@echo "Running tests..."
	@./scripts/test.sh

dev: all run

