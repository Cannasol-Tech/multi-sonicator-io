# File Header Standards for different File Types that an Axovia AI Agent might work with

## YAML File

```yaml
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the file.
< *Version: 1.0
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

## C, C++, C#, Java, JavaScript, TypeScript, Python, Ruby, Swift, Kotlin, Go, Rust, PHP, FILE

> *Doxygen is the standard for C, C++, C#, Java, JavaScript, TypeScript, Python, Ruby, Swift, Kotlin, Go, Rust, PHP*
>
> *Note: All of the following fields do not need to be used - use only as needed.*
>
> **Required Fields*: `@file`, `@author`, `@date`, `@brief`, `@version`*

```c
/**
< *@file TITLE_OF_FILE
< *@author Stephen Boyett
< *@date 2023-08-25
< *@brief Brief description of the file.
< *@details More detailed description of the file.
< *@note Any important notes about the file.
< *@warning Any important warnings about the file.
< *@todo Any tasks that need to be done.
< *@bug Any bugs that need to be fixed.
< *@see Any related files or documentation.
< *@version 1.0
< *@since 1.0
< *@sa Any additional information.
< *@deprecated Any deprecation information.
 */
 ```

## HTML File

```html
<!--
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the HTML file.
< *Version: 1.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-->
```

## CSS File

```css
/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the CSS file.
< *Version: 1.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
```

## SQL File

```sql
-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the SQL file.
< *Version: 1.0
-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

## Shell Script (Bash, Zsh, etc.)

```bash
#!/bin/bash
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the shell script.
< *Version: 1.0
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

## PowerShell Script

```powershell
<#
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the PowerShell script.
< *Version: 1.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#>
```

## Markdown File

```markdown
<!--
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the Markdown file.
< *Version: 1.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-->
```

## JSON File

```json
{
  "_header": {
    "title": "TITLE_OF_FILE",
    "author": "Stephen Boyett",
    "date": "2023-08-25",
    "brief": "Brief description of the JSON file.",
    "version": "1.0"
  }
}
```

## XML File

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!--
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the XML file.
< *Version: 1.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-->
```

## Dockerfile

```dockerfile
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the Dockerfile.
< *Version: 1.0
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

## Docker Compose File

```yaml
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the Docker Compose file.
< *Version: 1.0
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

version: '3.8'
```

## Makefile

```makefile
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the Makefile.
< *Version: 1.0
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

## Configuration Files (.ini, .conf, .cfg)

```ini
; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the configuration file.
< *Version: 1.0
; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

## TOML File

```toml
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the TOML file.
< *Version: 1.0
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

## R Script

```r
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the R script.
< *Version: 1.0
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

## MATLAB Script

```matlab
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the MATLAB script.
< *Version: 1.0
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

## LaTeX File

```latex
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the LaTeX file.
< *Version: 1.0
% ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

## Jupyter Notebook (First Cell)

```python
"""
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
< *Title: TITLE_OF_FILE
< *Author: Stephen Boyett
< *Date: 2023-08-25
< *Brief: Brief description of the Jupyter notebook.
< *Version: 1.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
"""
```

## Usage Guidelines

### Required Fields

All file headers must include these minimum fields:

- **Title**: Clear, descriptive name of the file
- **Author**: Full name of the primary author
- **Date**: Creation date in YYYY-MM-DD format
- **Brief**: One-line description of the file's purpose
- **Version**: Semantic version number (e.g., 1.0, 2.1.3)

### Optional Fields (for Doxygen-compatible files)

- **Details**: Extended description when brief is insufficient
- **Note**: Important implementation notes
- **Warning**: Critical warnings for users/maintainers
- **Todo**: Outstanding tasks or improvements needed
- **Bug**: Known issues that need addressing
- **See**: References to related files or documentation
- **Since**: Version when feature was introduced
- **Deprecated**: Deprecation information and alternatives

### Best Practices

1. **Consistency**: Use the same format across all files in a project
2. **Brevity**: Keep descriptions concise but informative
3. **Updates**: Update version and date when making significant changes
4. **Relevance**: Only include optional fields that add value
5. **Automation**: Consider using IDE templates or snippets for consistency

### Version Guidelines

- **Major.Minor.Patch** format (e.g., 1.2.3)
- **Major**: Breaking changes or complete rewrites
- **Minor**: New features or significant enhancements
- **Patch**: Bug fixes or minor improvements

### Date Format

- Always use **YYYY-MM-DD** format for consistency
- Update when making significant changes to the file
- Consider using automated tools to maintain current dates
