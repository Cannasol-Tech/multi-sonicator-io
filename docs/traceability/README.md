# BDD Scenario Traceability

This directory contains machine- and human-readable artifacts linking Behave BDD scenarios to product and agile sources.

What this adds on top of existing compliance checks:
- A lightweight rule: every scenario marked with `@TODO:<key>` must also include a `@trace:<key>` tag.
- A generated CSV matrix for fast auditing in CI.

How it works:
- Run `make traceability` to scan `test/acceptance/features/*.feature` files.
- The scanner writes `docs/traceability/bdd_traceability_matrix.csv` and fails if any `@TODO` lacks a matching `@trace`.

Tagging conventions:
- `@TODO:<key>` — placeholder scenarios that are not yet implemented
- `@trace:<key>` — stable linkage key for dashboards/matrices (e.g., `modbus-performance`, `hil-pin-matrix`)
- You may also include `@prd-<section>` or `@req-<short>` tags as needed for deeper PRD coverage (see docs/requirements/prd-to-scenarios-traceability.md)

Recommended authoring pattern:
- Put tags on the line(s) immediately before each `Scenario:`
- Safe placeholder scenarios should also include `@pending` so they are excluded from normal runs

CI integration:
- The main CI workflow invokes `make traceability` early to enforce the rule
- The existing traceability compliance workflows remain unchanged and complementary
