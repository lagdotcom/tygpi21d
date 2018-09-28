/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"
#include "dosjun.h"

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef void(*ft_free_fn)(void*);
typedef bool(*ft_io_fn)(FILE*, void*);

typedef struct loaderspec {
	djn_type type;
	size_t size;
	ft_free_fn free;
	ft_io_fn read;
	ft_io_fn write;
} loaderspec;

/* G L O B A L S ///////////////////////////////////////////////////////// */

djn_file *current_reading_file;

noexport loaderspec loaders[] = {
	{ ftCampaign, sizeof(campaign), null,          Read_Campaign, null },
	{ ftDropTable,sizeof(droptable),Free_DropTable,Read_DropTable,null },
	{ ftGlobals,  sizeof(globals),  Free_Globals,  Read_Globals,  Write_Globals },
	{ ftGraphic,  sizeof(grf),      Free_GRF,      Read_GRF,      null },
	{ ftItem,     sizeof(item),     null,          Read_Item,     null },
	{ ftMonster,  sizeof(monster),  Free_Monster,  Read_Monster,  null },
	{ ftMusic,    sizeof(sng),      Free_SNG,      Read_SNG,      null },
	{ ftNPC,      sizeof(npc),      null,          Read_NPC,      Write_NPC },
	{ ftPalette,  sizeof(palette),  null,          Read_Palette,  null },
	{ ftParty,    sizeof(party),    null,          Read_Party,    Write_Party },
	{ ftPC,       sizeof(pc),       Free_PC,       Read_PC,       Write_PC },
	{ ftScript,   sizeof(script),   Free_Script,   Read_Script,   null },
	/* { ftSound,    sizeof(wav),      Free_WAV,      Read_WAV,      null }, */
	{ ftStrings,  sizeof(strings),  Free_Strings,  Read_Strings,  null },
	{ ftZone,     sizeof(zone),     Free_Zone,     Read_Zone,     null },
	{ ftOverlay,  sizeof(overlay),  Free_Overlay,  Read_Overlay,  Write_Overlay },

	{ ftUnknown, 0, null, null, null }
};

/* F U N C T I O N S ///////////////////////////////////////////////////// */

#define NAMEOF(f) (f->name ? f->name : "(unknown)")

noexport loaderspec *Get_Spec(djn_type ty)
{
	loaderspec *spec;

	for (spec = loaders; ; spec++) {
		if (spec->type == ty || spec->type == ftUnknown)
			return spec;
	}
}

