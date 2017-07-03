
static int merbok_gps_init(GpsCallbacks* callbacks);
static int merbok_gps_start(void);
static int merbok_gps_stop(void);
static void merbok_gps_cleanup(void);
static int  merbok_gps_inject_time(GpsUtcTime time, int64_t timeReference,
                     int uncertainty);
static int  merbok_gps_inject_location(double latitude, double longitude, float accuracy);
static void merbok_gps_delete_aiding_data(GpsAidingData flags);
static int merbok_gps_set_position_mode(GpsPositionMode mode, GpsPositionRecurrence recurrence,
        uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time);
const void *merbok_gps_get_extension(const char* name);



static int merbok_gps_init(GpsCallbacks* callbacks)
{
	return 0;
}

static int merbok_gps_start(void)
{
	return 0;
}

static int merbok_gps_stop(void)
{
	return 0;
}

static void merbok_gps_cleanup(void)
{
	return 0;
}

static int  merbok_gps_inject_time(GpsUtcTime time, int64_t timeReference,
                     int uncertainty)
{
	return 0;
}

static int  merbok_gps_inject_location(double latitude, double longitude, float accuracy)
{
	return 0;
}

static void merbok_gps_delete_aiding_data(GpsAidingData flags)
{
	return 0;
}

static int merbok_gps_set_position_mode(GpsPositionMode mode, GpsPositionRecurrence recurrence,
        uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time)
{
	return 0;
}

const void *merbok_gps_get_extension(const char* name)
{
	return NULL;
}