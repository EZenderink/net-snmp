#include <net-snmp/net-snmp-config.h>

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <net-snmp/agent/table.h>
#include <net-snmp/agent/table_data2.h>
#include <net-snmp/agent/table_container.h>
#include <net-snmp/agent/read_only.h>

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

/** @defgroup tdata tdata: Implement a table with datamatted storage.
 *  @ingroup table
 *
 *  This helper helps you implement a table where all the rows are
 *  expected to be stored within the agent itself and not in some
 *  external storage location.  It can be used to store a list of
 *  rows, where a row consists of the indexes to the table and a
 *  generic data pointer.  You can then implement a subhandler which
 *  is passed the exact row definition and data it must return data
 *  for or accept data for.  Complex GETNEXT handling is greatly
 *  simplified in this case.
 *
 *  @{
 */

/* ==================================
 *
 * TData API: Table maintenance
 *
 * ================================== */

/*
 * generates the index portion of an table oid from a varlist.
 */
void
_netsnmp_tdata_generate_index_oid(netsnmp_tdata_row *row)
{
    build_oid(&row->oid_index.oids, &row->oid_index.len, NULL, 0, row->indexes);
}

/** creates and returns a 'tdata' table data structure */
netsnmp_tdata *
netsnmp_tdata_create(const char *name)
{
    netsnmp_tdata *table = SNMP_MALLOC_TYPEDEF(netsnmp_tdata);
    if ( !table )
        return NULL;

    if (name)
        table->name = strdup(name);
    table->container = netsnmp_container_find( "table_container" );
    return table;
}

/** creates and returns a pointer to new row data structure */
netsnmp_tdata_row *
netsnmp_tdata_create_row(void)
{
    netsnmp_tdata_row *row = SNMP_MALLOC_TYPEDEF(netsnmp_tdata_row);
    return row;
}

/** clones a 'tdata' row. DOES NOT CLONE THE TABLE-SPECIFIC ENTRY DATA. */
netsnmp_tdata_row *
netsnmp_tdata_clone_row(netsnmp_tdata_row *row)
{
    netsnmp_tdata_row *newrow = NULL;
    if (!row)
        return NULL;

    memdup((u_char **) & newrow, (u_char *) row,
           sizeof(netsnmp_tdata_row));
    if (!newrow)
        return NULL;

    if (row->indexes) {
        newrow->indexes = snmp_clone_varbind(newrow->indexes);
        if (!newrow->indexes)
            return NULL;
    }

    if (row->oid_index.oids) {
        memdup((u_char **) & newrow->oid_index.oids,
               (u_char *) row->oid_index.oids,
               row->oid_index.len * sizeof(oid));
        if (!newrow->oid_index.oids)
            return NULL;
    }

    return newrow;
}

/** deletes the memory used by the specified row
 *  returns the table-specific entry data
 *  (that it doesn't know how to delete) */
void           *
netsnmp_tdata_delete_row(netsnmp_tdata_row *row)
{
    void           *data;

    if (!row)
        return NULL;

    /*
     * free the memory we can 
     */
    if (row->indexes)
        snmp_free_varbind(row->indexes);
    SNMP_FREE(row->oid_index.oids);
    data = row->data;
    free(row);

    /*
     * return the void * pointer 
     */
    return data;
}

/**
 * Adds a row to the given table (stored in proper lexographical order).
 *
 * returns SNMPERR_SUCCESS on successful addition.
 *      or SNMPERR_GENERR  on failure (E.G., indexes already existed)
 */
int
netsnmp_tdata_add_row(netsnmp_tdata     *table,
                      netsnmp_tdata_row *row)
{
    if (!row || !table)
        return SNMPERR_GENERR;

    if (row->indexes)
        _netsnmp_tdata_generate_index_oid(row);

    if (!row->oid_index.oids) {
        snmp_log(LOG_ERR,
                 "illegal data attempted to be added to table %s (no index)\n",
                 table->name);
        return SNMPERR_GENERR;
    }

    /*
     * we don't store the index info as it
     * takes up memory. 
     */
    if (!table->store_indexes) {
        snmp_free_varbind(row->indexes);
        row->indexes = NULL;
    }

    /*
     * add this row to the stored table
     */
    CONTAINER_INSERT( table->container, row );
    DEBUGMSGTL(("tdata_add_row", "added row (%x)\n", row));

    return SNMPERR_SUCCESS;
}

/** swaps out origrow with newrow.  This does *not* delete/free anything! */
NETSNMP_INLINE void
netsnmp_tdata_replace_row(netsnmp_tdata *table,
                               netsnmp_tdata_row *origrow,
                               netsnmp_tdata_row *newrow)
{
    netsnmp_tdata_remove_row(table, origrow);
    netsnmp_tdata_add_row(table, newrow);
}

/**
 * removes a row from the given table and returns it (no free's called)
 *
 * returns the row pointer itself on successful removing.
 *      or NULL on failure (bad arguments)
 */
netsnmp_tdata_row *
netsnmp_tdata_remove_row(netsnmp_tdata *table,
                              netsnmp_tdata_row *row)
{
    if (!row || !table)
        return NULL;

    CONTAINER_REMOVE( table->container, row );
    return row;
}

