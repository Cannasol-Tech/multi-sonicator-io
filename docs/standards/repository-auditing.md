# Repository Auditing — Standard Operating Procedure (SOP)

Last updated: 2025-08-28
Owner: Engineering Productivity / DevSecOps
Applies to: All repositories under the Axovia-AI organization (and any internal forks)

## Purpose

Ensure each repository remains compliant, secure, maintainable, and operationally healthy by running a periodic audit against a consistent, documented checklist with clear success criteria and follow-up actions.

## Scope

- Code repositories (public and private)
- GitHub settings and policies
- CI/CD workflows and environments
- Documentation, testing, security, compliance, and release processes

## Roles & Responsibilities

- Audit Lead: Coordinates the audit, collects evidence, assembles report
- Repo Maintainers: Provide context, approve changes, remediate findings
- DevSecOps: Advises on security configuration and risk acceptance
- Eng Management: Reviews audit results, prioritizes remediation

## Frequency

- Standard cadence: Quarterly for active repos; Semi-annual for archived/low-activity repos
- Triggered audits: Post-incident, before major releases, or after policy updates

## Success Criteria

- Audit Score ≥ 85/100 OR all Critical/High findings remediated or accepted with time-bound plan
- No open Critical findings at audit close
- Evidence and report filed in repository docs or org-wide audit space

## Preparation

- Ensure you have Maintainer or Admin permissions for settings read (or a designated readout)
- Clone repo and fetch default branch
- Collect previous audit report (if any) and open issues/PRs labeled "audit" or "compliance"

## Audit Checklist

Mark each item:

- [ ] Pass, [ ] Fail, [ ] N/A, notes in report

### A. Repository Metadata & Hygiene

- [ ] README present, current, with quickstart, architecture overview, and status badges
- [ ] LICENSE present and correct
- [ ] CODEOWNERS enforces ownership for critical paths
- [ ] CONTRIBUTING and PULL REQUEST TEMPLATE exist and match our standards
- [ ] SECURITY.md with disclosure process and supported versions
- [ ] CODE_OF_CONDUCT present (if open-source)
- [ ] FUNDING/Support links (if applicable)
- [ ] .gitignore covers language/tool artifacts; .gitattributes configured if needed
- [ ] Git LFS used for large binaries; no large files committed accidentally
- [ ] Environment example present (e.g., .env.example) with safe defaults

### B. Access & Governance

- [ ] Team permissions follow least-privilege; bots scoped minimally
- [ ] Branch protection rules on default branch (require PRs, approvals, status checks)
- [ ] Dismiss stale reviews on new commits; require linear history or squash/rebase
- [ ] Require signed commits (or DCO) enforced
- [ ] Secret scanning enabled; Dependabot alerts enabled
- [ ] Security advisories configuration (for public repos)

### C. CI/CD & Workflow Security

- [ ] All required status checks configured and enforced
- [ ] Workflow permissions least-privilege (GITHUB_TOKEN permissions set explicitly)
- [ ] External actions pinned by commit SHA or trusted version
- [ ] Environments use required reviewers for prod deploys
- [ ] Secrets are referenced via GitHub Encrypted Secrets (no plaintext in repo)
- [ ] CI caches and artifacts policies are safe (no secrets)

### D. Dependencies & Supply Chain

- [ ] Lockfiles committed (package-lock.json, poetry.lock, Cargo.lock, etc.)
- [ ] Automated updates configured (Dependabot/Renovate) with sensible batch/schedule
- [ ] Vulnerability scanning configured (GitHub, OSV-Scanner, Trivy, etc.)
- [ ] No unsupported/EOL dependencies; pinned versions for critical libraries
- [ ] License compliance reviewed; incompatible licenses flagged

### E. Code Quality & Testing

- [ ] Linters and formatters configured and enforced in CI
- [ ] Test suite exists with meaningful coverage; coverage threshold documented
- [ ] Flaky tests tracked; quarantine or retry strategy defined
- [ ] Static analysis (type checks, code scanning) enabled and passing
- [ ] Architecture docs exist and are up to date

### F. Releases & Change Management

- [ ] Semantic Versioning followed; tags match release versions
- [ ] CHANGELOG maintained with human-readable entries
- [ ] Release artifacts reproducible; SBOM generated (where applicable)
- [ ] Release notes templated and automated where possible
- [ ] Backport policy defined for supported branches

### G. Operations & Observability

- [ ] Runtime configs documented; feature flags cataloged
- [ ] Logging/metrics/tracing approach documented (if applicable)
- [ ] On-call/runbook links for services
- [ ] Backup/restore or disaster recovery notes where relevant

### H. Repository Structure

- [ ] Follows documented project structure standard
- [ ] Centralized modules/services; avoids duplication and multiple sources of truth
- [ ] Clear separation of concerns (app, infra, docs, tests)

## Procedure

1) Plan
   - Confirm scope, stakeholders, and timeline
   - Create an Audit tracking issue in the repo with this checklist
2) Automated Checks
   - Run vulnerability and license scans; run linters/tests; export CI status
   - Collect GitHub settings via API (branch protections, secrets, required checks)
3) Manual Review
   - Validate docs, governance, workflow pinning, and any context-specific risks
4) Score & Classify Findings
   - Assign severity and remediation owner
