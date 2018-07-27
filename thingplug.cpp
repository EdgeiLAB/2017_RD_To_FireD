#include <Arduino.h>
#include <string.h>
#include "thingplug.h"

typedef void (*mqttCallback)(char*);
using namespace std;



/* The pathes will be created dynamically */
char *mqttPubTopic = NULL;
char *mqttSubTopic = NULL;
char *mqttPubPath  = NULL;
char *mqttRemoteCSE  = NULL;
char *mqttContainer  = NULL;
char *mqttSubscription = NULL;
char *mqttLatest = NULL;

const char frameMqttPubTopic[]  = "/oneM2M/req_msg/%s/%s";  /* appEUI, deviceId */
const char frameMqttSubTopic[]  = "/oneM2M/resp/%s/%s";     /* deviceId, appEUI */
const char frameMqttPubPath[] = "/oneM2M/req/%s/%s";        /* deviceId, appEUI */
const char frameMqttRemoteCSE[] = "/%s/v1_0/remoteCSE-%s";    /* appEUI, deviceId */
const char frameMqttContainer[] = "%s/container-%s";        /* remoteCSE, container */
const char frameMqttSubscription[] = "%s/subscription-%s";  /* container, notifyName */
const char frameMqttLatest[] = "%s/latest"; /*container*/

char *strNL    = new char[BUF_SIZE_SMALL];
char *strExt   = new char[BUF_SIZE_SMALL];
char *strDkey    = new char[BUF_SIZE_SMALL];
char *dataName   = new char[BUF_SIZE_SMALL];
char *dataValue  = new char[BUF_SIZE_SMALL];
//void (*mqttCallback)(char*) = NULL;


static char *address    = new char[BUF_SIZE_SMALL];
static char *userName   = new char[BUF_SIZE_SMALL];
static char *passWord   = new char[BUF_SIZE_SMALL];
static char *deviceId   = new char[BUF_SIZE_SMALL];
static char *passCode   = new char[BUF_SIZE_SMALL];


enum MqttStep step = CREATE_NODE;

/* step 1 - params: appEUI, deviceId, ri, deviceId, deviceId, deviceId */
const char frameCreateNode[] =
"<m2m:req>\
<op>1</op>\
<ty>14</ty>\
<to>/%s/v1_0</to>\
<fr>%s</fr>\
<ri>%s</ri>\
<cty>application/vnd.onem2m-prsp+xml</cty>\
<nm>%s</nm>\
<pc>\
<nod>\
<ni>%s</ni>\
<mga>MQTT|%s</mga>\
</nod>\
</pc>\
</m2m:req>";

/* step 2 - params: AppEUI, deviceId, ri, passCode, deviceId, deviceId, nl */
const char frameCreateRemoteCSE[] = 
"<m2m:req>\
<op>1</op>\
<ty>16</ty>\
<to>/%s/v1_0</to>\
<fr>%s</fr>\
<ri>%s</ri>\
<passCode>%s</passCode>\
<cty>application/vnd.onem2m-prsp+xml</cty>\
<nm>%s</nm>\
<pc>\
<csr>\
<cst>3</cst>\
<csi>%s</csi>\
<rr>true</rr>\
<nl>%s</nl>\
</csr>\
</pc>\
</m2m:req>";

/* step 3-params: cse, deviceId, ri, container, dKey */
const char frameCreateContainer[] = 
"<m2m:req>\
<op>1</op>\
<ty>3</ty>\
<to>%s</to>\
<fr>%s</fr>\
<ri>%s</ri>\
<cty>application/vnd.onem2m-prsp+xml</cty>\
<nm>%s</nm>\
<dKey>%s</dKey>\
<pc>\
<cnt>\
<lbl>con</lbl>\
</cnt>\
</pc>\
</m2m:req>";

