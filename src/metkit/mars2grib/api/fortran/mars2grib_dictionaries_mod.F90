!> @file
!!
!! @brief Fortran API for mars2grib dictionaries.
!!

module mars2grib_dictionaries_mod

    use, intrinsic :: iso_c_binding, only: c_ptr
    use, intrinsic :: iso_c_binding, only: c_null_ptr

implicit none

    private

    type :: mars2grib_dict
        private
        type(c_ptr) :: impl = c_null_ptr
    contains
        procedure, public, pass :: create            => mars2grib_dict_create_f
        procedure, public, pass :: destroy           => mars2grib_dict_destroy_f

        procedure, public, pass :: iterator_begin    => mars2grib_dict_iterator_begin_f
        procedure, public, pass :: iterator_destroy  => mars2grib_dict_iterator_destroy_f
        procedure, public, pass :: iterate           => mars2grib_dict_iterate_f

        procedure, public, pass :: dict_type         => mars2grib_dict_type_f
        procedure, public, pass :: has               => mars2grib_dict_has_f

        procedure, public, pass :: get_string        => mars2grib_dict_get_string_f
        procedure, public, pass :: get_bool          => mars2grib_dict_get_bool_f
        procedure, public, pass :: get_long          => mars2grib_dict_get_long_f
        procedure, public, pass :: get_double        => mars2grib_dict_get_double_f
        procedure, public, pass :: get_float         => mars2grib_dict_get_float_f

        procedure, public, pass :: get_string_array  => mars2grib_dict_get_string_array_f
        procedure, public, pass :: get_long_array    => mars2grib_dict_get_long_array_f
        procedure, public, pass :: get_double_array  => mars2grib_dict_get_double_array_f
        procedure, public, pass :: get_float_array   => mars2grib_dict_get_float_array_f

        procedure, public, pass :: set_string        => mars2grib_dict_set_string_f
        procedure, public, pass :: set_bool          => mars2grib_dict_set_bool_f
        procedure, public, pass :: set_long          => mars2grib_dict_set_long_f
        procedure, public, pass :: set_double        => mars2grib_dict_set_double_f
        procedure, public, pass :: set_float         => mars2grib_dict_set_float_f

        procedure, public, pass :: set_string_array  => mars2grib_dict_set_string_array_f
        procedure, public, pass :: set_long_array    => mars2grib_dict_set_long_array_f
        procedure, public, pass :: set_double_array  => mars2grib_dict_set_double_array_f
        procedure, public, pass :: set_float_array   => mars2grib_dict_set_float_array_f

        procedure, public, pass :: to_yaml           => mars2grib_dict_to_yaml_f
        procedure, public, pass :: to_json           => mars2grib_dict_to_json_f

        procedure, public, pass :: c_ptr             => mars2grib_dict_c_ptr_f
    end type mars2grib_dict

    type :: mars2grib_dict_iterator
        private
        type(c_ptr) :: impl = c_null_ptr
    contains
        procedure, public, pass :: c_ptr             => mars2grib_dict_iterator_c_ptr_f
    end type mars2grib_dict_iterator

    public :: mars2grib_dict
    public :: mars2grib_dict_iterator

contains

! ======================================================================
! c pointer accessors
! ======================================================================

    function mars2grib_dict_c_ptr_f(dict) result(ptr)
        use, intrinsic :: iso_c_binding, only: c_ptr
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        type(c_ptr) :: ptr
        ptr = dict%impl
        return
    end function mars2grib_dict_c_ptr_f


    function mars2grib_dict_iterator_c_ptr_f(it) result(ptr)
        use, intrinsic :: iso_c_binding, only: c_ptr
        implicit none
        class(mars2grib_dict_iterator), intent(in) :: it
        type(c_ptr) :: ptr
        ptr = it%impl
        return
    end function mars2grib_dict_iterator_c_ptr_f


