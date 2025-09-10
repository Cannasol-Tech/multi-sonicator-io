/**
 * @file test_standards_compliance.cpp
 * @brief Unit tests for company standards compliance validation
 * 
 * Tests to ensure the project structure complies with Axovia Flow company standards
 * as defined in .axovia-flow/company-standards/project-structure.md
 * 
 * @author Multi-Sonicator I/O Controller Team
 * @date 2025-09-10
 * @version 1.0.0
 */

#include <unity.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <string>
#include <vector>

// Test configuration
const std::string PROJECT_ROOT = "./";
const std::string DOCS_PATH = "docs/";
const std::string SRC_PATH = "src/";
const std::string BIN_PATH = "bin/";
const std::string STANDARDS_PATH = ".axovia-flow/company-standards/";

/**
 * @brief Helper function to check if a file exists
 * @param filepath Path to file to check
 * @return true if file exists, false otherwise
 */
bool file_exists(const std::string& filepath) {
    struct stat buffer;
    return (stat(filepath.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
}

/**
 * @brief Helper function to check if a directory exists
 * @param dirpath Path to directory to check
 * @return true if directory exists, false otherwise
 */
bool dir_exists(const std::string& dirpath) {
    struct stat buffer;
    return (stat(dirpath.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
}

/**
 * @brief Helper function to count files in root directory
 * @return Number of regular files in root directory
 */
int count_root_files() {
    DIR* dir = opendir(PROJECT_ROOT.c_str());
    if (dir == nullptr) return -1;
    
    int count = 0;
    struct dirent* entry;
    struct stat file_stat;
    
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        std::string full_path = PROJECT_ROOT + entry->d_name;
        if (stat(full_path.c_str(), &file_stat) == 0 && S_ISREG(file_stat.st_mode)) {
            count++;
        }
    }
    
    closedir(dir);
    return count;
}

void setUp(void) {
    // Set up function, called before each test
}

void tearDown(void) {
    // Tear down function, called after each test
}

/**
 * @brief Test that all required root directory files exist
 */
void test_required_root_files_exist(void) {
    // Required root files per company standards
    std::vector<std::string> required_files = {
        ".gitignore",
        ".npmignore", 
        "Makefile",
        "LICENSE",
        "README.md"
    };
    
    for (const auto& file : required_files) {
        std::string filepath = PROJECT_ROOT + file;
        TEST_ASSERT_TRUE_MESSAGE(file_exists(filepath), 
            ("Required root file missing: " + file).c_str());
    }
}

/**
 * @brief Test that core directories exist
 */
void test_core_directories_exist(void) {
    // Required core directories
    std::vector<std::string> required_dirs = {
        DOCS_PATH,
        SRC_PATH,
        BIN_PATH
    };
    
    for (const auto& dir : required_dirs) {
        TEST_ASSERT_TRUE_MESSAGE(dir_exists(dir), 
            ("Required core directory missing: " + dir).c_str());
    }
}

/**
 * @brief Test that required docs subdirectories exist
 */
void test_docs_subdirectories_exist(void) {
    // Required docs subdirectories per company standards
    std::vector<std::string> required_subdirs = {
        "docs/architecture",
        "docs/planning",
        "docs/requirements",
        "docs/standards", 
        "docs/agile",
        "docs/agent-reports",
        "docs/testing"
    };
    
    for (const auto& dir : required_subdirs) {
        TEST_ASSERT_TRUE_MESSAGE(dir_exists(dir), 
            ("Required docs subdirectory missing: " + dir).c_str());
    }
}

/**
 * @brief Test that each docs subdirectory has a README.md
 */
void test_docs_subdirectory_readmes_exist(void) {
    std::vector<std::string> subdirs = {
        "docs/architecture",
        "docs/planning", 
        "docs/requirements",
        "docs/standards",
        "docs/agile",
        "docs/agent-reports",
        "docs/testing"
    };
    
    for (const auto& dir : subdirs) {
        std::string readme_path = dir + "/README.md";
        TEST_ASSERT_TRUE_MESSAGE(file_exists(readme_path), 
            ("Missing README.md in directory: " + dir).c_str());
    }
}

/**
 * @brief Test that standards files exist and are readable
 */
void test_standards_files_exist(void) {
    std::vector<std::string> standards_files = {
        "project-structure.md",
        "coding-style.md",
        "sw-testing-standard.md"
    };
    
    for (const auto& file : standards_files) {
        std::string filepath = STANDARDS_PATH + file;
        TEST_ASSERT_TRUE_MESSAGE(file_exists(filepath), 
            ("Standards file missing: " + file).c_str());
        
        // Test that file is readable and has content
        FILE* fp = fopen(filepath.c_str(), "r");
        TEST_ASSERT_NOT_NULL_MESSAGE(fp, 
            ("Cannot read standards file: " + file).c_str());
        
        if (fp) {
            // Check file has reasonable content (>100 chars)
            fseek(fp, 0, SEEK_END);
            long file_size = ftell(fp);
            fclose(fp);
            
            TEST_ASSERT_GREATER_THAN_MESSAGE(100, file_size, 
                ("Standards file too small (possible corruption): " + file).c_str());
        }
    }
}

/**
 * @brief Test that root directory doesn't contain prohibited files
 * Note: This is a warning-level check, not a hard failure
 */
void test_root_directory_cleanliness(void) {
    // Common prohibited files that should be in subdirectories
    std::vector<std::string> prohibited_files = {
        "package.json",
        "package-lock.json", 
        "requirements.txt",
        "pytest.ini",
        "behave.ini",
        "setup.cfg",
        "platformio.ini"
    };
    
    // Count violations for reporting
    int violations = 0;
    std::string violation_msg = "Prohibited files found in root: ";
    
    for (const auto& file : prohibited_files) {
        std::string filepath = PROJECT_ROOT + file;
        if (file_exists(filepath)) {
            violations++;
            violation_msg += file + " ";
        }
    }
    
    // This is a warning, not a hard failure for now
    // Can be changed to TEST_ASSERT_EQUAL_INT(0, violations) for stricter enforcement
    if (violations > 0) {
        TEST_MESSAGE(("WARNING: " + violation_msg + "(should be in config/ or appropriate subdirs)").c_str());
    }
}

/**
 * @brief Test that the project has proper version control setup
 */
void test_version_control_setup(void) {
    TEST_ASSERT_TRUE_MESSAGE(dir_exists(".git"), 
        "Project should be under Git version control");
    TEST_ASSERT_TRUE_MESSAGE(file_exists(".gitignore"), 
        ".gitignore file is required for version control");
}

/**
 * @brief Integration test - verify make targets work
 * This test checks that the standards validation can be run via make
 */
void test_make_targets_exist(void) {
    // This is a simplified check - in a full test environment,
    // we would actually execute the make targets
    TEST_ASSERT_TRUE_MESSAGE(file_exists("Makefile"), 
        "Makefile is required for standards validation");
    
    // Check that Makefile contains required targets
    FILE* makefile = fopen("Makefile", "r");
    TEST_ASSERT_NOT_NULL_MESSAGE(makefile, "Cannot read Makefile");
    
    if (makefile) {
        char line[1024];
        bool has_check_standards = false;
        bool has_update_standards = false;
        
        while (fgets(line, sizeof(line), makefile)) {
            if (strstr(line, "check-standards:") != nullptr) {
                has_check_standards = true;
            }
            if (strstr(line, "update-standards:") != nullptr) {
                has_update_standards = true;
            }
        }
        fclose(makefile);
        
        TEST_ASSERT_TRUE_MESSAGE(has_check_standards, 
            "Makefile missing required target: check-standards");
        TEST_ASSERT_TRUE_MESSAGE(has_update_standards, 
            "Makefile missing required target: update-standards");
    }
}

/**
 * @brief Main test runner
 */
int main(void) {
    UNITY_BEGIN();
    
    // Root directory compliance tests
    RUN_TEST(test_required_root_files_exist);
    RUN_TEST(test_root_directory_cleanliness);
    RUN_TEST(test_version_control_setup);
    
    // Core directory structure tests  
    RUN_TEST(test_core_directories_exist);
    RUN_TEST(test_docs_subdirectories_exist);
    RUN_TEST(test_docs_subdirectory_readmes_exist);
    
    // Standards files integrity tests
    RUN_TEST(test_standards_files_exist);
    
    // Integration tests
    RUN_TEST(test_make_targets_exist);
    
    return UNITY_END();
}