/* step 4-params: appEUI, deviceId, ri, deviceId, dkey, strExt */
const char frameCreateMgmtCmd[] = 
"<m2m:req>\
<op>1</op>\
<ty>12</ty>\
<to>/%s/v1_0</to>\
<fr>%s</fr>\
<ri>%s</ri>\
<cty>application/vnd.onem2m-prsp+xml</cty>\
<nm>%s_turnOn</nm>\
<dKey>%s</dKey>\
<pc>\
<mgc>\
<cmt>turnOn</cmt>\
<exe>false</exe>\
<ext>%s</ext>\
</mgc>\
</pc>\
</m2m:req>";

/* step 4-1 - params: mqttContainer, deviceId, strRi, notificationName, uKey, deviceId */
const char frameSubscribe[] =
"<m2m:req>\
<op>1</op>\
<to>%s</to>\
<fr>%s</fr>\
<ty>23</ty>\
<ri>%s</ri>\
<nm>%s</nm>\
<uKey>%s</uKey>\
<cty>application/vnd.onem2m-prsp+xml</cty>\
<pc>\
<sub>\
<enc>\
<rss>1</rss>\
</enc>\
<nu>MQTT|%s</nu>\
<nct>2</nct>\
</sub>\
</pc>\
</m2m:req>";

/* step 5 - params: container, deviceId, ri, dkey, name, value */
const char frameCreateContentInstance[] = 
"<m2m:req>\
<op>1</op>\
<ty>4</ty>\
<to>%s</to>\
<fr>%s</fr>\
<ri>%s</ri>\
<cty>application/vnd.onem2m-prsp+xml</cty>\
<dKey>%s</dKey>\
<pc>\
<cin>\
<cnf>%s</cnf>\
<con>%s</con>\
</cin>\
</pc>\
</m2m:req>";

/* step 6 - params: container, deviceId, ri, dKey, deviceId */
const char frameDeleteSubscribe[] = 
"<m2m:req>\
<op>4</op>\
<to>%s</to>\
<fr>%s</fr>\
<ri>%s</ri>\
<uKey>%s</uKey>\
<cty>application/vnd.onem2m-prsp+xml</cty>\
<pc>\
<sub>\
<enc>\
<rss>1</rss>\
</enc>\
<nu>MQTT|%s</nu>\
<nct>2</nct>\
</sub>\
</pc>\
</m2m:req>";

/*step 6-2 - params: mqttLatest, deviceId, ri, uKey*/
const char frameLatest[] = 
"<m2m:req>\
<op>2</op>\
<to>%s</to>\
<fr>%s</fr>\
<ri>%s</ri>\
<cty>application/vnd.onem2m-prsp+xml</cty>\
<uKey>%s</uKey>\
</m2m:req>";

int mqttConnect(PubSubClient* client, const char* addr, const char* id, const char* pw, const char* devId) 
{
  mqttPubTopic = new char[BUF_SIZE_SMALL];
  mqttSubTopic = new char[BUF_SIZE_SMALL];
  mqttPubPath = new char[BUF_SIZE_SMALL];
  mqttRemoteCSE = new char[BUF_SIZE_SMALL];
  
  strcpy(address, addr);
  strcpy(userName, id);
  strcpy(passWord, pw);
  strcpy(deviceId, devId);

  int rc = 0;

  sprintf(mqttPubTopic, frameMqttPubTopic, APP_EUI, deviceId);
  sprintf(mqttSubTopic, frameMqttSubTopic, deviceId, APP_EUI);
  sprintf(mqttPubPath, frameMqttPubPath, deviceId, APP_EUI);
  sprintf(mqttRemoteCSE, frameMqttRemoteCSE, APP_EUI, deviceId);

  Serial.println(mqttPubTopic);
  Serial.println(mqttSubTopic);
  Serial.println(mqttPubPath);
  Serial.println(mqttRemoteCSE);   

  Serial.println("Attempting MQTT connection...");
  client->setServer(addr, MQTT_BROKER_PORT);
  boolean result = client->connect(deviceId, id, pw);
  if (result)
  {
      Serial.println("Mqtt connected");
      
      // registration of the topics
      Serial.println("MQTT connected");
      step = CREATE_NODE;
      if (client->subscribe(mqttSubTopic)) Serial.println("mqttSubTopic subscribed");
      else Serial.println("mqttSubTopic Not subscribed");
        
      if (client->subscribe(mqttPubTopic)) Serial.println("mqttPubTopic subscribed");
      else Serial.println("mqttPubTopic Not subscribed");

      client->setCallback(callbackArrived);
  }
  else 
  {
      Serial.println("Failed to connect, return code " + String(rc));
      return FALSE;
  }

  delete[] mqttPubTopic;
  delete[] mqttSubTopic;

  mqttPubTopic = NULL;
  mqttSubTopic = NULL;
    
  return TRUE;
}

