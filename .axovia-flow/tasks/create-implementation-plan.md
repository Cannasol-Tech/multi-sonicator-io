<!-- Powered by BMAD™ Core -->

# Create Implementation Plan Task

## ⚠️ CRITICAL EXECUTION NOTICE ⚠️

**THIS IS AN EXECUTABLE WORKFLOW - NOT REFERENCE MATERIAL**

When this task is invoked:

1. **DISABLE ALL EFFICIENCY OPTIMIZATIONS** - This workflow requires full user interaction
2. **MANDATORY STEP-BY-STEP EXECUTION** - Each section must be processed sequentially with user feedback
3. **ELICITATION IS REQUIRED** - When `elicit: true`, you MUST use the 1-9 format and wait for user response
4. **NO SHORTCUTS ALLOWED** - Complete implementation plans cannot be created without following this workflow

**VIOLATION INDICATOR:** If you create a complete implementation plan without user interaction, you have violated this workflow.

## Purpose

To create comprehensive Implementation Plans that translate Feature Specifications into actionable, visual checklists for development teams. This task ensures Implementation Plans follow Axovia AI company standards and provide clear, trackable progress indicators separated by story.

## SEQUENTIAL Task Execution (Do not proceed until current Task is complete)

### 0. Load Configuration and Validate Prerequisites

- Load `.bmad-core/core-config.yaml` and `.axovia-flow/flow-config.yaml` from the project root
- Verify access to `.axovia-flow/company-standards/implementation-plans.md`
- Verify access to `.axovia-flow/company-standards/planning-definition-of-done.md`
- If company standards not found, HALT and inform user: "Required company standards not found. Please ensure .axovia-flow/company-standards/ directory contains implementation-plans.md and planning-definition-of-done.md before proceeding."
- Extract key configurations and standard requirements

### 1. Feature Specification Analysis

#### 1.1 Identify Source Feature Specification

**MANDATORY USER INTERACTION:**

Present the following numbered options and WAIT for user response:

1. **Load from Feature Specification file** - Use existing feature spec document
2. **Reference Feature ID** - Load feature spec by ID (F-XXX format)
3. **Create from Epic/Story requirements** - Build plan from epic/story definitions
4. **Manual specification** - Gather feature requirements through elicitation

**End with:** "Select 1-4 or provide the feature specification source:"

#### 1.2 Load and Analyze Feature Specification

Based on user selection:
- **If feature spec file**: Load and parse the feature specification document
- **If feature ID**: Locate feature spec in `docs/agile/features/` directory
- **If epic/story**: Load relevant epic and story files
- **If manual**: Prepare for comprehensive feature requirement elicitation

