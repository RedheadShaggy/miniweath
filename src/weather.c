/*
	TODO Add output format options
	     Rewrite shitty code in json_to_weather()
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "jsmn.h"
#include "weather.h"

int main(int argc, char const *argv[]){
	struct weather cur_weather;
	int result = get_weather(&cur_weather, argv[1]);
	if(result < 0){
		printf("Error %d\n", result);
		return 0;
	}
	printf("Temperature = %d\n", cur_weather.temperature);
	return 0;
}

/**
	Gets current weather from OpenWeatherMap.org for city.

	@param	wthr 		Weather struct to store data.
	@param	city 		I think you get it.

	@return 0 if success or negative error code.
 */
int get_weather(struct weather *wthr,const char *city){
	char *headers = malloc(128);
	snprintf(headers, 128, "GET /data/2.5/weather?q={%s}&units=metric HTTP/1.1\r\n"
						   "Host: %s\r\n"
						   "Connection: close\r\n\r\n",
						   city, HOST);

	char *buffer = malloc(512);
	int content_length = send_get_request(buffer, 512, headers, ADDR);
	free(headers);
	if(content_length <= 0) {
		free(buffer);
		printf("%d\n", content_length);
		return content_length; // If content_length is negative, that means a error occured, we return error code.
	}

	char *raw_json_data = malloc(content_length);
	strcpy(raw_json_data, buffer);
	free(buffer);

	int result = json_to_weather(wthr, raw_json_data);
	free(raw_json_data);
	if(result != 0)
		return result;

	return 0;
}

/**
	Parse JSON string into weather struct.

	@param	wthr 		Weather structure for storing values.
	@param 	json 		JSON string with weather information from openweathermap.org.

	@return 0 if success or JSMN error code if parsing failed.
 */
jsmnerr_t json_to_weather(struct weather *wthr,const char* json){
	jsmn_parser json_parser;
	jsmntok_t *tokens;
	int tokens_num;

	/* Count tokens and allocate place for them. */
	jsmn_init(&json_parser);
	tokens_num =  jsmn_parse(&json_parser, json, strlen(json), NULL, 0);

	if(tokens_num < 0)
		return tokens_num;
	if(tokens_num == 0)
		return JSMN_ERROR_PART;

	/* Re-init json_parser for resetting position and parse the actual tokens. */
	tokens = malloc(tokens_num * sizeof *tokens);
	jsmn_init(&json_parser);
	tokens_num = jsmn_parse(&json_parser, json, strlen(json), tokens, tokens_num);

	/*
		If first key in JSON is "cod", that means there's no weather information.
	*/
	char token_value[16];
	json_get_token_val(token_value, 16, &tokens[1], json);
	if(strcmp(token_value, "cod") == 0)
		return JSMN_ERROR_INVAL;

	/*
		This sucks.
		The most shitty code you ever seen.
		I don't know how to manage this, but i try rewrite this code later.
		And also i must handle weather description.
	*/
	int i = 0;
	for(i = 0; i < tokens_num; i++){
		json_get_token_val(token_value, 16, &tokens[i], json);
		if(strcmp(token_value, "temp") == 0){
			i++;
			json_get_token_val(token_value, 16, &tokens[i], json);
			wthr->temperature  = atoll(token_value);
		}
		else if(strcmp(token_value, "pressure") == 0){
			i++;
			json_get_token_val(token_value, 16, &tokens[i], json);
			wthr->pressure  = atoll(token_value);
		}
		else if(strcmp(token_value, "humidity") == 0){
			i++;
			json_get_token_val(token_value, 16, &tokens[i], json);
			wthr->humidity  = atoll(token_value);
		}
		else if(strcmp(token_value, "speed") == 0){
			i++;
			json_get_token_val(token_value, 16, &tokens[i], json);
			wthr->wind_speed  = atoll(token_value);
		}
		else if(strcmp(token_value, "deg") == 0){
			i++;
			json_get_token_val(token_value, 16, &tokens[i], json);
			wthr->wind_direction  = atoll(token_value);
		}
		else if(strcmp(token_value, "all") == 0){
			i++;
			json_get_token_val(token_value, 16, &tokens[i], json);
			wthr->cloudness  = atoll(token_value);
		}
		else if(strcmp(token_value, "sunrise") == 0){
			i++;
			json_get_token_val(token_value, 16, &tokens[i], json);
			wthr->sunrise  = atoll(token_value);
		}
		else if(strcmp(token_value, "sunset") == 0){
			i++;
			json_get_token_val(token_value, 16, &tokens[i], json);
			wthr->sunset  = atoll(token_value);
		}
	}

	free(tokens);

	return 0;
}

/**
	Get vlaue of JSMN token.

	@param	buf 		Where to store value.
	@param	len 		Destination buffer length.
	@param	token 		JSMN token.
	@param 	json 		JSON string, where this token is.

	@return length of token's value or its negative if buffer too small.
 */
ssize_t json_get_token_val(char *buf,const size_t len, const jsmntok_t *token, const char *json){
	int token_value_len = token->end - token->start;

	if(len < token_value_len+1)
		return -token_value_len;

	memset(buf, 0, len);
	memcpy(buf, &json[token->start], token_value_len);
	return token_value_len;
}

/**
	Sends HTTP GET request to IP-address
	and returns length of retrieved content or error code.

	@param	buf 		Where to store retrieved content.
	@param	len 		Destination buffer length.
	@param	headers		HTTP headers string.
	@param 	dst_ip		Destination IP.

	@return length of retrieved content or error code.
 */
ssize_t send_get_request(char *buf, const size_t len, const char* headers, const char* dst_ip){
	static const unsigned int BUFF_SIZE = 1024;

	struct sockaddr_in dst;
	dst.sin_family = AF_INET;
	dst.sin_port = htons(PORT);
	inet_pton(AF_INET, dst_ip, &dst.sin_addr);

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(connect(sock, (struct sockaddr*)&dst, sizeof(dst)) == -1)
		return errno;

	if(send(sock, headers, strlen(headers), 0) == -1)
		return errno;

	char buffer[BUFF_SIZE];
	int bytes_received = recv(sock, buffer, BUFF_SIZE-1, 0);
	buffer[BUFF_SIZE] = '\0'; // Adding terminating char for strchr function
	close(sock);

	/*
		Response content is always JSON, so we searching for opening bracket
		that detects content start.
		For detect content end we iterating chars from end and searching for closing bracket.
	*/
	char *content = strchr(buffer, '{');
	int content_length = bytes_received - (content - buffer);
	while(content[content_length] != '}')
		content_length--;
	content_length++; // adding closing bracket in content length

	if(len < content_length)
		return -ENOMEM;
	memset(buf, 0, len);
	memcpy(buf, content, content_length);

	content_length++; // adding terminating char in content length
	return content_length;
}

void print_usage() {
	printf("weather CITY [FORMAT][OPTIONS]\n"
			"\n"
			"Output formats:\n"
			"	-R          - Raw output(without parsing)\n"
			"	-J[OPTIONS] - JSON\n"
			"	-H[OPTIONS] - Human readable (default)\n"
			"\n"
			"Options:\n"
			"	d - Description\n"
			"	t - Temperature(Celsius)\n"
			"	p - Pressure\n"
			"	h - Humidity\n"
			"	w - Wind (speed and direction)\n"
			"	c - Cloudiness\n"
			"	S - Sunrise\n"
			"	s - Sunset\n"
			"	T - Timestamp\n");
}