void callbackArrived(char * topicName, uint8_t * payload, unsigned int len)
{
  Serial.print("MQTT Message arrived: Topic=");
  Serial.println(topicName);
  std::map<string, mqttCallback> callback;
  std::map<string, mqttCallback>::iterator iter;  
  std::pair<string, string> latestData;
  std::vector<pair<string, string>> latestDataVector(1000);
  if(len>0) payload[len]='\0';
  Serial.println("Payload(" +String(len) + "): ");
  Serial.println((char*)payload);

  mqttCallback callbackFunction = NULL;

  // error check
  char strRsc[BUF_SIZE_SMALL];
  char strRi[BUF_SIZE_SMALL];
  char strCon[BUF_SIZE_SMALL];  
  char strRoute[BUF_SIZE_SMALL];
  char notifySubName[BUF_SIZE_SMALL];  
  int rc = 0;

  rc = parseValue(strRsc, (char*)payload, len, "rsc");  
  
  if(rc==MQTTCLIENT_SUCCESS )
  {
    printResultCode(strRsc);
    int resultCode = atoi(strRsc);
    if(((resultCode == 4000) || (resultCode == 4004)) && step == GET_LATEST_DATA_REQUESTED) {
      step = FINISH;
      return;
    }
    else if(resultCode == 4000 || resultCode == 4004) {
      return;
    }

    generateRi(strRi);

    switch(step) {
    case CREATE_NODE_REQUESTED:
          // parse response message
          rc = parseValue(strNL, strstr((char*)payload, "pc"), len, "ri");
          if(rc==MQTTCLIENT_SUCCESS) {
        Serial.println("ri: "+ String(strNL));
        strcpy(strExt, strNL);
        step = CREATE_REMOTE_CSE;
      }
      break;
  
    case  CREATE_REMOTE_CSE_REQUESTED : 
      rc = parseValue(strDkey, (char*)payload, len, "dKey");
      Serial.println("dKey=" + String(strDkey));
      if(rc==MQTTCLIENT_SUCCESS) {
        step = CREATE_CONTAINER;
            }
            break;
  
    case CREATE_CONTAINER_REQUESTED:
      step = CREATE_MGMT_CMD;
      break;
  
    case CREATE_MGMT_CMD_REQUESTED:
      step = SUBSCRIBE;
      break;
  
    case SUBSCRIBE_REQUESTED:
      step = CREATE_CONTENT_INSTANCE;
      break;
  
    case GET_LATEST_DATA_REQUESTED:
    {
      parseValue(strRi, (char*)payload, len, "ri");

      rc = parseValue(strCon, (char*)payload, len, "con");
      if(rc ==MQTTCLIENT_FAILURE) {
        Serial.println("can not find value");
        step = FINISH;
        break;
      }
      string notificationName = "";
      
      for(int i = 0; i < latestDataVector.size(); i++) {
        latestData = latestDataVector.at(i);
        if(latestData.first.compare(strRi) == 0) {
          notificationName = latestData.second;
        }else continue;
      }
      Serial.print("latestData callback : " + String(strRi) + "\t" + String(notificationName.c_str()) + "\t" + String(strCon));


      iter = callback.find(notificationName);
      if(iter != callback.end()) 
        callbackFunction = callback[notificationName];
      else 
        callbackFunction = NULL;
        
      if(strCon != NULL && callbackFunction != NULL) 
        callbackFunction(strCon);
      step = FINISH;
      break;
    }
    default:
      step = FINISH;
      break;
    }
  }
  else
  {
    // Notification from ThingPlug Server
    rc = parseValue(strCon, (char*)payload, len, "con");
    
    parseValue(strRoute, (char*)payload, len, "sr");
    char *pStrRoute = strtok(strRoute, "-");
    while (pStrRoute) {
      strncpy(notifySubName, pStrRoute, BUF_SIZE_SMALL);
      pStrRoute = strtok(NULL, "-");
    }
    //iter를 이용한 반환 필요
    iter = callback.find(notifySubName);
    if(iter != callback.end()) 
      callbackFunction = callback[notifySubName];
    else 
      callbackFunction = NULL;
      
    if(rc==MQTTCLIENT_SUCCESS && callbackFunction!=NULL) callbackFunction(strCon);
  } 

  callback.clear(); 
//  memset(strRsc, 0, BUF_SIZE_SMALL);
//  memset(strRi, 0, BUF_SIZE_SMALL);
//  memset(strCon, 0, BUF_SIZE_SMALL);
//  memset(strRoute, 0, BUF_SIZE_SMALL);
//  memset(notifySubName, 0, BUF_SIZE_SMALL);    
  callbackFunction = NULL;
       
  return; // Do Not Need to be recalled.
}

