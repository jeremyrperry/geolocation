geolocation_basic
=================

A Drupal Geolocation Module


This module offers both a basic geolocation and weather services functionality to both the site and as a REST API web service using JSON as the output format.

For the geolocation portion, it has a number of features that can locate a user's location from a number of external services.  The primary method this module uses is to resolve the user's IP address to a general location.  It comes set up to install Maxmind's legacy Geolite city database as two tables into the site's mail database, and will use these tables as the primary lookup method.  It alos includes the ability to make an API call to freegeoip.net if the database can't resolve the ip address, or if the data is not installed.  There are also a number of lookup methods for a user's location:  city/state/lat/lon by postal code, lat/lon by full address, full address by lat/lon, and city/state/zip by lat/lon.  These methods will use a combination of the Geolite data whenever possible and are augmented by the Google Maps API.  The module also has the ability to use the Mapquest API, though this would require manual setup by a developer at this time.

For the Weather portion, this module is set up to make an API callout to the National Weather Service REST API, which is public information and can be accessed without any restrictions.  A user's weather information can be looked up by lat/lon, postal code, or IP address.  I also took the time to go through the very confusing NWS API to reorganize the data into a more useable format that isn't as confusing to parse through.  Because the NWS API can be sluggish at times, the module is designed to cache the weather data and use it for up to three hours.  The time can be manually adjusted in the code.

This module uses Object Oriented Programming wherever possible and requires the Module Object Oriented Programming API (moopapi) as a dependency.  Code-level documentation is mostly complete as of 9/25/13.

Please be aware that while this module should for the most part work out of the box, it is still under active development and should only be installed by knowledgeable developers.  It may require minor code modifications to work properly with your Drupal setup.  I am planning a number of revisions including but not limited to full documentation, admin pages for adding in API keys, make use of Maxmind's more advanced Geolocation databases, etc.

While the module will work without the Geolite data installed, this step is highly recommended in order to keep from hitting API limitations from Freegeoip.net and particularly Google Maps.  The data is in CSV format and because of the size of the datasets, I'm not sure how to provide an automated import into the local tables.  It will be necessary to use a DB admin tool to perform this task.   The tables you will need to load the IP and city data into are geolocation_ip_locations and geolocation_cities respectively.  Also, be aware that some of the column names in the provided tables differ from the CSV data column names provided by Maxmind, so please plan accordingly. The Maxmind links are below:

Geolite Database Page:  http://dev.maxmind.com/geoip/legacy/geolite/
Geolite Database City Download (CSV):  http://geolite.maxmind.com/download/geoip/database/GeoLiteCity_CSV/GeoLiteCity-latest.zip

There is no copyright restriction on this module, and you are free to use or modify it in whole or in part for any purpose.  While I hope you find this module useful, it comes with no warranty, express or implied.  I will try to be helpful in answering basic questions, but I will not provide formal unpaid tech support.  Feedback, bug reports, etc. are always welcome.  Use of this module does not exempt one from the applicable copyright, licencing, and API policies set forth by the third party services utilized.

GitHub Project URL - http://github.com/jeremyrperry/geolocation_basic
Drupal Sandbox Project URL - https://drupal.org/sandbox/jeremyrperry/2094265
