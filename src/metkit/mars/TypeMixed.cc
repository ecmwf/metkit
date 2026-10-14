// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "metkit/mars/TypeMixed.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/TypesFactory.h"


namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeMixed::TypeMixed(const std::string& typeName, Keyword key, const eckit::Value& val) : Type(typeName, key, val) {
    eckit::Value types = val["type"];

    eckit::Value cfg;

    for (size_t i = 0; i < types.size(); ++i) {
        if (types[i].isString()) {
            cfg         = val;
            cfg["type"] = types[i];

            auto k = TypesFactory::build(0, cfg);
            types_.emplace_back(MarsLanguage::context(0), k);
        }
        else {  // it is a subtype, potentially with a Context
            cfg               = types[i];
            eckit::Value type = cfg["type"];

            const Context& c =
                cfg.contains("context") ? MarsLanguage::addContext(cfg["context"]) : MarsLanguage::context(0);

            auto k = TypesFactory::build(0, cfg);
            types_.emplace_back(c, k);
        }
    }
}


TypeMixed::TypeMixed(const std::string& type, Keyword key, MemFile& file) : Type(type, key, file) {
    uint8_t numSubtypes = file.read8();
    for (uint8_t i = 0; i < numSubtypes; ++i) {
        uint16_t ctxId = file.read16();
        std::string nestedTypeName{file.readString()};
        Keyword nestedTypeKey = file.read16();
        ASSERT(nestedTypeKey == 0);
        auto type = TypesFactory::build(nestedTypeName, 0, file);
        types_.emplace_back(MarsLanguage::context(ctxId), type);
    }
}


void TypeMixed::write(std::ofstream& file) const {
    Type::write(file);
    write8(file, types_.size());
    for (const auto& [ctx, type] : types_) {
        write16(file, ctx.get().id());
        type->write(file);
    }
}

void TypeMixed::print(std::ostream& out) const {
    out << "TypeMixed[name=" << name();
    for (const auto& [ctx, type] : types_) {
        out << "," << *type;
    }
    out << "]";
}

bool TypeMixed::expand(std::string& value, const MarsRequest& request) const {

    for (const auto& [ctx, type] : types_) {
        if (ctx.get().matches(request)) {
            std::string tmp = value;
            if (type->expand(tmp, request)) {
                value = tmp;
                return true;
            }
        }
    }
    return false;
}


static TypeBuilder<TypeMixed> type("mixed");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
