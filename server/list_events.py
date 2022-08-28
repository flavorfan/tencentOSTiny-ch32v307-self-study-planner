import datetime
from cal_setup import get_calendar_service

import os 
if 0:
    os.environ['http_proxy'] = 'http://10.71.2.78:8001'
    os.environ['https_proxy'] = 'http://10.71.2.78:8001'
else:
    os.environ['http_proxy'] = 'http://127.0.0.1:8001'
    os.environ['https_proxy'] = 'http://127.0.0.1:8001'

# id ='primary'
id = 'k0mgl1e1k6s6ls497shujei18c@group.calendar.google.com'  # study

def main():
    service = get_calendar_service()
    # Call the Calendar API
    now = datetime.datetime.utcnow().isoformat() + 'Z' # 'Z' indicates UTC time
    print('Getting List o 10 events')
    events_result = service.events().list(calendarId=id, timeMin=now,
                                        maxResults=10, singleEvents=True,
                                        orderBy='startTime').execute()
    events = events_result.get('items', [])

    if not events:
        print('No upcoming events found.')
    for event in events:
        start = event['start'].get('dateTime', event['start'].get('date'))
        print(start, event['summary'])

if __name__ == '__main__':
    main()