bool Load_Djn(char *filename, djn *d)
{
	FILE *fp = fopen(filename, "rb");
	djn_file *f;
	int i;

	if (!fp) {
		dief("Load_Djn: Could not open for reading: %s\n", filename);
		return false;
	}

	Log("Load_Djn: %s", filename);

	fread(d, 1, DJN_HEADERSZ, fp);
	d->files = SzAlloc(d->count, djn_file, "Load_Djn");
	fseek(fp, d->directory, SEEK_SET);

	for (i = 0; i < d->count; i++) {
		f = &d->files[i];
		fread(f, 1, DJN_FILESZ, fp);
		f->parent = d;
		if (d->flags & dfDesign) {
			f->name = Read_LengthString(fp, "Load_Djn.filename");
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
	loaderspec *spec = Get_Spec(f->type);

	Log("Free_Djn_File: @%p, #%d(%d) %s", f, f->id, f->type, NAMEOF(f));

	Free(f->name);

	if (f->object == null)
		return;

	if (spec->free)
		spec->free(f->object);

	Free(f->object);
}

void Free_Djn(djn *d)
{
	int i;

	Log("Free_Djn: @%p", d);

	Close_Djn(d);

	for (i = 0; i < d->count; i++)
		Free_Djn_File(&d->files[i]);

	Free(d->files);
}

noexport void *Load_File(djn_file* f)
{
	djn *d = f->parent;
	loaderspec *spec = Get_Spec(f->type);

	Log("Load_File: #%d(%d) %s", f->id, f->type, NAMEOF(f));

	if (d->f == null) {
		dief("Load_File: could not load #%d, djn is already closed", f->id);
		return null;
	}

	if (!spec->read)
		dief("Load_File: don't know how to parse type %d", f->type);

	fseek(d->f, f->offset, SEEK_SET);
	f->object = Allocate(1, spec->size, "Load_File");

	Log("Load_File: @%ld, allocated %d bytes", f->offset, spec->size);

	if (f->object == null) {
		dief("Load_File: out of memory reading #%d", f->id);
		return null;
	}

	current_reading_file = f;
	spec->read(d->f, f->object);
	return f->object;
}

djn_file *Lookup_File_Entry(djn *chain, file_id id, bool load, bool chained)
{
	int i;
	djn *d = chain;
	djn_file *f;

	/* Special case: cannot look up ID 0 */
	if (id == 0)
		return null;

	while (d) {
		for (i = 0; i < d->count; i++) {
			f = &d->files[i];
			if (f->id == id) {
				if (load && f->object == null)
					f->object = Load_File(f);

				return f;
			}
		}

		d = chained ? d->next : null;
	}

	Log("Lookup_File_Entry: could not find file #%d%s", id, chained ? " in chain" : "");
	return null;
}

void *Lookup_File(djn *chain, file_id id, bool chained)
{
	djn_file *f;

	/* Special case: cannot look up ID 0 */
	if (id == 0)
		return null;

	f = Lookup_File_Entry(chain, id, true, chained);
	if (f)
		return f->object;

	return null;
}

bool In_Djn(djn *chain, file_id id, bool chained)
{
	/* Special case: cannot look up ID 0 */
	if (id == 0)
		return null;

	return Lookup_File_Entry(chain, id, false, chained) != null;
}

void *Find_File_Type(djn *chain, djn_type ty)
{
	int i;
	djn *d = chain;
	djn_file *f;

	while (d) {
		for (i = 0; i < d->count; i++) {
			f = &d->files[i];
			if (f->type == ty) {
				if (f->object == null) {
					f->object = Load_File(f);
				}

				return f->object;
			}
		}

		d = d->next;
	}

	Log("Find_File_Type: could not find file of type %d", ty);
	return null;
}

void Unload_File(djn *chain, file_id id)
{
	djn_file *f = Lookup_File_Entry(chain, id, false, true);

	if (f != null) {
		Log("Unload_File: #%d(%d) %s", id, f->type, NAMEOF(f));
		Free_Djn_File(f);
	} else {
		Log("Unload_File: #%d(NOT FOUND)", id);
	}
}

void Add_to_Djn(djn *d, void *obj, file_id id, djn_type ty)
{
	djn_file *f; 

	/* Don't add a file twice */
	if (In_Djn(d, id, false))
		return;

	Log("Add_to_Djn: adding #%d(%d)", id, ty);

	d->count++;
	d->files = Reallocate(d->files, d->count, sizeof(djn_file), "Add_to_Djn");
	if (d->files == null)
		die("Add_to_Djn: out of memory");

	f = &d->files[d->count - 1];
	f->id = id;
	f->object = obj;
	f->type = ty;
	f->parent = d;
	f->name = null;
}

bool Save_Djn(char *filename, djn *d)
{
	int i;
	loaderspec *spec;
	djn_file *f;
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		dief("Save_Djn: Could not open for writing: %s\n", filename);
		return false;
	}

	fseek(fp, DJN_HEADERSZ, SEEK_SET);
	for (i = 0, f = d->files; i < d->count; i++, f++) {
		spec = Get_Spec(f->type);

		if (!spec->write) {
			fclose(fp);
			dief("Save_Djn: cannot write file of type %d", f->type);
			return false;
		}

		f->offset = ftell(fp);
		spec->write(fp, f->object);
		f->size = ftell(fp) - f->offset;
	}

	d->directory = ftell(fp);
	for (i = 0, f = d->files; i < d->count; i++, f++) {
		fwrite(f, DJN_FILESZ, 1, fp);

		if (d->flags & dfDesign)
			Write_LengthString(f->name, fp);
	}

	fseek(fp, 0, SEEK_SET);
	fwrite(d, DJN_HEADERSZ, 1, fp);

	fclose(fp);
	return true;
}

int Find_Files_of_Type(djn *d, djn_type ty, djn_subtype sty, file_id *buffer, int max)
{
	djn_file *f;
	int count = 0,
		i;

	for (i = 0; i < d->count; i++) {
		f = &d->files[i];

		if (f->type == ty && (sty == stAny || f->subtype == sty)) {
			buffer[count] = f->id;
			count++;

			if (count == max)
				break;
		}
	}

	return count;
}

/* Get the file ID for a loaded file. */
file_id Get_File_ID(djn *chain, void *data)
{
	/* TODO: This is dumb. We should store the ID in the file somewhere instead. */
	djn *d = chain;
	djn_file *f;
	int i;

	while (d) {
		for (i = 0; i < d->count; i++) {
			f = &d->files[i];
			if (f->object == data)
				return f->id;
		}

		d = d->next;
	}

	return 0;
}
