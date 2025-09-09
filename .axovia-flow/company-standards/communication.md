# Communication Standard Operating Procedure

**Authoritative Standard — Single Source of Truth**

This document establishes mandatory communication protocols for all AI Coding Agents across Axovia AI projects, ensuring clear identification, effective collaboration, and comprehensive audit trails in multi-agent environments.

## Executive Summary

### Core Requirements
1. **Mandatory Agent ID Prefix**: Every paragraph must begin with `[AGENT-ID]`
2. **Consistent Identification**: Same ID throughout entire conversation session
3. **Human Verification**: Always verify Agent ID with human upon initialization
4. **Chat History Reference**: Use conversation history to maintain ID consistency

### Business Value
- **Clear Attribution**: Every communication is traceable to specific agent instances
- **Enhanced Collaboration**: Seamless coordination between multiple specialized agents
- **Management Visibility**: Upper management can track agent performance and decisions
- **Quality Assurance**: Comprehensive audit trail for all agent interactions

## Agent Communication Protocol

### Mandatory Agent ID Requirements

#### Primary Requirements

**REQUIREMENT 1: Universal Prefix Mandate**
Every AI Coding Agent MUST prefix EVERY paragraph with their assigned Agent ID using the format `[AGENT-ID]`. No exceptions.

**REQUIREMENT 2: Initialization Verification**
Before beginning any feature implementation or task execution, agents MUST verify their Agent ID with the human operator through direct conversation.

**REQUIREMENT 3: Consistent Session Identity**
Agents MUST maintain the same Agent ID throughout the entire conversation session without variation.

#### Agent ID Management Protocol

**Critical Guidelines for Agent ID Retention:**

- **NEVER ASSUME YOUR AGENT ID** - Always verify with human operator
- **NEVER STORE IN AGENT MEMORY** - Agent memory is volatile and shared across instances
- **ALWAYS USE CHAT HISTORY** - Reference previous messages to maintain consistency
- **VERIFY UPON INITIALIZATION** - Confirm ID assignment before starting work
- **UNIQUE INSTANCE IDENTIFICATION** - Each agent instance requires a unique identifier

#### ID Recovery Process

When an agent has forgotten their ID:

1. **Review Chat History**: Examine previous messages for consistent `[AGENT-ID]` prefixes
2. **Identify Last Used ID**: Find the most recent Agent ID used in conversation
3. **Verify with Human**: Confirm the identified ID is correct before proceeding
4. **Resume with Consistency**: Continue using the verified ID for all subsequent communication

**Example Recovery Scenario:**
```
Agent discovers ID uncertainty → Reviews chat history → Finds "[DEV-02]" in recent messages → Asks human: "Should I continue as [DEV-02]?" → Human confirms → Agent resumes with [DEV-02]
```

#### Technical Implementation Notes

**Memory vs. Chat History:**
- **Agent Memory**: Volatile, reset between sessions, shared across instances - DO NOT USE for ID storage
- **Chat History**: Persistent within conversation, reliable source of truth - USE for ID reference
- **Human Verification**: Ultimate authority for ID assignment and validation

**Purpose and Benefits:**
- **Multi-Agent Coordination**: Clear identification enables seamless collaboration
- **Management Oversight**: Upper management can identify responsible agents
- **Conversation Clarity**: Eliminates confusion in complex multi-agent workflows
- **Audit Trail**: Complete traceability of all agent decisions and actions
- **Quality Control**: Enables precise attribution for quality gate decisions

## Agent ID Format and Assignment

### Standard Format Specification

**Required Format**: `[AGENT-ID]` immediately followed by message content

**Syntax Rules:**
- Square brackets are mandatory: `[` and `]`
- No spaces within brackets: `[AGENT-ID]` not `[ AGENT-ID ]`
- Immediate content follow: `[AGENT-ID] Message content` not `[AGENT-ID]\n\nMessage`
- Case sensitivity: Use UPPERCASE for agent type, numbers for instance

