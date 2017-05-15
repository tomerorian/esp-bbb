/*
  This a simple example of the aREST Library for the ESP8266 WiFi chip.
  See the README file for more details.

  Written in 2015 by Marco Schwartz under a GPL license.
*/

// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>
#include <EEPROM.h>
#include "wifi.h"

// Create aREST instance
aREST rest = aREST();

// WiFi parameters
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;


// BBB site creds
String bbb_username;
String bbb_password;

// 10bis site creds
String bis_username;
String bis_password;

// The port to listen for incoming TCP connections
#define LISTEN_PORT 80

// String lengths
#define BBB_USERNAME_ADDRESS 0
#define BBB_USERNAME_LEN 30
#define BBB_PASSWORD_ADDRESS 30
#define BBB_PASSWORD_LEN 19
#define BIS_USERNAME_ADDRESS 49
#define BIS_USERNAME_LEN 30
#define BIS_PASSWORD_ADDRESS 79
#define BIS_PASSWORD_LEN 19

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

// Declare functions to be exposed to the API
int set_bbb_username(String username);
int set_bbb_password(String password);
int set_bis_username(String username);
int set_bis_password(String password);

/** 
 *  Saves a string to the EEPROM
 *  
 *  @param start Starting location (first 2 bytes are used by the system)
 *  @param buffer_size The size to use (min 3 for empty string)
 *  @param string The string to save
 *  
 *  Note: First 2 bytes are used by the method to indicate a string, last byte must be '\0'
 */
void save_string(int start, int buffer_size, String string)
{
  int str_max_len = buffer_size - 3;
  
  if (str_max_len < 0) {
    return;
  }

  // String indicator
  EEPROM.write(start, 123);
  EEPROM.write(start + 1, 234);

  start += 2;
  int last = start;
  for (int i = start; i - start < string.length() && i - start < str_max_len; i++) {
    EEPROM.write(i, string[i-start] - '\0');
    last = i;
  }

  EEPROM.write(last+1, '\0');
}

String load_string(int start) {
  if (EEPROM.read(start) != 123 || EEPROM.read(start + 1) != 234) {
    return "";
  }

  String ret = "";
  int i = start + 2;
  char value = char(EEPROM.read(i));
  
  while (value != '\0') {
    ret += value;
    i += 1;
    value = char(EEPROM.read(i));
  }

  return ret;
}

void wifi_connect() 
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());  
}

void setup_rest() 
{
  // Set ID and name
  rest.set_id("123456");
  rest.set_name("beef-bun-button");

  // Variables
  // TODO: Set if I want to
//  rest.variable("bbb-username", &bbb_username);
//  rest.variable("bbb-password", &bbb_password);
//  rest.variable("bis-username", &bis_username);
//  rest.variable("bis-password", &bis_password);
  
  // Function to be exposed
  rest.function("bbb-set-username", set_bbb_username);
  rest.function("bbb-set-password", set_bbb_password);
  rest.function("bis-set-username", set_bis_username);
  rest.function("bis-set-password", set_bis_password);
}

void load_credentials()
{
  EEPROM.begin(512);
  bbb_username = load_string(BBB_USERNAME_ADDRESS);
  bbb_password = load_string(BBB_PASSWORD_ADDRESS);
  bis_username = load_string(BIS_USERNAME_ADDRESS);
  bis_password = load_string(BIS_PASSWORD_ADDRESS);
  EEPROM.end();
}

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  load_credentials();

  setup_rest();

  wifi_connect();
}

void handle_rest() 
{
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  int client_timeout = 1000;
  while(!client.available() && client_timeout > 0) {
    delay(1);
    client_timeout -= 1;
  }

  if (client_timeout <= 0) {
    return;
  }

  rest.handle(client);  
}

void loop() 
{
  handle_rest();
}

// Custom function accessible by the API
int set_bbb_username(String username)
{
  username.trim();

  if (username.length() > BBB_USERNAME_LEN - 3) {
    return 1;
  }
  
  bbb_username = username; 

  EEPROM.begin(512);
  save_string(BBB_USERNAME_ADDRESS, BBB_USERNAME_LEN, bbb_username);
  EEPROM.end();
  
  return 0;
}

int set_bbb_password(String password)
{
  password.trim();

  if (password.length() > BBB_PASSWORD_LEN - 3) {
    return 1;
  }
  
  bbb_password = password; 

  EEPROM.begin(512);
  save_string(BBB_PASSWORD_ADDRESS, BBB_PASSWORD_LEN, bbb_password);
  EEPROM.end();
  
  return 0;
}

int set_bis_username(String username)
{
  username.trim();

  if (username.length() > BIS_USERNAME_LEN - 3) {
    return 1;
  }
  
  bis_username = username;

  EEPROM.begin(512);
  save_string(BIS_USERNAME_ADDRESS, BIS_USERNAME_LEN, bis_username);
  EEPROM.end();
  
  return 0;
}

int set_bis_password(String password)
{
  password.trim();

  if (password.length() > BIS_PASSWORD_LEN - 3) {
    return 1;
  }
  
  bis_password = password;

  EEPROM.begin(512);
  save_string(BIS_PASSWORD_ADDRESS, BIS_PASSWORD_LEN, bis_password);
  EEPROM.end();
  
  return 0;
}

