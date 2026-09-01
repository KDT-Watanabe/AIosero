# GitHub Authentication Setup
# Use when setting up GitHub authentication: HTTPS tokens, SSH keys, gh CLI login.

---
name: github-auth
category: github
description: "GitHub auth setup: HTTPS tokens, SSH keys, gh CLI login"
version: 1.0.0
author: Hermes Agent Team
tags: [github, auth, ssh, cli]

## Overview

This skill provides a comprehensive guide and automation for setting up GitHub authentication in your development environment. It covers multiple authentication methods including HTTPS tokens, SSH keys, and the gh CLI.

## Quick Start

```bash
# Check current authentication status
gh auth status

# Login with browser (recommended for first-time setup)
gh auth login --web

# Or use a personal access token
gh auth login --with-token <<< "YOUR_TOKEN_HERE"

# Verify SSH key is added
gh auth status | grep -A 5 "ssh"
```

## Authentication Methods

### 1. HTTPS Token (Personal Access Token)

**Best for:** Most users, quick setup, CI/CD pipelines

#### Generate a Personal Access Token

1. Visit: https://github.com/settings/tokens
2. Click "New token (classic)"
3. Choose scopes:
   - `repo` - Full control of private repositories
   - `read:user`, `write:user` - User-level permissions
4. Copy the generated token

#### Login with Token

```bash
# Interactive login (prompts for token)
gh auth login --with-token

# Or pipe directly
echo "YOUR_TOKEN" | gh auth login --with-token
```

### 2. SSH Key

**Best for:** Git over SSH, multiple accounts, advanced users

#### Generate SSH Key (if not exists)

```bash
# Check if key exists
ls -la ~/.ssh/id_ed25519.pub

# Generate new key (recommended: ed25519)
ssh-keygen -t ed25519 -C "your_email@example.com"
# Press Enter to accept default path and comment

# Add to GitHub SSH keys
cat ~/.ssh/id_ed25519.pub | pbcopy  # macOS
cat ~/.ssh/id_ed25519.pub | clip     # Windows
```

#### Configure gh CLI for SSH

```bash
# Set the default git protocol to SSH
gh config set git_protocol ssh

# Verify configuration
gh config get git_protocol
```

### 3. gh CLI Login (OAuth)

**Best for:** Interactive experience, multiple accounts

```bash
# Full OAuth flow with browser
gh auth login --web

# Add a second account
gh auth login --add

# List all authenticated accounts
gh auth list
```

## Verification Commands

```bash
# Check authentication status
gh auth status

# Test HTTPS access
git ls-remote https://github.com/owner/repo.git

# Test SSH access (if using SSH)
git ls-remote git@github.com:owner/repo.git

# Verify gh CLI is configured
gh config get --all | head -20
```

## Troubleshooting

### Common Issues

#### Issue: "Not authenticated" after login

**Solution:**
```bash
# Re-login with fresh session
gh auth logout
gh auth login --web

# Or clear cached credentials
rm -rf ~/.config/gh/cached-credentials-*
```

#### Issue: SSH key not recognized

**Solution:**
```bash
# Check if GitHub accepts the key
ssh-add -l  # Ensure key is loaded in agent

# Add to ssh-agent if needed
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/id_ed25519

# Test connection
ssh -T git@github.com
```

#### Issue: Token expired or revoked

**Solution:**
```bash
# Logout and re-login
gh auth logout
gh auth login --with-token <<< "NEW_TOKEN"
```

## Configuration Reference

### gh CLI Config File Locations

| Platform | Default Path |
|----------|--------------|
| macOS/Linux | `~/.config/gh/` |
| Windows | `%APPDATA%\gh\` |

### Important Config Options

```bash
# Git protocol (HTTPS vs SSH)
gh config set git_protocol https  # or ssh

# Default user for gh commands
gh config set user.name "Your Name"
gh config set user.email "your@email.com"

# Browser for OAuth flow
gh config set browser chrome

# Telemetry (optional)
gh config set telemetry off
```

## Related Skills

- `github-pr-workflow`: PR lifecycle management after auth is set up
- `github-code-review`: Code review with authenticated access
- `hermes-agent-skill-authoring`: Creating new skills

## Quick Reference Card

```bash
# === AUTHENTICATION FLOW ===

# 1. Check status first
gh auth status

# 2. Login (choose method)
gh auth login --web          # Browser (recommended)
gh auth login --with-token   # Token (CI/CD)
eval "$(ssh-agent -s)" && ssh-add ~/.ssh/id_ed25519  # SSH

# 3. Verify
gh auth status | grep "Logged in"

# === DAILY COMMANDS ===

# Quick check if authenticated
gh auth status | grep -q "Logged in" && echo "✓ Authenticated!" || echo "✗ Need to login"

# List all configured accounts
gh auth list

# Switch between accounts (if multiple)
gh auth switch <account-id>
```

## See Also

- [GitHub CLI Documentation](https://cli.github.com/manual/gh_auth_login)
- [Personal Access Tokens Guide](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/managing-your-personal-access-tokens)
- [SSH Keys for GitHub](https://docs.github.com/en/authentication/connecting-to-github-with-ssh)
