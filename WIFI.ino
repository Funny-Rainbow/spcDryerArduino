#include  <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define dryerUrl "http://spcdryer.orgfree.com/spcWeb.php"

int adoptedData = 0;
int dryerCondition = 1;//1 working, 2 resting
//typedef struct{
//  uint32_t V1;
//  uint8_t VerifyTest1;
//}VerifyString;
//VerifyString Verify;
//spi_flash_read(Sector_VERIFY_INFO*4096,(uint32 *)&Verify.VerifyTest1, 1);
//}
long s;
unsigned long lastMs = 0;
static WiFiClient espClient;

// SSID to connect to
char ssid[] = "UM_SECURED_WLAN";
char username[] = "dc02790";
char identity[] = "dc02790";
char password[] = "fxxiaoqikai124";

uint8_t target_esp_mac[6] = {0x30, 0x0a, 0xc4, 0x9a, 0x58, 0x28};//mac地址很奇怪，我修改的一般为第一位，但是奇数总是不可用

extern "C" {
#include "user_interface.h"
#include "wpa2_enterprise.h"
#include "c_types.h"
}

void allSetup() {

  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  delay(1000);
  Serial.setDebugOutput(true);
  Serial.printf("SDK version: %s\n", system_get_sdk_version());
  Serial.printf("Free Heap: %4d\n",ESP.getFreeHeap());

  // Setting ESP into STATION mode only (no AP mode or dual mode)
  //wifi connection
  wifi_set_opmode(STATION_MODE);

  struct station_config wifi_config;

  memset(&wifi_config, 0, sizeof(wifi_config));
  strcpy((char*)wifi_config.ssid, ssid);
  strcpy((char*)wifi_config.password, password);

  wifi_station_set_config(&wifi_config);
  wifi_set_macaddr(STATION_IF,target_esp_mac);
  

  wifi_station_set_wpa2_enterprise_auth(1);

  // Clean up to be sure no old data is still inside
  wifi_station_clear_cert_key();
  wifi_station_clear_enterprise_ca_cert();
  wifi_station_clear_enterprise_identity();
  wifi_station_clear_enterprise_username();
  wifi_station_clear_enterprise_password();
  wifi_station_clear_enterprise_new_password();
  
  wifi_station_set_enterprise_identity((uint8*)identity, strlen(identity));
  wifi_station_set_enterprise_username((uint8*)username, strlen(username));
  wifi_station_set_enterprise_password((uint8*)password, strlen((char*)password));  
  wifi_station_connect();
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    Serial.print(WiFi.status());
  }
  WiFi.setAutoReconnect(true);//设置保持wifi连接和自动重连
  WiFi.persistent(true);
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
    Serial.begin(115200);
    
    // 初始化 wifi
    allSetup();
}


void httpClientRequest(const char* postdata){
  HTTPClient httpClient;
  WiFiClient client;
  httpClient.begin(client,dryerUrl);
  Serial.println("URL: ");
  Serial.println(dryerUrl);
  httpClient.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = httpClient.POST(postdata);
  Serial.println(httpCode);
    if (httpCode == HTTP_CODE_OK) {
    String responsePayload = httpClient.getString();
    Serial.println("Server Response Payload: ");
    Serial.println(responsePayload);
  } else {
    Serial.println("Server Respose Code：");
    Serial.println(httpCode);
  }
  httpClient.end();
}

//dryerNum=2r&dryerCondition=78

int data = 0;
void dataDealing(){
  data+=1;
}


//ADC发送A0引脚数据,并预处理
void handleData()
{
  int shakingAvg = 0;
  int i;//每次采集的数据为600次机器周期采集数据的平均值
  for(i=0; i<300; i++)
  {
    if (analogRead(A0)<1000){
      shakingAvg = 200;
    }
    delay(10);
  }
    adoptedData=adoptedData*0.3+shakingAvg;//对数据进行加权
    dryerCondition = 2;
    Serial.println("dryerCondition:");
    Serial.println(adoptedData);
    if (adoptedData>50){
      const char* postdata = "dryerNum=2r&dryerCondition=2";
      httpClientRequest(postdata);
    }
    else{
      const char* postdata = "dryerNum=2r&dryerCondition=1";
      httpClientRequest(postdata);
    }
}

void loop()//循环函数
{
  
  delay(500);
  handleData();
}
