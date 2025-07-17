#!/usr/bin/env python3
"""
Generic Documentation Generation Tool
Automatically generates documentation from source code and configuration files
"""

import os
import sys
import json
import yaml
import subprocess
import argparse
from pathlib import Path
from datetime import datetime
import re

class DocumentationGenerator:
    def __init__(self, config_path=None, project_path=None):
        """
        Initialize documentation generator
        
        Args:
            config_path: Path to configuration file
            project_path: Path to project root directory
        """
        self.project_path = Path(project_path) if project_path else Path.cwd()
        self.config = self.load_config(config_path)
        self.generated_docs = []
        
    def load_config(self, config_path):
        """Load configuration from file or use defaults"""
        default_config = {
            'project': {
                'name': 'Project Documentation',
                'version': '1.0.0',
                'description': 'Auto-generated project documentation',
                'author': 'Automation Suite'
            },
            'sources': {
                'code_directories': ['src', 'lib', 'include'],
                'documentation_directories': ['docs'],
                'config_files': ['*.yml', '*.yaml', '*.json'],
                'exclude_patterns': ['*/.git/*', '*/node_modules/*', '*/__pycache__/*']
            },
            'output': {
                'directory': 'generated-docs',
                'formats': ['markdown', 'html'],
                'api_reference': True,
                'user_manual': True,
                'changelog': True
            },
            'templates': {
                'api_template': 'api_reference.md.j2',
                'manual_template': 'user_manual.md.j2',
                'readme_template': 'README.md.j2'
            },
            'parsing': {
                'extract_comments': True,
                'extract_functions': True,
                'extract_classes': True,
                'extract_constants': True
            }
        }
        
        if config_path and Path(config_path).exists():
            with open(config_path, 'r') as f:
                if config_path.endswith('.yaml') or config_path.endswith('.yml'):
                    user_config = yaml.safe_load(f)
                else:
                    user_config = json.load(f)
            # Merge with defaults
            default_config.update(user_config)
            
        return default_config
    
    def scan_source_files(self):
        """Scan source files for documentation extraction"""
        source_files = []
        
        for directory in self.config['sources']['code_directories']:
            dir_path = self.project_path / directory
            if dir_path.exists():
                # Find source files
                patterns = ['*.cpp', '*.c', '*.h', '*.hpp', '*.py', '*.js', '*.ts']
                for pattern in patterns:
                    source_files.extend(dir_path.rglob(pattern))
        
        return source_files
    
    def extract_code_documentation(self, file_path):
        """Extract documentation from a source code file"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
        except UnicodeDecodeError:
            print(f"Warning: Could not read {file_path} (encoding issue)")
            return None
        
        file_info = {
            'path': str(file_path.relative_to(self.project_path)),
            'language': self.detect_language(file_path),
            'functions': [],
            'classes': [],
            'constants': [],
            'comments': [],
            'file_description': ''
        }
        
        # Extract file-level description
        file_description = self.extract_file_description(content, file_info['language'])
        if file_description:
            file_info['file_description'] = file_description
        
        # Extract functions
        if self.config['parsing']['extract_functions']:
            file_info['functions'] = self.extract_functions(content, file_info['language'])
        
        # Extract classes
        if self.config['parsing']['extract_classes']:
            file_info['classes'] = self.extract_classes(content, file_info['language'])
        
        # Extract constants
        if self.config['parsing']['extract_constants']:
            file_info['constants'] = self.extract_constants(content, file_info['language'])
        
        # Extract comments
        if self.config['parsing']['extract_comments']:
            file_info['comments'] = self.extract_comments(content, file_info['language'])
        
        return file_info
    
    def detect_language(self, file_path):
        """Detect programming language from file extension"""
        suffix = file_path.suffix.lower()
        language_map = {
            '.cpp': 'cpp',
            '.c': 'c',
            '.h': 'c',
            '.hpp': 'cpp',
            '.py': 'python',
            '.js': 'javascript',
            '.ts': 'typescript',
            '.ino': 'arduino'
        }
        return language_map.get(suffix, 'unknown')
    
    def extract_file_description(self, content, language):
        """Extract file-level description from comments"""
        if language in ['c', 'cpp', 'arduino']:
            # Look for /** ... */ or /* ... */ at the beginning
            match = re.search(r'/\*\*?(.*?)\*/', content, re.DOTALL)
            if match:
                return self.clean_comment_text(match.group(1))
        elif language == 'python':
            # Look for triple-quoted docstring at the beginning
            match = re.search(r'^\s*"""(.*?)"""', content, re.DOTALL | re.MULTILINE)
            if match:
                return self.clean_comment_text(match.group(1))
        
        return ''
    
    def extract_functions(self, content, language):
        """Extract function definitions and documentation"""
        functions = []
        
        if language in ['c', 'cpp', 'arduino']:
            # C/C++ function pattern
            pattern = r'(?:/\*\*(.*?)\*/\s*)?(\w+\s+)?(\w+)\s*\([^)]*\)\s*{'
            matches = re.finditer(pattern, content, re.DOTALL)
            
            for match in matches:
                doc = self.clean_comment_text(match.group(1)) if match.group(1) else ''
                return_type = match.group(2).strip() if match.group(2) else 'void'
                name = match.group(3)
                
                functions.append({
                    'name': name,
                    'return_type': return_type,
                    'documentation': doc,
                    'signature': match.group(0).split('{')[0].strip()
                })
        
        elif language == 'python':
            # Python function pattern
            pattern = r'def\s+(\w+)\s*\([^)]*\):\s*(?:\n\s*"""(.*?)""")?'
            matches = re.finditer(pattern, content, re.DOTALL)
            
            for match in matches:
                name = match.group(1)
                doc = self.clean_comment_text(match.group(2)) if match.group(2) else ''
                
                functions.append({
                    'name': name,
                    'documentation': doc,
                    'signature': match.group(0).split(':')[0].strip()
                })
        
        return functions
    
    def extract_classes(self, content, language):
        """Extract class definitions and documentation"""
        classes = []
        
        if language in ['c', 'cpp', 'arduino']:
            # C++ class pattern
            pattern = r'(?:/\*\*(.*?)\*/\s*)?class\s+(\w+)(?:\s*:\s*[^{]*)?{'
            matches = re.finditer(pattern, content, re.DOTALL)
            
            for match in matches:
                doc = self.clean_comment_text(match.group(1)) if match.group(1) else ''
                name = match.group(2)
                
                classes.append({
                    'name': name,
                    'documentation': doc,
                    'type': 'class'
                })
        
        elif language == 'python':
            # Python class pattern
            pattern = r'class\s+(\w+)(?:\([^)]*\))?:\s*(?:\n\s*"""(.*?)""")?'
            matches = re.finditer(pattern, content, re.DOTALL)
            
            for match in matches:
                name = match.group(1)
                doc = self.clean_comment_text(match.group(2)) if match.group(2) else ''
                
                classes.append({
                    'name': name,
                    'documentation': doc,
                    'type': 'class'
                })
        
        return classes
    
    def extract_constants(self, content, language):
        """Extract constant definitions"""
        constants = []
        
        if language in ['c', 'cpp', 'arduino']:
            # C/C++ define pattern
            pattern = r'#define\s+(\w+)\s+([^\n]+)'
            matches = re.finditer(pattern, content)
            
            for match in matches:
                name = match.group(1)
                value = match.group(2).strip()
                
                constants.append({
                    'name': name,
                    'value': value,
                    'type': 'define'
                })
        
        elif language == 'python':
            # Python constant pattern (uppercase variables)
            pattern = r'^([A-Z_][A-Z0-9_]*)\s*=\s*([^\n]+)'
            matches = re.finditer(pattern, content, re.MULTILINE)
            
            for match in matches:
                name = match.group(1)
                value = match.group(2).strip()
                
                constants.append({
                    'name': name,
                    'value': value,
                    'type': 'constant'
                })
        
        return constants
    
    def extract_comments(self, content, language):
        """Extract standalone comments"""
        comments = []
        
        if language in ['c', 'cpp', 'arduino']:
            # Single line comments
            pattern = r'//\s*(.+)'
            matches = re.finditer(pattern, content)
            
            for match in matches:
                text = match.group(1).strip()
                if len(text) > 10:  # Only meaningful comments
                    comments.append({
                        'text': text,
                        'type': 'single_line'
                    })
        
        elif language == 'python':
            # Python comments
            pattern = r'#\s*(.+)'
            matches = re.finditer(pattern, content)
            
            for match in matches:
                text = match.group(1).strip()
                if len(text) > 10:  # Only meaningful comments
                    comments.append({
                        'text': text,
                        'type': 'single_line'
                    })
        
        return comments[:10]  # Limit to first 10 comments
    
    def clean_comment_text(self, text):
        """Clean up comment text"""
        if not text:
            return ''
        
        # Remove comment markers and extra whitespace
        text = re.sub(r'\*+', '', text)  # Remove asterisks
        text = re.sub(r'^\s*[\*\-\+]\s*', '', text, flags=re.MULTILINE)  # Remove bullets
        text = '\n'.join(line.strip() for line in text.split('\n'))
        text = re.sub(r'\n\s*\n', '\n\n', text)  # Clean up multiple newlines
        
        return text.strip()
    
    def generate_api_reference(self, source_files_info):
        """Generate API reference documentation"""
        api_doc = f"# API Reference\n\n"
        api_doc += f"Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n"
        
        # Group by directory
        by_directory = {}
        for file_info in source_files_info:
            if not file_info:
                continue
            
            directory = str(Path(file_info['path']).parent)
            if directory not in by_directory:
                by_directory[directory] = []
            by_directory[directory].append(file_info)
        
        # Generate documentation for each directory
        for directory, files in sorted(by_directory.items()):
            if directory != '.':
                api_doc += f"## {directory}\n\n"
            
            for file_info in files:
                api_doc += f"### {Path(file_info['path']).name}\n\n"
                
                if file_info['file_description']:
                    api_doc += f"{file_info['file_description']}\n\n"
                
                # Classes
                if file_info['classes']:
                    api_doc += "#### Classes\n\n"
                    for cls in file_info['classes']:
                        api_doc += f"##### `{cls['name']}`\n\n"
                        if cls['documentation']:
                            api_doc += f"{cls['documentation']}\n\n"
                
                # Functions
                if file_info['functions']:
                    api_doc += "#### Functions\n\n"
                    for func in file_info['functions']:
                        signature = func.get('signature', func['name'])
                        api_doc += f"##### `{signature}`\n\n"
                        if func['documentation']:
                            api_doc += f"{func['documentation']}\n\n"
                
                # Constants
                if file_info['constants']:
                    api_doc += "#### Constants\n\n"
                    for const in file_info['constants']:
                        api_doc += f"- **`{const['name']}`**: `{const['value']}`\n"
                    api_doc += "\n"
                
                api_doc += "---\n\n"
        
        return api_doc
    
    def generate_user_manual(self, project_info):
        """Generate user manual"""
        manual = f"# {self.config['project']['name']} - User Manual\n\n"
        manual += f"Version: {self.config['project']['version']}\n\n"
        manual += f"Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n"
        
        manual += "## Overview\n\n"
        manual += f"{self.config['project']['description']}\n\n"
        
        manual += "## Installation\n\n"
        manual += "### Prerequisites\n\n"
        manual += "- Development environment setup\n"
        manual += "- Required dependencies\n\n"
        
        manual += "### Build Instructions\n\n"
        manual += "```bash\n"
        manual += "# Clone the repository\n"
        manual += "git clone <repository-url>\n"
        manual += "cd <project-directory>\n\n"
        manual += "# Build the project\n"
        manual += "make build\n"
        manual += "```\n\n"
        
        manual += "## Usage\n\n"
        manual += "### Basic Operations\n\n"
        manual += "Describe basic usage patterns here.\n\n"
        
        manual += "### Configuration\n\n"
        manual += "Configuration options and settings.\n\n"
        
        manual += "### Troubleshooting\n\n"
        manual += "Common issues and solutions.\n\n"
        
        return manual
    
    def generate_changelog(self):
        """Generate changelog from git history"""
        changelog = "# Changelog\n\n"
        changelog += f"Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n"
        
        try:
            # Get git log
            result = subprocess.run(
                ['git', 'log', '--oneline', '-20'],
                capture_output=True,
                text=True,
                cwd=self.project_path
            )
            
            if result.returncode == 0:
                changelog += "## Recent Changes\n\n"
                for line in result.stdout.strip().split('\n'):
                    if line.strip():
                        changelog += f"- {line}\n"
                changelog += "\n"
            else:
                changelog += "No git history available.\n\n"
                
        except Exception as e:
            changelog += f"Error generating changelog: {e}\n\n"
        
        return changelog
    
    def save_documentation(self, content, filename, format_type='markdown'):
        """Save documentation to file"""
        output_dir = self.project_path / self.config['output']['directory']
        output_dir.mkdir(exist_ok=True)
        
        if format_type == 'markdown':
            file_path = output_dir / f"{filename}.md"
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
        elif format_type == 'html':
            file_path = output_dir / f"{filename}.html"
            # Convert markdown to HTML (basic conversion)
            html_content = self.markdown_to_html(content)
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(html_content)
        
        self.generated_docs.append(str(file_path))
        return file_path
    
    def markdown_to_html(self, markdown_content):
        """Basic markdown to HTML conversion"""
        html = f"""<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>{self.config['project']['name']} Documentation</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 40px; }}
        h1, h2, h3, h4, h5, h6 {{ color: #333; }}
        code {{ background-color: #f4f4f4; padding: 2px 4px; border-radius: 3px; }}
        pre {{ background-color: #f4f4f4; padding: 10px; border-radius: 5px; overflow-x: auto; }}
    </style>
</head>
<body>
"""
        
        # Basic markdown conversion
        content = markdown_content
        content = re.sub(r'^# (.+)$', r'<h1>\1</h1>', content, flags=re.MULTILINE)
        content = re.sub(r'^## (.+)$', r'<h2>\1</h2>', content, flags=re.MULTILINE)
        content = re.sub(r'^### (.+)$', r'<h3>\1</h3>', content, flags=re.MULTILINE)
        content = re.sub(r'^#### (.+)$', r'<h4>\1</h4>', content, flags=re.MULTILINE)
        content = re.sub(r'^##### (.+)$', r'<h5>\1</h5>', content, flags=re.MULTILINE)
        content = re.sub(r'`([^`]+)`', r'<code>\1</code>', content)
        content = re.sub(r'\*\*([^*]+)\*\*', r'<strong>\1</strong>', content)
        content = re.sub(r'\n\n', '</p><p>', content)
        content = f'<p>{content}</p>'
        content = re.sub(r'<p></p>', '', content)
        
        html += content + "\n</body>\n</html>"
        return html
    
    def generate_all_documentation(self):
        """Generate complete documentation suite"""
        print("🔥 Starting documentation generation...")
        
        # Scan source files
        print("  Scanning source files...")
        source_files = self.scan_source_files()
        print(f"    Found {len(source_files)} source files")
        
        # Extract documentation from source files
        print("  Extracting documentation...")
        source_files_info = []
        for file_path in source_files:
            file_info = self.extract_code_documentation(file_path)
            if file_info:
                source_files_info.append(file_info)
        
        print(f"    Processed {len(source_files_info)} files")
        
        # Generate different types of documentation
        generated_files = []
        
        if self.config['output']['api_reference']:
            print("  Generating API reference...")
            api_doc = self.generate_api_reference(source_files_info)
            for format_type in self.config['output']['formats']:
                file_path = self.save_documentation(api_doc, 'api_reference', format_type)
                generated_files.append(file_path)
        
        if self.config['output']['user_manual']:
            print("  Generating user manual...")
            manual = self.generate_user_manual(source_files_info)
            for format_type in self.config['output']['formats']:
                file_path = self.save_documentation(manual, 'user_manual', format_type)
                generated_files.append(file_path)
        
        if self.config['output']['changelog']:
            print("  Generating changelog...")
            changelog = self.generate_changelog()
            for format_type in self.config['output']['formats']:
                file_path = self.save_documentation(changelog, 'changelog', format_type)
                generated_files.append(file_path)
        
        # Generate summary report
        summary = self.generate_summary_report(source_files_info, generated_files)
        self.save_documentation(summary, 'documentation_summary', 'markdown')
        
        print(f"\n✅ Documentation generation complete!")
        print(f"   Generated {len(generated_files)} documentation files")
        print(f"   Output directory: {self.config['output']['directory']}")
        
        return generated_files
    
    def generate_summary_report(self, source_files_info, generated_files):
        """Generate documentation summary report"""
        summary = f"# Documentation Generation Summary\n\n"
        summary += f"Generated on {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n"
        
        # Statistics
        total_files = len(source_files_info)
        total_functions = sum(len(f['functions']) for f in source_files_info if f)
        total_classes = sum(len(f['classes']) for f in source_files_info if f)
        total_constants = sum(len(f['constants']) for f in source_files_info if f)
        
        summary += "## Statistics\n\n"
        summary += f"- **Source files processed**: {total_files}\n"
        summary += f"- **Functions documented**: {total_functions}\n"
        summary += f"- **Classes documented**: {total_classes}\n"
        summary += f"- **Constants documented**: {total_constants}\n"
        summary += f"- **Documentation files generated**: {len(generated_files)}\n\n"
        
        # Generated files
        summary += "## Generated Files\n\n"
        for file_path in generated_files:
            summary += f"- {Path(file_path).name}\n"
        summary += "\n"
        
        # Coverage by file
        summary += "## Coverage by File\n\n"
        for file_info in source_files_info:
            if not file_info:
                continue
            
            file_name = Path(file_info['path']).name
            items = len(file_info['functions']) + len(file_info['classes']) + len(file_info['constants'])
            summary += f"- **{file_name}**: {items} documented items\n"
        
        return summary

def main():
    parser = argparse.ArgumentParser(description='Generic Documentation Generation Tool')
    parser.add_argument('--config', '-c', help='Configuration file path')
    parser.add_argument('--project-path', '-p', help='Project root directory', default='.')
    parser.add_argument('--output-dir', '-o', help='Output directory', default='generated-docs')
    parser.add_argument('--formats', nargs='+', choices=['markdown', 'html'], 
                       default=['markdown'], help='Output formats')
    parser.add_argument('--api-only', action='store_true', help='Generate API reference only')
    parser.add_argument('--manual-only', action='store_true', help='Generate user manual only')
    
    args = parser.parse_args()
    
    # Initialize generator
    generator = DocumentationGenerator(
        config_path=args.config,
        project_path=args.project_path
    )
    
    # Override output directory if specified
    if args.output_dir != 'generated-docs':
        generator.config['output']['directory'] = args.output_dir
    
    # Override formats if specified
    generator.config['output']['formats'] = args.formats
    
    # Override document types if specified
    if args.api_only:
        generator.config['output']['api_reference'] = True
        generator.config['output']['user_manual'] = False
        generator.config['output']['changelog'] = False
    elif args.manual_only:
        generator.config['output']['api_reference'] = False
        generator.config['output']['user_manual'] = True
        generator.config['output']['changelog'] = False
    
    # Generate documentation
    generated_files = generator.generate_all_documentation()
    
    print(f"\nGenerated files:")
    for file_path in generated_files:
        print(f"  {file_path}")

if __name__ == '__main__':
    main()