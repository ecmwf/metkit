/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File DHSProtocol.cc
// Baudouin Raoult - (c) ECMWF Feb 12

#include "metkit/DHSProtocol.h"

#include "eckit/net/TCPClient.h"
#include "eckit/net/TCPStream.h"

#include "metkit/ClientTask.h"
#include "metkit/RequestEnvironment.h"
#include "eckit/config/Configuration.h"


namespace metkit {

static eckit::Reanimator<DHSProtocol> dhsProtocolReanimator;

DHSProtocol::DHSProtocol(const std::string& name,
                         const std::string& host,
                         int port,
                         bool forwardMessages )
    : name_(name),
      host_(host),
      port_(port),
      done_(false),
      error_(false),
      sending_(false),
      foreward_(forwardMessages)
{}

DHSProtocol::DHSProtocol(const eckit::Configuration& params):
    BaseProtocol(params),
    name_(params.getString("name")),
    host_(params.getString("host")),
    port_(params.getInt("port", 9000)),
    done_(false),
    error_(false),
    sending_(false),
    foreward_(false)
{
}

DHSProtocol::DHSProtocol(eckit::Stream& s):
    BaseProtocol(s) {
    s >> name_;
    s >> host_;
    s >> port_;
    s >> done_;
    s >> error_;
    s >> sending_;
    s >> foreward_;
}

DHSProtocol::~DHSProtocol()
{
    done_ = true;
    cleanup();
}

const eckit::ReanimatorBase& DHSProtocol::reanimator() const {
    return dhsProtocolReanimator;
}

const eckit::ClassSpec& DHSProtocol::classSpec() {
    static eckit::ClassSpec spec = { &BaseProtocol::classSpec(), "DHSProtocol" };
    return spec;
}

eckit::Length DHSProtocol::retrieve(const MarsRequest& request)
{
    std::string host = callback_.localHost();
    int    port = callback_.localPort();

    eckit::Log::info() << "DHSProtocol: call back on " << host << ":" << port << std::endl;

    task_.reset(new ClientTask(request, RequestEnvironment::instance().request(), host, port));

    eckit::TCPStream s(eckit::TCPClient().connect(host_, port_));

    task_->send(s);

    ASSERT(task_->receive(s) == 'a'); // Acknoledgement

    eckit::Length result = 0;
    while (wait(result)) {
        ;
    }

    eckit::Log::info() << "DHSProtocol::retrieve " << result << std::endl;
    return result;
}

void DHSProtocol::archive(const MarsRequest& request, const eckit::Length& size)
{
    std::string host = callback_.localHost();
    int    port = callback_.localPort();

    eckit::Log::info() << "DHSProtocol::archive " << size << std::endl;
    eckit::Log::info() << "DHSProtocol: call back on " << host << ":" << port << std::endl;

    task_.reset(new ClientTask(request, RequestEnvironment::instance().request(), host, port));

    eckit::TCPStream s(eckit::TCPClient().connect(host_, port_));

    task_->send(s);

    eckit::Log::info() << "DHSProtocol: task sent." << std::endl;

    ASSERT(task_->receive(s) == 'a'); // Acknoledgement

    eckit::Length result = size;
    while (wait(result)) {
        ;
    }
    eckit::Log::info() << "DHSProtocol: archive completed." << std::endl;
}

void DHSProtocol::cleanup()
{
    if (socket_.isConnected())
    {
        if ( sending_ )
        {
            unsigned long version = 1;
            unsigned long long crc = 0;

            try {
                eckit::InstantTCPStream s(socket_);
                s << version;
                s << crc;
            }
            catch (std::exception& e)
            {
                eckit::Log::error() << "** " << e.what() << " Caught in " << Here() << std::endl;
                eckit::Log::error() << "** Exception is ignored" << std::endl;
            }
        }
        socket_.close();
    }

    sending_ = false;

    if (!done_) {
        eckit::Length result = 0;
        while (wait(result)) {
            ;
        }
    }

    if (error_)
    {
        error_ = false;
        throw eckit::UserError(std::string("Error from [") + name_ + "]: "  + msg_);
    }
}

void DHSProtocol::print(std::ostream& s) const
{
    s << "DHSProtocol[" << name_ << "]";
}

void DHSProtocol::encode(eckit::Stream& s) const {
    BaseProtocol::encode(s);
    s << name_;
    s << host_;
    s << port_;
    s << done_;
    s << error_;
    s << sending_;
    s << foreward_;
}

long DHSProtocol::read(void* buffer, long len)
{
    return socket_.read(buffer, len);
}

long DHSProtocol::write(const void* buffer, long len)
{
    return socket_.write(buffer, len);
}

bool DHSProtocol::wait(eckit::Length& size)
{
    for (;;) {

        socket_ = callback_.accept();

        eckit::InstantTCPStream s(socket_);

        char code = task_->receive(s);

        eckit::Log::debug() << "DHSProtocol: code [" << code << "]" << std::endl;

        std::string msg;
        long long bytes;

        switch (code) {
        /* OK */
        case 'o':
            done_ = true;
            return false;
            break;

        /* read source */
        case 'r':
            bytes = size;
            eckit::Log::debug() << "DHSProtocol:r [" << bytes << "]" << std::endl;
            s << bytes;
            sending_ = true;
            return false;
            break;

        /* get */
        case 'h':
            NOTIMP;
            break;

        case 'w':
            s >> bytes;
            eckit::Log::debug() << "DHSProtocol:w " << bytes << std::endl;
            size = bytes;
            return false;
            break;

        case 'm':
            NOTIMP;
            break;

        case 'X':
            NOTIMP;
            break;

        case 'e':
            s >> msg_;
            eckit::Log::error() << msg_ << " [" << name_ << "]" << std::endl;
            error_ = true;
            done_ = true;
            return false;
            break;

        case 'y':    /* retry */
            NOTIMP;
            break;

        case 'I': /* info */
            s >> msg;
            eckit::Log::info() << msg << " [" << name_ << "]" << std::endl;
            if (foreward_) {
                eckit::Log::userInfo() << msg << " [" << name_ << "]" << std::endl;
            }
            break;

        case 'W': /* warning */
            s >> msg;
            eckit::Log::warning() << msg << " [" << name_ << "]" << std::endl;
            if (foreward_) {
                eckit::Log::userWarning() << msg << " [" << name_ << "]" << std::endl;
            }
            break;

        case 'D': /* debug */
            s >> msg;
            eckit::Log::debug() << msg << " [" << name_ << "]" << std::endl;
            if (foreward_) {
                eckit::Log::userInfo() << msg << " [" << name_ << "]" << std::endl;
            }
            break;

        case 'E': /* error */
            s >> msg;
            eckit::Log::error() << msg << " [" << name_ << "]" << std::endl;
            if (foreward_) {
                eckit::Log::userError() << msg << " [" << name_ << "]" << std::endl;
            }
            break;

        case 'N': /* notification */
            NOTIMP;
            break;

        case 'p': /* ping */
            s << 'p';
            break;

        case 's': /* statistics */
        {
            int n;
            s >> n;
            std::string key, value;
            for (int i = 0; i < n; i++)
            {
                s >> key >> value;
                eckit::Log::info() << "DHSProtocol:s " << key << "=" << value << std::endl;
            }
        }
        break;

        case 'S': /* notification start */
            NOTIMP;
            break;

        case 't': /* new timeout */
            NOTIMP;
            break;

        default:
            throw eckit::Exception(std::string("Unknown code [") + code + "]");
            break;
        }
    }
}

static ProtocolBuilder<DHSProtocol> builder("dhsbase");

}
