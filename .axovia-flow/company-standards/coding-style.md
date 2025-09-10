# Coding Style Standards

## Purpose

This document defines the coding style standards for all development work in Axovia Flow projects. These standards ensure consistency, readability, and maintainability across the codebase.

## General Principles

### Code Quality

- **Clean Code**: Write code that is self-documenting and easy to understand
- **SOLID Principles**: Follow Single Responsibility, Open/Closed, Liskov Substitution, Interface Segregation, and Dependency Inversion
- **DRY Principle**: Don't Repeat Yourself - create reusable components
- **KISS Principle**: Keep It Simple, Stupid - avoid unnecessary complexity

### Documentation

- **Docstrings**: All functions, classes, and modules must have comprehensive docstrings
- **Comments**: Use comments to explain WHY, not WHAT
- **README Files**: Every module/package should have a README explaining its purpose

## Project Structure Standards

### Root Directory Organization

**AXOVIA FLOW STANDARD OF PROCEDURE RULE: DO NOT CLUTTER THE ROOT REPOSITORY!**

All Axovia Flow projects must adhere to the strict root directory standard:

**ONLY these 6 files are permitted in the root directory: (MANDATORY)!**

> *NOTE: if you need to add additional files to the root directory, try to find another solution in `docs/` or `config/` or `scripts/`.  If you cannot find a solution then please raise an issue on GitHub or contact the Project Manager or Lead Engineer.*

1. **`.gitignore`** - Version control exclusions (mandatory)
2. **`.gitmodules`** - Git submodule definitions (if using submodules)
3. **`.npmignore`** - NPM package exclusions (mandatory)
4. **`Makefile`** - Development workflow targets (mandatory)
5. **`LICENSE`** - Project license (mandatory)
6. **`README.md`** - Project documentation (mandatory)

**Prohibited in root directory:**

- Package files (`package.json`, `package-lock.json`, `requirements.txt`) - use `config/` directory and `Makefile` to reference files for version control
- Build artifacts (`dist/`, `build/`, compiled binaries)
- IDE files (`.vscode/`, `.idea/`, `*.swp`)
- OS files (`.DS_Store`, `Thumbs.db`)
- Test files (use `test/` directory)
- Documentation files (use `docs/` directory)
- Configuration files (use `config/` directory)
- Source code (use `src/` directory)
- Scripts (use `scripts/` or `bin/` directories)

**📖 References:**

