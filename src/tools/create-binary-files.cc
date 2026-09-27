/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <fstream>

#include "eckit/config/Resource.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/io/FileLock.h"
#include "eckit/runtime/Tool.h"
#include "eckit/thread/AutoLock.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/Dictionary.h"
#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/Serialize.h"

namespace metkit::mars {
class ConfigBinaryFiles : public eckit::Tool {
public:

    ConfigBinaryFiles(int argc, char** argv) : eckit::Tool(argc, argv) {}

    void run() override {
        static bool metkitForceBinfileCreation = eckit::Resource<bool>("$METKIT_FORCE_BINFILE_CREATION", false);

        const std::vector<std::string> verbs{"retrieve", "archive", "compute", "disseminate", "flush",
                                             "get",      "list",    "pointdb", "read",        "write"};

        for (const auto& verb : verbs) {
            metkit::mars::MarsLanguage marsLanguage(verb);
        }

        if (metkitForceBinfileCreation) {  // creating the binary file
            eckit::PathName langBinFile = metkit::LibMetkit::languageBinaryFile();
            std::cout << "Creating language binary file: " << langBinFile << std::endl;

            if (!langBinFile.exists()) {

                // serialise across processes: only one writer generates params.bin at a time
                eckit::FileLock lock(langBinFile.asString() + ".lock");
                eckit::AutoLock<eckit::FileLock> locker(lock);

                if (!langBinFile.exists()) {  // another process may have written it while we were waiting for the lock

                    // write to a sibling temp file and rename, so readers never see a partial file
                    auto tmpFile = eckit::PathName::unique(langBinFile.asString());
                    {
                        std::ofstream file{tmpFile.localPath(), std::ios::binary};

                        try {
                            file.exceptions(std::fstream::failbit | std::fstream::badbit | std::fstream::eofbit);

                            // header (magic + version)
                            const char* header = "LANG";
                            file.write(header, 4);
                            write16(file, metkit::LibMetkit::binaryFilesVersion());

                            // dictionaries (verbs and keywords)
                            metkit::mars::MarsLanguage::writeDictionaries(file);

                            // write the number of languages/verbs (and leave room for their offsets and sizes)
                            write8(file, verbs.size());
                            size_t zero = 0;
                            std::map<Verb, uint32_t> verbHeaderOffsets;
                            std::map<Verb, uint32_t> verbBodyOffsets;
                            for (const auto& verb : verbs) {
                                Verb vv = MarsLanguage::verb(verb);
                                write8(file, vv);
                                verbHeaderOffsets[vv] = file.tellp();
                                // make room for the offset of the verb body
                                write32(file, zero);
                            }

                            // write contextes
                            metkit::mars::MarsLanguage::writeContexts(file);


                            // write languages (and updates their offsets and sizes)
                            for (const auto& verb : verbs) {
                                Verb vv            = MarsLanguage::verb(verb);
                                uint32_t verbStart = file.tellp();
                                const auto& ll     = metkit::mars::MarsLanguage::get(vv);
                                ll.write(file);
                                uint32_t pos = file.tellp();
                                file.seekp(verbHeaderOffsets[vv]);
                                write32(file, verbStart);
                                file.seekp(pos);
                            }

                            file.flush();
                            file.close();
                            eckit::PathName::rename(tmpFile, langBinFile);
                        }
                        catch (const std::exception& e) {
                            std::ostringstream ss;
                            ss << "Error writing " << langBinFile << ": " << e.what() << std::endl;
                            throw eckit::SeriousBug(ss.str(), Here());
                        }
                    }
                }
            }
        }


        metkit::mars::MarsRequest req{"retrieve"};
        req.setValue("param", "t");
        metkit::mars::MarsExpansion{true}.expand(req);
    }
};

}  // namespace metkit::mars

int main(int argc, char** argv) {
    metkit::mars::ConfigBinaryFiles app(argc, argv);
    return app.start();
}
