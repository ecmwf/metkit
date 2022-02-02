/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <chrono> 
#include "date.h" 

#include "eccodes.h"

#include "eckit/io/FileHandle.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "metkit/tool/MetkitTool.h"

#define MAX_VAL_LEN 1024
#define WRONG_KEY_LENGTH 65535

using namespace date;

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
            new SimpleOption<bool>("skip", "Skip broken messages, default = abort"));
        options_.push_back(
            new SimpleOption<bool>("fixDate", "Fix date in case of inconsistency, default = false"));
        options_.push_back(
            new SimpleOption<bool>("ignoreDate", "Ignore date in case of inconsistency, default = false"));
        options_.push_back(
            new SimpleOption<bool>("fixLength", "Fix message length in the key in case of inconsistency, default = false"));

    }

    virtual ~BufrCheck() {}

private:  // methods
    int minimumPositionalArguments() const { return 2; }

    Status checkMessageLength(codes_handle* h, int numMessages);
    Status checkSubType(codes_handle* h, int numMessages);
    Status checkDate(codes_handle* h, int numMessages);

    void process(const eckit::PathName& input, const eckit::PathName& output);

    virtual void execute(const eckit::option::CmdArgs& args);

    virtual void init(const CmdArgs& args);

    virtual void usage(const std::string& tool) const;

private:  // members
    bool skip_ = false;
    bool fixDate_ = false;
    bool ignoreDate_ = false;
    bool fixLength_ = false;
};

//----------------------------------------------------------------------------------------------------------------------

void BufrCheck::execute(const eckit::option::CmdArgs& args) {
    process(args(0), args(1));
}

void BufrCheck::init(const CmdArgs& args) {
    args.get("skip", skip_);
    args.get("fixDate", fixDate_);
    args.get("ignoreDate", ignoreDate_);
    args.get("fixLength", fixLength_);
}

void BufrCheck::usage(const std::string& tool) const {
    Log::info() << "Usage: " << tool << " [options] [input] [output]" << std::endl
                << std::endl;

    Log::info() << "Examples:" << std::endl
                << "=========" << std::endl
                << std::endl
                << tool << " --skip input.bufr output.bufr" << std::endl
                << tool << " --fixLength --ignoreDate input.bufr output.bufr" << std::endl
                << std::endl;
}

Status BufrCheck::checkMessageLength(codes_handle* h, int numMessages) {
    long totalLength;
    long messageLength;

    codes_get_long(h, "totalLength", &totalLength);
    codes_get_long(h, "messageLength", &messageLength);

    if (totalLength != messageLength && totalLength < WRONG_KEY_LENGTH) {
        Log::error() << "message " << numMessages
            << ", wrong key length in bufr message " << messageLength << " instead of " << totalLength
            << std::endl;
        if (fixLength_) {
            codes_set_long(h, "messageLength", totalLength);
            return Status::FIXED;
        }
        else {
            return Status::CORRUPTED;
        }
    }
    return Status::OK;
}

