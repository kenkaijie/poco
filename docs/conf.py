import os
from pathlib import Path

curr_dir = Path(__file__).parent

version = os.environ.get("VERSION", "unknown")

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "poco"
copyright = "2025, Kenneth Ng"
author = "Kenneth Ng"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "breathe",
    "myst_parser",
    "sphinxcontrib.plantuml",
    "sphinx.ext.autosectionlabel"
]

templates_path = ["_templates"]
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "sphinx_rtd_theme"
html_static_path = ["_static"]
html_logo = "_static/logo-outline.svg"

# -- Options for Auto Section Label -----------------------------------------
# https://www.sphinx-doc.org/en/master/usage/extensions/autosectionlabel.html

autosectionlabel_prefix_document = True
autosectionlabel_maxdepth = 2

# -- Options for Breathe -----------------------------------------------------

breathe_projects = {"poco": "../build/doxygen/xml"}
breathe_default_project = "poco"
