#!/usr/bin/env python3
"""
Add build metadata to coverage HTML reports.

This script injects commit information into an HTML coverage report,
integrating it into the existing header table structure.
"""

import sys
import os
import re
from datetime import datetime


def add_metadata_to_html(html_path, branch, commit_sha, build_date=None):
    """
    Add build metadata to coverage HTML report.
    
    Args:
        html_path: Path to the coverage HTML file
        branch: Git branch name
        commit_sha: Git commit SHA (short form)
        build_date: Build timestamp (optional, will use current time if not provided)
    """
    if not os.path.exists(html_path):
        print(f"Error: File not found: {html_path}", file=sys.stderr)
        return False
    
    if build_date is None:
        build_date = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    
    # Read the original HTML
    with open(html_path, 'r', encoding='utf-8') as f:
        html_content = f.read()
    
    # Remove any existing build-metadata rows (for idempotency)
    html_content = re.sub(r'    <tr id="build-metadata">.*?</tr>\n', '', html_content, flags=re.DOTALL)
    html_content = re.sub(r'    <tr id="build-metadata-2">.*?</tr>\n', '', html_content, flags=re.DOTALL)
    html_content = re.sub(r'    <tr id="build-metadata-3">.*?</tr>\n', '', html_content, flags=re.DOTALL)
    # Also handle old format with different indentation
    html_content = re.sub(r'          <tr id="build-metadata">.*?</tr>\n', '', html_content, flags=re.DOTALL)
    
    # Create metadata rows for the new HTML structure
    # These will be inserted into the legend table
    metadata_rows = f"""    <tr id="build-metadata">
      <th scope="row">Branch:</th>
      <td>{branch}</td>
    </tr>
    <tr id="build-metadata-2">
      <th scope="row">Commit:</th>
      <td>{commit_sha}</td>
    </tr>
    <tr id="build-metadata-3">
      <th scope="row">Build:</th>
      <td>{build_date}</td>
    </tr>
"""
    
    # Try to find the new HTML structure first (gcovr newer version)
    # Look for the legend table with Directory row
    new_pattern = r'(<table class="legend">\n    <tr>\n      <th scope="row">Directory:</th>)'
    
    match = re.search(new_pattern, html_content, re.DOTALL)
    if match:
        # New structure found - insert after the opening <table> tag, before Directory row
        insert_pos = match.start() + len('<table class="legend">\n')
        html_content = html_content[:insert_pos] + metadata_rows + html_content[insert_pos:]
    else:
        # Fall back to old HTML structure (older gcovr version)
        old_pattern = r'(          <tr>\n            <td width="10%" class="headerName">Directory:</td>)'
        
        match = re.search(old_pattern, html_content, re.DOTALL)
        if match:
            # Old structure - create single row metadata
            old_metadata_row = f"""          <tr id="build-metadata">
            <td class="headerName">Branch:</td>
            <td class="headerValue">{branch}</td>
            <td class="headerName">Commit:</td>
            <td class="headerValue">{commit_sha}</td>
            <td class="headerName">Build:</td>
            <td colspan="2" class="headerValue">{build_date}</td>
          </tr>
"""
            insert_pos = match.start()
            html_content = html_content[:insert_pos] + old_metadata_row + html_content[insert_pos:]
        else:
            print("Warning: Could not find Directory row in either new or old format", file=sys.stderr)
            return False
    
    # Write the modified HTML
    with open(html_path, 'w', encoding='utf-8') as f:
        f.write(html_content)
    
    print(f"Successfully added metadata to {html_path}")
    print(f"  Branch: {branch}")
    print(f"  Commit: {commit_sha}")
    print(f"  Date: {build_date}")
    return True


def main():
    """Main entry point for the script."""
    if len(sys.argv) < 2:
        print("Usage: add_coverage_metadata.py <html_file> [branch] [commit_sha] [build_date]")
        print("\nEnvironment variables (used if arguments not provided):")
        print("  CI_COMMIT_REF_NAME - Git branch name")
        print("  CI_COMMIT_SHORT_SHA - Git commit SHA")
        print("\nExample:")
        print("  add_coverage_metadata.py coverage/coverage.html master abc1234 '2024-01-01 12:00:00'")
        print("  add_coverage_metadata.py coverage/coverage.html")
        sys.exit(1)
    
    html_path = sys.argv[1]
    
    # Get branch name from args or environment
    branch = sys.argv[2] if len(sys.argv) > 2 else os.environ.get('CI_COMMIT_REF_NAME', 'unknown')
    
    # Get commit SHA from args or environment
    commit_sha = sys.argv[3] if len(sys.argv) > 3 else os.environ.get('CI_COMMIT_SHORT_SHA', 'unknown')
    
    # Get build date from args or use current time
    build_date = sys.argv[4] if len(sys.argv) > 4 else None
    
    success = add_metadata_to_html(html_path, branch, commit_sha, build_date)
    sys.exit(0 if success else 1)


if __name__ == '__main__':
    main()
