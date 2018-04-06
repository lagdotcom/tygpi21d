import codecs

def convert_pal_to_binary(jasc, outf):
	o = open(outf, 'wb')
	i = codecs.open(jasc, 'r', 'ascii')
	magic = i.readline().strip()
	if magic != 'JASC-PAL':
		raise Exception('Not a JASC-PAL: ' + magic)
	i.readline()
	ncols = int(i.readline().strip())
	for n in range(ncols):
		rgb = [int(x)>>2 for x in i.readline().strip().split(' ')]
		o.write(bytes(rgb))

convert_pal_to_binary('PALETTE.PAL', 'DOSJUN.PAL')
