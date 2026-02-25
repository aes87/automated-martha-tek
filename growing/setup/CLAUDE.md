# Mushroom Growing — Setup

## Overview
Documents and tools for designing and building the physical mushroom cultivation setup. Covers space planning, equipment selection, environmental targets, contamination controls, and workflow design.

## Tech Stack
- Language: Markdown (docs), Python or Node (if any tooling/calculators are needed)
- No framework required initially — this starts as a planning/documentation project

## Project Structure
```
setup/
├── CLAUDE.md
├── docs/           # Written plans, research, decisions
├── data/           # Environmental targets, equipment specs, cost tracking
└── tools/          # Any calculators or helper scripts
```

## Conventions
- Use Markdown for all documentation
- Include sources/reasoning when capturing environmental parameters (temp, humidity, CO2, FAE)
- Species-specific information should reference the shared species library when one is established
- Equipment choices should include model, purpose, cost, and supplier

## When Working Here
1. Read `/workspace/projects/mushroom-growing/CLAUDE.md` for domain conventions
2. Check memory at `/home/node/.claude/projects/-workspace/memory/mushroom-growing/` for prior context
3. When ingesting new context (user-provided specs, research, etc.), update memory files immediately
4. All decisions and rationale should be documented in `docs/` so they persist across sessions
