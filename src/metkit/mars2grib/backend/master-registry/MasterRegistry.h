#pragma once

#include "metkit/mars2grib/backend/master-registry/typelist/TypeList.h"
#include "metkit/mars2grib/backend/master-registry/typelist/TypeListAlgorithms.h"
#include "metkit/mars2grib/backend/master-registry/typelist/TypeListTraversal.h"

#include "metkit/mars2grib/backend/master-registry/indexing/CompactIndexing.h"
#include "metkit/mars2grib/backend/master-registry/indexing/EnumMapping.h"
#include "metkit/mars2grib/backend/master-registry/indexing/OffsetComputation.h"

#include "metkit/mars2grib/backend/master-registry/arrays/ArrayBuilders.h"
#include "metkit/mars2grib/backend/master-registry/arrays/ArrayConcat.h"
#include "metkit/mars2grib/backend/master-registry/arrays/ConstexprArray.h"

#include "metkit/mars2grib/backend/master-registry/tables/TableMaterialization.h"
#include "metkit/mars2grib/backend/master-registry/tables/TablePolicies.h"
#include "metkit/mars2grib/backend/master-registry/tables/TableTraversal.h"

#include "metkit/mars2grib/backend/master-registry/registries/CallbackRegistryBase.h"
#include "metkit/mars2grib/backend/master-registry/registries/GeneralRegistryBase.h"
#include "metkit/mars2grib/backend/master-registry/registries/MatcherRegistryBase.h"

#include "metkit/mars2grib/backend/master-registry/utilities/DependentFalse.h"
#include "metkit/mars2grib/backend/master-registry/utilities/StaticAssert.h"
