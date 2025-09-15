#!/usr/bin/env python3
"""
Documentation Validation Script

Validates completeness and consistency of project documentation
per BMad-Core standards and embedded development requirements.
"""

import os
import sys
from pathlib import Path
import re

def validate_prd_completeness():
    """Validate that PRD contains all required sections."""
    project_root = Path(__file__).parent.parent
    prd_file = project_root / "docs" / "prd.md"
    
    if not prd_file.exists():
        print("❌ PRD file not found")
        return False
    
    print("🔍 Validating PRD completeness...")
    
    required_sections = [
        "# Sonicator Multiplexer Product Requirements Document",
        "## 1. Goals and Background Context",
        "## 2. Requirements", 
        "### 2.1. Functional Requirements",
        "### 2.2. Non-Functional Requirements",
        "## 3. Technical Assumptions",
        "## 4. Out of Scope",
        "## 5. Epic List",
        "## 6. Success Metrics",
        "## 7. Timeline and Milestones",
        "## 8. Risks and Mitigations"
    ]
    
    content = prd_file.read_text()
    missing_sections = []
    
    for section in required_sections:
        if section not in content:
            missing_sections.append(section)
        else:
            print(f"✅ Found: {section}")
    
    if missing_sections:
        print(f"\n❌ PRD validation FAILED")
        print(f"Missing sections ({len(missing_sections)}):")
        for section in missing_sections:
            print(f"  - {section}")
        return False
    else:
        print("✅ PRD completeness validation PASSED")
        return True

def validate_architecture_completeness():
    """Validate that Architecture document contains required sections."""
    project_root = Path(__file__).parent.parent
    arch_file = project_root / "docs" / "architecture" / "architecture.md"
    
    if not arch_file.exists():
        print("❌ Architecture file not found")
        return False
    
    print("🔍 Validating Architecture document completeness...")
    
    required_sections = [
        "# Multi-Sonicator I/O Controller Embedded Architecture Document",
        "## 1. Introduction",
        "## System Architecture",
        "## Firmware Architecture",
        "### 1. Hardware Abstraction Layer (HAL)",
        "### 2. Communication Layer",
        "### 3. Sonicator Control Logic"
    ]
    
    content = arch_file.read_text()
    missing_sections = []
    
    for section in required_sections:
        if section not in content:
            missing_sections.append(section)
        else:
            print(f"✅ Found: {section}")
    
    if missing_sections:
        print(f"\n❌ Architecture validation FAILED") 
        print(f"Missing sections ({len(missing_sections)}):")
        for section in missing_sections:
            print(f"  - {section}")
        return False
    else:
        print("✅ Architecture completeness validation PASSED")
        return True

def validate_story_structure():
    """Validate that story files follow proper structure."""
    project_root = Path(__file__).parent.parent
    stories_dir = project_root / "docs" / "agile" / "stories"
    
    if not stories_dir.exists():
        print("❌ Stories directory not found")
        return False
    
    print("🔍 Validating story file structure...")
    
    story_files = list(stories_dir.glob("*.md"))
    
    if not story_files:
        print("⚠️  No story files found")
        return True
    
    required_story_sections = [
        "## Story",
        "## Epic Context", 
        "## Story Context",
        "## Acceptance Criteria",
        "## Definition of Done",
        "## Dependencies"
    ]
    
    valid_stories = 0
    
    for story_file in story_files:
        print(f"  📄 Checking: {story_file.name}")
        content = story_file.read_text()
        
        missing_sections = []
        for section in required_story_sections:
            if section not in content:
                missing_sections.append(section)
        
        if missing_sections:
            print(f"    ❌ Missing sections: {', '.join(missing_sections)}")
        else:
            print(f"    ✅ Story structure complete")
            valid_stories += 1
    
    print(f"\n📊 Story validation: {valid_stories}/{len(story_files)} stories valid")
    
    if valid_stories == len(story_files):
        print("✅ Story structure validation PASSED")
        return True
    else:
        print("❌ Story structure validation FAILED")
        return False

def validate_testing_standards():
    """Validate testing standards documentation exists and is complete."""
    project_root = Path(__file__).parent.parent
    testing_standard = project_root / "docs" / "sop" / "sw-testing.md"
    
    if not testing_standard.exists():
        print("❌ Software testing standard not found")
        return False
    
    print("🔍 Validating testing standards documentation...")
    
    required_sections = [
        "# Software Testing Official Organization-Wide Standards",
        "## Universal Testing Strategy",
        "### Three-Stage Foundation",
        "## Standardized Make Targets",
        "## BMad-Core Integration"
    ]
    
    content = testing_standard.read_text()
    missing_sections = []
    
    for section in required_sections:
        if section not in content:
            missing_sections.append(section)
        else:
            print(f"✅ Found: {section}")
    
    if missing_sections:
        print(f"\n❌ Testing standards validation FAILED")
        print(f"Missing sections ({len(missing_sections)}):")
        for section in missing_sections:
            print(f"  - {section}")
        return False
    else:
        print("✅ Testing standards validation PASSED")
        return True

def validate_readme_completeness():
    """Validate README contains essential project information."""
    project_root = Path(__file__).parent.parent
    readme_file = project_root / "README.md"
    
    if not readme_file.exists():
        print("❌ README.md not found")
        return False
    
    print("🔍 Validating README completeness...")
    
    required_elements = [
        "# Multi-Sonicator I/O Controller",
        "## Overview",
        "## Quick Start", 
        "## Documentation"
    ]
    
    content = readme_file.read_text()
    missing_elements = []
    
    for element in required_elements:
        if element not in content:
            missing_elements.append(element)
        else:
            print(f"✅ Found: {element}")
    
    if missing_elements:
        print(f"\n❌ README validation FAILED")
        print(f"Missing elements ({len(missing_elements)}):")
        for element in missing_elements:
            print(f"  - {element}")
        return False
    else:
        print("✅ README completeness validation PASSED")
        return True

def main():
    """Main documentation validation function."""
    print("📚 Multi-Sonicator I/O Controller - Documentation Validation")
    print("=" * 60)
    
    validations = [
        validate_prd_completeness,
        validate_architecture_completeness,
        validate_story_structure,
        validate_testing_standards,
        validate_readme_completeness
    ]
    
    results = []
    for validation in validations:
        results.append(validation())
        print()  # Add spacing between validations
    
    # Summary
    passed = sum(results)
    total = len(results)
    
    print("=" * 60)
    print(f"📊 DOCUMENTATION VALIDATION SUMMARY: {passed}/{total} checks passed")
    
    if all(results):
        print("🎉 All documentation validations PASSED - Docs are compliant!")
        sys.exit(0)
    else:
        print("💥 Some documentation validations FAILED - Please fix the issues above")
        sys.exit(1)

if __name__ == "__main__":
    main()
