# -*- coding: utf-8 -*-

import os
import sys
import time
import random
import requests
import xml.dom.minidom
from pathlib import Path

if len(sys.argv) == 1:
	print('usage:', sys.argv[0], 'startid')
	sys.exit(1)

start = int(sys.argv[1])
count = 1000
xmlfile ='2000.xml'

def nowtime():
	return time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())

def download(url, headers):
	for x in range(5):
		try:
			r = requests.get(url=url, headers=headers, timeout=x*8+4)
			if r.status_code == 200:
				return r
			else:
				time.sleep(3)
				continue
		except requests.exceptions.ConnectionError as e:
			print(e)
			time.sleep(3)
			continue
		except requests.exceptions.ConnectTimeout as e:
			print(e)
			time.sleep(3)
			continue
		except requests.exceptions.ReadTimeout as e:
			print(e)
			time.sleep(3)
			continue
		except requests.exceptions.RequestException as e:
			print(type(e))
			print(e)
			break

domtree = xml.dom.minidom.parse(xmlfile)
root = domtree.documentElement
items = root.getElementsByTagName('item')
for item in items:
	id = item.getElementsByTagName('id')[0].childNodes[0].data
	if int(id) < start:
		continue
	topic = item.getElementsByTagName('topic')[0].childNodes[0].data
	baseurl = item.getElementsByTagName('baseurl')[0].childNodes[0].data
	num = item.getElementsByTagName('num')[0].childNodes[0].data
	print(nowtime(), 'statistics: begin', id, topic, num, 'P')
	images = item.getElementsByTagName('images')[0]
	time1 = time.time()
	
	# 创建目录
	if not os.path.exists(id):
		os.mkdir(id)
		
	success = True
	for image in images.childNodes:
		imageurl = image.childNodes[0].data
		filename = id + '/' + image.nodeName[2:] + '.jpg'
		file = Path(filename)
		if not file.is_file():
			print('-------------getting', filename)
			# 立即关闭连接，否则或许连接池会用光
			r = download(imageurl, {'referer' : baseurl, 'Connection': 'close'})
			if not r:
				print(nowtime(), 'error: when downloading', filename)
				success = False
				sys.exit(2)
				break
			else:
				with open(filename, 'wb') as f:
					f.write(r.content)
			time.sleep(random.randint(200,2000)/1000)
			pass
		
	# 重命名目录
	if success == True:
		os.rename(id, id + '-' + topic + '【' + num + 'P】')
	
	time2 = time.time()
	print(nowtime(), 'statistics: end', id, 'using', int(time2-time1+0.5), 'secondes')
	
	count = count - 1
	if count <= 0:
		break
	pass