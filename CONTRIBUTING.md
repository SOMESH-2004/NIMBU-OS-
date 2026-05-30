# Contributing to NIMBU OS

First off, thanks for your interest in contributing to NIMBU OS! 🎉

This document provides guidelines and instructions for contributing to the project.

## Code of Conduct

- Be respectful and inclusive
- Focus on the code, not the person
- Help others learn and grow

## Getting Started

### Prerequisites

- **Linux/Unix environment** (or WSL on Windows)
- **i686-elf-gcc cross-compiler** (x86 32-bit)
- **nasm assembler** (for assembly code)
- **make** build system
- **QEMU** (optional, for testing)
- **git** version control

### Installation

```bash
# Clone repository
git clone https://github.com/yourusername/NIMBU-OS.git
cd NIMBU-OS

# Build the project
make clean
make all

# Run tests
make run
```

## Development Workflow

### 1. Create a Branch

```bash
git checkout -b feature/your-feature-name
# or
git checkout -b fix/bug-description
```

### 2. Make Changes

- Follow existing code style and conventions
- Keep commits focused and atomic
- Add comments for complex logic
- Update documentation

### 3. Test Your Changes

```bash
# Build
make clean
make all

# Run in QEMU
make run

# Test manually in shell:
# - Try new commands
# - Check for memory leaks
# - Verify process scheduling
```

### 4. Commit Changes

```bash
git add .
git commit -m "Clear description of changes"
```

**Commit message format**:
```
[Type] Brief description

Longer explanation if needed.
- Detail 1
- Detail 2

Fixes #123 (if applicable)
```

**Types**: feat, fix, docs, refactor, test, perf, style

### 5. Push and Create Pull Request

```bash
git push origin feature/your-feature-name
```

Then create PR on GitHub with clear description.

## Types of Contributions

### 1. Bug Fixes

```markdown
**Description**: Brief bug description
**Steps to reproduce**:
1. Step 1
2. Step 2
3. ...
**Expected behavior**: What should happen
**Actual behavior**: What happens instead
**Files affected**: List files
```

### 2. New Features

```markdown
**Feature**: Name of feature
**Description**: What it does
**Implementation approach**: How you'll do it
**Documentation needed**: What docs to update
**Testing plan**: How to test it
```

### 3. Documentation

- Fix typos and unclear explanations
- Add examples
- Improve formatting
- Add diagrams

### 4. Code Quality

- Refactoring for clarity
- Performance improvements
- Better error handling
- Removing duplication

## Coding Standards

### Style Guide

**C Code**:
```c
/* Use descriptive names */
int process_count;  /* Good */
int pc;             /* Avoid single letter vars */

/* Use prefixes for globals */
static int kernel_state = 0;  /* Static global */
static pmm_t pmm;             /* Module state */

/* Document complex functions */
/**
 * @brief Function description
 * @param param1 Description
 * @return Description
 */
int function_name(int param1) {
    /* Implementation */
}

/* Keep lines under 80 characters for readability */
vga_putstr("Long string example message here", VGA_COLOR_GREEN);

/* Use clear control flow */
if (condition) {
    action1();
} else {
    action2();
}
```

**Assembly**:
```asm
; Use descriptive labels
kernel_init_loop:
    mov eax, 0
    call function_name
    jmp kernel_init_loop

; Comment non-obvious operations
mov eax, [0xB8000]  ; Read from VGA buffer
```

### File Organization

```
src/kernel/module/
├── module.c        /* Implementation */
├── module_internal.h  /* Internal types (if needed) */
└── test_module.c   /* Unit tests (if applicable) */

include/
└── module.h        /* Public API */
```

### Naming Conventions

| Element | Convention | Example |
|---------|-----------|---------|
| Functions | lowercase_with_underscores | `pmm_alloc()`, `vmm_init()` |
| Types | lowercase_t suffix | `pmm_t`, `pcb_t` |
| Constants | UPPERCASE | `MAX_PROCESSES`, `PMM_PAGE_SIZE` |
| Macros | UPPERCASE | `PAGE_PRESENT`, `VGA_COLOR_BLACK` |
| Variables | lowercase_with_underscores | `kernel_stack`, `page_dir` |
| Static vars | s_name prefix (optional) | `s_kernel_state` |

## Documentation Standards

