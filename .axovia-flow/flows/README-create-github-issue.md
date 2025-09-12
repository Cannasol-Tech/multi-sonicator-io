# GitHub Issue Creation Flow

## Overview

The `create-github-issue.yaml` flow automates the creation of GitHub issues following your repository's issue template standards. It integrates with the existing `.github/ISSUE_TEMPLATE/` directory and ensures consistency with your project's issue management practices.

## Features

- **Template Compliance**: Automatically uses issue templates from `.github/ISSUE_TEMPLATE/`
- **Multiple Issue Types**: Supports bug reports, feature requests, enhancements, documentation, and questions
- **Label Management**: Combines template labels with user-provided labels
- **Title Formatting**: Applies consistent prefixes based on issue type
- **Preview Mode**: Shows issue details before creation
- **GitHub API Integration**: Creates issues directly via GitHub API

## Usage

### Basic Usage

```yaml
# Run the flow with minimal inputs
flow: create_github_issue
inputs:
  title: "Live Monitoring Display Shows GREEN When Hardware Not Configured"
  issue_type: bug_report
```

### Advanced Usage

```yaml
# Run with full configuration
flow: create_github_issue
inputs:
  title: "Add support for multiple sonicator configurations"
  issue_type: feature_request
  repository: "Cannasol-Tech/multi-sonicator-io"
  assignees: ["sboyett31"]
  labels: ["enhancement", "hardware"]
  milestone: "v2.0.0"
```

## Supported Issue Types

| Type | Prefix | Template |
|------|--------|----------|
| `bug_report` | `[BUG]` | `.github/ISSUE_TEMPLATE/bug_report.md` |
| `feature_request` | `[FEATURE]` | `.github/ISSUE_TEMPLATE/feature_request.md` |
| `enhancement` | `[ENHANCEMENT]` | `.github/ISSUE_TEMPLATE/enhancement.md` |
| `documentation` | `[DOCS]` | `.github/ISSUE_TEMPLATE/documentation.md` |
| `question` | `[QUESTION]` | `.github/ISSUE_TEMPLATE/question.md` |

**Note**: If a specific template doesn't exist, the flow falls back to `bug_report.md`.

## Flow Steps

1. **Template Validation**: Checks if the selected issue template exists
2. **Template Loading**: Parses frontmatter and body from the template
3. **Title Formatting**: Applies appropriate prefixes and formatting
4. **Label Preparation**: Combines template and user-provided labels
5. **Assignee Setup**: Formats assignees for GitHub API
6. **Body Creation**: Generates issue body with metadata footer
7. **Preview**: Shows user the issue details before creation
8. **GitHub Creation**: Creates the issue via GitHub API
9. **Summary**: Provides completion details and issue URL

## Integration

### Axovia Flow Framework

This flow integrates with the Axovia Flow™ framework and can be used by:
- **bmad-master**: For general issue management
- **pm**: For project management tasks
- **qa**: For quality assurance issue tracking

### GitHub API Requirements

The flow requires GitHub API access with the following permissions:
- `repo` scope for creating issues
- `write:repo_hook` for webhook management (if needed)

## Configuration

### Repository Standards

The flow automatically follows your repository's issue template standards:

1. **Template Location**: `.github/ISSUE_TEMPLATE/`
2. **Frontmatter Format**: YAML frontmatter with `name`, `about`, `title`, `labels`, `assignees`
3. **Body Format**: Markdown content following your template structure

### Customization

You can customize the flow by:

1. **Adding New Issue Types**: Create new templates in `.github/ISSUE_TEMPLATE/`
2. **Modifying Labels**: Update template frontmatter or provide additional labels
3. **Changing Title Formats**: Modify the `format_title` step in the flow

## Example Output

When the flow completes successfully, you'll see:

```
✅ GitHub Issue Created Successfully

- Issue #: 42
- URL: https://github.com/Cannasol-Tech/multi-sonicator-io/issues/42
- Repository: Cannasol-Tech/multi-sonicator-io
- Title: [BUG] Live Monitoring Display Shows GREEN When Hardware Not Configured
- Template Used: .github/ISSUE_TEMPLATE/bug_report.md

The issue has been created following your repository's issue template standards.
```

## Error Handling

The flow includes error handling for:
- Missing issue templates (falls back to bug_report.md)
- Invalid repository names
- GitHub API failures
- Network connectivity issues

## Quality Gates

The flow enforces quality gates for:
- **Template Validation**: Ensures issue template exists and is valid
- **Title Format**: Verifies title follows repository naming conventions
- **Label Compliance**: Checks labels match repository standards

## Related Files

- **Flow Definition**: `.axovia-flow/flows/create-github-issue.yaml`
- **Configuration**: `.axovia-flow/flow-config.yaml` (tasks section)
- **Issue Templates**: `.github/ISSUE_TEMPLATE/*.md`
- **Example Issues**: `.github/issues_to_create/*.md`

## Troubleshooting

### Common Issues

1. **Template Not Found**: Ensure your issue template exists in `.github/ISSUE_TEMPLATE/`
2. **GitHub API Errors**: Check your GitHub token permissions and repository access
3. **Invalid Repository**: Verify the repository name format is `owner/repo`

### Debug Mode

To debug the flow, check the following variables:
- `template_path`: Path to the loaded template
- `final_title`: Formatted issue title
- `final_labels`: Combined label list
- `final_body`: Generated issue body

---

*This flow is part of the Axovia Flow™ Agentic Framework for the Multi-Sonicator-IO project.*
