#define HOST      "api.openweathermap.org"
#define IP_ADDR   "188.226.175.223"
#define PORT      80
#define REQUEST_HEADERS "GET /data/2.5/weather?q={%s}&units=metric HTTP/1.1\r\n" \
						"Host: %s\r\n" \
						"Connection: close\r\n\r\n"

struct weather {
	char description[16];
	int temperature;
	int pressure;
	int humidity;
	int wind_speed;
	int wind_direction;
	int cloudiness;
	int sunrise;
	int sunset;
	int timestamp;
};

int get_weather(struct weather *wthr, const char *city);
ssize_t send_get_request(char *buf, const size_t len, const char* headers, const char* dst_ip);

int json_to_weather(struct weather *wthr, const char* json);
ssize_t json_get_token_val(char *buf,const size_t len, const jsmntok_t *token, const char *json);
int parse_tokens(struct weather *wthr, const jsmntok_t *tokens, const size_t len, const char *json);

void print_usage();
