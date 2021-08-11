//NTP Syncing - updates your time when charging to fix any drifting, or on demand via the menu
//Get your timezone from: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.json
#define TIMEZONE_STRING "EST5EDT,M3.2.0,M11.1.0"
#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "nist.time.gov"
#define NTP_SERVER_3 "0.ie.pool.ntp.org"

// Put your Access Point SSIDs and correpsonding passwords below. Put your most used accesspoint first.
const char* accessPoints[] = {"SSID1", "SSID2", "SSID3"};
const char* apPasswords[] = {"SSID1pass", "SSID2pass", "SSID3pass"};
const int accessPointCount = sizeof(accessPoints) / sizeof(accessPoints[0]); // number of known networks
