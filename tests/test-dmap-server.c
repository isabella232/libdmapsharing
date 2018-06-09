/*
 * Copyright (C) 2008 W. Michael Petullo <mike@flyn.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>

#include <libdmapsharing/dmap.h>
#include <libdmapsharing/test-dmap-av-record.h>
#include <libdmapsharing/test-dmap-image-record.h>
#include <libdmapsharing/test-dmap-av-record-factory.h>
#include <libdmapsharing/test-dmap-image-record-factory.h>
#include <libdmapsharing/test-dmap-db.h>
#include <libdmapsharing/test-dmap-container-record.h>
#include <libdmapsharing/test-dmap-container-db.h>

/* For use when deciding whether to test DAAP or DPAP. */
enum {
	DAAP,
	DPAP
};

static char *
dmap_sharing_default_share_name ()
{
	const gchar *real_name;

	real_name = g_get_real_name ();
	if (strcmp (real_name, "Unknown") == 0) {
		real_name = g_get_user_name ();
	}

	return g_strdup_printf ("%s's Media (libdmapsharing test)", real_name);
}

static void error_cb(DmapShare *share,
                     GError *error,
                     gpointer user_data)
{
	g_error("%s", error->message);
}

static void
create_share (guint conn_type)
{
	char *name = dmap_sharing_default_share_name ();
	DmapContainerRecord *dmap_container_record = \
		DMAP_CONTAINER_RECORD (test_dmap_container_record_new ());
	DmapContainerDb *dmap_container_db = \
		DMAP_CONTAINER_DB (test_dmap_container_db_new
					(dmap_container_record));
	DmapRecordFactory *factory;
	DmapRecord *record;
	DmapShare *share = NULL;
	GError *error = NULL;
	gboolean ok;
	DmapDb *db;

	if (conn_type == DAAP) { 
		factory = DMAP_RECORD_FACTORY (test_dmap_av_record_factory_new ());

	} else {
		factory = DMAP_RECORD_FACTORY (test_dmap_image_record_factory_new ());
	}

	record = DMAP_RECORD (dmap_record_factory_create (factory, NULL, NULL));
	db = DMAP_DB (test_dmap_db_new ());
	dmap_db_add (db, record, NULL);
	g_object_unref (record);

	g_warning ("initialize DAAP sharing");

	if (conn_type == DAAP) {
		share = DMAP_SHARE (dmap_av_share_new (name,
                                                       NULL,
                                                       db,
                                                       dmap_container_db,
		                                       NULL));
	} else {
		share = DMAP_SHARE (dmap_image_share_new (name,
                                                          NULL,
                                                          db,
                                                          dmap_container_db,
                                                          NULL));
	}

	g_assert (NULL != share);

	g_signal_connect(share, "error", G_CALLBACK(error_cb), NULL);

	ok = dmap_share_serve(share, &error);
	if (!ok) {
		g_error("Error starting server: %s", error->message);
	}

	ok = dmap_share_publish(share, &error);
	if (!ok) {
		g_error("Error publishing server: %s", error->message);
	}

	g_free (name);
}

int
main (int argc, char *argv[])
{
	guint conn_type = DAAP;
	static GMainLoop *loop;

	if (argc == 2)
		conn_type = atoi (argv[1]);

	loop = g_main_loop_new (NULL, FALSE);

	create_share (conn_type);

	g_main_loop_run (loop);

	exit(EXIT_SUCCESS);
}
