#define HOST      "api.openweathermap.org"
#define IP_ADDR   "188.226.175.223"
#define PORT      80

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

int get_weather(struct weather *wthr, const char *city);
ssize_t send_get_request(char *buf, const size_t len, const char* headers, const char* dst_ip);
int json_to_weather(struct weather *wthr, const char* json);
ssize_t json_get_token_val(char *buf,const size_t len, const jsmntok_t *token, const char *json);

void print_usage();
