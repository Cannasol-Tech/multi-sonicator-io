# PlatformIO Embedded Coding Standard

> **Note:** This document defines the coding standards for all PlatformIO embedded applications within the organization.  This document is meant to be used in conjunction with the `coding-style.md` document.  This document will focus on PlatformIO specific coding standards and the `coding-style.md` document will focus on general coding standards that apply to all programming languages.

## Overview

The fact that the project is a PlatformIO project guarantees that the project interacts with embedded hardware.  This project should be treated as an embedded project and best embedded software practices should be implemented when writing code.  This includes memory usage and optimization for hardware.  Hardware constraints for the specific device should be understood and clearly outlined in the documentation of the project.

## PlatformIO Specific Coding Standards

- Use PlatformIO specific naming conventions for embedded devices
- Use PlatformIO specific folder structures for embedded devices
- Use PlatformIO specific linting rules for embedded devices
- Use PlatformIO specific testing rules for embedded devices
- Use PlatformIO Testing Standard when writing tests for embedded projects. 
