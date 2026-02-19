#!/usr/bin/env -S uv run --quiet --script
# /// script
# requires-python = ">=3.8"
# dependencies = [
#     "GitPython>=3.1.0",
#     "click>=8.0.0", 
#     "python-dateutil>=2.8.0",
# ]
# ///
"""
Git Changelog Generator

This script analyzes git commits from a specified commit to HEAD and generates
a nicely formatted changelog in markdown format. It extracts JIRA issues from
commit messages and categorizes changes into features, fixes, documentation,
and general changes.

Usage:
    uv run utils/changelog/generate_changelog.py --from-commit <commit_hash>
    uv run utils/changelog/generate_changelog.py --from-commit <commit_hash> --to-commit <commit_hash>
    uv run utils/changelog/generate_changelog.py --from-tag <tag_name>
"""

import re
import os
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Set, Tuple, Optional
from dataclasses import dataclass, field

import click
import git
from git import Repo


def find_project_root(start_path: str = ".") -> str:
    """Find the project root by looking for git repository
    
    Args:
        start_path: Starting path to search from
        
    Returns:
        Path to the project root (git repository root)
    """
    current_path = Path(start_path).resolve()
    
    # Walk up the directory tree looking for .git
    while current_path != current_path.parent:
        if (current_path / '.git').exists():
            return str(current_path)
        current_path = current_path.parent
    
    # If no .git found, return the starting path
    return start_path


@dataclass
class CommitInfo:
    """Represents information extracted from a git commit"""
    hash: str
    message: str
    author: str
    date: datetime
    jira_issues: Set[str] = field(default_factory=set)
    category: str = "general"
    short_message: str = ""


