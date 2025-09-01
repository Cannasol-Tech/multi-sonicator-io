---
description: Standard for repositories to publish test and coverage results for the Functionality Reports Dashboard
---

# Cannasol Functionality Reports — Official Repository Release Standard of Procedure Docuementation

This document defines the required artifacts, formats, and process every repository must follow to publish acceptance results (and optional coverage and unit test summaries) consumable by the Functionality Reports Dashboard.  This document is meant to serve as the Overarching **SINGLE-SOURCE-OF-TRUTH** for all official company repositories.  This document dictates and mandates how the projects and repositories should structure their releases so that the functionality-reports user interface can accurately and effectively display the status of the current project.  The Definition of Done for a story should include that this Release Format Standard should be adhered to and a succesful pipeline release occurs with the implemented story.

## 1. Goals and Scope

- Ensure reliable, deterministic ingestion of test results across repositories.
- Provide a single source of truth per release for executive reporting.
- Support optional enrichment: code coverage and unit test summaries.

## Quick Checklist

Use this checklist in CI before publishing a release:

- [ ] Produce required JSON: `final/executive-report.json`
- [ ] Optionally produce: `final/coverage-summary.json`, `final/unit-test-summary.json`, `final/executive-report.md`
- [ ] Ensure JSON includes `version`, ISO timestamps, and required fields
- [ ] Validate JSON against schemas (see Schema References below)
- [ ] Create or update a GitHub Release for the commit/tag
- [ ] Upload artifacts as Release assets under `final/` names
- [ ] Ensure correct Content-Type for assets (JSON => `application/json`)

## 2. Artifacts Overview

- __Required__
  - `final/executive-report.json` — Acceptance (BDD/Behave) results; canonical machine-readable artifact.
- __Optional (recommended)__
  - `final/coverage-summary.json` — Code coverage totals (LCOV summarized) and optionally per-file.
  - `final/unit-test-summary.json` — Unit test aggregates and failures.
  - `final/executive-report.md` — Human-readable executive summary (for stakeholders). JSON remains the source of truth.

All artifacts are uploaded as GitHub Release assets for the commit/tag associated with the pipeline run.

## 3. File Naming and Location

- Place artifacts under a `final/` prefix when attaching to releases to keep naming consistent.
- Required: `final/executive-report.json`
- Optional: `final/coverage-summary.json`, `final/unit-test-summary.json`, `final/executive-report.md`
- Content-Type: `application/json` for JSON files; `text/markdown` for `.md`.

## 4. JSON Schemas (v1)

Version every artifact with a `version` field. Use ISO-8601 timestamps. Enums are lowercase.

### Schema References

The API validates artifacts using the following schema files:

- `functionality-reports/api/schemas/executive-report.schema.json`
- `functionality-reports/api/schemas/coverage-summary.schema.json`
- `functionality-reports/api/schemas/unit-test-summary.schema.json`

Keep schemas versioned and backward compatible per the guidance in Section 9.

### 4.1 executive-report.json (required)

Minimal shape:

```json
{
  "version": "1.0.0",
  "owner": "Cannasol-Tech",
  "repo": "example-repo",
  "releaseTag": "v1.2.3",
  "commit": "abc1234",
  "createdAt": "2025-08-14T22:30:00Z",
  "summary": { "total": 0, "passed": 0, "failed": 0, "skipped": 0, "durationMs": 0 },
  "scenarios": [
    {
      "feature": "Motor Control",
      "name": "Start motor when button pressed",
      "status": "passed",
      "durationMs": 4567,
      "steps": [
        { "keyword": "Given", "text": "the device is powered on", "status": "passed" },
        { "keyword": "When", "text": "the start button is pressed", "status": "passed" },
        { "keyword": "Then", "text": "the motor starts", "status": "passed" }
      ],
      "tags": ["smoke", "critical"],
      "evidenceUrl": "https://github.com/.../actions/runs/..."
    }
  ],
  "requirements": [
    { "id": "PRD-001", "status": "covered", "scenarios": ["Start motor when button pressed"] }
  ]
}
```

Field definitions:

- __version__: Schema version string (e.g., `1.0.0`).
- __owner__, __repo__: GitHub owner/name.
- __releaseTag__: Release tag (e.g., `v1.2.3`).
- __commit__: Short SHA or full SHA.
- __createdAt__: ISO timestamp of artifact generation.
- __summary__: Totals for acceptance scenarios and overall duration in ms.
- __scenarios__:
  - __feature__: Grouping or feature name.
  - __name__: Scenario title.
  - __status__: `passed|failed|skipped|unknown`.
  - __durationMs__: Execution time in ms.
  - __steps__: Ordered steps with `keyword` (`Given|When|Then|And|But`), `text`, and optional per-step `status`.
  - __tags__: Array of strings.
  - __evidenceUrl__: Optional link to logs/screenshots.
