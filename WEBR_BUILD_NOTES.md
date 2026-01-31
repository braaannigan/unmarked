# Building unmarked for webR: Summary and Lessons Learned

## Goal

Enable the `unmarked` R package to run in web browsers via [webR](https://docs.r-wasm.org/webr/latest/) (R compiled to WebAssembly). This allows:
- Interactive tutorials without R installation
- Browser-based ecological analyses
- Wider accessibility for teaching and reproducibility

## The Core Problem

The `unmarked` package fails to build for webR because of how it compiles TMB (Template Model Builder) code.

### Original Build Process
```
src/Makevars → calls src/TMB/compile.R → runs TMB::compile()
```

This fails during webR cross-compilation because:
1. `TMB::compile()` runs at package install time
2. It requires loading the `Matrix` package
3. Cross-compilation can't load WASM-compiled packages in the native R environment

## The Solution

Follow the `glmmTMB` approach: **compile TMB code inline** as part of the main package library instead of using a separate `TMB::compile()` step.

### Changes Made to unmarked

| File | Change |
|------|--------|
| `src/Makevars` | Removed `TMB::compile()` step, added inline compilation with `-DTMBAD_FRAMEWORK` |
| `src/Makevars.webr` | Created for webR builds (no OpenMP) |
| `src/Makevars.win` | Updated for inline TMB compilation |
| `src/unmarked_TMBExports.cpp` | Moved from `src/TMB/`, updated include paths |
| `src/RcppExports.cpp` | Changed `R_useDynamicSymbols(dll, TRUE)` for TMB symbol lookup |
| `NAMESPACE` | Removed separate `useDynLib(unmarked_TMBExports)` |
| `R/mixedModelTools.R`, `R/goccu.R`, `R/colext.R`, `R/IDS.R` | Changed `DLL = "unmarked"` |

## Validation Completed

### Native R Build ✅
- `R CMD build` succeeds
- `R CMD INSTALL` succeeds
- `R CMD check` passes
- All existing tests pass (same as before changes)
- TMB-based functions work (`goccu`, `occu(..., engine="TMB")`)

### Local webR Build ✅
- Built successfully using Docker: `ghcr.io/r-wasm/webr:main`
- Package compiles to WebAssembly

### Browser Testing ✅
- Package loads in webR: `library(unmarked)` works
- Basic models run: `occu(~1 ~1, data=umf)` works
- TMB engine works: `occu(~1 ~1, data=umf, engine="TMB")` works

## Constraints

1. **No changes to API** - Existing user code must work unchanged
2. **Cross-platform compatibility** - Must build on Linux, macOS, Windows, AND webR
3. **Maintainer accessibility** - unmarked maintainers are ecologists, not CS specialists

## Deployment Options

### Option 1: PR to Main Repository (Recommended)
- Submit PR to `ecoverseR/unmarked`
- Once merged, R-universe automatically builds WASM binaries
- Users install via: `webr::install("unmarked")`

### Option 2: Independent Distribution via R-universe
- Create `braaannigan/universe` repo with `packages.json`
- R-universe builds automatically
- Users install via: `webr::install("unmarked", repos = "https://braaannigan.r-universe.dev")`

### Option 3: GitHub Pages (Attempted)
- Host pre-built WASM binary on GitHub Pages
- **Blocked by**: repo.r-wasm.org returns 403 Forbidden for GitHub Actions IPs

## Lessons Learned

### 1. TMB Compilation Strategy
- `TMB::compile()` doesn't work for cross-compilation
- Inline compilation (like glmmTMB) works for both native and webR
- The `-DTMBAD_FRAMEWORK` flag is essential for TMB's automatic differentiation

### 2. Symbol Registration
- When compiling TMB inline, must enable dynamic symbol lookup
- Change `R_useDynamicSymbols(dll, FALSE)` to `TRUE` in `R_init_unmarked`
- Otherwise, TMB's internal C functions (`getParameterOrder`, etc.) aren't found

### 3. GitHub Actions + webR Repo
- `repo.r-wasm.org` blocks GitHub Actions IPs with 403 Forbidden
- curl with custom User-Agent doesn't help
- The official `r-wasm/actions/setup-rwasm` may work better (untested)

### 4. Docker Platform Mismatch
- webR Docker image is x86_64 only
- Running on Apple Silicon (arm64) causes segfaults via emulation
- Must use native x86 machines or GitHub Actions for reliable builds

### 5. Shell Syntax in GitHub Actions
- Default shell in containers is `sh`, not `bash`
- Bash arrays `()` cause syntax errors in sh
- Use `shell: bash` explicitly for bash-specific syntax

### 6. Dependency Chain
- TMB depends on: Matrix, RcppEigen
- reformulas depends on: Rdpack, rbibutils
- Must install in correct order when building manually

## Files Created for Reference

- `PR_DESCRIPTION.md` - Ready-to-use PR description for maintainers
- `WEBR_BUILD_NOTES.md` - This document
- `.github/workflows/build-webr.yaml` - GitHub Action for automated builds

## Next Steps

1. **Immediate**: Try `r-wasm/actions/setup-rwasm` action which may have proper repo access
2. **Alternative**: Set up R-universe for automatic builds (simpler, more reliable)
3. **Long-term**: Submit PR to ecoverseR/unmarked for official support

## Technical Reference

### Key TMB Configuration
```make
PKG_CPPFLAGS = -DSTRICT_R_HEADERS -DTMBAD_FRAMEWORK
PKG_LIBS = $(LAPACK_LIBS) $(BLAS_LIBS) $(SHLIB_OPENMP_CXXFLAGS)
```

### webR-specific (no OpenMP)
```make
PKG_CPPFLAGS = -DSTRICT_R_HEADERS -DTMBAD_FRAMEWORK
PKG_LIBS = $(LAPACK_LIBS) $(BLAS_LIBS)
PKG_CXXFLAGS =
```

### Symbol Registration Fix
```cpp
RcppExport void R_init_unmarked(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, TRUE);  // Changed from FALSE
}
```

## Resources

- [webR Documentation](https://docs.r-wasm.org/webr/latest/)
- [glmmTMB (reference implementation)](https://github.com/glmmTMB/glmmTMB)
- [r-wasm/rwasm issue #42](https://github.com/r-wasm/rwasm/issues/42)
- [R-universe](https://r-universe.dev/)
