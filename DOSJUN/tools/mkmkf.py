from glob import glob

# Config
DEFINES = ['MEMORY_DEBUG', 'FAR_MEMORY']
#DEFINES = ['FAR_MEMORY']
OBJECTS = {
	'DEMO': ['MKDEMO', 'CAMPAIGN', 'IO', 'ITEMS', 'MONSTERS', 'SAVE', 'ZONE'],
	'MAIN': ['DOSJUN', 'CAMPAIGN', 'CODE', 'COMBAT', 'FONT', 'FP', 'GFX', 'GRF', 'INPUT', 'IO', 'ITEMS', 'JFIGHTER', 'JCLERIC', 'JMAGE', 'JBARD', 'JROGUE', 'JRANGER', 'JOBS', 'LIST', 'LOGGING', 'MAIN', 'MEM', 'MONSTERS', 'MUS_SNG', 'MUSIC', 'PARTY', 'SAVE', 'SOUND', 'STATUS', 'ZONE'],
	'EDITOR': ['EDITOR', 'GFX', 'INPUT', 'IO', 'ITEMS', 'JC', 'JC_LEX', 'JC_PAR', 'MEM', 'MONSTERS', 'SAVE', 'ZONE'],
	'JCC': ['JCC', 'JC', 'JC_LEX', 'JC_PAR', 'MEM']
}
STANDALONE = ['DOSJUN', 'EDITOR', 'JCC', 'MKDEMO']
HEADERS = [h[h.index('\\')+1:] for h in glob('..\\*.H')]

# - MODEL   COD DAT
# =================
# t TINY	?	?
# s SMALL	<64	<64
# c COMPACT	<64 64+
# m MEDIUM	64+ <64
# l LARGE	64+ 64+
# h HUGE	64+	64+ individual data obj can be >64K
MODEL = 'l'
# remember that GAMELIB needs to be recompiled with this in mind

# Helper Stuff
def allobjects():
	objs = []
	for k, v in OBJECTS.items():
		objs += v
	return sorted(list(set(objs)))

def objname(o):
	return '%s.OBJ' % o

def wexe(exe, objs=None):
	if not objs: objs = exe
	f.write("""
%(exe)s.EXE: $(%(objs)s_OBJS) $(HEADERS)
	TCC $(CFLAGS_EXE) %(exe)s.OBJ DOSJUN.LIB ..\LIB\GAMELIB%(model)s.LIB
""" % { 'exe': exe, 'objs' : objs, 'model': MODEL.upper() })

# Write it
f = open('..\\MAKEFILE', 'w')

f.write('CFLAGS = -G -I\TC\INCLUDE -I..\LIB -m%s -d -w %s\n' % (MODEL, ' '.join(['-D%s' % d for d in DEFINES])))
f.write('CFLAGS_EXE = -G -L\TC\LIB -m%s -d -w\n' % MODEL)
for k, v in OBJECTS.items():
	f.write('%s_OBJS = %s\n' % (k, ' '.join(map(objname, v))))
f.write('HEADERS = %s\n' % ' '.join(HEADERS))

f.write("""
all: dosjun

demo: MKDEMO.EXE DEMO.ZON
dosjun: DOSJUN.EXE
editor: EDITOR.EXE
jcc: JCC.EXE

DEMO.ZON: MKDEMO.EXE
	MKDEMO
""")

wexe('DOSJUN', 'MAIN')
wexe('EDITOR')
wexe('MKDEMO')
wexe('JCC')

for o in allobjects():
	obj = objname(o)
	if o in STANDALONE:
		xtra = ''
	else:
		xtra = '\n\tTLIB DOSJUN.LIB +-%s.OBJ, DOSJUN.TXT' % o
	f.write( """
%(obj)s: %(o)s.C $(HEADERS)
	TCC $(CFLAGS) -c -o%(obj)s %(o)s.C%(extra)s\n""" % { 'o': o, 'obj': obj, 'extra': xtra })
