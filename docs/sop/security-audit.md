# Security Audit — Standard Operating Procedure (SOP)

Last updated: 2025-09-11
Owner: DevSecOps
Applies to: All repositories under Axovia-AI

## Purpose
Establish a consistent process to identify, assess, and remediate security risks across code, dependencies, CI/CD workflows, and repository settings.

## Scope
- Source code (secrets, insecure patterns)
- Dependencies and supply chain (vulns, licenses)
- CI/CD workflows (permissions, external actions, secrets usage)
- GitHub settings (branch protection, secret scanning)
- Artifacts and reports (no secrets in caches/artifacts)

## Success Criteria (Initial)
- No open Critical findings at audit close
- High findings have remediation plan with owner and due date
- Evidence of scans and configuration reviews captured in repo

## Audit Checklist (Pass/Fail/N/A with notes)
- [ ] Secret scanning enabled; no plaintext secrets in repo
- [ ] Required branch protections configured on default branch
- [ ] Actions use least-privilege permissions; external actions pinned by commit SHA
- [ ] Dependency vulnerability scans run and reviewed (OSV/Safety/Bandit)
- [ ] No EOL/unsupported critical dependencies
- [ ] LICENSE compliance validated for dependencies
- [ ] Build artifacts/caches do not contain secrets
- [ ] Environment variables documented; sensitive values stored as encrypted secrets
- [ ] Security advisories configured (public repos)

## Procedure
1) Plan: confirm scope and stakeholders
2) Automated scans: dependencies, SAST, license
3) Manual review: workflows, permissions, protections
4) Findings: severity, owner, due date
5) Report: store under docs/audits/<yyyy-mm>/security-audit.md with evidence links
6) Remediation: track via issues/PRs
7) Verification: re-run targeted checks and close

## Reporting Template
- Title, Date, Participants
- Summary with key risks
- Checklist outcomes
- Findings table (ID, Severity, Area, Description, Evidence, Owner, Due, Status)
- Remediation plan and sign-offs

## Recommended Tools (adapt per repo)
- Dependency and vuln scanning: osv-scanner, safety, bandit
- GitHub CLI for settings: branch protections, secrets
- License checks: license-checker

---

TODO
- Define severity thresholds and scoring rubric
- Add example GH CLI commands for settings export
- Integrate with CI quality gates for automated baseline checks