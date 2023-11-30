SERVER='http://10.10.10.1:3000/signalk/v1'
USERNAME='pi'
PASSWORD='raspberry'
KEY='cell/voltage'
VALUE=$1
token=`curl --silent -H 'Content-Type: application/json' -X POST ${SERVER}/auth/login -d "{\"username\": \"${USERNAME}\", \"password\": \"${PASSWORD}\"}" | cut -d'"' -f 4` 
SERVER='http://192.168.178.96:3002/signalk/v1'
curl -H 'Content-Type: application/json' -X PUT -H "Authorization: Bearer $token" ${SERVER}/api/vessels/self/${KEY} -d "{\"value\": \"${VALUE}\"}"
