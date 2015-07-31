struct weather {
	// char *description;
	int temperature;
	int pressure;
	int humidity;
	int wind_speed;
	int wind_direction;
	int cloudness;
	int sunrise;
	int sunset;
	int timestamp;
};

static const char* HOST = "api.openweathermap.org";
static const char* ADDR = "188.226.175.223";
static const unsigned int PORT = 80;

int get_weather(struct weather *wthr, const char *city);

int json_to_weather(struct weather *wthr, const char* json);
ssize_t json_get_token_val(char *buf,const size_t len, const jsmntok_t *token, const char *json);

ssize_t send_get_request(char *buf, const size_t len, const char* headers, const char* dst_ip);

void print_usage();
