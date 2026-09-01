# Writing Plans Mode
# Use when you need to plan implementation before coding.

---
name: writing-plans
category: software-development
description: "Write implementation plans: bite-sized tasks, paths, code"
version: 1.0.0
author: Hermes Agent Team
tags: [planning, implementation, tasks]

## Overview

This skill helps you create detailed implementation plans before coding begins. It breaks down large features into manageable tasks with clear acceptance criteria and estimated effort.

## Quick Start

```bash
# Create a plan for a new feature
hermes-skill-create-plan "Add user authentication"

# Or manually in .hermes/plans/ directory
mkdir -p .hermes/plans
cat > .hermes/plans/auth-implementation.md << 'EOF'
# Plan: User Authentication Implementation
Date: 2024-09-01
Author: Your Name
---

## Overview
...
EOF
```

## Plan Structure Template

### YAML Frontmatter (Required)

```yaml
---
name: <plan-name>                    # e.g., "auth-implementation"
project: <project-name>             # e.g., "AIosero"
date: 2024-09-01                   # Creation date
author: Your Name                  # Optional
version: 1.0                       # Plan version (independent of code)
---
```

### Markdown Body (Required)

#### Required Sections

1. **Overview** - What this plan covers
2. **Prerequisites** - Dependencies, setup needed
3. **Tasks Breakdown** - Hierarchical task list with estimates
4. **Acceptance Criteria** - How to verify completion
5. **Risks & Considerations** - Potential issues and mitigations

#### Optional Sections

- **Architecture Decisions** - Key design choices
- **API Design** - If APIs are involved
- **Testing Strategy** - Test coverage goals
- **Rollback Plan** - How to undo if needed

## Example: Complete Plan File

```markdown
# Plan: User Authentication Implementation
Date: 2024-09-01
Author: Your Name
Version: 1.0

---

## Overview

This plan outlines the implementation of user authentication for AIosero, including registration, login, session management, and password reset functionality.

**Estimated Effort:** 3-5 days  
**Priority:** High  
**Dependencies:** Database schema changes, API endpoints

## Prerequisites

### Technical
- [ ] PostgreSQL database with auth schema created
- [ ] User table migration ready
- [ ] JWT library selected (e.g., `jsonwebtoken`)
- [ ] Password hashing utility configured (bcrypt/argon2)

### Environment
- [ ] Development environment set up
- [ ] Test data prepared (test users, tokens)
- [ ] API documentation draft created

## Tasks Breakdown

### Phase 1: Database Schema (Estimated: 4 hours)

#### Task 1.1: Create User Table Migration
**Effort:** 2 hours  
**Acceptance Criteria:**
- [ ] Migration file created at `migrations/xxxx_create_users_table.sql`
- [ ] Includes all required fields: email, password_hash, created_at, updated_at
- [ ] Index on email for fast lookups
- [ ] Soft delete support (deleted_at column)

**Files to Create:**
```bash
mkdir -p migrations
cat > migrations/20240901_create_users_table.sql << 'EOF'
-- Migration: Create users table
CREATE TABLE IF NOT EXISTS users (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  email VARCHAR(255) UNIQUE NOT NULL,
  password_hash VARCHAR(255) NOT NULL,
  created_at TIMESTAMPTZ DEFAULT NOW(),
  updated_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_users_email ON users(email);
EOF
```

#### Task 1.2: Create User Model
**Effort:** 1 hour  
**Acceptance Criteria:**
- [ ] Model file at `src/models/user.ts` (or equivalent)
- [ ] Includes validation rules for email and password
- [ ] Includes serialization methods (toJSON, toPlain)

### Phase 2: API Endpoints (Estimated: 8 hours)

#### Task 2.1: Implement Registration Endpoint
**Effort:** 3 hours  
**Acceptance Criteria:**
- [ ] POST `/api/auth/register` endpoint implemented
- [ ] Validates email format and uniqueness
- [ ] Hashes password before storing
- [ ] Returns user object with id, email, created_at

**Sample Request/Response:**
```bash
# Request
curl -X POST http://localhost:3000/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "email": "user@example.com",
    "password": "SecurePass123!"
  }'

# Response (201 Created)
{
  "id": "uuid-here",
  "email": "user@example.com",
  "created_at": "2024-09-01T12:00:00.000Z"
}
```

#### Task 2.2: Implement Login Endpoint
**Effort:** 3 hours  
**Acceptance Criteria:**
- [ ] POST `/api/auth/login` endpoint implemented
- [ ] Validates credentials against database
- [ ] Generates JWT token with appropriate expiration
- [ ] Returns user object + token in response

**Sample Request/Response:**
```bash
# Request
curl -X POST http://localhost:3000/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "email": "user@example.com",
    "password": "SecurePass123!"
  }'

