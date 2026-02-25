# Mushroom Growing

## Overview
A collection of sub-projects related to building and managing a home mushroom cultivation operation. Each subfolder is an independent project focused on a specific aspect of the setup.

## Sub-Projects
- `setup/` - Planning and documenting the physical grow setup (space, equipment, environmental targets, workflow)
- `strains/` - Strain selection, materials planning, vendor sourcing, and per-batch grow tracking
- `controller-build-guide/` - Step-by-step construction guide for the DIY Martha Tent Controller (ESP32-S3, hardware assembly + firmware flash). Pure build guide — assumes parts selection and theory covered elsewhere. Inline explainers, checkpoints, and verification steps at every stage.

## Conventions
- These projects are domain-specific (mycology/horticulture) — include species names, substrate formulas, and environmental parameters precisely
- Prefer plain formats (Markdown, JSON, CSV) for data that needs to be human-readable
- Scripts should be runnable standalone with minimal dependencies

## When Working Here
1. Check this file and the relevant sub-project's CLAUDE.md before starting
2. All domain knowledge (species profiles, substrate recipes, etc.) should be captured in memory files under `/home/node/.claude/projects/-workspace/memory/mushroom-growing/`
3. Context shared across sub-projects (e.g. species library, environmental targets) lives in this folder or `shared/mushroom-growing/`