// generates a unique resource ID
void generateRi(char * buf)
{
  if(buf==NULL) return;
  sprintf(buf, "%s_%lu", deviceId, millis());
}

int parseValue(char* buf, const char * payload, const int len, const char * param)
{
  if(payload==NULL)
  {
    Serial.println("parseValue error: Payload is NULL");
    return MQTTCLIENT_FAILURE;
  } 

  int result = MQTTCLIENT_FAILURE;
  int lenParam = strlen(param);
  
  char tagBegin[BUF_SIZE_SMALL];
  sprintf(tagBegin, "<%s>", param);
  
  char tagEnd[BUF_SIZE_SMALL];
  sprintf(tagEnd, "</%s>", param);

  char * pBegin = strstr(payload, tagBegin);
  if(pBegin==NULL) return result;
  int indexBegin = pBegin - payload;
  if(indexBegin>0){
     char* pEnd = strstr(payload, tagEnd);
     int indexEnd = pEnd - payload;
     int indexValue = indexBegin+lenParam+2;
     int lenValue = indexEnd-indexValue;
     strncpy(buf, &payload[indexValue], lenValue);
     buf[lenValue]='\0';
     result = MQTTCLIENT_SUCCESS;
  }

  return result;
}

void printResultCode(char * buf)
{
  if(buf==NULL) return;
  Serial.print("[result code]: ");
  
  int resultCode = atoi(buf);
  switch(resultCode) {
    case 2000: Serial.println("OK"); break;
    case 2001: Serial.println("CREATED"); break;
    case 2002: Serial.println("DELETED"); break;
    case 2004: Serial.println("UPDATED"); break;
    case 2100: Serial.println("CONTENT_EMPTY"); break;
    case 4105: Serial.println("EXIST"); break;
    case 4004: Serial.println("NOT FOUND"); break;
    default: Serial.println("UNKNOWN ERROR"); break;
  }
}