Extract key information:
- Feature ID and name
- Scope definition (what will/won't be implemented)
- Functional requirements
- Acceptance criteria
- Success metrics

### 2. Story Decomposition Analysis

#### 2.1 Identify Implementing Stories

**ELICITATION REQUIRED** - Present content and use 1-9 format:

Analyze the feature specification and identify:
- Stories that implement this feature
- Story dependencies and sequencing
- Parallel work opportunities
- Interface definitions between stories

**Present detailed rationale explaining:**
- Story breakdown logic and reasoning
- Dependency analysis and critical path
- Parallel work opportunities identified
- Interface and handoff requirements

**MANDATORY 1-9 OPTIONS:**
1. Proceed to next section
2. Story Mapping Workshop (map user journey to stories)
3. Dependency Analysis (analyze story dependencies)
4. Parallel Work Planning (identify parallel work opportunities)
5. Interface Definition (define story interfaces)
6. Risk Assessment (assess story-level risks)
7. Effort Estimation (estimate story effort)
8. Resource Planning (plan resource allocation)
9. Timeline Analysis (analyze delivery timeline)

**End with:** "Select 1-9 or just type your question/feedback:"

### 3. Implementation Plan Structure Creation

#### 3.1 Define Plan Metadata and Overview

**ELICITATION REQUIRED**:

Draft and present:
- **Plan identification**: Feature ID, name, owner, dates
- **Purpose and overview**: Clear description of what will be delivered
- **Success criteria**: Measurable outcomes
- **Milestones and timeline**: Key delivery milestones

**Present detailed rationale explaining:**
- Timeline assumptions and constraints
- Milestone selection criteria
- Success criteria measurability
- Resource and ownership assignments

**MANDATORY 1-9 OPTIONS:**
1. Proceed to next section
2. Timeline Planning (detailed timeline analysis)
3. Milestone Definition (define detailed milestones)
4. Resource Allocation (plan resource assignments)
5. Success Metrics (define success measurements)
6. Risk Timeline Analysis (assess timeline risks)
7. Dependency Timeline (analyze dependency impacts)
8. Stakeholder Alignment (align with stakeholders)
9. Constraint Analysis (analyze delivery constraints)

### 4. Visual Checklist Creation (CRITICAL SECTION)

#### 4.1 Create Story-Specific Visual Checklists

**ELICITATION REQUIRED** - This is the core value-add section:

For each implementing story, create comprehensive visual checklists covering:
- **Planning & Setup**: Environment, dependencies, requirements review
- **Core Development Tasks**: Specific implementation tasks
- **Testing Tasks**: Unit, integration, acceptance, edge case testing
- **Documentation Tasks**: Code, API, user, architecture documentation
- **Quality Assurance**: Reviews, coverage, performance, security
- **Deployment Preparation**: Scripts, configuration, monitoring
- **Story Completion**: Verification, demo, sign-off

**Present detailed rationale explaining:**
- Task breakdown methodology
- Checklist completeness and coverage
- Quality gate integration
- Testing strategy alignment with company standards

**MANDATORY 1-9 OPTIONS:**
1. Proceed to next section
2. Task Breakdown Analysis (analyze task granularity)
3. Quality Gate Integration (integrate quality checkpoints)
4. Testing Strategy Alignment (align with testing standards)
5. Documentation Planning (plan documentation requirements)
6. Deployment Planning (plan deployment requirements)
7. Risk Mitigation Planning (plan risk mitigations)
8. Progress Tracking Design (design progress tracking)
9. Stakeholder Review Points (define review checkpoints)

### 5. Planning Definition of Done Compliance

#### 5.1 Ensure Company Standard Compliance

**ELICITATION REQUIRED**:

Verify and document compliance with Planning Definition of Done standard:
- Foundation & Alignment requirements
- Requirements & Acceptance (BDD-ready) criteria
- Test Strategy & Traceability requirements
- Risks, Dependencies, Assumptions documentation
- Story Decomposition for Parallel Work
- Repository & Standards Readiness
- Exit Criteria for "Ready for Dev"

**MANDATORY 1-9 OPTIONS:**
1. Proceed to next section
2. BDD Alignment Review (review BDD compliance)
3. Test Strategy Validation (validate test strategy)
4. Risk Assessment Review (review risk assessments)
5. Dependency Validation (validate dependencies)
6. Standards Compliance Check (check standards compliance)
7. Traceability Verification (verify traceability)
8. Quality Gate Alignment (align with quality gates)
9. Exit Criteria Validation (validate exit criteria)

### 6. QA Alignment and Test Strategy

#### 6.1 Define QA Integration

Draft and present:
- **Test strategy overview**: Unit, integration, acceptance testing approach
- **QA artifacts location**: Test plans, cases, traces, gates, assessments
- **Traceability plan**: BDD scenarios to PRD requirements mapping
- **Coverage targets**: ≥90% unit coverage and other quality metrics

### 7. Implementation Plan Completion

#### 7.1 Generate Final Implementation Plan

- Create implementation plan file using company standard template
- Apply all gathered information to template sections
- Ensure compliance with Implementation Plans standard
- Save to `docs/planning/{{feature_id}}-implementation-plan.md`

#### 7.2 Quality Validation and Completion

- Verify all mandatory sections are complete
- Check compliance with company standards
- Validate visual checklists are comprehensive and actionable
- Ensure Planning Definition of Done compliance
- Generate completion summary

## CRITICAL REMINDERS

**❌ NEVER:**
- Skip elicitation for efficiency
- Create plans without comprehensive story analysis
- Use formats other than 1-9 numbered options
- Make assumptions about implementation approach

**✅ ALWAYS:**
- Use exact 1-9 format when elicit: true
- Select options 2-9 from data/elicitation-methods only
- Provide detailed rationale explaining decisions
- End with "Select 1-9 or just type your question/feedback:"
- Follow company Implementation Plans standard exactly
- Create actionable, visual checklists for each story

## Agent Permissions

This task is designed for:
- **Primary Users**: SM (Scrum Master) agent
- **Collaboration**: Can involve PM, PO, Architect agents for specialized input
- **Output Ownership**: Scrum Master owns final implementation plan
- **Review Authority**: Product Owner and Architect validate plan completeness

## Success Criteria

Task is complete when:
- Implementation plan file is created following company standard
- Visual checklists are created for each implementing story
- All checklists are comprehensive and actionable
- Planning Definition of Done compliance is verified
- QA alignment and test strategy are defined
- Plan is ready for development team execution
- Progress tracking mechanisms are in place
