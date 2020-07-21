#!/usr/bin/python

import sys
import os
import getopt
import codecs
import pickle
from xml.dom import minidom
from xml.dom.minidom import Document

transdict = {}

def ImportDict(filepath):
    print "Importing", filepath, "..."
    if (os.path.splitext(filepath)[1].upper() == ".AD"):
        adfile = codecs.open(filepath, "r", "utf-8")
        adstring = adfile.read();
        addoc = minidom.parseString(adstring.encode("utf-8"))
        baselist = addoc.getElementsByTagName("base")
        for base in baselist:
            if len(base.childNodes) > 0:
                try:
                    basetxt = base.firstChild.nodeValue
                    trannode = base.parentNode.getElementsByTagName("tran")
                    if len(trannode[0].childNodes) > 0:
                        transdict[basetxt] = trannode[0].firstChild.nodeValue
                except:
                    print "Except when extracting node:", base.firstChild.nodeValue, u"\r\n"
        adfile.close()
    else:
        txtfile = codecs.open(filepath, "r", "utf-8")
        
        # construct dictionnary
        txtdata = txtfile.read()
        if len(txtdata) > 2 and txtdata[0] == u"\ufeff":
        	txtdata = txtdata[1:]
        txtpairs = txtdata.split("\r\n\r\n")

        for pair in txtpairs:
            item = pair.splitlines()
            if len(item) > 1 and len(item[1]) > 0:
                transdict[item[0]] = item[1]
                if len(item[0].strip()) != len(item[0]):	# base text with leading or padding space
                	transdict[item[0].strip()] = item[1].strip()


def Extract(wgfilepathlist, txtfilepath):
    logfile = codecs.open(os.path.splitext(txtfilepath)[0] + ".log", "a")
    stringlist = []
    for wgfilepath in wgfilepathlist:
        wgfile = codecs.open(wgfilepath, "r", "utf-8")
        wgstring = wgfile.read()
        wgdoc = minidom.parseString(wgstring.encode("utf-8"))
        baselist = wgdoc.getElementsByTagName("base")
        for base in baselist:
            if len(base.childNodes) > 0:
                try:
                    basetxt = base.firstChild.nodeValue
                    if not transdict.has_key(basetxt):
                        if basetxt not in stringlist:
                            stringlist.append(basetxt)
                except:
                    logfile.write("Except when extracting node: " + base.firstChild.nodeValue + u"\r\n")
        wgfile.close()
    stringlist.sort()
    txtfile = codecs.open(txtfilepath, "w", "utf-8")
    for s in stringlist:
        txtfile.write(s)
        txtfile.write(u"\r\n\r\n\r\n")
    txtfile.close()
    logfile.close()

def Integrate(wgfilepath, outputfilepath):
    logfile = codecs.open(os.path.splitext(wgfilepath)[0] + ".log", "a")

    wgfile = codecs.open(wgfilepath, "r", "utf-8")
    wgstring = wgfile.read()
    wgfile.close()
    wgdoc = minidom.parseString(wgstring.encode("utf-8"))
    baselist = wgdoc.getElementsByTagName("base")
    
    for base in baselist:
        if len(base.childNodes) > 0:
            try:
                trannode = base.parentNode.getElementsByTagName("tran")[0]
                textnode = wgdoc.createTextNode(transdict[base.firstChild.nodeValue])
                if len(trannode.childNodes) > 0:
                    trannode.replaceChild(textnode, trannode.lastChild)
                else:
                    trannode.appendChild(textnode)
            except:
                logfile.write("No translation for: \"" + base.firstChild.nodeValue.encode('utf8') + "\"\r\n")
                #logfile.write("No translation for: \r\n")
    outputfile = open(outputfilepath, "w")
    outputfile.write(wgdoc.toxml("utf-8"))
    
    wgfile.close()
    logfile.close()
    outputfile.close()


try:
    options, arguments = getopt.getopt(sys.argv[1:], "h", ["help"])
except getopt.error, msg:
    print msg
    print "for help: -h"
    sys.exit(2)

dictfilepath = ""
command = ""

for opt,attr in options:
    if opt in ("-h", "--help"):
        print   """
Extract/integrate translation strings from/to .wg file
Usage:
    Import translated strings into dictionary (text file or .ad file):
        wgtrans <dictionary> import <translated file> <.ad file>...
    Translate a .wg file:
        wgtrans <dictionary> translate <.wg file> <.wg file>...
    Extract strings to be translated from a .wg file:
        wgtrans <dictionary> extract <txt file> <.wg file> <.wg file>...
    Print out dictionary:
        wgtrans <dictionary> print [<txt file>]
                 """
        sys.exit(2)
        
if len(arguments) > 1:
    dictfilepath = arguments[0]
    command = arguments[1]
else:
    print "Arguments expected: -h for help"
    sys.exit(2)

if os.path.exists(dictfilepath):
    dictfile = open(dictfilepath, "r")
    transdict = pickle.load(dictfile)
    morewords = {}
    for base,tran in transdict.items():
    	if base.startswith('?#x0000;') and base.endswith('?#x0000;'):
    		if tran.startswith('?#x0000;'):
	    		morewords[base[8:-8]] = tran[8:-8]
	    	else:
	    		index = tran.find('?#x0000;');
	    		if index != -1:
	    			morewords[base[8:-8]] = tran[0:index]
    for base,tran in morewords.items():
    	if not transdict.has_key(base):
    		transdict[base] = tran
    dictfile.close()

if command == "import":
    if len(arguments) > 2:
        for d in arguments[2:]:
            ImportDict(d)
        dictfile = open(dictfilepath, "wb")
        pickle.dump(transdict, dictfile)
        dictfile.close()
    else:
        print "missing argument for import"
elif command == "extract":
    if len(arguments) > 3:
        if os.path.splitext(arguments[2])[1].lower() != ".ad":
            Extract(arguments[3:], arguments[2])
        else:
            print "first argument for extract should be output file"
    else:
        print "missing argument for extract"
elif command == "translate":
    if len(arguments) > 2:
        for wgfilepath in arguments[2:]:
            #outputfilepath = os.path.splitext(wgfilepath)[0] + "_tran.wg"
            Integrate(wgfilepath, wgfilepath)
    else:
        print "missing argument for integrate"
elif command == "print":
    if len(arguments) > 2:
        outfile = codecs.open(arguments[2], "w", "utf-8")
    for k,v in transdict.iteritems():
        if len(arguments) > 2:
            outfile.write(u"<phrase>\n    <source>" + k + u"</source>\n    <target>" + v + u"</target>\n</phrase>\n")
        else:
            print k, "\n", v, "\n\n"
else:
	print "Command not recognized:", command
