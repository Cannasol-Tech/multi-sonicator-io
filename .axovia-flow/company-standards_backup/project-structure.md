# Project Directory Structure Official Standard

```text
/**
 * Project Directory Structure Official Standard
 *
 * @author: Stephen Boyett
 * @date: 2025-08-28
 * @version: 1.1.0
 * @description: Official directory structure standard for all projects within the Stephen Boyett organization ecosystem.
 *
 * SCOPE: This standard applies to all projects created by or for:
 * - Axovia AI
 * - Silverline Software
 * - True Tune
 * - Cannasol Technologies
 * - Wylinek
 * - Any standalone projects owned by Stephen Boyett
 *
 * CORE PRINCIPLES:
 * 1. Consistency - Uniform structure, naming, and organization across all projects
 * 2. Efficiency - Minimize redundancy, maximize reusability and maintainability
 * 3. Scalability - Support growth from prototype to enterprise-scale applications
 * 4. Quality - Enable comprehensive testing, monitoring, and deployment practices
 * 5. Learning - Structure promotes knowledge transfer and continuous improvement
 *
 */
```

## Overview

This document defines the complete standardized directory structure and architectural patterns for all projects within the Stephen Boyett organization ecosystem. This standard ensures consistency, maintainability, and scalability across all projects regardless of technology stack or project size.

## Root Directory Standard

**FUNDAMENTAL RULE: MAINTAIN A CLEAN ROOT DIRECTORY**

The root directory should contain only essential project files that provide immediate context and control.

### Mandatory Root Directory Files

**These files MUST be present in every project root:**

1. **`README.md`** - Project documentation and quick start guide (mandatory)
2. **`LICENSE`** - Project license and usage terms (mandatory)
3. **`Makefile`** - Development workflow automation targets (mandatory)
4. **`.gitignore`** - Version control exclusions (mandatory)

### Conditional Root Directory Files

**These files are permitted when applicable:**

5. **`.gitmodules`** - Git submodule definitions (if using submodules)
6. **`.npmignore`** - NPM package exclusions (if publishing to NPM)
7. **`package.json`** - Node.js project metadata (if Node.js project)
8. **`requirements.txt`** - Python dependencies (if Python project)
9. **`Cargo.toml`** - Rust project metadata (if Rust project)
10. **`go.mod`** - Go module definition (if Go project)

### Prohibited Root Directory Files

**NEVER place these in the root directory:**
- Build artifacts: `dist/`, `build/`, compiled binaries, `node_modules/`
- IDE files: `.vscode/`, `.idea/`, `*.swp`, `.vs/`
- OS files: `.DS_Store`, `Thumbs.db`, `desktop.ini`
- Lock files: `package-lock.json`, `yarn.lock`, `Pipfile.lock` (use appropriate directories)
- Individual test files (use `test/` directory structure)
- Individual documentation files (use `docs/` directory structure)
- Individual configuration files (use `config/` directory structure)
- Individual source code files (use `src/` directory structure)
- Individual scripts (use `scripts/` or `bin/` directories)
- Temporary files: `*.tmp`, `*.log`, `*.cache`

### Root Directory Organization Principle

**Decision Rule**: If a file is not immediately needed by a developer or CI/CD system to understand or build the project, it belongs in a subdirectory.

See [Root Directory Standard](root-directory.md) for complete implementation details.

## Complete Directory Structure Standard

