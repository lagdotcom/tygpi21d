
def notename(nn):
	o, n = divmod(nn, 12)
	return ['C', 'C#', 'D', 'Eb', 'E', 'F', 'F#', 'G', 'Ab', 'A', 'Bb', 'B'][n] + str(o)

def generateFrequencies(A4):
	freqs = []
	twelth = pow(2, 1.0 / 12)
	nn = 57

	dn = nn
	df = A4
	while dn > 0:
		dn -= 1
		df /= twelth
		freqs.insert(0, (notename(dn), df))

	up = nn
	uf = A4
	freqs.append(('A4', A4))
	while up < 94:
		up += 1
		uf *= twelth
		freqs.append((notename(up), uf))
	return freqs

def freqNumber(freq, block):
	return freq * pow(2, 20 - block) / 49716

def discover(freq):
	for block in range(8):
		fn = freqNumber(freq, block)
		if fn < 1024:
			return fn, block

HEADER = '''
#define _BLOCK(n) ((n) << 10)

noexport UINT16 note_frequencies[] = {
'''

FOOTER = '''};

#undef _BLOCK
'''

if __name__ == '__main__':
	freqs = generateFrequencies(440)
	f = open('..\\NOTEFREQ.C', 'w')
	f.write(HEADER)
	for note, freq in freqs:
		fn, block = discover(freq)
		print '%3s: %dHz -> block %d, fn %d' % (note, freq, block, fn)
		f.write('\t/* %3s */ _BLOCK(%d) | %d,\n' % (note, block, fn))
	f.write(FOOTER)
	f.close()
