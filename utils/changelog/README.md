# Git Changelog Generator

A Python script that analyzes git commits and generates nicely formatted changelogs in Markdown format. It extracts JIRA issues from commit messages and categorizes changes automatically.

**Location**: `utils/changelog/generate_changelog.py`  
**Output**: Creates changelog files in the project root's `CHANGELOG/` directory  
**Dependencies**: Managed automatically using [uv](https://github.com/astral-sh/uv)

## Features

- 🔍 **Commit Analysis**: Analyzes git commits from a specified commit range (excludes merge commits)
- 🎫 **JIRA Integration**: Automatically extracts and links JIRA issue keys
- 📂 **Smart Categorization**: Categorizes commits into:
  - ♻️ Refactoring (highest priority)
  - ✨ Features
  - 🐛 Bug Fixes
  - 📚 Documentation
  - 🧪 Tests
  - 🔧 Maintenance/Chore
  - 📝 General Changes
- 📅 **Timestamped Output**: Saves changelog with current date in filename
- 🚫 **Clean Output**: Filters out merge commits for cleaner changelog
- 🎯 **Auto-detection**: Automatically finds project root and creates files in the correct location
- ⚡ **Zero-config**: Uses `uv` to automatically manage dependencies and virtual environmentsnerator

A Python script that analyzes git commits and generates nicely formatted changelogs in Markdown format. It extracts JIRA issues from commit messages and categorizes changes automatically.

## Features

- 🔍 **Commit Analysis**: Analyzes git commits from a specified commit range (excludes merge commits)
- 🎫 **JIRA Integration**: Automatically extracts and links JIRA issue keys
- 📂 **Smart Categorization**: Categorizes commits into:
  - ✨ Features
  - 🐛 Bug Fixes
  - 📚 Documentation
  - ♻️ Refactoring
  - 🧪 Tests
  - 🔧 Maintenance/Chore
  - 📝 General Changes
- 📅 **Timestamped Output**: Saves changelog with current date in filename
- � **Clean Output**: Filters out merge commits for cleaner changelog

## Installation

**Prerequisites**: [uv](https://github.com/astral-sh/uv) - A fast Python package installer and resolver

1. Install `uv` if not already available:
   ```bash
   # On macOS and Linux:
   curl -LsSf https://astral.sh/uv/install.sh | sh
   
   # Or with pip:
   pip install uv
   ```

2. No additional setup needed! `uv` will automatically manage dependencies when you run the script.

## Usage

The script uses `uv` to automatically handle dependencies and virtual environments. It can be run from anywhere in the project and automatically detects the project root.

### Basic Usage

Generate a changelog from a specific commit to HEAD:
```bash
# Using the convenient launcher (recommended):
./generate-changelog.sh --from-commit abc1234

# Or directly with uv:
uv run utils/changelog/generate_changelog.py --from-commit abc1234

# Or with executable permissions:
utils/changelog/generate_changelog.py --from-commit abc1234
```

### Advanced Usage

Generate a changelog between two specific commits:
```bash
./generate-changelog.sh --from-commit abc1234 --to-commit def5678
```

Include time in filename for multiple generations on same day:
```bash
./generate-changelog.sh --from-commit abc1234 --times
```

Specify a custom output directory:
```bash
uv run utils/changelog/generate_changelog.py --from-commit abc1234 --output-dir ./custom-changelog-dir
```

Use with a different repository:
```bash
uv run utils/changelog/generate_changelog.py --from-commit abc1234 --repo-path /path/to/other/repo
```

### Using Git Tags

You can also use git tags instead of commit hashes:
```bash
python generate_changelog.py --from-commit v1.0.0
python generate_changelog.py --from-commit v1.0.0 --to-commit v1.1.0
```

## Commit Message Conventions

The script automatically categorizes commits based on common patterns in commit messages:

### Features
- Messages containing: `feat`, `feature`, `implement`
- Prefixes: `feat:`, `feature:`, `[feat]`, `[feature]`

### Refactoring (Highest Priority)
- Messages containing: `refactor`, `refactoring`, `restructure`, `reorganize`
- Prefixes: `refactor:`, `[refactor]`

### Bug Fixes
- Messages containing: `fix`, `bug`, `resolve`, `repair`, `correct`
- Prefixes: `fix:`, `bugfix:`, `[fix]`, `[bug]`

### Documentation
- Messages containing: `doc`, `docs`, `documentation`, `readme`
- Prefixes: `docs:`, `doc:`, `[doc]`, `[docs]`

### Other Categories
- **Refactor**: `refactor`, `refactoring`, `restructure`
- **Tests**: `test`, `testing`, `spec`, `unittest`
- **Chore**: `chore`, `maintenance`, `cleanup`, `update`

## JIRA Integration

The script automatically detects JIRA issue keys in commit messages using the pattern `[A-Z]{2,10}-\d+` (e.g., `PROJ-123`, `FEATURE-456`).

**Note**: The script is configured to use the ESS JIRA instance at `https://jira.ess.eu/browse/`. If you need to change this, update the JIRA URL in the script:
```python
# In the script, look for this line and update it:
f"[{issue}](https://jira.ess.eu/browse/{issue})"
```

## Output Format

The generated changelog includes:

1. **Header** with generation date and commit range
2. **JIRA Issues Summary** with links to all referenced issues
3. **Categorized Changes** grouped by type with emoji indicators

The changelog focuses on actual code changes and excludes merge commits for better readability.

## Example Output

```markdown
# Changelog

**Generated on:** 2025-09-30 15:30:45
**Commit range:** `abc1234` to `HEAD`
**Total commits:** 8

## 🎫 JIRA Issues

- [PROJ-123](https://jira.ess.eu/browse/PROJ-123)
- [PROJ-124](https://jira.ess.eu/browse/PROJ-124)

## ✨ Features

- **Add user authentication system** ([PROJ-123](https://jira.ess.eu/browse/PROJ-123)) - `abc1234` by John Doe

## 🐛 Bug Fixes

- **Fix memory leak in data processing** ([PROJ-124](https://jira.ess.eu/browse/PROJ-124)) - `def5678` by Jane Smith
```

## Configuration

You can customize the script by modifying these variables:

- `CATEGORY_PATTERNS`: Add or modify patterns for commit categorization
- `JIRA_PATTERN`: Adjust the regex pattern for JIRA issue detection
- Category icons and titles in the `format_markdown_changelog` method

## Contributing

Feel free to submit issues and enhancement requests!

## License

This script is provided as-is for internal use.
