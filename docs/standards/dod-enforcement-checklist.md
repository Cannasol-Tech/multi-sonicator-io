# DoD Enforcement Checklist - MANDATORY QUALITY GATE

## ⚠️ CRITICAL: NO STORY CAN BE MARKED "DONE" WITHOUT THIS VALIDATION

This checklist MUST be completed by reviewers before approving any story as "Done". **NO EXCEPTIONS.**

---

## 🔍 PRE-APPROVAL VALIDATION CHECKLIST

### Story Information
- **Story ID**: _______________
- **Story Title**: _______________
- **Reviewer**: _______________
- **Review Date**: _______________

---

## ⛔ BLOCKING REQUIREMENTS - ALL MUST BE ✅

### 1. DoD Checklist Presence ⚠️ MANDATORY
- [ ] **DoD Section Exists**: Story contains "## Definition of Done Checklist Completion" section
- [ ] **All 7 Sections Present**: Requirements, Coding Standards, Testing, Functionality, Administration, Dependencies, Documentation
- [ ] **Evidence Documented**: Each section contains specific evidence, not just checkmarks
- [ ] **Agent Confirmation**: Final confirmation section completed by implementing agent

**❌ BLOCKING**: If ANY item above is incomplete, story CANNOT be marked "Done"

### 2. Test Architect Validation ⚠️ MANDATORY
- [ ] **QA Results Section**: Story contains comprehensive "## QA Results" section
- [ ] **Test Architect Review**: Review performed by Quinn (Test Architect) or authorized QA agent
- [ ] **Quality Gate File**: Gate file exists in `docs/qa/gates/{story-id}-{slug}.yml`
- [ ] **Gate Status**: Quality gate status is PASS (or WAIVED with explicit approval)

**❌ BLOCKING**: If ANY item above is incomplete, story CANNOT be marked "Done"

### 3. Test Coverage Validation ⚠️ MANDATORY
- [ ] **Coverage Documented**: Specific test coverage percentages documented in DoD checklist
- [ ] **Meets Requirement**: Coverage ≥85% for all modified modules
- [ ] **Test Evidence**: Unit test files referenced and validated
- [ ] **Test Execution**: Evidence of successful test execution provided

**❌ BLOCKING**: If coverage is below 85% or undocumented, story CANNOT be marked "Done"

### 4. Implementation Evidence ⚠️ MANDATORY
- [ ] **File List Complete**: All modified/created files listed in Dev Agent Record
- [ ] **Change Log Present**: Detailed change log with dates and descriptions
- [ ] **Requirements Traceability**: All acceptance criteria mapped to implementation evidence
- [ ] **Agent Model Documented**: Primary implementing agent/model documented

**❌ BLOCKING**: If implementation evidence is incomplete, story CANNOT be marked "Done"

---

## 🚨 CRITICAL FAILURE CONDITIONS - AUTOMATIC REJECTION

### Security & Performance
- [ ] **No Security Issues**: No unresolved security vulnerabilities identified
- [ ] **Performance Requirements**: All performance requirements met and documented
- [ ] **Error Handling**: Comprehensive error handling implemented and tested

### Code Quality
- [ ] **Linting Passes**: No linting errors or warnings introduced
- [ ] **Coding Standards**: Code follows company coding standards
- [ ] **Documentation**: Code properly documented with inline comments

### Integration
- [ ] **Build Success**: Project builds successfully without errors
- [ ] **Integration Testing**: Integration points tested and validated
- [ ] **Dependency Management**: Dependencies properly managed (no manual package edits)

**❌ AUTOMATIC REJECTION**: If ANY critical failure condition exists, story is automatically rejected

---

## 📋 REVIEWER CERTIFICATION

### Reviewer Declaration
I, _________________ (Reviewer Name), certify that:

- [ ] I have reviewed ALL sections of the DoD checklist
- [ ] I have verified ALL evidence provided is specific and adequate
- [ ] I have confirmed Test Architect validation is complete
- [ ] I have verified quality gate status is PASS or appropriately WAIVED
- [ ] I have confirmed ALL blocking requirements are satisfied
- [ ] I have verified NO critical failure conditions exist

**Reviewer Signature**: _________________
**Date**: _________________

### Test Architect Certification (Required)
I, Quinn (Test Architect), certify that:

- [ ] I have performed comprehensive DoD validation
- [ ] I have created quality gate file with appropriate status
- [ ] I have verified all test coverage requirements are met
- [ ] I have assessed and documented all risks
- [ ] I approve this story for "Done" status

**Test Architect Signature**: _________________
**Date**: _________________

---

## 🔒 ENFORCEMENT ACTIONS

### If DoD Incomplete:
1. **REJECT** story immediately
2. **BLOCK** status change to "Done"
3. **REQUIRE** complete DoD checklist before re-review
4. **DOCUMENT** rejection reason in story comments

### If Test Coverage Insufficient:
1. **REJECT** story immediately
2. **REQUIRE** additional tests to meet 85% threshold
3. **BLOCK** merge until coverage requirement met
4. **ESCALATE** to Test Architect for coverage waiver (if applicable)

### If Quality Gate FAIL:
1. **REJECT** story immediately
2. **REQUIRE** resolution of all FAIL conditions
3. **BLOCK** release inclusion until PASS status achieved
4. **ESCALATE** to Product Owner for waiver consideration (exceptional cases only)

---

## 📊 COMPLIANCE TRACKING

### Sprint Metrics
- **Stories Reviewed**: ___/___
- **DoD Compliance Rate**: ___%
- **Quality Gate Pass Rate**: ___%
- **Coverage Compliance Rate**: ___%

### Action Items
- [ ] Update team on DoD compliance requirements
- [ ] Schedule DoD training for developers
- [ ] Review and improve DoD template based on feedback
- [ ] Implement automated DoD validation tools

---

## 🎯 SUCCESS CRITERIA

**Target Metrics:**
- **DoD Compliance**: 100% (no exceptions)
- **Quality Gate Pass Rate**: ≥90%
- **Test Coverage Compliance**: 100% (≥85% requirement)
- **First-Pass Review Success**: ≥80%

**This checklist ensures consistent quality and prevents incomplete stories from being marked as "Done".**