/**
 * removes and frees a row of the given table and
 *  returns the table-specific entry data
 *
 * returns the void * pointer on successful deletion.
 *      or NULL on failure (bad arguments)
 */
void           *
netsnmp_tdata_remove_and_delete_row(netsnmp_tdata     *table,
                                    netsnmp_tdata_row *row)
{
    if (!row || !table)
        return NULL;

    /*
     * remove it from the list 
     */
    netsnmp_tdata_remove_row(table, row);
    return netsnmp_tdata_delete_row(row);
}


/* ==================================
 *
 * TData API: MIB maintenance
 *
 * ================================== */

Netsnmp_Node_Handler _netsnmp_tdata_helper_handler;

/** Creates a tdata handler and returns it */
netsnmp_mib_handler *
netsnmp_get_tdata_handler(netsnmp_tdata *table)
{
    netsnmp_mib_handler *ret = NULL;

    if (!table) {
        snmp_log(LOG_INFO,
                 "netsnmp_get_tdata_handler(NULL) called\n");
        return NULL;
    }

    ret = netsnmp_create_handler(TABLE_DATA2_NAME,
                               _netsnmp_tdata_helper_handler);
    if (ret) {
        ret->flags |= MIB_HANDLER_AUTO_NEXT;
        ret->myvoid = (void *) table;
    }
    return ret;
}

/*
 * The helper handler that takes care of passing a specific row of
 * data down to the lower handler(s).  The table_container helper
 * has already taken care of identifying the appropriate row of the
 * table (and converting GETNEXT requests into an equivalent GET request)
 * So all we need to do here is make sure that the row is accessible
 * using tdata-style retrieval techniques as well.
 */
int
_netsnmp_tdata_helper_handler(netsnmp_mib_handler *handler,
                                  netsnmp_handler_registration *reginfo,
                                  netsnmp_agent_request_info *reqinfo,
                                  netsnmp_request_info *requests)
{
    netsnmp_tdata *table = (netsnmp_tdata *) handler->myvoid;
    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    netsnmp_tdata_row          *row;

    switch ( reqinfo->mode ) {
    case MODE_GET:
    case MODE_SET_RESERVE1:

        for (request = requests; request; request = request->next) {
            if (request->processed)
                continue;
    
            table_info = netsnmp_extract_table_info(request);
            if (!table_info)
                continue;           /* ack */
            row = netsnmp_container_table_row_extract( request );

            netsnmp_request_add_list_data(request,
                                      netsnmp_create_data_list(
                                          TABLE_DATA2_TABLE, table, NULL));
            netsnmp_request_add_list_data(request,
                                      netsnmp_create_data_list(
                                          TABLE_DATA2_ROW,   row,   NULL));
        }
    }

    /* next handler called automatically - 'AUTO_NEXT' */
    return SNMP_ERR_NOERROR;
}


/** registers a handler as a data table.
 *  If table_info != NULL, it registers it as a normal table too. */
int
netsnmp_register_tdata(netsnmp_handler_registration    *reginfo,
                       netsnmp_tdata                   *table,
                       netsnmp_table_registration_info *table_info)
{
    netsnmp_inject_handler(reginfo, netsnmp_get_tdata_handler(table));
    return netsnmp_container_table_register(reginfo, table_info,
                  table->container, TABLE_CONTAINER_KEY_NETSNMP_INDEX);
}

/** registers a handler as a read-only data table
 *  If table_info != NULL, it registers it as a normal table too. */
int
netsnmp_register_read_only_tdata(netsnmp_handler_registration *reginfo,
                       netsnmp_tdata                   *table,
                       netsnmp_table_registration_info *table_info)
{
    netsnmp_inject_handler(reginfo, netsnmp_get_read_only_handler());
    return netsnmp_register_tdata(reginfo, table, table_info);
}

/** extracts the tdata table from the request structure */
netsnmp_tdata *
netsnmp_tdata_extract(netsnmp_request_info *request)
{
    return (netsnmp_tdata *) netsnmp_request_get_list_data(request,
                                                           TABLE_DATA2_TABLE);
}

/** extracts the tdata container from the request structure */
netsnmp_container *
netsnmp_tdata_extract_container(netsnmp_request_info *request)
{
    netsnmp_tdata *tdata = netsnmp_request_get_list_data(request,
                                                         TABLE_DATA2_TABLE);
    return ( tdata ? tdata->container : NULL );
}

/** extracts the tdata row being accessed from the request structure */
netsnmp_tdata_row *
netsnmp_tdata_extract_row(netsnmp_request_info *request)
{
    return (netsnmp_tdata_row *) netsnmp_container_table_row_extract(request);
}

/** extracts the (table-specific) entry being accessed from the
 *  request structure */
void           *
netsnmp_tdata_extract_entry(netsnmp_request_info *request)
{
    netsnmp_tdata_row *row =
        (netsnmp_tdata_row *) netsnmp_tdata_extract_row(request);
    if (row)
        return row->data;
    else
        return NULL;
}