void checkMqttPacketSize() 
{
  char *bufRequest = new char[BUF_SIZE_LARGE];
  
  if(MQTT_MAX_PACKET_SIZE<BUF_SIZE_LARGE)
  {
    sprintf(bufRequest, "\nERROR: MQTT_MAX_PACKET_SIZE(%d) is smaller than BUF_SIZE_LARGE(%d).", MQTT_MAX_PACKET_SIZE, BUF_SIZE_LARGE);
    Serial.println(bufRequest);
    sprintf(bufRequest, "Please modify MQTT_MAX_PACKET_SIZE for %d on Arduino\\libraries\\pubsubclient\\src\\PubSubClient.h\n\n", BUF_SIZE_LARGE);
    Serial.println(bufRequest);
    while(1){}
  }
  else
  {
    sprintf(bufRequest, "\nPacket Size OK: MQTT_MAX_PACKET_SIZE(%d), BUF_SIZE_LARGE(%d).", MQTT_MAX_PACKET_SIZE, BUF_SIZE_LARGE);
    Serial.println(bufRequest);
  }
  
  delete[] bufRequest;
  bufRequest = NULL;
}

int mqttCreateNode(PubSubClient* client, const char * devPw)
{
  char *bufRequest = new char[BUF_SIZE_LARGE]; 
  checkMqttPacketSize();

  strcpy(passCode, devPw);

  int rc = 0;
  char ri[BUF_SIZE_SMALL];
  generateRi(ri);

  sprintf(bufRequest,frameCreateNode, APP_EUI, deviceId, ri, deviceId, deviceId, deviceId);    
  Serial.println("1. Create Node :");
  Serial.print("payload=");
  Serial.println(bufRequest);

  // publish bufRequest
  step = CREATE_NODE_REQUESTED;
  if(client->publish(mqttPubPath, bufRequest) ) Serial.println("Publish success");
  else Serial.println("Publish failed");
    
  while(step == CREATE_NODE_REQUESTED) { client->loop(); }
    
  Serial.println("Create Node Success");
  Serial.println();

  delete[] bufRequest;
  bufRequest = NULL;
  
  return TRUE;
}

int mqttCreateRemoteCSE(PubSubClient* client)
{
  int rc = 0;
  char *bufRequest = new char[BUF_SIZE_LARGE];
  char strRi[BUF_SIZE_SMALL];
  generateRi(strRi);

  sprintf(bufRequest, frameCreateRemoteCSE, APP_EUI,
          deviceId, strRi, passCode, deviceId, deviceId, strNL);

  Serial.println("2. Create RemoteCSE :");
  Serial.print("payload=");
  Serial.println(bufRequest);
  step = CREATE_REMOTE_CSE_REQUESTED;
  client->publish(mqttPubPath, bufRequest);
  while(step == CREATE_REMOTE_CSE_REQUESTED) client->loop();
  Serial.println("Publish Success\n\n");

  delete[] bufRequest;
  bufRequest = NULL;
    
  return TRUE;
}


int mqttCreateContainer(PubSubClient* client, const char* con)
{
  char *bufRequest = new char[BUF_SIZE_LARGE];
  char container[BUF_SIZE_SMALL]; 
  char strRi[BUF_SIZE_SMALL];
  mqttContainer = new char[BUF_SIZE_SMALL];
  int rc = 0;

  generateRi(strRi);
  strcpy(container, con);
  sprintf(mqttContainer, frameMqttContainer, mqttRemoteCSE, container);
  sprintf( bufRequest, frameCreateContainer,
           mqttRemoteCSE, deviceId, strRi, container, strDkey);

  Serial.println("3. Create Container : ");
  Serial.print("payload= ");
  Serial.println(bufRequest);
  
  step = CREATE_CONTAINER_REQUESTED;
  client->publish(mqttPubPath, bufRequest);
  while(step == CREATE_CONTAINER_REQUESTED) client->loop();
  Serial.println("Publish Success");
  Serial.println();

  delete[] bufRequest;
  delete[] mqttContainer;
  
  bufRequest = NULL;
  mqttContainer = NULL;
  
  return TRUE;
}

