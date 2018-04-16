from PIL import Image
from struct import pack
import os

SKIP, JUMP, END, DATA = range(4)
MAXSTRIPLEN = 256 - DATA

class GRFImage:
	def __init__(self, source, spec):
		x, y, self.width, self.height = spec
		self.convert(source.crop((x, y, x+self.width, y+self.height)))
	def convert(self, source):
		d = []
		x = 0
		y = 0
		skipx = 0
		skipy = 0
		zeroes = False
		strip = []
		for c in source.getdata():
			if c:
				if strip: strip.append(c)
				else:
					if zeroes:
						d += self.addskip(skipx, skipy, x, y)
						zeroes = False
					strip = [c]
			else:
				if strip:
					d += self.addstrip(strip)
					strip = []
				if not zeroes:
					skipx = x
					skipy = y
					zeroes = True
			x += 1
			if x == self.width:
				y += 1
				x = 0
		if strip: d += self.addstrip(strip)
		d.append(END)
		self.data = bytes(d)
	def addskip(self, sx, sy, ex, ey):
		data = []
		xd = ex - sx
		yd = ey - sy
		while xd > 255:
			data += [SKIP, 255, 0]
			xd -= 255
		while xd < -255:
			data += [JUMP, 255, 0]
			xd += 255
		if xd < 0: return data + [JUMP, -xd, yd]
		else: return data + [SKIP, xd, yd]
	def addstrip(self, strip):
		data = []
		while len(strip) > MAXSTRIPLEN:
			substrip = strip[:MAXSTRIPLEN]
			strip = strip[MAXSTRIPLEN:]
			data += [255] + substrip
		return data + [len(strip) + END] + strip
	def write(self, f):
		f.write(pack('hhhh',
			self.width,
			self.height,
			len(self.data),
			0))
		f.write(self.data)

class GRF:
	def __init__(self):
		self.images = []
	def add(self, source, spec):
		self.images.append(GRFImage(source, spec))
	def save(self, filename):
		f = open(filename, 'wb')
		f.write(pack('3sbh26s',
			bytes('JUN', 'ascii'),
			1,
			len(self.images),
			bytes('\0'*26, 'ascii')))
		for im in self.images: im.write(f)

def convert_grf(prefix, spec):
	infile = f'{prefix}.pcx'
	outfile = f'{prefix}.GRF'
	try:
		ins, ous = os.stat(infile), os.stat(outfile)
		if ins.st_mtime <= ous.st_mtime:
			return
	except: pass
	print('Reading:', infile)
	i = Image.open(infile)
	g = GRF()
	for img in spec: g.add(i, img)
	print('Writing:', outfile)
	g.save(outfile)

if __name__ == '__main__':
	thing_spec = [(0, 0, 128, 128), (128, 0, 64, 64), (150, 86, 42, 42)]
	convert_grf('..\\THING\\BARREL', thing_spec)
	convert_grf('..\\THING\\SHINY', thing_spec)

	full_spec = [(0, 0, 320, 200)]
	convert_grf('..\\BACK', full_spec)