! ======================================================================
! dictionary lifecycle
! ======================================================================

    function mars2grib_dict_create_f(dict, dict_type) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_loc
        use, intrinsic :: iso_c_binding, only: c_null_char
        use, intrinsic :: iso_c_binding, only: c_char
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(inout) :: dict
        character(len=*),      intent(in)    :: dict_type
        integer :: err

        interface
            function mars2grib_dict_create(dict, dict_type) result(c_err) bind(c, name="mars2grib_dict_create")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                type(c_ptr) :: dict
                type(c_ptr), value :: dict_type
                integer(c_int) :: c_err
            end function mars2grib_dict_create
        end interface

        character(kind=c_char), allocatable, target :: c_dict_type
        type(c_ptr) :: c_dict_type_ptr
        integer(c_int) :: c_err

        call fstring2cstring(dict_type, c_dict_type, c_dict_type_ptr)

        c_err = mars2grib_dict_create(dict%impl, c_dict_type_ptr)
        err = int(c_err, kind(err))
        return
    end function mars2grib_dict_create_f


    function mars2grib_dict_destroy_f(dict) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_null_ptr
        implicit none
        class(mars2grib_dict), intent(inout) :: dict
        integer :: err

        interface
            function mars2grib_dict_destroy(dict) result(c_err) bind(c, name="mars2grib_dict_destroy")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                type(c_ptr) :: dict
                integer(c_int) :: c_err
            end function mars2grib_dict_destroy
        end interface

        integer(c_int) :: c_err

        c_err = mars2grib_dict_destroy(dict%impl)
        dict%impl = c_null_ptr
        err = int(c_err, kind(err))
        return
    end function mars2grib_dict_destroy_f


! ======================================================================
! iterator
! ======================================================================

    function mars2grib_dict_iterator_begin_f(dict, it) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_null_ptr
        implicit none
        class(mars2grib_dict),          intent(in)    :: dict
        class(mars2grib_dict_iterator), intent(inout) :: it
        integer :: err

        interface
            function mars2grib_dict_iterator_begin(dict, iterator) result(c_err) bind(c, name="mars2grib_dict_iterator_begin")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                type(c_ptr), value :: dict
                type(c_ptr) :: iterator
                integer(c_int) :: c_err
            end function mars2grib_dict_iterator_begin
        end interface

        integer(c_int) :: c_err

        it%impl = c_null_ptr
        c_err = mars2grib_dict_iterator_begin(dict%impl, it%impl)
        err = int(c_err, kind(err))
        return
    end function mars2grib_dict_iterator_begin_f


    function mars2grib_dict_iterate_f(dict, it, key, value_ptr, type_id, len) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_null_ptr
        use, intrinsic :: iso_fortran_env, only: int64
        use :: mars2grib_utils_mod, only: cstring2fstring
        use :: mars2grib_utils_mod, only: cstring_free
        implicit none
        class(mars2grib_dict),          intent(in)    :: dict
        class(mars2grib_dict_iterator), intent(inout) :: it
        character(len=:), allocatable,  intent(out)   :: key
        type(c_ptr),                   intent(out)   :: value_ptr
        integer(kind=int64),           intent(out)   :: type_id
        integer(kind=int64),           intent(out)   :: len
        integer :: err

        interface
            function mars2grib_dict_iterate(dict, iterator, key, value, type_id, len) result(c_err) bind(c, name="mars2grib_dict_iterate")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                type(c_ptr), value :: dict
                type(c_ptr) :: iterator
                type(c_ptr) :: key
                type(c_ptr) :: value
                integer(c_int) :: type_id
                integer(c_int) :: len
                integer(c_int) :: c_err
            end function mars2grib_dict_iterate
        end interface

        type(c_ptr) :: c_key
        type(c_ptr) :: c_value
        integer(c_int) :: c_type_id
        integer(c_int) :: c_len
        integer(c_int) :: c_err

        c_key = c_null_ptr
        c_value = c_null_ptr
        c_type_id = 0_c_int
        c_len = 0_c_int

        c_err = mars2grib_dict_iterate(dict%impl, it%impl, c_key, c_value, c_type_id, c_len)
        err = int(c_err, kind(err))

        value_ptr = c_value
        type_id = int(c_type_id, kind=int64)
        len = int(c_len, kind=int64)

        if (c_key .ne. c_null_ptr) then
            call cstring2fstring(c_key, key)
            call cstring_free(c_key)
        else
            key = ""
        end if

        return
    end function mars2grib_dict_iterate_f


    function mars2grib_dict_iterator_destroy_f(dict, it) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_null_ptr
        implicit none
        class(mars2grib_dict),          intent(in)    :: dict
        class(mars2grib_dict_iterator), intent(inout) :: it
        integer :: err

        interface
            function mars2grib_dict_iterator_destroy(dict, iterator) result(c_err) bind(c, name="mars2grib_dict_iterator_destroy")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                type(c_ptr), value :: dict
                type(c_ptr) :: iterator
                integer(c_int) :: c_err
            end function mars2grib_dict_iterator_destroy
        end interface

        integer(c_int) :: c_err

        c_err = mars2grib_dict_iterator_destroy(dict%impl, it%impl)
        it%impl = c_null_ptr
        err = int(c_err, kind(err))
        return
    end function mars2grib_dict_iterator_destroy_f


