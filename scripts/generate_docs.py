#!/usr/bin/env python3
"""
Documentation Generation Script for Multi-Sonicator CI/CD Pipeline
Generates API documentation, user manuals, and technical documentation
"""

import os
import sys
import json
import re
import subprocess
from datetime import datetime
import argparse

class DocumentationGenerator:
    def __init__(self, source_dir='.', output_dir='docs/generated'):
        self.source_dir = source_dir
        self.output_dir = output_dir
        self.version = self.get_version()
        
        # Create output directory
        os.makedirs(output_dir, exist_ok=True)
    
    def get_version(self):
        """Get version from config.h or default"""
        config_path = os.path.join(self.source_dir, 'include/config.h')
        if os.path.exists(config_path):
            with open(config_path, 'r') as f:
                content = f.read()
                match = re.search(r'#define\s+PROJECT_VERSION\s+"([^"]+)"', content)
                if match:
                    return match.group(1)
        return "1.0.0"
    
    def generate_api_documentation(self):
        """Generate API documentation from source code"""
        print("Generating API documentation...")
        
        api_docs = {
            'version': self.version,
            'generated': datetime.now().isoformat(),
            'modules': []
        }
        
        # Find all header files
        header_files = []
        for root, dirs, files in os.walk(self.source_dir):
            for file in files:
                if file.endswith('.h'):
                    header_files.append(os.path.join(root, file))
        
        # Parse header files
        for header_file in header_files:
            module_doc = self.parse_header_file(header_file)
            if module_doc:
                api_docs['modules'].append(module_doc)
        
        # Generate markdown documentation
        self.generate_api_markdown(api_docs)
        
        # Generate HTML documentation
        self.generate_api_html(api_docs)
        
        return api_docs
    
    def parse_header_file(self, header_file):
        """Parse a header file for API documentation"""
        try:
            with open(header_file, 'r') as f:
                content = f.read()
        except Exception as e:
            print(f"Error reading {header_file}: {e}")
            return None
        
        # Extract module information
        module_name = os.path.basename(header_file).replace('.h', '')
        
        # Extract file header comment
        file_doc = self.extract_file_documentation(content)
        
        # Extract classes and functions
        classes = self.extract_classes(content)
        functions = self.extract_functions(content)
        defines = self.extract_defines(content)
        
        if not classes and not functions and not defines:
            return None
        
        return {
            'name': module_name,
            'file': header_file,
            'documentation': file_doc,
            'classes': classes,
            'functions': functions,
            'defines': defines
        }
    
    def extract_file_documentation(self, content):
        """Extract file-level documentation"""
        # Look for file header comment
        match = re.search(r'/\*\*\s*\n\s*\*\s*@file[^*]*\*/\s*', content, re.MULTILINE)
        if match:
            doc = match.group(0)
            # Clean up the comment
            doc = re.sub(r'/\*\*|\*/|\s*\*\s*', '', doc)
            return doc.strip()
        return ""
    
    def extract_classes(self, content):
        """Extract class definitions and their documentation"""
        classes = []
        
        # Find class definitions
        class_pattern = r'/\*\*.*?\*/\s*class\s+(\w+)\s*\{([^}]*)\}'
        matches = re.finditer(class_pattern, content, re.MULTILINE | re.DOTALL)
        
        for match in matches:
            class_name = match.group(1)
            class_body = match.group(2)
            
            # Extract class documentation
            doc_match = re.search(r'/\*\*.*?\*/', match.group(0), re.DOTALL)
            class_doc = doc_match.group(0) if doc_match else ""
            
            # Extract methods
            methods = self.extract_methods(class_body)
            
            classes.append({
                'name': class_name,
                'documentation': class_doc,
                'methods': methods
            })
        
        return classes
    
    def extract_functions(self, content):
        """Extract function definitions and their documentation"""
        functions = []
        
        # Find function definitions
        function_pattern = r'/\*\*.*?\*/\s*[\w\s*]+\s+(\w+)\s*\([^)]*\)\s*[;{]'
        matches = re.finditer(function_pattern, content, re.MULTILINE | re.DOTALL)
        
        for match in matches:
            function_name = match.group(1)
            
            # Extract function documentation
            doc_match = re.search(r'/\*\*.*?\*/', match.group(0), re.DOTALL)
            function_doc = doc_match.group(0) if doc_match else ""
            
            # Extract function signature
            sig_match = re.search(r'([\w\s*]+\s+\w+\s*\([^)]*\))', match.group(0))
            signature = sig_match.group(1) if sig_match else ""
            
            functions.append({
                'name': function_name,
                'signature': signature,
                'documentation': function_doc
            })
        
        return functions
    
    def extract_methods(self, class_body):
        """Extract method definitions from class body"""
        methods = []
        
        # Find method definitions
        method_pattern = r'/\*\*.*?\*/\s*[\w\s*]+\s+(\w+)\s*\([^)]*\)\s*[;{]'
        matches = re.finditer(method_pattern, class_body, re.MULTILINE | re.DOTALL)
        
        for match in matches:
            method_name = match.group(1)
            
            # Extract method documentation
            doc_match = re.search(r'/\*\*.*?\*/', match.group(0), re.DOTALL)
            method_doc = doc_match.group(0) if doc_match else ""
            
            # Extract method signature
            sig_match = re.search(r'([\w\s*]+\s+\w+\s*\([^)]*\))', match.group(0))
            signature = sig_match.group(1) if sig_match else ""
            
            methods.append({
                'name': method_name,
                'signature': signature,
                'documentation': method_doc
            })
        
        return methods
    
    def extract_defines(self, content):
        """Extract #define statements"""
        defines = []
        
        # Find #define statements
        define_pattern = r'#define\s+(\w+)\s+([^\n]*)'
        matches = re.finditer(define_pattern, content)
        
        for match in matches:
            define_name = match.group(1)
            define_value = match.group(2).strip()
            
            defines.append({
                'name': define_name,
                'value': define_value
            })
        
        return defines
    
    def generate_api_markdown(self, api_docs):
        """Generate markdown API documentation"""
        output_file = os.path.join(self.output_dir, 'api_reference.md')
        
        with open(output_file, 'w') as f:
            f.write(f"# Multi-Sonicator API Reference\n\n")
            f.write(f"**Version:** {api_docs['version']}  \n")
            f.write(f"**Generated:** {api_docs['generated']}  \n\n")
            
            # Table of contents
            f.write("## Table of Contents\n\n")
            for module in api_docs['modules']:
                f.write(f"- [{module['name']}](#{module['name'].lower()})\n")
            f.write("\n")
            
            # Module documentation
            for module in api_docs['modules']:
                f.write(f"## {module['name']}\n\n")
                f.write(f"**File:** `{module['file']}`\n\n")
                
                if module['documentation']:
                    f.write(f"{module['documentation']}\n\n")
                
                # Classes
                if module['classes']:
                    f.write("### Classes\n\n")
                    for cls in module['classes']:
                        f.write(f"#### {cls['name']}\n\n")
                        f.write(f"{cls['documentation']}\n\n")
                        
                        if cls['methods']:
                            f.write("**Methods:**\n\n")
                            for method in cls['methods']:
                                f.write(f"- `{method['signature']}`\n")
                                if method['documentation']:
                                    f.write(f"  {method['documentation']}\n")
                            f.write("\n")
                
                # Functions
                if module['functions']:
                    f.write("### Functions\n\n")
                    for func in module['functions']:
                        f.write(f"#### {func['name']}\n\n")
                        f.write(f"```c\n{func['signature']}\n```\n\n")
                        f.write(f"{func['documentation']}\n\n")
                
                # Defines
                if module['defines']:
                    f.write("### Defines\n\n")
                    for define in module['defines']:
                        f.write(f"- `{define['name']}` = `{define['value']}`\n")
                    f.write("\n")
        
        print(f"API markdown documentation generated: {output_file}")
    
    def generate_api_html(self, api_docs):
        """Generate HTML API documentation"""
        output_file = os.path.join(self.output_dir, 'api_reference.html')
        
        html_content = f"""
        <!DOCTYPE html>
        <html>
        <head>
            <title>Multi-Sonicator API Reference</title>
            <style>
                body {{ font-family: Arial, sans-serif; margin: 20px; }}
                .header {{ background-color: #f0f0f0; padding: 20px; border-radius: 5px; }}
                .module {{ margin: 20px 0; border: 1px solid #ddd; border-radius: 5px; }}
                .module-header {{ background-color: #e0e0e0; padding: 10px; font-weight: bold; }}
                .content {{ padding: 10px; }}
                .class {{ margin: 10px 0; }}
                .method {{ margin-left: 20px; }}
                .function {{ margin: 10px 0; }}
                .define {{ margin-left: 20px; }}
                code {{ background-color: #f5f5f5; padding: 2px 4px; border-radius: 3px; }}
                pre {{ background-color: #f5f5f5; padding: 10px; border-radius: 5px; }}
            </style>
        </head>
        <body>
            <div class="header">
                <h1>Multi-Sonicator API Reference</h1>
                <p><strong>Version:</strong> {api_docs['version']}</p>
                <p><strong>Generated:</strong> {api_docs['generated']}</p>
            </div>
        """
        
        for module in api_docs['modules']:
            html_content += f"""
            <div class="module">
                <div class="module-header">{module['name']}</div>
                <div class="content">
                    <p><strong>File:</strong> <code>{module['file']}</code></p>
                    <p>{module['documentation']}</p>
            """
            
            # Classes
            if module['classes']:
                html_content += "<h3>Classes</h3>"
                for cls in module['classes']:
                    html_content += f"""
                    <div class="class">
                        <h4>{cls['name']}</h4>
                        <p>{cls['documentation']}</p>
                    """
                    
                    if cls['methods']:
                        html_content += "<h5>Methods:</h5>"
                        for method in cls['methods']:
                            html_content += f"""
                            <div class="method">
                                <code>{method['signature']}</code>
                                <p>{method['documentation']}</p>
                            </div>
                            """
                    
                    html_content += "</div>"
            
            # Functions
            if module['functions']:
                html_content += "<h3>Functions</h3>"
                for func in module['functions']:
                    html_content += f"""
                    <div class="function">
                        <h4>{func['name']}</h4>
                        <pre><code>{func['signature']}</code></pre>
                        <p>{func['documentation']}</p>
                    </div>
                    """
            
            # Defines
            if module['defines']:
                html_content += "<h3>Defines</h3>"
                for define in module['defines']:
                    html_content += f"""
                    <div class="define">
                        <code>{define['name']}</code> = <code>{define['value']}</code>
                    </div>
                    """
            
            html_content += "</div></div>"
        
        html_content += """
        </body>
        </html>
        """
        
        with open(output_file, 'w') as f:
            f.write(html_content)
        
        print(f"API HTML documentation generated: {output_file}")
    
    def generate_user_manual(self):
        """Generate user manual from markdown files"""
        print("Generating user manual...")
        
        # Find all markdown files in docs directory
        docs_dir = os.path.join(self.source_dir, 'docs')
        if not os.path.exists(docs_dir):
            print("No docs directory found")
            return
        
        markdown_files = []
        for root, dirs, files in os.walk(docs_dir):
            for file in files:
                if file.endswith('.md'):
                    markdown_files.append(os.path.join(root, file))
        
        # Combine markdown files into user manual
        manual_content = f"""# Multi-Sonicator User Manual

**Version:** {self.version}  
**Generated:** {datetime.now().isoformat()}

## Table of Contents

"""
        
        sections = []
        for md_file in sorted(markdown_files):
            section_title = os.path.basename(md_file).replace('.md', '').replace('_', ' ').title()
            sections.append((section_title, md_file))
            manual_content += f"- [{section_title}](#{section_title.lower().replace(' ', '-')})\n"
        
        manual_content += "\n"
        
        # Add sections
        for section_title, md_file in sections:
            manual_content += f"## {section_title}\n\n"
            try:
                with open(md_file, 'r') as f:
                    content = f.read()
                    # Remove the first header if it exists
                    content = re.sub(r'^#[^\n]*\n', '', content)
                    manual_content += content + "\n\n"
            except Exception as e:
                print(f"Error reading {md_file}: {e}")
        
        # Write user manual
        output_file = os.path.join(self.output_dir, 'user_manual.md')
        with open(output_file, 'w') as f:
            f.write(manual_content)
        
        print(f"User manual generated: {output_file}")
    
    def generate_build_info(self):
        """Generate build information documentation"""
        print("Generating build information...")
        
        build_info = {
            'version': self.version,
            'generated': datetime.now().isoformat(),
            'platform': os.uname().sysname if hasattr(os, 'uname') else 'Unknown',
            'python_version': sys.version,
            'git_info': self.get_git_info()
        }
        
        # Write build info
        output_file = os.path.join(self.output_dir, 'build_info.json')
        with open(output_file, 'w') as f:
            json.dump(build_info, f, indent=2)
        
        print(f"Build information generated: {output_file}")
    
    def get_git_info(self):
        """Get git repository information"""
        try:
            # Get git commit hash
            result = subprocess.run(['git', 'rev-parse', 'HEAD'], 
                                  capture_output=True, text=True)
            commit_hash = result.stdout.strip() if result.returncode == 0 else 'unknown'
            
            # Get git branch
            result = subprocess.run(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], 
                                  capture_output=True, text=True)
            branch = result.stdout.strip() if result.returncode == 0 else 'unknown'
            
            # Get git remote URL
            result = subprocess.run(['git', 'config', '--get', 'remote.origin.url'], 
                                  capture_output=True, text=True)
            remote_url = result.stdout.strip() if result.returncode == 0 else 'unknown'
            
            return {
                'commit': commit_hash,
                'branch': branch,
                'remote': remote_url
            }
        except Exception as e:
            return {'error': str(e)}

def main():
    parser = argparse.ArgumentParser(description='Generate documentation for Multi-Sonicator')
    parser.add_argument('--source', default='.', help='Source directory')
    parser.add_argument('--output', default='docs/generated', help='Output directory')
    parser.add_argument('--api-only', action='store_true', help='Generate only API documentation')
    parser.add_argument('--manual-only', action='store_true', help='Generate only user manual')
    
    args = parser.parse_args()
    
    generator = DocumentationGenerator(args.source, args.output)
    
    print("Multi-Sonicator Documentation Generator")
    print("=" * 40)
    
    if args.api_only:
        generator.generate_api_documentation()
    elif args.manual_only:
        generator.generate_user_manual()
    else:
        # Generate all documentation
        generator.generate_api_documentation()
        generator.generate_user_manual()
        generator.generate_build_info()
    
    print("Documentation generation complete!")
    return 0

if __name__ == "__main__":
    sys.exit(main())