```text
project-root/
├── .axovia-flow/              # Axovia Flow extension configuration (if applicable)
│   ├── flow-config.yaml       # Extension configuration
│   ├── company-standards/     # Organization-specific standards
│   ├── templates/             # Custom templates
│   └── tasks/                 # Custom workflow tasks
├── .bmad-core/                # BMad-Core integration (if applicable)
│   ├── core-config.yaml       # BMad configuration
│   ├── agents/                # AI agent definitions
│   ├── templates/             # BMad templates
│   ├── tasks/                 # BMad workflow tasks
│   └── data/                  # Knowledge base and preferences
├── bin/                       # Executable scripts and binaries
│   ├── setup.sh               # Environment setup script
│   ├── deploy.sh              # Deployment script
│   └── cleanup.sh             # Cleanup utilities
├── config/                    # Configuration files
│   ├── development.yaml       # Development environment config
│   ├── staging.yaml           # Staging environment config
│   ├── production.yaml        # Production environment config
│   └── local.yaml.example     # Local configuration template
├── docs/                      # All project documentation
│   ├── README.md              # Documentation index
│   ├── prd.md                 # Product Requirements Document
│   ├── architecture.md        # Technical Architecture Document
│   ├── architecture/          # Detailed architecture documentation
│   │   ├── README.md          # Architecture overview
│   │   ├── coding-standards.md # Code quality standards
│   │   ├── tech-stack.md      # Technology decisions
│   │   ├── source-tree.md     # Source code organization
│   │   ├── database-design.md # Database architecture
│   │   ├── api-design.md      # API specifications
│   │   └── deployment.md      # Deployment architecture
│   ├── agile/                 # Agile development artifacts
│   │   ├── README.md          # Agile process overview
│   │   ├── epics/             # Epic definitions
│   │   │   ├── README.md      # Epic management guide
│   │   │   └── epic-*.md      # Individual epic files
│   │   ├── features/          # Feature specifications
│   │   │   ├── README.md      # Feature process guide
│   │   │   └── feature-*.md   # Individual feature files
│   │   └── stories/           # User story definitions
│   │       ├── README.md      # Story management guide
│   │       └── story-*.md     # Individual story files
│   ├── planning/              # Project planning documents
│   │   ├── README.md          # Planning overview
│   │   ├── market-research.md # Market analysis
│   │   ├── competitive-analysis.md # Competitor research
│   │   └── roadmap.md         # Product roadmap
│   ├── qa/                    # Quality assurance documentation
│   │   ├── README.md          # QA process overview
│   │   ├── test-plan.md       # Testing strategy
│   │   ├── gates/             # Quality gates
│   │   └── assessments/       # QA assessments
│   └── api/                   # API documentation
│       ├── README.md          # API overview
│       ├── openapi.yaml       # OpenAPI specification
│       └── examples/          # API usage examples
├── scripts/                   # Development and automation scripts
│   ├── build.sh               # Build automation
│   ├── test.sh                # Test execution
│   ├── lint.sh                # Code quality checks
│   ├── format.sh              # Code formatting
│   └── migrate.sh             # Database migrations
├── src/                       # Source code (structure varies by language)
│   ├── main/                  # Main application code
│   │   ├── java/              # Java source (if Java project)
│   │   ├── python/            # Python source (if Python project)
│   │   ├── javascript/        # JavaScript source (if JS project)
│   │   └── resources/         # Application resources
│   ├── test/                  # Test code
│   │   ├── unit/              # Unit tests
│   │   ├── integration/       # Integration tests
│   │   └── e2e/               # End-to-end tests
│   └── shared/                # Shared utilities and libraries
├── test/                      # Additional testing resources
│   ├── fixtures/              # Test data and fixtures
│   ├── mocks/                 # Mock objects and services
│   └── reports/               # Test execution reports
└── tools/                     # Development tools and utilities
    ├── generators/            # Code generators
    ├── validators/            # Validation tools
    └── analyzers/             # Code analysis tools
```

## Directory Purpose and Guidelines

### Core Infrastructure Directories

#### `.axovia-flow/` - Axovia Flow Extension
- **Purpose**: Axovia Flow-specific configuration and extensions
- **When Required**: Projects using Axovia Flow orchestration
- **Key Files**: `flow-config.yaml`, company standards, custom templates
- **Maintenance**: Managed by Axovia Flow system

#### `.bmad-core/` - BMad-Core Integration
- **Purpose**: BMad methodology integration and AI agent definitions
- **When Required**: Projects using BMad development methodology
- **Key Files**: `core-config.yaml`, agent definitions, workflow templates
- **Maintenance**: Managed by BMad-Core system

#### `bin/` - Executable Scripts
- **Purpose**: Project-specific executable scripts and binaries
- **Contents**: Setup scripts, deployment tools, utility binaries
- **Requirements**: All scripts must be executable and documented
- **Naming**: Use kebab-case for script names

#### `config/` - Configuration Management
- **Purpose**: Environment-specific configuration files
- **Structure**: Separate files per environment (dev, staging, prod)
- **Security**: Never commit sensitive data; use `.example` templates
- **Format**: Prefer YAML over JSON for readability

### Documentation Directories

#### `docs/` - Project Documentation
- **Purpose**: Comprehensive project documentation hub
- **Structure**: Organized by document type and audience
- **Requirements**: Every subdirectory must have a README.md
- **Maintenance**: Keep synchronized with code changes

#### `docs/architecture/` - Technical Architecture
- **Purpose**: Detailed technical design and architecture decisions
- **Key Files**: System design, coding standards, technology stack
- **Audience**: Developers, architects, technical stakeholders
- **Updates**: Must be updated with significant architectural changes

#### `docs/agile/` - Agile Development Artifacts
- **Purpose**: Agile methodology artifacts and planning documents
- **Structure**: Organized by artifact type (epics, features, stories)
- **Integration**: Synchronized with project management tools
- **Naming**: Use consistent naming patterns for traceability

