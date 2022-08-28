import random
import time
from paho.mqtt import client as mqtt_client
import os 
import datetime
import pandas as pd
from time import time 

import json
from cal_setup import *

calendar_id = 'k0mgl1e1k6s6ls497shujei18c@group.calendar.google.com'  # study
broker = "0WTHPHECX4.iotcloud.tencentdevices.com"
port = 1883

g_pomodoro_status = {
    "toco_tal": 0,
    "toco_cnt": 0,
    "toco_cur": "None",
    "status": "idle",
    "p_cnt": 0,
    "w_cnt": 0,
    "b_cnt": 0
}

# ser-1 material model base, json based
# if 1:
client_id = "0WTHPHECX4dev001"
user_name = "0WTHPHECX4dev001;21010406;12365;4294967295"
password = "558a5aebdee041f0b544e415014dd7db6a2e94da;hmacsha1"   


# 自定义topic
sub_topic = "0WTHPHECX4/dev001/data"
pub_topic = "$thing/up/event/0WTHPHECX4/dev001"    

pub_topic_fmt = '{{"method":"event","tm":{0},"todo_cur":"{1}","todo_tal":{2}}}'

# get timestamp
# tm = datetime.datetime.now().timestamp()

# else:
#     # ser-2 self define 
#     client_id = "PEQCPEN2CKdev001"
#     user_name = "PEQCPEN2CKdev001;21010406;12365;4294967295"
#     password = "5b19e07ab2c47e80be3568dc1be37417de00d41b;hmacsha1"
#     # 物模型
#     pub_topic = "$thing/up/raw/PEQCPEN2CK/dev001"
#     # sub_topic = "$thing/down/raw/PEQCPEN2CK/dev001"
#     # 自定义topic
#     # sub_topic = "PEQCPEN2CK/dev001/data"
#     # pub_topic = "PEQCPEN2CK/dev001/data"
#     # e.g. subscribe([("my/topic", 0), ("another/topic", 2)])
#     sub_topic =[("$thing/down/raw/PEQCPEN2CK/dev001", 0),("PEQCPEN2CK/dev001/data", 2)]

# get calenda info 
# Todo: automate refresh df_event (by day)
def get_calendar_events(calendar_id):
    service = get_calendar_service()

    tomorrow = datetime.date.today().strftime('%Y-%m-%dT16:00:00.0000000Z')  # +8
    today = (datetime.date.today() - datetime.timedelta(days=1)).strftime('%Y-%m-%dT16:00:00.0000000Z')  # +8
    print(today, tomorrow)

    events_result = service.events().list(calendarId=calendar_id, timeMin=today,
                                            timeMax=tomorrow,
                                            singleEvents=True,
                                            orderBy='startTime').execute()
    events = events_result.get('items', [])
    # events to dataframe
    event_ls =[]
    for event in events:
        start = event['start'].get('dateTime', event['start'].get('date'))
        end = event['end'].get('dateTime', event['end'].get('date'))
        summary = event['summary']
        event_id = event['id']
        event_ls.append((event_id,summary, start,end))    
    df_event = pd.DataFrame(event_ls,columns=["id","summary","start","end"])
    df_event['done'] = 0
    print(df_event)
    return df_event

def update_events_statu(df_event, summary):
    df_event.loc[df_event.summary == summary,'done'] = 1

def get_first_undo_event(df_event):
    if  df_event[df_event.done == 0].shape[0] > 0:
        summary = (
            df_event[df_event.done == 0]
            .sort_values('start')
            .iloc[0]['summary']
        ) 
        return summary
    return None

def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)
    
    def on_subscribe(client, userdata, mid, granted_qos):
        print("Subscribed: " + str(mid) + " " + str(granted_qos))

    def on_publish(mosq, obj, mid):
        print("mid: " + str(mid))


    client = mqtt_client.Client(client_id)
    client.username_pw_set(user_name, password)

    client.on_subscribe = on_subscribe
    client.on_publish = on_publish
    client.on_connect = on_connect

    client.connect(broker, port)
    return client


def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
        # {"clientToken":"1000","method":"report","params":{"b_cnt":0,"p_cnt":0,"status":"idle","todo_cnt":0,"todo_cur":"None","todo_tal":0,"w_cnt":0}}        
        rlt = json.loads(msg.payload.decode())
        status = rlt["params"].get("status","")
        todo_tal = rlt["params"].get("todo_tal",0)
        todo_cnt = rlt["params"].get("todo_cnt",0)
        #  1: receive the first event
        if( status == "idle"  and todo_tal == 0):
            print("status: idle > get calendar and send todo_cur")
            summary = get_first_undo_event(df_event)
            g_pomodoro_status["toco_cur"] = summary
            print(f"send {summary}")
            client.publish(pub_topic, pub_topic_fmt.format(int(time()), summary, df_event.shape[0]))
            
        # 2: update the event(todo) done status to true
        elif (todo_cnt > g_pomodoro_status["toco_cnt"]):
            print("rec todo_cnt > toco_cnt > ")
            update_events_statu(df_event, g_pomodoro_status["toco_cur"])
            g_pomodoro_status["toco_cnt"] = todo_cnt
            summary = get_first_undo_event(df_event)
            if summary is not None:
                g_pomodoro_status["toco_cur"] = summary
                print(f"send {summary}")
                client.publish(pub_topic, pub_topic_fmt.format(int(time()), summary, df_event.shape[0]))
            #else all done 
            else:
                print("all todo is clear")        
        # Todo: save all report to database        
        # display df_event current status
        print(df_event)

    print(sub_topic)
    client.subscribe(sub_topic)
    client.on_message = on_message
    
def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()

def publish(client):
     msg_count = 0
     while True:
        time.sleep(10)       
        msg = '{"method":"report","from":"server","clientToken":"' + f"{msg_count:08d}" + '","params":{"status":"start"}}'
        result = client.publish(pub_topic, msg)
        # result: [0, 1]
        status = result[0]
        if status == 0:
            print(f"Send `{msg}` to topic `{pub_topic}`")
        else:
               print(f"Failed to send message to topic {pub_topic}")
        msg_count += 1


def wait_loop():
     msg_count = 0
     while True:
        time.sleep(5)
        print(f"{msg_count:08d}")        
        msg_count += 1




    wait_loop()
    


if __name__ == '__main__':
    # get today calenda
    df_event = get_calendar_events(calendar_id)

    run()