- [Complete Root Directory Standard](../standards/root-directory.md) - Detailed implementation guide
- [Project Structure Standards](project-structure.md) - Overall project organization
- [README Root Directory Section](../../README.md#root-directory-standard) - Quick reference

**🔧 Practical Guidelines:**

- Always check root directory compliance before committing
- Move prohibited files to appropriate subdirectories immediately
- Use `make validate-root` (when implemented) to check compliance
- Consult the standards documentation for any questions about file placement

## Language-Specific Standards

### Python

```python
# Function naming: snake_case
def process_user_data(user_id: str) -> Dict[str, Any]:
    """
    Process user data and return formatted result.
    
    Args:
        user_id: Unique identifier for the user
        
    Returns:
        Dictionary containing processed user data
        
    Raises:
        ValueError: If user_id is invalid
    """
    pass

# Class naming: PascalCase
class UserDataProcessor:
    """Handles user data processing operations."""
    
    def __init__(self, config: Config):
        self.config = config
```

### JavaScript/TypeScript

```javascript
// Function naming: camelCase
function processUserData(userId: string): UserData {
  /**
   * Process user data and return formatted result.
   * @param userId - Unique identifier for the user
   * @returns Processed user data object
   */
}

// Class naming: PascalCase
class UserDataProcessor {
  constructor(private config: Config) {}
}
```

## File Organization

### Directory Structure

- Use kebab-case for directory names: `user-management/`
- Group related files in logical directories
- Separate concerns: `models/`, `services/`, `controllers/`, `utils/`

### File Naming

- **Python**: snake_case.py (`user_service.py`)
- **JavaScript/TypeScript**: camelCase.js/ts (`userService.ts`)
- **Configuration**: kebab-case.yaml (`database-config.yaml`)
- **Documentation**: kebab-case.md (`coding-style.md`)

## Code Formatting

### Indentation

- **Python**: 4 spaces
- **JavaScript/TypeScript**: 2 spaces
- **YAML**: 2 spaces
- **Never use tabs**

### Line Length

- Maximum 100 characters per line
- Break long lines at logical points
- Use parentheses for line continuation in Python

### Imports

```python
# Python: Group imports
import os
import sys
from typing import Dict, List, Optional

from third_party_lib import something

from .local_module import LocalClass
```

```javascript
// JavaScript/TypeScript: Group imports
import fs from 'fs';
import path from 'path';

import { SomeClass } from 'third-party-lib';

import { LocalClass } from './localModule';
```

## Error Handling

### Exception Handling

- Use specific exception types, not generic `Exception`
- Always log errors with context
- Fail fast - validate inputs early
- Provide meaningful error messages

```python
# Good
try:
    result = process_data(user_input)
except ValidationError as e:
    logger.error(f"Invalid user input: {user_input}", exc_info=True)
    raise ValueError(f"User input validation failed: {e}")
```

## Testing Standards

### Test Organization

- Mirror source code structure in test directories
- Use descriptive test names: `test_should_process_valid_user_data`
- Group related tests in test classes
- One assertion per test when possible

### Test Naming

```python
def test_user_processor_should_return_formatted_data_when_given_valid_input():
    """Test that UserProcessor formats data correctly with valid input."""
    pass
```

## Version Control

### Commit Messages

```yaml
feat: add user data processing service

- Implement UserDataProcessor class
- Add validation for user input
- Include comprehensive error handling

Closes #123
```

### Branch Naming

- Feature branches: `feature/user-data-processing`
- Bug fixes: `fix/user-validation-error`
- Hotfixes: `hotfix/critical-security-patch`

## Performance Guidelines

### General Rules

- Profile before optimizing
- Use appropriate data structures
- Avoid premature optimization
- Cache expensive operations when appropriate

### Database Queries

- Use indexes appropriately
- Avoid N+1 query problems
- Use connection pooling
- Implement query timeouts

## Security Standards

### Input Validation

- Validate all user inputs
- Sanitize data before database operations
- Use parameterized queries
- Implement rate limiting

### Secrets Management

- Never commit secrets to version control
- Use environment variables for configuration
- Rotate secrets regularly
- Use secure secret management systems

## Code Review Guidelines

### Before Submitting

- [ ] Code follows style guidelines
- [ ] All tests pass
- [ ] Documentation is updated
- [ ] No debugging code left in
- [ ] Security considerations addressed

### Review Checklist

- [ ] Code is readable and maintainable
- [ ] Logic is correct and efficient
- [ ] Error handling is appropriate
- [ ] Tests cover edge cases
- [ ] Documentation is accurate

## Tools and Automation

### Linting

- **Python**: flake8, black, mypy
- **JavaScript/TypeScript**: ESLint, Prettier
- **YAML**: yamllint

### Pre-commit Hooks

- Run linters and formatters
- Execute tests
- Check for secrets
- Validate commit messages

## Build System Standards

**CRITICAL**: All projects must use Makefile targets for common operations. Never create standalone scripts in the root directory - use make targets that may reference scripts in `scripts/` directory.

## Makefile Requirements

- **ALWAYS** adhere to the Standard Operating Proecdure for creating and utilize Makefiles.
- **NEVER** create make targets that are not defined in the Makefiles SOP
- **IF** you need to add a new make target, raise an issue on Github in the Axovia Flow™ Project Repository (<https://github.com/Axovia-AI/axovia-flow/issues> )
  - *NOTE:* ALWAYS follow the procedure included int the Makefile SOP for proposing a new make target.
- **RETAIN** the Makefile SOP as the single source of truth for all makefile targets utilized across the company.
- **UNDERSTAND** that the purpose of this standard is to unify operations and increase efficiency.


## Continuous Improvement

This document is living and should be updated as the project evolves. Suggestions for improvements should be discussed with the team and documented through pull requests.
