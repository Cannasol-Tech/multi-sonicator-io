# Definition of Done (DoD) Template

## Purpose

This template provides a standardized checklist for validating story completion according to company standards. All stories marked as "Done" must complete this checklist to ensure consistent quality and compliance.

## Usage Instructions

1. **Copy this template** into the story file under "## Definition of Done Checklist Completion"
2. **Complete each section** systematically during story development
3. **Mark items as complete** only when evidence exists
4. **Document evidence** for each completed item
5. **Get Test Architect validation** before marking story as "Done"

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

### Final Confirmation: [ ] **CONFIRMED**
- [ ] I, [Agent Name], confirm that all applicable DoD items have been addressed

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

## Quality Gates

### Test Architect Review Required
- [ ] **DoD Compliance Verified**: Test Architect has validated all checklist items
- [ ] **Quality Gate Created**: Gate file created in `docs/qa/gates/`
- [ ] **Traceability Verified**: Requirements mapped to tests
- [ ] **Risk Assessment Complete**: Risk profile documented if needed

### Approval Criteria
- All DoD checklist items marked as complete with evidence
- Test Architect validation completed
- Quality gate status: PASS
- No blocking issues identified

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