! ======================================================================
! dictionary queries
! ======================================================================

    function mars2grib_dict_type_f(dict, dict_type, dict_type_id) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_null_ptr
        use, intrinsic :: iso_fortran_env, only: int64
        use :: mars2grib_utils_mod, only: cstring2fstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=:), allocatable, intent(out) :: dict_type
        integer(kind=int64), intent(out) :: dict_type_id
        integer :: err

        interface
            function mars2grib_dict_type(dict, dict_type, dict_type_id) result(c_err) bind(c, name="mars2grib_dict_type")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                type(c_ptr), value :: dict
                type(c_ptr) :: dict_type
                integer(c_int) :: dict_type_id
                integer(c_int) :: c_err
            end function mars2grib_dict_type
        end interface

        type(c_ptr) :: c_dict_type
        integer(c_int) :: c_dict_type_id
        integer(c_int) :: c_err

        c_dict_type = c_null_ptr
        c_dict_type_id = 0_c_int

        c_err = mars2grib_dict_type(dict%impl, c_dict_type, c_dict_type_id)
        err = int(c_err, kind(err))
        dict_type_id = int(c_dict_type_id, kind=int64)

        if (c_dict_type .ne. c_null_ptr) then
            call cstring2fstring(c_dict_type, dict_type)
        else
            dict_type = ""
        end if

        return
    end function mars2grib_dict_type_f


    function mars2grib_dict_has_f(dict, key, type_id) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_fortran_env, only: int64
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        integer(kind=int64), intent(out) :: type_id
        integer :: err

        interface
            function mars2grib_dict_has(dict, key, type_id) result(c_err) bind(c, name="mars2grib_dict_has")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                integer(c_int) :: type_id
                integer(c_int) :: c_err
            end function mars2grib_dict_has
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        integer(c_int) :: c_type_id
        integer(c_int) :: c_err

        call fstring2cstring(key, c_key, c_key_ptr)

        c_type_id = 0_c_int
        c_err = mars2grib_dict_has(dict%impl, c_key_ptr, c_type_id)
        err = int(c_err, kind(err))
        type_id = int(c_type_id, kind=int64)
        return
    end function mars2grib_dict_has_f