# Response (200 OK)
{
  "id": "uuid-here",
  "email": "user@example.com",
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6...",
  "expires_at": "2024-09-01T20:00:00.000Z"
}
```

#### Task 2.3: Implement Password Reset Flow
**Effort:** 4 hours  
**Acceptance Criteria:**
- [ ] POST `/api/auth/forgot-password` - request reset email
- [ ] GET `/api/auth/reset-password/:token` - apply new password
- [ ] Email template created for reset instructions

### Phase 3: Session Management (Estimated: 6 hours)

#### Task 3.1: Implement Token Refresh
**Effort:** 2 hours  
**Acceptance Criteria:**
- [ ] POST `/api/auth/refresh` endpoint implemented
- [ ] Validates current token, issues new one
- [ ] Handles expired tokens gracefully

#### Task 3.2: Add Middleware for Protected Routes
**Effort:** 4 hours  
**Acceptance Criteria:**
- [ ] Authentication middleware created at `src/middleware/auth.ts`
- [ ] Attaches user to request context (e.g., `req.user`)
- [ ] Returns 401 with error message on invalid/missing token

## Acceptance Criteria

### Functional Requirements

#### Registration Flow
- [ ] New user can register with valid email and password
- [ ] Email format is validated before database insert
- [ ] Duplicate email returns 409 Conflict
- [ ] Password meets minimum complexity requirements (8 chars, 1 uppercase, 1 number)
- [ ] Password is hashed using bcrypt with cost factor 12

#### Login Flow
- [ ] Valid credentials return user + token
- [ ] Invalid credentials return 401 Unauthorized
- [ ] Token expires after configured duration (default: 7 days)
- [ ] Refresh endpoint extends session without re-authentication

### Non-Functional Requirements

- [ ] API response times < 200ms for happy path
- [ ] Database queries optimized with proper indexing
- [ ] Error messages are user-friendly but not revealing
- [ ] Rate limiting applied to login attempts (max 5 per minute)

## Risks & Considerations

### Technical Risks

| Risk | Impact | Mitigation |
|------|--------|------------|
| Database migration fails on production | High | Use database versioning, test on staging first |
| JWT token leakage in logs | Medium | Sanitize tokens before logging |
| Password hash collision (extremely rare) | Low | Use strong hashing algorithm with high cost factor |

### Security Considerations

- [ ] Implement rate limiting on auth endpoints
- [ ] Use HTTPS for all API communication
- [ ] Set secure cookie flags if using server-side sessions
- [ ] Consider adding "remember me" functionality with longer-lived tokens
- [ ] Plan for account lockout after repeated failed login attempts

## Testing Strategy

### Unit Tests
```bash
# Run auth-specific tests
npm test -- src/auth/*.test.ts

# Coverage report
npm run test:coverage -- src/auth/
```

### Integration Tests
```bash
# End-to-end auth flow tests
npm test -- e2e/auth.e2e.test.ts

# API contract tests with Postman/Newman
newman run e2e/api-contracts/postman-collection.json
```

## Rollback Plan

If issues arise after deployment:

1. **Quick rollback (minutes):**
   ```bash
   # Revert database migration
   psql -d app_db -c "ALTER TABLE users DROP COLUMN IF EXISTS deleted_at;"
   
   # Or restore from backup
   pg_restore -d app_db backups/pre-auth-2024-09-01.dump
   ```

2. **Feature flag toggle (if implemented):**
   ```bash
   # Disable auth feature via config
   curl -X PATCH http://localhost:3000/api/config \
     -H "Content-Type: application/json" \
     -d '{"auth.enabled": false}'
   ```

## Related Plans

- `database-schema-planning`: Database design and migrations
- `api-design-documentation`: API contract definitions
- `security-audit-checklist`: Security review checklist

## Changelog

### 1.0 (2024-09-01)
- Initial plan creation
- Added database schema tasks
- Added API endpoint specifications

## See Also

- [Authentication Best Practices](https://auth0.com/blog/a-look-at-the-best-authentication-practices/)
- [JWT Guide](https://jwt.io/introduction)
- [Bcrypt Cost Factor Recommendations](https://www.schneier.com/essays/Bcrypt_Cost_Factor_Recommendations.html)