static long kTypes[256] = {
    -1, 1, 1, 1, 1, -1, -1, 1, -1, 1, -1, 1, 1, 1, 1, -1,           /* 15 */
    -1, -1, -1, 1, -1, 1, 1, 1, -1, -1, 1, -1, 1, -1, -1, 8,        /* 31	 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 47 */
    -1, 2, -1, 2, -1, 2, 2, 2, -1, 2, -1, 2, 2, 2, 2, 2,            /* 63 */
    -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* 79 */
    -1, -1, 3, 3, 3, 3, 3, 3, 3, 3, -1, 4, 4, -1, -1, 4,            /* 95 */
    4, -1, -1, -1, -1, 5, 5, 5, -1, -1, 5, -1, -1, 5, 1, 5,         /* 111 */
    5, 5, -1, -1, -1, -1, -1, -1, -1, 12, 12, 12, -1, 6, 12, 12,    /* 127 */
    -1, 2, -1, 6, 6, 6, -1, -1, 12, 12, 12, 12, 1, -1, 7, 7,        /* 143 */
    7, 7, 7, 1, 7, 7, 7, 7, -1, 12, 2, 2, 2, -1, -1, -1,            /* 159 */
    -1, -1, -1, -1, 10, 1, -1, -1, -1, -1, 1, -1, 1, -1, -1, -1,    /* 175 */
    1, -1, 1, -1, 1, 1, 1, -1, -1, -1, -1, -1, -1, 3, 3, -1,        /* 191 */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 12, 30, 30, -1, 2, 2,   /* 207 */
    2, 2, 12, 2, 3, 12, 12, -1, 2, 12, 12, -1, -1, -1, -1, -1,      /* 223 */
    12, -1, -1, -1, -1, -1, 5, 5, -1, -1, -1, -1, -1, 1, -1, -1,    /* 239 */
    2, -1, -1, -1, -1, -1, -1, -1, -1, -1, 2, 2, -1, -1, -1, 2,     /* 255 */
};

Status BufrCheck::checkSubType(codes_handle* h, int numMessages) {
    long type;
    long subtype;

    codes_get_long(h, "rdbType", &type);
    codes_get_long(h, "oldSubtype", &subtype);

    if (kTypes[subtype] != type) {
        Log::error() << "message " << numMessages
            << ", type " << type << " and known type " << kTypes[subtype] << " don't match for subtype " << subtype
            << std::endl;
        return Status::CORRUPTED;
    }
    return Status::OK;
}

Status BufrCheck::checkDate(codes_handle* h, int numMessages) {

    bool toFix = false;

    long edition;

    long typicalDate;
    long typicalYear;
    long typicalYearOfCentury;
    long typicalMonth;
    long typicalDay;
    long typicalHour;
    long typicalMinute;
    long typicalSecond = 0;
    codes_get_long(h, "typicalDate", &typicalDate);
    codes_get_long(h, "typicalMonth", &typicalMonth);
    codes_get_long(h, "typicalDay", &typicalDay);
    codes_get_long(h, "typicalHour", &typicalHour);
    codes_get_long(h, "typicalMinute", &typicalMinute);
    codes_get_long(h, "edition", &edition);
    if (edition == 3) {
        codes_get_long(h, "typicalYearOfCentury", &typicalYearOfCentury);
        typicalYear = typicalDate/10000;
    } else {
        codes_get_long(h, "typicalYear", &typicalYear);
        typicalYearOfCentury = typicalYear-2000;
        codes_get_long(h, "typicalSecond", &typicalSecond);
    }
    auto typicalYMD = year(typicalYear)/typicalMonth/typicalDay;
    long typicalTime = typicalHour*3600 + typicalMinute*60 + typicalSecond;

    if (!typicalYMD.ok()) {
        Log::error() << "message " << numMessages
            << ", date is weird " << typicalYMD.year() << "/" << typicalYMD.month() << "/" << typicalYMD.day()
            << std::endl;
        if (!fixDate_ && !ignoreDate_)
            return Status::CORRUPTED;

        toFix = !ignoreDate_;
    }

    long localYear;
    long localMonth;
    long localDay;
    long localHour;
    long localMinute;
    long localSecond;

    codes_get_long(h, "localYear", &localYear);
    codes_get_long(h, "localMonth", &localMonth);
    codes_get_long(h, "localDay", &localDay);
    codes_get_long(h, "localHour", &localHour);
    codes_get_long(h, "localMinute", &localMinute);
    codes_get_long(h, "localSecond", &localSecond);

    auto localYMD = year(localYear)/localMonth/localDay;
    long localTime = localHour*3600 + localMinute*60 + localSecond;

    if (!localYMD.ok()) {
        Log::error() << "message " << numMessages
            << ", local date is weird " << localYMD.year() << "/" << localYMD.month() << "/" << localYMD.day()
            << std::endl;
        return Status::CORRUPTED;
    }
    if (localYMD != typicalYMD || abs(typicalTime-localTime)>300) {
        Log::error() << "message " << numMessages << ", date-time and local date-time differs" << std::endl;
        toFix = !ignoreDate_;
    }

    if (toFix) {
        codes_set_long(h, "typicalDate", localYear*1000+localMonth*100+localDay);
        if (edition == 3) {
            codes_set_long(h, "typicalYearOfCentury", localYear-2000);
        } else {
            codes_set_long(h, "typicalYear", localYear);
            codes_set_long(h, "typicalSecond", localSecond);
        }
        codes_set_long(h, "typicalMonth", localMonth);
        codes_set_long(h, "typicalDay", localDay);
        codes_set_long(h, "typicalHour", localHour);
        codes_set_long(h, "typicalMinute", localMinute);
        return Status::FIXED;
    }
    return Status::OK;
}

