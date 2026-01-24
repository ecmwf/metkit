!> @file
!!
!! @brief Public API facade for mars2grib Fortran bindings.
!!
!! This module re-exports:
!!   - constants and constant conversion utilities
!!   - dictionary API (including iterators)
!!   - encoder API
!!
!! No implementation is provided here.
!!

module mars2grib_mod

! ------------------------------------------------------------------
! Re-export constants
! ------------------------------------------------------------------
    use :: mars2grib_constants_mod, only: M2G_OK
    use :: mars2grib_constants_mod, only: M2G_ERR_ARGS
    use :: mars2grib_constants_mod, only: M2G_ERR_ALLOC

    use :: mars2grib_constants_mod, only: M2G_UNDEFINED

    use :: mars2grib_constants_mod, only: M2G_STRING
    use :: mars2grib_constants_mod, only: M2G_BOOL
    use :: mars2grib_constants_mod, only: M2G_INT
    use :: mars2grib_constants_mod, only: M2G_LONG
    use :: mars2grib_constants_mod, only: M2G_FLOAT
    use :: mars2grib_constants_mod, only: M2G_DOUBLE

    use :: mars2grib_constants_mod, only: M2G_STRING_ARRAY
    use :: mars2grib_constants_mod, only: M2G_LONG_ARRAY
    use :: mars2grib_constants_mod, only: M2G_DOUBLE_ARRAY

    use :: mars2grib_constants_mod, only: M2G_DICT_UNKNOWN
    use :: mars2grib_constants_mod, only: M2G_DICT_MARS
    use :: mars2grib_constants_mod, only: M2G_DICT_GEOM
    use :: mars2grib_constants_mod, only: M2G_DICT_MISC
    use :: mars2grib_constants_mod, only: M2G_DICT_OPT

    use :: mars2grib_constants_mod, only: M2G_OK_STR
    use :: mars2grib_constants_mod, only: M2G_ERR_ARGS_STR
    use :: mars2grib_constants_mod, only: M2G_ERR_ALLOC_STR

    use :: mars2grib_constants_mod, only: M2G_UNDEFINED_STR

    use :: mars2grib_constants_mod, only: M2G_STRING_STR
    use :: mars2grib_constants_mod, only: M2G_BOOL_STR
    use :: mars2grib_constants_mod, only: M2G_INT_STR
    use :: mars2grib_constants_mod, only: M2G_LONG_STR
    use :: mars2grib_constants_mod, only: M2G_FLOAT_STR
    use :: mars2grib_constants_mod, only: M2G_DOUBLE_STR

    use :: mars2grib_constants_mod, only: M2G_STRING_ARRAY_STR
    use :: mars2grib_constants_mod, only: M2G_LONG_ARRAY_STR
    use :: mars2grib_constants_mod, only: M2G_DOUBLE_ARRAY_STR

    use :: mars2grib_constants_mod, only: M2G_DICT_UNKNOWN_STR
    use :: mars2grib_constants_mod, only: M2G_DICT_MARS_STR
    use :: mars2grib_constants_mod, only: M2G_DICT_GEOM_STR
    use :: mars2grib_constants_mod, only: M2G_DICT_MISC_STR
    use :: mars2grib_constants_mod, only: M2G_DICT_OPT_STR

    use :: mars2grib_constants_mod, only: mars2grib_const_to_string
    use :: mars2grib_constants_mod, only: mars2grib_const_from_string

! ------------------------------------------------------------------
! Re-export dictionary API
! ------------------------------------------------------------------
    use :: mars2grib_dictionaries_mod, only: mars2grib_dict
    use :: mars2grib_dictionaries_mod, only: mars2grib_dict_iterator

! ------------------------------------------------------------------
! Re-export encoder API
! ------------------------------------------------------------------
    use :: mars2grib_encoder_mod, only: mars2grib_encoder


implicit none

    private

! ------------------------------------------------------------------
! Public surface (one symbol per line)
! ------------------------------------------------------------------
    public :: M2G_OK
    public :: M2G_ERR_ARGS
    public :: M2G_ERR_ALLOC

    public :: M2G_UNDEFINED

    public :: M2G_STRING
    public :: M2G_BOOL
    public :: M2G_INT
    public :: M2G_LONG
    public :: M2G_FLOAT
    public :: M2G_DOUBLE

    public :: M2G_STRING_ARRAY
    public :: M2G_LONG_ARRAY
    public :: M2G_DOUBLE_ARRAY

    public :: M2G_DICT_UNKNOWN
    public :: M2G_DICT_MARS
    public :: M2G_DICT_GEOM
    public :: M2G_DICT_MISC
    public :: M2G_DICT_OPT

    public :: M2G_OK_STR
    public :: M2G_ERR_ARGS_STR
    public :: M2G_ERR_ALLOC_STR

    public :: M2G_UNDEFINED_STR

    public :: M2G_STRING_STR
    public :: M2G_BOOL_STR
    public :: M2G_INT_STR
    public :: M2G_LONG_STR
    public :: M2G_FLOAT_STR
    public :: M2G_DOUBLE_STR

    public :: M2G_STRING_ARRAY_STR
    public :: M2G_LONG_ARRAY_STR
    public :: M2G_DOUBLE_ARRAY_STR

    public :: M2G_DICT_UNKNOWN_STR
    public :: M2G_DICT_MARS_STR
    public :: M2G_DICT_GEOM_STR
    public :: M2G_DICT_MISC_STR
    public :: M2G_DICT_OPT_STR

    public :: mars2grib_const_to_string
    public :: mars2grib_const_from_string

    public :: mars2grib_dict
    public :: mars2grib_dict_iterator

    public :: mars2grib_encoder

end module mars2grib_mod
