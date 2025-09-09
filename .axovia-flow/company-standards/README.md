# Project Standards Documentation

## Overview

This directory is managed by the Axovia Flow project and meant to be the sole source of truth for Company Wide standards.  These standards are meant to be used across companies and adhered to for all Cannasol Technologies, Axovia AI, Silverline Software, or True Tune projects.  Also, when specified in any other project.  These company standard operating procedures should be strictly adhered to by all developers, agents, engineers and any one else working within the project's repository.

## Company Standard Operating Procedures Index

- [Coding Style Standards](./coding-style.md)
- [Root Directory Standard](./root-directory.md)
- [Project Structure Standards](./project-structure.md)
- [Repository Auditing — SOP](./repository-auditing.md)
- [SW Testing Standard](./sw-testing-standard.md)
- [Release Reporting Standard](./release-format.md)
- [File Header Standard](./file-header-standard.md)
- [Flutter Testing Standard](./flutter-testing-standard.md)
- [Executive Report Standard](./executive-report-standard.md)
- [React Testing Standard](./react-specific/react-testing-standard.md)
- [PlatformIO Testing Standard](./platformio-specific/platformio-testing-standard.md)
- [React Coding Standards](./react-specific/coding-standards.md)
- [Flutter Coding Standards](./flutter-specific/coding-standards.md)
- [PlatformIO Coding Standards](./platformio-specific/coding-standards.md)
- [Flutter Testing Standards](./flutter-specific/flutter-testing-standard.md)
- [React Testing Standards](./react-specific/react-testing-standard.md)
- [PlatformIO Testing Standards](./platformio-specific/platformio-testing-standard.md)

## Directory Purpose

This directory is used for storing all company-wide project standards.  This includes the coding standards, testing standards, and any other standards related documents.  This directory should be the single source of truth for all project standards.  The coding standards should definitely live here and any other standards related documents should also be stored here for a given project.

### Importing Standards to Other Repositories

#### Overview

These company standards are designed to be imported and used across all Cannasol Technologies, Axovia AI, Silverline Software, and True Tune projects. This section provides detailed step-by-step instructions for importing these standards into other repositories.

#### Method 1: Git Submodule (Recommended)

Git submodules provide the most maintainable approach for importing standards, as they allow automatic updates when standards are revised.

#### Step 1: Add the Standards Submodule

```bash
# Navigate to your target repository
cd /path/to/your/project

# Add the axovia-flow repository as a submodule
git submodule add https://github.com/Axovia-AI/axovia-flow.git .standards-source

# Initialize and update the submodule
git submodule update --init --recursive
```

#### Step 2: Create Standards Directory Structure

```bash
# Create the standards directory in your project
mkdir -p .axovia-flow/company-standards

# Create subdirectories for framework-specific standards
mkdir -p .axovia-flow/company-standards/flutter-specific
mkdir -p .axovia-flow/company-standards/platformio-specific
mkdir -p .axovia-flow/company-standards/react-specific
```

#### Step 3: Set Up Sync Script

Create a script to sync standards from the submodule to your project:

