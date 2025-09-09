# Project Initial Directory Structure Official Standard

```text

/**
 * Axovia Flow Project Directory Structure Official Standard
 * 
 * @author: Stephen Boyett`
 * @date: 2025-08-28
 * @version: 1.0.0
 * @description: This is the official - over-arching directory structure for all projects within our organization.  This directory structure should be used for all projects created by Cannasol-Tech, Axovia, Silverline, or True Tune.  T
 * 
 * This directory structure is based on the following principles:
 * 
 * 1. All projects should have a consistent directory structure.
 * 2. All projects should have a consistent naming convention.
 * 3. All projects should have a consistent file structure.
 * 4. All projects should have a consistent file naming convention.
 * 5. All projects should have a consistent file organization. 
 * 6. All projects should act to minimize redundant code. 
 * 7. All projects should be efficiently implemented. 
 * 8. All projects should be easily maintained. 
 * 9. All projects should be easily scalable. 
 * 10. All projects should be easily testable. 
 * 11. All projects should be easily deployable. 
 * 12. All projects should be easily monitored.
 * 13. All projects should teach you lessons and make you better.
 *
 */
```

## Overview

This document is meant to define the standardized directory structure and architectural patterns for all projects within the organization.  This document is not meant to be a complete directory structure for all projects, but rather a template for creating new projects.

## Root Directory Standard

**AXOVIA FLOW STANDARD OF PROCEDURE RULE: DO NOT CLUTTER THE ROOT REPOSITORY**

### Official Root Directory File List

**ONLY these 6 files are permitted in the root directory of any Axovia Flow project:**

1. **`.gitignore`** - Version control exclusions (mandatory)
2. **`.gitmodules`** - Git submodule definitions (if using submodules)
3. **`.npmignore`** - NPM package exclusions (mandatory)
4. **`Makefile`** - Development workflow targets (mandatory)
5. **`LICENSE`** - Project license (mandatory)
6. **`README.md`** - Project documentation (mandatory)

### Prohibited Root Directory Files

**NEVER place these in the root directory:**
- Package files: `package.json`, `package-lock.json`, `requirements.txt`
- Build artifacts: `dist/`, `build/`, compiled binaries
- IDE files: `.vscode/`, `.idea/`, `*.swp`
- OS files: `.DS_Store`, `Thumbs.db`
- Test files: Individual test files (use `test/` directory)
- Documentation files: Use `docs/` directory structure
- Configuration files: Use `config/` directory structure
- Source code: Use `src/` directory structure
- Scripts: Use `scripts/` or `bin/` directories

See [Root Directory Standard](root-directory.md) for complete implementation details.

## Core Directory Structure

```Cpp

'''
project-root/
├── docs/
│   ├── architecture/
│   │   ├── README.md
│   │   └── overview.md
│   ├── planning/    // --> All planning related documents that are solely for the purpose of planning.
│   │   ├── README.md
│   │   └── market-research.md
│   ├── requirements/ // --> Any other requirements related documents (SRD, TRD, etc.)
│   │   ├── README.md
│   │   └── prd.md     
│   ├── standards/
│   │   ├── README.md
│   │   ├── software-testing.md
│   │   └── initial-structure.md
│   ├── agile/
│   │   ├── README.md
│   │   ├── epics/
│   │   │   ├── README.md
│   │   └── stories/
│   │       └── README.md
│   ├── agent-reports/
│   │   ├── README.md
│   └── testing/
│       ├── README.md
│       └── test-plan.md
├── src/
├── bin/
'''

```
