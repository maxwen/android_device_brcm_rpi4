#define LOG_TAG  "gps_zkw"
#define LOG_NDEBUG 0

#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <log/log.h>

#define LOCATION_NLP_FIX "/data/vendor/location/LOCATION.DAT"
#define C_INVALID_FD -1
#include <cutils/sockets.h>
#include <cutils/properties.h>
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#endif
//#include <hardware/gps7.h>
#include <hardware/gps.h>

/* the name of the controlled socket */
/* TODO property */
#define GPS_CHANNEL_NAME        "/dev/ttyACM0"
#define TTY_BAUD                B115200   // B115200

#define  GPS_DEBUG  1
#define  NEMA_DEBUG 1   /*the flag works if GPS_DEBUG is defined*/
#if GPS_DEBUG
#define TRC(f)      ALOGD("%s", __func__)
#define ERR(f, ...) ALOGE("%s: line = %d, " f, __func__, __LINE__, ##__VA_ARGS__)
#define WAN(f, ...) ALOGW("%s: line = %d, " f, __func__, __LINE__, ##__VA_ARGS__)
#define DBG(f, ...) ALOGD("%s: line = %d, " f, __func__, __LINE__, ##__VA_ARGS__)
#define VER(f, ...) ((void)0)    // ((void)0)   //
#else
#  define DBG(...)    ((void)0)
#  define VER(...)    ((void)0)
#  define ERR(...)    ((void)0)
#endif
static int flag_unlock = 0;
GpsStatus gps_status;
const char* gps_native_thread = "GPS NATIVE THREAD";
static GpsCallbacks callback_backup;
static float report_time_interval = 0;
static int started = 0;

/*****************************************************************************/

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       C O N N E C T I O N   S T A T E                 *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

/* commands sent to the gps thread */
enum {
        CMD_QUIT  = 0,
        CMD_START = 1,
        CMD_STOP  = 2,
        CMD_RESTART = 3,
        CMD_DOWNLOAD = 4,

        CMD_TEST_START = 10,
        CMD_TEST_STOP = 11,
        CMD_TEST_SMS_NO_RESULT = 12,
};

static int gps_nmea_end_tag = 0;

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       N M E A   T O K E N I Z E R                     *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

typedef struct {
        const char*  p;
        const char*  end;
} Token;

#define  MAX_NMEA_TOKENS  24

typedef struct {
        int     count;
        Token   tokens[ MAX_NMEA_TOKENS ];
} NmeaTokenizer;

static int
nmea_tokenizer_init(NmeaTokenizer*  t, const char*  p, const char*  end)
{
        int    count = 0;
        char*  q;

        //  the initial '$' is optional
        if (p < end && p[0] == '$')
                p += 1;

        //  remove trailing newline
        if (end > p && (*(end-1) == '\n')) {
                end -= 1;
                if (end > p && (*(end-1) == '\r'))
                        end -= 1;
        }

        //  get rid of checksum at the end of the sentecne
        if (end >= p+3 && (*(end-3) == '*')) {
                end -= 3;
        }

        while (p < end) {
                const char*  q = p;

                q = memchr(p, ',', end-p);
                if (q == NULL)
                        q = end;

                if (q >= p) {
                        if (count < MAX_NMEA_TOKENS) {
                                t->tokens[count].p   = p;
                                t->tokens[count].end = q;
                                count += 1;
                        }
                }
                if (q < end)
                        q += 1;

                p = q;
        }

        t->count = count;
        return count;
}

static Token
nmea_tokenizer_get(NmeaTokenizer*  t, int  index)
{
        Token  tok;
        static const char*  dummy = "";

        if (index < 0 || index >= t->count) {
                tok.p = tok.end = dummy;
        } else
                tok = t->tokens[index];

        return tok;
}


static int
str2int(const char*  p, const char*  end)
{
        int   result = 0;
        int   len    = end - p;
        int   sign = 1;

        if (*p == '-')
        {
                sign = -1;
                p++;
                len = end - p;
        }

        for (; len > 0; len--, p++)
        {
                int  c;

                if (p >= end)
                        goto Fail;

                c = *p - '0';
                if ((unsigned)c >= 10)
                        goto Fail;

                result = result*10 + c;
        }
        return  sign*result;

Fail:
        return -1;
}

static double
str2float(const char*  p, const char*  end)
{
        int   result = 0;
        int   len    = end - p;
        char  temp[16];

        if (len >= (int)sizeof(temp))
                return 0.;

        memcpy(temp, p, len);
        temp[len] = 0;
        return strtod(temp, NULL);
}

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       N M E A   P A R S E R                           *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

// #define  NMEA_MAX_SIZE  83
#define  NMEA_MAX_SIZE  128
/*maximum number of SV information in GPGSV*/
#define  NMEA_MAX_SV_INFO 4
#define  LOC_FIXED(pNmeaReader) ((pNmeaReader->fix_mode == 2) || (pNmeaReader->fix_mode ==3))
typedef struct {
        int     pos;
        int     overflow;
        int     utc_year;
        int     utc_mon;
        int     utc_day;
        int     utc_diff;
        GpsLocation  fix;

        /*
         * The fix flag extracted from GPGSA setence: 1: No fix; 2 = 2D; 3 = 3D
         * if the fix mode is 0, no location will be reported via callback
         * otherwise, the location will be reported via callback
         */
        int     fix_mode;
        /*
         * Indicate that the status of callback handling.
         * The flag is used to report GPS_STATUS_SESSION_BEGIN or GPS_STATUS_SESSION_END:
         * (0) The flag will be set as true when callback setting is changed via nmea_reader_set_callback
         * (1) GPS_STATUS_SESSION_BEGIN: receive location fix + flag set + callback is set
         * (2) GPS_STATUS_SESSION_END:   receive location fix + flag set + callback is null
         */
        int     cb_status_changed;
        int     sv_count;           /*used to count the number of received SV information*/
        GpsSvStatus   sv_status_gps;
        GnssSvStatus  sv_status_gnss;
        GpsCallbacks  callbacks;
        char    in[ NMEA_MAX_SIZE+1 ];
        int     sv_status_can_report;
        int     location_can_report;
        int 	sv_used_in_fix[256];
} NmeaReader;


