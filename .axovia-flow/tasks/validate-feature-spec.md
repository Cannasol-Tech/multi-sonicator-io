# Validate Feature Specification Task

## Purpose

To validate feature specifications against a set of criteria to ensure they meet the company's standards and are ready for implementation.

## Agents

Follow the qa agent gate template provided in the qa agent configuration file. Perform a quality gate on the completeness of the feature specification for the feature being verified. 


## Parameters

```yaml 
when_to_apply: "Before feature specification approval"
featureSpecFileLocation: "docs/agile/features/FEATURE-{id}-{name-kebab-case}.md"
validation_criteria:
  - "Verify all DoD items (FS-DOD-1 through FS-DOD-12) are satisfied"
  - "Validate frontmatter schema compliance (agent, id, title, status, priority, epic, created, updated)"
  - "Check supports section has at least one FR or NFR entry with proper traceability"
  - "Ensure acceptance criteria have clear, passing, acceptance test scenarios tagged with the ac id"
  - "Validate functional requirements completeness (core functionality, user interactions, system behaviors, data handling, integration points)"
  - "Check implementation mapping references PRD requirements with valid links/IDs"
  - "Ensure file naming follows FEATURE-{id}-{name-kebab-case}.md convention"
```