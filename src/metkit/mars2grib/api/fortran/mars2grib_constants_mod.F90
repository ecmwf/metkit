!> @file
!!
!! @brief Constants for mars2grib Fortran API (single source of truth from C header).
!!

module mars2grib_constants_mod

    use, intrinsic :: iso_fortran_env, only: int64

implicit none

    ! Default visibility
    private

! Single source of truth (must be preprocessed; this provides macros)
#include "metkit/mars2grib/api/c/mars2grib_constant_values.h"

! Numeric constants (int64)
    integer(kind=int64), parameter :: M2G_OK            = int(M2G_VALUE_OK,            kind=int64)
    integer(kind=int64), parameter :: M2G_ERR_ARGS      = int(M2G_VALUE_ERR_ARGS,      kind=int64)
    integer(kind=int64), parameter :: M2G_ERR_ALLOC     = int(M2G_VALUE_ERR_ALLOC,     kind=int64)

    integer(kind=int64), parameter :: M2G_UNDEFINED     = int(M2G_VALUE_UNDEFINED,     kind=int64)

    integer(kind=int64), parameter :: M2G_STRING        = int(M2G_VALUE_STRING,        kind=int64)
    integer(kind=int64), parameter :: M2G_BOOL          = int(M2G_VALUE_BOOL,          kind=int64)
    integer(kind=int64), parameter :: M2G_INT           = int(M2G_VALUE_INT,           kind=int64)
    integer(kind=int64), parameter :: M2G_LONG          = int(M2G_VALUE_LONG,          kind=int64)
    integer(kind=int64), parameter :: M2G_FLOAT         = int(M2G_VALUE_FLOAT,         kind=int64)
    integer(kind=int64), parameter :: M2G_DOUBLE        = int(M2G_VALUE_DOUBLE,        kind=int64)

    integer(kind=int64), parameter :: M2G_STRING_ARRAY  = int(M2G_VALUE_STRING_ARRAY,  kind=int64)
    integer(kind=int64), parameter :: M2G_LONG_ARRAY    = int(M2G_VALUE_LONG_ARRAY,    kind=int64)
    integer(kind=int64), parameter :: M2G_DOUBLE_ARRAY  = int(M2G_VALUE_DOUBLE_ARRAY,  kind=int64)

    integer(kind=int64), parameter :: M2G_DICT_UNKNOWN  = int(M2G_VALUE_DICT_UNKNOWN,  kind=int64)
    integer(kind=int64), parameter :: M2G_DICT_MARS     = int(M2G_VALUE_DICT_MARS,     kind=int64)
    integer(kind=int64), parameter :: M2G_DICT_GEOM     = int(M2G_VALUE_DICT_GEOM,     kind=int64)
    integer(kind=int64), parameter :: M2G_DICT_MISC     = int(M2G_VALUE_DICT_MISC,     kind=int64)
    integer(kind=int64), parameter :: M2G_DICT_OPT      = int(M2G_VALUE_DICT_OPT,      kind=int64)

    ! String constants
    character(len=*), parameter :: M2G_OK_STR            = M2G_VALUE_OK_STR
    character(len=*), parameter :: M2G_ERR_ARGS_STR      = M2G_VALUE_ERR_ARGS_STR
    character(len=*), parameter :: M2G_ERR_ALLOC_STR     = M2G_VALUE_ERR_ALLOC_STR

    character(len=*), parameter :: M2G_UNDEFINED_STR     = M2G_VALUE_UNDEFINED_STR

    character(len=*), parameter :: M2G_STRING_STR        = M2G_VALUE_STRING_STR
    character(len=*), parameter :: M2G_BOOL_STR          = M2G_VALUE_BOOL_STR
    character(len=*), parameter :: M2G_INT_STR           = M2G_VALUE_INT_STR
    character(len=*), parameter :: M2G_LONG_STR          = M2G_VALUE_LONG_STR
    character(len=*), parameter :: M2G_FLOAT_STR         = M2G_VALUE_FLOAT_STR
    character(len=*), parameter :: M2G_DOUBLE_STR        = M2G_VALUE_DOUBLE_STR

    character(len=*), parameter :: M2G_STRING_ARRAY_STR  = M2G_VALUE_STRING_ARRAY_STR
    character(len=*), parameter :: M2G_LONG_ARRAY_STR    = M2G_VALUE_LONG_ARRAY_STR
    character(len=*), parameter :: M2G_DOUBLE_ARRAY_STR  = M2G_VALUE_DOUBLE_ARRAY_STR

    character(len=*), parameter :: M2G_DICT_UNKNOWN_STR  = M2G_VALUE_DICT_UNKNOWN_STR
    character(len=*), parameter :: M2G_DICT_MARS_STR     = M2G_VALUE_DICT_MARS_STR
    character(len=*), parameter :: M2G_DICT_GEOM_STR     = M2G_VALUE_DICT_GEOM_STR
    character(len=*), parameter :: M2G_DICT_MISC_STR     = M2G_VALUE_DICT_MISC_STR
    character(len=*), parameter :: M2G_DICT_OPT_STR      = M2G_VALUE_DICT_OPT_STR

    ! Public symbols
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

