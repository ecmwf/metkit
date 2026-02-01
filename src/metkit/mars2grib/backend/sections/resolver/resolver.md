
/**
 * @page mars2grib_section_resolver Section Resolver Subsystem
 *
 * @tableofcontents
 *
 * @section resolver_overview Overview
 *
 * The **Section Resolver** subsystem is responsible for determining the
 * concrete layout of a GRIB section given:
 *
 * - A set of *declarative template rules* (recipes)
 * - A *runtime description of active concepts*
 *
 * Its purpose is to select, in a deterministic and efficient way, a
 * `SectionLayoutData` that fully defines how a GRIB section must be encoded.
 *
 * Conceptually, the resolver answers the question:
 *
 * @verbatim
 *   “Given the active concepts for this request,
 *    which section template applies?”
 * @endverbatim
 *
 * The resolver is a **pure backend subsystem**. It contains no frontend
 * orchestration logic and no configuration policy. It operates solely on:
 *
 * - Immutable configuration data
 * - Immutable runtime state
 *
 * and produces deterministic results.
 *
 * --------------------------------------------------------------------------
 *
 * @section resolver_big_picture Big picture
 *
 * Section resolution is defined by the interaction of three orthogonal elements:
 *
 * - **Declarative rules** describing which combinations of concept variants
 *   are admissible for a template
 * - **Runtime state** describing which concept variants are active
 * - **Resolution algorithms** that match the runtime state against the
 *   admissible rule space
 *
 * These responsibilities are intentionally separated to ensure:
 *
 * - Clear ownership of concerns
 * - Predictable behavior
 * - Efficient hot-path execution
 * - Long-term maintainability
 *
 * --------------------------------------------------------------------------
 *
 * @section resolver_namespace_structure Namespace organization
 *
 * The resolver subsystem is organized into a hierarchy of namespaces,
 * each with a clearly defined role.
 *
 * @subsection resolver_ns_root backend::sections::resolver
 *
 * This is the **public API** of the resolver subsystem.
 *
 * It exposes only the types required to *use* the resolver:
 *
 * - `ActiveConceptsData`
 *   Runtime description of which concept variants are active
 *
 * - `SectionLayoutData`
 *   Result of section resolution; fully describes how a section must be encoded
 *
 * - `SectionTemplateSelector`
 *   The main algorithmic entry point performing section resolution
 *
 * Frontend code must depend **only** on this namespace.
 *
 * --------------------------------------------------------------------------
 *
 * @subsection resolver_ns_dsl backend::sections::resolver::dsl
 *
 * This namespace defines the **declarative grammar** used to describe
 * section template rules.
 *
 * It contains no algorithms and performs no resolution.
 *
 * Types in this namespace are used to *describe*:
 *
 * - Which concepts participate in defining a template
 * - Which variants of each concept are admissible
 *
 * The main types are:
 *
 * - `Select`
 *   Compile-time selector defining a subset (possibly all) of variants
 *   for a single concept
 *
 * - `Recipe`
 *   Runtime representation of a single GRIB template number together with
 *   the full combinatorial space of admissible concept-variant combinations
 *
 * - `Recipes`
 *   Section-scoped container aggregating all `Recipe` instances defining
 *   all template numbers valid for a section
 *
 * The DSL namespace is intentionally expressive and declarative.
 * It defines *what is allowed*, not *what is selected*.
 *
 * --------------------------------------------------------------------------
 *
 * @subsection resolver_ns_detail backend::sections::resolver::detail
 *
 * This namespace contains **internal infrastructure** used by the resolver
 * algorithms.
 *
 * It includes:
 *
 * - Key representations and compression utilities
 * - Lookup tables and indices
 * - Intermediate data structures
 * - Performance-oriented helpers
 *
 * Types in this namespace are:
 *
 * - Not part of the public API
 * - Free to change
 * - Optimized for internal use
 *
 * No frontend code should depend on this namespace.
 *
 * --------------------------------------------------------------------------
 *
 * @subsection resolver_ns_debug backend::sections::resolver::debug
 *
 * This namespace contains **debugging and introspection utilities**.
 *
 * Typical contents include:
 *
 * - Human-readable printers
 * - JSON-like serialization helpers
 * - Diagnostic formatting utilities
 *
 * Debug code is intentionally isolated to ensure that:
 *
 * - Hot-path code remains minimal
 * - Debug facilities do not pollute public APIs
 * - Debug functionality can be gated or compiled out if needed
 *
 * --------------------------------------------------------------------------
 *
 * @section resolver_dsl The recipe DSL
 *
 * @subsection resolver_select Select: defining admissible variants
 *
 * A `Select` object defines a **subset of variants of a single concept**.
 *
 * It always refers to exactly one concept and expresses which of its
 * variants are admissible when defining a GRIB template.
 *
 * Two modes are supported:
 *
 * - **Explicit selection**
 *   When one or more variants are specified, only those variants are admissible.
 *
 * - **Implicit full selection (wildcard)**
 *   When no variants are specified, *all variants* of the concept are assumed
 *   to be admissible.
 *
 * `Select` is a compile-time construct and introduces no runtime overhead.
 *
 * --------------------------------------------------------------------------
 *
 * @subsection resolver_recipe Recipe: defining a template number
 *
 * A `Recipe` represents the runtime realization of a **single GRIB template
 * number**.
 *
 * A template number is defined by:
 *
 * - An ordered set of concepts
 * - For each concept, a set of admissible variants
 *
 * Because each concept may participate with multiple variants, the process
 * of defining a template number is **inherently combinatorial**.
 *
 * A `Recipe` captures this combinatorial space and provides a deterministic
 * way to enumerate all admissible combinations.
 *
 * --------------------------------------------------------------------------
 *
 * @subsection resolver_recipes Recipes: section-scoped template definitions
 *
 * A `Recipes` object represents the **complete set of template definitions**
 * valid for a single GRIB section.
 *
 * Each `Recipes` instance:
 *
 * - Is defined for exactly one section
 * - Aggregates multiple `Recipe` objects
 * - Represents all template numbers admissible for that section
 *
 * It provides a uniform expansion mechanism producing
 * `ResolvedTemplateData` payloads.
 *
 * --------------------------------------------------------------------------
 *
 * @section resolver_runtime Active concept state
 *
 * `ActiveConceptsData` represents the **runtime state** of concept activation
 * for a specific encoding request.
 *
 * It answers questions such as:
 *
 * - Which concepts are active?
 * - Which variant of a concept is active?
 * - Is a concept missing or unspecified?
 *
 * This runtime state is matched against the declarative recipe space
 * during resolution.
 *
 * --------------------------------------------------------------------------
 *
 * @section resolver_algorithm Resolution algorithm
 *
 * The resolution process performed by `SectionTemplateSelector` can be
 * summarized as:
 *
 * 1. Build a signature from the active concept state
 * 2. Match this signature against the admissible recipe space
 * 3. Select the corresponding section layout
 *
 * The algorithm is:
 *
 * - Deterministic
 * - Stateless
 * - Optimized for hot-path execution
 *
 * --------------------------------------------------------------------------
 *
 * @section resolver_dependency_rules Dependency rules
 *
 * The following dependency rules are enforced by design:
 *
 * - Frontend code depends only on `backend::sections::resolver`
 * - Resolver algorithms may depend on `dsl`, `detail`, and `debug`
 * - DSL types do not depend on resolver algorithms
 * - Debug utilities do not affect hot-path logic
 *
 * These rules ensure a clean layering and prevent accidental coupling.
 *
 * --------------------------------------------------------------------------
 *
 * @section resolver_summary Summary
 *
 * The section resolver subsystem provides a compiler-like architecture
 * for GRIB section template selection:
 *
 * - Declarative grammar (`dsl`)
 * - Runtime state (`ActiveConceptsData`)
 * - Deterministic resolution algorithms
 *
 * This design ensures correctness, performance, and long-term maintainability.
 */
