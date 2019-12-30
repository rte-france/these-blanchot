import os
import sys

n_master_var = 0

instance = sys.argv[1]

# Recuperation du nombre de variables du probleme maitre dans le fichier info.txt
file = open(instance + "infos.txt", 'r')
name = "NMasterVars"
for line in file :
	if line.split()[0] == name :
		n_master_var = int(line.split()[1])

print("Taille master : " , n_master_var)

# Ecriture du fichier annotations
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
