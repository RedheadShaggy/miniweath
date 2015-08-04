/*
	TODO Add output format options
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "jsmn/jsmn.h"
#include "miniweath.h"

int main(int argc, char const *argv[]){
	if(argc < 2 || argv[1][0] == '-'){
		print_usage();
		return 0;
	}

	struct weather cur_weather;
	if(request_weather(&cur_weather, argv[1]) != 0)
		return -1;

	if(argc == 3 && argv[2][0] == '-'){
		switch(argv[2][1]){
			case 'J':
				print_json(&cur_weather);
				break;
			default:
				print_usage();
		}
	} else {
		print_human(&cur_weather);
	}

	return 0;
}

/**
	Get current weather from OpenWeatherMap.org for city.

	Sends HTTP GET request to api.openweather.org, parse json response
	and store needed values in given weather structure.

	@param	wthr 		Weather struct to store data.
	@param	city 		I think you get it.

	@return 0 if success or negative error code.
 */
int request_weather(struct weather *wthr,const char *city){
	char *headers = malloc(128);
	snprintf(headers, 128, HTTPGET_HEADERS, city, HOST);

	char *buffer = malloc(512);
	int content_length = send_get_request(buffer, 512, headers, IP_ADDR);
	free(headers);
	if(content_length <= 0) {
		free(buffer);
		return content_length; // If content_length is negative, that means a error occured, we return error code.
	}

	char *json = malloc(content_length);
	strcpy(json, buffer);
	free(buffer);

	if(json_to_weather(wthr, json) != 0){
		free(json);
		return -1;
	}

	free(json);

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
	
	/* Count tokens and allocate place for them. */
	jsmn_init(&json_parser);
	int tokens_num =  jsmn_parse(&json_parser, json, strlen(json), NULL, 0);

	if(tokens_num < 0)
		return tokens_num;
	if(tokens_num == 0)
		return JSMN_ERROR_PART;

	/* Re-init json_parser for resetting position and parse the actual tokens. */
	jsmntok_t *tokens = malloc(tokens_num * sizeof *tokens);
	jsmn_init(&json_parser);
	tokens_num = jsmn_parse(&json_parser, json, strlen(json), tokens, tokens_num);

	if(parse_tokens(wthr, tokens, tokens_num, json) != 0){
		free(tokens);
		return JSMN_ERROR_PART;
	}

	free(tokens);

	return 0;
}

/**
	Parse jsmn tokens and store needed in weather struct.

	@param	wthr 		Weather structure for storing values.
	@param 	tokens 		Array with jsmn tokens.
	@param 	len 		Length of array with jsmn tokens.
	@param 	json 		Json string, where this tokens placed.

	@return 0 if success or -1 if there's no weather information.
 */
int parse_tokens(struct weather *wthr, const jsmntok_t *tokens, const size_t len, const char *json){
	/* If first key in JSON is "cod", that means there's no weather information. */
	char token_key[16];
	json_get_token_val(token_key, 16, &tokens[1], json);
	if(strcmp(token_key, "cod") == 0)
		return -1;

	int i;
	for(i = 0; i < len; i++){
		/* If token is a string and has 1 child, that means this token is a key */
		if(tokens[i].type == JSMN_STRING && tokens[i].size == 1){
			/* And if his child is primitive or string (not an object or something else), then it's probably needed token */
			if(tokens[i+1].type == JSMN_PRIMITIVE || tokens[i+1].type == JSMN_STRING){
				/* At this step we have key-value pair (i and i+1) */
				char token_value[16];
				json_get_token_val(token_key, 16, &tokens[i], json);
				json_get_token_val(token_value, 16, &tokens[i+1], json);

				if(strcmp(token_key, "temp") == 0){wthr->temperature  = atoll(token_value);}
				else if(strcmp(token_key, "pressure") == 0){wthr->pressure  = atoll(token_value);}
				else if(strcmp(token_key, "humidity") == 0){wthr->humidity  = atoll(token_value);}
				else if(strcmp(token_key, "speed") == 0){wthr->wind_speed  = atoll(token_value);}
				else if(strcmp(token_key, "deg") == 0){wthr->wind_direction  = atoll(token_value);}
				else if(strcmp(token_key, "all") == 0){wthr->cloudiness  = atoll(token_value);}
				else if(strcmp(token_key, "sunrise") == 0){wthr->sunrise  = atoll(token_value);}
				else if(strcmp(token_key, "sunset") == 0){wthr->sunset  = atoll(token_value);}
				else if(strcmp(token_key, "description") == 0){strcpy(wthr->description, token_value);}
				else if(strcmp(token_key, "name") == 0){strcpy(wthr->city, token_value);}
			}
		}
	}

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
	struct sockaddr_in dst;
	dst.sin_family = AF_INET;
	dst.sin_port = htons(PORT);
	inet_pton(AF_INET, dst_ip, &dst.sin_addr);

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(connect(sock, (struct sockaddr*)&dst, sizeof(dst)) == -1)
		return errno;

	if(send(sock, headers, strlen(headers), 0) == -1)
		return errno;

	char buffer[1024];
	int bytes_received = recv(sock, buffer, 1024-1, 0);
	buffer[1024-1] = '\0'; // Adding terminating char for strchr function
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

/* -----------------------------------------------------------
 * Printing functions
 */

void print_usage() {
	printf("miniweath CITY [FORMAT]\n"
			"\n"
			"Output formats:\n"
			"	-J - JSON\n");
}

void print_human(const struct weather *wthr){
	time_t raw_time;
	char time_buf[10];

	printf("%s\n\n", wthr->city);
	printf("    %d\u00B0C\n\n", wthr->temperature);
	printf("Wind        %d m/s to %d\u00B0\n", wthr->wind_speed, wthr->wind_direction);
	printf("Cloudiness  %s (%d%%)\n", wthr->description, wthr->cloudiness);
	printf("Pressure    %d hpa\n", wthr->pressure);
	printf("Humidity    %d%%\n", wthr->humidity);

	raw_time = wthr->sunrise;
	strftime (time_buf, 10, "%R", localtime (&raw_time));
	printf("Sunrise     %s\n", time_buf);

	raw_time = wthr->sunset;
	strftime (time_buf, 10, "%R", localtime (&raw_time));
	printf("Sunset      %s\n", time_buf);
}

void print_json(const struct weather *wthr){
	time_t timer;

	time(&timer);
	printf("{\"dt\":%ld, ", timer);
	printf("\"name\":\"%s\", ", wthr->city);
	printf("\"description\":\"%s\", ", wthr->description);
	printf("\"cloudiness\":%d, ", wthr->cloudiness);
	printf("\"wind_speed\":%d, ", wthr->wind_speed);
	printf("\"wind_direction\":%d, ", wthr->wind_direction);
	printf("\"pressure\":%d, ", wthr->pressure);
	printf("\"temp\":%d, ", wthr->temperature);
	printf("\"humidity\":%d, ", wthr->humidity);
	printf("\"sunrise\":%d, ", wthr->sunrise);
	printf("\"sunset\":%d}\n", wthr->sunset);
}