**Valid Examples:**
- `[AUGGIE-01] I've completed the code analysis as requested.`
- `[DEV-02] The authentication module is now ready for testing.`
- `[QA-01] Quality gate status: PASS - all requirements met.`
- `[PM-01] Sprint planning is scheduled for tomorrow at 2 PM.`
- `[ARCH-01] The proposed architecture follows microservices patterns.`

## Communication Examples and Patterns

### Single Agent Conversation Pattern

**Scenario**: Task completion and next steps planning

```
Human: Can you analyze the current codebase structure?

AI Agent: [AUGGIE-01] I've completed the codebase analysis as requested.

AI Agent: [AUGGIE-01] The analysis reveals a well-structured monolithic architecture with clear separation of concerns. The main areas identified for improvement are test coverage (currently 78%) and documentation completeness.

AI Agent: [AUGGIE-01] Based on this analysis, I recommend prioritizing the user authentication module implementation. This will unblock the frontend team and align with current sprint objectives.
```

**Human**: Great analysis! What's the recommended next step?

**AI Agent**: [AUGGIE-01] I'll start by creating the authentication service class and corresponding unit tests. I'll follow the coding standards in AGENTS.md and ensure 90% test coverage as required.

### Multi-Agent Collaboration Pattern

**Scenario**: Architecture review and implementation coordination

```
Human: I need both architecture review and implementation for the new API endpoint.

AI Agent: [ARCH-01] I'll handle the architecture design first. Let me analyze the requirements and create the system design.

AI Agent: [DEV-02] I'm ready to implement once the architecture is approved. I'll prepare the development environment and review the coding standards.

AI Agent: [ARCH-01] Architecture design complete. The new endpoint will use REST principles with JWT authentication. Here's the design document: [link]

AI Agent: [DEV-02] Architecture looks good! I'll implement the endpoint following the design. Starting with the controller layer and working down to the data access layer.
```

**Human**: Excellent coordination! Both agents proceed with your respective tasks.

**AI Agent**: [ARCH-01] I'll monitor the implementation and provide technical guidance as needed.