5) Report & Sign-off
   - Share draft with maintainers, collect feedback, finalize
6) Remediation
   - Open issues/PRs for each finding with clear acceptance criteria
7) Verification
   - Re-run targeted checks; update status to Closed when complete

## Scoring (Guideline)

- Critical: 10 points each
- High: 5 points each
- Medium: 2 points each
- Low: 1 point each
- Start from 100; subtract per unresolved finding at close. N/A items do not affect score.

## Reporting

- Location: docs/audits/<yyyy-mm>/repository-audit.md (or org-level audit space)
- Must include: scope, date, participants, evidence links, checklist outcomes, score, findings table, remediation plan, due dates

## Smaller-Scale Audits (Targeted)

In addition to the full repository audit, run focused audits as lightweight, higher-cadence checks. Store results under `docs/qa/repo-audits/`.

### 1) Consistency Audit

- Purpose: Ensure single source of truth, coherent structure, and standards alignment for humans and agents
- Cadence: After each story; at least daily during active development
- Scope: Paths/naming, packaging vs. promises (README/Makefile/package.json), standards alignment, prohibited artifacts
- Outputs: `docs/qa/repo-audits/consistency/repo-consistency-audit-<YYYY-MM-DD>.md`

### 2) Security Audit

- Purpose: Identify and remediate security risks across code, CI/CD, dependencies, and settings
- Cadence: Monthly baseline; before releases; after security-impacting changes
- Scope: Secrets exposure, dependency vulns, workflow security (pinned SHAs, permissions), access controls, container/image scanning
- Outputs: `docs/qa/repo-audits/security/repo-security-audit-<YYYY-MM-DD>.md`

### 3) Performance Audit

- Purpose: Detect performance risks in code paths, build pipeline, and runtime configurations
- Cadence: Before/after major features; monthly baseline
- Scope: Algorithmic hotspots, N+1/IO, build times and cache efficacy, bundle/image sizes, load/perf test coverage and SLAs
- Outputs: `docs/qa/repo-audits/performance/repo-performance-audit-<YYYY-MM-DD>.md`

### 4) Testing Audit

- Purpose: Validate testing strategy and implementation against org standards and PRD alignment
- Cadence: After major merges; before releases; monthly baseline
- Scope: Unit coverage thresholds/trends, BDD mapping to PRD, Integration/E2E reliability, flakiness management, report generation/schema validation
- Outputs: `docs/qa/repo-audits/testing/repo-testing-audit-<YYYY-MM-DD>.md`

## Remediation SLAs (Default)

- Critical: 7 days
- High: 14 days
- Medium: 30 days
- Low: 60 days
(Teams may negotiate based on risk and resourcing.)

## Recordkeeping

- Store report, evidence (screenshots, logs), and tool outputs
- Link all remediation issues/PRs from the report and label with `audit`

## Standard References

- Coding Style: ./coding-style.md
- Pull Requests: ./pull-requests.md
- Release Format: ./release-format.md
- Project Structure: ./project-structure.md
- SW Testing: ./sw-testing.md
- File Header: ./file-header.md
- Root Directory Layout: ./root-directory.md

## Recommended Tools & Example Commands

Note: Commands are examples; adapt to language/tooling used in the repo.

- GitHub CLI
  - gh repo view --json name,defaultBranchRef,visibility
  - gh api repos/:owner/:repo/branches/:branch/protection
  - gh secret list
- Vulnerabilities
  - osv-scanner -r .
  - npm audit --production | yarn npm audit | pnpm audit
  - pip-audit | poetry export && pip-audit -r requirements.txt
  - cargo audit
- Licenses
  - npx license-checker --summary
- CI Security
  - grep -R "uses:" .github/workflows | ensure pinned SHAs
  - Validate workflow permissions: permissions: contents: read (tighten as needed)

## Templates

### Audit Report (copy into docs/audits/<date>/repository-audit.md)

Title: <repo> Repository Audit — <yyyy-mm-dd>

- Repo: <org>/<repo>
- Default Branch: <main/master>
- Audit Date: <date>
- Auditors: <names>
- Maintainers Present: <names>
- Scope: <areas>
- Score: <n/100>

#### Summary

- Overall health:
- Key risks:
- Immediate actions:

#### Checklist Outcomes

- A. Metadata & Hygiene: <pass/fail summary>
- B. Access & Governance: <pass/fail summary>
- C. CI/CD & Workflow Security: <pass/fail summary>
- D. Dependencies & Supply Chain: <pass/fail summary>
- E. Code Quality & Testing: <pass/fail summary>
- F. Releases & Change Management: <pass/fail summary>
- G. Operations & Observability: <pass/fail summary>
- H. Repository Structure: <pass/fail summary>

#### Findings Table

| ID | Severity | Area | Description | Evidence/Link | Owner | Due | Status |
|----|----------|------|-------------|---------------|-------|-----|--------|
| 1  | High     | CI   | Example...  | link          | @user | yyyy-mm-dd | Open |

#### Remediation Plan

- [ ] Item 1 — details, acceptance criteria
- [ ] Item 2 — details, acceptance criteria

#### Approvals

- Maintainer Sign-off: <name/date>
- DevSecOps Sign-off: <name/date>

---

Questions or improvements? Open an issue labeled `audit` or propose changes via PR.
