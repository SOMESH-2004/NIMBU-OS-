# GitHub Repository Setup Guide

This guide walks you through setting up NIMBU OS on GitHub as a public repository.

## Prerequisites

- Git installed on your system
- GitHub account
- NIMBU OS source code ready
- All documentation files generated

## Step-by-Step Setup

### Step 1: Verify Repository Structure

Ensure your local directory has all files:

```bash
cd /path/to/NIMBU\ OS
ls -la

# Should see:
# - LICENSE
# - README.md
# - CHANGELOG.md
# - SECURITY.md
# - CONTRIBUTING.md
# - CODE_OF_CONDUCT.md
# - ROADMAP.md
# - .gitignore
# - .github/ (directory)
# - All documentation files
# - Source code (src/, include/, etc.)
```

### Step 2: Initialize Local Git Repository

```bash
# Navigate to project directory
cd /path/to/NIMBU\ OS

# Initialize git
git init

# Configure user (if not already done)
git config user.name "Your Name"
git config user.email "your.email@example.com"

# Or configure globally
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"
```

### Step 3: Add All Files

```bash
# Add all files to staging area
git add .

# Verify what will be committed
git status

# You should see all files listed, with .gitignore excluding:
# - *.o (object files)
# - *.elf (old kernels)
# - *.bin (old binaries)
# - build/ directories
# - .vscode/, .idea/ (IDE files)
```

### Step 4: Create Initial Commit

```bash
# Create first commit with descriptive message
git commit -m "Initial commit: NIMBU OS - Minimal x86 Linux Kernel

- Core kernel features (GDT, IDT, memory management, processes)
- Device drivers (VGA, keyboard, serial)
- Shell with 8+ commands
- Comprehensive documentation (11,000+ lines)
- MIT License - Open source ready"
```

### Step 5: Create GitHub Repository

1. Go to https://github.com/new
2. Enter repository name: `NIMBU-OS` (or preferred name)
3. Choose **Public** (for open source)
4. **Don't** initialize with README (we have our own)
5. **Don't** add .gitignore (we have our own)
6. **Don't** add license (we have our own - MIT)
7. Click **Create repository**

### Step 6: Connect Local Repository to GitHub

```bash
# Add remote
git remote add origin https://github.com/yourusername/NIMBU-OS.git

# Verify remote
git remote -v

# Should show:
# origin  https://github.com/yourusername/NIMBU-OS.git (fetch)
# origin  https://github.com/yourusername/NIMBU-OS.git (push)
```

### Step 7: Push to GitHub

```bash
# Rename branch to main (if needed)
git branch -M main

# Push to GitHub
git push -u origin main

# First time will ask for authentication:
# - Enter username and personal access token (or GitHub password)
# - Or use SSH if configured

# Verify push succeeded
git log --oneline
```

### Step 8: Verify on GitHub

1. Go to https://github.com/yourusername/NIMBU-OS
2. Verify all files are present
3. README.md should display with badges
4. .github/workflows/ci.yml should trigger automatically
5. Check **Actions** tab for CI status

## Post-Setup Configuration

### Repository Settings

1. Go to **Settings** → **General**
   - Description: "Minimal x86 32-bit Linux Kernel"
   - Website: (optional)
   - Topics: `os`, `kernel`, `x86`, `education`

2. **Settings** → **Branches**
   - Set default branch to `main`
   - Add branch protection for `main`:
     - Require pull request reviews
     - Require passing status checks

3. **Settings** → **Collaborators**
   - Add team members (if applicable)

### Enable GitHub Features

1. **Issues** (enabled by default)
   - Use for bug reports and feature requests
   - Pre-configured templates available

2. **Discussions** (optional, Settings → Features)
   - Good for Q&A and community discussions

3. **Projects** (optional, Settings → Features)
   - Kanban board for issue tracking

4. **Wiki** (optional, Settings → Features)
   - For additional documentation

### Add Repository Topics

In **Settings** → **General** → **Topics**, add:
- `os` - Operating System
- `kernel` - Kernel development
- `x86` - x86 architecture
- `education` - Educational project
- `operating-system` - Clear labeling
- `assembly` - Uses assembly code
- `c` - Uses C language

## Creating Release

### Version 1.0.0 Release