static void
nmea_reader_update_utc_diff(NmeaReader* const r)
{
        time_t         now = time(NULL);
        struct tm      tm_local;
        struct tm      tm_utc;
        unsigned long  time_local, time_utc;

        gmtime_r(&now, &tm_utc);
        localtime_r(&now, &tm_local);


        time_local = mktime(&tm_local);


        time_utc = mktime(&tm_utc);

        r->utc_diff = time_utc - time_local;
}


static void
nmea_reader_init(NmeaReader* const r)
{
        memset(r, 0, sizeof(*r));

        r->pos      = 0;
        r->overflow = 0;
        r->utc_year = -1;
        r->utc_mon  = -1;
        r->utc_day  = -1;
        r->utc_diff = 0;
        r->callbacks.location_cb= NULL;
        r->callbacks.status_cb= NULL;
        r->callbacks.sv_status_cb= NULL;
        r->sv_count = 0;
        r->fix_mode = 0;    /*no fix*/
        r->cb_status_changed = 0;
        memset((void*)&r->sv_status_gps, 0x00, sizeof(r->sv_status_gps));
        memset((void*)&r->sv_status_gnss, 0x00, sizeof(r->sv_status_gnss));
        memset((void*)&r->in, 0x00, sizeof(r->in));

        nmea_reader_update_utc_diff(r);
}

static void
nmea_reader_set_callback(NmeaReader* const r, GpsCallbacks* const cbs)
{
        if (!r) {           /*this should not happen*/
                return;
        } else if (!cbs) {  /*unregister the callback */
                return;
        } else {/*register the callback*/
                r->fix.flags = 0;
                r->sv_count = 0;
                r->sv_status_gps.num_svs = 0;
                r->sv_status_gnss.num_svs = 0;
        }
}


static int
nmea_reader_update_time(NmeaReader* const r, Token  tok)
{
        int        hour, minute;
        double     seconds;
        struct tm  tm;
        struct tm  tm_local;
        time_t     fix_time;

        if (tok.p + 6 > tok.end)
                return -1;

        memset((void*)&tm, 0x00, sizeof(tm));
        if (r->utc_year < 0) {
                //  no date yet, get current one
                time_t  now = time(NULL);
                gmtime_r(&now, &tm);
                r->utc_year = tm.tm_year + 1900;
                r->utc_mon  = tm.tm_mon + 1;
                r->utc_day  = tm.tm_mday;
        }

        hour    = str2int(tok.p,   tok.p+2);
        minute  = str2int(tok.p+2, tok.p+4);
        seconds = str2float(tok.p+4, tok.end);

        tm.tm_hour = hour;
        tm.tm_min  = minute;
        tm.tm_sec  = (int) seconds;
        tm.tm_year = r->utc_year - 1900;
        tm.tm_mon  = r->utc_mon - 1;
        tm.tm_mday = r->utc_day;
        tm.tm_isdst = -1;

        if (mktime(&tm) == (time_t)-1)
                ERR("mktime error: %d %s\n", errno, strerror(errno));

        fix_time = mktime(&tm);
        localtime_r(&fix_time, &tm_local);

        // fix_time += tm_local.tm_gmtoff;
        // DBG("fix_time: %d\n", (int)fix_time);
        r->fix.timestamp = (long long)fix_time * 1000;
        return 0;
}

static int
nmea_reader_update_date(NmeaReader* const r, Token  date, Token  time)
{
        Token  tok = date;
        int    day, mon, year;

        if (tok.p + 6 != tok.end) {
                ERR("date not properly formatted: '%.*s'", tok.end-tok.p, tok.p);
                return -1;
        }
        day  = str2int(tok.p, tok.p+2);
        mon  = str2int(tok.p+2, tok.p+4);
        year = str2int(tok.p+4, tok.p+6) + 2000;

        if ((day|mon|year) < 0) {
                ERR("date not properly formatted: '%.*s'", tok.end-tok.p, tok.p);
                return -1;
        }

        r->utc_year  = year;
        r->utc_mon   = mon;
        r->utc_day   = day;

        return nmea_reader_update_time(r, time);
}


static double
convert_from_hhmm(Token  tok)
{
        double  val     = str2float(tok.p, tok.end);
        int     degrees = (int)(floor(val) / 100);
        double  minutes = val - degrees*100.;
        double  dcoord  = degrees + minutes / 60.0;
        return dcoord;
}


static int
nmea_reader_update_latlong(NmeaReader* const r,
                           Token        latitude,
                           char         latitudeHemi,
                           Token        longitude,
                           char         longitudeHemi)
{
        double   lat, lon;
        Token    tok;

        tok = latitude;
        if (tok.p + 6 > tok.end) {
                ERR("latitude is too short: '%.*s'", tok.end-tok.p, tok.p);
                return -1;
        }
        lat = convert_from_hhmm(tok);
        if (latitudeHemi == 'S')
                lat = -lat;

        tok = longitude;
        if (tok.p + 6 > tok.end) {
                ERR("longitude is too short: '%.*s'", tok.end-tok.p, tok.p);
                return -1;
        }
        lon = convert_from_hhmm(tok);
        if (longitudeHemi == 'W')
                lon = -lon;

        r->fix.flags    |= GPS_LOCATION_HAS_LAT_LONG;
        r->fix.latitude  = lat;
        r->fix.longitude = lon;
        return 0;
}
/* this is the state of our connection to the daemon */
typedef struct {
        int                     init;
        int                     fd;
        GpsCallbacks            callbacks;
        pthread_t               thread;
        int                     control[2];
        int                     sockfd;
        int                     epoll_hd;
        int                     flag;
        int                     start_flag;
        //   int                     thread_exit_flag;
} GpsState;

