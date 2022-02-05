#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2009-2022 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    osmGet.py
# @author  Daniel Krajzewicz
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2009-08-01

from __future__ import absolute_import
from __future__ import print_function
import os
import gzip
import base64
import ssl
import json

try:
    import httplib
    import urlparse
    from urllib2 import urlopen
except ImportError:
    # python3
    import http.client as httplib
    import urllib.parse as urlparse
    from urllib.request import urlopen
try:
    import certifi
    HAVE_CERTIFI = True
except ImportError:
    HAVE_CERTIFI = False

import sumolib

def readBuildingShapeKeysFromXML(keyValueDict, pathToXML):
    with open(pathToXML, 'r') as osmPolyconvert:
        kvd = keyValueDict
        for polygon in sumolib.xml.parse(osmPolyconvert, 'polygonType'):
            keyValue = polygon.id.split('.')
            try:
                key = keyValue[0]
                value = keyValue[1]                             # key without value throws IndexError
                kvd[key] = list(set(kvd[key] + [value]))        # kyd[key] could throw KeyError
            except KeyError:
                kvd[key] = [value]
            except IndexError:
                try:
                    kvd[key] = list(set(kvd[key] + ['.']))      # kyd[key] could throw KeyError
                except KeyError:
                    kvd[key] = ['.']
    return kvd

def readCompressed(conn, urlpath, query, roadTypesJSON, downloadShapesPolygon,filename):
    # generate query string for each road-type category
    commonQueryStringKeyPart = "<has-kv k=\"%s\" "
    commonQueryStringRegvPart = "modv=\"\" regv=\"%s\"/>"
    commonQueryStringOnlyKey = "/>"
    queryStringNode = []

    commonQueryStringNode = """
    <query type="nwr">
            %s
            %s
        </query>"""

    for category in roadTypesJSON:
        keyQueryString = commonQueryStringKeyPart % category
        typeList = []
        if len(roadTypesJSON[category])>0:
            for type in roadTypesJSON[category]:
                typeList.append(type)
            separator = "|"
            typeListStringPerCategory = separator.join(typeList)
            regvQueryString = commonQueryStringRegvPart % typeListStringPerCategory
            finalQueryStringPerCategory = keyQueryString + regvQueryString
            queryStringNode.append(commonQueryStringNode % (finalQueryStringPerCategory, query))

    if downloadShapesPolygon == 'True':
        keyValueDict = {}
        keyValueDict = readBuildingShapeKeysFromXML(keyValueDict, "../data/typemap/osmPolyconvert.typ.xml")
        keyValueDict = readBuildingShapeKeysFromXML(keyValueDict, "../data/typemap/osmPolyconvertRail.typ.xml")
        for category, value in keyValueDict.items():
            if category in roadTypesJSON:
                continue
            valuePerCategory = []
            keyQueryString = commonQueryStringKeyPart % category
            if ('.' in value):
                finalQueryStringPerCategory = keyQueryString + commonQueryStringOnlyKey
            else:
                for val in value:
                    valuePerCategory.append(val)
                separator = "|"
                valueStringPerCategory = separator.join(valuePerCategory)
                regvQueryString = commonQueryStringRegvPart % valueStringPerCategory
                finalQueryStringPerCategory = keyQueryString + regvQueryString

            queryStringNode.append(commonQueryStringNode % (finalQueryStringPerCategory, query))

    separator = "\n"
    unionQueryString = separator.join(queryStringNode)

    conn.request("POST", "/" + urlpath, """
    <osm-script timeout="240" element-limit="1073741824">
    <union>
       %s                            
    </union>
    <union>
       <item/>
       <recurse type="way-node"/>
    </union>
    <print mode="body"/>
    </osm-script>""" % unionQueryString, headers={'Accept-Encoding': 'gzip'})

    response = conn.getresponse()
    print(response.status, response.reason)
    if response.status == 200:
        with open(filename, "wb") as out:
            if filename.endswith(".gz"):
                out.write(response.read())
            else:
                out.write(gzip.decompress(response.read()))

optParser = sumolib.options.ArgumentParser(description="Get network from OpenStreetMap")
optParser.add_argument("-p", "--prefix", default="osm", help="for output file")
optParser.add_argument("-b", "--bbox", help="bounding box to retrieve in geo coordinates west,south,east,north")
optParser.add_argument("-t", "--tiles", type=int,
                       default=1, help="number of tiles the output gets split into")
optParser.add_argument("-d", "--output-dir", help="optional output directory (must already exist)")
optParser.add_argument("-a", "--area", type=int, help="area id to retrieve")
optParser.add_argument("-x", "--polygon", help="calculate bounding box from polygon data in file")
optParser.add_argument("-u", "--url", default="www.overpass-api.de/api/interpreter",
                       help="Download from the given OpenStreetMap server")
# alternatives: overpass.kumi.systems/api/interpreter, sumo.dlr.de/osm/api/interpreter
optParser.add_argument("-w", "--wikidata", action="store_true",
                       default=False, help="get the corresponding wikidata")
