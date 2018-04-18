from random import randrange

def sortalg(items):
	size = len(items)
	for i in range(0, size - 1):
		a = items[i]
		for j in range(i, size):
			b = items[j]
			if b > a:
				items[i], items[j] = b, a
				a, b = b, a

def sortest(items):
	print 'Start:', items
	sortalg(items)
	print 'End:', items

if __name__ == '__main__':
	sortest([randrange(0, 100) for i in xrange(10)])
