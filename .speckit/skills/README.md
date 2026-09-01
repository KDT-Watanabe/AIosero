# .speckit/skills Directory

This directory contains custom **Hermes Agent Skills** for your project. Each skill provides reusable knowledge, commands, and workflows specific to your development environment.

## Directory Structure

```
.speckit/
├── config.yaml              # Main Speckit configuration
└── skills/                  # Custom skill files
    ├── hermes-agent.md      # Skill authoring guide (this one!)
    ├── github-auth.md       # GitHub authentication setup
    └── <your-skill>.md      # Your custom skills...
```

## Creating a New Skill

### 1. Create the File

```bash
# Navigate to skills directory
cd .speckit/skills

# Create new skill file
cat > my-custom-skill.md << 'EOF'
# My Custom Skill
Description here...

---
name: my-custom-skill
category: my-category
description: "Short description"
version: 1.0.0
tags: [tag1, tag2]
---

## Overview
...
EOF
```

### 2. YAML Frontmatter (Required)

```yaml
---
name: <unique-name>                    # Lowercase with hyphens/underscores
category: <subdirectory>              # e.g., "devops", "web"
description: "<one-line summary>"     # First ~57 chars should be self-contained
version: 1.0.0                        # Semantic versioning
author: Your Name                     # Optional
tags: [tag1, tag2]                    # Optional: for discovery/filtering
---
```

### 3. Markdown Body (Required)

**Minimum required sections:**

- **Overview** - What the skill does
- **Quick Start** - Minimal example to get started
- **Usage Examples** - Real-world scenarios
- **API Reference** - Commands/arguments (if applicable)
- **Related Skills** - Links to complementary skills

## Quick Reference

### Common Categories

| Category | Example Use Case |
|----------|------------------|
| `hermes-agent` | Hermes-specific utilities |
| `github` | GitHub workflow automation |
| `devops` | Deployment, CI/CD tasks |
| `web` | Web development helpers |
| `data-science` | Data analysis workflows |

### File Naming Conventions

```bash
# Good names:
my-custom-skill.md           # Clear and descriptive
github-pr-review-helper.md   # Specific purpose
hermes-agent-authoring.md    # Related to Hermes Agent

# Avoid:
skill1.md                    # Too vague
temp-skill.md                # Implies temporary
```

## Verification Checklist

Before publishing a new skill, verify:

- [ ] YAML frontmatter is valid and complete
- [ ] Name follows naming conventions (lowercase, hyphens)
- [ ] Description fits within ~57 characters for quick reference
- [ ] At least one usage example provided
- [ ] Related skills are documented
- [ ] Version number follows semantic versioning

## Tips & Best Practices

1. **Keep descriptions concise** - The first line should explain the trigger
2. **Use consistent naming** - Follow existing patterns in your project
3. **Document everything** - Even simple skills benefit from examples
4. **Version incrementally** - Bump version for each meaningful change
5. **Test before publishing** - Use `hermes-skill-test` to validate

## See Also

- [Hermes Agent Documentation](https://hermes-agent.nousresearch.com/docs)
- [Skill Authoring Guide](./hermes-agent.md)
- [GitHub Skills Reference](../.speckit/skills/github-auth.md)
