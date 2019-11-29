/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef IndexFile_H
#define IndexFile_H

#include "eckit/container/BTree.h"
#include "eckit/memory/Counted.h"
// #include "eckit/filesystem/PathName.h"

#include "metkit/pointdb/FieldInfoData.h"
#include "pointdb/FieldInfoKey.h"


class IndexFile : public eckit::BTree<FieldInfoKey,
                  metkit::pointdb::FieldInfoData, 65536>,
                  public eckit::Counted {
public:
    IndexFile(const eckit::PathName& path) : eckit::BTree<FieldInfoKey, metkit::pointdb::FieldInfoData, 65536>(path) {}
    static IndexFile& lookUp(const eckit::PathName& path);

};

#endif