int mqttCreateMgmtCmd(PubSubClient* client)
{
  char *bufRequest = new char[BUF_SIZE_LARGE];
  int rc = 0;

  char strRi[BUF_SIZE_SMALL];;
  generateRi(strRi);

  sprintf( bufRequest, frameCreateMgmtCmd,
      APP_EUI, deviceId, strRi, deviceId, strDkey, strExt);

  Serial.println("4. Create Mgmt Cmd : ");
  Serial.print("payload= ");
  Serial.println(bufRequest);
  
  step = CREATE_MGMT_CMD_REQUESTED;
  client->publish(mqttPubPath, bufRequest);
  while(step == CREATE_MGMT_CMD_REQUESTED) client->loop();
  Serial.println("Publish Success");
  Serial.println();

  delete[] bufRequest;
  bufRequest = NULL;
  
  return TRUE;
}

int mqttCreateContentInstance(PubSubClient* client, const char* con, char * dataValue)
{
  char *bufRequest = new char[BUF_SIZE_LARGE];

  mqttRemoteCSE = new char[BUF_SIZE_SMALL];
  mqttContainer = new char[BUF_SIZE_SMALL];
  int rc = 0;
  long count = 0;   
  char container[BUF_SIZE_SMALL];
  char strRi[BUF_SIZE_SMALL];
  generateRi(strRi);

  strcpy(container, con);
  strcpy(dataName, "text"); // data type
  sprintf(mqttRemoteCSE, frameMqttRemoteCSE, APP_EUI, deviceId);
  delay(1);
  sprintf(mqttContainer, frameMqttContainer, mqttRemoteCSE, container);
  delay(1);
  sprintf(bufRequest, frameCreateContentInstance, 
      mqttContainer, deviceId, strRi, strDkey, dataName, dataValue);
  delay(1);
  Serial.println("5. Create Content Instance : ");
  Serial.print("payload= ");
  Serial.println(bufRequest);
  step = CREATE_CONTENT_INSTANCE_REQUESTED;
  client->publish(mqttPubPath, bufRequest);
  while(step == CREATE_CONTENT_INSTANCE_REQUESTED) {
    client -> loop();
    count++;
    if(count >= 999999) {
      Serial.println("Forced Termination");
      step = FINISH;
      count = 0;
      return FALSE; 
    }
  }
  Serial.println("Publish Success");
  Serial.println();
  
  delete[] bufRequest;
  delete[] mqttRemoteCSE;
  delete[] mqttContainer;
  
  bufRequest = NULL;
  mqttRemoteCSE = NULL;
  mqttContainer = NULL;
  

  return TRUE;
}

/*
int mqttSubscribe(PubSubClient* client, const char* targetDevId, const char* con, void (*fp)(char *))
{
    //mqttCallback = fp;

    int rc = 0;

    char strRi[128] = "";
    generateRi(strRi);
    char notifySubName[BUF_SIZE_SMALL] = "";
    strcpy(targetDeviceId, targetDevId);
    strcpy(container, con);
    strcat(notifySubName, container);
    strcat(notifySubName, targetDeviceId);
    
    callback.insert(pair<string, mqttCallback>(notifySubName, fp));

    sprintf(mqttRemoteCSE, frameMqttRemoteCSE, APP_EUI, targetDeviceId);
    sprintf(mqttContainer, frameMqttContainer, mqttRemoteCSE, container);

    sprintf( bufRequest, frameSubscribe,
             mqttContainer, deviceId, strRi, notifySubName, passWord, deviceId);

    printf("4-1. Subscribe :\n payload=%s\n", bufRequest);
    step = SUBSCRIBE_REQUESTED;
    client->publish(mqttPubPath, bufRequest);
    while(step == SUBSCRIBE_REQUESTED) client->loop();
    printf("Publish Success\n\n");

    return TRUE;
}

int mqttDeleteSubscribe(PubSubClient* client, const char* targetDevId, const char* con)
{
    char strRi[128] = "";
    generateRi(strRi);
    char notifySubName[BUF_SIZE_SMALL] = "";
    strcpy(container, con);
    strcpy(targetDeviceId, targetDevId);
    strcat(notifySubName, container);
    strcat(notifySubName, targetDeviceId);

    sprintf(mqttRemoteCSE, frameMqttRemoteCSE, APP_EUI, targetDeviceId);
    sprintf(mqttContainer, frameMqttContainer, mqttRemoteCSE, container);
    sprintf(mqttSubscription, frameMqttSubscription, mqttContainer, notifySubName);
    sprintf(bufRequest, frameDeleteSubscribe, mqttSubscription, deviceId, strRi, passWord, deviceId);

    printf("4-2. Delete Subscribe : \n payload=%s\n", bufRequest);
    step = DELETE_SUBSCRIBE_REQUESTED;
    client -> publish(mqttPubPath, bufRequest);
    while(step == DELETE_SUBSCRIBE_REQUESTED) client -> loop();
    printf("Publish Success\n\n");

    return TRUE;
}
*/