static GpsState  _gps_state[1];

static int
nmea_reader_update_altitude(NmeaReader* const r,
                            Token        altitude,
                            Token        units)
{
        double  alt;
        Token   tok = altitude;

        if (tok.p >= tok.end)
                return -1;

        r->fix.flags   |= GPS_LOCATION_HAS_ALTITUDE;
        r->fix.altitude = str2float(tok.p, tok.end);
        return 0;
}


static int
nmea_reader_update_bearing(NmeaReader* const r,
                           Token        bearing)
{
        double  alt;
        Token   tok = bearing;

        if (tok.p >= tok.end)
                return -1;

        r->fix.flags   |= GPS_LOCATION_HAS_BEARING;
        r->fix.bearing  = str2float(tok.p, tok.end);
        return 0;
}


static int
nmea_reader_update_speed(NmeaReader* const r,
                         Token        speed)
{
        double  alt;
        Token   tok = speed;

        if (tok.p >= tok.end)
                return -1;

        r->fix.flags   |= GPS_LOCATION_HAS_SPEED;

        // knot to m/s
        r->fix.speed = str2float(tok.p, tok.end) / 1.942795467;
        return 0;
}

// Add by LCH for accuracy
static int
nmea_reader_update_accuracy(NmeaReader* const r,
                            Token accuracy)
{
        double  alt;
        Token   tok = accuracy;

        if (tok.p >= tok.end)
                return -1;

        r->fix.flags   |= GPS_LOCATION_HAS_ACCURACY;
        r->fix.accuracy = str2float(tok.p, tok.end);
        return 0;
}

/*
static int
nmea_reader_update_sv_status_gps(NmeaReader* r, int sv_index,
                              int id, Token elevation,
                              Token azimuth, Token snr)
{
       // int prn = str2int(id.p, id.end);
    int prn = id;
    if ((prn <= 0) || (prn < 65 && prn > GPS_MAX_SVS)|| (prn > 96) || (r->sv_count >= GPS_MAX_SVS)) {
        VER("sv_status_gps: ignore (%d)", prn);
        return 0;
    }
    sv_index = r->sv_count+r->sv_status_gps.num_svs;
    if (GPS_MAX_SVS <= sv_index) {
        ERR("ERR: sv_index=[%d] is larger than GPS_MAX_SVS.\n", sv_index);
        return 0;
    }
    r->sv_status_gps.sv_list[sv_index].prn = prn;
    r->sv_status_gps.sv_list[sv_index].snr = str2float(snr.p, snr.end);
    r->sv_status_gps.sv_list[sv_index].elevation = str2int(elevation.p, elevation.end);
    r->sv_status_gps.sv_list[sv_index].azimuth = str2int(azimuth.p, azimuth.end);
    r->sv_count++;
    VER("sv_status_gps(%2d): %2d, %2f, %3f, %2f", sv_index,
       r->sv_status_gps.sv_list[sv_index].prn, r->sv_status_gps.sv_list[sv_index].elevation,
       r->sv_status_gps.sv_list[sv_index].azimuth, r->sv_status_gps.sv_list[sv_index].snr);
    return 0;
}
*/

static int 
get_svid(int prn, int sv_type)
{
        if (sv_type == GNSS_CONSTELLATION_GLONASS && prn >= 1 && prn <= 32)
                return prn + 64;
        else if (sv_type == GNSS_CONSTELLATION_BEIDOU && prn >= 1 && prn <= 32)
                return prn + 200;

        return prn;
}

static int
nmea_reader_update_sv_status_gnss(NmeaReader* r, int sv_index,
                                  int id, Token elevation,
                                  Token azimuth, Token snr)
{
        int prn = id;
        sv_index = r->sv_count + r->sv_status_gnss.num_svs;
        if (GNSS_MAX_SVS <= sv_index) {
                ERR("ERR: sv_index=[%d] is larger than GNSS_MAX_SVS.\n", sv_index);
                return 0;
        }

        if ((prn > 0) && (prn < 32)) {
                r->sv_status_gnss.gnss_sv_list[sv_index].svid = prn;
                r->sv_status_gnss.gnss_sv_list[sv_index].constellation = GNSS_CONSTELLATION_GPS;
        } else if ((prn >= 65) && (prn <= 96)) {
                r->sv_status_gnss.gnss_sv_list[sv_index].svid = prn-64;
                r->sv_status_gnss.gnss_sv_list[sv_index].constellation = GNSS_CONSTELLATION_GLONASS;
        } else if ((prn >= 201) && (prn <= 237)) {
                r->sv_status_gnss.gnss_sv_list[sv_index].svid = prn-200;
                r->sv_status_gnss.gnss_sv_list[sv_index].constellation = GNSS_CONSTELLATION_BEIDOU;
        } else if ((prn >= 401) && (prn <= 436)) {
                r->sv_status_gnss.gnss_sv_list[sv_index].svid = prn-400;
                r->sv_status_gnss.gnss_sv_list[sv_index].constellation = GNSS_CONSTELLATION_GALILEO;
        } else {
                DBG("sv_status: ignore (%d)", prn);
                return 0;
        }

        r->sv_status_gnss.gnss_sv_list[sv_index].c_n0_dbhz = str2float(snr.p, snr.end);
        r->sv_status_gnss.gnss_sv_list[sv_index].elevation = str2int(elevation.p, elevation.end);
        r->sv_status_gnss.gnss_sv_list[sv_index].azimuth = str2int(azimuth.p, azimuth.end);
        r->sv_status_gnss.gnss_sv_list[sv_index].flags = 0;
        if (1 == r->sv_used_in_fix[prn]) {
                r->sv_status_gnss.gnss_sv_list[sv_index].flags |= GNSS_SV_FLAGS_USED_IN_FIX;
        }

        r->sv_count++;
        DBG("sv_status(%2d): %2d, %d, %2f, %3f, %2f, %d",
            sv_index, r->sv_status_gnss.gnss_sv_list[sv_index].svid, r->sv_status_gnss.gnss_sv_list[sv_index].constellation,
            r->sv_status_gnss.gnss_sv_list[sv_index].elevation, r->sv_status_gnss.gnss_sv_list[sv_index].azimuth,
            r->sv_status_gnss.gnss_sv_list[sv_index].c_n0_dbhz, r->sv_status_gnss.gnss_sv_list[sv_index].flags);
        return 0;
 }