```bash
# Create the sync script
cat > scripts/sync-company-standards.sh << 'EOF'
#!/usr/bin/env bash
set -euo pipefail

# Company Standards Sync Script
# Syncs standards from axovia-flow submodule to local .axovia-flow/company-standards

SUBMODULE_PATH=".standards-source"
SOURCE_DIR="$SUBMODULE_PATH/.axovia-flow/company-standards"
TARGET_DIR=".axovia-flow/company-standards"

echo "Syncing company standards from $SOURCE_DIR to $TARGET_DIR..."

# Ensure submodule is up to date
git submodule update --remote $SUBMODULE_PATH

# Sync all standard files
rsync -av --delete \
  --exclude="README.md" \
  "$SOURCE_DIR/" "$TARGET_DIR/"

# Create a custom README that references the source
cat > "$TARGET_DIR/README.md" << 'INNER_EOF'
# Project Standards Documentation

## Overview

This directory contains company-wide standards imported from the [Axovia Flow](https://github.com/Axovia-AI/axovia-flow) repository. These standards are the single source of truth for all Cannasol Technologies, Axovia AI, Silverline Software, and True Tune projects.

**⚠️ DO NOT EDIT FILES DIRECTLY**: These files are automatically synced from the source repository. To modify standards, submit changes to the [axovia-flow repository](https://github.com/Axovia-AI/axovia-flow/tree/main/.axovia-flow/company-standards).

## Last Sync

Standards last synced on: $(date)
Source commit: $(cd $SUBMODULE_PATH && git rev-parse HEAD)

## Available Standards

INNER_EOF

# Append the standards index from the source
if [[ -f "$SOURCE_DIR/README.md" ]]; then
  # Extract the standards index from source README
  sed -n '/## Company Standard Operating Procedures Index/,/## Directory Purpose/p' "$SOURCE_DIR/README.md" | head -n -1 >> "$TARGET_DIR/README.md"
fi

echo "Standards sync completed successfully!"
echo "Updated files in $TARGET_DIR"
EOF

# Make the script executable
chmod +x scripts/sync-company-standards.sh
```

#### Step 4: Run Initial Sync

```bash
# Run the sync script to import all standards
./scripts/sync-company-standards.sh

# Commit the changes
git add .
git commit -m "Import company standards via submodule

- Added axovia-flow as submodule for standards source
- Created sync script for automated standards updates
- Imported all current company standards"
```

#### Step 5: Set Up Automated Updates (Optional)

Add a GitHub Action or CI job to automatically sync standards:

```yaml
# .github/workflows/sync-standards.yml
name: Sync Company Standards

on:
  schedule:
    # Run weekly on Mondays at 9 AM UTC
    - cron: '0 9 * * 1'
  workflow_dispatch: # Allow manual trigger

jobs:
  sync-standards:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: true
          token: ${{ secrets.GITHUB_TOKEN }}

      - name: Sync Standards
        run: |
          ./scripts/sync-company-standards.sh

      - name: Create Pull Request
        uses: peter-evans/create-pull-request@v5
        with:
          token: ${{ secrets.GITHUB_TOKEN }}
          commit-message: "chore: sync company standards"
          title: "Sync Company Standards"
          body: |
            Automated sync of company standards from axovia-flow repository.

            Please review changes and merge if appropriate.
          branch: sync-standards
```

### Method 2: Direct Copy (Alternative)

If submodules are not suitable for your project, you can directly copy the standards files.

#### Step 1: Clone the Source Repository

```bash
# Clone the axovia-flow repository temporarily
git clone https://github.com/Axovia-AI/axovia-flow.git /tmp/axovia-flow-standards
```

#### Step 2: Copy Standards Files

```bash
# Navigate to your target repository
cd /path/to/your/project

# Create standards directory structure
mkdir -p .axovia-flow/company-standards/flutter-specific
mkdir -p .axovia-flow/company-standards/platformio-specific
mkdir -p .axovia-flow/company-standards/react-specific

# Copy all standards files
cp -r /tmp/axovia-flow-standards/.axovia-flow/company-standards/* .axovia-flow/company-standards/

# Clean up temporary clone
rm -rf /tmp/axovia-flow-standards
```

#### Step 3: Create Update Script

