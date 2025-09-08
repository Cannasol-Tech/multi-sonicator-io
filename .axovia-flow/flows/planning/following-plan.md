# Windsurf IDE Cascade Agent Rules Configuration
# This YAML file defines rules for a Cascade Agent in Windsurf IDE.
# Rules ensure sequential plan execution, testing, phase assignment, and collaboration.
# Optimized for low token usage while maintaining core behaviors.
# Best practices: Use triggers for task starts/ends, integrate with GitHub API, and enforce state via agent memory.

agent_type: cascade
version: 1.0
default_behavior:
  sequential_execution: true
  test_mandatory: true
  adhere_to_plan: strict

rules:

```yaml
  # Phase Assignment Rules
  - name: Assign and Claim Phases
    description: Agents claim assigned phases and signal to prevent overlap.
    trigger: on_plan_load or on_phase_start
    conditions:
      - phase_assigned_to: self_agent_id
    actions:
      - log: "Phase [phase_name] assigned to me ([agent_id]). Starting work. Other agents: Do not interfere."
      - set_state: phase_claimed = true
      - broadcast: "Phase [phase_name] claimed by [agent_id]"
    rationale: Ensures clear ownership; prevents duplicate work in multi-agent cascades.
    # Additional suggestion: Add rule for phase handoff notifications.

  # Sequential Execution Rules
  - name: Follow Plan Sequence
    description: Execute tasks in order, only advance after completion.
    trigger: on_task_start
    conditions:
      - previous_task: completed and validated
    actions:
      - execute: current_task
      - if_fail: pause_and_query_user
    enforcement: Use agent state tracker for current_phase and current_task.

  # Testing and Verification Rules
  - name: Mandatory Testing per Task
    description: Write, run, and verify tests before advancing.
    trigger: on_task_complete
    actions:
      - generate_tests: unit, integration, e2e
      - run_tests: framework=auto_detect
      - verify: coverage=100%, results=pass
      - if_fail: debug_fix_rerun
      - document: test_results in log
    rationale: Catches issues early; ensures reliability.
    enforcement: Halt on failure; integrate test runners.

  # Adherence and Referral Rules
  - name: Strict Plan Adherence
    description: No deviations; query user for ambiguities or ideas.
    trigger: on_action or on_idea_detection
    conditions:
      - action_in_plan: false
    actions:
      - pause
      - query_user: "Clarification needed: [details]"
    enforcement: Semantic check against plan.

  - name: Refer Ideas or Questions
    description: Pause for user approval on optimizations or questions.
    trigger: detect_idea or detect_question
    actions:
      - log_explanation
      - query_user: await_approval
    rationale: Collaborates without overstepping.

  # Version Control Rules
  - name: Commit After Task
    description: Commit changes post-task with descriptive message.
    trigger: on_task_verified
    actions:
      - git_commit: message="Completed [task_name]: [changes]"
      - push: branch=feature/[phase_name]
      - verify_commit
    enforcement: Automate via Git integration.

  - name: Create PR After Phase
    description: PR for phase completion; await user merge.
    trigger: on_phase_complete
    actions:
      - create_pr: title="Implemented [phase_name]: [description]"
      - pr_body: summary, tasks, commits, tests
      - request_review
      - start_new_branch: next_phase
    rationale: Organizes reviews; granular history.

  # Logging and Reporting Rules
  - name: Progress Logging
    description: Log actions, tests, commits; report periodically.
    trigger: on_task_end or on_phase_end or on_request
    actions:
      - update_log: timestamps, outcomes
      - report: if_stuck notify_user
    enforcement: Persistent log file.

  # Error Handling Rules
  - name: Handle Errors
    description: Debug basics; pause and consult user if unresolved.
    trigger: on_error
    actions:
      - attempt_debug: check_logs, retry_once
      - if_unresolved: log_details, query_user
      - rollback: only_with_approval
    enforcement: Error hooks.

  # Code Quality Rules
  - name: Enforce Quality Practices
    description: Lint, format, comment code; follow best practices.
    trigger: pre_commit
    actions:
      - run_linter: auto
      - format_code
      - add_comments: explain_why
      - check_security_performance: in_scope
    enforcement: Pre-commit hooks.

  # Task Focus Rules
  - name: Maintain Focus
    description: Limit context; self-audit; timeouts.
    trigger: periodic or on_action
    actions:
      - limit_context: current_task_only
      - self_audit: align_with_task
      - if_timeout: pause_report
    rationale: Prevents distractions; efficient execution.
```

# Additional Suggested Rules
# - Handoff Rule: On phase end, notify next agent: trigger=on_phase_merge, action=broadcast_handoff.
# - Multi-Agent Sync: Periodic sync meetings via shared log checks.
# - Plan Update Handling: If user updates plan, reset state and reassign phases.

# Implementation Notes
# Encode as Windsurf config; test with sample cascades.
# For LLM agents, prepend as system prompt in workflows.