optParser.add_argument("-r", "--road_types", help="only delivers osm data to the specified road-types")
optParser.add_argument("-s", "--shapes_polygon", help="determines if polygon data (buildings, areas , etc.) is downloaded")
optParser.add_argument("-z", "--gzip", action="store_true",
                       default=False, help="save gzipped output")

def get(args=None):
    options = optParser.parse_args(args=args)
    if not options.bbox and not options.area and not options.polygon:
        optParser.error("At least one of 'bbox' and 'area' and 'polygon' has to be set.")
    if options.polygon:
        west = 1e400
        south = 1e400
        east = -1e400
        north = -1e400
        for area in sumolib.output.parse_fast(options.polygon, 'poly', ['shape']):
            for coord in area.shape.split():
                point = tuple(map(float, coord.split(',')))
                west = min(point[0], west)
                south = min(point[1], south)
                east = max(point[0], east)
                north = max(point[1], north)
    if options.bbox:
        west, south, east, north = [float(v) for v in options.bbox.split(',')]
        if south > north or west > east:
            optParser.error("Invalid geocoordinates in bbox.")

    if options.output_dir:
        options.prefix = os.path.join(options.output_dir, options.prefix)

    if "http" in options.url:
        url = urlparse.urlparse(options.url)
    else:
        url = urlparse.urlparse("https://" + options.url)
    if os.environ.get("https_proxy") is not None:
        headers = {}
        proxy_url = urlparse.urlparse(os.environ.get("https_proxy"))
        if proxy_url.username and proxy_url.password:
            auth = '%s:%s' % (proxy_url.username, proxy_url.password)
            headers['Proxy-Authorization'] = 'Basic ' + base64.b64encode(auth)
        conn = httplib.HTTPSConnection(proxy_url.hostname, proxy_url.port)
        conn.set_tunnel(url.hostname, 443, headers)
    else:
        if url.scheme == "https":
            if HAVE_CERTIFI:
                context = ssl.create_default_context(cafile=certifi.where())
                conn = httplib.HTTPSConnection(url.hostname, url.port, context=context)
            else:
                conn = httplib.HTTPSConnection(url.hostname, url.port)
        else:
            conn = httplib.HTTPConnection(url.hostname, url.port)

    # Initially "Add Polygon" Checkbox is True
    downloadShapesPolygon = True
    if options.shapes_polygon:
        downloadShapesPolygon = options.shapes_polygon

    if options.road_types:
        roadTypesJSON = json.loads(options.road_types.replace("\'", "\"").lower())

    suffix = ".osm.xml.gz" if options.gzip else ".osm.xml"
    if (options.area and options.road_types):
        if options.area < 3600000000:
            options.area += 3600000000
        readCompressed(conn, url.path, '<area-query ref="%s"/>' %
                       options.area, roadTypesJSON, downloadShapesPolygon, options.prefix + "_city" + suffix)
    if ((options.bbox or options.polygon) and options.road_types):
        if options.tiles == 1:
            readCompressed(conn, url.path, '<bbox-query n="%s" s="%s" w="%s" e="%s"/>' %
                           (north, south, west, east), roadTypesJSON, downloadShapesPolygon,
                           options.prefix + "_bbox" + suffix)
        else:
            num = options.tiles
            b = west
            for i in range(num):
                e = b + (east - west) / float(num)
                readCompressed(conn, url.path, '<bbox-query n="%s" s="%s" w="%s" e="%s"/>' % (
                    north, south, b, e), roadTypesJSON, downloadShapesPolygon,
                               "%s%s_%s%s" % (options.prefix, i, num, suffix))
                b = e

    conn.close()
    # extract the wiki data according to the wikidata-value in the extracted osm file
    if options.wikidata:
        filename = options.prefix + '.wikidata.xml.gz'
        osmFile = options.prefix + "_bbox" + suffix
        codeSet = set()
        # deal with invalid characters
        bad_chars = [';', ':', '!', "*", ')', '(', '-', '_', '%', '&', '/', '=', '?', '$', '//', '\\', '#', '<', '>']
        for line in sumolib.xml._open(osmFile, encoding='utf8'):
            subSet = set()
            if 'wikidata' in line and line.split('"')[3][0] == 'Q':
                basicData = line.split('"')[3]
                for i in bad_chars:
                    basicData = basicData.replace(i, ' ')
                elems = basicData.split(' ')
                for e in elems:
                    if e and e[0] == 'Q':
                        subSet.add(e)
                codeSet.update(subSet)

        # make and save query results iteratively
        codeList = list(codeSet)
        interval = 50  # the maximal number of query items
        outf = gzip.open(filename, "wb")
        for i in range(0, len(codeSet), interval):
            j = i + interval
            if j > len(codeSet):
                j = len(codeSet)
            subList = codeList[i:j]
            content = urlopen("https://www.wikidata.org/w/api.php?action=wbgetentities&ids=%s&format=json" %
                              ("|".join(subList))).read()
            print(type(content))
            outf.write(content + b"\n")
        outf.close()


if __name__ == "__main__":
    get()
