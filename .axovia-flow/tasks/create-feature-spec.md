<!-- 
-----------------
# Author: Stephen Boyett
# Company: Axovia AI
# Date: 2025-09-06
# Brief: Executable workflow for creating feature specifications.
# Version: 1.0
-----------------
 -->

# Create Feature Specification Task

## ⚠️ CRITICAL EXECUTION NOTICE ⚠️

**THIS IS AN EXECUTABLE WORKFLOW - NOT REFERENCE MATERIAL**

When this task is invoked:

1. **DISABLE ALL EFFICIENCY OPTIMIZATIONS** - This workflow requires full user interaction
2. **MANDATORY STEP-BY-STEP EXECUTION** - Each section must be processed sequentially with user feedback
3. **ELICITATION IS REQUIRED** - When `elicit: true`, you MUST use the 1-9 format and wait for user response
4. **NO SHORTCUTS ALLOWED** - Complete feature specifications cannot be created without following this workflow

**VIOLATION INDICATOR:** If you create a complete feature specification without user interaction, you have violated this workflow.

## Purpose

To create comprehensive, company-standard Feature Specifications that define EXACTLY what will and won't be implemented by development teams. This task ensures Feature Specifications follow Axovia AI's Feature Specification Standard and serve as definitive contracts between Product and Development.

## SEQUENTIAL Task Execution (Do not proceed until current Task is complete)

### 0. Load Configuration and Validate Prerequisites

- Load `.bmad-core/core-config.yaml` from the project root if available
- Verify access to `.axovia-flow/company-standards/feature-specification.md`
- If Feature Specification Standard not found, HALT and inform user: "Feature Specification Standard not found. This file is required for creating compliant feature specifications. Please ensure .axovia-flow/company-standards/feature-specification.md exists before proceeding."
- Extract key configurations and standard requirements

### 1. Feature Identification and Context Gathering

#### 1.1 Identify Feature Requirements Source

**MANDATORY USER INTERACTION:**

Present the following numbered options and WAIT for user response:

1. **Create from PRD section** - Extract requirements from existing PRD
2. **Create from Epic definition** - Base on epic requirements  
3. **Create from user requirements** - Gather requirements through elicitation
4. **Create from existing draft** - Refine existing feature specification

**End with:** "Select 1-4 or describe your feature specification source:"

#### 1.2 Gather Source Context

Based on user selection:
- **If PRD section**: Load and review relevant PRD sections
- **If Epic**: Load epic file and extract feature requirements
- **If user requirements**: Prepare for comprehensive elicitation
- **If existing draft**: Load existing specification for refinement

### 2. Feature Specification Setup

#### 2.1 Define Basic Feature Information

**ELICITATION REQUIRED** - Present content and use 1-9 format:

Gather and present:
- **Feature ID**: Generate next sequential F-XXX ID
- **Feature Name**: Clear, descriptive name
- **Priority**: Critical/High/Medium/Low
- **Epic Reference**: Link to parent epic if applicable
- **Owner**: Product Owner responsible

**Present detailed rationale explaining:**
- ID assignment logic and sequence
- Naming conventions and clarity
- Priority assessment criteria
- Epic relationship and dependencies

**MANDATORY 1-9 OPTIONS:**
1. Proceed to next section
2. Stakeholder Analysis (identify all affected parties)
3. Competitive Analysis (review similar features)
4. User Journey Mapping (map user experience)
5. Technical Feasibility Review (assess implementation complexity)
6. Risk Assessment (identify potential risks)
7. Assumption Testing (validate key assumptions)
8. Constraint Analysis (identify limitations)
9. Impact Analysis (assess business impact)

**End with:** "Select 1-9 or just type your question/feedback:"

### 3. Scope Definition (CRITICAL SECTION)

#### 3.1 Define Implementation Scope

**ELICITATION REQUIRED** - This is the most critical section:

Draft and present:
- **EXACTLY What Will Be Implemented**: Precise functionality list
- **EXACTLY What Will NOT Be Implemented**: Explicit exclusions
- **Scope Boundaries**: Clear start/stop boundaries

**Present detailed rationale explaining:**
- Scope decisions and trade-offs made
- Boundary definitions and reasoning
- Exclusion rationale and future considerations
- Risk areas requiring validation