```bash
# Create update script for future syncing
cat > scripts/update-company-standards.sh << 'EOF'
#!/usr/bin/env bash
set -euo pipefail

# Company Standards Update Script
# Downloads latest standards from axovia-flow repository

TEMP_DIR=$(mktemp -d)
REPO_URL="https://github.com/Axovia-AI/axovia-flow.git"
TARGET_DIR=".axovia-flow/company-standards"

echo "Downloading latest company standards..."

# Clone the repository
git clone "$REPO_URL" "$TEMP_DIR/axovia-flow"

# Backup current standards
if [[ -d "$TARGET_DIR" ]]; then
  cp -r "$TARGET_DIR" "$TARGET_DIR.backup.$(date +%Y%m%d_%H%M%S)"
fi

# Copy new standards
mkdir -p "$TARGET_DIR"
cp -r "$TEMP_DIR/axovia-flow/.axovia-flow/company-standards/"* "$TARGET_DIR/"

# Add source attribution to README
cat > "$TARGET_DIR/README.md" << 'INNER_EOF'
# Project Standards Documentation

## Overview

This directory contains company-wide standards copied from the [Axovia Flow](https://github.com/Axovia-AI/axovia-flow) repository. These standards are the single source of truth for all Cannasol Technologies, Axovia AI, Silverline Software, and True Tune projects.

**⚠️ IMPORTANT**: These files were copied on $(date). To get the latest updates, run `./scripts/update-company-standards.sh` or submit changes to the [axovia-flow repository](https://github.com/Axovia-AI/axovia-flow/tree/main/.axovia-flow/company-standards).

## Last Update

Standards last updated on: $(date)
Source: $REPO_URL

INNER_EOF

# Append standards index
if [[ -f "$TEMP_DIR/axovia-flow/.axovia-flow/company-standards/README.md" ]]; then
  sed -n '/## Company Standard Operating Procedures Index/,/## Directory Purpose/p' "$TEMP_DIR/axovia-flow/.axovia-flow/company-standards/README.md" | head -n -1 >> "$TARGET_DIR/README.md"
fi

# Clean up
rm -rf "$TEMP_DIR"

echo "Standards updated successfully!"
echo "Review changes in $TARGET_DIR and commit when ready."
EOF

# Make the script executable
chmod +x scripts/update-company-standards.sh
```

### Method 3: Selective Import

For projects that only need specific standards, you can selectively import individual files.

#### Step 1: Identify Required Standards

Review the available standards and identify which ones apply to your project:

- **Universal Standards** (recommended for all projects):
  - `coding-style.md`
  - `root-directory.md`
  - `project-structure.md`
  - `sw-testing-standard.md`
  - `file-header.md`
  - `pull-requests.md`

- **Framework-Specific Standards** (choose based on your tech stack):
  - `react-specific/` - For React applications
  - `flutter-specific/` - For Flutter applications
  - `platformio-specific/` - For PlatformIO/embedded projects

#### Step 2: Create Selective Import Script

