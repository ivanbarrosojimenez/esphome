import json
from datetime import datetime, timezone
from test_integration_real_calendar import make_event_record
from test_calc_start_index import calc_event_height, SCREEN_H, VISIBILITY_THRESHOLD, DEVICE1_VISIBILITY_THRESHOLD

with open('tests/fixtures/calendar_real.json','r',encoding='utf-8') as f:
    arr=json.load(f)

now = datetime(2026,2,8,12,0,0,tzinfo=timezone.utc).astimezone()
events=[]
for o in arr:
    r=make_event_record(o, now)
    if r:
        events.append(r)

start=7
print('total events',len(events))
y=0
count=0
for i in range(start,len(events)):
    next_same=(i+1<len(events) and events[i]['day']==events[i+1]['day'])
    h=calc_event_height(events[i]['title'], events[i]['location'], next_same, 16)
    vis=VISIBILITY_THRESHOLD
    req=int((h*vis)+0.999)
    print(i, events[i]['title'][:40].ljust(40), 'h=',h,'req=',req,'y_before=',y)
    if y+req>SCREEN_H:
        print('break at i',i,'y',y,'req',req,'SCREEN_H',SCREEN_H)
        break
    count+=1
    y+=h+1
print('count',count)
