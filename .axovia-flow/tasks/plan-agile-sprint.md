# Plan Agile Sprint Task

## Purpose

To plan an Agile Sprint based on the feature specifications in the project.

## Agents

- Director: Director of Flow (DOF)

- Primary: Scrum Master (SM)
- Secondary: Product Owner (PO)
- Tertiary: Developer (Dev)

## Parameters

```yaml
required:
  - feature_specs_path: axoviaFlow.fileLocations.features
  - stories_path: axoviaFlow.fileLocations.stories
  - epics_path: axoviaFlow.fileLocations.epics
  - prd_shards: axoviaFlow.fileLocations.prd
  - architecture_shards: axoviaFlow.fileLocations.architecture
  - qa_assessments: axoviaFlow.fileLocations.qa.assessments
  - qa_gates: axoviaFlow.fileLocations.qa.gates
optional
  - previous_sprint_report: axoviaFlow.fileLocations.previousSprintReport
``` 


## Output

```yaml
sprint_plan:
  path: axoviaFlow.fileLocations.sprintPlan
  content:
    confidence_level: 0-5
    dependencies: []
    confidence_reasoning: ""
    concerns: []
    risks: []
    stories: []
    features: []
    epics: []
```

## Process 

**CRITICAL PROCESS WORKFLOW**

1. Load Configuration and Validate Prerequisites
2. Load Feature Specifications
3. Load Stories
4. Load Epics
5. Load PRD Shards
6. Load Architecture Shards
7. Load QA Assessments
8. Load QA Gates
9. Load Previous Sprint Report
