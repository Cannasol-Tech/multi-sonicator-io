# Flutter Application Testing Standard

> **Note:** This document defines the official testing standard for all Flutter applications within the organization.

## Overview

Flutter applications require specialized testing approaches due to their cross-platform nature and unique architecture. This standard defines the **official testing requirements** for all Flutter projects, emphasizing the use of Flutter's built-in testing framework with strict mocking limitations.  Tests should be written in a modular format to where indivudual modules can be tested in isolation.  All tests should be written in a way that allows for easy debugging and maintenance.  Each subset of tests (unit, widget, integration, and golden) should all be able to be ran independently using their respective make targets, i.e. `make test-widget`.  The entirety of the test suite should be able to be ran using the `make test` target.`

---

## Framework Requirements

### Primary Testing Framework

- **Foundation**: Flutter's built-in testing framework (`flutter_test`) for ALL Flutter testing
- **Coverage**: Widget tests, unit tests, golden tests (UI consistency), and integration testing foundation

### Mocking Framework (Unit Tests ONLY)

- **Standard**: Mocktail ^1.0.4 (OFFICIAL - no exceptions)
- **Scope**: **MOCKING IS ONLY TO BE USED FOR UNIT TESTING**
- **Prohibition**: Widget tests, integration tests, and golden tests must NOT use mocking
- **Rationale**: Mocking in non-unit tests defeats the purpose of testing real component interactions

### Specialized Frameworks

- **BLoC Testing**: `bloc_test` framework permitted when using BLoC pattern for business logic
  - Note: If this does not integrate with the flutter project we can use a different framework / pattern for business logic.
- **Integration Testing**: Flutter's `integration_test` framework for end-to-end testing
  - Note: This testing should use NO mocking whatsoever and provide extensive edge case testing for the application
- **Golden Testing**: Flutter's built-in golden test support for visual regression testing
  - Note: This testing should use NO mocking whatsoever and provide testing for the visual aspects of the Application.  
    - These tests should ensure things like:
      - UI Consistency
      - Visual Regression
      - Responsive Design
      - Dark Mode
      - Light Mode
      - Test any Localized content
      - Any platform specific UI interactions
      - Any style guide / user experience design requirements
      - Any accessibility requirements
      - Any thing else the developer would like to cover using the Golden Testing (NOTE: This does not take away from coverage requirements for the Unit, Widget, or Integration tests)

---

## Testing Architecture

### Test Organization Structure

```markdown
test/
├── unit/                   # Business logic tests (70% of test effort)
│   ├── models/               # Data model unit tests
│   ├── services/             # Service layer unit tests  
│   ├── handlers/             # Business logic handler tests
│   └── utils/                # Utility function tests
├── widget/                 # UI component tests (20% of test effort)
│   ├── pages/                # Page widget tests
│   ├── components/           # Reusable component tests
│   └── dialogs/              # Dialog widget tests
├── integration/            # End-to-end tests (10% of test effort)
│   ├── flows/                # Complete user journey tests
│   └── platform/             # Platform-specific integration tests
├── golden/                 # Visual regression tests
│   └── screenshots/          # Golden file storage
└── helpers/                # Test utilities (centralized)
    ├── mocks.dart            # Mock definitions (unit tests only)
    ├── test_data.dart        # Test data constants
    └── test_utils.dart       # Common test utilities
```

### Testing Pyramid Distribution

- **Unit Tests**: 70% - Business logic, models, services, utilities
- **Widget Tests**: 20% - UI components, user interactions, widget behavior
- **Integration Tests**: 10% - Critical user flows, platform-specific features

---

## Framework-Specific Standards

### 1. Unit Testing

- **Framework**: `flutter_test` + `mocktail` (mocking permitted)
- **Coverage Requirement**: ≥85% statement coverage
- **Scope**: Business logic, data models, services, utilities
- **Mocking**: Permitted and encouraged for external dependencies
- **Location**: `test/unit/`

```dart
// Example: Unit test with mocking
import 'package:flutter_test/flutter_test.dart';
import 'package:mocktail/mocktail.dart';

class MockFirebaseAuth extends Mock implements FirebaseAuth {}

void main() {
  group('UserService Tests', () {
    late MockFirebaseAuth mockAuth;
    
    setUp(() {
      mockAuth = MockFirebaseAuth();
    });
    
    test('should authenticate user successfully', () async {
      // Arrange
      when(() => mockAuth.signInWithEmailAndPassword(
        email: any(named: 'email'),
        password: any(named: 'password'),
      )).thenAnswer((_) async => mockUserCredential);
      
      // Act & Assert
      // Test business logic with mocked dependencies
    });
  });
}
```

### 2. Widget Testing

- **Framework**: `flutter_test` ONLY (NO mocking)
- **Coverage Requirement**: ≥70% widget coverage
- **Scope**: UI components, user interactions, widget rendering
- **Mocking**: **PROHIBITED** - Test real widget behavior
- **Location**: `test/widget/`

