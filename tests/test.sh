#!/bin/bash
BASE=http://localhost:8080

echo "=== create user ==="
curl -s -X POST $BASE/users \
  -H "Content-Type: application/json" \
  -d '{"login":"exg0rd","password":"secret","first_name":"Egor","last_name":"Sayapin","email":"egor@example.com"}'
echo

echo "=== duplicate user (expect 409) ==="
curl -s -o /dev/null -w "%{http_code}" -X POST $BASE/users \
  -H "Content-Type: application/json" \
  -d '{"login":"exg0rd","password":"secret","first_name":"Egor","last_name":"Sayapin","email":"egor@example.com"}'
echo

echo "=== login ==="
TOKEN=$(curl -s -X POST $BASE/auth/login \
  -H "Content-Type: application/json" \
  -d '{"login":"exg0rd","password":"secret"}' | sed 's/.*"token":"\([^"]*\)".*/\1/')
echo "Token: $TOKEN"

echo "=== login wrong password (expect 401) ==="
curl -s -o /dev/null -w "%{http_code}" -X POST $BASE/auth/login \
  -H "Content-Type: application/json" \
  -d '{"login":"exg0rd","password":"wrong"}'
echo

echo "=== get user by login ==="
curl -s $BASE/users/exg0rd \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== get user not found (expect 404) ==="
curl -s -o /dev/null -w "%{http_code}" $BASE/users/nobody \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== search users by name mask ==="
curl -s "$BASE/users/search?first_name=Egor&last_name=Saya" \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== create event ==="
EVENT=$(curl -s -X POST $BASE/events \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"title":"MAI Lecture","description":"System design","event_date":"2026-09-01","location":"Moscow"}')
echo $EVENT
EVENT_ID=$(echo $EVENT | sed 's/.*"id":\([0-9]*\),.*/\1/')
echo "Event ID: $EVENT_ID"

echo "=== get all events ==="
curl -s $BASE/events \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== search events by date ==="
curl -s "$BASE/events/search?date=2026-09-01" \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== register on event ==="
curl -s -X POST $BASE/events/$EVENT_ID/participants \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== register again (expect 409) ==="
curl -s -o /dev/null -w "%{http_code}" -X POST $BASE/events/$EVENT_ID/participants \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== get participants of event ==="
curl -s $BASE/events/$EVENT_ID/participants \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== get user events ==="
# get user id first
USER_ID=$(curl -s $BASE/users/exg0rd \
  -H "Authorization: Bearer $TOKEN" | sed 's/.*"id":\([0-9]*\).*/\1/')
echo "User ID: $USER_ID"
curl -s $BASE/users/$USER_ID/events \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== cancel registration ==="
curl -s -X DELETE $BASE/events/$EVENT_ID/participants \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== participants after cancel (should be empty) ==="
curl -s $BASE/events/$EVENT_ID/participants \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== access without token (expect 401) ==="
curl -s -o /dev/null -w "%{http_code}" $BASE/events
echo

echo "=== logout ==="
curl -s -X POST $BASE/auth/logout \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== access after logout (expect 401) ==="
curl -s -o /dev/null -w "%{http_code}" $BASE/events \
  -H "Authorization: Bearer $TOKEN"
echo
