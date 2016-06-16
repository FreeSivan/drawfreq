#! /usr/bin/python

__author__ = 'xiwen.yxw'

import os
import sys
import json

def getFileFromNet(newurl, newFile):
    mapp = 'curl';
    cmd  = mapp + ' ' + newurl + ' -o ' + newFile;
    os.system(cmd);

def getCommonNameFromUrl(urlTxt, suffixName):
    cm = 'test';
    s = urlTxt.rfind('/');
    e = urlTxt.rfind(suffixName);

    if s < e:
        cm = urlTxt[s + 1: e-1];

    e = cm.find('_');
    if e != -1:
        cm = cm[0 : e];

    return cm;

def getSuffixNameFromUrl(urlTxt):
    cm = 'test'
    if urlTxt.find("m4a") >= 0:
        cm = "m4a"
    elif urlTxt.find("ape") >= 0:
        cm = "ape"
    elif urlTxt.find("flac") >= 0:
        cm = "flac"
    elif urlTxt.find("w4a") >= 0:
        cm = "w4a"
    elif urlTxt.find("wma") >= 0:
        cm = "wma"
    elif urlTxt.find("wav") >= 0:
        cm = "wav"
    elif urlTxt.find("mp3") >= 0:
        cm = "mp3"
    return cm

def getUrlfromjson(lst):
    min = 65535
    index = -1
    if lst == None:
        return ""
    for m in range(len(lst)) :
        if lst[m]["rate"] == 128:
            min = 128
            index = m
        elif lst[m]["rate"] == 192:
            if min != 128:
                min = 192
                index = m
        elif lst[m]["rate"] == 320:
            if min != 128 and min != 192:
                min = 320
                index = m
        elif lst[m]["rate"] >= 32:
            if min != 128 and min != 192 and min != 320:
                min = 32
                index = m
    if index != -1:
        return lst[index]["path"]
    return ""

def main():
    tmpUrl = "http://10.218.143.55:7001/resource/song/"+sys.argv[1]+"/audios"
    tmpJson = 'curl' + ' ' + tmpUrl
    tmpResult = os.popen(tmpJson).read()
    dct = json.loads(tmpResult)
    url = getUrlfromjson(dct["resultObj"])
    if "" == url:
        return
    suffixName = getSuffixNameFromUrl(url)
    commonName = getCommonNameFromUrl(url, suffixName)
    srcFile = commonName + "." + suffixName
    getFileFromNet(url, srcFile)

if __name__ == "__main__":
    main();

