
list( APPEND metkit_mars_rules_srcs
    AccessRule.cc
    AccessRule.h
    Cost.cc
    Cost.h
    Expression.h
    MarsExpression.cc
    MarsExpression.h
    MarsTaskProxy.h
    QueuePermission.cc
    QueuePermission.h
)

ecbuild_add_library (
    TARGET metkit-mars-rules

    SOURCES
        ${metkit_mars_rules_srcs}

    PUBLIC_INCLUDES
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>

    PUBLIC_LIBS
        metkit
)

ecbuild_generate_yy(
    YYPREFIX marsrules_
    YACC rulesy
    LEX rulesl
    DEPENDANT MarsExpression.cc
)