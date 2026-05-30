# GitHub Workflows

This directory contains GitHub Actions workflows for NIMBU OS CI/CD automation.

## Available Workflows

### ci.yml - Continuous Integration

**File**: `.github/workflows/ci.yml`  
**Trigger**: Push to `main` or `develop` branches, Pull Requests  
**Status Badge**: 

```markdown
![CI Build & Test](https://github.com/yourusername/NIMBU-OS/actions/workflows/ci.yml/badge.svg)
```

#### Jobs

1. **Build** - Compiles NIMBU OS kernel
   - Installs dependencies (build-essential, nasm, qemu, gcc-multilib)
   - Installs i686-elf cross-compiler
   - Compiles kernel with `make all`
   - Verifies build artifacts (nimbu.elf, nimbu.bin)
   - Uploads artifacts to GitHub

2. **Lint** - Code quality checks
   - Runs `cppcheck` on C code
   - Validates shell scripts with `shellcheck`
   - Validates Makefile syntax

3. **Documentation** - Documentation validation
   - Checks all markdown files exist
   - Verifies documentation completeness
   - Checks for broken links

4. **Build Success/Failure** - Final status reporting
   - Reports overall build success/failure

#### Artifacts

- **nimbu.elf** - ELF format kernel (debuggable)
- **nimbu.bin** - Binary format kernel (runnable)
- Retained for 5 days

#### Usage

The workflow runs automatically on:
- Every push to `main` or `develop`
- Every pull request to `main` or `develop`

No manual configuration needed!

## Adding to README Badge

Add this to your README.md for CI status:

```markdown
[![CI Build & Test](https://github.com/yourusername/NIMBU-OS/actions/workflows/ci.yml/badge.svg)](https://github.com/yourusername/NIMBU-OS/actions/workflows/ci.yml)
```

## Setting Up First Time

1. Push code to GitHub with these workflow files
2. GitHub automatically enables Actions
3. Workflows run on next push/PR
4. View results in **Actions** tab on GitHub

## Testing Locally

To test the build workflow locally before pushing:

```bash
# Install dependencies (on Ubuntu/Debian)
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  gcc-multilib \
  nasm \
  qemu-system-i386 \
  xorriso

# Install cross-compiler
sudo apt-get install -y gcc-i686-linux-gnu binutils-i686-linux-gnu

# Build
make clean
make all

# Verify artifacts
ls -lh nimbu.elf nimbu.bin
```

## Environment Variables

Currently, no environment variables are required. All tools are installed via `apt-get`.

## Secrets & Credentials

The CI workflow doesn't require any secrets. All dependencies are open-source and publicly available.

## Troubleshooting

### Workflow Failed: Missing Dependencies
- Check the logs in the **Actions** tab
- Verify installation commands in `ci.yml`
- Cross-compiler might not be available on GitHub

### Build Timeout
- GitHub Actions provides 360 minutes per job
- Current build takes ~2-3 minutes
- If slower, may need to cache dependencies

### Artifacts Not Uploading
- Check disk space in logs
- Verify artifact paths are correct
- Check retention days setting

## Future Enhancements

Planned workflow improvements:

- [ ] Test in QEMU (automated kernel execution)
- [ ] Performance benchmarking
- [ ] Memory leak detection (ASan)
- [ ] Code coverage reports
- [ ] Dependency vulnerability scanning
- [ ] Documentation generation
- [ ] Release automation
- [ ] Deploy to GitHub Pages

## Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Workflow Syntax](https://docs.github.com/en/actions/using-workflows/workflow-syntax-for-github-actions)
- [Available Runners](https://docs.github.com/en/actions/using-github-hosted-runners)

## Contact

Questions about workflows? Check:
1. [CONTRIBUTING.md](../../CONTRIBUTING.md)
2. GitHub Actions logs
3. Create an issue with `question` label

---

**Last Updated**: 2026-05-30  
**Maintained By**: NIMBU OS Development Team  
**License**: MIT
