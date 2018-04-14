from PIL import Image
from decimal import Decimal
from struct import unpack

class Vec:
	def __init__(self, args):
		self.components = [Decimal(n) for n in args]
		self.order = len(self.components)
	def __add__(self, v):
		return Vec([self[i] + v[i] for i in range(self.order)])
	def __sub__(self, v):
		return Vec([self[i] - v[i] for i in range(self.order)])
	def __mul__(self, v):
		return Vec([v*n for n in self.components])
	def __getitem__(self, i):
		return self.components[i]
	def __repr__(self):
		return 'V(' + ','.join([str(n) for n in self.components]) + ')'
	def int(self):
		return [round(n) for n in self.components]

def avgc(colours):
	if not colours: return (0,0,0)
	l = len(colours)
	r, g, b, = 0, 0, 0
	for c in colours:
		r += c[0]
		g += c[1]
		b += c[2]
	return int(r/l), int(g/l), int(b/l)

def shear_to_fit(src, a, b, c, d):
	img = src.convert('RGB')
	sh, sw = img.size
	# only work with square images
	if sh > sw: zl = sw
	else: zl = sh
	# make new image
	dw = max(a[0], b[0], c[0], d[0]) + 1
	dh = max(a[1], b[1], c[1], d[1]) + 1
	dst = Image.new('RGB', (dw, dh))
	dp = []
	for y in range(dh):
		row = []
		for x in range(dw): row.append([])
		dp.append(row)
	# map old pixels to new pixels!
	va, vb, vc, vd = Vec(a), Vec(b), Vec(c), Vec(d)
	def interp(a, b, p): return (b - a) * p
	def mapxy(x, y):
		xp = Decimal(x) / zl
		yp = Decimal(y) / zl
		yq = 1 - yp
		vab = va + (vb - va) * xp
		vcd = vc + (vd - vc) * xp
		vm = vab * yq + vcd * yp
		dx, dy = vm.int()
		#print(f'{x},{y} => {vab} -> {vcd} => {vm}')
		return dx, dy
	abort = False
	for y in range(zl):
		for x in range(zl):
			sc = img.getpixel((x, y))
			dx, dy = mapxy(x, y)
			try:
				dp[dy][dx].append(sc)
			except:
				print(f'Could not write to {dx},{dy}')
				abort = True
				break
		if abort: break
	for y in range(dh):
		for x in range(dw):
			dst.putpixel((x, y), avgc(dp[y][x]))
	return dst

FLATPALETTE = []
Colours = []
def loadpalette(fn):
	global FLATPALETTE, Colours
	f = open(fn, 'rb')
	for i in range(256):
		r, g, b = unpack('BBB', f.read(3))
		r *= 4
		g *= 4
		b *= 4
		FLATPALETTE += [r, g, b]
		Colours.append([r, g, b])

ResolveCache = {}
def resolve(c):
	global ResolveCache
	ck = ','.join([str(n) for n in c])
	if ck in ResolveCache: return ResolveCache[ck]
	tr, tg, tb = c
	best = 99999999
	besti = None
	for i in range(256):
		h = Colours[i]
		dr = (tr - h[0]) ** 2
		dg = (tg - h[1]) ** 2
		db = (tb - h[2]) ** 2
		score = dr + dg + db
		if score < best:
			best, besti = score, i
	ResolveCache[ck] = besti
	return besti

def palettize(img):
	print('... palettizing')
	cpy = Image.new('P', img.size)
	for y in range(img.size[1]):
		for x in range(img.size[0]):
			c = img.getpixel((x, y))
			cpy.putpixel((x, y), resolve(c))
	cpy.putpalette(FLATPALETTE)
	return cpy

def shear_one(src, dst):
	print('Producting sheet one for', src)
	i = Image.open(src)
	o = Image.new('RGB', (128, 192))
	print('... rendering top')
	pt = shear_to_fit(i, (1,  0), (126, 0), (32, 31), (95,  31))
	print('... rendering left')
	pl = shear_to_fit(i, (0,  0), (31, 31), (0, 127), (31,  96))
	print('... rendering right')
	pr = shear_to_fit(i, (0, 31), (31,  0), (0,  96), (31, 127))
	print('... rendering bottom')
	pb = shear_to_fit(i, (32, 0), (95,  0), (1,  31), (126, 31))
	print('... rendering middle')
	pm = shear_to_fit(i, (0,  0), (63,  0), (0,  63), (63,  63))
	o.paste(pt, (0, 0))
	o.paste(pl, (0, 32))
	o.paste(pr, (96, 32))
	o.paste(pb, (0, 160))
	o.paste(pm, (32, 64))
	o = palettize(o)
	print('... saving to', dst)
	o.save(dst)

def shear_two(src, dst):
	print('Producting sheet two for', src)
	i = Image.open(src)
	o = Image.new('RGB', (64, 128))
	print('... rendering left')
	ml = shear_to_fit(i, (32, 0), (63, 0), (0, 31), (32, 31))
	print('... rendering right')
	mr = shear_to_fit(i, (0,  0), (31, 0), (31, 31), (63, 31))
	print('... rendering middle')
	md = shear_to_fit(i, (0,  0), (63,  0), (0,  63), (63,  63))
	o.paste(ml.crop(( 0, 0, 32, 32)), (32,  0))
	o.paste(mr.crop((32, 0, 64, 64)), ( 0,  0))
	o.paste(ml.crop((32, 0, 64, 32)), ( 0, 96))
	o.paste(mr.crop(( 0, 0, 32, 32)), (32, 96))
	o.paste(md.crop((32, 0, 64, 64)), ( 0, 32))
	o.paste(md.crop(( 0, 0, 32, 64)), (32, 32))
	o = palettize(o)
	print('... saving to', dst)
	o.save(dst)

def shear_all(src, dst):
	shear_one(src, dst+'1.png')
	shear_two(src, dst+'2.png')

if __name__ == '__main__':
	loadpalette('..\\DOSJUN.PAL')
	shear_all('source.jpg', 'dest')