static void
nmea_reader_parse(NmeaReader* const r)
{
        /* we received a complete sentence, now parse it to generate
         * a new GPS fix...
         */
        NmeaTokenizer  tzer[1];
        Token          tok;
        GnssConstellationType sv_type = GNSS_CONSTELLATION_GPS;


#if NEMA_DEBUG
        DBG("Received: '%.*s'", r->pos, r->in);
#endif
        if (r->pos < 9) {
                ERR("Too short. discarded. '%.*s'", r->pos, r->in);
                return;
        }
        if (r->pos < (int)sizeof(r->in)) {
                nmea_tokenizer_init(tzer, r->in, r->in + r->pos);
        }
#if NEMA_DEBUG
        {
                int  n;
                DBG("Found %d tokens", tzer->count);
                for (n = 0; n < tzer->count; n++) {
                        Token  tok = nmea_tokenizer_get(tzer, n);
                        DBG("%2d: '%.*s'", n, tok.end-tok.p, tok.p);
                }
        }
#endif

        tok = nmea_tokenizer_get(tzer, 0);
        if (tok.p + 5 > tok.end) {
                ERR("sentence id '%.*s' too short, ignored.", tok.end-tok.p, tok.p);
                return;
        }

        //  ignore first two characters.
        if (!memcmp(tok.p, "BD", 2)) {
                sv_type = GNSS_CONSTELLATION_BEIDOU;
                DBG("BDS SV type");
        }
        else if (!memcmp(tok.p, "GL", 2)) {
                sv_type = GNSS_CONSTELLATION_GLONASS;
                DBG("GLN SV type");
        }
        tok.p += 2;
        if (!memcmp(tok.p, "GGA", 3)) {
                //  GPS fix
                Token  tok_time          = nmea_tokenizer_get(tzer, 1);
                Token  tok_latitude      = nmea_tokenizer_get(tzer, 2);
                Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer, 3);
                Token  tok_longitude     = nmea_tokenizer_get(tzer, 4);
                Token  tok_longitudeHemi = nmea_tokenizer_get(tzer, 5);
                Token  tok_altitude      = nmea_tokenizer_get(tzer, 9);
                Token  tok_altitudeUnits = nmea_tokenizer_get(tzer, 10);

                nmea_reader_update_time(r, tok_time);
                nmea_reader_update_latlong(r, tok_latitude,
                                           tok_latitudeHemi.p[0],
                                           tok_longitude,
                                           tok_longitudeHemi.p[0]);
                nmea_reader_update_altitude(r, tok_altitude, tok_altitudeUnits);

        }
        else if (!memcmp(tok.p, "GSA", 3)) {
                Token tok_fix = nmea_tokenizer_get(tzer, 2);
                Token tok_svs = nmea_tokenizer_get(tzer, 18);
                switch(tok_svs.p[0]) {
                case '1':
                        sv_type = GNSS_CONSTELLATION_GPS;
                        break;
                case '2':
                        sv_type = GNSS_CONSTELLATION_GLONASS;
                        break;
                case '4':
                        sv_type = GNSS_CONSTELLATION_BEIDOU;
                        break;
                default:
                        break;
                }
                int idx, max = 12;  /*the number of satellites in GPGSA*/

                r->fix_mode = str2int(tok_fix.p, tok_fix.end);

                if (LOC_FIXED(r)) {  /* 1: No fix; 2: 2D; 3: 3D*/
                        Token  tok_accuracy = nmea_tokenizer_get(tzer, 15);
                        nmea_reader_update_accuracy(r, tok_accuracy);   // pdop

                        for (idx = 0; idx < max; idx++) {
                                Token tok_satellite = nmea_tokenizer_get(tzer, idx+3);
                                if (tok_satellite.p == tok_satellite.end) {
                                        DBG("GSA: found %d active satellites\n", idx);
                                        break;
                                }
                                int prn = str2int(tok_satellite.p, tok_satellite.end);
                                int svid = get_svid(prn, sv_type);
                                if (svid >= 0 && svid < 256)
                                        r->sv_used_in_fix[svid] = 1;
                                        
                                /*
                                if (sv_type == GNSS_CONSTELLATION_BEIDOU) {
                                        sate_id += 200;
                                }
                                else if (sv_type == GNSS_CONSTELLATION_GLONASS) {
                                        sate_id += 64;
                                }
                                if (sate_id >= 1 && sate_id <= 32) {     // GP
                                        r->sv_used_in_fix[sate_id] = 1;
                                } else if (sate_id >= 193 && sate_id <= 197) {
                                        r->sv_used_in_fix[sate_id] = 0;
                                        DBG("[debug mask]QZSS, just ignore. satellite id is %d\n ", sate_id);
                                        continue;
                                } else if (sate_id >= 65 && sate_id <= 96) {     // GL
                                        r->sv_used_in_fix[sate_id] = 1;
                                } else if (sate_id >= 201 && sate_id <= 232) {     // BD
                                        r->sv_used_in_fix[sate_id] = 1;
                                }
                                else {
                                        VER("GSA: invalid sentence, ignore!!");
                                        break;
                                }
                                DBG("GSA:sv_used_in_fix[%d] = %d\n", svid, r->sv_used_in_fix[svid]);
                                */
                        }
                }
        }
        else if (!memcmp(tok.p, "RMC", 3)) {
                Token  tok_time          = nmea_tokenizer_get(tzer, 1);
                Token  tok_fixStatus     = nmea_tokenizer_get(tzer, 2);
                Token  tok_latitude      = nmea_tokenizer_get(tzer, 3);
                Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer, 4);
                Token  tok_longitude     = nmea_tokenizer_get(tzer, 5);
                Token  tok_longitudeHemi = nmea_tokenizer_get(tzer, 6);
                Token  tok_speed         = nmea_tokenizer_get(tzer, 7);
                Token  tok_bearing       = nmea_tokenizer_get(tzer, 8);
                Token  tok_date          = nmea_tokenizer_get(tzer, 9);

                VER("in RMC, fixStatus=%c", tok_fixStatus.p[0]);
                if (tok_fixStatus.p[0] == 'A')
                {
                        nmea_reader_update_date(r, tok_date, tok_time);

                        nmea_reader_update_latlong(r, tok_latitude,
                                                   tok_latitudeHemi.p[0],
                                                   tok_longitude,
                                                   tok_longitudeHemi.p[0]);

                        nmea_reader_update_bearing(r, tok_bearing);
                        nmea_reader_update_speed(r, tok_speed);
                        r->location_can_report = 1;
                }
                r->sv_status_can_report = 1;
        } else if (!memcmp(tok.p, "GSV", 3)) {
                Token tok_num = nmea_tokenizer_get(tzer, 1);    // number of messages
                Token tok_seq = nmea_tokenizer_get(tzer, 2);    // sequence number
                Token tok_cnt = nmea_tokenizer_get(tzer, 3);    // Satellites in view
                int num = str2int(tok_num.p, tok_num.end);
                int seq = str2int(tok_seq.p, tok_seq.end);
                int cnt = str2int(tok_cnt.p, tok_cnt.end);
                int sv_base = (seq - 1)*NMEA_MAX_SV_INFO;
                int sv_num = cnt - sv_base;
                int idx, base = 4, base_idx;
                if (sv_num > NMEA_MAX_SV_INFO)
                        sv_num = NMEA_MAX_SV_INFO;
                if (seq == 1)   /*if sequence number is 1, a new set of GSV will be parsed*/
                        r->sv_count = 0;
                for (idx = 0; idx < sv_num; idx++) {
                        base_idx = base*(idx+1);
                        Token tok_id  = nmea_tokenizer_get(tzer, base_idx+0);
                        int prn = str2int(tok_id.p, tok_id.end);
                        int svid = get_svid(prn, sv_type);
                        /*
                        if (sv_type == GNSS_CONSTELLATION_BEIDOU) {
                                sv_id += 200;
                                DBG("It is BDS SV: %d", sv_id);
                        }
                        else if (sv_type == GNSS_CONSTELLATION_GLONASS) {
                                sv_id += 64;
                                DBG("It is GLN SV: %d", sv_id);
                        }
                        */
                        Token tok_ele = nmea_tokenizer_get(tzer, base_idx+1);
                        Token tok_azi = nmea_tokenizer_get(tzer, base_idx+2);
                        Token tok_snr = nmea_tokenizer_get(tzer, base_idx+3);
                        nmea_reader_update_sv_status_gnss(r, sv_base+idx, svid, tok_ele, tok_azi, tok_snr);
                }
                if (seq == num) {
                        if (r->sv_count <= cnt) {
                                DBG("r->sv_count = %d", r->sv_count);
                                r->sv_status_gnss.num_svs += r->sv_count;


                        } else {
                                ERR("GPGSV incomplete (%d/%d), ignored!", r->sv_count, cnt);
                                r->sv_count = r->sv_status_gnss.num_svs = 0;
                        }
                }
        }
        // Add for Accuracy
        else if (!memcmp(tok.p, "ACCURACY", 8)) {
                if ((r->fix_mode == 3) || (r->fix_mode == 2)) {
                        Token  tok_accuracy = nmea_tokenizer_get(tzer, 1);
                        nmea_reader_update_accuracy(r, tok_accuracy);
                        DBG("GPS get accuracy from driver:%f\n", r->fix.accuracy);
                }
                else {
                        DBG("GPS get accuracy failed, fix mode:%d\n", r->fix_mode);
                }
        }
        else {
                tok.p -= 2;
                VER("unknown sentence '%.*s", tok.end-tok.p, tok.p);
        }
        //if (!LOC_FIXED(r)) {
        //    VER("Location is not fixed, ignored callback\n");
        //} else if (r->fix.flags != 0 && gps_nmea_end_tag) {
        if (r->location_can_report) {
#if NEMA_DEBUG
                char   temp[256];
                char*  p   = temp;
                char*  end = p + sizeof(temp);
                struct tm   utc;

                p += snprintf(p, end-p, "sending fix");
                if (r->fix.flags & GPS_LOCATION_HAS_LAT_LONG) {
                        p += snprintf(p, end-p, " lat=%g lon=%g", r->fix.latitude, r->fix.longitude);
                }
                if (r->fix.flags & GPS_LOCATION_HAS_ALTITUDE) {
                        p += snprintf(p, end-p, " altitude=%g", r->fix.altitude);
                }
                if (r->fix.flags & GPS_LOCATION_HAS_SPEED) {
                        p += snprintf(p, end-p, " speed=%g", r->fix.speed);
                }
                if (r->fix.flags & GPS_LOCATION_HAS_BEARING) {
                        p += snprintf(p, end-p, " bearing=%g", r->fix.bearing);
                }
                if (r->fix.flags & GPS_LOCATION_HAS_ACCURACY) {
                        p += snprintf(p, end-p, " accuracy=%g", r->fix.accuracy);
                        DBG("GPS accuracy=%g\n", r->fix.accuracy);
                }
                gmtime_r((time_t*) &r->fix.timestamp, &utc);
                p += snprintf(p, end-p, " time=%s", asctime(&utc));
                VER(temp);
#endif

                callback_backup.location_cb(&r->fix);
                r->fix.flags = 0;
                r->location_can_report = 0;
        }

        DBG("r->sv_status_gnss.num_svs = %d, gps_nmea_end_tag = %d, sv_status_can_report = %d", r->sv_status_gnss.num_svs, gps_nmea_end_tag, r->sv_status_can_report);
        if (r->sv_status_can_report) {
                r->sv_status_can_report = 0;
                if (r->sv_status_gnss.num_svs != 0) {
                        r->sv_status_gnss.size = sizeof(GnssSvStatus);
                        DBG("Report sv status");
                        callback_backup.gnss_sv_status_cb(&r->sv_status_gnss);
                        r->sv_count = r->sv_status_gnss.num_svs = 0;
                        memset(r->sv_used_in_fix, 0, 256*sizeof(int));
                }
        }
}


