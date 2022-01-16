#!/bin/sh

echo "From:sparkstorefeedback@163.com
To:sparkstorefeedback@163.com
Subject: spark-store_3.0.2: $(lsb_release -a | grep "Description" | sed -e "s#\t#@#" | cut -d "@" -f 2)

$(uname -a)" | tee /tmp/spark-store-install/feedback.txt > /dev/null

curl -s --url "smtp://smtp.163.com" --mail-from "sparkstorefeedback@163.com" --mail-rcpt "sparkstorefeedback@163.com" --upload-file /tmp/spark-store-install/feedback.txt --user "sparkstorefeedback@163.com:YWYGLQNOPLWNNJJY"
