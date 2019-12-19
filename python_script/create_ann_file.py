import os
import sys

n_master_var = 167

instance = sys.argv[1]

file = open(instance + "annotation.ann", 'w')

file.write("<?xml version='1.0' encoding='utf-8'?>\n")
file.write("<CPLEXAnnotations>\n")
file.write("<CPLEXAnnotation name='cpxBendersPartition' type='long' default='1'>\n")
file.write("<object type='1'>\n")

for i in range(n_master_var) :
	file.write("<anno name='C%i' index='%i' value='0'/>\n" % (i,i))
file.write("</object>\n")
file.write("</CPLEXAnnotation>\n")
file.write("</CPLEXAnnotations>")
file.close()
