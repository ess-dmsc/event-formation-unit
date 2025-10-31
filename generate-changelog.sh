#!/bin/bash
# Changelog Generator Launcher
# This script provides a convenient way to run the changelog generator using uv

# Get the directory where this script is located (project root)
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"

# Change to project root and run with uv
cd "$PROJECT_ROOT"
uv run utils/changelog/generate_changelog.py "$@"
