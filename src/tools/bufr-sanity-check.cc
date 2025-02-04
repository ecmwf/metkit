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

#include "eckit/message/Reader.h"
#include "eckit/message/Message.h"

#include "metkit/codes/BUFRDecoder.h"
#include "metkit/codes/BufrContent.h"
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
            new SimpleOption<bool>("ignore-type", "Ignore inconsistent type/subtype, default = false"));
        options_.push_back(
            new SimpleOption<long>("acceptable-time-discrepancy", "Acceptable time discrepancy in seconds, default = 300"));
           
        options_.push_back(
            new SimpleOption<bool>("verbose", "Print details of all corrupted messages, default = false"));
    }

    virtual ~BufrCheck() {}

private:  // methods
    int minimumPositionalArguments() const { return 2; }

    Status checkMessageLength(const message::Message& msg, int numMessage, eckit::StringDict& transformation);
    Status checkSubType(const message::Message& msg, int numMessage);
    Status checkDate(const message::Message& msg, int numMessage, eckit::StringDict& transformation);

    void process(const PathName& input, const PathName& output);

    virtual void execute(const CmdArgs& args);

    virtual void init(const CmdArgs& args);

    virtual void usage(const std::string& tool) const;

private:  // members
    bool verbose_;
    bool abort_;
    bool patch_;
    bool skip_;
    bool ignoreLength_;
    bool ignoreDate_;
    bool ignoreCentury_;
    bool ignoreType_;
    long timeThreshold_;
};

//----------------------------------------------------------------------------------------------------------------------

void BufrCheck::execute(const CmdArgs& args) {
    process(args(0), args(1));
}

void BufrCheck::init(const CmdArgs& args) {

    patch_ = args.getBool("patch-on-error", false);
    skip_ = args.getBool("skip-on-error", false);
    abort_ = args.getBool("abort-on-error", !(patch_ || skip_));

    if (abort_ + patch_ + skip_>1) {
        throw UserError("Inconsistent configuration. You can only specify one of [--abort-on-error, --patch-on-error, --skip-on-error]");
    }

    verbose_ = args.getBool("verbose", false);
    ignoreLength_ = args.getBool("dont-patch-length", false);
    ignoreDate_ = args.getBool("dont-patch-date", false);
    ignoreCentury_ = args.getBool("ignore-century", false);
    ignoreType_ = args.getBool("ignore-type", false);
    timeThreshold_ = args.getLong("acceptable-time-discrepancy", 300);
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
                << tool << " --patch-on-error --ignore-century --acceptable-time-discrepancy=600 input.bufr output.bufr" << std::endl
                << std::endl
                << tool << " --patch-on-error --dont-patch-date input.bufr output.bufr" << std::endl
                << std::endl;
}

Status BufrCheck::checkMessageLength(const message::Message& msg, int numMessage, eckit::StringDict& transformation) {

    long totalLength = msg.getLong("totalLength");
    long messageLength = msg.getLong("messageLength");

    if (totalLength != messageLength && totalLength < WRONG_KEY_LENGTH) {
        if (verbose_) {
            Log::error() << "message " << numMessage
                << ", wrong key length in bufr message " << messageLength << " instead of " << totalLength
                << std::endl;
        }
        if (!patch_ || ignoreLength_) {
            return Status::CORRUPTED;
        } else {
            transformation.emplace("messageLength", std::to_string(totalLength));
            return Status::FIXED;
        }
    }
    return Status::OK;
}

Status BufrCheck::checkSubType(const message::Message& msg, int numMessage) {

    long type = msg.getLong("rdbType");
    long subtype = msg.getLong("oldSubtype");
    long expectedType;

    if (codes::BUFRDecoder::typeBySubtype(subtype, expectedType)) {
        if (type == expectedType) {
            return Status::OK;
        } else {
            if (verbose_ || !ignoreType_) {
                Log::error() << "message " << numMessage
                    << ", type " << type << " and expected type " << expectedType << " don't match for subtype " << subtype
                    << std::endl;
            }
            return Status::CORRUPTED;
        }
    } else {
        if (verbose_ || !ignoreType_) {
            Log::error() << "message " << numMessage
                << ", unknown subtype " << subtype
                << std::endl;
        }
        return Status::CORRUPTED;
    }
}

