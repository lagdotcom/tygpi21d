/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"
#include "dosjun.h"

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef void(*ft_free_fn)(void*);
typedef void(*ft_read_fn)(FILE*, void*);
typedef void(*ft_write_fn)(FILE*, void*);

typedef struct loaderspec {
	djn_type type;
	size_t size;
	ft_free_fn free;
	ft_read_fn read;
	ft_write_fn write;
} loaderspec;

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport loaderspec loaders[] = {
	{ ftCampaign, sizeof(campaign), Free_Campaign, Read_Campaign, null },
	{ ftFont, sizeof(font), Free_Font, Read_Font, null },
	{ ftGlobals, sizeof(globals), Free_Globals, Read_Globals, Write_Globals },
	{ ftGraphic, sizeof(grf), Free_GRF, Read_GRF, null },
	{ ftItem, sizeof(item), Free_Item, Read_Item, null },
	{ ftMonster, sizeof(monster), Free_Monster, Read_Monster, null },
	{ ftMusic, sizeof(sng), Free_SNG, Read_SNG, null },
	{ ftNPC, sizeof(npc), Free_NPC, Read_NPC, Write_NPC },
	{ ftPalette, sizeof(palette), Free_Palette, Read_Palette, null },
	{ ftParty, sizeof(partystatus), Free_Party, Read_Party, Write_Party },
	{ ftPC, sizeof(pc), Free_PC, Read_PC, Write_PC },
	{ ftScript, sizeof(script), Free_Script, Read_Script, null },
	{ ftSound, sizeof(wav), Free_WAV, Read_WAV, null },
	{ ftStrings, sizeof(strings), Free_Strings, Read_Strings, null },
	{ ftZone, sizeof(zone), Free_Zone, Read_Zone, null },
	{ ftZoneOverlay, sizeof(zone_overlay), Free_Overlay, Read_Overlay, Write_Overlay },

	{ ftUnknown, null, null }
};

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport loaderspec Get_Spec(djn_type ty)
{
	int i;
	loaderspec spec;

	for (i = 0; ; i++) {
		spec = loaders[i++];
		if (spec.type == ty || spec.type == ftUnknown)
			return spec;
	}
}

bool Load_Djn(char *filename, djn *d)
{
	FILE *fp = fopen(filename, "rb");
	djn_file *x;
	UINT32 i;

	if (!fp) {
		dief("Load_Djn: Could not open for reading: %s\n", filename);
		return false;
	}

	Log("Load_Djn: %s", filename);

	fread(d, 1, DJN_HEADERSZ, fp);
	d->files = SzAlloc(d->count, djn_file, "Load_Djn");
	fseek(fp, d->directory, SEEK_SET);

	for (i = 0; i < d->count; i++) {
		x = &d->files[i];
		fread(x, 1, DJN_FILESZ, fp);
		if (d->flags & dfDesign) {
			x->name = Read_LengthString(fp, "Load_Djn.filename");
		}
	}

	d->f = fp;
	return true;
}

void Close_Djn(djn *d)
{
	if (d->f) {
		fclose(d->f);
		d->f = null;
	}
}

noexport void Free_Djn_File(djn_file *f)
{
	loaderspec spec = Get_Spec(f->type);

	Free(f->name);

	if (f->object == null)
		return;

	if (spec.free)
		spec.free(f->object);

	Free(f->object);
}

void Free_Djn(djn *d)
{
	int i;

	Close_Djn(d);

	for (i = 0; i < d->count; i++)
		Free_Djn_File(&d->files[i]);

	Free(d->files);
}

void *Load_File(djn *d, djn_file* f)
{
	loaderspec spec = Get_Spec(f->type);

	if (d->f == null) {
		dief("Load_File: could not load #%x, djn is already closed", f->id);
		return null;
	}

	fseek(d->f, f->offset, SEEK_SET);

	if (!spec.read)
		dief("Load_File: don't know how to parse type %d", f->type);

	f->object = Allocate(1, spec.size, "Load_File");
	spec.read(d->f, f->object);
	return f->object;
}

void *Lookup_File(djn *chain, file_id id)
{
	int i;
	djn *d = chain;
	djn_file *x;

	while (d) {
		for (i = 0; i < d->count; i++) {
			x = &d->files[i];
			if (x->id == id) {
				if (x->object == null) {
					x->object = Load_File(d, x);
				}

				return x->object;
			}
		}

		d = d->next;
	}

	Log("Lookup_File: could not find file #%x", id);
	return null;
}

void *Find_File_Type(djn *chain, djn_type ty)
{
	int i;
	djn *d = chain;
	djn_file *x;

	while (d) {
		for (i = 0; i < d->count; i++) {
			x = &d->files[i];
			if (x->type == ty) {
				if (x->object == null) {
					x->object = Load_File(d, x);
				}

				return x->object;
			}
		}

		d = d->next;
	}

	Log("Find_File_Type: could not find file of type %d", ty);
	return null;
}

void Unload_File(djn *chain, file_id id)
{
	djn_file *f = Lookup_File(chain, id);

	if (f != null) {
		Free_Djn_File(f);
	}
}
