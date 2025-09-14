# Definition of Done (DoD) Template - MANDATORY QUALITY GATE

## ⚠️ CRITICAL ENFORCEMENT NOTICE

**THIS IS A MANDATORY QUALITY GATE - NO EXCEPTIONS**

- ❌ **BLOCKING**: Stories CANNOT be marked as "Done" without complete DoD validation
- ❌ **BLOCKING**: Stories CANNOT be merged without Test Architect DoD approval
- ❌ **BLOCKING**: Stories CANNOT be released without PASS quality gate status
- ✅ **REQUIRED**: All 7 DoD sections must be completed with evidence
- ✅ **REQUIRED**: Test Architect must validate and create quality gate file

## Purpose

This template provides a **mandatory** standardized checklist for validating story completion according to company standards. **NO STORY** can be marked as "Done" without completing this checklist and receiving Test Architect validation.

## MANDATORY Usage Instructions - NO BYPASSING ALLOWED

1. **MANDATORY**: Copy this template into the story file under "## Definition of Done Checklist Completion"
2. **MANDATORY**: Complete each section systematically during story development
3. **MANDATORY**: Mark items as complete only when evidence exists
4. **MANDATORY**: Document evidence for each completed item
5. **MANDATORY**: Get Test Architect validation using `*review {story}` command
6. **MANDATORY**: Receive PASS quality gate status before marking story as "Done"

## ⛔ ENFORCEMENT RULES

### For Developers:
- **CANNOT** change story status to "Done" without complete DoD checklist
- **CANNOT** merge code without Test Architect DoD approval
- **MUST** provide specific evidence for every checklist item
- **MUST** request Test Architect review when DoD is complete

### For Test Architects:
- **MUST** validate every DoD checklist item before approval
- **MUST** create quality gate file using `*gate {story}` command
- **MUST** provide PASS/CONCERNS/FAIL/WAIVED decision with rationale
- **CANNOT** approve stories with incomplete DoD evidence

### For Project Managers:
- **CANNOT** accept stories as "Done" without Test Architect DoD approval
- **CANNOT** include stories in releases without PASS quality gate status
- **MUST** enforce DoD compliance across all development teams
- **MUST** track DoD compliance metrics in sprint reviews

---

## Definition of Done Checklist Completion

### DoD Validation Date: [YYYY-MM-DD]

### Validated By: [Agent Name/Test Architect]

**COMPREHENSIVE DOD ANALYSIS** - [Story Title]

### 1. Requirements Met: [ ] **STATUS**
- [ ] All functional requirements specified in the story are implemented
- [ ] All acceptance criteria defined in the story are met (X/X ACs satisfied)

**Evidence:**
- [List specific evidence of requirement completion]
- [Reference implementation files, test results, etc.]

### 2. Coding Standards & Project Structure: [ ] **STATUS**
- [ ] All new/modified code strictly adheres to Operational Guidelines
- [ ] All new/modified code aligns with Project Structure
- [ ] Adherence to Tech Stack requirements
- [ ] Basic security best practices applied
- [ ] No new linter errors or warnings introduced
- [ ] Code is well-commented with appropriate documentation

**Evidence:**
- [List coding standards compliance evidence]
- [Reference linting results, code review notes]

### 3. Testing: [ ] **STATUS**
- [ ] All required unit tests implemented
- [ ] All tests pass successfully
- [ ] Test coverage meets project standards (≥85% requirement)

**Evidence:**
- [List test implementation details]
- [Reference coverage reports, test execution results]

### 4. Functionality & Verification: [ ] **STATUS**
- [ ] Functionality manually verified
- [ ] Edge cases and error conditions handled gracefully
- [ ] Integration testing completed where applicable

**Evidence:**
- [List verification activities performed]
- [Reference manual testing results, integration test outcomes]

### 5. Story Administration: [ ] **STATUS**
- [ ] All tasks within the story file are marked as complete
- [ ] Clarifications and decisions documented
- [ ] Story wrap-up section completed with detailed notes

**Evidence:**
- [Reference completed task lists, decision documentation]

### 6. Dependencies, Build & Configuration: [ ] **STATUS**
- [ ] Project builds successfully without errors
- [ ] Project linting passes
- [ ] Dependencies properly managed (no manual package file edits)
- [ ] No security vulnerabilities introduced
- [ ] Environment variables and configurations handled securely

