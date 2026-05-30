# GitHub Configuration

This directory contains GitHub-specific configuration and automation for NIMBU OS.

## Directory Structure

```
.github/
├── README.md                          # This file
├── workflows/                         # GitHub Actions workflows
│   ├── README.md                      # Workflows documentation
│   └── ci.yml                         # CI/CD pipeline
├── ISSUE_TEMPLATE/                    # Issue templates
│   ├── bug_report.md                  # Bug report template
│   ├── feature_request.md             # Feature request template
│   └── documentation.md               # Documentation issue template
└── pull_request_template.md           # Pull request template
```

## GitHub Automation

### CI/CD Pipeline (workflows/ci.yml)

Automatically runs on every push and pull request:

- ✅ **Build Job** - Compiles kernel and verifies artifacts
- ✅ **Lint Job** - Checks code quality (cppcheck, shellcheck)
- ✅ **Documentation Job** - Validates documentation files
- ✅ **Status Check** - Reports overall success/failure

**View Status**: Go to **Actions** tab on GitHub repository

### Issue Templates

Three pre-configured issue templates help contributors:

1. **Bug Report** (`bug_report.md`)
   - Structured bug reporting
   - Environment information
   - Reproduction steps
   - Expected vs actual behavior

2. **Feature Request** (`feature_request.md`)
   - Feature description
   - Motivation and use cases
   - Proposed implementation
   - Alternative approaches

3. **Documentation Issue** (`documentation.md`)
   - Problem description
   - Location in docs
   - Suggested fix

### Pull Request Template

When creating a PR, contributors see:

- Clear description format
- Type of change (bug fix, feature, docs, etc.)
- Testing checklist
- Documentation update checklist
- Code quality reminders

## First-Time GitHub Setup

### 1. Create Repository

```bash
# Initialize local git (if not done)
git init
git add .
git commit -m "Initial commit: NIMBU OS"

# Add remote and push
git remote add origin https://github.com/yourusername/NIMBU-OS.git
git branch -M main
git push -u origin main
```

### 2. GitHub Actions Auto-Enable

Once you push code with `.github/workflows/ci.yml`, GitHub automatically:
- ✅ Enables Actions for the repository
- ✅ Runs the workflow on next push
- ✅ Shows status in PR and commit views

### 3. Add Status Badge

Add to README.md for CI status:

```markdown
[![CI Build & Test](https://github.com/yourusername/NIMBU-OS/actions/workflows/ci.yml/badge.svg)](https://github.com/yourusername/NIMBU-OS/actions/workflows/ci.yml)
```

## Workflow Features

### Build Job
- Installs cross-compiler (i686-elf-gcc)
- Compiles kernel
- Verifies artifacts generated
- Uploads build artifacts

### Lint Job
- Checks C code with cppcheck
- Validates shell scripts
- Validates Makefile syntax

### Documentation Job
- Checks all docs exist
- Reports doc sizes
- Validates internal links

### Artifact Management
- Builds uploaded after successful compile
- Retained for 5 days
- Accessible from Actions tab

## GitHub Features Enabled

✅ **Issues** - Bug reports, feature requests, discussions  
✅ **Pull Requests** - Code review and collaboration  
✅ **Actions** - CI/CD automation  
✅ **Discussions** - Community Q&A (optional)  
✅ **Wiki** - Documentation (optional)  
✅ **Releases** - Version releases  
✅ **Projects** - Kanban boards (optional)  

## Repository Settings Recommendations

### Branch Protection (main)

For production use, enable on `main` branch:
- ✅ Require pull request reviews before merging
- ✅ Require passing CI checks
- ✅ Require status checks to pass

### Default Branch

Set to `main` in Settings → Branches

### Issue Labels

Pre-configured labels:
- `bug` - Bug reports
- `enhancement` - Feature requests
- `documentation` - Doc improvements
- `good first issue` - Beginner-friendly tasks
- `help wanted` - Open for contributions
- `wontfix` - Intentionally not fixed
- `duplicate` - Duplicate issue

### Collaborators

Add team members in Settings → Collaborators

## Best Practices

### Using Issue Templates
1. Go to **Issues** tab
2. Click **New issue**
3. Choose template (bug, feature, docs)
4. Fill out template completely
5. Submit issue

### Creating Pull Requests
1. Create feature branch: `git checkout -b feature/your-feature`
2. Make changes and commit
3. Push to GitHub
4. Create PR from GitHub UI
5. Workflow automatically tests code
6. Address review comments
7. Merge when approved

### Workflow Status
- ✅ All checks passed - Ready to merge
- ❌ Failed - Fix issues and push again
- ⏳ Running - Wait for completion

## Monitoring

### View CI Status

1. **Commits**: Green checkmark = passed, red X = failed
2. **Pull Requests**: Checks shown in PR view
3. **Actions Tab**: Full workflow logs and history

### Debug Failed Builds

1. Go to **Actions** tab
2. Click failed workflow run
3. Expand job details
4. View error messages and logs
5. Fix locally and push retry

## Troubleshooting

### Workflow Not Running
- Check `.github/workflows/ci.yml` exists
- Verify workflow YAML syntax
- Push again to trigger

### Build Fails
- Check error logs in Actions tab
- Verify Makefile builds locally: `make clean && make`
- Check cross-compiler installed
- See [CONTRIBUTING.md](../CONTRIBUTING.md) for setup

### Missing Dependencies
- Ubuntu runner has most tools pre-installed
- Additional tools installed via `apt-get` in workflow
- View workflow logs for installation output

## Future Enhancements

Planned GitHub automation:

- [ ] Automated testing in QEMU
- [ ] Code coverage reports
- [ ] Performance benchmarking
- [ ] GitHub Pages documentation
- [ ] Release automation
- [ ] Security scanning
- [ ] Dependency updates

## Resources

- [GitHub Documentation](https://docs.github.com)
- [GitHub Actions Guide](https://docs.github.com/en/actions)
- [Setting up CI/CD](https://docs.github.com/en/actions/automating-builds-and-tests)

## Questions?

- Check [CONTRIBUTING.md](../CONTRIBUTING.md)
- Review [CODE_OF_CONDUCT.md](../CODE_OF_CONDUCT.md)
- Create an issue with questions
- Check existing issues and discussions

---

**Last Updated**: 2026-05-30  
**Maintained By**: NIMBU OS Development Team  
**License**: MIT - See [LICENSE](../LICENSE)
