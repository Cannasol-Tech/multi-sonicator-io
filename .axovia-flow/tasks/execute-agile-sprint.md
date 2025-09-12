# Execute Agile Sprint Task

## Purpose

To execute an Agile Sprint based on the feature specifications in the project.

## Agents

- Director: Director of Flow (DOF)

- Primary: Scrum Master (SM)
- Secondary: Product Owner (PO)
- Tertiary: Developer (Dev)
- Tertiary: Quality Assurance (QA)

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
  - sprint_plan: axoviaFlow.fileLocations.sprintPlan
``` 
