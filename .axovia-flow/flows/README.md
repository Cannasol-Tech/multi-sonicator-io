# Axovia Flow Flows Directory

## Overview

The `.axovia-flow/flows/` directory contains executable workflow definitions that guide AI agents through complex, multi-phase development processes. These flows implement structured methodologies for planning, implementation, and project management within the Axovia Flow Agentic Coding Framework.

## Directory Structure

```
.axovia-flow/flows/
├── README.md                    # This file - directory documentation
├── planning/                    # Agile planning and project setup workflows
│   ├── agile-planning.md       # Main agile planning workflow (executable)
│   ├── agile-planning.flow.yaml # YAML reference (documentation only)
│   ├── new-agile-yaml.md       # New agile workflow variant
│   └── following-plan.md       # Implementation plan execution workflow
└── [future-categories]/        # Additional workflow categories as needed
```

## Workflow Types

### Planning Workflows
Located in `planning/`, these workflows handle project initialization and agile methodologies:

- **Agile Planning** (`agile-planning.md`) - Complete 4-phase agile planning process:
  - Phase 1: Project understanding and feature identification
  - Phase 2: Feature validation and scope definition
  - Phase 3: Feature specification generation
  - Phase 4: User story creation and backlog preparation

- **Implementation Plan Following** (`following-plan.md`) - Structured execution of existing implementation plans with phase-based progression and agent coordination

### Workflow Format

Flows are primarily written as **executable Markdown** (`.md`) files that Windsurf and compatible AI agents can process directly. YAML files (`.flow.yaml`) serve as reference documentation and portability aids but are not directly executable in this framework.

## Usage Patterns

### Direct Execution
```bash
# Run agile planning workflow
/agile-planning

# Run new agile workflow variant  
/new-agile-workflow
```

### Integration with Tasks
Flows often integrate with task definitions in `.axovia-flow/tasks/` directory, providing the procedural framework while tasks define specific implementation steps.

### Multi-Agent Coordination
Flows include handoff mechanisms and synchronization points for coordinating multiple AI agents working on different aspects of a project while maintaining alignment with company standards.

## Standards Compliance

All flows are designed to:
- Adhere to company standards defined in `.axovia-flow/company-standards/`
- Generate outputs in standardized formats and locations
- Include validation checkpoints and approval gates
- Maintain traceability from requirements through implementation
- Support both single-agent and multi-agent execution patterns

## Extension Points

The flows directory is designed for extensibility:
- Add new workflow categories as subdirectories
- Create specialized flows for different project types or methodologies
- Integrate with external tools and systems through standardized interfaces
- Support custom validation and quality gates per project requirements