```bash
# Create selective import script
cat > scripts/import-selected-standards.sh << 'EOF'
#!/usr/bin/env bash
set -euo pipefail

# Selective Standards Import Script
# Imports only specified standards from axovia-flow repository

REPO_URL="https://github.com/Axovia-AI/axovia-flow.git"
TEMP_DIR=$(mktemp -d)
TARGET_DIR=".axovia-flow/company-standards"

# Define which standards to import (modify as needed)
UNIVERSAL_STANDARDS=(
  "coding-style.md"
  "root-directory.md"
  "project-structure.md"
  "sw-testing-standard.md"
  "file-header.md"
  "pull-requests.md"
)

# Define framework-specific directories to import (uncomment as needed)
FRAMEWORK_DIRS=(
  # "react-specific"
  # "flutter-specific"
  # "platformio-specific"
)

echo "Importing selected company standards..."

# Clone the repository
git clone "$REPO_URL" "$TEMP_DIR/axovia-flow"
SOURCE_DIR="$TEMP_DIR/axovia-flow/.axovia-flow/company-standards"

# Create target directory
mkdir -p "$TARGET_DIR"

# Import universal standards
for standard in "${UNIVERSAL_STANDARDS[@]}"; do
  if [[ -f "$SOURCE_DIR/$standard" ]]; then
    cp "$SOURCE_DIR/$standard" "$TARGET_DIR/"
    echo "Imported: $standard"
  else
    echo "Warning: $standard not found in source"
  fi
done

# Import framework-specific directories
for dir in "${FRAMEWORK_DIRS[@]}"; do
  if [[ -d "$SOURCE_DIR/$dir" ]]; then
    cp -r "$SOURCE_DIR/$dir" "$TARGET_DIR/"
    echo "Imported directory: $dir"
  else
    echo "Warning: $dir directory not found in source"
  fi
done

# Create custom README for selective import
cat > "$TARGET_DIR/README.md" << 'INNER_EOF'
# Project Standards Documentation

## Overview

This directory contains selected company-wide standards imported from the [Axovia Flow](https://github.com/Axovia-AI/axovia-flow) repository.

**⚠️ SELECTIVE IMPORT**: Only specific standards have been imported. To import additional standards or get updates, run `./scripts/import-selected-standards.sh` or visit the [source repository](https://github.com/Axovia-AI/axovia-flow/tree/main/.axovia-flow/company-standards).

## Imported Standards

INNER_EOF

# List imported files
echo "### Universal Standards" >> "$TARGET_DIR/README.md"
for standard in "${UNIVERSAL_STANDARDS[@]}"; do
  if [[ -f "$TARGET_DIR/$standard" ]]; then
    echo "- [$standard](./$standard)" >> "$TARGET_DIR/README.md"
  fi
done

if [[ ${#FRAMEWORK_DIRS[@]} -gt 0 ]]; then
  echo "" >> "$TARGET_DIR/README.md"
  echo "### Framework-Specific Standards" >> "$TARGET_DIR/README.md"
  for dir in "${FRAMEWORK_DIRS[@]}"; do
    if [[ -d "$TARGET_DIR/$dir" ]]; then
      echo "- **$dir/**: Framework-specific standards" >> "$TARGET_DIR/README.md"
    fi
  done
fi

echo "" >> "$TARGET_DIR/README.md"
echo "## Last Import" >> "$TARGET_DIR/README.md"
echo "" >> "$TARGET_DIR/README.md"
echo "Standards imported on: $(date)" >> "$TARGET_DIR/README.md"
echo "Source: $REPO_URL" >> "$TARGET_DIR/README.md"

# Clean up
rm -rf "$TEMP_DIR"

echo "Selective import completed successfully!"
echo "Imported standards are available in $TARGET_DIR"
EOF

# Make the script executable
chmod +x scripts/import-selected-standards.sh
```

#### Step 3: Customize and Run Import

```bash
# Edit the script to select which standards you need
nano scripts/import-selected-standards.sh

# Run the selective import
./scripts/import-selected-standards.sh

# Commit the imported standards
git add .axovia-flow/company-standards/ scripts/import-selected-standards.sh
git commit -m "Import selected company standards

- Imported universal coding and testing standards
- Added selective import script for future updates"
```

### Post-Import Integration Steps

After importing standards using any of the above methods, follow these steps to integrate them into your project:

#### Step 1: Update Project Documentation

```bash
# Update your main README.md to reference the standards
echo "
## Development Standards

This project follows company-wide development standards. See [.axovia-flow/company-standards/](./.axovia-flow/company-standards/) for detailed guidelines on:

- Code style and formatting
- Testing requirements and coverage
- Project structure conventions
- Pull request processes
- File header standards

All contributors must adhere to these standards.
" >> README.md
```

#### Step 2: Configure Development Tools

Update your project's development tools to enforce the imported standards:

**For JavaScript/Node.js projects:**
```bash
# Update package.json scripts
npm pkg set scripts.lint="eslint . --ext .js,.jsx,.ts,.tsx"
npm pkg set scripts.format="prettier --write ."
npm pkg set scripts.test="jest --coverage"
npm pkg set scripts.test:watch="jest --watch"

# Install required dependencies
npm install --save-dev eslint prettier jest
```

**For Python projects:**
```bash
# Update requirements-dev.txt or pyproject.toml
echo "flake8>=6.0.0
black>=23.0.0
pytest>=7.0.0
pytest-cov>=4.0.0" >> requirements-dev.txt

# Or for pyproject.toml projects
pip install black flake8 pytest pytest-cov
```

#### Step 3: Set Up Pre-commit Hooks