contains

! ------------------------------------------------------------------
! int64 -> string conversion (pure Fortran)
! ------------------------------------------------------------------
    function mars2grib_const_to_string(value) result(str)
    implicit none
        integer(kind=int64), intent(in) :: value
        character(len=:), allocatable   :: str

        select case (value)
        case (M2G_OK);            str = M2G_OK_STR
        case (M2G_ERR_ARGS);      str = M2G_ERR_ARGS_STR
        case (M2G_ERR_ALLOC);     str = M2G_ERR_ALLOC_STR

        case (M2G_UNDEFINED);     str = M2G_UNDEFINED_STR

        case (M2G_STRING);        str = M2G_STRING_STR
        case (M2G_BOOL);          str = M2G_BOOL_STR
        case (M2G_INT);           str = M2G_INT_STR
        case (M2G_LONG);          str = M2G_LONG_STR
        case (M2G_FLOAT);         str = M2G_FLOAT_STR
        case (M2G_DOUBLE);        str = M2G_DOUBLE_STR

        case (M2G_STRING_ARRAY);  str = M2G_STRING_ARRAY_STR
        case (M2G_LONG_ARRAY);    str = M2G_LONG_ARRAY_STR
        case (M2G_DOUBLE_ARRAY);  str = M2G_DOUBLE_ARRAY_STR

        case (M2G_DICT_UNKNOWN);  str = M2G_DICT_UNKNOWN_STR
        case (M2G_DICT_MARS);     str = M2G_DICT_MARS_STR
        case (M2G_DICT_GEOM);     str = M2G_DICT_GEOM_STR
        case (M2G_DICT_MISC);     str = M2G_DICT_MISC_STR
        case (M2G_DICT_OPT);      str = M2G_DICT_OPT_STR

        case default
            str = ""
        end select
    end function mars2grib_const_to_string

! ------------------------------------------------------------------
! string -> int64 conversion (pure Fortran)
! returns 0 on success, -1 on failure
! ------------------------------------------------------------------
    function mars2grib_const_from_string(name, value) result(err)
    implicit none
        character(len=*), intent(in)    :: name
        integer(kind=int64), intent(out):: value
        integer                         :: err

        select case (trim(name))
        case (M2G_OK_STR);            value = M2G_OK
        case (M2G_ERR_ARGS_STR);      value = M2G_ERR_ARGS
        case (M2G_ERR_ALLOC_STR);     value = M2G_ERR_ALLOC

        case (M2G_UNDEFINED_STR);     value = M2G_UNDEFINED

        case (M2G_STRING_STR);        value = M2G_STRING
        case (M2G_BOOL_STR);          value = M2G_BOOL
        case (M2G_INT_STR);           value = M2G_INT
        case (M2G_LONG_STR);          value = M2G_LONG
        case (M2G_FLOAT_STR);         value = M2G_FLOAT
        case (M2G_DOUBLE_STR);        value = M2G_DOUBLE

        case (M2G_STRING_ARRAY_STR);  value = M2G_STRING_ARRAY
        case (M2G_LONG_ARRAY_STR);    value = M2G_LONG_ARRAY
        case (M2G_DOUBLE_ARRAY_STR);  value = M2G_DOUBLE_ARRAY

        case (M2G_DICT_UNKNOWN_STR);  value = M2G_DICT_UNKNOWN
        case (M2G_DICT_MARS_STR);     value = M2G_DICT_MARS
        case (M2G_DICT_GEOM_STR);     value = M2G_DICT_GEOM
        case (M2G_DICT_MISC_STR);     value = M2G_DICT_MISC
        case (M2G_DICT_OPT_STR);      value = M2G_DICT_OPT

        case default
            err = -1
            value = M2G_UNDEFINED
            return
        end select

        err = 0
    end function mars2grib_const_from_string

end module mars2grib_constants_mod
