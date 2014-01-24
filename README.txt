geolocation_basic
=================

A Drupal Geolocation Module


This module offers both a basic geolocation and weather services functionality to both the site and as a REST API web service using JSON as the output format.

For the geolocation portion, it has a number of features that can locate a user's location from a number of external services.  The primary method this module uses is to resolve the user's IP address to a general location using Maxmind resources.  The module is fully compatible with both Maxmind's GeoLite(free) and GeoIP(paid) databases, be it CSV or the binary format.  Using the internal Maxmind resources is not required, as the module will also work with freegeoip.net right out of the box.  There are also a number of lookup methods for a user's location:  city/state/lat/lon by postal code, lat/lon by full address, full address by lat/lon, and city/state/zip by lat/lon.  These methods will use a combination of the Maxmind data whenever possible and are augmented by the Google Maps API.  The module also has the ability to use the Mapquest API, though this would require manual setup by a developer at this time.

For the Weather portion, this module is set up to make an API callout to the National Weather Service REST API, which is public information and can be accessed without any restrictions.  A user's weather information can be looked up by lat/lon, postal code, or IP address.  I also took the time to go through the very confusing NWS API to reorganize the data into a more useable format that isn't as confusing to parse through.  Because the NWS API can be sluggish at times, the module is designed to cache the weather data and use it for up to three hours.  The time can be manually adjusted in the code.

The group of geolocation and weather functions can either be called up via their REST API links or by invoking the PHP code locally.  Please read docs/api_documentation.txt for specific examples.

This module supports using Mongodb as an optional data source.  While basic documentation and a dump of the data structure has been provided, using Mongodb as the module's data source will require setup by a knowledgeable developer.  The use of Drupal's mongodb module will also be required.  Further documentation can be found in docs/mongodb_instructions.txt.

This module uses Object Oriented Programming wherever possible and requires the Module Object Oriented Programming API (moopapi) as a dependency.  Code-level documentation is complete as of 1/16/2014.

Please be aware that while this module should for the most part work out of the box, it is still under active development, should only be installed by a knowledgeable developer, and should always go through a thorough testing/QA cycle before being put on a production site.  It may require minor code modifications to work properly with your Drupal setup.

While the module will work without any Maxmind database, using either one is highly recommended to keep from hitting API limitations from Freegeoip.net and particularly Google Maps.  Please be aware that the module's DMA, City Postal By Geo, and Postal Code lookup functions utilize the data from the CSV Database whenever possible.  Because Maxmind's binary database API doesn't support any form of lookup outside of an IP address, setting up the CSV city data is still recommended even when using the binary database to avoid using the Google Maps API.  Regardless of the format chosen, you will need to download the databases directly from Maxmind as they are too big to include as a part of the module.  If the CSV version is chosen, be aware that some of the column names in the modules's table/collection structure are different from the provided CSV tables, so please plan accordingly. The CSV data can be imported into either a traditional Drupal-supported RDBMS (i.e. MySQL, Postgres, SQL Server, etc.) or a Mongodb instance.  The Maxmind Geolite links are below.

CSV:

Geolite CSV Database Page - http://dev.maxmind.com/geoip/legacy/geolite/
Geolite CSV Database City Download - http://geolite.maxmind.com/download/geoip/database/GeoLiteCity_CSV/GeoLiteCity-latest.zip

Binary:

Geolite Binary Database Page - http://dev.maxmind.com/geoip/geoip2/geolite2/
Geolite Binary Database Download - http://geolite.maxmind.com/download/geoip/database/GeoLite2-City.mmdb.gz

Further RDBMS setup instructions can be found in docs/rdbms_geolocation_instructions.txt.

If you are using the binary database, please see docs/binary_db_instructions.txt for further setup instructions.

Maxmind's full subscription GeoIP databases can be downloaded from the appropriate section of the site.

There is no copyright restriction on this module, and you are free to use or modify it in whole or in part for any purpose.  While I hope you find this module useful, it comes with no warranty, express or implied.  I will try to be helpful in answering basic questions, but I will not provide formal unpaid tech support.  Feedback, bug reports, etc. are always welcome on the project's GitHub page or Drupal Sandbox profile.  Use of this module does not exempt one from the applicable copyright, licensing, and API policies set forth by the third party services utilized.

GitHub Project URL - http://github.com/jeremyrperry/geolocation_basic
Drupal Sandbox Project URL - https://drupal.org/sandbox/jeremyrperry/2094265
