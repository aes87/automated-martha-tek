# Mushroom Growing — Strains

## Overview
Sub-project for selecting, planning, and tracking mushroom strains grown in the Martha tent. Covers strain selection rationale, vendor sourcing, substrate and materials planning, and per-batch grow logs.

## Structure
```
strains/
├── CLAUDE.md
├── catalog/          # Strain and species profiles (one file per species/strain)
├── vendors/          # Vendor profiles and notes
├── materials/        # Substrate recipes, consumables BOM, reorder tracking
└── grows/            # Grow logs — one file per batch
```

## Conventions
- **Species names**: always use binomial (e.g. *Pleurotus ostreatus*) plus common name
- **Strain IDs**: use format `SPECIES-SOURCE-DATE` (e.g. `POSTR-SPWNRUN-2026-03`)
- **Grow log files**: named `YYYY-MM-DD_strain-id.md` (inoculation date)
- **Substrate amounts**: in grams; moisture in % (wet weight basis unless noted)
- **Environmental targets**: defer to `setup/` project — do not duplicate numbers here
- **Vendors**: never store login credentials; store URLs, names, reliability notes only

## When Working Here
1. Read `/workspace/projects/mushroom-growing/CLAUDE.md` and this file first
2. Check memory at `/home/node/.claude/projects/-workspace/memory/mushroom-growing/`
3. When adding a strain, create a catalog entry and update `materials/bom.md` if new consumables are needed
4. When starting a grow, create a grow log entry and link to the strain catalog entry
