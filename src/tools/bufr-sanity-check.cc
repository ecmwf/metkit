/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "eckit/io/FileHandle.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/types/Date.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/config/Resource.h"
#include "eckit/parser/YAMLParser.h"

#include "eckit/message/Reader.h"
#include "eckit/message/Message.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/codes/CodesContent.h"
#include "metkit/tool/MetkitTool.h"

#define WRONG_KEY_LENGTH 65535

using namespace metkit;
using namespace eckit;
using namespace eckit::option;

//----------------------------------------------------------------------------------------------------------------------

enum Status {
    OK,
    FIXED,
    CORRUPTED
};

static std::map<long, long> subtypes_;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void readTable()
{
    eckit::PathName bufrSubtypesPath = eckit::Resource<eckit::PathName>("bufrSubtypesPath;$BUFR_SUBTYPES_PATH", LibMetkit::bufrSubtypesYamlFile());

    const eckit::Value bufrSubtypes = eckit::YAMLParser::decodeFile(bufrSubtypesPath);
    const eckit::Value subtypes = bufrSubtypes["subtypes"];
    ASSERT(subtypes.isList());
    for (size_t i = 0; i < subtypes.size(); ++i) {
        const eckit::Value s = subtypes[i];
        ASSERT(s.isList());
        ASSERT(s.size() == 2);
        subtypes_[s[0]] = s[1];
    }
}


class BufrCheck : public MetkitTool {
public:

    BufrCheck(int argc, char **argv) : MetkitTool(argc, argv) {

        options_.push_back(
            new SimpleOption<bool>("abort-on-error", "Abort in case of corrupted message, default = true"));
        options_.push_back(
            new SimpleOption<bool>("patch-on-error", "Try to patch corrupted messages, default = false"));
        options_.push_back(
            new SimpleOption<bool>("skip-on-error", "Skip corrupted messages, default = false"));

        options_.push_back(
            new SimpleOption<bool>("dont-patch-length", "Disable patching of message length in corrupted messages, default = false"));
        options_.push_back(
            new SimpleOption<bool>("dont-patch-date", "Disable patching of date/time in corrupted messages, default = false"));
        options_.push_back(
            new SimpleOption<bool>("ignore-century", "Disable patching of century in corrupted messages, default = false"));
        options_.push_back(
            new SimpleOption<long>("acceptable-time-discrepancy", "Acceptable time discrepancy in seconds, default = 300"));
           
        options_.push_back(
            new SimpleOption<bool>("verbose", "Print details of all corrupted messages, default = false"));
    }

    virtual ~BufrCheck() {}

private:  // methods
    int minimumPositionalArguments() const { return 2; }

    Status checkMessageLength(codes::CodesContent& c, int numMessage);
    Status checkSubType(codes::CodesContent& c, int numMessage);
    Status checkDate(codes::CodesContent& c, int numMessage);

    bool bufrTypeBySubtype(long subtype, long& type);

    void process(const PathName& input, const PathName& output);

    virtual void execute(const CmdArgs& args);

    virtual void init(const CmdArgs& args);

    virtual void usage(const std::string& tool) const;

private:  // members
    bool verbose_ = false;
    bool abort_ = true;
    bool patch_ = false;
    bool skip_ = false;
    bool ignoreLength_ = false;
    bool ignoreDate_ = false;
    bool ignoreCentury_ = false;
    long timeThreshold_ = 300;
};

//----------------------------------------------------------------------------------------------------------------------

void BufrCheck::execute(const CmdArgs& args) {
    process(args(0), args(1));
}

void BufrCheck::init(const CmdArgs& args) {

    // abort-on-error OR patch-on-error OR skip-on-error
    int cnt = 0;
    bool hasAbort = args.get("abort-on-error", abort_);
    if (hasAbort)
        cnt++;
    if (args.get("patch-on-error", patch_))
        cnt++;
    if (args.get("skip-on-error", skip_))
        cnt++;

    if (cnt>1) {
        throw UserError("Inconsistent configuration. You can only specify one of [--abort-on-error, --patch-on-error, --skip-on-error]");
    }
    if (cnt == 1) {
        abort_ = abort_ && hasAbort;
    }

    args.get("verbose", verbose_);
    args.get("dont-patch-length", ignoreLength_);
    args.get("dont-patch-date", ignoreDate_);
    args.get("ignore-century", ignoreCentury_);
    args.get("acceptable-time-discrepancy", timeThreshold_);
}