int mqttGetLatest(PubSubClient* client, const char* targetDevId, const char* con, void(*fp)(char*))
{
  std::map<string, mqttCallback> callback;
  std::map<string, mqttCallback>::iterator iter;
  pair<string, string> latestData;
  vector<pair<string, string>> latestDataVector(1000);  
  
  char *bufRequest = new char[BUF_SIZE_LARGE];
   
  mqttRemoteCSE = new char[BUF_SIZE_SMALL];
  mqttContainer = new char[BUF_SIZE_SMALL];
  mqttLatest = new char[BUF_SIZE_SMALL];
  long count = 0;    
  char strRi[BUF_SIZE_SMALL];
  char notificationName[BUF_SIZE_SMALL];
  char targetDeviceId[BUF_SIZE_SMALL];  
  char container[BUF_SIZE_SMALL]; 
 
  generateRi(strRi);
  
  strcpy(container, con);
  strcpy(targetDeviceId, targetDevId);
  strcat(notificationName, container);
  strcat(notificationName, targetDeviceId);
 
  Serial.println(); 
  Serial.println(); 

  latestData = make_pair(strRi, notificationName);
  if(latestDataVector.size() >= 1000) {
    for(int i = 0; i < 500; i++) {
      latestDataVector.erase(latestDataVector.begin());
    }
  }
  latestDataVector.push_back(latestData);
  
  Serial.print("latestData : " + String(strRi) + "\t" + String(notificationName));
  callback.insert(pair<string, mqttCallback>(notificationName, fp));
  callback[notificationName] = fp;
     
  sprintf(mqttRemoteCSE, frameMqttRemoteCSE, APP_EUI, targetDeviceId);
  sprintf(mqttContainer, frameMqttContainer, mqttRemoteCSE, container);
  sprintf(mqttLatest, frameMqttLatest, mqttContainer);
  sprintf(bufRequest, frameLatest, mqttLatest, deviceId, strRi, passWord);

  Serial.println("5. get Latest Data : ");
  Serial.print("payload= ");
  Serial.println(bufRequest);
  
  step = GET_LATEST_DATA_REQUESTED;
  client -> publish(mqttPubPath, bufRequest);       // 누수 발생
  
  while(step == GET_LATEST_DATA_REQUESTED) {
    client -> loop();
    count++;
    if(count >= 999999) {
      Serial.println("Forced Termination");
      step = FINISH;
      count = 0;
      return FALSE; 
    }
  }
  Serial.println("Publish success");
  Serial.println();
    
  delete[] bufRequest;
  delete[] mqttLatest;
  delete[] mqttRemoteCSE;
  delete[] mqttContainer;
//  memset(strRi, 0, BUF_SIZE_SMALL);
//  memset(notificationName, 0, BUF_SIZE_SMALL);
//  memset(targetDeviceId, 0, BUF_SIZE_SMALL);
//  memset(container, 0, BUF_SIZE_SMALL);  
  
  bufRequest = NULL;
  mqttLatest = NULL;
  mqttRemoteCSE = NULL;
  mqttContainer = NULL;
  
  return TRUE;

}
