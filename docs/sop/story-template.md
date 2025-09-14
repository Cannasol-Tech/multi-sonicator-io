---
# {{ banners.header.ai_generated }}
id: {{ id | default: "S-X.Y" }}
title: {{ title | default: "<Short story title>" }}
status: {{ status | default: "backlog" }} # backlog | ready | in_progress | blocked | in_review | done | archived
owner: {{ owner | default: "<role or person>" }}
assignee: {{ assignee | default: "<current person/agent>" }}
updated_at: {{ updated_at | default: "YYYY-MM-DD" }}
version: {{ version | default: "1.0" }}
links:
  epic: {{ links.epic | default: "docs/epics/<epic-file>.md" }}
  tests: {{ links.tests | default: [] }}
provenance:
  banner: "{{ banners.header.ai_generated }}"
---

# {{ title | default: "Story X.Y: <Title>" }}
<!-- {{ provenance.banner }} -->

**Story ID**: {{ id | default: "S-X.Y" }}  
**Epic**: {{ epic | default: "<Epic reference>" }}  
**Priority**: {{ priority | default: "Medium" }}  
**Owner**: {{ owner | default: "<role or person>" }}  
**Created**: {{ created_at | default: "YYYY-MM-DD" }}  
**Updated**: {{ updated_at | default: "YYYY-MM-DD" }}

## Status
<!-- {{ provenance.banner }} -->

{{ human_status | default: "Backlog" }}

## User Story
<!-- {{ provenance.banner }} -->

As a {{ persona | default: "user/role" }},  
I want {{ goal | default: "<desired capability>" }},  
so that {{ benefit | default: "<business value>" }}.

## Acceptance Criteria
<!-- {{ provenance.banner }} -->

1. {{ ac1 | default: "<criterion 1>" }}
2. {{ ac2 | default: "<criterion 2>" }}
3. {{ ac3 | default: "<criterion 3>" }}

## Tasks

- [ ] {{ task1 | default: "<task>" }}
- [ ] {{ task2 | default: "<task>" }}
- [ ] {{ task3 | default: "<task>" }}

## Definition of Done

- [ ] Acceptance criteria satisfied  
- [ ] Tests passing (unit/integration/e2e as applicable)  
- [ ] Documentation updated

## Links

- Epic: {{ links.epic | default: "docs/epics/<epic-file>.md" }}
- Tests: {{ links.tests | default: [] }}