```bash
# Install pre-commit
pip install pre-commit

# Create .pre-commit-config.yaml
cat > .pre-commit-config.yaml << 'EOF'
repos:
  - repo: https://github.com/pre-commit/pre-commit-hooks
    rev: v4.4.0
    hooks:
      - id: trailing-whitespace
      - id: end-of-file-fixer
      - id: check-yaml
      - id: check-added-large-files

  - repo: https://github.com/psf/black
    rev: 23.3.0
    hooks:
      - id: black
        language_version: python3

  - repo: https://github.com/pycqa/flake8
    rev: 6.0.0
    hooks:
      - id: flake8
EOF

# Install the hooks
pre-commit install
```

#### Step 4: Update Makefile

Add or update your Makefile to include standard development commands:

```bash
# Add to existing Makefile or create new one
cat >> Makefile << 'EOF'

# Development Standards Compliance
.PHONY: lint format test test-coverage validate-standards

lint:
	@echo "Running code linting..."
	flake8 src/ tests/
	eslint . --ext .js,.jsx,.ts,.tsx || true

format:
	@echo "Formatting code..."
	black src/ tests/
	prettier --write . || true

test:
	@echo "Running tests..."
	pytest tests/ --cov=src --cov-report=html --cov-report=term-missing

test-coverage:
	@echo "Checking test coverage requirements..."
	pytest tests/ --cov=src --cov-fail-under=90

validate-standards:
	@echo "Validating compliance with company standards..."
	@./scripts/validate-standards-compliance.sh || echo "Create validate-standards-compliance.sh script"

EOF
```

### Maintenance and Updates

#### Keeping Standards Current

**For Submodule Method:**
```bash
# Update standards to latest version
git submodule update --remote .standards-source
./scripts/sync-company-standards.sh
git add .
git commit -m "Update company standards to latest version"
```

**For Direct Copy Method:**
```bash
# Update standards manually
./scripts/update-company-standards.sh
git add .axovia-flow/company-standards/
git commit -m "Update company standards"
```

**For Selective Import Method:**
```bash
# Update selected standards
./scripts/import-selected-standards.sh
git add .axovia-flow/company-standards/
git commit -m "Update selected company standards"
```

#### Monitoring for Standards Changes

Set up notifications to stay informed about standards updates:

1. **Watch the axovia-flow repository** on GitHub for notifications
2. **Subscribe to releases** to get notified of major standards updates
3. **Set up automated checks** in your CI/CD pipeline to detect when standards are out of date

#### Creating Standards Compliance Reports

Create a script to validate your project's compliance with imported standards:

```bash
cat > scripts/validate-standards-compliance.sh << 'EOF'
#!/usr/bin/env bash
set -euo pipefail

echo "=== Company Standards Compliance Check ==="
echo ""

# Check if standards directory exists
if [[ ! -d ".axovia-flow/company-standards" ]]; then
  echo "❌ FAIL: .axovia-flow/company-standards directory not found"
  exit 1
fi

echo "✅ PASS: Standards directory exists"

# Check for required files based on root-directory.md standard
REQUIRED_ROOT_FILES=(".gitignore" "README.md" "Makefile" "LICENSE")
for file in "${REQUIRED_ROOT_FILES[@]}"; do
  if [[ -f "$file" ]]; then
    echo "✅ PASS: Required root file exists: $file"
  else
    echo "❌ FAIL: Missing required root file: $file"
  fi
done

# Check test coverage if applicable
if [[ -f "pytest.ini" ]] || [[ -f "pyproject.toml" ]] || [[ -f "setup.cfg" ]]; then
  echo ""
  echo "Checking Python test coverage..."
  if command -v pytest >/dev/null 2>&1; then
    if pytest --cov=src --cov-report=term-missing --cov-fail-under=90 >/dev/null 2>&1; then
      echo "✅ PASS: Test coverage meets 90% requirement"
    else
      echo "❌ FAIL: Test coverage below 90% requirement"
    fi
  else
    echo "⚠️  WARN: pytest not available, cannot check coverage"
  fi
fi

# Check for proper project structure
EXPECTED_DIRS=("src" "tests" "docs" "config")
for dir in "${EXPECTED_DIRS[@]}"; do
  if [[ -d "$dir" ]]; then
    echo "✅ PASS: Standard directory exists: $dir"
  else
    echo "⚠️  WARN: Standard directory missing: $dir (may not be applicable)"
  fi
done

echo ""
echo "=== Compliance Check Complete ==="
EOF

chmod +x scripts/validate-standards-compliance.sh
```

