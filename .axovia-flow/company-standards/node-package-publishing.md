<<<<<<< HEAD
# Axovia Flow - Publishing SOP (Node Packages)

This document describes standard operating procedures for publishing Node packages at Axovia AI.

Applies to: @axovia/axovia-flow and similar CLI-oriented packages.

## Prerequisites
- Node.js 18+
- NPM 8+
- Authenticated to the private npm registry/scope (@axovia)
- Python 3.9+ available for packages that wrap Python CLIs

## Versioning
- Use Semantic Versioning (MAJOR.MINOR.PATCH)
- Bump version in package.json
- Changelog entry for each release

## CLI-only Publishing (Manual)
1. Clean and build (if applicable)
   - `npm ci`
   - `npm run test`
2. Smoke test locally
   - `node bin/axovia-flow.js --help`
3. Bump version in package.json
4. Publish (scoped, restricted)
   - `npm publish --access restricted`
5. Tag and push
   - `git tag v<version>`
   - `git push origin --tags`

## CI-driven Publishing
Trigger: tagged release (vX.Y.Z) on main

- CI validates: lint, tests, packaging
- If all green, CI publishes to npm with restricted access using an NPM_TOKEN with publish rights

Example GitHub Actions steps:
- checkout
- setup-node (Node 18)
- npm ci
- npm test
- npm publish --provenance --access restricted

## Rollback Procedure
- Deprecate the faulty version: `npm deprecate @axovia/axovia-flow@X.Y.Z "reason"`
- Publish a hotfix version: bump PATCH and publish

## Security and Access
- Use least-privileged npm tokens
- Rotate tokens every 90 days
- Keep publish rights limited to release engineers or CI only

## Notes
- This package is CLI-only for now. TypeScript types are provided as placeholders.
- Ensure README examples use the scoped package name: `npx @axovia/axovia-flow`

=======
# Node.js Package Publishing Standard

**Document Type**: Company Standard  
**Scope**: All Axovia AI Node.js packages and libraries  
**Status**: Active  
**Version**: 1.0  
**Effective Date**: 2025-09-08  
**Author**: Stephen Boyett  
**Company**: Axovia AI  

## Purpose

This document establishes the **SOLE SOURCE OF TRUTH** for Node.js package publishing standards across all Axovia AI projects. It defines requirements, processes, and quality gates for publishing packages to public and private registries.

## Scope and Authority

### Applicability

- **ALL** Node.js packages intended for distribution
- **ALL** CLI tools and utilities built with Node.js
- **ALL** libraries and frameworks for internal or external use
- **ALL** npm packages published to public or private registries

### Compliance Requirement

- **MANDATORY**: All packages MUST follow these standards before publishing
- **NO EXCEPTIONS**: Deviations require written CTO approval
- **ENFORCEMENT**: All packages must pass quality gates before publication

## Core Publishing Principles

### 1. Quality First

**Requirement**: No package may be published without comprehensive testing and validation.

**Implementation Standards**:

- 100% test coverage for public APIs
- All tests must pass in CI/CD pipeline
- Security vulnerability scanning completed
- Performance benchmarks established

### 2. Semantic Versioning

**Requirement**: All packages MUST follow semantic versioning (semver) strictly.

**Implementation Standards**:

- **MAJOR**: Breaking changes that require user code updates
- **MINOR**: New features that are backward compatible
- **PATCH**: Bug fixes that are backward compatible
- **Pre-release**: Use alpha, beta, rc suffixes for pre-release versions

### 3. Comprehensive Documentation

**Requirement**: All packages must have complete, accurate documentation.

**Implementation Standards**:

- README.md with installation, usage, and examples
- API documentation for all public interfaces
- CHANGELOG.md with detailed version history
- Contributing guidelines and code of conduct

### 4. Security and Compliance

**Requirement**: All packages must meet security and compliance standards.

**Implementation Standards**:

- No secrets or credentials in package contents
- Dependency vulnerability scanning
- License compliance verification
- Privacy and data protection compliance

## Package Structure Requirements

### Mandatory Files

Every publishable package MUST include:

```md
package.json          # Package metadata and dependencies
README.md            # Comprehensive documentation
LICENSE              # Open source license
CHANGELOG.md         # Version history and changes
.npmignore          # Files to exclude from package
```

### Recommended Files

```md
CONTRIBUTING.md      # Contribution guidelines
CODE_OF_CONDUCT.md   # Community standards
SECURITY.md          # Security policy and reporting
docs/               # Additional documentation
examples/           # Usage examples
```

### Package.json Requirements

**Mandatory Fields**:

```json
{
  "name": "@axovia-ai/package-name",
  "version": "1.0.0",
  "description": "Clear, concise package description",
  "main": "index.js",
  "author": "Axovia AI",
  "license": "MIT",
  "repository": {
    "type": "git",
    "url": "https://github.com/Axovia-AI/package-name.git"
  },
  "bugs": {
    "url": "https://github.com/Axovia-AI/package-name/issues"
  },
  "homepage": "https://github.com/Axovia-AI/package-name#readme",
  "keywords": ["relevant", "searchable", "keywords"],
  "engines": {
    "node": ">=16.0.0",
    "npm": ">=8.0.0"
  }
}
```

**CLI Package Requirements**:

```json
{
  "bin": {
    "package-name": "./bin/package-name.js"
  },
  "preferGlobal": true,
  "files": [
    "bin/",
    "lib/",
    "README.md",
    "LICENSE"
  ]
}
```

## Publishing Process

### Phase 1: Pre-Publication Validation (MANDATORY)

**Quality Gates Checklist**:

- [ ] All tests pass with 100% coverage for public APIs
- [ ] Security vulnerability scan completed (0 high/critical vulnerabilities)
- [ ] Documentation review completed and approved
- [ ] License compliance verified
- [ ] Version number follows semantic versioning
- [ ] CHANGELOG.md updated with all changes
- [ ] Package size optimized (< 10MB for most packages)
- [ ] Dependencies audited and minimized

**Validation Commands**:

```bash
# Run all tests
npm test

# Security audit
npm audit --audit-level high

# Package size check
npm pack --dry-run

# Lint package.json
npm run lint:package

# Validate documentation
npm run docs:validate
```

### Phase 2: Registry Selection

**Public NPM Registry** (for open source packages):

```bash
# Verify registry
npm config get registry

# Should return: https://registry.npmjs.org/
```

**GitHub Packages** (for organization packages):

```bash
# Configure registry for scoped packages
npm config set @axovia-ai:registry https://npm.pkg.github.com
```

**Private Registry** (for internal packages):

```bash
# Configure private registry
npm config set registry https://your-private-registry.com
```

### Phase 3: Publication Execution

**Pre-Publication Steps**:

```bash
# Clean build
npm run clean
npm run build

# Final test run
npm test

# Version bump (if needed)
npm version patch|minor|major

# Create git tag
git tag v$(node -p "require('./package.json').version")
```

**Publication Commands**:

```bash
# Dry run first
npm publish --dry-run

# Publish to public registry
npm publish

# Publish to GitHub Packages
npm publish --registry=https://npm.pkg.github.com

# Publish with specific tag
npm publish --tag beta
```

**Post-Publication Steps**:

```bash
# Push tags to repository
git push origin --tags

# Create GitHub release
gh release create v1.0.0 --title "Release v1.0.0" --notes-file CHANGELOG.md

# Update documentation
npm run docs:deploy
```

## Version Management Standards

### Version Numbering

- **Format**: MAJOR.MINOR.PATCH
- **Pre-release**: MAJOR.MINOR.PATCH-alpha.1
- **Build metadata**: MAJOR.MINOR.PATCH+build.1

### Release Types

