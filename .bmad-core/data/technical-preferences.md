# User-Defined Preferred Patterns and Preferences

## Testing Standards

**Primary Authority:** `sw-testing-standard.md` - 3-Stage Testing Approach

### Core Requirements

- **Unit Testing:** Minimum 95% coverage for all business logic
- **Acceptance Testing:** ALWAYS BDD/Gherkin scenarios mapping (many-to-one) to PRD requirements via @functionality tags
- **Integration Testing:** HIL for hardware projects, component interaction testing for software projects
- **Implementation Methods:** BDD/Gherkin scenarios implemented via Playwright (UI), API testing, or integration testing

### Implementation Guidance

- Use `test-levels-framework.md` for detailed test level decision criteria
- Apply `test-priorities-matrix.md` for P0/P1/P2/P3 risk-based prioritization
- Leverage `test-design` task for automated test scenario generation

### Quality Gates

- All stories must pass QA gate validation before completion
- Standardized reporting required: `functionality-report.json` + `executive-report.json`
- Traceability matrix required mapping tests to PRD requirements

## Architecture Preferences

- Favor async/await patterns for scalable systems
- Use established design patterns (Orchestrator, Registry, Blackboard)
- Implement comprehensive logging and monitoring from the start

## Development Practices

- Follow BMad-Core agent workflows for consistent quality
- Use structured logging with correlation IDs for debugging
- Implement proper error handling and graceful degradation

## Build System Standards

**CRITICAL**: All projects must use Makefile targets for common operations. Never create standalone scripts - use make targets that may reference scripts in `scripts/` directory.

### Required Make Targets

**Development Workflow:**

- `make install` - Install all dependencies and setup development environment
- `make build` - Build the project for development
- `make clean` - Clean build artifacts and temporary files
- `make dev` - Start development server with hot reload

**Testing Workflow:**

- `make test` - Run all tests (unit, integration, acceptance)
- `make test-unit` - Run unit tests only
- `make test-integration` - Run integration tests only
- `make test-acceptance` - Run acceptance tests (BDD/Gherkin scenarios)
- `make test-watch` - Run tests in watch mode
- `make coverage` - Generate test coverage reports

**Quality Assurance:**

- `make lint` - Run code linting and formatting checks
- `make format` - Auto-format code according to standards
- `make validate` - Run all quality checks (lint, test, coverage)

**Production Workflow:**

- `make build-prod` - Build for production deployment
- `make deploy` - Deploy to production environment
- `make release` - Create and tag a new release

### Implementation Guidelines

- Make targets should be simple and descriptive
- Complex logic should be in `scripts/` directory, called by make targets
- All targets should have help text accessible via `make help`
- Use `.PHONY` declarations for non-file targets
- Include error handling and status reporting

## Definition of Done Extension

- Adhere to all of the requirements of our official definition of done.
- A story is not complete unless it is successfully released to production and the pipeline is green.
- The functionality report must be accurately prepared according to the release format standard in `docs/standards/release-format.md`
