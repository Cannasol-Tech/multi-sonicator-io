#!/usr/bin/env python3
"""
YAML to Markdown Template Renderer

Renders Markdown templates with YAML data as single source of truth.
Supports nested YAML structures and maintains clean separation.

Usage:
    python3 scripts/yaml_to_markdown.py config.yaml template.md output.md

Example YAML (config.yaml):
    project:
      name: "Multi-Sonicator"
      version: "1.0.0"
      features:
        - "Real-time control"
        - "Safety interlocks"

Example Template (template.md):
    # {{ project.name }} Documentation

    Version: {{ project.version }}

    ## Features

    {% for feature in project.features %}
    - {{ feature }}
    {% endfor %}
"""

import argparse
import os
import sys
from pathlib import Path
from typing import Any, Dict

import yaml
from jinja2 import Environment, FileSystemLoader, select_autoescape


class YamlMarkdownRenderer:
    """Renders Markdown templates with YAML data."""

    def __init__(self):
        self.env = Environment(
            loader=FileSystemLoader('.'),
            autoescape=select_autoescape(['html', 'xml']),
            trim_blocks=True,
            lstrip_blocks=True
        )

    def load_yaml(self, yaml_path: str) -> Dict[str, Any]:
        """Load YAML data from file."""
        try:
            with open(yaml_path, 'r', encoding='utf-8') as f:
                return yaml.safe_load(f) or {}
        except Exception as e:
            print(f"Error loading YAML {yaml_path}: {e}")
            sys.exit(1)

    def render_template(self, template_path: str, data: Dict[str, Any]) -> str:
        """Render template with data."""
        try:
            template = self.env.get_template(template_path)
            return template.render(**data)
        except Exception as e:
            print(f"Error rendering template {template_path}: {e}")
            sys.exit(1)

    def render(self, yaml_path: str, template_path: str, output_path: str):
        """Load YAML, render template, write output."""
        # Load data
        data = self.load_yaml(yaml_path)

        # Render template
        output = self.render_template(template_path, data)

        # Write output
        output_path = Path(output_path)
        output_path.parent.mkdir(parents=True, exist_ok=True)

        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(output)

        print(f"Rendered {template_path} with {yaml_path} -> {output_path}")


def main():
    parser = argparse.ArgumentParser(
        description='Render Markdown templates with YAML data'
    )
    parser.add_argument('yaml_file', help='YAML data file')
    parser.add_argument('template_file', help='Markdown template file')
    parser.add_argument('output_file', help='Output Markdown file')

    args = parser.parse_args()

    # Validate inputs
    if not os.path.exists(args.yaml_file):
        print(f"YAML file not found: {args.yaml_file}")
        sys.exit(1)

    if not os.path.exists(args.template_file):
        print(f"Template file not found: {args.template_file}")
        sys.exit(1)

    # Render
    renderer = YamlMarkdownRenderer()
    renderer.render(args.yaml_file, args.template_file, args.output_file)


if __name__ == '__main__':
    main()