static void
nmea_reader_addc(NmeaReader* const r, int  c)
{
        if (r->overflow) {
                r->overflow = (c != '\n');
                return;
        }

        if ((r->pos >= (int) sizeof(r->in)-1 ) || (r->pos < 0)) {
                r->overflow = 1;
                r->pos      = 0;
                DBG("nmea sentence overflow\n");
                return;
        }

        r->in[r->pos] = (char)c;
        r->pos       += 1;

        if (c == '\n') {
                nmea_reader_parse(r);

                DBG("start nmea_cb\n");
		r->in[r->pos] = 0;
                callback_backup.nmea_cb(r->fix.timestamp, r->in, r->pos);
                r->pos = 0;
        }
}


static void
gps_state_done(GpsState*  s)
{
        char   cmd = CMD_QUIT;

        write(s->control[0], &cmd, 1);
        close(s->control[0]);
        s->control[0] = -1;
        close(s->control[1]);
        s->control[1] = -1;
        close(s->fd);
        s->fd = -1;
        close(s->sockfd);
        s->sockfd = -1;
        close(s->epoll_hd);
        s->epoll_hd = -1;
        s->init = 0;
        return;
}


static void
gps_state_start(GpsState*  s)
{
        char  cmd = CMD_START;
        int   ret;

        do {
                ret = write(s->control[0], &cmd, 1);
        }
        while (ret < 0 && errno == EINTR);

        if (ret != 1)
                ERR("%s: could not send CMD_START command: ret=%d: %s",
                    __FUNCTION__, ret, strerror(errno));
}