! ======================================================================
! scalar getters
! ======================================================================

    function mars2grib_dict_get_string_f(dict, key, value) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_null_ptr
        use :: mars2grib_utils_mod, only: fstring2cstring
        use :: mars2grib_utils_mod, only: cstring2fstring
        use :: mars2grib_utils_mod, only: cstring_free
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        character(len=:), allocatable, intent(out) :: value
        integer :: err

        interface
            function mars2grib_dict_get_string(dict, key, value) result(c_err) bind(c, name="mars2grib_dict_get_string")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                type(c_ptr) :: value
                integer(c_int) :: c_err
            end function mars2grib_dict_get_string
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        type(c_ptr) :: c_value
        integer(c_int) :: c_err

        call fstring2cstring(key, c_key, c_key_ptr)

        c_value = c_null_ptr
        c_err = mars2grib_dict_get_string(dict%impl, c_key_ptr, c_value)
        err = int(c_err, kind(err))

        if (c_value .ne. c_null_ptr) then
            call cstring2fstring(c_value, value)
            call cstring_free(c_value)
        else
            value = ""
        end if

        return
    end function mars2grib_dict_get_string_f


    function mars2grib_dict_get_bool_f(dict, key, value) result(err)
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_long
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        logical, intent(out) :: value
        integer :: err

        interface
            function mars2grib_dict_get_bool(dict, key, value) result(c_err) bind(c, name="mars2grib_dict_get_bool")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                use, intrinsic :: iso_c_binding, only: c_long
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                integer(c_long) :: value
                integer(c_int) :: c_err
            end function mars2grib_dict_get_bool
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        integer(c_long) :: c_value
        integer(c_int) :: c_err

        call fstring2cstring(key, c_key, c_key_ptr)

        c_value = 0_c_long
        c_err = mars2grib_dict_get_bool(dict%impl, c_key_ptr, c_value)
        err = int(c_err, kind(err))
        value = (c_value /= 0_c_long)
        return
    end function mars2grib_dict_get_bool_f


    function mars2grib_dict_get_long_f(dict, key, value) result(err)
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_long
        use, intrinsic :: iso_fortran_env, only: int64
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        integer(kind=int64), intent(out) :: value
        integer :: err

        interface
            function mars2grib_dict_get_long(dict, key, value) result(c_err) bind(c, name="mars2grib_dict_get_long")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                use, intrinsic :: iso_c_binding, only: c_long
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                integer(c_long) :: value
                integer(c_int) :: c_err
            end function mars2grib_dict_get_long
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        integer(c_long) :: c_value
        integer(c_int) :: c_err

        call fstring2cstring(key, c_key, c_key_ptr)

        c_value = 0_c_long
        c_err = mars2grib_dict_get_long(dict%impl, c_key_ptr, c_value)
        err = int(c_err, kind(err))
        value = int(c_value, kind=int64)
        return
    end function mars2grib_dict_get_long_f


    function mars2grib_dict_get_double_f(dict, key, value) result(err)
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_double
        use, intrinsic :: iso_fortran_env, only: real64
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        real(kind=real64), intent(out) :: value
        integer :: err

        interface
            function mars2grib_dict_get_double(dict, key, value) result(c_err) bind(c, name="mars2grib_dict_get_double")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                use, intrinsic :: iso_c_binding, only: c_double
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                real(c_double) :: value
                integer(c_int) :: c_err
            end function mars2grib_dict_get_double
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        real(c_double) :: c_value
        integer(c_int) :: c_err

        call fstring2cstring(key, c_key, c_key_ptr)

        c_value = 0.0_c_double
        c_err = mars2grib_dict_get_double(dict%impl, c_key_ptr, c_value)
        err = int(c_err, kind(err))
        value = real(c_value, kind=real64)
        return
    end function mars2grib_dict_get_double_f


    function mars2grib_dict_get_float_f(dict, key, value) result(err)
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_float
        use, intrinsic :: iso_fortran_env, only: real32
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        real(kind=real32), intent(out) :: value
        integer :: err

        interface
            function mars2grib_dict_get_float(dict, key, value) result(c_err) bind(c, name="mars2grib_dict_get_float")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                use, intrinsic :: iso_c_binding, only: c_float
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                real(c_float) :: value
                integer(c_int) :: c_err
            end function mars2grib_dict_get_float
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        real(c_float) :: c_value
        integer(c_int) :: c_err

        call fstring2cstring(key, c_key, c_key_ptr)

        c_value = 0.0_c_float
        c_err = mars2grib_dict_get_float(dict%impl, c_key_ptr, c_value)
        err = int(c_err, kind(err))
        value = real(c_value, kind=real32)
        return
    end function mars2grib_dict_get_float_f


