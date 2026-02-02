@mainpage Encoder Architecture Overview

@tableofcontents

@section intro_overview Overview

This documentation describes the architecture of the Encoder: a modular,
 concept-driven system designed to construct encoded products (e.g. GRIB)
 from high-level semantic information.

 The Encoder is not a monolithic procedure. Instead, it is built as a
 composition of independent, orthogonal units—called *concepts*—that
 collectively describe *what* must be encoded, while the Encoder framework
 determines *how* this information is mapped to a concrete binary
 representation.

 The primary design goals are:
 - Separation of concerns between *semantics* and *encoding mechanics*
 - Compile-time validation of encoder configurations
 - Extensibility without modification of existing code paths
 - Deterministic and reproducible encoding behaviour

@section intro_audience Intended Audience

 This documentation targets:
 - Developers extending or maintaining the Encoder
 - Reviewers validating correctness and design choices
 - Advanced users integrating the Encoder into production workflows

 Familiarity with modern C++ (templates, constexpr, type traits) and
 encoded data formats (e.g. GRIB) is assumed.

@section intro_structure Documentation Structure

 The documentation is organised as follows:
 - @ref architecture_page : High-level architecture and data flow
 - @ref concepts_page     : Definition and role of concepts
 - @ref stages_page       : Encoding stages and execution model
 - @ref recipes_page      : Template selection and validation

 Each section builds progressively from abstract principles to concrete
 implementation details.
