from struct import unpack

class Wall:
	TYPES = ['Normal', 'Door', 'Locked']
	def __init__(self, f):
		self.colour, self.type, padding = unpack('<bb2s', f.read(4))
	def dump(self):
		print "%d/%s" % (self.colour, Wall.TYPES[self.type])

class Tile:
	DIRS = ['N', 'E', 'S', 'W']
	def __init__(self, f):
		self.walls = []
		for i in range(4): self.walls.append(Wall(f))
		self.floor, self.ceiling, self.description, self.script, self.etable, padding = unpack('<bbhhh8s', f.read(16))
	def dump(self):
		print "%df %dc %dd %ds %de" % (self.floor, self.ceiling, self.description, self.script, self.etable)
		i = 0
		for d in Tile.DIRS:
			print "    %s:" % d,
			self.walls[i].dump()
			i += 1

class Encounter:
	SIZE = 6
	def __init__(self, f):
		self.monster = unpack('<hhhhhh', f.read(12))
		self.minimum = unpack('<bbbbbb', f.read(6))
		self.maximum = unpack('<bbbbbb', f.read(6))
	def dump(self):
		for i in range(Encounter.SIZE):
			if not self.monster[i]: continue
			print "%d-%d x#%d" % (self.minimum[i], self.maximum[i], self.monster[i])

class Etable:
	SIZE = 6
	def __init__(self, f):
		self.poss = unpack('<b', f.read(1))[0]
		self.encounters = unpack('<hhhhhh', f.read(12))
		self.percentages = unpack('<bbbbbb', f.read(6))
	def dump(self):
		for i in range(Etable.SIZE):
			if not self.percentages[i]: continue
			if i >= self.poss: break
			print "%d%% #%d" % (self.percentages[i], self.encounters[i])

class Zone:
	def __init__(self, f):
		self.tiles = []
		self.strings = []
		self.scripts = []
		self.encounters = []
		self.cstrings = []
		self.etables = []
		self.textures = []
		self.jun, self.version, self.campaign, self.width, self.height, self.nstrings, self.nscripts, self.nencounters, self.ncstrings, self.netables, self.ntextures, padding = unpack('<3sb8sbbhhhhhh6s', f.read(32))
		for i in range(self.width * self.height): self.tiles.append(Tile(f))
		for i in range(self.nstrings): self.strings.append(nstring(f, 1))
		for i in range(self.nscripts): self.scripts.append(nstring(f))
		for i in range(self.nencounters): self.encounters.append(Encounter(f))
		for i in range(self.ncstrings): self.cstrings.append(nstring(f, 1))
		for i in range(self.netables): self.etables.append(Etable(f))
		for i in range(self.ntextures): self.textures.append(nstring(f, 1))
	def dump(self):
		print "%s ZONE v%d in %s.CMP" % (self.jun, self.version, self.campaign.rstrip('\0'))
		print "%dx%d" % (self.width, self.height)
		print "%dst %dsc %den %dcs %det %dtx" % (self.nstrings, self.nscripts, self.nencounters, self.ncstrings, self.netables, self.ntextures)
		#self.dumptiles()
		print self.strings
		print self.cstrings
		print self.textures
		print "[Encounters]"
		for i in range(self.nencounters):
			print "  %d:" % i,
			self.encounters[i].dump()
		print "[Encounter Tables]"
		for i in range(self.netables):
			print "  %d:" % i,
			self.etables[i].dump()
	def dumptiles(self):
		i = 0
		for y in range(self.height):
			for x in range(self.width):
				print "[Tile @%2d,%2d]" % (x, y)
				self.tiles[i].dump()
				i += 1

def nstring(f, x = 0):
	sl = unpack('<h', f.read(2))[0]
	st = f.read(sl + x)
	return st

def zonedump(fn):
	f = open(fn, 'rb')
	z = Zone(f)
	z.dump()

if __name__ == '__main__':
	zonedump('..\\etr_1.zon')
