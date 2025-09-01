# Test Levels Framework

Comprehensive guide for determining appropriate test levels (unit, integration, acceptance) for different scenarios.

**CRITICAL**: All Acceptance Tests must map to PRD requirements through @functionality tags. E2E tests are a specific type of Acceptance test for UI automation.

## Test Level Decision Matrix

### Unit Tests

**When to use:**

- Testing pure functions and business logic
- Algorithm correctness
- Input validation and data transformation
- Error handling in isolated components
- Complex calculations or state machines

**Characteristics:**

- Fast execution (immediate feedback)
- No external dependencies (DB, API, file system)
- Highly maintainable and stable
- Easy to debug failures

**Example scenarios:**

```yaml
unit_test:
  component: 'PriceCalculator'
  scenario: 'Calculate discount with multiple rules'
  justification: 'Complex business logic with multiple branches'
  mock_requirements: 'None - pure function'
```

### Integration Tests

**When to use:**

- Component interaction verification
- Database operations and transactions
- API endpoint contracts
- Service-to-service communication
- Middleware and interceptor behavior

**Characteristics:**

- Moderate execution time
- Tests component boundaries
- Focuses on contracts and interactions
- Tests edge cases extensively to ensure functionality robustness
- May use test databases or containers
- Validates system integration points
- Tests as much as possible without mocking or stubbing
- Meant to validate the real integration points of the system as well as the system as a whole

**Example scenarios:**

```yaml
integration_test:
  components: ['UserService', 'AuthRepository']
  scenario: 'Create user with role assignment'
  justification: 'Critical data flow between service and persistence'
  test_environment: 'In-memory database'
```

### End-to-End Tests

**When to use:**

- Critical user journeys
- Cross-system workflows
- Visual regression testing
- Compliance and regulatory requirements
- Final validation before release

**Characteristics:**

- Slower execution
- Tests complete workflows
- Requires full environment setup
- Most realistic but most brittle

**Example scenarios:**

```yaml
e2e_test:
  journey: 'Complete checkout process'
  scenario: 'User purchases with saved payment method'
  justification: 'Revenue-critical path requiring full validation'
  environment: 'Staging with test payment gateway'
```

## Test Level Selection Rules

### Favor Unit Tests When

- Logic can be isolated
- No side effects involved
- Fast feedback needed
- High cyclomatic complexity

### Favor Integration Tests When

- Testing persistence layer
- Validating service contracts
- Testing middleware/interceptors
- Component boundaries critical

### Favor Acceptance Tests When

- Validating PRD requirements and business rules
- Testing complete user workflows end-to-end
- Verifying system behavior matches business expectations
- Mission-critical functionality that must work

**CRITICAL**: All Acceptance Tests MUST use BDD/Gherkin scenarios that map (many-to-one) to PRD requirements via @functionality tags.

#### Implementation Methods

**BDD/Gherkin + Playwright (E2E UI Automation):**

- User interface workflows and interactions
- Cross-browser compatibility testing
- Visual regression and UI behavior
- Full user journey validation

**BDD/Gherkin + API Testing:**

- Backend business logic validation
- Service contract verification
- Data processing workflows

**BDD/Gherkin + Integration Testing:**

- Multi-system interactions
- Database operations
- External service integrations

## Anti-patterns to Avoid

- Acceptance tests without BDD/Gherkin scenarios
- Acceptance tests that don't map to PRD requirements
- Unit testing framework behavior
- Integration testing third-party libraries
- Duplicate coverage across levels

## Duplicate Coverage Guard

**Before adding any test, check:**

1. Is this already tested at a lower level?
2. Can a unit test cover this instead of integration?
3. Can an integration test cover this instead of E2E?

**Coverage overlap is only acceptable when:**

- Testing different aspects (unit: logic, integration: interaction, e2e: user experience)
- Critical paths requiring defense in depth
- Regression prevention for previously broken functionality

## Test Naming Conventions

- Unit: `test_{component}_{scenario}`
- Integration: `test_{flow}_{interaction}`
- Acceptance: `test_{requirement}_{scenario}`
- E2E: `test_{journey}_{outcome}`

## Test ID Format

`{EPIC}.{STORY}-{LEVEL}-{SEQ}`

Examples:

- `1.3-UNIT-001`
- `1.3-INT-002`
- `1.3-E2E-001`
