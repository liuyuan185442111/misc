# -*- coding: utf-8 -*-

'''
【Python】requests 详解超时和重试
https://www.cnblogs.com/gl1573/p/10129382.html

Python Requests中异常总结
https://blog.csdn.net/weixin_39198406/article/details/81482082

python爬虫 requests异常requests.exceptions.ConnectionError: HTTPSConnectionPool Max retries exceeded
https://blog.csdn.net/a1007720052/article/details/83383220
'''

import xml.dom.minidom
import requests
import os
import time
import random
from pathlib import Path

def download(url, headers):
	for x in range(5):
		try:
			r = requests.get(url=url, headers=headers, timeout=x*8+4)
			if r.status_code == 200:
				return r
			else:
				continue
		except requests.exceptions.ConnectTimeout:
			print('ConnectTimeout')
			time.sleep(3)
			continue
		except requests.exceptions.ConnectionError:
			print('ConnectionError')
			time.sleep(3)
			continue
		else:
			print('other exception')
			time.sleep(3)
			continue		

start = 12
count = 1

domtree = xml.dom.minidom.parse('1_567.xml')
root = domtree.documentElement
items = root.getElementsByTagName('item')
for item in items:
	id = item.getElementsByTagName('id')[0].childNodes[0].data
	if int(id)<start:
		continue
	topic = item.getElementsByTagName('topic')[0].childNodes[0].data
	baseurl = item.getElementsByTagName('baseurl')[0].childNodes[0].data
	num = item.getElementsByTagName('num')[0].childNodes[0].data
	print('begin', id, topic, num, 'P')
	images = item.getElementsByTagName('images')[0]
	
	# 创建目录
	if not os.path.exists(id):
		os.mkdir(id)
		
	success = True
	for image in images.childNodes:
		imageurl = image.childNodes[0].data
		filename = id + '/' + image.nodeName[2:] + '.jpg'
		file = Path(filename)
		if not file.is_file():
			print('-------------getting ' + filename)
			# 最好指定立即关闭连接，或许连接池会用光，然后就连接失败
			r = download(imageurl, {'referer' : baseurl, 'Connection': 'close'})
			if not r:
				print('download ' + filename + ' error')
				success = False
				break
			else:
				with open(filename, 'wb') as f:
					f.write(r.content)
			time.sleep(random.randint(200,2000)/1000)
		
	# 重命名目录
	if success == True:
		os.rename(id, id + '-' + topic + '【' + num + 'P】')
	
	count = count - 1
	if count<=0:
		break

