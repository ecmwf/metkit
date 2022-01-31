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

class BufrCheck : public MetkitTool {
public:

    BufrCheck(int argc, char **argv) : MetkitTool(argc, argv) {
        options_.push_back(
            new SimpleOption<bool>("fixDate", "Fix date in case of inconsistency, default = false"));
        options_.push_back(
            new SimpleOption<bool>("ignoreDate", "Ignore date in case of inconsistency, default = false"));
        options_.push_back(
            new SimpleOption<bool>("fixKey", "Fix key length in case of inconsistency, default = false"));

    }

    virtual ~BufrCheck() {}

private:  // methods
    int minimumPositionalArguments() const { return 2; }

    bool checkKeyLength(codes_handle* h, int cnt);
    bool checkSubType(codes_handle* h, int cnt);
    bool checkDate(codes_handle* h, int cnt);

    void process(const eckit::PathName& input, const eckit::PathName& output);

    virtual void execute(const eckit::option::CmdArgs& args);

    virtual void init(const CmdArgs& args);

    virtual void usage(const std::string& tool) const;

private:  // members
    bool fixDate_ = false;
    bool ignoreDate_ = false;
    bool fixKey_ = false;
};

//----------------------------------------------------------------------------------------------------------------------

void BufrCheck::execute(const eckit::option::CmdArgs& args) {
    process(args(0), args(1));
}

void BufrCheck::init(const CmdArgs& args) {
    args.get("fixDate", fixDate_);
    args.get("ignoreDate", ignoreDate_);
    args.get("fixKey", fixKey_);
}

void BufrCheck::usage(const std::string& tool) const {
    Log::info() << "Usage: " << tool << " [options] [input] [output]" << std::endl
                << std::endl;

    Log::info() << "Examples:" << std::endl
                << "=========" << std::endl
                << std::endl
                << tool << " input.bufr output.bufr" << std::endl
                << tool << " --fixKey --ignoreDate input.bufr output.bufr" << std::endl
                << std::endl;
}

bool BufrCheck::checkKeyLength(codes_handle* h, int cnt) {
    long totalLength;
    long messageLength;

    codes_get_long(h, "totalLength", &totalLength);
    codes_get_long(h, "messageLength", &messageLength);

    if (totalLength != messageLength && totalLength < WRONG_KEY_LENGTH) {
        Log::error() << "message " << cnt
            << ", wrong key length in bufr message " << messageLength << " instead of " << totalLength
            << std::endl;
        if (fixKey_) {
            codes_set_long(h, "messageLength", totalLength);
        }
        else {
            return false;
        }
    }
    return true;
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

bool BufrCheck::checkSubType(codes_handle* h, int cnt) {
    long type;
    long subtype;

    codes_get_long(h, "rdbType", &type);
    codes_get_long(h, "oldSubtype", &subtype);

    if (kTypes[subtype] != type) {
        Log::error() << "message " << cnt
            << ", type " << type << " and known type " << kTypes[subtype] << " don't match for subtype " << subtype
            << std::endl;
        return false;
    }
    return true;
}

bool BufrCheck::checkDate(codes_handle* h, int cnt) {

    bool toFix = false;

    long typicalDate;
    long typicalYearOfCentury;
    long typicalMonth;
    long typicalDay;
    long typicalHour;
    long typicalMinute;
    codes_get_long(h, "typicalDate", &typicalDate);
    codes_get_long(h, "typicalYearOfCentury", &typicalYearOfCentury);
    codes_get_long(h, "typicalMonth", &typicalMonth);
    codes_get_long(h, "typicalDay", &typicalDay);
    codes_get_long(h, "typicalHour", &typicalHour);
    codes_get_long(h, "typicalMinute", &typicalMinute);
    long typicalYear = typicalDate/10000;
    auto typicalYMD = year(typicalYear)/typicalMonth/typicalDay;
    long typicalTime = typicalHour*3600 + typicalMinute*60;

    if ((typicalYear%100) != typicalYearOfCentury || !typicalYMD.ok()) {
        Log::error() << "message " << cnt
            << ", date is weird " << typicalYMD.year() << "/" << typicalYMD.month() << "/" << typicalYMD.day()
            << std::endl;
        if (!fixDate_ && !ignoreDate_)
            return false;

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
        Log::error() << "message " << cnt
            << ", local date is weird " << localYMD.year() << "/" << localYMD.month() << "/" << localYMD.day()
            << std::endl;
        return false;
    }
    if (localYMD != typicalYMD || abs(typicalTime-localTime)>300) {
        Log::error() << "message " << cnt << ", date-time and local date-time differs" << std::endl;
        toFix = !ignoreDate_;
    }

    if (toFix) {
        codes_set_long(h, "typicalDate", localYear*1000+localMonth*100+localDay);
        codes_set_long(h, "typicalYearOfCentury", localYear%100);
        codes_set_long(h, "typicalMonth", localMonth);
        codes_set_long(h, "typicalDay", localDay);
        codes_set_long(h, "typicalHour", localHour);
        codes_set_long(h, "typicalMinute", localMinute);
    }
    return true;
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
    int cnt=0;
 	void *buffer = NULL;
	size_t size = 0;
    bool ok;
    /* loop over the messages in the bufr file */
    while ((h = codes_handle_new_from_file(NULL, in, PRODUCT_BUFR, &err)) != NULL || err != CODES_SUCCESS)
    {
        if (h == NULL) {
            Log::error() << "unable to create handle for message " << cnt << std::endl;
            cnt++;
            continue;
        }

        ok = checkKeyLength(h, cnt);
        ok &= checkSubType(h, cnt);
        ok &= checkDate(h, cnt);

        if (ok) {
            CODES_CHECK(codes_get_message(h,const_cast<const void**>(&buffer),&size),0);
            
            out.write(buffer, size);
        }
 
        /* delete handle */
        codes_handle_delete(h);
 
        cnt++;
    }
 
    fclose(in);
    out.close();
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    BufrCheck tool(argc, argv);
    return tool.start();
}
