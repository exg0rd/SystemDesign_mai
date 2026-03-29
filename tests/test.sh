#!/bin/bash
BASE=http://localhost:8080

echo "=== create user ==="
curl -s -X POST $BASE/users \
  -H "Content-Type: application/json" \
  -d '{"login":"exg0rd","password":"secret","first_name":"egor","last_name":"sayapin","email":"esaiapin@egor.com"}'
echo

echo "=== duplictae user (expect 409) ==="
curl -s -o /dev/null -w "%{http_code}" -X POST $BASE/users \
  -H "Content-Type: application/json" \
  -d '{"login":"exg0rd","password":"secret","first_name":"egor","last_name":"sayapin","email":"saiapin@egor.com}'
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

echo "=== sech users by name ==="
curl -s "$BASE/users/search?first_name=egor&last_name=sayapin" \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== create event ==="
curl -s -X POST $BASE/events \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"title":"mai lecture","description":"system design lecture","date":"2026-03-29","location":"Moscow"}'
echo

echo "=== get all event ==="
curl -s $BASE/events \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== access with out token  401 ==="
curl -s -o /dev/null -w "%{http_code}" $BASE/events
echo

echo "=== log out ==="
curl -s -X POST $BASE/auth/logout \
  -H "Authorization: Bearer $TOKEN"
echo

echo "=== access after log out 401 ==="
curl -s -o /dev/null -w "%{http_code}" $BASE/events \
  -H "Authorization: Bearer $TOKEN"
echo
