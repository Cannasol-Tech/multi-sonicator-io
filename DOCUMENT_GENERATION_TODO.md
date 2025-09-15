# Documentation Generation TODO

This checklist defines how we will generate developer documentation for the project and integrate it into CI.

- Scope: Firmware (C/C++) + Web UI (TypeScript: backend and frontend)
- Decision: Use Doxygen for C/C++; use TypeDoc for TypeScript
- Outcome: Consistent, searchable docs published by CI; stored under `docs/` or published as artifacts/pages

## 1) Decisions and Targets

- C/C++ (firmware in `src/` and `include/`): Doxygen
- TypeScript (web-ui): TypeDoc (backend and frontend built separately)
- Optionally stitch outputs into a single site (MkDocs) later; initial goal is to generate separate outputs reliably

## 2) Directory Layout (proposed)

- Doxygen config: `docs/doxygen/Doxyfile`
- Doxygen HTML output: `docs/site/firmware/` (or CI artifact)
- TypeDoc backend output: `web-ui/backend/docs/` or `docs/site/web/backend/`
- TypeDoc frontend output: `web-ui/frontend/docs/` or `docs/site/web/frontend/`

Choose either “co-located” (inside each package) or centralized under `docs/site/`; centralized is better for a unified site.

## 3) Doxygen Setup (C/C++)

Tasks:
- Ensure `docs/doxygen/Doxyfile` exists with at least:
  - `PROJECT_NAME = "Multi-Sonicator IO Firmware"`
  - `INPUT = src include`
  - `RECURSIVE = YES`
  - `FILE_PATTERNS = *.c *.cpp *.h *.hpp`
  - `EXTRACT_ALL = YES`
  - `EXTRACT_PRIVATE = NO` (set YES if needed)
  - `GENERATE_HTML = YES`
  - `OUTPUT_DIRECTORY = docs/site/firmware`
- Note: If we ever want minimal TS scraping via Doxygen, map `.ts` as JavaScript, but TypeDoc is preferred for TS.

Commenting guidelines (C/C++):
- Use standard Doxygen tags: `@file`, `@brief`, `@details`, `@param`, `@return`, `@note`, `@warning`, `@see`, etc.
- Example is present in `src/modules/control/sonicator_modbus_bridge.cpp`.

## 4) TypeDoc Setup (TypeScript)

Do this for both `web-ui/backend/` and `web-ui/frontend/`.

Tasks (per package):
- Add dev dependencies:
  - `npm i -D typedoc typedoc-plugin-markdown` (plugin optional; for HTML, plain `typedoc` is enough)
- Create `typedoc.json` in the package root (backend and frontend):
  ```json
  {
    "$schema": "https://typedoc.org/schema.json",
    "entryPoints": ["src/index.ts"],
    "tsconfig": "tsconfig.json",
    "out": "docs",
    "excludePrivate": true,
    "excludeProtected": false,
    "excludeExternals": false,
    "readme": "none"
  }
  ```
  - Adjust `entryPoints` to actual main files (e.g., `src/` glob or multiple points)
  - Change `out` to `../docs/site/web/backend` or `../docs/site/web/frontend` if centralizing outputs
- Add npm scripts to each `package.json`:
  ```json
  {
    "scripts": {
      "docs": "typedoc"
    }
  }
  ```

Commenting guidelines (TypeScript):
- Use JSDoc/TypeDoc tags: `@param`, `@returns`, `@throws`, `@template`, `@deprecated`, `@example`, `@remarks`, `@see`, `@public/@private/@internal`, `@packageDocumentation`.
- Avoid Doxygen-only tags in TS; TypeDoc may ignore them.

## 5) Optional: Central Aggregation with MkDocs

If we want a unified site later:
- Add `mkdocs.yml` at repo root
- Layout suggestions:
  - `docs/site/firmware` (Doxygen HTML)
  - `docs/site/web/backend` (TypeDoc HTML)
  - `docs/site/web/frontend` (TypeDoc HTML)
- Use MkDocs `nav` to link to each section or host static outputs directly (MkDocs can copy static dirs).

## 6) CI Integration (GitHub Actions)

Add or extend a workflow (e.g., `.github/workflows/docs.yml`):
- Triggers: on `push` to `main`, and manual `workflow_dispatch`
- Jobs:
  1) Setup (checkout, Node, Python if needed, doxygen install)
  2) Build Doxygen docs
     - `doxygen docs/doxygen/Doxyfile`
     - Upload `docs/site/firmware` as artifact or publish to Pages
  3) Build TypeDoc (backend and frontend)
     - `cd web-ui/backend && npm ci && npm run docs`
     - `cd web-ui/frontend && npm ci && npm run docs`
     - Upload generated docs or publish consolidated `docs/site` to Pages

Option A: Publish to Pages via `actions/upload-pages-artifact` → `actions/deploy-pages`

Option B: Keep as build artifacts for download (simpler initial step)

## 7) Makefile Convenience (optional)

Add targets in `Makefile`:
- `docs-firmware`: run Doxygen
- `docs-web-backend`: run TypeDoc in `web-ui/backend`
- `docs-web-frontend`: run TypeDoc in `web-ui/frontend`
- `docs-all`: run all of the above

## 8) Acceptance Criteria

- Doxygen builds successfully from `src/` and `include/` and produces HTML
- TypeDoc builds successfully for both web-ui packages and produces HTML
- CI job runs on `main` and stores/publishes the outputs
- Documented commenting guidelines are followed in new code (Doxygen for C/C++, JSDoc/TypeDoc for TS)
- No hardcoded secrets, and builds are deterministic

## 9) Follow-ups (Nice to Have)

- Add link(s) to generated docs in `README.md`
- If needed, add typedoc JSON output and generate custom reports
- Consider a single unified docs portal (MkDocs), with a simple theme and navigation