### Source Code Directories

#### `src/` - Source Code Organization
- **Purpose**: All application source code and resources
- **Structure**: Language-specific organization patterns
- **Separation**: Clear separation between main code and test code
- **Standards**: Follow language-specific conventions

#### `test/` - Testing Resources
- **Purpose**: Test data, fixtures, and additional testing resources
- **Organization**: By test type (unit, integration, e2e)
- **Coverage**: Maintain comprehensive test coverage
- **Automation**: Integrate with CI/CD pipelines

### Utility Directories

#### `scripts/` - Development Scripts
- **Purpose**: Development workflow automation scripts
- **Contents**: Build, test, lint, format, and deployment scripts
- **Requirements**: All scripts must be idempotent and documented
- **Integration**: Called by Makefile targets

#### `tools/` - Development Tools
- **Purpose**: Custom development tools and utilities
- **Contents**: Code generators, validators, analyzers
- **Maintenance**: Keep tools updated with project evolution
- **Documentation**: Each tool must have usage documentation

## Implementation Guidelines

### Project Initialization Checklist

**Phase 1: Core Structure**
- [ ] Create root directory with mandatory files
- [ ] Initialize version control with appropriate `.gitignore`
- [ ] Create `Makefile` with essential targets
- [ ] Write comprehensive `README.md`
- [ ] Add appropriate `LICENSE` file

**Phase 2: Directory Structure**
- [ ] Create all mandatory directories with README.md files
- [ ] Set up configuration management structure
- [ ] Initialize documentation framework
- [ ] Create script templates and utilities
- [ ] Set up testing infrastructure

**Phase 3: Integration Setup**
- [ ] Configure Axovia Flow integration (if applicable)
- [ ] Set up BMad-Core integration (if applicable)
- [ ] Initialize CI/CD pipeline configuration
- [ ] Set up development environment automation
- [ ] Configure quality gates and validation

### Naming Conventions

#### File Naming
- **Documentation**: Use kebab-case (`api-design.md`, `test-plan.md`)
- **Configuration**: Use environment names (`development.yaml`, `production.yaml`)
- **Scripts**: Use kebab-case with action verbs (`build.sh`, `deploy.sh`)
- **Source Code**: Follow language-specific conventions

#### Directory Naming
- **Standard Directories**: Use lowercase with hyphens (`docs/`, `src/`, `config/`)
- **Subdirectories**: Use descriptive names (`architecture/`, `agile/`, `planning/`)
- **Avoid**: Spaces, special characters, camelCase in directory names

### Maintenance Requirements

#### Documentation Synchronization
- Update architecture docs with code changes
- Keep agile artifacts current with development progress
- Maintain API documentation with code changes
- Review and update standards quarterly

#### Directory Hygiene
- Regular cleanup of temporary files and build artifacts
- Archive completed agile artifacts appropriately
- Maintain consistent README.md files across directories
- Enforce naming conventions through automation

#### Quality Assurance
- Validate directory structure in CI/CD pipelines
- Automated checks for required files and documentation
- Regular audits of project structure compliance
- Training and onboarding materials for new team members

## Technology-Specific Adaptations

### Node.js Projects
```text
src/
├── main/
│   ├── javascript/
│   │   ├── controllers/
│   │   ├── services/
│   │   ├── models/
│   │   └── utils/
│   └── resources/
└── test/
    ├── unit/
    ├── integration/
    └── fixtures/
```

### Python Projects
```text
src/
├── main/
│   ├── python/
│   │   ├── package_name/
│   │   │   ├── __init__.py
│   │   │   ├── models/
│   │   │   ├── services/
│   │   │   └── utils/
│   │   └── resources/
└── test/
    ├── unit/
    ├── integration/
    └── fixtures/
```

### Java Projects
```text
src/
├── main/
│   ├── java/
│   │   └── com/company/project/
│   │       ├── controller/
│   │       ├── service/
│   │       ├── model/
│   │       └── util/
│   └── resources/
└── test/
    ├── java/
    └── resources/
```

## Compliance and Validation

### Automated Validation
- Directory structure validation scripts
- Required file presence checks
- Naming convention enforcement
- Documentation completeness verification

### Manual Review Process
- Quarterly structure compliance audits
- New project structure reviews
- Standards evolution and updates
- Team training and knowledge transfer

### Exception Handling
- Document any deviations from standard structure
- Obtain approval for structural modifications
- Update standards based on validated improvements
- Maintain backward compatibility when possible

---

**Document Version**: 1.1.0
**Last Updated**: 2025-08-28
**Next Review**: 2025-11-28
**Owner**: Stephen Boyett
**Reviewers**: Architecture Team, Development Leads
