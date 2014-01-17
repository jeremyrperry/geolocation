<?php
/**
*This script houses the necessary functions to call up Maxmind binary database and the related PHP API.
*@link http://www.jeremyrperry.com
*@version 3.0
*@author Jeremy Perry jperry@phluant.com, jeremyrperry@gmail.com
*@package Phluant Mobile Drupal 7 Geolocation Module
*
*/

//Requirements are performed.
require_once 'binary_db_api/vendor/autoload.php';
use GeoIp2\Database\Reader;

/**
*The getBinaryDb function returns the database reader object based on the database requested.  If the binary db is named any different from the default names provided by Maxmind, this script needs to be adjusted accordingly.
*@param string $version, default is geolite
*@return The creader object based on the database version being used.
*/
function getBinaryDb($version = 'geolite'){
	switch($version){
		case 'geolite':
			return new Reader(dirname(__FILE__).'/binary_db/GeoLite2-City.mmdb');
			break;
		case 'geoip': 
			return new Reader(dirname(__FILE__).'/binary_db/GeoIP2-City.mmdb');
			break;
		default:
			return false;
	}		
}
