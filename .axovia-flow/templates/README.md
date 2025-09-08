# Axovia Flow Templates Directory

## Overview

This directory contains template files used by the Axovia Flow agentic workflow framework for generating standardized project artifacts. Templates provide consistent structure and formatting for features, implementation plans, stories, and other development deliverables.

## Template Hierarchy

The Axovia Flow framework follows a two-tier template system:

1. **Primary Templates** (`.axovia-flow/templates/`) - Framework-specific templates that extend or override core templates
2. **Fallback Templates** (`.bmad-core/templates/`) - Core templates that provide base functionality

**Template Resolution Order:**
- First, check `.axovia-flow/templates/` for the requested template
- If not found, fallback to `.bmad-core/templates/`
- This allows projects to customize templates while maintaining core compatibility

## Available Templates

### Feature Specification Templates
- **Purpose**: Generate feature specifications following company standards
- **Output Location**: `docs/agile/features/`
- **Standard Compliance**: `.axovia-flow/company-standards/feature-specification-standard.md`

### Implementation Plan Templates  
- **Purpose**: Create visual implementation plans with story-based checklists
- **Output Location**: `docs/planning/`
- **Features**: Progress tracking, dependency mapping, risk assessment

### Story Templates
- **Primary Location**: `.bmad-core/templates/story-tmpl.yaml`
- **Purpose**: Generate user stories from feature specifications
- **Output Location**: `docs/agile/stories/`
- **Traceability**: Links to feature IDs and PRD references

## Template Configuration

Templates are configured in `.axovia-flow/flow-config.yaml`:

```yaml
templates:
  feature_spec:
    file: ".axovia-flow/templates/feature-spec-tmpl.yaml"
    description: "Feature specification template following Axovia AI standards"
    output_location: "docs/agile/features"

  implementation_plan:
    file: ".axovia-flow/templates/implementation-plan-tmpl.yaml" 
    description: "Implementation plan template with visual checklists by story"
    output_location: "docs/planning"
```

## Template Standards

All templates must:
- Follow the file header standard (`.axovia-flow/company-standards/file-header-standard.md`)
- Include proper metadata and versioning
- Maintain traceability to source requirements
- Support quality gate validation
- Be compatible with both manual and agent-driven generation

## Usage by AI Agents

AI agents use templates through the task system:
- Tasks reference templates via configuration paths
- Templates provide structure for agent outputs
- Quality gates validate template compliance
- Fallback mechanism ensures availability

## Custom Template Development

When creating custom templates:
1. Place in `.axovia-flow/templates/` to override core templates
2. Follow existing naming conventions
3. Include proper YAML frontmatter for metadata
4. Test with both manual and automated generation
5. Validate against company standards

## Integration Notes

- Templates integrate with `.bmad-core/core-config.yaml` for path resolution
- Company standards provide validation rules for template outputs  
- Quality gates ensure generated artifacts meet requirements
- Task workflows orchestrate template usage across the development lifecycle