void BufrCheck::usage(const std::string& tool) const {
    Log::info() << "Usage: " << tool << " [options] [input] [output]" << std::endl
                << std::endl;

    Log::info() << "Examples:" << std::endl
                << "=========" << std::endl
                << std::endl
                << tool << " input.bufr output.bufr" << std::endl
                << std::endl
                << tool << " --skip-on-error --verbose input.bufr output.bufr" << std::endl
                << std::endl
                << tool << " --patch-on-error --ignore-century-mismatch --acceptable-time-discrepancy=600 input.bufr output.bufr" << std::endl
                << std::endl;
}

Status BufrCheck::checkMessageLength(codes::CodesContent& c, int numMessage) {

    long totalLength = c.getLong("totalLength");
    long messageLength = c.getLong("messageLength");

    if (totalLength != messageLength && totalLength < WRONG_KEY_LENGTH) {
        if (verbose_) {
            Log::error() << "message " << numMessage
                << ", wrong key length in bufr message " << messageLength << " instead of " << totalLength
                << std::endl;
        }
        if (!patch_ || ignoreLength_) {
            return Status::CORRUPTED;
        } else {
            c.setLong("messageLength", totalLength);
            return Status::FIXED;
        }
    }
    return Status::OK;
}

bool BufrCheck::bufrTypeBySubtype(long subtype, long& type) {
    pthread_once(&once, readTable);
    
    auto s = subtypes_.find(subtype);
    if (s != subtypes_.end()) {
        type = s->second;
        return true;
    }
    return false;
}

Status BufrCheck::checkSubType(codes::CodesContent& c, int numMessage) {

    long type = c.getLong("rdbType");
    long subtype = c.getLong("oldSubtype");
    long expectedType;

    if (bufrTypeBySubtype(subtype, expectedType)) {
        if (type == expectedType) {
            return Status::OK;
        } else {
            if (verbose_) {
                Log::error() << "message " << numMessage
                    << ", type " << type << " and expected type " << expectedType << " don't match for subtype " << subtype
                    << std::endl;
            }
            return Status::CORRUPTED;
        }
    } else {
        if (verbose_) {
            Log::error() << "message " << numMessage
                << ", unknown subtype " << subtype
                << std::endl;
        }
        return Status::CORRUPTED;
    }
}

Status BufrCheck::checkDate(codes::CodesContent& c, int numMessage) {

    bool toFix = false;

    long localYear = c.getLong("localYear");

    long typicalYear = c.getLong("typicalYear");
    if (ignoreCentury_) {
        typicalYear = (localYear/100)*100 + typicalYear%100;
    }
    long typicalMonth = c.getLong("typicalMonth");
    long typicalDay = c.getLong("typicalDay");
    long typicalJulian = 0;
    try {
        Date date(typicalYear, typicalMonth, typicalDay);
        typicalJulian = date.julian();
    } catch(...) {
        if (verbose_) {
            Log::error() << "message " << numMessage
                << ", date is weird " << typicalYear << "/" << typicalMonth << "/" << typicalDay
                << std::endl;
        }
        if (!patch_)
            return Status::CORRUPTED;

        toFix = !ignoreDate_;
    }

    long typicalHour = c.getLong("typicalHour");
    long typicalMinute = c.getLong("typicalMinute");
    long typicalSecond = c.getLong("typicalSecond");

    if (typicalHour>23 || typicalHour<0 || typicalMinute>59 || typicalMinute<0 || typicalSecond>59 || typicalSecond<0) {
        if (verbose_) {
            Log::error() << "message " << numMessage
                << ", typical time is weird " << typicalHour << ":" << typicalMinute << ":" << typicalSecond
                << std::endl;
        }
        if (!patch_)
            return Status::CORRUPTED;

        toFix = !ignoreDate_;
    }
    long typicalTime = typicalHour*3600 + typicalMinute*60 + typicalSecond;


    long localMonth = c.getLong("localMonth");
    long localDay = c.getLong("localDay");
    long localJulian = 0;

    try {
        Date date(localYear, localMonth, localDay);
        localJulian = date.julian();
    } catch(...) {
        if (verbose_) {
            Log::error() << "message " << numMessage
                << ", date is weird " << localYear << "/" << localMonth << "/" << localDay
                << std::endl;
        }
        return Status::CORRUPTED;
    }

    long localHour = c.getLong("localHour");
    long localMinute = c.getLong("localMinute");
    long localSecond = c.getLong("localSecond");

    // we accept localSecond==60 for backward compatibility (filterbufr behaviour)
    if (localHour>23 || localHour<0 || localMinute>59 || localMinute<0 || localSecond>60 || localSecond<0) {
        if (verbose_) {
            Log::error() << "message " << numMessage
                << ", local time is weird " << localHour << ":" << localMinute << ":" << localSecond
                << std::endl;
        }
        return Status::CORRUPTED;
    }
    long localTime = localHour*3600 + localMinute*60 + localSecond;

    if (abs((typicalJulian-localJulian)*86400 + typicalTime-localTime)>timeThreshold_) {
        if (verbose_) {
            Log::error() << "message " << numMessage << ", date-time (" <<
            typicalYear << "/" << typicalMonth << "/" << typicalDay << " " << typicalHour << ":" << typicalMinute << ":" << typicalSecond << 
            ") and local date-time (" <<
            localYear << "/" << localMonth << "/" << localDay << " " << localHour << ":" << localMinute << ":" << localSecond << 
            ") differs" << std::endl;
        }
        toFix = !ignoreDate_;
    }

    if (toFix) {
        if (c.getLong("edition") == 3) {
            c.setLong("typicalYearOfCentury", localYear-2000);
        } else {
            c.setLong("typicalYear", localYear);
            c.setLong("typicalSecond", localSecond);
        }
        c.setLong("typicalMonth", localMonth);
        c.setLong("typicalDay", localDay);
        c.setLong("typicalHour", localHour);
        c.setLong("typicalMinute", localMinute);
        return Status::FIXED;
    }
    return Status::OK;
}