**MANDATORY 1-9 OPTIONS:**
1. Proceed to next section
2. Scope Validation Workshop (validate with stakeholders)
3. Boundary Analysis (examine edge cases)
4. Exclusion Impact Assessment (assess excluded functionality impact)
5. Future Scope Planning (plan for excluded features)
6. Dependency Mapping (map scope dependencies)
7. Assumption Documentation (document scope assumptions)
8. Risk Identification (identify scope-related risks)
9. Stakeholder Alignment Check (verify stakeholder agreement)

### 4. Functional Requirements Definition

#### 4.1 Core Functionality Specification

**ELICITATION REQUIRED**:

Draft and present:
- **Core Functionality**: Step-by-step feature operation
- **User Interactions**: Every interaction with expected responses
- **System Behaviors**: All system behaviors specified
- **Data Handling**: Exact data processing requirements
- **Integration Points**: Precise integration specifications

**Present detailed rationale explaining:**
- Functionality design decisions
- Interaction patterns chosen
- Data handling approach
- Integration strategy and dependencies

**MANDATORY 1-9 OPTIONS:**
1. Proceed to next section
2. User Story Mapping (map user interactions)
3. Process Flow Analysis (analyze business processes)
4. Data Flow Modeling (model data movement)
5. Integration Planning (plan system integrations)
6. Error Scenario Planning (plan error handling)
7. Performance Requirements (define performance needs)
8. Security Requirements (define security needs)
9. Usability Requirements (define usability needs)

### 5. Acceptance Criteria and Definition of Done

#### 5.1 Define Measurable Acceptance Criteria

**ELICITATION REQUIRED**:

Draft and present:
- **Primary Acceptance Criteria**: Must-have functionality
- **Secondary Acceptance Criteria**: Should-have functionality  
- **Definition of Done**: Exact completion requirements
- **Test Scenarios**: Specific test cases
- **Quality Gates**: Quality validation requirements

**MANDATORY 1-9 OPTIONS:**
1. Proceed to next section
2. Test Case Development (develop detailed test cases)
3. Quality Metrics Definition (define quality measures)
4. Acceptance Test Planning (plan acceptance testing)
5. Performance Criteria (define performance acceptance)
6. Security Criteria (define security acceptance)
7. Usability Criteria (define usability acceptance)
8. Integration Criteria (define integration acceptance)
9. Rollback Criteria (define rollback conditions)

### 6. Implementation Mapping and Traceability

#### 6.1 Map to Development Artifacts

Draft and present:
- **Related PRD Requirements**: Traceability to PRD
- **Implementing User Stories**: Stories that will implement feature
- **Quality Assurance**: QA requirements
- **Success Metrics**: Measurable success criteria

### 7. Feature Specification Completion

#### 7.1 Generate Final Specification

- Create feature specification file using company standard template
- Apply all gathered information to template sections
- Ensure compliance with Feature Specification Standard
- Save to `docs/agile/features/feature-{ID}-{name-kebab-case}.md`

#### 7.2 Quality Validation

- Verify all mandatory sections are complete
- Check compliance with company standards
- Validate traceability and completeness
- Generate completion summary

## CRITICAL REMINDERS

**❌ NEVER:**
- Skip elicitation for efficiency
- Create specifications without user interaction
- Use formats other than 1-9 numbered options
- Make assumptions about requirements

**✅ ALWAYS:**
- Use exact 1-9 format when elicit: true
- Select options 2-9 from data/elicitation-methods only
- Provide detailed rationale explaining decisions
- End with "Select 1-9 or just type your question/feedback:"
- Follow company Feature Specification Standard exactly

## Agent Permissions

This task is designed for:
- **Primary Users**: PM (Product Manager) and PO (Product Owner) agents
- **Collaboration**: Can involve other agents for specialized elicitation
- **Output Ownership**: Product Owner owns final specification
- **Review Authority**: Product Manager validates business alignment

## Success Criteria

Task is complete when:
- Feature specification file is created following company standard
- All mandatory sections are populated with detailed requirements
- Scope is clearly defined with explicit inclusions/exclusions
- Acceptance criteria are measurable and testable
- Traceability to source requirements is established
- Specification passes company standard compliance check
