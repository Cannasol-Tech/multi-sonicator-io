# DoD Enforcement Guide - MANDATORY QUALITY GATES

## ⚠️ CRITICAL: ZERO TOLERANCE POLICY

**NO STORY CAN BE MARKED "DONE" WITHOUT COMPLETE DoD VALIDATION**

This guide documents all enforcement mechanisms to ensure 100% DoD compliance across all development work.

---

## 🔒 ENFORCEMENT MECHANISMS

### 1. Template-Based Enforcement
**Location**: `docs/standards/definition-of-done-template.md`

**Mandatory Elements**:
- ⚠️ **BLOCKING NOTICES**: Clear warnings that DoD is mandatory
- 🚫 **ENFORCEMENT RULES**: Specific rules for developers, test architects, and PMs
- ✅ **7-Section Checklist**: Complete DoD validation framework
- 📋 **Evidence Requirements**: Specific evidence documentation for each section

### 2. Reviewer Checklist Enforcement
**Location**: `docs/standards/dod-enforcement-checklist.md`

**Blocking Requirements**:
- ❌ **DoD Checklist Presence**: Must exist in story file
- ❌ **Test Architect Validation**: Must be completed by Quinn or authorized QA agent
- ❌ **Quality Gate File**: Must exist with PASS status
- ❌ **Test Coverage**: Must meet ≥85% requirement

### 3. Automated Makefile Validation
**Location**: `Makefile` (lines 626-717)

**Available Commands**:
```bash
# Validate specific story DoD compliance
make validate-dod STORY=1.3

# Check all completed stories for DoD compliance
make check-dod-compliance

# Enforce DoD gate before story completion
make enforce-dod-gate STORY=1.3

# Complete story readiness validation
make validate-story-completion STORY=1.3
```

---

## 🚫 BLOCKING CONDITIONS - AUTOMATIC FAILURE

### Story-Level Blocks
1. **Missing DoD Section**: Story lacks "## Definition of Done Checklist Completion"
2. **Incomplete Evidence**: Any DoD section lacks specific evidence
3. **Missing QA Results**: No Test Architect review section
4. **No Quality Gate**: Missing gate file in `docs/qa/gates/`
5. **Failed Quality Gate**: Gate status is FAIL without approved waiver

### Technical Blocks
1. **Test Coverage**: Below 85% requirement for any modified module
2. **Build Failures**: Project doesn't build successfully
3. **Linting Errors**: New linting errors or warnings introduced
4. **Security Issues**: Unresolved security vulnerabilities

### Process Blocks
1. **No Test Architect Review**: Missing Quinn validation
2. **Incomplete File List**: Dev Agent Record missing file references
3. **Missing Change Log**: No detailed change tracking
4. **Agent Model Undocumented**: Primary implementing agent not recorded

---

## 👥 ROLE-SPECIFIC ENFORCEMENT

### For Developers
**CANNOT DO**:
- ❌ Mark story status as "Done" without complete DoD checklist
- ❌ Merge code without Test Architect DoD approval
- ❌ Skip evidence documentation for any DoD section
- ❌ Bypass quality gate requirements

**MUST DO**:
- ✅ Use DoD template from `docs/standards/definition-of-done-template.md`
- ✅ Complete all 7 DoD sections with specific evidence
- ✅ Request Test Architect review using `*review {story}` command
- ✅ Validate DoD compliance using `make validate-dod STORY={id}`

### For Test Architects (Quinn)
**CANNOT DO**:
- ❌ Approve stories with incomplete DoD evidence
- ❌ Create PASS quality gates without validating all checklist items
- ❌ Skip comprehensive review requirements
- ❌ Allow DoD bypassing for any reason without explicit waiver

**MUST DO**:
- ✅ Validate every DoD checklist item before approval
- ✅ Create quality gate file using `*gate {story}` command
- ✅ Provide PASS/CONCERNS/FAIL/WAIVED decision with rationale
- ✅ Update story QA Results section with comprehensive review

### For Project Managers
**CANNOT DO**:
- ❌ Accept stories as "Done" without Test Architect DoD approval
- ❌ Include stories in releases without PASS quality gate status
- ❌ Allow DoD compliance exceptions without documented waivers
- ❌ Skip DoD compliance tracking in sprint reviews

**MUST DO**:
- ✅ Enforce DoD compliance across all development teams
- ✅ Use `make check-dod-compliance` for sprint validation
- ✅ Track DoD compliance metrics (target: 100%)
- ✅ Escalate DoD violations immediately

---

## 🔍 VALIDATION WORKFLOW

### Step 1: Developer Self-Validation
```bash
# Before requesting review
make validate-dod STORY=1.3
```

### Step 2: Test Architect Review
```bash
# Quinn executes comprehensive review
*review 1.3
```

### Step 3: Quality Gate Creation
```bash
# Quinn creates quality gate
*gate 1.3
```

### Step 4: Final Validation
```bash
# Validate complete story readiness
make validate-story-completion STORY=1.3
```

---

## 📊 COMPLIANCE MONITORING

### Daily Checks
- **Active Stories**: Verify DoD progress on in-development stories
- **Review Queue**: Ensure Test Architect reviews are current
- **Quality Gates**: Monitor gate status distribution

### Sprint Reviews
- **DoD Compliance Rate**: Target 100% (no exceptions)
- **Quality Gate Pass Rate**: Target ≥90%
- **Test Coverage Compliance**: Target 100% (≥85% requirement)
- **First-Pass Review Success**: Target ≥80%

### Release Gates
- **All Stories**: Must have PASS quality gate status
- **No Exceptions**: Zero tolerance for incomplete DoD
- **Waiver Process**: Explicit Product Owner approval required for any waivers

---

## 🚨 ESCALATION PROCEDURES

### DoD Violation Response
1. **Immediate**: Block story progression
2. **Notify**: Alert developer and Test Architect
3. **Document**: Record violation in story comments
4. **Resolve**: Complete DoD requirements before proceeding

### Repeated Violations
1. **First Violation**: Developer coaching and DoD training
2. **Second Violation**: Team lead involvement and process review
3. **Third Violation**: Formal process improvement plan required

### Emergency Waivers
1. **Product Owner Approval**: Required for any DoD waiver
2. **Documentation**: Explicit waiver reason and timeline
3. **Follow-up**: Mandatory DoD completion in next sprint
4. **Review**: Post-release waiver impact assessment

---

## 🎯 SUCCESS METRICS

### Target KPIs
- **DoD Compliance**: 100% (mandatory)
- **Quality Gate Pass Rate**: ≥90%
- **Test Coverage**: 100% compliance with ≥85% requirement
- **Review Cycle Time**: ≤2 days from DoD completion to approval

### Quality Indicators
- **Zero Defect Releases**: Stories with complete DoD have minimal post-release issues
- **Reduced Rework**: Complete DoD reduces story reopening rates
- **Team Confidence**: Consistent quality standards improve team velocity

**This enforcement framework ensures no story can bypass DoD requirements and maintains consistent quality across all development work.**
