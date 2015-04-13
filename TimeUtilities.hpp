#ifndef TimeUtilities_hpp_
#define TimeUtilities_hpp_

#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <string>

namespace Tools {
    std::string getTimeStampString() {
        const boost::posix_time::ptime now =
            boost::posix_time::second_clock::local_time();
        return boost::posix_time::to_iso_string(now);
    }
}
#endif
