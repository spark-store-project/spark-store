import os
import requests
import json


# sha=os.system("git rev-parse HEAD")
sha = os.getenv("GIT_COMMIT")
# sha = '48fed26c51a8c42554e45f72f43e49703e04c97f'
#get sha from environment 
url = "https://gitee.com/api/v5/repos/spark-store-project/spark-store/commits/{}/comments".format(sha)

token = os.getenv("gitee_token")

# process = os.popen("git symbolic-ref --short -q HEAD")

body = "构建详情请见" + os.getenv("JENKINS_URL") + "blue/organizations/jenkins/" + os.getenv("JOB_NAME").replace("/", "/detail/") + "/" + str(os.getenv("BUILD_ID"))

# process.close()

d = {
    'access_token': token,
    "body": body
}

h = {
    "Content-Type": "application/json;charset=UTF-8"
}

res = requests.post(url,headers=h, data=json.dumps(d))
# print(res.status_code)
# print(res.content)