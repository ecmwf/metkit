!> @file
!!
!! @brief Fortran API for mars2grib encoders.
!!

module mars2grib_encoder_mod

implicit none

    private

    type :: mars2grib_encoder
        private
        type(c_ptr) :: impl = c_null_ptr
    contains
        procedure, public, pass :: open        => mars2grib_encoder_open_f
        procedure, public, pass :: encode64    => mars2grib_encoder_encode64_f
        procedure, public, pass :: encode32    => mars2grib_encoder_encode32_f
        procedure, public, pass :: close       => mars2grib_encoder_close_f
        procedure, public, pass :: c_ptr       => mars2grib_encoder_c_ptr_f
    end type mars2grib_encoder

    public :: mars2grib_encoder

contains

! ======================================================================
! c pointer accessor
! ======================================================================

    function mars2grib_encoder_c_ptr_f(enc) result(ptr)
        use, intrinsic :: iso_c_binding, only: c_ptr
        implicit none
        class(mars2grib_encoder), intent(in) :: enc
        type(c_ptr) :: ptr
        ptr = enc%impl
        return
    end function mars2grib_encoder_c_ptr_f


! ======================================================================
! encoder lifecycle
! ======================================================================

    function mars2grib_encoder_open_f(enc, opt_dict) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_null_ptr
        implicit none
        class(mars2grib_encoder), intent(inout) :: enc
        type(c_ptr),            intent(in), optional :: opt_dict
        integer :: err

        interface
            function mars2grib_encoder_open(opt_dict, encoder) result(c_err) &
                    bind(c, name="mars2grib_encoder_open")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                type(c_ptr), value :: opt_dict
                type(c_ptr) :: encoder
                integer(c_int) :: c_err
            end function mars2grib_encoder_open
        end interface

        integer(c_int) :: c_err
        type(c_ptr) :: c_opt_dict

        if (present(opt_dict)) then
            c_opt_dict = opt_dict
        else
            c_opt_dict = c_null_ptr
        end if

        enc%impl = c_null_ptr
        c_err = mars2grib_encoder_open(c_opt_dict, enc%impl)
        err = int(c_err, kind(err))
        return
    end function mars2grib_encoder_open_f


    function mars2grib_encoder_close_f(enc) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_null_ptr
        implicit none
        class(mars2grib_encoder), intent(inout) :: enc
        integer :: err

        interface
            function mars2grib_encoder_close(encoder) result(c_err) &
                    bind(c, name="mars2grib_encoder_close")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                type(c_ptr) :: encoder
                integer(c_int) :: c_err
            end function mars2grib_encoder_close
        end interface

        integer(c_int) :: c_err

        c_err = mars2grib_encoder_close(enc%impl)
        enc%impl = c_null_ptr
        err = int(c_err, kind(err))
        return
    end function mars2grib_encoder_close_f


! ======================================================================
! encoding (double precision)
! ======================================================================

    function mars2grib_encoder_encode64_f(enc, mars_dict, misc_dict, geom_dict, data, data_len, out_handle) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_long
        use, intrinsic :: iso_c_binding, only: c_double
        use, intrinsic :: iso_fortran_env, only: int64
        implicit none
        class(mars2grib_encoder), intent(in)  :: enc
        type(c_ptr),            intent(in)  :: mars_dict
        type(c_ptr),            intent(in)  :: misc_dict
        type(c_ptr),            intent(in)  :: geom_dict
        real(c_double),         intent(in)  :: data(*)
        integer(kind=int64),    intent(in)  :: data_len
        type(c_ptr),            intent(out) :: out_handle
        integer :: err

        interface
            function mars2grib_encoder_encode64(encoder, mars_dict, misc_dict, geom_dict, data, data_len, out_handle) &
                    result(c_err) bind(c, name="mars2grib_encoder_encode64")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                use, intrinsic :: iso_c_binding, only: c_long
                use, intrinsic :: iso_c_binding, only: c_double
                type(c_ptr),    value :: encoder
                type(c_ptr),    value :: mars_dict
                type(c_ptr),    value :: misc_dict
                type(c_ptr),    value :: geom_dict
                real(c_double), value :: data(*)
                integer(c_long), value :: data_len
                type(c_ptr) :: out_handle
                integer(c_int) :: c_err
            end function mars2grib_encoder_encode64
        end interface

        integer(c_int) :: c_err
        integer(c_long) :: c_len

        out_handle = c_null_ptr
        c_len = int(data_len, kind=c_long)

        c_err = mars2grib_encoder_encode64( &
                    enc%impl, &
                    mars_dict, &
                    misc_dict, &
                    geom_dict, &
                    data, &
                    c_len, &
                    out_handle )

        err = int(c_err, kind(err))
        return
    end function mars2grib_encoder_encode64_f


! ======================================================================
! encoding (single precision)
! ======================================================================

    function mars2grib_encoder_encode32_f(enc, mars_dict, misc_dict, geom_dict, data, data_len, out_handle) result(err)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_int
        use, intrinsic :: iso_c_binding, only: c_long
        use, intrinsic :: iso_c_binding, only: c_float
        use, intrinsic :: iso_fortran_env, only: int64
        implicit none
        class(mars2grib_encoder), intent(in)  :: enc
        type(c_ptr),            intent(in)  :: mars_dict
        type(c_ptr),            intent(in)  :: misc_dict
        type(c_ptr),            intent(in)  :: geom_dict
        real(c_float),          intent(in)  :: data(*)
        integer(kind=int64),    intent(in)  :: data_len
        type(c_ptr),            intent(out) :: out_handle
        integer :: err

        interface
            function mars2grib_encoder_encode32(encoder, mars_dict, misc_dict, geom_dict, data, data_len, out_handle) &
                    result(c_err) bind(c, name="mars2grib_encoder_encode32")
                use, intrinsic :: iso_c_binding, only: c_ptr
                use, intrinsic :: iso_c_binding, only: c_int
                use, intrinsic :: iso_c_binding, only: c_long
                use, intrinsic :: iso_c_binding, only: c_float
                type(c_ptr),   value :: encoder
                type(c_ptr),   value :: mars_dict
                type(c_ptr),   value :: misc_dict
                type(c_ptr),   value :: geom_dict
                real(c_float), value :: data(*)
                integer(c_long), value :: data_len
                type(c_ptr) :: out_handle
                integer(c_int) :: c_err
            end function mars2grib_encoder_encode32
        end interface

        integer(c_int) :: c_err
        integer(c_long) :: c_len

        out_handle = c_null_ptr
        c_len = int(data_len, kind=c_long)

        c_err = mars2grib_encoder_encode32( &
                    enc%impl, &
                    mars_dict, &
                    misc_dict, &
                    geom_dict, &
                    data, &
                    c_len, &
                    out_handle )

        err = int(c_err, kind(err))
        return
    end function mars2grib_encoder_encode32_f

end module mars2grib_encoder_mod