```bash
# Create a tag for version 1.0.0
git tag -a v1.0.0 -m "NIMBU OS v1.0.0 Release

Core OS features:
- Bootloader and kernel (3.5 KB)
- Memory management (PMM, VMM, heap)
- Process management and scheduling
- Interrupt handling
- Device drivers (VGA, keyboard, serial)
- Shell interface with 8 commands
- Full documentation (11,000+ lines)

Ready for educational use!"

# Push tags to GitHub
git push origin --tags

# Create release on GitHub:
# 1. Go to Releases page
# 2. Click 'Create new release'
# 3. Select tag v1.0.0
# 4. Add release notes (from CHANGELOG.md)
# 5. Publish release
```

## GitHub Workflows

### CI/CD Pipeline

The CI workflow automatically runs on:
- Every push to `main` or `develop` branches
- Every pull request

**Status**: Check **Actions** tab for workflow status

### Adding CI Badge to README

The CI badge is already in README.md:

```markdown
[![CI Build & Test](https://github.com/yourusername/NIMBU-OS/actions/workflows/ci.yml/badge.svg)](https://github.com/yourusername/NIMBU-OS/actions/workflows/ci.yml)
```

## Contributing Setup

For contributors to help:

1. They fork repository (click **Fork**)
2. Clone their fork: `git clone <their-fork-url>`
3. Create feature branch: `git checkout -b feature/something`
4. Make changes and commit
5. Push to their fork
6. Create Pull Request to main repository
7. CI workflow automatically tests PR
8. Maintainer reviews and merges

## Next Steps

### Immediate
- [ ] Push repository to GitHub
- [ ] Verify all files present
- [ ] Check CI workflow running
- [ ] Add GitHub topics

### Short Term
- [ ] Announce release on social media
- [ ] Get first community feedback
- [ ] Close any initial issues
- [ ] Plan v1.1.0 features

### Long Term
- [ ] Build community
- [ ] Accept contributions
- [ ] Release v1.1.0, v2.0.0, etc.
- [ ] Consider GitHub Pages for docs

## Troubleshooting

### Authentication Issues

**Problem**: "fatal: Authentication failed"

**Solution**:
```bash
# Use HTTPS with personal access token
git remote set-url origin https://TOKEN@github.com/username/NIMBU-OS.git

# Or use SSH (recommended)
ssh-keygen -t ed25519 -C "your_email@example.com"
# Follow prompts, then:
git remote set-url origin git@github.com:username/NIMBU-OS.git
```

### Push Rejected

**Problem**: "Everything up-to-date" but files not on GitHub

**Solution**:
```bash
# Verify remote is correct
git remote -v

# Check branch
git branch -a

# Push explicitly
git push origin main --force  # Use --force only if necessary!
```

### CI Workflow Not Running

**Problem**: No workflow status in Actions tab

**Solution**:
- Verify `.github/workflows/ci.yml` pushed successfully
- Check workflow YAML syntax
- Push a new commit to trigger

### Files Missing on GitHub

**Problem**: Some files not appearing on GitHub

**Solution**:
- Check `.gitignore` - might be excluding them
- Verify file added: `git add path/to/file`
- Check `git status` shows file staged
- Commit and push again

## Resources

- [Git Documentation](https://git-scm.com/doc)
- [GitHub Help](https://docs.github.com)
- [GitHub CLI](https://cli.github.com/)
- [GitHub Actions](https://docs.github.com/en/actions)

## Support

Need help? Check:

1. [README.md](README.md) - Project overview
2. [CONTRIBUTING.md](CONTRIBUTING.md) - Contribution guidelines
3. [GitHub Docs](https://docs.github.com)
4. Create an issue on GitHub

---

**Last Updated**: 2026-05-30  
**Created By**: NIMBU OS Development Team  
**License**: MIT - See [LICENSE](LICENSE)

## Quick Copy-Paste Commands

```bash
# All commands in sequence
cd /path/to/NIMBU\ OS
git init
git config user.name "Your Name"
git config user.email "your.email@example.com"
git add .
git commit -m "Initial commit: NIMBU OS - Minimal x86 Linux Kernel"
git remote add origin https://github.com/yourusername/NIMBU-OS.git
git branch -M main
git push -u origin main

# Tag version
git tag -a v1.0.0 -m "NIMBU OS v1.0.0 Release"
git push origin --tags

# Verify
git remote -v
git log --oneline
```

Then go to https://github.com/yourusername/NIMBU-OS to verify! 🎉
