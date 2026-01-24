!> @file
!!
!! @brief Utility routines for C/Fortran string interoperability.
!!

module mars2grib_utils_mod

implicit none

    private

    public :: cstring2fstring
    public :: cstring_array2fstring_array
    public :: fstring2cstring
    public :: fstring_array2cstring_array
    public :: cstring_free
    public :: cstring_array_free

contains

! ======================================================================
! Convert C string (char*) to Fortran allocatable string
! ======================================================================

    subroutine cstring2fstring(cstr, fstr)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_char
        use, intrinsic :: iso_c_binding, only: c_null_char
        use, intrinsic :: iso_c_binding, only: c_f_pointer
        implicit none
        type(c_ptr),                intent(in)  :: cstr
        character(len=:), allocatable, intent(out) :: fstr

        character(kind=c_char), pointer :: p(:)
        integer :: n

        if (.not. c_associated(cstr)) then
            fstr = ""
            return
        end if

        call c_f_pointer(cstr, p, [huge(1)])

        n = 0
        do while (p(n+1) /= c_null_char)
            n = n + 1
        end do

        allocate(character(len=n) :: fstr)
        fstr = transfer(p(1:n), fstr)
        return
    end subroutine cstring2fstring


! ======================================================================
! Convert C string array (char**) to Fortran allocatable string array
! ======================================================================

    subroutine cstring_array2fstring_array(carray, n, farray)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_f_pointer
        implicit none
        type(c_ptr), intent(in) :: carray
        integer,     intent(in) :: n
        character(len=:), allocatable, intent(out) :: farray(:)

        type(c_ptr), pointer :: p(:)
        integer :: i

        if (n <= 0) then
            allocate(farray(0))
            return
        end if

        call c_f_pointer(carray, p, [n])
        allocate(farray(n))

        do i = 1, n
            call cstring2fstring(p(i), farray(i))
        end do

        return
    end subroutine cstring_array2fstring_array


! ======================================================================
! Convert Fortran string to C string (allocates memory)
! ======================================================================

    subroutine fstring2cstring(fstr, cstr, cptr)
        use, intrinsic :: iso_c_binding, only: c_char
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_loc
        use, intrinsic :: iso_c_binding, only: c_null_char
        implicit none
        character(len=*),              intent(in)  :: fstr
        character(kind=c_char), allocatable, intent(out) :: cstr
        type(c_ptr),                  intent(out) :: cptr

        integer :: n

        n = len_trim(fstr)
        allocate(cstr(n+1))
        cstr(1:n) = transfer(fstr(1:n), cstr(1:n))
        cstr(n+1) = c_null_char
        cptr = c_loc(cstr)
        return
    end subroutine fstring2cstring


! ======================================================================
! Convert Fortran string array to C string array (char**)
! ======================================================================

    subroutine fstring_array2cstring_array(farray, carray)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_char
        use, intrinsic :: iso_c_binding, only: c_loc
        implicit none
        character(len=*), intent(in) :: farray(:)
        type(c_ptr),      intent(out) :: carray

        type(c_ptr), allocatable, target :: ptrs(:)
        character(kind=c_char), allocatable, target :: buffers(:,:)
        integer :: i
        integer :: n
        integer :: maxlen

        n = size(farray)
        if (n == 0) then
            carray = c_null_ptr
            return
        end if

        maxlen = 0
        do i = 1, n
            maxlen = max(maxlen, len_trim(farray(i)))
        end do

        allocate(buffers(maxlen+1, n))
        allocate(ptrs(n))

        do i = 1, n
            call fstring2cstring(farray(i), buffers(:,i), ptrs(i))
        end do

        carray = c_loc(ptrs)
        return
    end subroutine fstring_array2cstring_array


! ======================================================================
! Free C string (char*)
! ======================================================================

    subroutine cstring_free(cstr)
        use, intrinsic :: iso_c_binding, only: c_ptr
        implicit none
        type(c_ptr), intent(inout) :: cstr

        interface
            subroutine mars2grib_free(p) bind(c, name="mars2grib_free")
                use, intrinsic :: iso_c_binding, only: c_ptr
                type(c_ptr), value :: p
            end subroutine mars2grib_free
        end interface

        if (c_associated(cstr)) then
            call mars2grib_free(cstr)
            cstr = c_null_ptr
        end if

        return
    end subroutine cstring_free


! ======================================================================
! Free C string array (char**)
! ======================================================================

    subroutine cstring_array_free(carray, n)
        use, intrinsic :: iso_c_binding, only: c_ptr
        use, intrinsic :: iso_c_binding, only: c_f_pointer
        implicit none
        type(c_ptr), intent(inout) :: carray
        integer,     intent(in)    :: n

        interface
            subroutine mars2grib_free(p) bind(c, name="mars2grib_free")
                use, intrinsic :: iso_c_binding, only: c_ptr
                type(c_ptr), value :: p
            end subroutine mars2grib_free
        end interface

        type(c_ptr), pointer :: p(:)
        integer :: i

        if (.not. c_associated(carray)) return

        call c_f_pointer(carray, p, [n])

        do i = 1, n
            if (c_associated(p(i))) then
                call mars2grib_free(p(i))
            end if
        end do

        call mars2grib_free(carray)
        carray = c_null_ptr
        return
    end subroutine cstring_array_free

end module mars2grib_utils_mod