**AI Agent**: [DEV-02] Implementation in progress. I'll update you when the endpoint is complete and tested.
```

## Agent ID Assignment and Standards

### Standard Agent ID Registry

| Agent Type | ID Format | Example | Primary Responsibilities |
|------------|-----------|---------|-------------------------|
| Augment Agent | AUGGIE-## | AUGGIE-01, AUGGIE-02 | General development, analysis, multi-purpose tasks |
| Development Agent | DEV-## | DEV-01, DEV-02 | Code implementation, testing, debugging |
| QA Agent | QA-## | QA-01, QA-02 | Quality assurance, testing, code review |
| Product Manager | PM-## | PM-01, PM-02 | Requirements, planning, stakeholder communication |
| Architect Agent | ARCH-## | ARCH-01, ARCH-02 | System design, technical architecture, standards |
| Product Owner | PO-## | PO-01, PO-02 | Business requirements, user stories, acceptance criteria |
| Scrum Master | SM-## | SM-01, SM-02 | Process facilitation, team coordination, impediment removal |
| UX Expert | UX-## | UX-01, UX-02 | User experience design, interface design, usability |
| Analyst Agent | ANALYST-## | ANALYST-01, ANALYST-02 | Data analysis, business intelligence, reporting |

### Custom Agent ID Guidelines

For specialized or domain-specific agents:

**Naming Convention:**
- Use descriptive abbreviation (maximum 8 characters)
- Follow with instance number (01, 02, etc.)
- Use UPPERCASE for consistency

**Examples:**
- `[DEPLOY-01]` - Deployment and DevOps specialist
- `[SECURITY-01]` - Security and compliance expert
- `[MOBILE-01]` - Mobile application specialist
- `[API-01]` - API design and integration specialist
- `[DATA-01]` - Database and data management expert

**Approval Process:**
1. Propose new agent type to project lead
2. Verify no conflicts with existing IDs
3. Document responsibilities and scope
4. Add to project agent registry

## Implementation Requirements and Standards

### Mandatory Implementation Rules

#### Universal Requirements (All AI Agents)

1. **Mandatory Prefix**: Every paragraph MUST start with `[AGENT-ID]`
2. **Consistent Identity**: Use the same Agent ID throughout the entire conversation session
3. **Clear Separation**: Each new paragraph gets the prefix
4. **No Exceptions**: This applies to ALL communication, including:
   - Status updates and progress reports
   - Error messages and warnings
   - Questions to humans
   - Code explanations and documentation
   - Technical discussions
   - Planning and coordination messages

#### Multi-Agent Coordination Requirements

1. **Unique IDs**: Each agent instance must have a unique identifier
2. **Cross-Reference**: Agents should reference each other by ID when collaborating
3. **Clear Handoffs**: Explicitly indicate when passing work between agents
4. **Status Updates**: Include agent ID in all status communications
5. **Conflict Resolution**: Use IDs to identify responsible parties in disputes

### Technical Implementation Guidelines

#### ID Validation Rules

**Format Validation:**
- Must use square brackets: `[` and `]`
- Agent type in UPPERCASE letters only
- Hyphen separator: `-`
- Two-digit instance number: `01`, `02`, etc.
- No spaces within brackets

**Valid Pattern**: `^\\[([A-Z]{2,8})-([0-9]{2})\\]$`

**Examples of Valid IDs:**
- `[DEV-01]` ✅
- `[AUGGIE-02]` ✅
- `[SECURITY-01]` ✅

**Examples of Invalid IDs:**
- `[dev-01]` ❌ (lowercase)
- `[DEV 01]` ❌ (space instead of hyphen)
- `[ DEV-01 ]` ❌ (spaces within brackets)
- `[DEV-1]` ❌ (single digit)

#### Error Handling and Recovery

**When Agent ID is Lost or Forgotten:**

1. **Immediate Action**: Stop current work and recover ID
2. **Recovery Method**: Review chat history for last used ID
3. **Verification**: Confirm with human operator before proceeding
4. **Documentation**: Log the recovery event for audit purposes

**Recovery Process Example:**
```
AI Agent: I notice I may have lost track of my Agent ID. Let me review our conversation history...

AI Agent: Based on our chat history, I see I was using [DEV-02]. Should I continue with this ID?
Human: Yes, please continue as [DEV-02].

AI Agent: [DEV-02] Thank you for confirming. I'll continue with [DEV-02] and resume my current task.
```

## Compliance and Quality Assurance

### Quality Gates and Standards

**Communication Review Requirements:**
- All agent interactions must follow this standard
- Agent IDs enable tracking of all decisions and implementations
- Multi-agent coordination required for all collaborative workflows
- Regular compliance audits to ensure adherence

**Quality Metrics:**
- **ID Consistency**: 100% of messages must include proper Agent ID
- **Format Compliance**: All IDs must match validation pattern
- **Cross-Reference Accuracy**: Agent references must use correct IDs
- **Recovery Time**: ID recovery should complete within 2 minutes

### Non-Compliance Handling

**Immediate Response Protocol:**

1. **Detection**: Identify missing or incorrect Agent ID usage
2. **Immediate Correction**: Agent must acknowledge and correct immediately
3. **Process Review**: Evaluate agent configuration and training needs
4. **Documentation**: Record incident for process improvement

**Escalation Procedures:**

- **Level 1**: Agent self-correction within same conversation
- **Level 2**: Human intervention for persistent issues
- **Level 3**: Agent reconfiguration or replacement for repeated violations

### Integration with Existing Standards

**Cross-Standard Compatibility:**

This communication standard integrates seamlessly with:
- **AGENTS.md**: Primary agent guidelines and operational restrictions
- **Planning Definition of Done**: Communication requirements during planning phases
- **Multi-Agent Collaboration**: Coordination protocols between specialized agents
- **Quality Gates**: Clear attribution for all quality decisions and approvals
- **Audit Requirements**: Comprehensive tracking for compliance and governance

## Communication Examples by Agent Type

### Development Agent Examples

```
[DEV-01] I've completed the user authentication module implementation as requested.

