/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/grib/GribToRequest.h"

#include "grib_api.h"

#include "eckit/config/Resource.h"
#include "eckit/config/ResourceMgr.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/parser/StringTools.h"

#include "metkit/MarsRequest.h"



namespace metkit {
namespace grib {

//----------------------------------------------------------------------------------------------------------------------

static void upper_case(char *p)
{
	while(*p)
	{
		if(islower(*p)) *p = toupper(*p);
		p++;
	}

}

void GribToRequest::handleToRequest(grib_handle * const g, MarsRequest& req) {

	grib_keys_iterator* ks;

	std::string name;

	char value[80];
	size_t len = sizeof(value);
	int e;

	ASSERT(g);

    static std::string gribToRequestNamespace = eckit::Resource<std::string>("gribToRequestNamespace", "mars");

    ks  = grib_keys_iterator_new(g, GRIB_KEYS_ITERATOR_ALL_KEYS, gribToRequestNamespace.c_str());

	while(grib_keys_iterator_next(ks))
	{

		name = grib_keys_iterator_get_name(ks);

		if((e = grib_keys_iterator_get_string(ks,value,&len))) {
			std::ostringstream oss;
			oss << "Cannot get "<<name<<" as string "<<e<<" ("<<grib_get_error_message(e)<<")";
			throw eckit::Exception(oss.str(),Here());
		}

		name = eckit::StringTools::upper(name);

		if( name == "EXPVER" ) {
			upper_case(value);
		}

		req.setValue(name, std::string(value));
		len = sizeof(value);
	}


	name = "identifier";
	len = sizeof(value);
	if((e = grib_get_string(g,name.c_str(),value,&len))) {
		std::ostringstream oss;
		oss << "Cannot get "<<name<<" as string "<<e<<" ("<<grib_get_error_message(e)<<")";
		throw eckit::Exception(oss.str(),Here());
	}

	if(strcmp(value,"GRIB") != 0) {
		std::ostringstream oss;
		oss << "Unexpected message type ("<< value<<")";
		throw eckit::Exception(oss.str(),Here());
	}

#if 0

	const char *stream = get_value(r,"STREAM",0);
	const char *number = get_value(r,"NUMBER",0);


	{
		mars.pseudogrib = (strcmp(value,"BUDG") == 0) || (strcmp(value,"TIDE") == 0);
		if(mars.pseudogrib)
		{
			marslog(LOG_WARN,"Pseudo GRIB encountered (%s)",value);
			if(stream == NULL)
				stream = getenv("MARS_PSEUDOGRIB_STREAM");

			if(stream != NULL)
			{
				marslog(LOG_DBUG,"Setting STREAM to '%s'",stream);
				set_value(r,"STREAM","%s",stream);
			}

			if(number != NULL)
			{
				marslog(LOG_DBUG,"Setting NUMBER to '%s'",number);
				set_value(r,"NUMBER","%s",number);
			}
		}
		else {
			if(strcmp(value,"GRIB") != 0)
				marslog(LOG_EXIT,"Unexpected message type (%s)",value);
			else
			{
				/* Get the edition */
				long edition = 0;
				if(e = grib_get_long(g,"edition",&edition))
				{
					marslog(LOG_EXIT,"Cannot get edition as long: %d (%s)",e,
							grib_get_error_message(e));
				}
				set_value(r,"_EDITION","%ld",edition);
			}
		}
	}

	if(grib_get_long(g,"localDefinitionNumber",&local) ==  0 && local == 191) /* TODO: Not grib2 compatible, but speed-up process */
	if(grib_get_size(g,"freeFormData",&len) ==  0 && len != 0)
	{
		char buffer[10240];
		len = sizeof(buffer);
		if(e = grib_get_bytes(g,"freeFormData",buffer,&len))
			marslog(LOG_EROR,"Cannot get freeFormData %d (%s)",name,e,
					grib_get_error_message(e));
		else {
			request* s = decode_free_format_request(buffer,len);

			if(mars.debug)
			{
				marslog(LOG_DBUG,"Free format request:");
				print_all_requests(s);
			}
			/* set_value(r,"PARAM","%d.%d",s1->parameter,s1->version); */

			reset_language(mars_language());
			mars.expflags = EXPAND_MARS | EXPAND_NO_DEFAULT;
			s = expand_mars_request(s);
			if( s == NULL)
			{
				/* if(mars.exit_on_failed_expand) */
				{
					e = -2;
					marslog(LOG_EROR,"Failed to expand request");
				}
			}
			mars.expflags = EXPAND_MARS;
			reqcpy(r,s);
			free_all_requests(s);

		}

	}
#endif

	grib_keys_iterator_delete(ks);
}

void GribToRequest::handleToRequest(const grib::GribHandle& grib, MarsRequest& req) {
	NOTIMP;
}

void GribToRequest::gribToRequest(const void* buffer, size_t length, MarsRequest& req) {

    grib_handle* grib = grib_handle_new_from_message(0, const_cast<void*>(buffer), length);

	GribToRequest::handleToRequest(grib, req);

	grib_handle_delete(grib);
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace grib
} // namespace metkit
