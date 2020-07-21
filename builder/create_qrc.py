#!/usr/bin/python
# -*- coding: utf-8 -*-
import glob
import os
import xml.dom.minidom as Dom 

def create_qrc_fun(projects_dir,project_name):
	images_dir = os.path.join(projects_dir, "images")
	widgets_dir = os.path.join(projects_dir, "widgets")
	qml_dir = os.path.join(projects_dir, "qml")
	qrc_file = os.path.join(projects_dir, project_name + ".qrc")
	if os.path.exists(images_dir) or os.path.exists(widgets_dir) or os.path.exists(qml_dir): 
		doc = Dom.Document()
		root_node = doc.createElement("RCC")
		doc.appendChild(root_node)
		qrc_node = doc.createElement("qresource")
		qrc_node.setAttribute("prefix", "/")
		res_dirs = [images_dir, widgets_dir, qml_dir]
		for res_dir in res_dirs:
			for rt, dirs, files in os.walk(res_dir):
				for file in files:
					file_node = doc.createElement("file")
					res_file = os.path.join(rt,file)
					res_file = res_file.replace(projects_dir + os.path.sep, '')
					file_text = doc.createTextNode(res_file)
					file_node.appendChild(file_text)
					qrc_node.appendChild(file_node)
		root_node.appendChild(qrc_node)
		f = open(qrc_file, "w")
		std = doc.toprettyxml(indent = "\t", newl = "\n", encoding = "utf-8").decode("utf-8")
		f.write(std)  
		f.close()
		print ('create %s qrc done.' % project_name)

tools_path = os.path.split(os.path.realpath(__file__))[0]
src_path = os.path.abspath(os.path.join(tools_path, "../"))
modules=os.listdir(src_path)
print ('creating qrc ...')
for module in modules:
	m_d = os.path.join(src_path,module) 
	if os.path.isdir(m_d):
		for pro in os.listdir(m_d):
			pro_d = os.path.join(m_d, pro)
			if os.path.isdir(pro_d):
				create_qrc_fun(pro_d,pro)
print ('create qrc done.')