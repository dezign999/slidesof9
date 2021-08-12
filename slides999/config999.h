//NTP Syncing - updates your time twice a day to fix any drifting
//Enter your gmtOffset below, or leave it at "0" and the offset will be determined via http://ip-api.com automatically
const char* ntpServer = "pool.ntp.org";
RTC_DATA_ATTR int  gmtOffset = 0;

// Put your Access Point SSIDs and correpsonding passwords below. Put your most used accesspoint first.
const char* accessPoints[] = {"SSID1", "SSID2"};
const char* apPasswords[] = {"SSID1PASS", "SSID2PASS"};
const int accessPointCount = sizeof(accessPoints) / sizeof(accessPoints[0]); // number of known networks
