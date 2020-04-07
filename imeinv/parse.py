# -*- coding: utf-8 -*-

import xml.dom.minidom

domtree = xml.dom.minidom.parse('1_567.xml')
root = domtree.documentElement
items = root.getElementsByTagName('item')
start = 3
count = 1
for item in items:
    id = item.getElementsByTagName('id')[0].childNodes[0].data
    if int(id)<start:
        continue
    topic = item.getElementsByTagName('topic')[0].childNodes[0].data
    baseurl = item.getElementsByTagName('baseurl')[0].childNodes[0].data
    print(id, topic, baseurl)
    images = item.getElementsByTagName('images')[0]
    for image in images.childNodes:
        imageurl = image.childNodes[0].data
        print(image.nodeName[2:], imageurl)
    count = count - 1
    if count<=0:
        break