! ======================================================================
! array getters
! ======================================================================

    function mars2grib_dict_get_string_array_f(dict, key, values, vlen) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_null_ptr
        use, intrinsic :: iso_fortran_env, only: int64
        use :: mars2grib_utils_mod, only: fstring2cstring
        use :: mars2grib_utils_mod, only: cstring_array2fstring_array
        use :: mars2grib_utils_mod, only: cstring_array_free
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        character(len=:), allocatable, intent(out) :: values(:)
        integer(kind=int64), intent(out) :: vlen
        integer :: err

        interface
            function mars2grib_dict_get_string_array(dict, key, value, vlen) result(c_err) bind(c, name="mars2grib_dict_get_string_array")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                type(c_ptr) :: value
                integer(c_int) :: vlen
                integer(c_int) :: c_err
            end function mars2grib_dict_get_string_array
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        type(c_ptr) :: c_values
        integer(c_int) :: c_vlen
        integer(c_int) :: c_err

        call fstring2cstring(key, c_key, c_key_ptr)

        c_values = c_null_ptr
        c_vlen = 0_c_int
        c_err = mars2grib_dict_get_string_array(dict%impl, c_key_ptr, c_values, c_vlen)
        err = int(c_err, kind(err))
        vlen = int(c_vlen, kind=int64)

        if (c_values .ne. c_null_ptr) then
            call cstring_array2fstring_array(c_values, c_vlen, values)
            call cstring_array_free(c_values, c_vlen)
        else
            allocate(values(0))
        end if

        return
    end function mars2grib_dict_get_string_array_f


    function mars2grib_dict_get_long_array_f(dict, key, values, vlen) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_long
        use, intrinsic :: iso_c_binding, only: c_f_pointer
        use, intrinsic :: iso_fortran_env, only: int64
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        integer(kind=int64), allocatable, intent(out) :: values(:)
        integer(kind=int64), intent(out) :: vlen
        integer :: err

        interface
            function mars2grib_dict_get_long_array(dict, key, value, vlen) result(c_err) bind(c, name="mars2grib_dict_get_long_array")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                use, intrinsic :: iso_c_binding, only: c_long
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                type(c_ptr) :: value
                integer(c_int) :: vlen
                integer(c_int) :: c_err
            end function mars2grib_dict_get_long_array
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        type(c_ptr) :: c_values_ptr
        integer(c_int) :: c_vlen
        integer(c_int) :: c_err

        integer(c_long), pointer :: c_values(:)
        integer(kind=int64) :: i

        call fstring2cstring(key, c_key, c_key_ptr)

        c_values_ptr = c_null_ptr
        c_vlen = 0_c_int
        c_err = mars2grib_dict_get_long_array(dict%impl, c_key_ptr, c_values_ptr, c_vlen)
        err = int(c_err, kind(err))
        vlen = int(c_vlen, kind=int64)

        if (c_values_ptr .ne. c_null_ptr .and. c_vlen > 0_c_int) then
            call c_f_pointer(c_values_ptr, c_values, [c_vlen])
            allocate(values(c_vlen))
            do i = 1_int64, int(c_vlen, kind=int64)
                values(i) = int(c_values(i), kind=int64)
            end do
        else
            allocate(values(0))
        end if

        return
    end function mars2grib_dict_get_long_array_f


    function mars2grib_dict_get_double_array_f(dict, key, values, vlen) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_double
        use, intrinsic :: iso_c_binding, only: c_f_pointer
        use, intrinsic :: iso_fortran_env, only: real64
        use, intrinsic :: iso_fortran_env, only: int64
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        real(kind=real64), allocatable, intent(out) :: values(:)
        integer(kind=int64), intent(out) :: vlen
        integer :: err

        interface
            function mars2grib_dict_get_double_array(dict, key, value, vlen) result(c_err) bind(c, name="mars2grib_dict_get_double_array")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                use, intrinsic :: iso_c_binding, only: c_double
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                type(c_ptr) :: value
                integer(c_int) :: vlen
                integer(c_int) :: c_err
            end function mars2grib_dict_get_double_array
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        type(c_ptr) :: c_values_ptr
        integer(c_int) :: c_vlen
        integer(c_int) :: c_err

        real(c_double), pointer :: c_values(:)
        integer(kind=int64) :: i

        call fstring2cstring(key, c_key, c_key_ptr)

        c_values_ptr = c_null_ptr
        c_vlen = 0_c_int
        c_err = mars2grib_dict_get_double_array(dict%impl, c_key_ptr, c_values_ptr, c_vlen)
        err = int(c_err, kind(err))
        vlen = int(c_vlen, kind=int64)

        if (c_values_ptr .ne. c_null_ptr .and. c_vlen > 0_c_int) then
            call c_f_pointer(c_values_ptr, c_values, [c_vlen])
            allocate(values(c_vlen))
            do i = 1_int64, int(c_vlen, kind=int64)
                values(i) = real(c_values(i), kind=real64)
            end do
        else
            allocate(values(0))
        end if

        return
    end function mars2grib_dict_get_double_array_f


    function mars2grib_dict_get_float_array_f(dict, key, values, vlen) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_float
        use, intrinsic :: iso_c_binding, only: c_f_pointer
        use, intrinsic :: iso_fortran_env, only: real32
        use, intrinsic :: iso_fortran_env, only: int64
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        real(kind=real32), allocatable, intent(out) :: values(:)
        integer(kind=int64), intent(out) :: vlen
        integer :: err

        interface
            function mars2grib_dict_get_float_array(dict, key, value, vlen) result(c_err) bind(c, name="mars2grib_dict_get_float_array")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                use, intrinsic :: iso_c_binding, only: c_float
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                type(c_ptr) :: value
                integer(c_int) :: vlen
                integer(c_int) :: c_err
            end function mars2grib_dict_get_float_array
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        type(c_ptr) :: c_values_ptr
        integer(c_int) :: c_vlen
        integer(c_int) :: c_err

        real(c_float), pointer :: c_values(:)
        integer(kind=int64) :: i

        call fstring2cstring(key, c_key, c_key_ptr)

        c_values_ptr = c_null_ptr
        c_vlen = 0_c_int
        c_err = mars2grib_dict_get_float_array(dict%impl, c_key_ptr, c_values_ptr, c_vlen)
        err = int(c_err, kind(err))
        vlen = int(c_vlen, kind=int64)

        if (c_values_ptr .ne. c_null_ptr .and. c_vlen > 0_c_int) then
            call c_f_pointer(c_values_ptr, c_values, [c_vlen])
            allocate(values(c_vlen))
            do i = 1_int64, int(c_vlen, kind=int64)
                values(i) = real(c_values(i), kind=real32)
            end do
        else
            allocate(values(0))
        end if

        return
    end function mars2grib_dict_get_float_array_f


