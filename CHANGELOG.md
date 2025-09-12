# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Fixed
- Fixed frequency precision display in web UI to show exactly two decimal places (e.g., "2.01 kHz" instead of "2.0100000000000002kHz")
- Implemented uniform sizing for parameter signal cards in Control Panel

### Changed
- Enhanced signal cards with responsive grid layout that automatically adjusts based on screen size
- Added overflow protection for card content to prevent text from breaking layouts
- Improved visual consistency across light and dark themes

### Technical
- Added `formatFrequency` utility usage throughout the Control Panel for consistent frequency formatting
- Implemented CSS Grid with `auto-fit` and `minmax()` for responsive parameter card layout
- Added text overflow and word-break CSS rules to prevent content overflow in cards
