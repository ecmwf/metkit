# two_dimensions_merge
list(APPEND merge_tests_ok simple_two_way_merge
        simple_three_way_merge
        complex_two_way_merge
        type_upgrade_merge
        attribute_merge
        multi_merge
        multi_dimensions_merge
        signature_merge_1
        signature_merge_2
        new_missing_values_merge
        new_dimension_merge_1
        new_dimension_merge_2
        retype_merge
        coordinates_overlap_merge_1
        coordinates_overlap_merge_2
        coordinates_overlap_merge_3
        coordinates_overlap_merge_4
        coordinates_overlap_merge_5
        coordinates_overlap_merge_6
        date_merge_1
        date_merge_2
        variable_merge_1
        variable_merge_2
        variable_merge_3
        variable_merge_4
        variable_merge_5
        bounds_merge_1
        bounds_merge_2
        monotonic
        )

list(APPEND merge_tests_fail bounds_merge)
list(APPEND merge_tests_only coordinates_overlap_merge)

foreach(_TEST
    bounds_merge_2
    # ${merge_tests_ok}
    )
    ecbuild_add_test(TARGET "nccombine_${_TEST}"
                     COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/${_TEST}.sh")
endforeach()