! ======================================================================
! scalar setters
! ======================================================================

    function mars2grib_dict_set_string_f(dict, key, value) result(err)
        use, intrinsic :: iso_c_binding, only: c_int
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        character(len=*), intent(in) :: value
        integer :: err

        interface
            function mars2grib_dict_set_string(dict, key, value) result(c_err) bind(c, name="mars2grib_dict_set_string")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                type(c_ptr), value :: value
                integer(c_int) :: c_err
            end function mars2grib_dict_set_string
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        character(kind=c_char), allocatable, target :: c_value
        type(c_ptr) :: c_value_ptr
        integer(c_int) :: c_err

        call fstring2cstring(key, c_key, c_key_ptr)
        call fstring2cstring(value, c_value, c_value_ptr)

        c_err = mars2grib_dict_set_string(dict%impl, c_key_ptr, c_value_ptr)
        err = int(c_err, kind(err))
        return
    end function mars2grib_dict_set_string_f


    function mars2grib_dict_set_bool_f(dict, key, value) result(err)
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_long
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        logical, intent(in) :: value
        integer :: err

        interface
            function mars2grib_dict_set_bool(dict, key, value) result(c_err) bind(c, name="mars2grib_dict_set_bool")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                use, intrinsic :: iso_c_binding, only: c_long
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                integer(c_long), value :: value
                integer(c_int) :: c_err
            end function mars2grib_dict_set_bool
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        integer(c_long) :: c_value
        integer(c_int) :: c_err

        call fstring2cstring(key, c_key, c_key_ptr)

        if (value) then
            c_value = 1_c_long
        else
            c_value = 0_c_long
        end if

        c_err = mars2grib_dict_set_bool(dict%impl, c_key_ptr, c_value)
        err = int(c_err, kind(err))
        return
    end function mars2grib_dict_set_bool_f


    function mars2grib_dict_set_long_f(dict, key, value) result(err)
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_long
        use, intrinsic :: iso_fortran_env, only: int64
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        integer(kind=int64), intent(in) :: value
        integer :: err

        interface
            function mars2grib_dict_set_long(dict, key, value) result(c_err) bind(c, name="mars2grib_dict_set_long")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                use, intrinsic :: iso_c_binding, only: c_long
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                integer(c_long), value :: value
                integer(c_int) :: c_err
            end function mars2grib_dict_set_long
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        integer(c_long) :: c_value
        integer(c_int) :: c_err

        call fstring2cstring(key, c_key, c_key_ptr)

        c_value = int(value, kind=c_long)
        c_err = mars2grib_dict_set_long(dict%impl, c_key_ptr, c_value)
        err = int(c_err, kind(err))
        return
    end function mars2grib_dict_set_long_f


    function mars2grib_dict_set_double_f(dict, key, value) result(err)
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_double
        use, intrinsic :: iso_fortran_env, only: real64
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        real(kind=real64), intent(in) :: value
        integer :: err

        interface
            function mars2grib_dict_set_double(dict, key, value) result(c_err) bind(c, name="mars2grib_dict_set_double")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                use, intrinsic :: iso_c_binding, only: c_double
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                real(c_double), value :: value
                integer(c_int) :: c_err
            end function mars2grib_dict_set_double
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        real(c_double) :: c_value
        integer(c_int) :: c_err

        call fstring2cstring(key, c_key, c_key_ptr)

        c_value = real(value, kind=c_double)
        c_err = mars2grib_dict_set_double(dict%impl, c_key_ptr, c_value)
        err = int(c_err, kind(err))
        return
    end function mars2grib_dict_set_double_f


    function mars2grib_dict_set_float_f(dict, key, value) result(err)
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_float
        use, intrinsic :: iso_fortran_env, only: real32
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        real(kind=real32), intent(in) :: value
        integer :: err

        interface
            function mars2grib_dict_set_float(dict, key, value) result(c_err) bind(c, name="mars2grib_dict_set_float")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                use, intrinsic :: iso_c_binding, only: c_float
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                real(c_float), value :: value
                integer(c_int) :: c_err
            end function mars2grib_dict_set_float
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        real(c_float) :: c_value
        integer(c_int) :: c_err

        call fstring2cstring(key, c_key, c_key_ptr)

        c_value = real(value, kind=c_float)
        c_err = mars2grib_dict_set_float(dict%impl, c_key_ptr, c_value)
        err = int(c_err, kind(err))
        return
    end function mars2grib_dict_set_float_f


