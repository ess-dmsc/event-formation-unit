#!/usr/bin/env python3
"""
Aggregate Google Benchmark results into multiple report formats.

This script collects JSON output from Google Benchmark runs and generates:
- Plain text summary report
- HTML report with styled tables
- GitLab metrics format for CI/CD integration

Usage:
    python3 aggregate_benchmarks.py [benchmark_results_dir]

Default directory: ./benchmark_results
"""

import json
import os
import sys
from pathlib import Path


def load_benchmark_json(filepath):
    """Load benchmark data from JSON file."""
    try:
        with open(filepath, "r") as f:
            return json.load(f)
    except Exception as e:
        print(f"Warning: Could not load {filepath}: {e}", file=sys.stderr)
        return None


def format_time(ns):
    """Format nanoseconds to appropriate unit."""
    if ns < 1000:
        return f"{ns:.2f} ns"
    elif ns < 1000000:
        return f"{ns/1000:.2f} us"
    elif ns < 1000000000:
        return f"{ns/1000000:.2f} ms"

    return f"{ns/1000000000:.2f} s"


def main():
    # Determine results directory
    results_dir = Path("benchmark_results")

    if len(sys.argv) > 1:
        results_dir = Path(sys.argv[1])

    if not results_dir.exists():
        print("No benchmark results found")
        return 1

    json_files = sorted(results_dir.glob("*.json"))
    if not json_files:
        print("No benchmark JSON files found")
        return 1

    all_benchmarks = []
    warnings = []

    # Load all benchmark results
    for json_file in json_files:
        data = load_benchmark_json(json_file)
        if data and "benchmarks" in data:
            # Check for warnings in context
            if "context" in data:
                ctx = data["context"]
                if ctx.get("cpu_scaling_enabled", False):
                    warnings.append(
                        "CPU scaling is enabled - measurements may be noisy"
                    )
                if (
                    "library_build_type" in ctx
                    and ctx["library_build_type"].lower() == "debug"
                ):
                    warnings.append("Library built as DEBUG - timings may be affected")

            for bench in data["benchmarks"]:
                bench["source_file"] = json_file.stem
                all_benchmarks.append(bench)

    if not all_benchmarks:
        print("No valid benchmark data found")
        return 1

    # Deduplicate warnings
    warnings = sorted(set(warnings))

    # Generate text report
    with open(results_dir / "report.txt", "w") as f:
        f.write("=" * 80 + "\n")
        f.write("BENCHMARK RESULTS SUMMARY\n")
        f.write("=" * 80 + "\n")

        # Write warnings if any
        if warnings:
            f.write("\nWARNINGS:\n")
            for warning in warnings:
                f.write(f"  ⚠️  {warning}\n")
            f.write("\n")

        f.write("\n")

        current_source = None
        for bench in all_benchmarks:
            source = bench["source_file"]
            if source != current_source:
                f.write(f"\n{source}:\n")
                f.write("-" * 80 + "\n")
                current_source = source

            name = bench["name"]
            time_ns = bench.get("cpu_time", bench.get("real_time", 0))
            iterations = bench.get("iterations", 0)

            f.write(
                f"  {name:<50} {format_time(time_ns):>15} ({iterations:>12,} iterations)\n"
            )

        f.write("\n" + "=" * 80 + "\n")

    # Generate HTML report
    with open(results_dir / "report.html", "w") as f:
        f.write(
            """<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Benchmark Results</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
        h1 { color: #333; }
        .warnings { background: #fff3cd; border-left: 4px solid #ffc107; padding: 15px; margin: 20px 0; border-radius: 5px; }
        .warnings h2 { color: #856404; margin-top: 0; font-size: 1.1em; }
        .warnings ul { margin: 10px 0; padding-left: 20px; }
        .warnings li { color: #856404; margin: 5px 0; }
        .benchmark-group { background: white; margin: 20px 0; padding: 20px; border-radius: 5px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
        table { width: 100%; border-collapse: collapse; }
        th { background: #4CAF50; color: white; padding: 12px; text-align: left; }
        td { padding: 10px; border-bottom: 1px solid #ddd; }
        tr:hover { background-color: #f5f5f5; }
        .metric { font-weight: bold; color: #2196F3; }
        .source-name { color: #4CAF50; font-size: 1.2em; margin-bottom: 10px; }
    </style>
</head>
<body>
    <h1>Benchmark Results</h1>
"""
        )

        # Write warnings if any
        if warnings:
            f.write('    <div class="warnings">\n')
            f.write("        <h2>⚠️ Warnings</h2>\n")
            f.write("        <ul>\n")
            for warning in warnings:
                f.write(f"            <li>{warning}</li>\n")
            f.write("        </ul>\n")
            f.write("    </div>\n")

        current_source = None
        for bench in all_benchmarks:
            source = bench["source_file"]
            if source != current_source:
                if current_source is not None:
                    f.write("    </table>\n</div>\n")
                f.write(f'<div class="benchmark-group">\n')
                f.write(f'    <div class="source-name">{source}</div>\n')
                f.write("    <table>\n")
                f.write(
                    "        <tr><th>Benchmark Name</th><th>CPU Time</th><th>Iterations</th></tr>\n"
                )
                current_source = source

            name = bench["name"]
            time_ns = bench.get("cpu_time", bench.get("real_time", 0))
            iterations = bench.get("iterations", 0)

            f.write(
                f'        <tr><td>{name}</td><td class="metric">{format_time(time_ns)}</td><td>{iterations:,}</td></tr>\n'
            )

        if current_source is not None:
            f.write("    </table>\n</div>\n")

        f.write("</body>\n</html>\n")

    # Generate GitLab metrics format
    with open(results_dir / "metrics.txt", "w") as f:
        for bench in all_benchmarks:
            name = bench["name"]
            source = bench["source_file"]
            time_ns = bench.get("cpu_time", bench.get("real_time", 0))
            metric_name = f"benchmark_{source}_{name}".replace("/", "_").replace(
                " ", "_"
            )
            f.write(f"{metric_name} {time_ns}\n")

    # Print summary to console
    print("\n" + "=" * 80)
    print("BENCHMARK RESULTS SUMMARY")
    print("=" * 80)

    # Print warnings if any
    if warnings:
        print("\nWARNINGS:")
        for warning in warnings:
            print(f"  ⚠️  {warning}")
        print()

    print()

    current_source = None
    for bench in all_benchmarks:
        source = bench["source_file"]
        if source != current_source:
            print(f"\n{source}:")
            print("-" * 80)
            current_source = source

        name = bench["name"]
        time_ns = bench.get("cpu_time", bench.get("real_time", 0))
        iterations = bench.get("iterations", 0)

        print(f"  {name:<50} {format_time(time_ns):>15} ({iterations:>12,} iterations)")

    print("\n" + "=" * 80)
    print(f"Reports saved to: {results_dir.absolute()}/")
    print("  - report.txt  (plain text)")
    print("  - report.html (HTML format)")
    print("  - metrics.txt (GitLab format)")
    print("  - *.json      (raw JSON data)")
    print("=" * 80 + "\n")

    return 0


if __name__ == "__main__":
    sys.exit(main())