### Troubleshooting Common Issues

#### Issue: Submodule Not Updating

**Problem**: `git submodule update` doesn't pull the latest standards.

**Solution**:
```bash
# Force update to latest remote version
git submodule update --remote --force .standards-source
./scripts/sync-company-standards.sh
```

#### Issue: Merge Conflicts During Standards Update

**Problem**: Local modifications conflict with updated standards.

**Solution**:
```bash
# Backup your local changes
cp .axovia-flow/company-standards/modified-file.md .axovia-flow/company-standards/modified-file.md.backup

# Reset to clean state
git checkout -- .axovia-flow/company-standards/

# Re-run standards sync
./scripts/sync-company-standards.sh

# Manually merge your changes if needed
```

#### Issue: Standards Script Permissions

**Problem**: Scripts are not executable after cloning.

**Solution**:
```bash
# Make all scripts executable
chmod +x scripts/*.sh

# Or specifically for standards scripts
chmod +x scripts/sync-company-standards.sh
chmod +x scripts/validate-standards-compliance.sh
```

#### Issue: Missing Dependencies for Standards Validation

**Problem**: Linting or testing tools required by standards are not installed.

**Solution**:
```bash
# For Python projects
pip install -r requirements-dev.txt
# or
pip install flake8 black pytest pytest-cov

# For Node.js projects
npm install --save-dev eslint prettier jest

# For pre-commit hooks
pip install pre-commit
pre-commit install
```

### Best Practices

#### 1. Regular Standards Updates

- **Weekly**: Check for standards updates in active development projects
- **Monthly**: Update standards in maintenance-mode projects
- **Before Major Releases**: Ensure compliance with latest standards

#### 2. Team Communication

- **Notify team members** when standards are updated
- **Document any project-specific exceptions** to standards
- **Include standards compliance** in code review checklists

#### 3. Gradual Adoption

For existing projects with extensive codebases:

```bash
# Create a migration plan
cat > STANDARDS_MIGRATION.md << 'EOF'
# Standards Migration Plan

## Phase 1: Infrastructure (Week 1)
- [ ] Import company standards
- [ ] Set up sync scripts
- [ ] Configure development tools
- [ ] Add Makefile targets

## Phase 2: New Code (Week 2-4)
- [ ] Apply standards to all new code
- [ ] Update code review process
- [ ] Train team on new standards

## Phase 3: Legacy Code (Month 2-3)
- [ ] Gradually refactor existing code
- [ ] Focus on high-traffic modules first
- [ ] Update tests to meet coverage requirements

## Phase 4: Full Compliance (Month 4)
- [ ] Achieve 100% standards compliance
- [ ] Enable automated enforcement
- [ ] Document any approved exceptions
EOF
```

#### 4. Continuous Integration

Add standards validation to your CI pipeline:

```yaml
# .github/workflows/standards-compliance.yml
name: Standards Compliance

on: [push, pull_request]

jobs:
  validate-standards:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: true

      - name: Set up Python
        uses: actions/setup-python@v4
        with:
          python-version: '3.11'

      - name: Install dependencies
        run: |
          pip install -r requirements-dev.txt

      - name: Validate Standards Compliance
        run: |
          ./scripts/validate-standards-compliance.sh

      - name: Check Code Formatting
        run: |
          black --check src/ tests/

      - name: Run Linting
        run: |
          flake8 src/ tests/

      - name: Verify Test Coverage
        run: |
          pytest tests/ --cov=src --cov-fail-under=90
```