static void
gps_state_stop(GpsState*  s)
{
        char  cmd = CMD_STOP;
        int   ret;

        do {
                ret = write(s->control[0], &cmd, 1);
        }
        while (ret < 0 && errno == EINTR);

        if (ret != 1)
                ERR("%s: could not send CMD_STOP command: ret=%d: %s",
                    __FUNCTION__, ret, strerror(errno));
}

static void
gps_state_restart(GpsState*  s)
{
        char  cmd = CMD_RESTART;
        int   ret;

        do {
                ret = write(s->control[0], &cmd, 1);
        }
        while (ret < 0 && errno == EINTR);

        if (ret != 1)
                ERR("%s: could not send CMD_RESTART command: ret=%d: %s",
                    __FUNCTION__, ret, strerror(errno));
}


static int
epoll_register(int  epoll_fd, int  fd)
{
        struct epoll_event  ev;
        int                 ret, flags;

        /* important: make the fd non-blocking */
        flags = fcntl(fd, F_GETFL);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);

        ev.events  = EPOLLIN;
        ev.data.fd = fd;
        do {
                ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
        } while (ret < 0 && errno == EINTR);
        if (ret < 0)
                ERR("epoll ctl error, error num is %d\n, message is %s\n", errno, strerror(errno));
        return ret;
}


static int
epoll_deregister(int  epoll_fd, int  fd)
{
        int  ret;
        do {
                ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
        } while (ret < 0 && errno == EINTR);
        return ret;
}

/*for reducing the function call to get data from kernel*/
static char buff[2048];
/* this is the main thread, it waits for commands from gps_state_start/stop and,
 * when started, messages from the GPS daemon. these are simple NMEA sentences
 * that must be parsed to be converted into GPS fixes sent to the framework
 */