/** inserts a newly created tdata row into a request */
NETSNMP_INLINE void
netsnmp_insert_tdata_row(netsnmp_request_info *request,
                         netsnmp_tdata_row *row)
{
    netsnmp_container_table_row_insert(request, (netsnmp_index *)row);
}


/* ==================================
 *
 * Generic API: Row operations
 *
 * ================================== */

/** returns the (table-specific) entry data for a given row */
void *
netsnmp_tdata_row_entry( netsnmp_tdata_row *row )
{
    if (row)
        return row->data;
    else
        return NULL;
    //return ( row ? row->data : NULL );
}

/** returns the first row in the table */
netsnmp_tdata_row *
netsnmp_tdata_get_first_row(netsnmp_tdata *table)
{
    return (netsnmp_tdata_row *)CONTAINER_FIRST( table->container );
}

/** finds a row in the 'tdata' table given another row */
netsnmp_tdata_row *
netsnmp_tdata_get_from_row(netsnmp_tdata     *table,
                           netsnmp_tdata_row *row)
{
    return CONTAINER_FIND( table->container, row );
}

/** returns the next row in the table */
netsnmp_tdata_row *
netsnmp_tdata_get_next_row(netsnmp_tdata      *table,
                           netsnmp_tdata_row  *row)
{
    return (netsnmp_tdata_row *)CONTAINER_NEXT( table->container, row  );
}

/** finds a row in the 'tdata' table given the index values */
netsnmp_tdata_row *
netsnmp_tdata_get(netsnmp_tdata         *table,
                  netsnmp_variable_list *indexes)
{
    oid             searchfor[      MAX_OID_LEN];
    size_t          searchfor_len = MAX_OID_LEN;

    build_oid_noalloc(searchfor, MAX_OID_LEN, &searchfor_len, NULL, 0,
                      indexes);
    return netsnmp_tdata_get_from_oid(table, searchfor, searchfor_len);
}

/** finds a row in the 'tdata' table given the index OID */
netsnmp_tdata_row *
netsnmp_tdata_get_from_oid(netsnmp_tdata *table,
                           oid * searchfor, size_t searchfor_len)
{
    netsnmp_index index;
    if (!table)
        return NULL;

    index.oids = searchfor;
    index.len  = searchfor_len;
    return CONTAINER_FIND( table->container, &index );
}

/** finds the lexically next row in the 'tdata' table
    given the index values */
netsnmp_tdata_row *
netsnmp_tdata_getnext(netsnmp_tdata         *table,
                      netsnmp_variable_list *indexes)
{
    oid             searchfor[      MAX_OID_LEN];
    size_t          searchfor_len = MAX_OID_LEN;

    build_oid_noalloc(searchfor, MAX_OID_LEN, &searchfor_len, NULL, 0,
                      indexes);
    return netsnmp_tdata_getnext_from_oid(table, searchfor, searchfor_len);
}

/** finds the lexically next row in the 'tdata' table
    given the index OID */
netsnmp_tdata_row *
netsnmp_tdata_getnext_from_oid(netsnmp_tdata *table,
                               oid * searchfor, size_t searchfor_len)
{
    netsnmp_index index;
    if (!table)
        return NULL;

    index.oids = searchfor;
    index.len  = searchfor_len;
    return CONTAINER_NEXT( table->container, &index );
}

int
netsnmp_tdata_num_rows(netsnmp_tdata *table)
{
    if (!table)
        return 0;
    return CONTAINER_SIZE( table->container );
}

/* ==================================
 *
 * Generic API: Index operations on a 'tdata' table
 *
 * ================================== */


/** compare a row with the given index values */
int
netsnmp_tdata_compare(netsnmp_tdata_row     *row,
                      netsnmp_variable_list *indexes)
{
    oid             searchfor[      MAX_OID_LEN];
    size_t          searchfor_len = MAX_OID_LEN;

    build_oid_noalloc(searchfor, MAX_OID_LEN, &searchfor_len, NULL, 0,
                      indexes);
    return netsnmp_tdata_compare_oid(row, searchfor, searchfor_len);
}

/** compare a row with the given index OID */
int
netsnmp_tdata_compare_oid(netsnmp_tdata_row     *row,
                          oid * compareto, size_t compareto_len)
{
    netsnmp_index *index = (netsnmp_index *)row;
    return snmp_oid_compare( index->oids, index->len,
                             compareto,   compareto_len);
}

int
netsnmp_tdata_compare_subtree(netsnmp_tdata_row     *row,
                              netsnmp_variable_list *indexes)
{
    oid             searchfor[      MAX_OID_LEN];
    size_t          searchfor_len = MAX_OID_LEN;

    build_oid_noalloc(searchfor, MAX_OID_LEN, &searchfor_len, NULL, 0,
                      indexes);
    return netsnmp_tdata_compare_subtree_oid(row, searchfor, searchfor_len);
}

int
netsnmp_tdata_compare_subtree_oid(netsnmp_tdata_row     *row,
                                  oid * compareto, size_t compareto_len)
{
    netsnmp_index *index = (netsnmp_index *)row;
    return snmp_oidtree_compare( index->oids, index->len,
                                 compareto,   compareto_len);
}

/*
 * @} 
 */