! ======================================================================
! array setters
! ======================================================================

    function mars2grib_dict_set_string_array_f(dict, key, values) result(err)
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_fortran_env, only: int64
        use :: mars2grib_utils_mod, only: fstring2cstring
        use :: mars2grib_utils_mod, only: fstring_array2cstring_array
        use :: mars2grib_utils_mod, only: cstring_array_free
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        character(len=*), intent(in) :: values(:)
        integer :: err

        interface
            function mars2grib_dict_set_string_array(dict, key, value, vlen) result(c_err) bind(c, name="mars2grib_dict_set_string_array")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                type(c_ptr), value :: value
                integer(c_int), value :: vlen
                integer(c_int) :: c_err
            end function mars2grib_dict_set_string_array
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        type(c_ptr) :: c_values
        integer(c_int) :: c_vlen
        integer(c_int) :: c_err

        call fstring2cstring(key, c_key, c_key_ptr)

        c_vlen = int(size(values), kind=c_int)
        call fstring_array2cstring_array(values, c_values)

        c_err = mars2grib_dict_set_string_array(dict%impl, c_key_ptr, c_values, c_vlen)
        err = int(c_err, kind(err))

        call cstring_array_free(c_values, c_vlen)
        return
    end function mars2grib_dict_set_string_array_f


    function mars2grib_dict_set_long_array_f(dict, key, values) result(err)
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_long
        use, intrinsic :: iso_fortran_env, only: int64
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        integer(kind=int64), intent(in) :: values(:)
        integer :: err

        interface
            function mars2grib_dict_set_long_array(dict, key, value, vlen) result(c_err) bind(c, name="mars2grib_dict_set_long_array")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                use, intrinsic :: iso_c_binding, only: c_long
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                integer(c_long), value :: value(*)
                integer(c_int), value :: vlen
                integer(c_int) :: c_err
            end function mars2grib_dict_set_long_array
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        integer(c_long), allocatable :: c_values(:)
        integer(c_int) :: c_vlen
        integer(c_int) :: c_err
        integer(kind=int64) :: i

        call fstring2cstring(key, c_key, c_key_ptr)

        c_vlen = int(size(values), kind=c_int)
        allocate(c_values(c_vlen))
        do i = 1_int64, int(c_vlen, kind=int64)
            c_values(i) = int(values(i), kind=c_long)
        end do

        c_err = mars2grib_dict_set_long_array(dict%impl, c_key_ptr, c_values, c_vlen)
        err = int(c_err, kind(err))
        return
    end function mars2grib_dict_set_long_array_f


    function mars2grib_dict_set_double_array_f(dict, key, values) result(err)
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_double
        use, intrinsic :: iso_fortran_env, only: real64
        use, intrinsic :: iso_fortran_env, only: int64
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        real(kind=real64), intent(in) :: values(:)
        integer :: err

        interface
            function mars2grib_dict_set_double_array(dict, key, value, vlen) result(c_err) bind(c, name="mars2grib_dict_set_double_array")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                use, intrinsic :: iso_c_binding, only: c_double
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                real(c_double), value :: value(*)
                integer(c_int), value :: vlen
                integer(c_int) :: c_err
            end function mars2grib_dict_set_double_array
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        real(c_double), allocatable :: c_values(:)
        integer(c_int) :: c_vlen
        integer(c_int) :: c_err
        integer(kind=int64) :: i

        call fstring2cstring(key, c_key, c_key_ptr)

        c_vlen = int(size(values), kind=c_int)
        allocate(c_values(c_vlen))
        do i = 1_int64, int(c_vlen, kind=int64)
            c_values(i) = real(values(i), kind=c_double)
        end do

        c_err = mars2grib_dict_set_double_array(dict%impl, c_key_ptr, c_values, c_vlen)
        err = int(c_err, kind(err))
        return
    end function mars2grib_dict_set_double_array_f


    function mars2grib_dict_set_float_array_f(dict, key, values) result(err)
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_float
        use, intrinsic :: iso_fortran_env, only: real32
        use, intrinsic :: iso_fortran_env, only: int64
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: key
        real(kind=real32), intent(in) :: values(:)
        integer :: err

        interface
            function mars2grib_dict_set_float_array(dict, key, value, vlen) result(c_err) bind(c, name="mars2grib_dict_set_float_array")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                use, intrinsic :: iso_c_binding, only: c_float
                type(c_ptr), value :: dict
                type(c_ptr), value :: key
                real(c_float), value :: value(*)
                integer(c_int), value :: vlen
                integer(c_int) :: c_err
            end function mars2grib_dict_set_float_array
        end interface

        character(kind=c_char), allocatable, target :: c_key
        type(c_ptr) :: c_key_ptr
        real(c_float), allocatable :: c_values(:)
        integer(c_int) :: c_vlen
        integer(c_int) :: c_err
        integer(kind=int64) :: i

        call fstring2cstring(key, c_key, c_key_ptr)

        c_vlen = int(size(values), kind=c_int)
        allocate(c_values(c_vlen))
        do i = 1_int64, int(c_vlen, kind=int64)
            c_values(i) = real(values(i), kind=c_float)
        end do

        c_err = mars2grib_dict_set_float_array(dict%impl, c_key_ptr, c_values, c_vlen)
        err = int(c_err, kind(err))
        return
    end function mars2grib_dict_set_float_array_f


