# 📋 Feature Mapping: Epics to Agile Features

**Cross-reference between current Epic structure and Agile Planning Workflow features**

---

## 🔗 **Epic to Feature Mapping**

### **Feature 1: HIL Testing Framework & Hardware Verification**
- **Epic Reference**: `docs/epics/epic-0-hil-testing-framework-hardware-verification.md`
- **Stories**: 9 stories (0.1-0.9)
- **Status**: 11% complete (1/9 stories)
- **Priority**: Critical - Foundation for all development

### **Feature 2: Foundational Control & Communication**  
- **Epic Reference**: `docs/epics/epic-1-foundational-control-communication.md`
- **Stories**: 4 stories (1.1-1.4)
- **Status**: 25% complete (1/4 stories)
- **Priority**: Critical - Core functionality

### **Feature 3: Testing Infrastructure**
- **Epic Reference**: `docs/epics/epic-2-testing-infrastructure.md`
- **Stories**: 1 story (2.0)
- **Status**: 0% complete
- **Priority**: High - Quality assurance

### **Feature 4: Web UI Development**
- **Epic Reference**: `docs/epics/epic-3-web-ui-development.md`  
- **Stories**: 5 stories (3.0-3.4)
- **Status**: 0% complete (3.0 at 95% but not passed QA)
- **Priority**: Medium-High - User interface

### **Feature 5: Multi-Sonicator Coordination**
- **Epic Reference**: `docs/epics/epic-4-multi-sonicator-coordination.md`
- **Stories**: 4 stories (4.1-4.4)
- **Status**: 0% complete
- **Priority**: High - Advanced functionality

### **Feature 6: Production Deployment**
- **Epic Reference**: `docs/epics/epic-5-production-deployment-operations.md`
- **Stories**: 1 story (5.1)
- **Status**: 0% complete
- **Priority**: Medium - Operations

---

## 📊 **User Story Mapping**

### **Current Structure** → **Agile Workflow Structure**

```bash
# Current: docs/agile/stories/[epic].[story]-[name].md
# Agile:   docs/user-stories/[feature_num]-[feature_name].md

docs/agile/stories/0.1.project-structure-cicd-pipeline.md
→ docs/user-stories/1-hil-testing-framework.md#story-0.1

docs/agile/stories/0.2.hil-test-harness-setup.md  
→ docs/user-stories/1-hil-testing-framework.md#story-0.2

docs/agile/stories/1.4.single-sonicator-control.md
→ docs/user-stories/2-foundational-control.md#story-1.4

docs/agile/stories/3.0.integration-story.md
→ docs/user-stories/4-web-ui-development.md#story-3.0
```

---

**Source of Truth**: `docs/agile/stories/STORY_STATUS_DASHBOARD.md`  
**Cross-Reference**: This mapping file
**Update Frequency**: After epic/story changes