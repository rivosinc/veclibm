<!--
SPDX-FileCopyrightText: (c) Meta Platforms, Inc. and affiliates.

SPDX-License-Identifier: MIT
-->
# Contributing to veclibm

We welcome contributions to veclibm! This document provides guidelines for
contributing to this project.

## Getting Started

1. Fork the repository on GitHub.
2. Clone your fork locally.
3. Create a new branch for your changes.
4. Make your changes and commit them with clear, descriptive messages.
5. Push your changes to your fork.
6. Open a pull request against the `main` branch.

## Building

See the [README](README.md) for build and test instructions.

## Code Style

This project uses `clang-format` for C/C++ code formatting. Please run
`clang-format` on your changes before submitting a pull request. A
`.clang-format` configuration file is included in the repository.

Pre-commit hooks are available via `.pre-commit-config.yaml` to help enforce
formatting automatically.

## Licensing

This project is licensed under the MIT License. All new source files must
include the appropriate SPDX copyright and license headers:

```
// SPDX-FileCopyrightText: 2025 Rivos Inc.
//
// SPDX-License-Identifier: MIT
```

This project follows the [REUSE specification](https://reuse.software/) for
license compliance. You can verify compliance locally by running:

```
reuse lint
```

## Reporting Issues

Please use GitHub Issues to report bugs or request features.

## Code of Conduct

Please read our [Code of Conduct](CODE_OF_CONDUCT.md) before contributing.