! ======================================================================
! serialization
! ======================================================================

    function mars2grib_dict_to_yaml_f(dict, fname) result(err)
        use, intrinsic :: iso_c_binding, only: c_int
        use :: mars2grib_utils_mod, only: fstring2cstring
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=*), intent(in) :: fname
        integer :: err

        interface
            function mars2grib_dict_to_yaml(dict, fname) result(c_err) bind(c, name="mars2grib_dict_to_yaml")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                type(c_ptr), value :: dict
                type(c_ptr), value :: fname
                integer(c_int) :: c_err
            end function mars2grib_dict_to_yaml
        end interface

        character(kind=c_char), allocatable, target :: c_fname
        type(c_ptr) :: c_fname_ptr
        integer(c_int) :: c_err

        call fstring2cstring(fname, c_fname, c_fname_ptr)

        c_err = mars2grib_dict_to_yaml(dict%impl, c_fname_ptr)
        err = int(c_err, kind(err))
        return
    end function mars2grib_dict_to_yaml_f


    function mars2grib_dict_to_json_f(dict, value) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_null_ptr
        use :: mars2grib_utils_mod, only: cstring2fstring
        use :: mars2grib_utils_mod, only: cstring_free
        implicit none
        class(mars2grib_dict), intent(in) :: dict
        character(len=:), allocatable, intent(out) :: value
        integer :: err

        interface
            function mars2grib_dict_to_json(dict, value) result(c_err) bind(c, name="mars2grib_dict_to_json")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                type(c_ptr), value :: dict
                type(c_ptr) :: value
                integer(c_int) :: c_err
            end function mars2grib_dict_to_json
        end interface

        type(c_ptr) :: c_value
        integer(c_int) :: c_err

        c_value = c_null_ptr
        c_err = mars2grib_dict_to_json(dict%impl, c_value)
        err = int(c_err, kind(err))

        if (c_value .ne. c_null_ptr) then
            call cstring2fstring(c_value, value)
            call cstring_free(c_value)
        else
            value = ""
        end if

        return
    end function mars2grib_dict_to_json_f

end module mars2grib_dictionaries_mod