void BufrCheck::process(const PathName& input, const PathName& output) {

    message::Reader reader(input);
    FileHandle out(output.path());
    out.openForWrite(0);
    AutoClose closer(out);
    
    int err=0;
 	void *buffer = NULL;
	size_t size = 0;
    bool ok;
    unsigned int numMessage=0;
    unsigned int missingKey=0;
    unsigned int messageLength=0;
    unsigned int inconsistentSubType=0;
    unsigned int inconsistentDate=0;

    message::Message msg;

    while ( (msg = reader.next()) ) {
        codes_handle* h = codes_handle_new_from_message(nullptr, msg.data(), msg.length());
        if(!h) {
            throw FailedLibraryCall("eccodes", "codes_handle_new_from_message", "failed to create handle", Here());
        }
        codes::CodesContent c(h, true);

        // verify the presence of section 2 (to store the MARS key)
        long localSectionPresent = c.getLong("localSectionPresent");

        ok = localSectionPresent != 0;
        if (!ok) {
            missingKey++;
        } else {
            switch (checkMessageLength(c, numMessage)) {
                case Status::CORRUPTED:
                    ok = false;
                    messageLength++;
                    break;
                case Status::FIXED:
                    messageLength++;
                    break;
                case Status::OK: ;
            };
            switch (checkSubType(c, numMessage)) {
                case Status::CORRUPTED:
                    ok = false;
                    inconsistentSubType++;
                    break;
                case Status::FIXED:
                    inconsistentSubType++;
                    break;
                case Status::OK: ;
            };
            switch (checkDate(c, numMessage)) {
                case Status::CORRUPTED:
                    ok = false;
                    inconsistentDate++;
                    break;
                case Status::FIXED:
                    inconsistentDate++;
                    break;
                case Status::OK: ;
            };
        }

        if (ok) {
            out.write(msg.data(), msg.length());
        } else if (abort_) {
            Log::error() << "message " << numMessage << " not compliant" << std::endl;
            exit(1);
        }
 
        numMessage++;
    }

    if (missingKey)
        Log::warning() << missingKey << " message" << (missingKey>1?"s miss ":" misses ") << " the MARS key" << std::endl;
    if (messageLength)
        Log::warning() << messageLength << " message" << (messageLength>1?"s ":" ") << " with incoherent message length in the MARS key" << std::endl;
    if (inconsistentSubType)
        Log::warning() << inconsistentSubType << " message" << (inconsistentSubType>1?"s ":" ") << " with unknown subtype" << std::endl;
    if (inconsistentDate)
        Log::warning() << inconsistentDate << " message" << (inconsistentDate>1? "s " : " ") << " with inconsistent date" << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    BufrCheck tool(argc, argv);
    return tool.start();
}