void
gps_state_thread(void*  arg)
{
        static float count = 0;
        GpsState*   state = (GpsState*) arg;
        //   state->thread_exit_flag=0;
        NmeaReader  reader[1];
        int         gps_fd     = state->fd;
        int         control_fd = state->control[1];
        //int         atc_fd = state->sockfd;

        int epoll_fd = state->epoll_hd;
        int         test_started = 0;

        nmea_reader_init(reader);

        //  register control file descriptors for polling
        if (epoll_register(epoll_fd, control_fd) < 0)
                ERR("epoll register control_fd error, error num is %d\n, message is %s\n", errno, strerror(errno));
        if (epoll_register(epoll_fd, gps_fd) < 0)
                ERR("epoll register gps_fd error, error num is %d\n, message is %s\n", errno, strerror(errno));
        //if (epoll_register(epoll_fd, atc_fd) < 0)
        //        ERR("epoll register atc_fd error, error num is %d\n, message is %s\n", errno, strerror(errno));

        DBG("gps thread running: PPID[%d], PID[%d]\n", getppid(), getpid());
        DBG("HAL thread is ready, realease lock, and CMD_START can be handled\n");
        //  now loop
        for (;;) {
                struct epoll_event   events[4];
                int                  ne, nevents;
                nevents = epoll_wait(epoll_fd, events, 4, -1);
                if (nevents < 0) {
                        /*if (errno != EINTR)
                                ERR("epoll_wait() unexpected error: %s", strerror(errno));*/
                        continue;
                }
                VER("gps thread received %d events", nevents);
                for (ne = 0; ne < nevents; ne++) {
                        if ((events[ne].events & (EPOLLERR|EPOLLHUP)) != 0) {
                                ERR("EPOLLERR or EPOLLHUP after epoll_wait() !?");
                                goto Exit;
                        }
                        if ((events[ne].events & EPOLLIN) != 0) {
                                int  fd = events[ne].data.fd;

                                if (fd == control_fd) {
                                        char  cmd = 255;
                                        int   ret;
                                        DBG("gps control fd event");
                                        do {
                                                ret = read(fd, &cmd, 1);
                                        } while (ret < 0 && errno == EINTR);

                                        if (cmd == CMD_QUIT) {
                                                DBG("gps thread quitting on demand");
                                                goto Exit;
                                        }
                                        else if (cmd == CMD_START) {
                                                if (!started) {
                                                        DBG("gps thread starting  location_cb=%p", &callback_backup);
                                                        started = 1;
                                                        nmea_reader_set_callback(reader, &state->callbacks);
                                                }
                                        }
                                        else if (cmd == CMD_STOP) {
                                                if (started) {
                                                        DBG("gps thread stopping");
                                                        started = 0;
                                                        nmea_reader_set_callback(reader, NULL);
                                                        DBG("CMD_STOP has been receiving from HAL thread, release lock so can handle CLEAN_UP\n");
                                                }
                                        }
                                        else if (cmd == CMD_RESTART) {
                                                reader->fix_mode = 0;
                                        }
                                }
                                else if (fd == gps_fd) {
                                        if (!flag_unlock) {
                                                flag_unlock = 1;
                                                DBG("got first NMEA sentence, release lock to set state ENGINE ON, SESSION BEGIN");
                                        }
                                        VER("gps fd event");
                                        if (report_time_interval > ++count) {
                                                DBG("[trace]count is %f\n", count);
                                                int ret = read(fd, buff, sizeof(buff));
                                                continue;
                                        }
                                        count = 0;
                                        for (;;) {
                                                int  nn, ret;
                                                ret = read(fd, buff, sizeof(buff));
                                                if (ret < 0) {
                                                        if (errno == EINTR)
                                                                continue;
                                                        if (errno != EWOULDBLOCK)
                                                                ERR("error while reading from gps daemon socket: %s: %p", strerror(errno), buff);
                                                        break;
                                                }
                                                DBG("received %d bytes:\n", ret);
                                                gps_nmea_end_tag = 0;
                                                for (nn = 0; nn < ret; nn++)
                                                {
                                                        if (nn == (ret-1))
                                                                gps_nmea_end_tag = 1;

                                                        nmea_reader_addc(reader, buff[nn]);
                                                }
                                        }
                                        VER("gps fd event end");
                                }
                                else
                                {
                                        ERR("epoll_wait() returned unkown fd %d ?", fd);
                                }
                        }
                }
        }
Exit:
        DBG("HAL thread is exiting, release lock to clean resources\n");
        return;
}


static void
gps_state_init(GpsState*  state)
{
    char path[PROPERTY_VALUE_MAX];
        state->control[0] = -1;
        state->control[1] = -1;
        state->fd         = -1;
 
        property_get("gps.device.path", path, GPS_CHANNEL_NAME);
    
        DBG("Try open gps hardware:  %s", path);
        state->fd = open(path, O_RDONLY);    // support poll behavior
        //state->fd = open(path, O_RDWR | O_NONBLOCK | O_NOCTTY);
        int epoll_fd   = epoll_create(2);
        state->epoll_hd = epoll_fd;

        if (state->fd < 0) {
                ERR("no gps hardware detected: %s:%d, %s", path, state->fd, strerror(errno));
                return;
        }

        struct termios cfg;
        tcgetattr(state->fd, &cfg);
        cfmakeraw(&cfg);
        cfsetispeed(&cfg, TTY_BAUD);
        cfsetospeed(&cfg, TTY_BAUD);
        tcsetattr(state->fd, TCSANOW, &cfg);

        DBG("Open gps hardware succeed: %s", path);

        if (socketpair(AF_LOCAL, SOCK_STREAM, 0, state->control) < 0) {
                ERR("could not create thread control socket pair: %s", strerror(errno));
                goto Fail;
        }
        state->thread = callback_backup.create_thread_cb(gps_native_thread, gps_state_thread, state);
        if (!state->thread) {
                ERR("could not create gps thread: %s", strerror(errno));
                goto Fail;
        }

        DBG("gps state initialized, the thread is %d\n", (int)state->thread);
        return;

Fail:
        gps_state_done(state);
}


