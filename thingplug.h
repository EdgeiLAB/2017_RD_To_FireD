#ifndef __THINGPLUG_H__
#define __THINGPLUG_H__

#include <PubSubClient.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <vector>

#define TRUE      1
#define FALSE     0
#define APP_EUI           "thingplug"
#define BUF_SIZE_SMALL      128
#define BUF_SIZE_LARGE      1024
#define MQTT_BROKER_PORT    1883

#define MQTTCLIENT_SUCCESS  0
#define MQTTCLIENT_FAILURE  1

using namespace std;

/* MQTT Process Steps */
enum MqttStep {
  CREATE_NODE,
  CREATE_NODE_REQUESTED,
  CREATE_REMOTE_CSE,
  CREATE_REMOTE_CSE_REQUESTED,
  CREATE_CONTAINER,
  CREATE_CONTAINER_REQUESTED,
  CREATE_MGMT_CMD,
  CREATE_MGMT_CMD_REQUESTED,
  SUBSCRIBE,
  SUBSCRIBE_REQUESTED,
  CREATE_CONTENT_INSTANCE,
  CREATE_CONTENT_INSTANCE_REQUESTED,
  DELETE_SUBSCRIBE,
  DELETE_SUBSCRIBE_REQUESTED,
  GET_LATEST_DATA,
  GET_LATEST_DATA_REQUESTED,
  FINISH
};

/* Functions */
int mqttConnect(PubSubClient* client, const char* addr, const char* id, const char* pw, const char* devId);

void callbackArrived(char * topicName, uint8_t * payload, unsigned int len);

/* generates a unique resource ID */
void generateRi(char * buf);

int parseValue(char* buf, const char * payload, const int len, const char * param);

void printResultCode(char * buf);

void checkMqttPacketSize();

int mqttCreateNode(PubSubClient* client, const char * devPw);

int mqttCreateRemoteCSE(PubSubClient* client);

int mqttCreateContainer(PubSubClient* client, const char* con);

int mqttCreateMgmtCmd(PubSubClient* client);

int mqttCreateContentInstance(PubSubClient* client, const char* con, char * dataValue);

int mqttSubscribe(PubSubClient* client, const char* targetDevId, const char* con, void (*fp)(char *));

int mqttDeleteSubscribe(PubSubClient* client, const char* targetDevId, const char* con);

int mqttGetLatest(PubSubClient* client, const char* targetdevId, const char* con, void(*fp)(char*));

#endif