Status BufrCheck::checkDate(const message::Message& msg, int numMessage, eckit::StringDict& transformation) {

    bool toFix = false;

    long localYear = msg.getLong("localYear");

    long typicalYear = msg.getLong("typicalYear");
    if (ignoreCentury_) {
        typicalYear = (localYear/100)*100 + typicalYear%100;
    }
    long typicalMonth = msg.getLong("typicalMonth");
    long typicalDay = msg.getLong("typicalDay");
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
        if (!patch_) {
            return Status::CORRUPTED;
        }
        toFix = !ignoreDate_;
    }

    long typicalHour = msg.getLong("typicalHour");
    long typicalMinute = msg.getLong("typicalMinute");
    long typicalSecond = msg.getLong("typicalSecond");

    if (typicalHour>23 || typicalHour<0 || typicalMinute>59 || typicalMinute<0 || typicalSecond>59 || typicalSecond<0) {
        if (verbose_) {
            Log::error() << "message " << numMessage
                << ", typical time is weird " << typicalHour << ":" << typicalMinute << ":" << typicalSecond
                << std::endl;
        }
        if (!patch_) {
            return Status::CORRUPTED;
        }
        toFix = !ignoreDate_;
    }
    long typicalTime = typicalHour*3600 + typicalMinute*60 + typicalSecond;


    long localMonth = msg.getLong("localMonth");
    long localDay = msg.getLong("localDay");
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

    long localHour = msg.getLong("localHour");
    long localMinute = msg.getLong("localMinute");
    long localSecond = msg.getLong("localSecond");

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
        if (msg.getLong("edition") == 3) {
            transformation.emplace("typicalYearOfCentury", std::to_string(localYear-2000));
        } else {
            transformation.emplace("typicalYear", std::to_string(localYear));
            transformation.emplace("typicalSecond", std::to_string(localSecond));
        }
        transformation.emplace("typicalMonth", std::to_string(localMonth));
        transformation.emplace("typicalDay", std::to_string(localDay));
        transformation.emplace("typicalHour", std::to_string(localHour));
        transformation.emplace("typicalMinute", std::to_string(localMinute));
        return Status::FIXED;
    }
    return Status::OK;
}

void BufrCheck::process(const PathName& input, const PathName& output) {

    message::Reader reader(input);
    FileHandle out(output.path());
    Offset pos;
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

    message::Message rawMsg;

    do {
        try {
            pos = reader.position();
            if ((rawMsg = reader.next())) {
                codes_handle* h = codes_handle_new_from_message(nullptr, rawMsg.data(), rawMsg.length());
                if(!h) {
                    throw FailedLibraryCall("eccodes", "codes_handle_new_from_message", "failed to create handle", Here());
                }
                codes::BufrContent* c = new codes::BufrContent(h, true);
                message::Message msg(c);

                // verify the presence of section 2 (to store the MARS key)
                long localSectionPresent = msg.getLong("localSectionPresent");
                ok = localSectionPresent != 0;
                if (!ok) {
                    missingKey++;
                } else {
                    eckit::StringDict transformation;
                    
                    switch (checkMessageLength(msg, numMessage, transformation)) {
                        case Status::CORRUPTED:
                            ok = false;
                            messageLength++;
                            break;
                        case Status::FIXED:
                            messageLength++;
                            break;
                        case Status::OK:
                            break;
                    };

                    switch (checkSubType(msg, numMessage)) {
                        case Status::CORRUPTED:
                            if (!ignoreType_) {
                                ok = false;
                            }
                            inconsistentSubType++;
                            break;
                        case Status::FIXED:
                            inconsistentSubType++;
                            break;
                        case Status::OK:
                            break;
                    };

                    switch (checkDate(msg, numMessage, transformation)) {
                        case Status::CORRUPTED:
                            ok = false;
                            inconsistentDate++;
                            break;
                        case Status::FIXED:
                            inconsistentDate++;
                            break;
                        case Status::OK:
                            break;
                    };

                    if (ok) {
                        if (transformation.size() == 0) {
                            msg.write(out);
                        } else {
                            eckit::message::Message m = msg.transform(transformation);
                            m.write(out);
                        }
                    }
                }

                if (!ok && abort_) {
                    Log::error() << "message " << numMessage << " not compliant" << std::endl;
                    exit(1);
                }
        
                numMessage++;
            }
        }
        catch(eckit::Exception& e) {
            Log::warning() << " Error parsing message " << numMessage << " - offset " << pos << std::endl;
            if (verbose_) {
                e.dumpStackTrace(Log::warning());
            }
        }
    } while ( pos != reader.position() );

    if (missingKey)
        Log::warning() << missingKey << " message" << (missingKey>1?"s miss ":" misses ") << " the MARS key" << std::endl;
    if (messageLength)
        Log::warning() << messageLength << " message" << (messageLength>1?"s ":" ") << " with incoherent message length in the MARS key" << std::endl;
    if (inconsistentSubType)
        Log::warning() << inconsistentSubType << " message" << (inconsistentSubType>1?"s ":" ") << " with unknown or inconsistent subtype" << std::endl;
    if (inconsistentDate)
        Log::warning() << inconsistentDate << " message" << (inconsistentDate>1? "s " : " ") << " with inconsistent date" << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    BufrCheck tool(argc, argv);
    return tool.start();
}
