/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <unistd.h>

#include "eckit/config/Resource.h"
#include "eckit/log/Bytes.h"
#include "eckit/log/Log.h"
#include "eckit/serialisation/HandleStream.h"

#include "metkit/MarsHandle.h"




const unsigned long startCRC = 0xffffffffL;

eckit::ClassSpec MarsHandle::classSpec_ = {&TCPHandle::classSpec(),"MarsHandle",};
eckit::Reanimator<MarsHandle> MarsHandle::reanimator_;

class MarsHandleStream : public eckit::HandleStream {

    MarsHandle& handle_;

public:

    MarsHandleStream(MarsHandle& handle):
        eckit::HandleStream(handle), handle_(handle)
        { handle_.streamMode_ = true; }

    ~MarsHandleStream()
        { handle_.streamMode_ = false; }
};

void MarsHandle::encode(eckit::Stream& s) const
{
    TCPHandle::encode(s);
    s << clientID_;
    s << doCRC_;
}

MarsHandle::MarsHandle(eckit::Stream& s)
    : TCPHandle(s),
      length_(0),
      total_(0),
      receiving_(false),
      streamMode_(false),
      doCRC_(false),
      crc_(startCRC) {
    s >> clientID_;

    if(s.endObjectFound())
    {
        eckit::Log::info() << "Got old metkit without CRC" << std::endl;
        return;
    }

    s >> doCRC_;
    if(doCRC_) eckit::Log::info() << "Got new metkit with CRC" << std::endl;
}

MarsHandle::MarsHandle(const std::string& host, int port, unsigned long long clientID)
    : TCPHandle(host, port),
      clientID_(clientID),
      length_(0),
      total_(0),
      receiving_(false),
      streamMode_(false),
      doCRC_(false),
      crc_(startCRC) {}

MarsHandle::~MarsHandle()
{
}

std::string MarsHandle::title() const
{
    std::ostringstream os;

    os << "Client[" ;

    os << eckit::TCPSocket::hostName(host_);

    os << ":" << port_ << "]";
    return os.str();
}

eckit::Length MarsHandle::openForRead()
{
    static long size = eckit::Resource<long>("archiveSocketBufferSize",0);

    connection_.bufferSize(size);

    eckit::TCPHandle::openForRead();

    MarsHandleStream s(*this);


    s << clientID_;     // Send info
    s << 'r';     // Send the request
    s >> length_; // Get the length back

    eckit::Log::status() << "Receiving " << eckit::Bytes(length_) << std::endl;

    total_     = 0;
    receiving_ = true;
    crc_       = startCRC;

    return length_;
}

void MarsHandle::openForWrite(const eckit::Length& length)
{
    eckit::TCPHandle::openForWrite(length);

    MarsHandleStream s(*this);

    length_ = length;

    s << clientID_; // Send info
    s << 'w';
    s << length_; // Send the length

    eckit::Log::status() << "Sending " << eckit::Bytes(length_) << std::endl;

    total_     = 0;
    receiving_ = false;
}

void MarsHandle::openForAppend(const eckit::Length&)
{
    NOTIMP;
}

long MarsHandle::read(void *buffer,long length)
{
    if(streamMode_) return TCPHandle::read(buffer,length);

    long long  left = length_ - total_;

    if(left < length) {
        length = left;
        if(length == 0) return 0;
    }

    long len = TCPHandle::read(buffer,length);

    if(doCRC_) updateCRC(buffer,len);

    total_ += len;

    return len;
}

long MarsHandle::write(const void *buffer,long length)
{
    long len = TCPHandle::write(buffer,length);
    total_ += len;
    return len;
}

void MarsHandle::close()
{
    bool gotCRC   = false;
    unsigned long version = 0;
    unsigned long crc = 0;

    if(length_ > 0 && total_ != length_)
    {
        TCPHandle::close();

		eckit::Log::error() << "Recieved/Sent " << total_
					 << " bytes instead of " << length_ << std::endl;
        if(eckit::Exception::throwing())
        {
            eckit::Log::error() << "A expection is already active" << std::endl;
            return;
        }
        throw eckit::ShortFile("Bad total in MarsHandle");
	}

    if(receiving_)
    {

        crc_ ^= 0xffffffff;

        length_ = 0;

        // Try to read CRC


        try {
            MarsHandleStream s(*this);
            s >> version;

            unsigned long long c;
            s >> c;

            crc = ((unsigned long)(c));

            gotCRC = true;

        }
        catch(std::exception& e)
        {
            eckit::Log::warning() << "Cannot read crc: " << e.what() << std::endl;
        }

    }


    if(doCRC_ && gotCRC)
    {
        eckit::Log::info() << "Local CRC " << crc_ << ", remote CRC " << crc << std::endl;
        ASSERT(version == 1);
        if(crc != crc_) {

            {
                FILE *p = popen("mail mab mar","w");
                if(p) {
                    fprintf(p,"CRC error\n");
                    pclose(p);
                }
            }

            {
                eckit::PathName lock("~/locks/pause_if_crc_error");
                while(lock.exists())
                {
                    eckit::Log::status() << "**** CRC ERROR ****" << std::endl;
                    ::sleep(120);
                }
            }

            eckit::TCPHandle::close();

            throw eckit::SeriousBug("Invalide checksum");
        }
    }

    TCPHandle::close();

}

eckit::Length MarsHandle::estimate()
{
    return length_;
}


void MarsHandle::updateCRC(void* buffer,long length)
{
}
