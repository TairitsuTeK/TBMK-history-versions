PYTHON ?= python3

.PHONY: all list report clean

all:
	$(PYTHON) tools/build.py --all

list:
	$(PYTHON) tools/build.py --list

report:
	$(PYTHON) tools/report.py

clean:
	$(PYTHON) tools/build.py --clean
