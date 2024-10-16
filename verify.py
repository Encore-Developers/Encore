import json
import os
import sys

hasError = False

ids = []

icons = []
usedIcons = []
enNames = []

def CI(message, isError = False):
	global hasError
	if isError == True:
		hasError = True
	if(os.getenv("CI") == "true"):
		print(message)

def scanIcons(parent):
	global icons
	
	path = os.path.join(parent, "icons")
	count = 0
	for file in os.listdir(path):
		name = os.path.splitext(os.path.basename(file))[0]
		if name in icons:
			print(f"ERROR: Found duplicate icon `{name}` in `{parent}`")
			CI(f"::error file={parent}/icons/{name}.png::[{parent}] Found duplicate icon `{name}` in `{parent}`", True)
		else:
			icons.append(name)
			count += 1
	
	print(f"Found {count} icons in `{parent}`")

def scan(parent):
	global ids

	index = open(os.path.join(parent, "index.json"))
	data = json.load(index)
	
	for source in data["sources"]:
		# Look for duplicate ids
		for id in source["ids"]:
			if id in ids:
				sourceName = source["names"]["en-US"]
				print(f"ERROR: Duplicate id `{id}` in `{sourceName}`")
				CI(f"::error file={parent}/index.json::[{parent}] Duplicate id `{id}` in `{sourceName}`", True)
			else:
				ids.append(id)
		
		# Look for a duplicate name
		name = source["names"]["en-US"]
		if name in enNames:
			print(f"WARN: Duplicate name `{name}` can be consolidated")
			CI(f"::warning file={parent}/index.json::[{parent}] Duplicate name `{name}` can be consolidated")
		else:
			enNames.append(name)

		# See if icon exists
		icon = source["icon"]
		if not icon in icons:
			print(f"ERROR: Icon `{icon}` does not exist")
			CI(f"::error file={parent}/index.json::[{parent}] Icon `{icon}` does not exist", True)

		elif not icon in usedIcons:
			usedIcons.append(icon)
	
	# Check for extra icons
	unused = list(set(icons).difference(usedIcons))
	if len(unused) != 0:
		print(f"WARNING: {len(unused)} unused icons")
		for i in unused:
			print(f" - {i}")
			CI(f"::warning file={parent}/icons/{i}.png::[{parent}] Unused icon ({parent}/icons/{i}.png)")

	print(f"Done scanning `{parent}`")

scanIcons("base")
scan("base")

scanIcons("extra")
scan("extra")

if hasError == True:
	sys.exit(1)