void BufrCheck::process(const eckit::PathName& input, const eckit::PathName& output)
{
    FILE* in = fopen(input.path().c_str(),"rb");
    if (!in) {
        Log::error() << "unable to open input file " << input.path() << std::endl;
        return;
    }

    eckit::FileHandle out(output.path());
    out.openForWrite(0);
    
    /* message handle. Required in all the eccodes calls acting on a message.*/
    codes_handle* h=NULL;
 
    int err=0;
 	void *buffer = NULL;
	size_t size = 0;
    bool ok;
    unsigned int numMessages=0;
    unsigned int missingKey=0;
    unsigned int messageLength=0;
    unsigned int inconsistentSubType=0;
    unsigned int inconsistentDate=0;
    /* loop over the messages in the bufr file */
    while ((h = codes_handle_new_from_file(NULL, in, PRODUCT_BUFR, &err)) != NULL || err != CODES_SUCCESS)
    {
        if (h == NULL) {
            Log::error() << "unable to create handle for message " << numMessages << std::endl;
            numMessages++;
            continue;
        }

        // verify the presence of section 2 (to store the MARS key)
        long section1Flags;
        codes_get_long(h, "section1Flags", &section1Flags);

        ok = section1Flags != 0;
        if (!ok) {
            missingKey++;
        } else {
            switch (checkMessageLength(h, numMessages)) {
                case Status::CORRUPTED:
                    ok = false;
                    messageLength++;
                    break;
                case Status::FIXED:
                    messageLength++;
                    break;
                case Status::OK: ;
            };
            switch (checkSubType(h, numMessages)) {
                case Status::CORRUPTED:
                    ok = false;
                    inconsistentSubType++;
                    break;
                case Status::FIXED:
                    inconsistentSubType++;
                    break;
                case Status::OK: ;
            };
            switch (checkDate(h, numMessages)) {
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
            CODES_CHECK(codes_get_message(h,const_cast<const void**>(&buffer),&size),0);
            
            out.write(buffer, size);
        } else if (!skip_) {
            Log::error() << "message " << numMessages << " not compliant" << std::endl;
            fclose(in);
            out.close();
            exit(1);
        }
 
        /* delete handle */
        codes_handle_delete(h);
 
        numMessages++;
    }

    if (missingKey)
        Log::warning() << missingKey << " message" << (missingKey>1?"s miss ":" misses ") << " the MARS key" << std::endl;
    if (messageLength)
        Log::warning() << messageLength << " message" << (messageLength>1?"s ":" ") << " with incoherent message length in the MARS key" << std::endl;
    if (inconsistentSubType)
        Log::warning() << inconsistentSubType << " message" << (inconsistentSubType>1?"s ":" ") << " with unknown subtype" << std::endl;
    if (inconsistentDate)
        Log::warning() << inconsistentDate << " message" << (inconsistentDate>1? "s " : " ") << " with inconsistent date" << std::endl;

    fclose(in);
    out.close();
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    BufrCheck tool(argc, argv);
    return tool.start();
}
