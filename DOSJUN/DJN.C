/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"
#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

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

void Free_Djn(djn *d)
{
	int i;
	djn_file *f;

	Close_Djn(d);

	for (i = 0; i < d->count; i++) {
		f = &d->files[i];
		Free(f->name);
		Free(f->object);
	}

	Free(d->files);
}

void *Load_File(djn *d, djn_file* f)
{
	if (d->f == null) {
		dief("Load_File: could not load #%x, djn is already closed", f->id);
		return null;
	}

	fseek(d->f, f->offset, SEEK_SET);

	switch (f->type) {
		case ftCampaign:
			f->object = SzAlloc(1, campaign, "Load_File:campaign");
			Read_Campaign(d->f, f->object);
			break;

		default:
			dief("Load_File: don't know how to parse type %d", f->type);
			break;
	}

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
