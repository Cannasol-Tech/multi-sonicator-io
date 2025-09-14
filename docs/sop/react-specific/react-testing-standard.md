# React UI Application Testing Standard

> **Note:** This document defines the official testing standard for all React UI applications within the organization.

## Overview

React applications require comprehensive testing strategies that cover component behavior, user interactions, and integration with backend services. This standard defines the **official testing requirements** for all React projects, emphasizing modern testing practices with React Testing Library and Jest.

---

## Framework Requirements

### Primary Testing Framework

- **Foundation**: Jest + React Testing Library for ALL React testing
- **Philosophy**: Test behavior, not implementation details
- **Coverage**: Component tests, integration tests, and end-to-end testing

### Mocking Framework (Unit Tests ONLY)

- **Standard**: Jest's built-in mocking capabilities
- **Scope**: **MOCKING IS ONLY TO BE USED FOR UNIT TESTING**
- **Prohibition**: Component tests and integration tests should minimize mocking
- **Rationale**: Test real component interactions and user behavior

### Specialized Frameworks

- **Component Testing**: React Testing Library for user-centric testing
- **End-to-End Testing**: Cypress or Playwright for full application flows
- **Visual Testing**: Storybook + Chromatic for component visual regression
- **Accessibility Testing**: jest-axe for automated accessibility validation

---

## Testing Architecture

### Test Organization Structure

```markdown
src/
├── components/
│   ├── Button/
│   │   ├── Button.tsx
│   │   ├── Button.test.tsx        # Component tests
│   │   └── Button.stories.tsx     # Storybook stories
│   └── ...
├── pages/
│   ├── LoginPage/
│   │   ├── LoginPage.tsx
│   │   ├── LoginPage.test.tsx     # Page component tests
│   │   └── LoginPage.integration.test.tsx  # Integration tests
│   └── ...
├── services/
│   ├── api/
│   │   ├── authService.ts
│   │   └── authService.test.ts    # Service unit tests
│   └── ...
├── utils/
│   ├── helpers.ts
│   └── helpers.test.ts            # Utility unit tests
└── __tests__/
    ├── setup.ts                   # Test configuration
    ├── mocks/                     # Mock definitions (unit tests only)
    └── fixtures/                  # Test data and fixtures
```

### Testing Pyramid Distribution

- **Unit Tests**: 70% - Services, utilities, pure functions
- **Component Tests**: 20% - Component behavior and user interactions
- **Integration/E2E Tests**: 10% - Critical user flows and page interactions

---

## Framework-Specific Standards

### 1. Unit Testing

- **Framework**: Jest (mocking permitted)
- **Coverage Requirement**: ≥90% statement coverage
- **Scope**: Services, utilities, pure functions, business logic
- **Mocking**: Permitted for external dependencies and APIs
- **Location**: Co-located with source files (`*.test.ts`)

```typescript
// Example: Service unit test with mocking
import { authService } from './authService';
import { apiClient } from '../api/client';

jest.mock('../api/client');
const mockApiClient = apiClient as jest.Mocked<typeof apiClient>;

describe('AuthService', () => {
  beforeEach(() => {
    jest.clearAllMocks();
  });

  test('should authenticate user successfully', async () => {
    // Arrange
    const mockUser = { id: '1', email: 'test@example.com' };
    mockApiClient.post.mockResolvedValue({ data: mockUser });

    // Act
    const result = await authService.login('test@example.com', 'password');

    // Assert
    expect(result).toEqual(mockUser);
    expect(mockApiClient.post).toHaveBeenCalledWith('/auth/login', {
      email: 'test@example.com',
      password: 'password'
    });
  });
});
```

### 2. Component Testing

- **Framework**: React Testing Library + Jest (minimal mocking)
- **Coverage Requirement**: ≥80% component coverage
- **Scope**: Component behavior, user interactions, rendering
- **Mocking**: **MINIMIZE** - Only mock external services, not React components
- **Location**: Co-located with components (`*.test.tsx`)

```typescript
// Example: Component test with minimal mocking
import { render, screen, fireEvent, waitFor } from '@testing-library/react';
import { LoginForm } from './LoginForm';
import { authService } from '../../services/authService';

jest.mock('../../services/authService');
const mockAuthService = authService as jest.Mocked<typeof authService>;

describe('LoginForm', () => {
  test('should submit form with user credentials', async () => {
    // Arrange
    const mockOnSuccess = jest.fn();
    mockAuthService.login.mockResolvedValue({ id: '1', email: 'test@example.com' });

    render(<LoginForm onSuccess={mockOnSuccess} />);

    // Act
    fireEvent.change(screen.getByLabelText(/email/i), {
      target: { value: 'test@example.com' }
    });
    fireEvent.change(screen.getByLabelText(/password/i), {
      target: { value: 'password123' }
    });
    fireEvent.click(screen.getByRole('button', { name: /sign in/i }));

    // Assert
    await waitFor(() => {
      expect(mockAuthService.login).toHaveBeenCalledWith('test@example.com', 'password123');
      expect(mockOnSuccess).toHaveBeenCalled();
    });
  });
});
```

### 3. Integration Testing