```dart
// Example: Widget test without mocking
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

void main() {
  testWidgets('Login button triggers callback', (WidgetTester tester) async {
    bool callbackTriggered = false;
    
    await tester.pumpWidget(
      MaterialApp(
        home: LoginButton(
          onPressed: () => callbackTriggered = true,
        ),
      ),
    );
    
    await tester.tap(find.byType(ElevatedButton));
    expect(callbackTriggered, isTrue);
  });
}
```

### 3. Golden Testing (Visual Regression)

- **Framework**: Flutter's built-in golden test support
- **Scope**: Critical UI components, visual consistency across platforms
- **Mocking**: **PROHIBITED** - Test actual visual output
- **Location**: `test/golden/`

```dart
// Example: Golden test
testWidgets('Login page golden test', (WidgetTester tester) async {
  await tester.pumpWidget(MaterialApp(home: LoginPage()));
  await expectLater(
    find.byType(LoginPage),
    matchesGoldenFile('golden/login_page.png'),
  );
});
```

### 4. Integration Testing

- **Framework**: Flutter's `integration_test` framework
- **Scope**: End-to-end user flows, platform-specific features
- **Mocking**: **PROHIBITED** - Test complete system integration
- **Location**: `integration_test/`

```dart
// Example: Integration test
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();
  
  testWidgets('Complete sign-in flow', (WidgetTester tester) async {
    // Test complete user journey without mocks
    await tester.pumpWidget(MyApp());
    
    // Navigate through actual app flow
    await tester.tap(find.text('Sign In'));
    await tester.enterText(find.byKey(Key('email')), 'test@example.com');
    // ... complete flow testing
  });
}
```

---

## Coverage Requirements

| Test Type | Minimum Coverage | Enforcement |
|-----------|------------------|-------------|
| Unit Tests | 85% | CI/CD Pipeline |
| Widget Tests | 70% | CI/CD Pipeline |
| Integration Tests | Critical Paths | Manual Review |
| Overall Project | 80% | CI/CD Pipeline |

---

## Make Targets for Flutter

Flutter projects must implement these standardized make targets:

```bash
# Unit Testing (with mocking allowed)
make test-unit
# - Executes: flutter test test/unit/
# - Coverage: ≥85% requirement
# - Mocking: Mocktail permitted

# Widget Testing (no mocking)
make test-widget  
# - Executes: flutter test test/widget/
# - Coverage: ≥70% requirement
# - Mocking: PROHIBITED

# Golden Testing (visual regression)
make test-golden
# - Executes: flutter test test/golden/
# - Updates: flutter test --update-goldens (when needed)
# - Mocking: PROHIBITED

# Integration Testing (end-to-end)
make test-integration
# - Executes: flutter test integration_test/
# - Scope: Complete user flows
# - Mocking: PROHIBITED

# Complete Flutter Test Suite
make test
# - Executes all Flutter test types in sequence
# - Equivalent to: make test-unit && make test-widget && make test-integration
```

---

## Prohibited Practices

### Strictly Forbidden

- ❌ Using Mockito (use Mocktail exclusively)
- ❌ Mocking in widget tests (defeats purpose of UI testing)
- ❌ Mocking in integration tests (defeats purpose of E2E testing)
- ❌ Mocking in golden tests (defeats purpose of visual testing)
- ❌ Creating mocks outside centralized helpers
- ❌ Hardcoded test data (use centralized test_data.dart)

### Required Practices

- ✅ Use Flutter's built-in testing framework as foundation
- ✅ Use Mocktail exclusively for unit test mocking
- ✅ Centralize all mocks in `test/helpers/mocks.dart`
- ✅ Follow testing pyramid distribution (70/20/10)
- ✅ Maintain coverage requirements with CI/CD enforcement

---

## Cross-Platform Considerations

### Platform-Specific Testing

- **iOS**: Test iOS-specific integrations (Sign in with Apple, etc.)
- **Android**: Test Android-specific features (back button behavior, etc.)
- **Web**: Test web-specific interactions and responsive design
- **Desktop**: Test desktop-specific UI patterns and interactions

### Test Execution Strategy

- **Unit Tests**: Run on all platforms (business logic is platform-agnostic)
- **Widget Tests**: Run on primary platform with cross-platform validation
- **Integration Tests**: Run on each target platform separately
- **Golden Tests**: Generate and validate on each platform

---

## Quality Gates for Flutter

All Flutter stories must pass these quality gates:

- [ ] Unit tests achieve ≥85% coverage (mocking permitted)
- [ ] Widget tests achieve ≥70% coverage (NO mocking)
- [ ] Integration tests cover critical user flows (NO mocking)
- [ ] Golden tests validate UI consistency (NO mocking)
- [ ] All tests use Flutter's built-in testing framework
- [ ] Mocktail used exclusively for unit test mocking
- [ ] Test organization follows prescribed structure

---

✅ **This standard is authoritative for all Flutter applications. Compliance is mandatory and enforced through CI/CD pipelines.**
