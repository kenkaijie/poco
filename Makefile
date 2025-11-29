#
# Top-level makefile for task handling.
#
# Note compilation is handled by CMake directly.
#
# SPDX-FileCopyrightText: Copyright contributors to the poco project.
# SPDX-License-Identifier: MIT

VERSION_CMD = git describe --first-parent --dirty 2>/dev/null || echo unknown
export VERSION = $(shell $(VERSION_CMD))

builddir = build
docsdir = docs

export DOXYGEN_OUTPUT_DIR = $(builddir)/doxygen

$(builddir)/docs $(builddir)/doxygen:
	mkdir -p $@

.PHONY: docs
docs: docs-gen | $(builddir)/docs
	sphinx-build docs $(builddir)/docs

.PHONY: docs-gen
docs-gen: | $(builddir)/docs $(builddir)/doxygen
	doxygen
	breathe-apidoc -g file -o $(docsdir)/_gen $(builddir)/doxygen/xml

.PHONY: docs-gen-clean
docs-gen-clean: | $(builddir)/docs $(builddir)/doxygen
	rm -rf $(builddir)/doxygen/*
	rm -rf $(docsdir)/_gen/*

.PHONY: docs-clean
docs-clean: | $(builddir)/docs
	rm -rf $(builddir)/docs/*

.PHONY: clean
clean: docs-clean docs-gen-clean

.PHONY: project-version
project-version:
	@echo $(VERSION)