- **Framework**: React Testing Library + Jest (NO component mocking)
- **Scope**: Page-level interactions, routing, state management
- **Mocking**: **PROHIBITED** for React components - Only mock external APIs
- **Location**: `*.integration.test.tsx` files

```typescript
// Example: Integration test
import { render, screen, fireEvent, waitFor } from '@testing-library/react';
import { BrowserRouter } from 'react-router-dom';
import { QueryClient, QueryClientProvider } from 'react-query';
import { LoginPage } from './LoginPage';
import { server } from '../../__tests__/mocks/server'; // MSW server

describe('LoginPage Integration', () => {
  test('should complete login flow and redirect to dashboard', async () => {
    // Arrange
    const queryClient = new QueryClient();
    render(
      <QueryClientProvider client={queryClient}>
        <BrowserRouter>
          <LoginPage />
        </BrowserRouter>
      </QueryClientProvider>
    );

    // Act - Complete login flow
    fireEvent.change(screen.getByLabelText(/email/i), {
      target: { value: 'test@example.com' }
    });
    fireEvent.change(screen.getByLabelText(/password/i), {
      target: { value: 'password123' }
    });
    fireEvent.click(screen.getByRole('button', { name: /sign in/i }));

    // Assert - Check for successful navigation
    await waitFor(() => {
      expect(window.location.pathname).toBe('/dashboard');
    });
  });
});
```

### 4. End-to-End Testing

- **Framework**: Cypress or Playwright
- **Scope**: Complete user workflows, cross-browser testing
- **Mocking**: **PROHIBITED** - Test against real or staging environments
- **Location**: `cypress/e2e/` or `e2e/` directory

```typescript
// Example: Cypress E2E test
describe('User Authentication Flow', () => {
  it('should allow user to sign in and access dashboard', () => {
    cy.visit('/login');
    
    cy.get('[data-testid="email-input"]').type('test@example.com');
    cy.get('[data-testid="password-input"]').type('password123');
    cy.get('[data-testid="login-button"]').click();
    
    cy.url().should('include', '/dashboard');
    cy.get('[data-testid="user-menu"]').should('contain', 'test@example.com');
  });
});
```

---

## Coverage Requirements

> **IMPORTANT:** When working in a React Repository, these Coverage Requirements Override the Global Coverage Requirements. 

| Test Type | Minimum Coverage | Enforcement |
|-----------|------------------|-------------|
| Unit Tests | 90% | CI/CD Pipeline |
| Component Tests | 80% | CI/CD Pipeline |
| Integration Tests | Critical Paths | Manual Review |
| Overall Project | 85% | CI/CD Pipeline |

---

## Make Targets for React

React projects must implement these standardized make targets:

```bash
# Unit Testing (with mocking allowed)
make test-unit
# - Executes: npm test -- --testPathPattern=test
# - Coverage: ≥90% requirement
# - Mocking: Jest mocking permitted

# Component Testing (minimal mocking)
make test-component
# - Executes: npm test -- --testPathPattern=components
# - Coverage: ≥80% requirement
# - Mocking: Minimize to external services only

# Integration Testing (no component mocking)
make test-integration
# - Executes: npm test -- --testPathPattern=integration
# - Scope: Page-level interactions
# - Mocking: External APIs only

# End-to-End Testing
make test-e2e
# - Executes: npx cypress run or npx playwright test
# - Scope: Complete user workflows
# - Mocking: PROHIBITED

# Complete React Test Suite
make test
# - Executes all React test types in sequence
# - Equivalent to: make test-unit && make test-component && make test-integration
```

---

## Prohibited Practices

### Strictly Forbidden

- ❌ Testing implementation details (internal state, private methods)
- ❌ Shallow rendering (use React Testing Library's render)
- ❌ Mocking React components in integration tests
- ❌ Testing CSS styles directly (use visual regression tests)
- ❌ Hardcoded test data (use fixtures and factories)

### Required Practices

- ✅ Test user behavior and interactions
- ✅ Use semantic queries (getByRole, getByLabelText)
- ✅ Mock external services and APIs appropriately
- ✅ Include accessibility testing with jest-axe
- ✅ Use data-testid sparingly, prefer semantic queries

---

## Accessibility Testing

### Required Accessibility Tests

```typescript
import { axe, toHaveNoViolations } from 'jest-axe';

expect.extend(toHaveNoViolations);

test('should not have accessibility violations', async () => {
  const { container } = render(<LoginForm />);
  const results = await axe(container);
  expect(results).toHaveNoViolations();
});
```

### Accessibility Standards

- **WCAG 2.1 AA compliance** required for all components
- **Automated testing** with jest-axe in component tests
- **Manual testing** with screen readers for critical flows
- **Keyboard navigation** testing for all interactive elements

---

## Quality Gates for React

All React stories must pass these quality gates:

- [ ] Unit tests achieve ≥90% coverage (mocking permitted for external services)
- [ ] Component tests achieve ≥80% coverage (minimal mocking)
- [ ] Integration tests cover critical page interactions (no component mocking)
- [ ] Accessibility tests pass with no violations
- [ ] Visual regression tests validate component appearance
- [ ] End-to-end tests cover primary user workflows

---

✅ **This standard is authoritative for all React applications. Compliance is mandatory and enforced through CI/CD pipelines.**