### Support and Contributing

#### Getting Help

- **Documentation Issues**: Open an issue in the [axovia-flow repository](https://github.com/Axovia-AI/axovia-flow/issues)
- **Standards Questions**: Contact the development team or project maintainers
- **Implementation Support**: Refer to the specific standard documents for detailed guidance

#### Contributing to Standards

To propose changes or additions to company standards:

1. **Fork** the [axovia-flow repository](https://github.com/Axovia-AI/axovia-flow)
2. **Create a feature branch** for your proposed changes
3. **Update the relevant standard documents** in `.axovia-flow/company-standards/`
4. **Submit a pull request** with detailed explanation of the changes
5. **Participate in review process** with the standards committee

#### Reporting Issues

When reporting issues with standards import or compliance:

1. **Include your import method** (submodule, direct copy, or selective)
2. **Provide error messages** and relevant log output
3. **Describe your project structure** and technology stack
4. **Include steps to reproduce** the issue

---

## Quick Reference

### Import Commands Summary

```bash
# Method 1: Submodule (Recommended)
git submodule add https://github.com/Axovia-AI/axovia-flow.git .standards-source
./scripts/sync-company-standards.sh

# Method 2: Direct Copy
git clone https://github.com/Axovia-AI/axovia-flow.git /tmp/axovia-flow-standards
cp -r /tmp/axovia-flow-standards/.axovia-flow/company-standards/* .axovia-flow/company-standards/

# Method 3: Selective Import
./scripts/import-selected-standards.sh

# Update Standards
git submodule update --remote .standards-source  # For submodule method
./scripts/update-company-standards.sh            # For direct copy method
./scripts/import-selected-standards.sh           # For selective method

# Validate Compliance
./scripts/validate-standards-compliance.sh
make validate-standards
```

### Essential Files to Create

- `scripts/sync-company-standards.sh` (for submodule method)
- `scripts/update-company-standards.sh` (for direct copy method)
- `scripts/import-selected-standards.sh` (for selective method)
- `scripts/validate-standards-compliance.sh` (for all methods)
- `.pre-commit-config.yaml` (recommended)
- Updated `Makefile` with standards targets

## Directory Structure

```markdown
  standards/
    ├──flutter-specific/                  # Flutter Application specific company standards
    |    ├── coding-standards.md              # Flutter Coding Standards
    |    └── testing-standard.md              # Flutter Testing Standard
    ├──platformio-specific/               # PlatformIO Application specific company standards
    |    ├── coding-standards.md              # PlatformIO Coding Standards
    |    └── testing-standard.md              # PlatformIO Testing Standard
    ├──react-specific/                    # React Application specific company standards
    |    ├── coding-standards.md              # React Coding Standards
    |    └── testing-standard.md              # React Testing Standard
    ├──testing-standards-overview.md      # Overview of Testing Standards -- Outlines best practices for testing standards
    ├──testing-framework-guide.md         # Guide to Testing Frameworks -- Outlines best practices for testing frameworks
    ├──release-report.md                  # Release Report Standard -- Outlines best practices for generating release reports
    ├──coding-style.md                    # Coding Style Standard -- Outlines best practices for coding style
    ├──root-directory.md                  # Root Directory Standard -- Outlines best practices for managing clutter in the root directory of projects
    ├──project-structure.md               # Project Structure Standard -- Outlines best practices for managing the structure of projects
    ├──repository-auditing.md             # Repository Auditing Standard -- Outlines best practices for auditing repositories
    ├──sw-testing-standard.md             # SW Testing Standard -- Outlines best practices for testing software
    ├──file-header-standard.md            # File Header Standard -- Outlines best practices for file headers
    ├──flutter-testing-standard.md        # Flutter Testing Standard -- Outlines best practices for testing Flutter applications
    ├──executive-report-standard.md       # Executive Report Standard -- Outlines best practices for generating executive reports
    └──README.md                          # This file -- an overview of our Company Standards directory
```
