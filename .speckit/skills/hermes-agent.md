# Hermes Agent Skill Authoring
# Use when building new SKILL.md files for your project.
# Provides templates, validation, and best practices for skill creation.

---
name: hermes-agent-skill-authoring
category: software-development
description: "Author in-repo SKILL.md files: frontmatter and structure"
version: 1.0.0
author: Hermes Agent Team
tags: [hermes, skills, authoring, templates]

## Overview

This skill helps you create well-structured `SKILL.md` files for your Hermes Agent projects. It ensures consistency across all your custom skills with proper YAML frontmatter and markdown body formatting.

## Quick Start

```bash
# Create a new skill file using the template
hermes-skill-create my-custom-skill

# Or manually create in .speckit/skills/ directory
mkdir -p .speckit/skills/my-category
cat > .speckit/skills/my-category/SKILL.md << 'EOF'
# My Custom Skill
Description here...

---
name: my-custom-skill
category: my-category
description: "Short description"
version: 1.0.0
tags: [tag1, tag2]
EOF
```

## SKILL.md Structure Template

### YAML Frontmatter (Required)

```yaml
---
name: <skill-name>                    # Unique identifier (lowercase, hyphens/underscores)
category: <category-subdir>          # e.g., "devops", "web", "hermes-agent"
description: "<one-line summary>"    # First 57 chars should be self-contained trigger
version: 1.0.0                       # Semantic versioning
author: "<your name or org>"         # Optional
tags: [tag1, tag2]                   # Optional: for discovery and filtering
---
```

### Markdown Body (Required)

#### Required Sections

1. **Overview** - Brief introduction to what the skill does
2. **Quick Start** - Minimal example to get started
3. **Usage Examples** - Real-world usage scenarios
4. **API Reference** - If applicable, document commands/arguments
5. **Related Skills** - Links to complementary skills

#### Optional Sections

- **Configuration** - Default settings and options
- **Performance Notes** - Any known performance characteristics
- **Known Issues** - Current limitations or bugs
- **Changelog** - Version history
- **Contributing** - How to contribute to the skill

## Example: Complete Skill File

```markdown
# GitHub PR Review Helper
# Use when reviewing pull requests with automated assistance.

---
name: github-pr-review-helper
category: github
description: "Automated PR review assistance and checklist generation"
version: 1.0.2
author: Your Name
tags: [github, pr, review, automation]
---

## Overview

This skill provides automated assistance for reviewing GitHub pull requests, including code quality checks, documentation verification, and change summary generation.

## Quick Start

```bash
# Review a PR with the helper
hermes-skill-run github-pr-review-helper --args='{"pr_number": 123}'

# Generate review checklist
hermes-skill-create-checklist --repo="owner/repo" --pr=123
```

## Usage Examples

### Basic Review

```bash
# Run full review suite
hermes-skill-run github-pr-review-helper \
  --args='{"pr_number": 456, "depth": "full"}'
```

### Quick Check

```bash
# Just check code quality
hermes-skill-run github-pr-review-helper \
  --args='{"pr_number": 456, "checks": ["code-quality"]}'
```

## API Reference

### Arguments

| Name | Type | Required | Default | Description |
|------|------|----------|---------|-------------|
| pr_number | integer | Yes | - | The PR number to review |
| depth | string | No | "standard" | Review depth: "quick", "standard", "full" |
| checks | array | No | ["all"] | Specific checks to run |

## Related Skills

- `github-code-review`: Detailed diff analysis and inline comments
- `requesting-code-review`: Pre-commit review gates
- `hermes-agent-skill-authoring`: Creating new skills

## Changelog

### 1.0.2 (2024-09-01)
- Added: Quick check mode for fast reviews
- Fixed: Improved error handling for invalid PR numbers

### 1.0.1 (2024-08-15)
- Added: Code quality metrics reporting
- Changed: Updated default review depth to "standard"

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests with `hermes-skill-test github-pr-review-helper`
5. Submit a pull request
```

## Validation Checklist

Before publishing a new skill, verify:

- [ ] YAML frontmatter is valid and complete
- [ ] Name follows naming conventions (lowercase, hyphens)
- [ ] Description fits within 57 characters for quick reference
- [ ] At least one usage example provided
- [ ] Related skills are documented
- [ ] Version number follows semantic versioning

## Common Patterns

### Pattern: Command with Arguments

```yaml
commands:
  - name: my-command
    description: "Command description"
    handler: my-handler-function
    arguments:
      - name: arg1
        required: true
        type: string
```

### Pattern: Event-Driven Skill

```yaml
listeners:
  - event: push
    action: |
      # Your event handling code here
      echo "Push detected!"
```

## Tips & Best Practices

1. **Keep descriptions concise** - The first line should explain the trigger
2. **Use consistent naming** - Follow existing patterns in your project
3. **Document everything** - Even simple skills benefit from examples
4. **Version incrementally** - Bump version for each meaningful change
5. **Test before publishing** - Use `hermes-skill-test` to validate

## See Also

- [Hermes Agent Documentation](https://hermes-agent.nousresearch.com/docs)
- [Skill Authoring Guide](./hermes-agent-skill-authoring.md)
- [GitHub Skills Reference](../.speckit/skills/github-auth.md)