**Evidence:**
- [List build verification results]
- [Reference dependency management activities]

### 7. Documentation: [ ] **STATUS**
- [ ] Comprehensive inline code documentation
- [ ] Technical documentation updated where applicable
- [ ] User-facing documentation updated if needed

**Evidence:**
- [List documentation updates performed]
- [Reference updated documentation files]

### Final Confirmation: [ ] **CONFIRMED** ⚠️ MANDATORY
- [ ] I, [Agent Name], confirm that all applicable DoD items have been addressed
- [ ] I, [Agent Name], confirm that all evidence has been documented and validated
- [ ] I, [Agent Name], request Test Architect review for quality gate validation

**⛔ CRITICAL**: This story CANNOT be marked as "Done" until Test Architect validates this DoD checklist and creates a PASS quality gate.

## Dev Agent Record

### Tasks Completed
- [ ] **[Task Category]**: [Description of work completed]
- [ ] **[Task Category]**: [Description of work completed]

### Implementation Quality Metrics
- **Code Quality**: [Description of implementation quality]
- **Test Coverage**: [Specific coverage percentages]
- **Performance**: [Performance characteristics if applicable]
- **Memory Efficiency**: [Resource utilization if applicable]

### File List
**Implemented/Modified Files:**
- `[file path]` - [Description of changes]
- `[file path]` - [Description of changes]

### Change Log
- **[Date]**: [Description of major changes/milestones]
- **[Date]**: [Description of major changes/milestones]

### Agent Model Used
**Primary**: [Agent type/model used for implementation]
**Testing**: [Agent type/model used for testing]
**Validation**: [Agent type/model used for validation]

---

## ⛔ MANDATORY QUALITY GATES - BLOCKING REQUIREMENTS

### 🚫 STORY CANNOT BE MARKED "DONE" WITHOUT:

#### 1. Complete DoD Checklist ⚠️ MANDATORY
- [ ] **DoD Compliance Verified**: Test Architect has validated ALL checklist items
- [ ] **Evidence Documented**: Specific evidence provided for every checklist item
- [ ] **Quality Gate Created**: Gate file created in `docs/qa/gates/` with PASS status
- [ ] **Traceability Verified**: All requirements mapped to tests with evidence
- [ ] **Risk Assessment Complete**: Risk profile documented and acceptable

#### 2. Test Architect Approval ⚠️ MANDATORY
- [ ] **Test Architect Review**: `*review {story}` command executed by Test Architect
- [ ] **Quality Gate Status**: PASS (CONCERNS/FAIL/WAIVED require explicit approval)
- [ ] **QA Results Updated**: Comprehensive review results added to story file
- [ ] **Gate File Created**: Quality gate file exists in `docs/qa/gates/`

#### 3. Blocking Conditions ⚠️ AUTOMATIC FAILURE
- ❌ **FAIL**: Any DoD checklist item incomplete or lacking evidence
- ❌ **FAIL**: Test coverage below 85% requirement
- ❌ **FAIL**: Quality gate status is FAIL without approved waiver
- ❌ **FAIL**: No Test Architect validation performed
- ❌ **FAIL**: Critical security or performance issues unresolved

### 🔒 ENFORCEMENT MECHANISMS

#### Automated Checks (Future Implementation)
- **Pre-merge Hook**: Verify DoD checklist completion before code merge
- **Status Validation**: Block status change to "Done" without quality gate PASS
- **Release Gate**: Prevent release inclusion without DoD compliance

#### Manual Enforcement (Immediate)
- **Code Review**: Reviewers must verify DoD completion before approval
- **Sprint Review**: PM must validate DoD compliance before story acceptance
- **Release Planning**: Only stories with PASS quality gates included in releases

---

## Template Notes

**For Developers:**
- Complete this checklist systematically during development
- Provide specific evidence for each item
- Request Test Architect review when ready

**For Test Architects:**
- Validate evidence provided for each checklist item
- Create quality gate file using `*gate {story}` command
- Update story QA Results section with comprehensive review

**For Project Managers:**
- Ensure DoD completion before accepting story as "Done"
- Use quality gate status for release planning decisions
- Track DoD compliance metrics across sprints