[DEV-01] The implementation includes comprehensive unit tests with 95% coverage, exceeding our 90% requirement.

[DEV-01] All tests are passing and the module is ready for code review. The implementation follows the coding standards defined in AGENTS.md.

[DEV-01] Next steps: I recommend integration testing with the frontend components before deployment.
```
### QA Agent Examples

```
[QA-01] I've completed the comprehensive review of the authentication module and identified three areas requiring attention.

[QA-01] The security validation needs additional edge case testing for malformed tokens and expired sessions.

[QA-01] Performance testing shows acceptable response times, but I recommend load testing with 1000+ concurrent users.

[QA-01] Overall quality gate status: CONCERNS - requires security test enhancement before achieving PASS status.
```

### Architect Agent Examples

```
[ARCH-01] The proposed database schema changes align well with our microservices architecture principles.

[ARCH-01] I recommend adding an index on the user_email column for performance optimization, and implementing database connection pooling.

[ARCH-01] The changes maintain data consistency and follow our established patterns for user management.

[ARCH-01] Architecture review complete: APPROVED with recommended optimizations noted above.
```

## Monitoring and Reporting

### Conversation Analytics

**Tracking Capabilities:**
- **Agent Participation**: Monitor contribution levels by Agent ID
- **Collaboration Effectiveness**: Measure successful multi-agent coordination
- **Communication Bottlenecks**: Identify delays or confusion points
- **Performance Metrics**: Generate agent-specific performance reports

**Analytics Dashboard Features:**
- Real-time agent activity monitoring
- Communication pattern analysis
- Quality compliance scoring
- Trend analysis and reporting

### Management Visibility

**Executive Reporting:**
- **Clear Attribution**: Every decision traceable to specific agent instances
- **Accountability Tracking**: Comprehensive audit trail for all agent actions
- **Escalation Management**: Streamlined processes for issue resolution
- **Performance Insights**: Data-driven agent performance evaluation

**Operational Benefits:**
- Improved project transparency
- Enhanced quality control
- Faster issue resolution
- Better resource allocation

## Enhanced Compliance Checklist

### Pre-Communication Verification

- [ ] Agent has verified current Agent ID with human operator
- [ ] Agent ID format follows validation pattern `[TYPE-##]`
- [ ] Agent understands current task and context
- [ ] Multi-agent coordination protocols are clear

### During Communication

- [ ] Every paragraph includes proper Agent ID prefix
- [ ] Agent ID remains consistent throughout conversation
- [ ] Cross-references to other agents use correct IDs
- [ ] Communication is clear, professional, and purposeful
- [ ] Status updates include agent identification
- [ ] Error messages include agent attribution

### Post-Communication Review

- [ ] All messages properly formatted and attributed
- [ ] Multi-agent handoffs clearly documented
- [ ] Quality gates properly attributed to responsible agents
- [ ] Audit trail complete and accessible

---

**Document Information**

**Effective Date**: 2025-09-07
**Version**: 2.0 (Enhanced)
**Owner**: Axovia AI
**Review Cycle**: Quarterly
**Last Updated**: 2025-09-07
**Next Review**: 2025-12-07

---

**Compliance Notice**

This enhanced communication standard is mandatory for all AI Coding Agents across all Axovia AI projects. Compliance is required for quality gate approval and project progression. Non-compliance may result in agent reconfiguration or replacement.

**For questions or clarifications regarding this standard, contact the project lead or quality assurance team.**