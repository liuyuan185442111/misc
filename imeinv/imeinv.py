# -*- coding: utf-8 -*-
import scrapy
import re

# 将ROBOTSTXT_OBEY改为False

class BeautyItem(scrapy.Item):
	id = scrapy.Field()
	baseurl = scrapy.Field()
	topic = scrapy.Field()
	tags = scrapy.Field()
	intro = scrapy.Field()
	num = scrapy.Field()
	images = scrapy.Field()

from beauty.items import BeautyItem

class AimeinvSpider(scrapy.Spider):
	name = 'imeinv'
	allowed_domains = ['2meinv.com']
	start_urls = ['https://www.2meinv.com/article-63.html']

	images = {}
	endid = '9999'

	def parse(self, response):
		curnum = response.xpath("//h1/span").re("[0-9]+")[0]
		maxnum = response.xpath("//h1/span").re("[0-9]+")[1]
		if curnum == '1':
			# 初始化
			self.baseurl = response.url
			self.id = re.match('.*-([0-9]+)\.html',response.url).group(1)
			self.topic = response.xpath("//h1/text()").extract()[0]
			self.tags = ','.join(response.xpath("/html/body/div[2]/div/div/span/a/text()").extract())
			self.intro = ''.join([str(i) for i in response.xpath("/html/body/div[2]/div/p/text()").extract()[1:]])
		self.images['No'+curnum] = response.xpath("//div[@class='pp hh']/a/img/@src").extract()[0]
		if curnum != maxnum:
			# 读取下一页
			post_list = response.xpath("//div[@class='page-show']/a[contains(text(),'下一页')]/@href").extract()
			yield scrapy.Request(post_list[0], callback=self.parse)
		else:
			# 本篇完成 读取上一篇
			item = BeautyItem(id=self.id,baseurl=self.baseurl,topic=self.topic,tags=self.tags,intro=self.intro,num=maxnum,images=self.images)
			yield item
			pre_list = response.xpath("//div[@class='page-show']/a[contains(text(),'上一篇')]/@href").extract()
			if len(pre_list) == 0:
				return
			else:
				nexturl = pre_list[0]
				nextid = re.match('.*-([0-9]+)\.html',nexturl).group(1)
				if nextid == self.endid:
					return
				else:
					self.images = {}
					yield scrapy.Request(nexturl, callback=self.parse)
					pass
				pass
			pass
		pass
