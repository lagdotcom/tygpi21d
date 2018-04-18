import glob, os

class JascPal:
	def __init__(self, fn):
		f = open(fn, 'r')
		if f.readline() != 'JASC-PAL\n': raise Exception('Not a JASC-PAL')
		f.readline() # 0100
		self.size = int(f.readline().strip())
		self.data = ''
		for i in range(self.size):
			c = f.readline().strip()
			for co in c.split(' '):
				self.data += chr(int(co))
		print 'Loaded %d bytes from %s' % (len(self.data), fn)

def replacePcxPalette(fn, pal):
	f = open(fn, 'r+b')
	f.seek(-len(pal.data), 2)
	f.write(pal.data)
	print 'Processed:', fn

def getFilenames(*globs):
	fns = []
	for g in globs:
		fns += glob.glob(g)
	return fns

if __name__ == '__main__':
	pal = JascPal('..\\palette.pal')
	for fn in getFilenames('..\\*.PCX', '..\\PICS\\*.PCX', '..\\THING\\*.PCX', '..\\WALL\\*.PCX'):
		replacePcxPalette(fn, pal)
