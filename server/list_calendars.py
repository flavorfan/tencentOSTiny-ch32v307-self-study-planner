# import datetime
from cal_setup import get_calendar_service

import os 
# set os env proxy 
if 0:
    os.environ['http_proxy'] = 'http://10.71.2.78:8001'
    os.environ['https_proxy'] = 'http://10.71.2.78:8001'
else:
    os.environ['http_proxy'] = 'http://127.0.0.1:8001'
    os.environ['https_proxy'] = 'http://127.0.0.1:8001'
    
def main():
    service = get_calendar_service()
    # Call the Calendar API
    print('Getting list of calendars')
    calendars_result = service.calendarList().list().execute()

    calendars = calendars_result.get('items', [])

    if not calendars:
        print('No calendars found.')
    for calendar in calendars:
        summary = calendar['summary']
        id = calendar['id']
        primary = "Primary" if calendar.get('primary') else ""
        print("%s\t%s\t%s" % (summary, id, primary))

if __name__ == '__main__':
    main()