- **Alpha**: Early development, unstable API
- **Beta**: Feature complete, API stable, testing phase
- **Release Candidate (RC)**: Production ready, final testing
- **Stable**: Production ready, fully tested

### Deprecation Policy

- **Notice Period**: 6 months minimum for major version changes
- **Documentation**: Clear migration guides for breaking changes
- **Support**: Previous major version supported for 12 months

## Security Requirements

### Dependency Management

```bash
# Regular dependency audits
npm audit

# Update dependencies
npm update

# Check for outdated packages
npm outdated
```

### Secret Management

- **NO SECRETS**: Never include API keys, passwords, or tokens
- **Environment Variables**: Use environment variables for configuration
- **Documentation**: Clear instructions for required environment setup

### Vulnerability Response

- **Immediate**: Patch critical vulnerabilities within 24 hours
- **High**: Patch high vulnerabilities within 1 week
- **Medium/Low**: Include in next regular release cycle

## Documentation Standards

### README.md Requirements

```markdown
# Package Name

Brief description of what the package does.

## Installation

\`\`\`bash
npm install @axovia-ai/package-name
\`\`\`

## Usage

\`\`\`javascript
const package = require('@axovia-ai/package-name');
// Usage examples
\`\`\`

## API Documentation

### Methods
- `method1(param)` - Description
- `method2(param)` - Description

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md)

## License

MIT - see [LICENSE](LICENSE)
```

### CHANGELOG.md Format

```markdown
# Changelog

## [1.0.0] - 2025-09-08

### Added
- New feature descriptions

### Changed
- Modified feature descriptions

### Deprecated
- Features marked for removal

### Removed
- Removed features

### Fixed
- Bug fixes

### Security
- Security improvements
```

## Quality Assurance

### Automated Testing Requirements

- **Unit Tests**: 100% coverage for public APIs
- **Integration Tests**: All major workflows tested
- **End-to-End Tests**: CLI tools fully tested
- **Performance Tests**: Benchmarks for critical paths

### Manual Review Requirements

- **Code Review**: All changes reviewed by senior developer
- **Documentation Review**: Technical writer approval
- **Security Review**: Security team approval for public packages

### Continuous Integration

```yaml
# Required CI checks
- lint
- test
- security-audit
- build
- package-size-check
- documentation-build
```

## Monitoring and Maintenance

### Post-Publication Monitoring

- **Download Statistics**: Track package adoption
- **Issue Tracking**: Monitor GitHub issues and npm feedback
- **Security Alerts**: Automated vulnerability monitoring
- **Performance Metrics**: Monitor package performance in production

### Maintenance Schedule

- **Weekly**: Dependency updates and security patches
- **Monthly**: Performance review and optimization
- **Quarterly**: Major version planning and deprecation review
- **Annually**: Complete package audit and modernization

## Compliance Checklist

### Pre-Publication Checklist

- [ ] Package follows semantic versioning
- [ ] All tests pass with required coverage
- [ ] Security audit completed with no critical issues
- [ ] Documentation complete and accurate
- [ ] License properly configured
- [ ] Repository properly configured with issues and homepage
- [ ] CHANGELOG.md updated
- [ ] Version tagged in git
- [ ] Package size optimized
- [ ] Dependencies minimized and audited

### Post-Publication Checklist

- [ ] Package successfully published to registry
- [ ] Installation tested from registry
- [ ] Documentation deployed and accessible
- [ ] GitHub release created
- [ ] Team notified of new release
- [ ] Monitoring configured for new version

---

**AUTHORITY**: This document is the official company standard for Node.js package publishing. All packages must comply with these requirements. Questions or requests for exceptions must be directed to the Engineering Leadership Team.

**MAINTENANCE**: This standard is reviewed quarterly and updated as needed. All changes require formal approval and communication to all engineering teams.

**EFFECTIVE DATE**: This standard is effective immediately for all new packages and must be implemented for existing packages within 60 days.
>>>>>>> agent-dev