- __requirements__: Optional PRD mapping array.

### 4.2 coverage-summary.json (optional)

```json
{
  "version": "1.0.0",
  "owner": "Cannasol-Tech",
  "repo": "example-repo",
  "releaseTag": "v1.2.3",
  "commit": "abc1234",
  "createdAt": "2025-08-14T22:30:00Z",
  "totals": {
    "lines": { "pct": 84.2, "covered": 842, "total": 1000 },
    "statements": { "pct": 83.1, "covered": 831, "total": 1000 },
    "functions": { "pct": 80.0, "covered": 80, "total": 100 },
    "branches": { "pct": 75.5, "covered": 151, "total": 200 }
  },
  "files": [
    {
      "path": "src/motor/controller.ts",
      "lines": { "pct": 92.0, "covered": 92, "total": 100 }
    }
  ]
}
```

### 4.3 unit-test-summary.json (optional)

```json
{
  "version": "1.0.0",
  "owner": "Cannasol-Tech",
  "repo": "example-repo",
  "releaseTag": "v1.2.3",
  "commit": "abc1234",
  "createdAt": "2025-08-14T22:30:00Z",
  "summary": { "total": 120, "passed": 118, "failed": 2, "skipped": 0, "durationMs": 321000 },
  "suites": [
    { "name": "controllers", "total": 40, "passed": 40, "failed": 0, "skipped": 0 }
  ],
  "failures": [
    { "suite": "sensors", "test": "should debounce input", "message": "Expected true but got false", "evidenceUrl": "https://github.com/.../actions/runs/..." }
  ]
}
```

## 5. Release Process (CI Guidance)

- Generate artifacts during CI (e.g., GitHub Actions) after tests complete.
- Attach artifacts to a GitHub Release for the commit/tag.
- Suggested steps:
  1. Run acceptance (Behave), unit tests, and coverage.
  2. Produce JSON artifacts per the shapes above.
  3. Create or update a GitHub Release for the tag.
  4. Upload artifacts as release assets under `final/` names.

### 5.1 Example GitHub Actions Snippet (bash pseudo-steps)

```yaml
name: Publish Executive Reports
on:
  workflow_dispatch:
  push:
    tags:
      - 'v*'
    branches:
      - main
  pull_request:
    branches:
      - main
    types: [opened, synchronize, reopened]
permissions:
  contents: write
jobs:
  publish:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Setup Node
        uses: actions/setup-node@v4
        with:
          node-version: 20

      # Run your tests here (behave/unit/coverage)
      - name: Run tests
        run: |
          # ... project-specific commands
          echo "Run tests and produce artifacts"

      # Ensure final/ directory and write JSON files
      - name: Prepare artifacts
        run: |
          mkdir -p final
          # write executive-report.json / coverage-summary.json / unit-test-summary.json

      - name: Create Release (if needed)
        id: create_release
        uses: softprops/action-gh-release@v2
        with:
          tag_name: ${{ github.ref_name }}
          name: Release ${{ github.ref_name }}
          draft: false
          prerelease: false
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}

      - name: Upload Assets
        uses: softprops/action-gh-release@v2
        with:
          files: |
            final/executive-report.json
            final/coverage-summary.json
            final/unit-test-summary.json
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
```

Note:

- On pull requests, prefer uploading JSON files as workflow run artifacts instead of publishing a full release. Reserve release publication for tag pushes (versions) and optionally for merges to `main` that you want to snapshot.

## 6. Validation and Quality Gates

- Validate JSON artifacts against schemas (AJV or equivalent) during CI.
- Reject releases that fail validation, or mark them with a failure status.
- Keep schemas versioned; consumers (API) must accept compatible versions.

## 7. Consumer Expectations (This Project)

- API (`functionality-reports/api/`) will:
  - Prefer `final/executive-report.json` on the latest release; fallback to parsing Gherkin in release body when absent.
  - Optionally fetch and merge `coverage-summary.json` and `unit-test-summary.json` into a single normalized response.
  - Use robust error handling, rate-limit awareness, and consistent JSON error shapes.
- Frontend (`functionality-reports/`):
  - Renders acceptance scenarios as the primary signal.
  - Displays coverage and unit test summaries when present.

## 8. Backward Compatibility

- If a repository cannot produce JSON immediately, include readable Gherkin in the Release description. The system will parse it as a fallback.
- Transition to JSON artifacts should be prioritized for reliability and TDD.

## 9. Versioning and Evolution

- Start with `version: 1.0.0` for all artifacts.
- Only use additive changes for minor versions; breaking changes require a major bump and a deprecation window.
- Document changes in a changelog section within this file when updated.

---

Questions or requests for examples? Open an issue in the dashboard repo with your language/framework and CI runner, and we’ll provide tailored snippets.
