from PIL import Image
from struct import pack

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
		xd = ex - sx
		yd = ey - sy
		if abs(xd) > 255: raise Exception(f'Skip X too high: {xd}')
		if xd < 0: return [JUMP, -xd, yd]
		return [SKIP, xd, yd]
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
	print('Reading:', infile)
	i = Image.open(infile)
	g = GRF()
	for img in spec: g.add(i, img)
	print('Writing:', outfile)
	g.save(outfile)

if __name__ == '__main__':
	thing_spec = [(0, 0, 128, 128), (128, 0, 64, 64), (150, 86, 42, 42)]

	convert_grf('THING\\BARREL', thing_spec)
	convert_grf('THING\\SHINY', thing_spec)