/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       I N T E R F A C E                               *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/
static int
zkw_gps_init(GpsCallbacks* callbacks)
{
        GpsState*  s = _gps_state;
        int get_time = 20;
        int res = 0;
        if (s->init)
                return 0;


        s->callbacks = *callbacks;
        callback_backup = *callbacks;

        gps_state_init(s);
        s->init = 1;
        if (s->fd < 0) {
                return -1;
        }
        DBG("Set GPS_CAPABILITY_SCHEDULING \n");
        callback_backup.set_capabilities_cb(GPS_CAPABILITY_SCHEDULING);
        return 0;
}

static void
zkw_gps_cleanup(void)
{
        GpsState*  s = _gps_state;

        if (s->init)
                gps_state_done(s);
        DBG("zkw_gps_cleanup done");
        //     return NULL;
}

int
zkw_gps_start()
{
        GpsState*  s = _gps_state;
        int err;
        int count=0;

        if (!s->init) {
                ERR("%s: called with uninitialized state !!", __FUNCTION__);
                return -1;
        }

        DBG("HAL thread has initialiazed\n");
        gps_state_start(s);

        gps_status.status = GPS_STATUS_ENGINE_ON;
        DBG("gps_status = GPS_STATUS_ENGINE_ON\n");
        callback_backup.status_cb(&gps_status);
        gps_status.status = GPS_STATUS_SESSION_BEGIN;
        DBG("gps_status = GPS_STATUS_SESSION_BEGIN\n");
        callback_backup.status_cb(&gps_status);
        callback_backup.acquire_wakelock_cb();
        s->start_flag = 1;
        DBG("s->start_flag = 1\n");
        return 0;
}
int
zkw_gps_stop()
{
        GpsState*  s = _gps_state;
        int err;
        int count=0;

        if (!s->init) {
                ERR("%s: called with uninitialized state !!", __FUNCTION__);
                return -1;
        }

        gps_state_stop(s);

        gps_status.status = GPS_STATUS_SESSION_END;
        callback_backup.status_cb(&gps_status);
        DBG("gps_status = GPS_STATUS_SESSION_END\n");
        gps_status.status = GPS_STATUS_ENGINE_OFF;
        DBG("gps_status = GPS_STATUS_ENGINE_OFF\n");
        callback_backup.status_cb(&gps_status);
        callback_backup.release_wakelock_cb();
        s->start_flag = 0;
        DBG("s->start_flag = 0\n");
        return 0;
}
static int
zkw_gps_inject_time(GpsUtcTime time, int64_t timeReference, int uncertainty)
{
        return 0;
}

static int
zkw_gps_inject_location(double latitude, double longitude, float accuracy)
{
        return 0;
}

static void
zkw_gps_delete_aiding_data(GpsAidingData flags)
{
        return;
}

static int
zkw_gps_set_position_mode(GpsPositionMode mode, GpsPositionRecurrence recurrence, uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time)
{
        // FIXME - support fix_frequency
        return 0;
}

static const void*
zkw_gps_get_extension(const char* name)
{
        DBG("zkw_gps_get_extension name=[%s]\n", name);
        /*
            TRC();
            if (strncmp(name, "agps", strlen(name)) == 0) {
                return &zkwAGpsInterface;
            }
            if (strncmp(name, "gps-ni", strlen(name)) == 0) {
                return &zkwGpsNiInterface;
            }
            if (strncmp(name, "agps_ril", strlen(name)) == 0) {
                return &zkwAGpsRilInterface;
            }
            if (strncmp(name, "supl-certificate", strlen(name)) == 0) {
               return &zkwSuplCertificateInterface;
            }
            if (strncmp(name, GPS_MEASUREMENT_INTERFACE, strlen(name)) == 0) {
               return &zkwGpsMeasurementInterface;
            }
            if (strncmp(name, GPS_NAVIGATION_MESSAGE_INTERFACE, strlen(name)) == 0) {
               return &zkwGpsNavigationMessageInterface;
            }*/
        return NULL;
}

static const GpsInterface  zkwGpsInterface = {
        sizeof(GpsInterface),
        zkw_gps_init,
        zkw_gps_start,
        zkw_gps_stop,
        zkw_gps_cleanup,
        zkw_gps_inject_time,
        zkw_gps_inject_location,
        zkw_gps_delete_aiding_data,
        zkw_gps_set_position_mode,
        zkw_gps_get_extension,
};

const GpsInterface* gps__get_gps_interface(struct gps_device_t* dev)
{
        DBG("gps__get_gps_interface HAL\n");

        return &zkwGpsInterface;
}

static int open_gps(const struct hw_module_t* module, char const* name,
                    struct hw_device_t** device) {
        DBG("open_gps HAL 1\n");
        struct gps_device_t *dev = malloc(sizeof(struct gps_device_t));
        if (dev != NULL) {
                memset(dev, 0, sizeof(*dev));

                dev->common.tag = HARDWARE_DEVICE_TAG;
                dev->common.version = 0;
                dev->common.module = (struct hw_module_t*)module;
                //   dev->common.close = (int (*)(struct hw_device_t*))close_lights;
                DBG("open_gps HAL 2\n");
                dev->get_gps_interface = gps__get_gps_interface;
                DBG("open_gps HAL 3\n");
                *device = (struct hw_device_t*)dev;
        } else {
                DBG("malloc failed dev = NULL!\n");
        }
        return 0;
}


static struct hw_module_methods_t gps_module_methods = {
        .open = open_gps
};


struct hw_module_t HAL_MODULE_INFO_SYM = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = GPS_HARDWARE_MODULE_ID,
        .name = "Hardware GPS Module",
        .author = "",
        .methods = &gps_module_methods,
};
