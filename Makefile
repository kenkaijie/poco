#
# Top-level makefile for task handling.
#
# SPDX-FileCopyrightText: Copyright contributors to the poco project.
# SPDX-License-Identifier: MIT

.PHONY: clean docs docs-clean docs-gen docs-gen-clean project-version

VERSION_CMD = git describe --first-parent --dirty 2>/dev/null || echo unknown
export VERSION = $(shell $(VERSION_CMD))

builddir = build
docsdir = docs

export DOXYGEN_OUTPUT_DIR = $(builddir)/doxygen

$(builddir)/docs $(builddir)/doxygen:
	mkdir -p $@

docs: docs-gen | $(builddir)/docs
	sphinx-build docs $(builddir)/docs

docs-gen: | $(builddir)/docs $(builddir)/doxygen
	doxygen
	breathe-apidoc -g file -o $(docsdir)/_gen $(builddir)/doxygen/xml

docs-gen-clean: | $(builddir)/docs $(builddir)/doxygen
	rm -rf $(builddir)/doxygen/*
	rm -rf $(docsdir)/_gen/*

docs-clean: | $(builddir)/docs
	rm -rf $(builddir)/docs/*
	
clean: docs-clean docs-gen-clean

project-version:
	@echo $(VERSION)