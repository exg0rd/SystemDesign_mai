#!/bin/bash
BASE=http://localhost:8080

echo "=== Create user ==="
curl -s -X POST $BASE/users \
  -H "Content-Type: application/json" \
  -d '{"login":"testuser","password":"testpass","first_name":"Test","last_name":"User","email":"test@example.com"}'
echo

echo "=== Login ==="
TOKEN=$(curl -s -X POST $BASE/auth/login \
  -H "Content-Type: application/json" \
  -d '{"login":"testuser","password":"testpass"}' | grep -o '"token":"[^"]*"' | cut -d'"' -f4)
echo "Token: $TOKEN"

echo "=== Get user by login ==="
curl -s $BASE/users/testuser \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== Search users ==="
curl -s "$BASE/users/search?first_name=Test&last_name=User" \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== Create event ==="
EVENT_RESP=$(curl -s -X POST $BASE/events \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"title":"Test Event","description":"Test description","date":"2026-12-25","location":"Moscow"}')
echo $EVENT_RESP
EVENT_ID=$(echo $EVENT_RESP | grep -o '"id":"[^"]*"' | cut -d'"' -f4)
echo "Event ID: $EVENT_ID"

echo "=== Get all events ==="
curl -s $BASE/events \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== Search events by date ==="
curl -s "$BASE/events/search?date_from=2026-12-01&date_to=2026-12-31" \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== Register for event ==="
curl -s -X POST $BASE/events/$EVENT_ID/register \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== Get event participants ==="
curl -s $BASE/events/$EVENT_ID/participants \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== Get user events ==="
curl -s $BASE/users/me/events \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== Unregister from event ==="
curl -s -X DELETE $BASE/events/$EVENT_ID/unregister \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== Logout ==="
curl -s -X POST $BASE/auth/logout \
  -H "Authorization: Bearer $TOKEN"
echo