class ChangelogGenerator:
    """Generates changelog from git commits"""
    
    # Patterns for categorizing commits
    CATEGORY_PATTERNS = {
        "refactor": [
            r"\b(refactor|refactoring|restructure|reorganize)\b",
            r"\[refactor\]",
            r"^refactor:",
            r"^ref:",
        ],
        "feature": [
            r"\b(feat|feature|implement)\b",
            r"\[feat\]",
            r"\[feature\]",
            r"^feat:",
            r"^feature:",
        ],
        "fix": [
            r"\b(fix|bug|resolve|repair|correct)\b",
            r"\[fix\]",
            r"\[bug\]",
            r"^fix:",
            r"^bugfix:",
        ],
        "documentation": [
            r"\b(doc|docs|documentation|readme)\b",
            r"\[doc\]",
            r"\[docs\]",
            r"^docs:",
            r"^doc:",
        ],
        "test": [
            r"\b(test|testing|spec|unittest)\b",
            r"\[test\]",
            r"^test:",
        ],
        "chore": [
            r"\b(chore|maintenance|cleanup|update)\b",
            r"\[chore\]",
            r"^chore:",
        ]
    }
    
    # Pattern for extracting JIRA issues
    JIRA_PATTERN = r'\b([A-Z]{2,10}-\d+)\b'
    
    def __init__(self, repo_path: str = "."):
        """Initialize the changelog generator
        
        Args:
            repo_path: Path to the git repository
        """
        try:
            self.repo = Repo(repo_path)
        except git.exc.InvalidGitRepositoryError:
            raise ValueError(f"'{repo_path}' is not a valid git repository")
    
    def extract_jira_issues(self, message: str) -> Set[str]:
        """Extract JIRA issue keys from commit message
        
        Args:
            message: Git commit message
            
        Returns:
            Set of JIRA issue keys found in the message
        """
        matches = re.findall(self.JIRA_PATTERN, message, re.IGNORECASE)
        return set(matches)
    
    def categorize_commit(self, message: str) -> str:
        """Categorize a commit based on its message
        
        Args:
            message: Git commit message
            
        Returns:
            Category name (feature, fix, documentation, etc.)
        """
        message_lower = message.lower()
        
        for category, patterns in self.CATEGORY_PATTERNS.items():
            for pattern in patterns:
                if re.search(pattern, message_lower):
                    return category
        
        return "general"
    
    def get_short_message(self, message: str) -> str:
        """Get a short version of the commit message
        
        Args:
            message: Full commit message
            
        Returns:
            First line of the commit message, cleaned up
        """
        first_line = message.split('\n')[0].strip()
        
        # Remove common prefixes
        prefixes_to_remove = [
            r'^(feat|feature|fix|bug|doc|docs|refactor|test|chore):\s*',
            r'^\[(feat|feature|fix|bug|doc|docs|refactor|test|chore)\]\s*',
        ]
        
        for prefix in prefixes_to_remove:
            first_line = re.sub(prefix, '', first_line, flags=re.IGNORECASE)
        
        return first_line.strip()
    
    def get_commits_since(self, from_commit: str, to_commit: str = "HEAD") -> List[CommitInfo]:
        """Get commits from a specific commit to another commit
        
        Args:
            from_commit: Starting commit hash or tag
            to_commit: Ending commit hash or tag (default: HEAD)
            
        Returns:
            List of CommitInfo objects (excluding merge commits)
        """
        try:
            # Get the commit range
            if to_commit == "HEAD":
                commits = list(self.repo.iter_commits(f"{from_commit}..HEAD"))
            else:
                commits = list(self.repo.iter_commits(f"{from_commit}..{to_commit}"))
        except git.exc.GitCommandError as e:
            raise ValueError(f"Invalid commit range: {e}")
        
        commit_infos = []
        
        for commit in commits:
            # Skip merge commits (commits with more than one parent)
            if len(commit.parents) > 1:
                continue
                
            jira_issues = self.extract_jira_issues(commit.message)
            category = self.categorize_commit(commit.message)
            short_message = self.get_short_message(commit.message)
            
            commit_info = CommitInfo(
                hash=commit.hexsha[:8],
                message=commit.message.strip(),
                author=commit.author.name,
                date=datetime.fromtimestamp(commit.committed_date),
                jira_issues=jira_issues,
                category=category,
                short_message=short_message
            )
            
            commit_infos.append(commit_info)
        
        return commit_infos
    
    def group_commits_by_category(self, commits: List[CommitInfo]) -> Dict[str, List[CommitInfo]]:
        """Group commits by category
        
        Args:
            commits: List of CommitInfo objects
            
        Returns:
            Dictionary mapping category names to lists of commits
        """
        grouped = {}
        
        for commit in commits:
            if commit.category not in grouped:
                grouped[commit.category] = []
            grouped[commit.category].append(commit)
        
        return grouped
    
    def format_markdown_changelog(self, commits: List[CommitInfo], from_commit: str, to_commit: str = "HEAD") -> str:
        """Format commits into a markdown changelog
        
        Args:
            commits: List of CommitInfo objects
            from_commit: Starting commit
            to_commit: Ending commit
            
        Returns:
            Formatted markdown string
        """
        if not commits:
            return "No commits found in the specified range."
        
        # Group commits by category
        grouped_commits = self.group_commits_by_category(commits)
        
        # Collect all JIRA issues
        all_jira_issues = set()
        for commit in commits:
            all_jira_issues.update(commit.jira_issues)
        
        # Generate markdown
        md_lines = []
        
        # Header
        md_lines.append(f"# Changelog")
        md_lines.append("")
        md_lines.append(f"**Generated on:** {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        md_lines.append(f"**Commit range:** `{from_commit}` to `{to_commit}`")
        md_lines.append(f"**Total commits:** {len(commits)}")
        md_lines.append("")
        
        # JIRA Issues Summary
        if all_jira_issues:
            md_lines.append("## 🎫 JIRA Issues")
            md_lines.append("")
            for issue in sorted(all_jira_issues):
                md_lines.append(f"- [{issue}](https://jira.ess.eu/browse/{issue})")
            md_lines.append("")
        
        # Category order for display
        category_order = ["feature", "fix", "documentation", "refactor", "test", "chore", "general"]
        category_icons = {
            "feature": "✨",
            "fix": "🐛",
            "documentation": "📚",
            "refactor": "♻️",
            "test": "🧪",
            "chore": "🔧",
            "general": "📝"
        }
        
        category_titles = {
            "feature": "Features",
            "fix": "Bug Fixes",
            "documentation": "Documentation",
            "refactor": "Refactoring",
            "test": "Tests",
            "chore": "Maintenance",
            "general": "General Changes"
        }
        
        # Generate sections for each category
        for category in category_order:
            if category in grouped_commits:
                icon = category_icons.get(category, "📝")
                title = category_titles.get(category, category.title())
                
                md_lines.append(f"## {icon} {title}")
                md_lines.append("")
                
                for commit in grouped_commits[category]:
                    # Format commit entry
                    commit_line = f"- **{commit.short_message}**"
                    
                    # Add JIRA issues if present
                    if commit.jira_issues:
                        jira_links = []
                        for issue in sorted(commit.jira_issues):
                            jira_links.append(f"[{issue}](https://jira.ess.eu/browse/{issue})")
                        commit_line += f" ({', '.join(jira_links)})"
                    
                    # Add commit hash and author
                    commit_line += f" - `{commit.hash}` by {commit.author}"
                    
                    md_lines.append(commit_line)
                
                md_lines.append("")
        
        return "\n".join(md_lines)
    
    def save_changelog(self, content: str, output_dir: str = "CHANGELOG", include_time: bool = False) -> str:
        """Save changelog to a file
        
        Args:
            content: Markdown content to save
            output_dir: Directory to save the changelog
            include_time: Whether to include time in filename
            
        Returns:
            Path to the saved file
        """
        # Resolve the output directory path
        output_path = Path(output_dir).resolve()
        output_path.mkdir(parents=True, exist_ok=True)
        
        # Generate filename with current date and optionally time
        if include_time:
            timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        else:
            timestamp = datetime.now().strftime("%Y-%m-%d")
        filename = f"changelog_{timestamp}.md"
        filepath = output_path / filename
        
        # Write the file
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        
        return str(filepath)


@click.command()
@click.option('--from-commit', required=True, help='Starting commit hash or tag')
@click.option('--to-commit', default='HEAD', help='Ending commit hash or tag (default: HEAD)')
@click.option('--output-dir', default=None, help='Output directory for changelog (default: CHANGELOG in project root)')
@click.option('--repo-path', default=None, help='Path to git repository (default: auto-detect project root)')
@click.option('--times', is_flag=True, help='Include hours and minutes in filename')
def main(from_commit: str, to_commit: str, output_dir: str, repo_path: str, times: bool):
    """Generate a changelog from git commits"""
    
    try:
        # Auto-detect project root if not specified
        if repo_path is None:
            repo_path = find_project_root()
        
        # Set default output directory relative to project root
        if output_dir is None:
            output_dir = os.path.join(repo_path, 'CHANGELOG')
        
        # Initialize changelog generator
        generator = ChangelogGenerator(repo_path)
        
        # Get commits
        click.echo(f"Analyzing commits from {from_commit} to {to_commit}...")
        commits = generator.get_commits_since(from_commit, to_commit)
        
        if not commits:
            click.echo("No commits found in the specified range.")
            return
        
        click.echo(f"Found {len(commits)} commits")
        
        # Generate changelog
        click.echo("Generating changelog...")
        changelog_content = generator.format_markdown_changelog(commits, from_commit, to_commit)
        
        # Save changelog
        output_path = generator.save_changelog(changelog_content, output_dir, times)
        
        click.echo(f"✅ Changelog saved to: {output_path}")
        
        # Show summary
        grouped_commits = generator.group_commits_by_category(commits)
        click.echo("\n📊 Summary:")
        for category, commit_list in grouped_commits.items():
            click.echo(f"  {category}: {len(commit_list)} commits")
        
        # Show JIRA issues
        all_jira_issues = set()
        for commit in commits:
            all_jira_issues.update(commit.jira_issues)
        
        if all_jira_issues:
            click.echo(f"\n🎫 JIRA Issues found: {len(all_jira_issues)}")
            for issue in sorted(all_jira_issues):
                click.echo(f"  - {issue}")
    
    except Exception as e:
        click.echo(f"❌ Error: {e}", err=True)
        sys.exit(1)


if __name__ == "__main__":
    main()
