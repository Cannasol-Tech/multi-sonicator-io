# BDD Traceability Matrix (Lightweight)

This matrix is generated as CSV by `make traceability` to support rapid auditing. See `bdd_traceability_matrix.csv` in this folder.

Sources referenced:
- Agile plan: `docs/agile/**`
- PRD: `docs/prd.md` and `docs/prd/**`
- Features: `test/acceptance/features/*.feature`

Rule enforced:
- Every `@TODO:<key>` must have a corresponding `@trace:<key>` on the same scenario line(s).

For full PRD-to-Scenario coverage, continue to use `docs/prd/prd-to-scenarios-traceability.md` and existing compliance scripts.