### API Documentation

```c
/**
 * @brief Brief description (one line)
 * 
 * Longer description with more details.
 * Can be multiple paragraphs.
 * 
 * @param param1 Description of param1
 * @param param2 Description of param2
 * 
 * @return Description of return value
 * 
 * @note Important note if applicable
 * @warning Warning if applicable
 * 
 * Example:
 * @code
 * result = function_name(arg1, arg2);
 * @endcode
 */
```

### File Documentation

```c
/**
 * @file module.c
 * @brief Brief description of module
 * @author Your Name
 * @version 1.0
 * 
 * Longer description of what this module does.
 */
```

### Implementation Comments

```c
/* Explain WHY, not WHAT */
pmm.bitmap = (uint8_t *)0x1000;  /* Bitmap placed before kernel */
                                  /* to preserve lower memory */

/* Avoid obvious comments */
x = y + 1;  /* Don't write "add 1 to y" */

/* Use comments for non-obvious algorithms */
/* Find first free page using linear search over bitmap */
for (uint32_t i = 0; i < pmm.total_pages; i++) {
    ...
}
```

## Testing

### Manual Testing

1. **Build and run**:
   ```bash
   make clean
   make all
   make run
   ```

2. **Test in shell**:
   - Try new commands
   - Use `mem` to check memory
   - Use `ps` to check processes
   - Look for error messages

3. **Debug output**:
   ```c
   vga_putstr("Debug: value = ", VGA_COLOR_YELLOW);
   serial_putstr("Debug message\n");  /* For serial logging */
   ```

### Performance Testing

- Monitor with `mem` command
- Check process runtimes with `ps`
- Look for memory leaks

### Edge Cases

- Empty input
- Maximum values
- Invalid parameters
- Boundary conditions

## Pull Request Process

### Before Submitting

1. ✅ Code follows style guide
2. ✅ Documentation updated
3. ✅ Tested on QEMU
4. ✅ No new compiler warnings
5. ✅ Commit messages are clear
6. ✅ Related issues referenced

### PR Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix (fixes #issue)
- [ ] New feature (relates to #issue)
- [ ] Documentation update
- [ ] Refactoring

## Testing Done
- [ ] Tested on QEMU
- [ ] Shell commands work
- [ ] Memory usage normal
- [ ] No regressions

## Documentation
- [ ] Updated DOCUMENTATION.md
- [ ] Updated API_REFERENCE.md
- [ ] Updated code comments

## Related Issues
Fixes #123
Related to #456
```

## Review Process

- Community reviews code
- Maintainers verify quality
- Changes tested
- Merged when approved

## Build & Test Commands

```bash
# Full build
make clean
make all

# Run in QEMU
make run

# Debug with GDB
make debug

# View structure
tree src/ include/

# Count LOC
wc -l src/**/*.c include/*.h

# Check for issues
grep "TODO\|FIXME\|HACK" src/kernel/**/*.c
```

## Common Issues

### Build Errors

**Problem**: `i686-elf-gcc: command not found`
```bash
# Install cross-compiler
sudo apt-get install crossbuild-essential-i386
```

**Problem**: `nasm: command not found`
```bash
# Install NASM assembler
sudo apt-get install nasm
```

### Runtime Issues

- Add debug output using `vga_putstr()`
- Check memory with `mem` command
- Monitor processes with `ps` command
- Use serial port for logging

## Resources

- [DOCUMENTATION.md](DOCUMENTATION.md) - Technical reference
- [API_REFERENCE.md](API_REFERENCE.md) - Function documentation
- [SOURCE_CODE_GUIDE.md](SOURCE_CODE_GUIDE.md) - Implementation details
- [VISUAL_REFERENCE.md](VISUAL_REFERENCE.md) - Architecture diagrams
- [OSDev Wiki](https://wiki.osdev.org/) - x86 reference
- [Intel x86 Manual](https://www.intel.com/content/dam/develop/external/us/en/documents/pdf/319973-014.pdf)

## Questions?

- Check [DOCUMENTATION_INDEX.md](DOCUMENTATION_INDEX.md) for help
- Review existing code for patterns
- Create a discussion issue on GitHub
- Contact maintainers

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

---

Thank you for contributing to NIMBU OS! 🎉

Happy coding! 🚀
