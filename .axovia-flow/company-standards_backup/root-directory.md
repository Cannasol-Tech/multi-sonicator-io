# Axovia Flow Root Directory Standard

## OFFICIAL STANDARD OF PROCEDURE RULE

**AXOVIA FLOW ROOT DIRECTORY FILE LIST**

This document establishes the official rule for what files are allowed in the root directory of any Axovia Flow project. This standard ensures clean repository organization, prevents clutter, and maintains consistency across all Axovia Flow implementations.

## Purpose

The root directory should contain only essential configuration and documentation files. All project code, tests, documentation, and supporting files should be organized in appropriate subdirectories. This standard prevents repository pollution and ensures consistent project structure across all Axovia Flow implementations.

## Official Root Directory File List

### ALLOWED FILES (MANDATORY)

The following files are permitted in the root directory:

#### 1. `.gitignore`
- **Purpose**: Define files and directories to exclude from version control
- **Required**: Yes
- **Template**: Must include standard exclusions for Node.js, Python, and development artifacts

#### 2. `.gitmodules`
- **Purpose**: Define Git submodules used by the project
- **Required**: Yes (if using submodules)
- **Usage**: For BMad-Core and other framework integrations

#### 3. `.npmignore`
- **Purpose**: Define files to exclude when publishing to npm
- **Required**: Yes
- **Template**: Must exclude development files, tests, and documentation

#### 4. `Makefile`
- **Purpose**: Provide make targets for common development workflows
- **Required**: Yes
- **Content**: Must include targets for `test`, `build`, `install`, `clean`, etc.

#### 5. `LICENSE`
- **Purpose**: Define the project's license terms
- **Required**: Yes
- **Format**: Standard license text (typically MIT, Apache 2.0, etc.)

#### 6. `README.md`
- **Purpose**: Provide project overview, installation, and usage instructions
- **Required**: Yes
- **Content**: Must follow the Axovia Flow README template

#### 7. `package.json`
- **Purpose**: Defines project metadata and npm dependencies
- **Required**: Yes
- **Usage**: Essential for `npm install` and script execution

#### 8. `package-lock.json`
- **Purpose**: Records the exact versions of dependencies
- **Required**: Yes
- **Usage**: Ensures reproducible builds with `npm ci`

## PROHIBITED FILES

### NEVER ALLOWED IN ROOT DIRECTORY

1. **Dependency files**: `requirements.txt`
2. **Build artifacts**: `dist/`, `build/`, compiled binaries
3. **IDE files**: `.vscode/`, `.idea/`, `*.swp`, `*.swo`
4. **OS files**: `.DS_Store`, `Thumbs.db`, desktop.ini
5. **Test files**: Individual test files (use `test/` directory)
6. **Documentation files**: Use `docs/` directory structure
7. **Configuration files**: Use `config/` directory structure
8. **Source code**: Use `src/` directory structure
9. **Scripts**: Use `scripts/` or `bin/` directories
10. **Archives**: `.tgz`, `.tar.gz`, `.zip` files

## Implementation Guidelines

### Directory Structure Enforcement

All Axovia Flow projects must follow this structure:

```markdown
<ROOT_DIR>/
├── .gitignore          # ✓ ALLOWED
├── .gitmodules         # ✓ ALLOWED
├── .npmignore          # ✓ ALLOWED
├── Makefile           # ✓ ALLOWED
├── LICENSE            # ✓ ALLOWED
├── README.md          # ✓ ALLOWED
├── bin/               # Executables and scripts
├── config/            # Configuration files
├── docs/              # Documentation
├── scripts/           # Utility scripts
├── src/               # Source code
├── test/             # Test suites
└── .bmad-core/        # Framework integration
```

### CI/CD Integration

1. **Pre-commit hooks** must validate root directory compliance
2. **CI pipelines** must fail if prohibited files are found in root
3. **Linting rules** must enforce this standard

### Migration Strategy

For existing projects:

1. **Audit current root directory** against this standard
2. **Move prohibited files** to appropriate subdirectories:
   - Source code → `src/`
   - Tests → `test/`
   - Documentation → `docs/`
   - Configuration → `config/`
   - Scripts → `scripts/` or `bin/`
3. **Update references** in documentation and code
4. **Test thoroughly** after migration

## Compliance Validation

### Automated Checks

Projects must implement automated validation:

```bash
# Example Makefile target
.PHONY: validate-root
validate-root:
	@echo "Validating root directory compliance..."
	@./scripts/validate-root-directory.sh
```

### Manual Review Checklist

- [ ] Only allowed files present in root (currently 8)
- [ ] No prohibited dependency files in root (e.g., `requirements.txt`)
- [ ] No build artifacts in root
- [ ] No IDE-specific files in root
- [ ] No OS-specific files in root
- [ ] All subdirectories properly organized

## Exceptions

### Temporary Exceptions

**Only allowed with explicit approval and documentation:**

1. **Emergency hotfixes** - May temporarily add files with immediate cleanup plan
2. **Experimental features** - May add files during development with cleanup commitment
3. **Third-party integrations** - May require additional root files per vendor specifications

### Permanent Exceptions

**None allowed.** All exceptions must be temporary with defined cleanup timelines.

## Enforcement

### Repository Owners

- **Must enforce** this standard in all Axovia Flow repositories
- **Must reject** pull requests that violate this rule
- **Must implement** automated validation in CI/CD

### Contributors

- **Must follow** this standard when contributing
- **Must move** any accidentally added files to appropriate directories
- **Must update** documentation references when moving files

### Tools and Automation

- **Pre-commit hooks** should validate compliance
- **CI/CD pipelines** should fail on violations
- **IDE configurations** should exclude prohibited root files

## Related Documentation

- [Project Structure Standards](project-structure.md)
- [Coding Style Standards](coding-style.md)
- [Architecture Overview](../architecture/overview.md)
- [BMad-Core Integration](../../.bmad-core/README.md)

## Version History

- **v1.0.0** (2025-08-28): Initial standard definition
- Future versions will be tracked with semantic versioning

---

**This document is the SINGLE SOURCE OF TRUTH for Axovia Flow root directory standards. All projects must comply with this rule.**
