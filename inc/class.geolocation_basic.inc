<?php
/**
*This script houses the Geolocation Module class.  It is written in OOP in order to be more easily ported over to Drupal 8 when the proper time comes.
*@link http://www.jeremyrperry.com
*@version 2.0
*@author Jeremy Perry jperry@phluant.com, jeremyrperry@gmail.com
*@package Phluant Mobile Drupal 7 Geolocation Module
*@todo Updated on 9-19-13 to remove Phluant-specific code from the class.
*
*/

class geolocation_basic{
	protected $mapquest_api;

	/**
	*The destruct function ensures the default database is set so there are no potential errors.
	*@return see above.
	*/
	function __destruct(){
		db_set_active('default');
	}

	/**
	*The apiLimitCheck function checks the inputted API object against the maximum API limits and returns the allowed number of API callups remaining
	*@param object $obj
	*@return The remaining value and database query are outputted to array fromat.
	*/
	protected function apiLimitCheck($obj){
		//Ensures the database is set to default.
		db_set_active('default');
		//Query values are declared.
		$timestamp = strtotime('-'.$obj->limit_time_unit);
		$queryArr = array(
			':api_val_id'=>$obj->id,
			':timestamp'=>$timestamp,
		);
		//Query is ran, and values are outputted.
		$query = "select sum(insert_count) as used from geolocation_api_track where api_val_id = '".$obj->id."' and timestamp >= '".$timestamp."'";
		$q = db_query($query);
		$r = $q->fetchAssoc();
		$output['value'] = $obj->limit_qty - $r['used'];
		$output['query'] = $query;
		return $output;
	}

	/**
	*The degreesToDirection function is able to take a degree and convert it into the corresponding geographic direction.  It works in conjunction with the getWeather function.
	*@param string $degree
	*@return The degree number is converted into the corresponding geographical direction.
	*/
	protected function degreeToDirection($degree){
		if($degree > 348.75 || $degree <= 11.25){
			return 'N';
		}
		if($degree > 11.25 && $degree <= 33.75){
			return 'NNE';
		}
		if($degree > 33.75 && $degree <= 56.25){
			return 'NE';
		}
		if($degree > 56.25 && $degree <= 78.75){
			return 'ENE';
		}
		if($degree > 78.75 && $degree <= 101.25){
			return 'E';
		}
		if($degree > 101.25 && $degree <= 123.75){
			return 'ESE';
		}
		if($degree > 123.75 && $degree <= 146.25){
			return 'SE';
		}
		if($degree > 146.25 && $degree <= 168.75){
			return 'SSE';
		}
		if($degree > 168.75 && $degree <= 191.25){
			return 'S';
		}
		if($degree > 191.25 && $degree <= 213.75){
			return 'SSW';
		}
		if($degree > 213.75 && $degree <= 236.25){
			return 'SW';
		}
		if($degree > 236.25 && $degree <= 258.75){
			return 'WSW';
		}
		if($degree > 258.75 && $degree <= 281.25){
			return 'W';
		}
		if($degree > 281.25 && $degree <= 303.75){
			return 'WNW';
		}
		if($degree > 303.75 && $degree <= 326.25){
			return 'NW';
		}
		if($degree > 326.25 && $degree <= 348.75){
			return 'NNW';
		}
	}

	/**
	*The error function is available throughout the object to return an error notifying the user of an invalid request.
	*@param boolean $isJson, defaults to true.
	*@return The error message data in either JSON or PHP associative array format.
	*/
	protected function error($isJson= true){
		$output['status'] = 'error';
		$output['msg'] = 'invalid request';
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	/**
	*The getApi function allows for an api value to be called from the database .
	*@param string $name
	*@return The database result object
	*/
	protected function getApi($name){
		db_set_active('default');
		$q = db_query("select * from geolocation_api_values where name = :name", array(':name'=>$name));
		return $q->fetchObject();
	}

	/**
	*The getAddressByGeo function is set up to make an API call the Google Maps Geocoder to get the full address of the inputted geocoordinates.  It outputs a much more simplified version of the original Google data.  Because of the query limitations imposed by Google, this function is used sparingly.
	*@param string $geo
	*@param boolean $isJson default is true.
	*@return Geoolocation information in either JSON or PHP associative array format.
	*/
	protected function getAddressByGeo($geo, $isJson = true){
		//The geocoordinates are presumed to be in the correct comma delimited format, so the API query is ran immediately.
		$url = 'http://maps.googleapis.com/maps/api/geocode/json?latlng='.$geo.'&sensor=false';
		//The information returned is decoded from JSON for PHP to work with the data easiery.
		$json = json_decode(file_get_contents($url), true);
		//The address is obtained by exploding the formatted address field out into an array.
		$address = explode(',', $json['results'][0]['formatted_address']);
		$results['address'] = trim($address[0]);
		$results['city'] = trim($address[1]);
		//The state and postal code have a slightly modified method of retreival.
		$stateZip = explode(' ', trim($address[2]));
		$results['state_region'] = $stateZip[0];
		$results['postal_code'] = $stateZip[1];
		$results['country'] = trim($address[3]);
		$output['status'] = 'success';
		$output['results'] = $results;
		$dma = $this->getDma($output['results']['lat'].','.$output['results']['lng'], false);
		if($dma['status'] == 'success'){
			$output['results']['dma'] = $dma['results'];
		}
		//Converts to JSON format unless otherwise specified, and output is returned.
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	/**
	*The getCityPostalByGeo function provides a lookup of the nearest city, state, and zip based in inputted geocoordinates.
	*@param string $val
	*@param boolean $isJson default is true.
	*@return Geolocation information in either JSON or PHP associative array format.
	*/
	protected function getCityPostalByGeo($val, $isJson = true){
		//The latitude and longitude is determined by a comma separation in $val.
		$coords = explode(',', $val);
		$lat = $coords[0];
		$lng = $coords[1];
		//Geolocation database is set, and a query is run to determine the closest city and state based on the lat/lon values.
		$query = "select *, round(degrees(acos(sin(radians(:lat))*sin(radians(lat))+cos(radians(:lat))*cos(radians(lat))*cos(radians(:lng - lng)))) *69.09) as distance from geolocation_cities where postal_code != '' order by distance asc limit 1";
		$insertArr = array(
			':lat'=>$lat,
			':lng'=>$lng,
		);
		$q = db_query($query, $insertArr);
		//Query results are good
		if($q){
			if($q->rowCount() >0){
				$r = $q->fetchAssoc();
				$output['status'] = 'success';
				$output['results'] = $r;
			}
			else{
				$output = $this->getAddressByGeo($val, false);
			}
			$dma = $this->getDma($output['results']['lat'].','.$output['results']['lng'], false);
			if($dma['status'] == 'success'){
				$output['results']['dma'] = $dma['results'];
			}
		}
		//No bueno
		else{
			$output['status'] = 'error';
			$output['results'] = 'Query failure';
		}
		//Output is converted to JSON unless the boolean was set to false, database set to default, and output is returned.
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	/**
	*The getDma function pulls up the closest possible DMA based on the inputted geolocation value.  Because of variations in DMA boundaries, this function may have a noticeable margin of error.
	*@param string $geo
	*@param boolean $isJson
	*@return DMA information
	*/
	protected function getDma($geo, $isJson = true){
		$geo = explode(',', $geo);
		//The next four lines draw up the latitude/longitude permiter to search in, and modified for PHP.  It uses the values from $lat, $lng, and $dist to perform the calculations.  It is based on that 1 degree of latitude is equivelant to 69.09 miles and 1 degree of longitude is equivelant to the cosign of the latitude times 69.09.  For more detailed information, see slides 10 and 11 of the GeoSpacial MySQL Search whitepaper at http://www.scribd.com/doc/2569355/Geo-Distance-Search-with-MySQL.
		$dist = 1000;
		$lat = $geo[0];
		$lng = $geo[1];
		$lng1 = $lng-$dist/abs(cos(deg2rad($lat))*69.09);
		$lng2 = $lng+$dist/abs(cos(deg2rad($lat))*69.09);
		$lat1 = $lat-($dist/69.09);
		$lat2 = $lat+($dist/69.09);
		//Output array is set
		$output = array();
		//Query is set and run
		$insertVals = array(
			':lat0'=>$lat,
			':lng0'=>$lng,
			':lat1'=>$lat1,
			':lat2'=>$lat2,
			':lng1'=>$lng1,
			':lng2'=>$lng2,
		);
		$query = "select *, round(degrees(acos(sin(radians(:lat0))*sin(radians(lat))+cos(radians(:lat0))*cos(radians(lat))*cos(radians(:lng0 - lng)))) *69.09) as distance from geolocation_dma where lng between :lng1 and :lng2 and lat between :lat1 and :lat2 order by distance limit 1";
		$q = db_query($query, $insertVals);
		//Outputs an error message if there was a problem with the query
		if(!$q){
			$output['status'] = 'error';
			$output['error_msg'] = 'query failure';
		}
		//If query is successful, results are outputted.
		else{
			$output['status'] = 'success';
			$output['results'] = $q->fetchAssoc();
		}
		//Output is converted into JSON by default, unless otherwise specified.
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	/**
	*The getGeoByAddress function does a latitude/longitude lookup based on the full address through the Google Maps Geocoder API.  Because of the query limitations imposed by Google, this function is used sparingly and only when exact street address based coordinates are needed.
	*@param string $address
	*@param boolean $isJson default is true
	*@return Latitude/longitude information in either JSON or PHP associative array format.
	*/
	protected function getGeoByAddress($address, $isJson = true){
		//Address is URL encoded to ensure the API query can be run properly.
		$address = urlencode($address);
		//API query is run
		$url = 'http://maps.googleapis.com/maps/api/geocode/json?address='.$address.'&sensor=false';
		//Data is decoded from JSON so PHP can work with it easier.
		$json = json_decode(file_get_contents($url), true);
		$address = explode(',', $json['results'][0]['formatted_address']);
		$results['city'] = trim($address[0]);
		//The state and postal code have a slightly modified method of retreival.
		$stateZip = explode(' ', trim($address[1]));
		$results['state_region'] = $stateZip[0];
		$results['postal_code'] = $stateZip[1];
		$results['country'] = trim($address[2]);
		$results['lat'] = $json['results'][0]['geometry']['location']['lat'];
		$results['lng'] = $json['results'][0]['geometry']['location']['lng'];
		$output['status'] = 'success';
		$output['results'] = $results;
		$dma = $this->getDma($output['results']['lat'].','.$output['results']['lng'], false);
		if($dma['status'] == 'success'){
			$output['results']['dma'] = $dma['results'];
		}
		//Converts to JSON format unless otherwise specified, and output is returned.
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	/**
	*The getGeoByAddressMq function is the MapQuest version of getGeoByAddress (lat/lon lookup by full address).  Because MaqQuest has more generous API lookups with their community edition, this function is designed for bulk lookups (i.e. location information importing).
	*@param array $address
	*@param boolean $isJson defaults to true.
	*@return The full street address geolocation information in either JSON or PHP associative array format.
	*/
	protected function getGeoByAddressMq($address, $isJson = true){
		if(!defined($this->mapquest_api)){
			$this->mapquest_api = $this->getApi('mapquest');
		}
		$results = array();
		//Counts out the number of addresses.
		$count = count($address);
		//Calls up the apiLimitCheck function to see how many API calls are remaining
		$limit = $this->apiLimitCheck($this->mapquest_api);
		//The system will resolve as many addresses as possible, but will omit any values exceeding the API limitation.
		if($limit['value'] < $count){
			$output['msg'] = 'Notice: '.$limit.' of the '.$count.' records without lat/lon values were updated due to API limitations.<br />';
			$count = $limit['value'];
		}
		//Sends a notice to the user when less than 1000 API calls remain.
		elseif($limit['value'] > $count){
			$remainder = $limit['value'] - $count;
			if($remainder > 1000){
				$output['msg'] = 'Notice: there are '.$remainder.' API calls remaining for this API\'s limitation settings.  Contact an admin for more information.<br />';
			}
		}
		$results = array();
		//MapQuest can run batches of 100 addresses at a time, so the batch number is determined by that fact.
		$batchNumber = ceil($count/100);
		//Loops though the number of batch lookups to be performed
		for($i=0; $i<$batchNumber; $i++){
			$start = $i*100;
			//Determines the number of lookups to be performed in the current batch, particularly because the last batch will very unlikely have 100 lookups.
			if($i+1 == $batchNumber){
				$end = $count;
			}
			else{
				$end = $start+100;
			}
			//The addresses in the batch are added to the location variable.
			$location = '';
			for($m=$start; $m<$end; $m++){
				$location .= '&location='.urlencode($address[$m]['address']);
			}
			//The URL is formulated. It utilizes an API key stored in the database.
			$url = 'http://www.mapquestapi.com/geocoding/v1/batch?key='.$this->mapquest_api->api_key.'&callback=renderBatch'.$location;
			//The content retreived is processed as necessary for PHP to efficiently read the incomming JSON
			$json = file_get_contents($url);
			$json = str_replace('renderBatch(', '', $json);
			$json = substr($json, 0, -2);
			$arr = json_decode($json, true);
			$rCount = count($arr['results']);
			//The results are looped through to populate the lat/lon information.
			for($r=0; $r<$rCount; $r++){
				$indexAt = $start+$r;
				$mqResults['index'] = $address[$indexAt]['index'];
				$mqResults['lat'] = $arr['results'][$r]['locations'][0]['displayLatLng']['lat'];
				$mqResults['lng'] = $arr['results'][$r]['locations'][0]['displayLatLng']['lng'];
				$results[] = $mqResults;
			}
		}
		//The database is updated with the number of API lookups performed
		$insert = db_insert('geolocation_api_track')
			->fields(array(
				'api_val_id'=>$this->mapquest_api->id,
				'insert_count'=>$count,
				'timestamp'=>REQUEST_TIME
			))
			->execute();
		///Results finalized, formatted as JSON unless specified otherwise, and returned.
		$output['status'] = 'success';
		$output['results'] = $results;
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	/**
	*The getIp function is able to resolve a user's IP address to their local area.  It will attempt to pull data from the local database first, and then reach out to freegeoip.net as a secondary measure.
	*@param string $ipAddress, boolean $json defaults as true.
	*@return The user's location information in either JSON or PHP associative array format.
	*/
	protected function getIp($ipAddress, $isJson = true){
		$output = array();
		//Changes to the proper database
		//The system will check to see if it can resolve the converted IP address first.
		$q = db_query("select loc_id from geolocation_ip_locations where INET_ATON(:ipa) between start_ip_num and end_ip_num limit 1", array(':ipa'=>$this->ipCheck($ipAddress)));
		//There was an error in the database query
		if(!$q){
			$output['status'] = 'error';
			$output['results'] = 'Query failure';
		}
		//There were results
		else{
			$r = $q->fetchAssoc();
			$locId = $r['loc_id'];
			//If the results come back blank, the system will run a REST API query to freegeoip.net
			if($locId == '' || is_null($locId)){
				$results = json_decode(file_get_contents("http://freegeoip.net/json/".$ipAddress),true);
				//The output array gets populated.
				$output['results']['country'] = $results['country_name'];
				$output['results']['state_region'] = $results['region_name'];
				$output['results']['city'] = $results['city'];
				$output['results']['postal_code'] = $results['zipcode'];
				$output['results']['lat'] = $results['latitude'];
				$output['results']['lng'] = $results['longitude'];
				$output['results']['metro_code'] = $results['metro_code'];
				$output['results']['area_code'] = $results['areacode'];
				$dataGood = true;
				//Ensures that at least latitude and longitude are populated, as the system has secondary methods to pull information.
				$checkFor = array('lat','lng');
				foreach($checkFor as $c){
					if($output['results'][$c] == '' || is_null($output['results'][$c])){
						$dataGood = false;
						break;
					}
				}
				//The lat/lon check passed, and the data is sent through the ipCorrect function to ensure necessary values are there.
				if($dataGood){
					$output['status'] = 'success';
					$output['results'] = $this->ipCorrect($output['results'], false);
				}
				//The lat/lon checks failed, no bueno!
				else{
					$output['status'] = 'error';
					$output['msg'] = 'Unable to resolve IP address';
				}
			}
			//A good result came back from the database, so the IP address will be resolved internally
			else{
				//Query is run against the geolocation cities table.
				$query2 = "select * from geolocation_cities where id = '".$locId."'";
				$q2 = db_query($query2);
				if($q2){
					//The results are sent through the ipCorrect function to ensure necessary values are there.
					$r2 = $q2->fetchAssoc();
					$output['status'] = 'success';
					$output['results'] = $this->ipCorrect($r2);
				}
			}
			$dma = $this->getDma($output['results']['lat'].','.$output['results']['lng'], false);
			if($dma['status'] == 'success'){
				$output['results']['dma'] = $dma['results'];
			}
		}
		//Data will be converted to JSON unless boolean is false.
		if($isJson){
			$output = json_encode($output);
		}
		//Data is returned to the callup source.
		return $output;
	}

	/**
	*The getIpv4 function is able to convert an IP Address into an Ipv4 value.  It is a backup just in case the database query can't perform this task directly.
	*@param string $ip
	*@return the converted Ipv4 value
	*/
	protected function getIpv4($ip){
		$ipArr = explode('.', $ip);
		$output =  (16777216 * $ipArr[0])+(65536 * $ipArr[1]) + (256 * $ipArr[2])+ $ipArr[3];
		return $output;
	}

	/**
	*The getPostalCode function provides a lookup of the City, State, Latitude, and Longitude of an inputted postal code value.
	*@param string $postalCode
	*@param boolean $isJson default is true.
	*@return Geolocation information in either JSON or PHP associative array format.
	*/	
	protected function getPostalCode($postalCode, $isJson = true){
		$output = array();
		//Query is run
		$q = db_query("select * from geolocation_cities where postal_code = :pc", array(':pc'=>$postalCode));
		//Successful query
		if($q){
			if($q->rowCount() > 0){
				$r = $q->fetchAssoc();
				$output['status'] = 'success';
				$output['results'] = $r;
				$dma = $this->getDma($output['results']['lat'].','.$output['results']['lng'], false);
				if($dma['status'] == 'success'){
					$output['results']['dma'] = $dma['results'];
				}
			}
			else{
				$output = $this->getGeoByAddress($postalCode, false);
			}
			
		}
		//No bueno.
		else{
			$output['status'] = 'error';
			$output['results'] = 'Query failure';
		}
		//Output converted to JSON by default.
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	/**
	*The getWeather function is able to pull a National Weather Service weather feed based on the user's geocoordinates.  Its main job is to simplify the data received from NWS, and provides a local cache to help with performance.
	*@param array $request
	*@param boolean $isJson, default is true
	*@return The weater information in either JSON or PHP associative array format.
	*/
	protected function getWeather($request, $isJson = true){
		$output = array();
		//NWS is set to pull weather based on geocoordinates.  If a postal code or ip address is detected, the system will call up the respective functions to retreive the geocoordinates.
		switch($request['subtype']){
			case 'postal_code':
				$results = $this->getPostalCode($request['value'], false);
				$lat = $results['results']['lat'];
				$lng = $results['results']['lng'];
				break;
			case 'ip_address':
				$results = $this->getIp($request['value'], false);
				$lat = $results['results']['lat'];
				$lng = $results['results']['lng'];
				break;
			//The default presumes geocoordinates are properly entered.  In this case, the city, state, and zip are obtained from the getCityPostalByGeo function.
			default:
				$geo = explode(',', $request['value']);
				$results = $this->getCityPostalByGeo($request['value'], false);
				$lat = $geo[0];
				$lng = $geo[1];
		}
		//Start and end times are set.  A custom end time can be specified from the user or API end using the PHP conversion technique.
		$start = REQUEST_TIME;
		$endString = '+1day';
		if(isset($request['end'])){
			$endString = '+'.$request['end'];
		}
		$end = strtotime($endString);
		//Because the NwS feed can be sluggish at times, the system is set up to store forecasts in the local database, and will attempt to check for a stored version less than 3 hours old with the same zipcode and forecast length.
		$timestamp = $start-(60*60*3);
		db_set_active('default');
		$qArray = array(':postal_code'=>$results['results']['postal_code'],':timestamp'=>$timestamp, ':forecast_length'=>$request['end']);
		$q = db_query('select * from geolocation_weather_cache where postal_code = :postal_code and timestamp >= :timestamp and forecast_length = :forecast_length order by timestamp desc limit 1', $qArray);
		$rowCount = $q->rowCount();
		//If a row was returned from the query, that information will be used.
		if($rowCount > 0){
			//Unnecessary rows from the database are removed.
			$remove = array('id', 'timestamp', 'forecast_length');;
			$r = $q->fetchAssoc();
			foreach($r as $k=>$v){
				if(in_array($k, $remove)){
					unset($r[$k]);
				}
			}
			//Database results are inputted into the output array
			$r['data'] = json_decode($r['data']);
			$output['status'] = 'success';
			$output['results'] = $r;
		}
		else{
			//Database caches older than three hours and matching the user's postal code and forecast length are deleted to conserve database space.
			db_query('delete from geolocation_weather_cache where postal_code = :postal_code and timestamp < :timestamp and forecast_length = :forecast_length', $qArray);
			//The time values are converted to the format needed by the NWS.
			$timeStart = date('Y-m-d', $start).'T'.date('H:i:s', $start);
			$timeEnd = date('Y-m-d', $end).'T'.date('H:i:s', $end);
			//NWS link is determined and ran.
			$nwsLink = 'http://graphical.weather.gov/xml/sample_products/browser_interface/ndfdXMLclient.php?lat='.$lat.'&lon='.$lng.'&product=time-series&begin='.$timeStart.'&end='.$timeEnd.'&Unit=e&maxt=maxt&mint=mint&temp=temp&pop12=pop12&qpf=qpf&rh=rh&sky=sky&wspd=wspd&wdir=wdir&wx=wx&icons=icons';
			$xml = simplexml_load_string(file_get_contents($nwsLink));
			//Ensures the processing will only occur if there is data returned.
			if($xml){
				//The content is scrubbed from the simplexml structure to make the data easier to work with.
				$content = $this->objectToArray(json_decode(json_encode($xml), true));
				$output['status'] = 'success';
				//The necessary location information, API feed source, and NwS weather page are provided.
				$output['results']['city'] = $results['results']['city'];
				$output['results']['state_region'] = $results['results']['state_region'];
				$output['results']['postal_code'] = $results['results']['postal_code'];
				$output['results']['lat'] = $lat;
				$output['results']['lng'] = $lng;
				$output['results']['nws_xml'] = $nwsLink;
				$output['results']['nws_page'] = $content['data']['moreWeatherInformation'];
				//NWS feeds have a funky time matrix that can be tough to decipher.  This part goes through and extracts the necessary layout keys for later use.
				//The tTimes array will have repeititive use throughout the code.
				$tTimes = array('start_valid_time', 'end_valid_time');
				$timeLayout = array();
				foreach($content['data']['time-layout'] as $tl){
					$layoutKey = $tl['layout-key'];
					foreach($tTimes as $tt){
						//Values with underscores are easier to read with JSON, but the feed uses dashes.  This section replaces the underscores with the dashes to better perform the search in the time matrix.
						$searchFor = str_replace('_', '-', $tt);
						if(array_key_exists($searchFor, $tl)){
							$timeLayout[$layoutKey][$tt] = $tl[$searchFor];
						}
					}
				}
				//Puts the data paramaters into a different variable so it's easier to sort through in the remainder of the function.
				$params = $content['data']['parameters'];
				//The weather conditions icons are extracted from the code, and the appropriate start and end times are added 
				$citl = $params['conditions-icon']['@attributes']['time-layout'];
				$iCount = 0;
				foreach($params['conditions-icon']['icon-link'] as $i){
					//Value is determined, and each tTime value is gone through to ensure the necessary time values are added.
					$input['value'] = $i;
					foreach($tTimes as $tt){
						if(array_key_exists($tt, $timeLayout[$citl])){
							$input[$tt] = $timeLayout[$citl][$tt][$iCount];
						}
					}
					$output['results']['data']['icon'][] = $input;
					$iCount++;
				}
				//The weather conditions attributes are extracted from the code, and the appropriate start and end times are added. 
				$wCount = 0;
				$wtl = $params['weather']['@attributes']['time-layout'];
				foreach($params['weather']['weather-conditions'] as $weather){
					$input['value'] = '';
					//Each attribute is put into a string
					if(count($weather) > 0){
						if(count($weather['value']['@attributes']) > 0){
							foreach($weather['value']['@attributes'] as $a){
								$input['value'] .= $a.' ';
							}
						}
					}
					//Each tTime value is gone through to ensure the necessary time values are added.
					foreach($tTimes as $tt){
						if(array_key_exists($tt, $timeLayout[$wtl])){
							$input[$tt] = $timeLayout[$wtl][$tt][$wCount];
						}
					}
					$output['results']['data']['weather_conditions'][] = $input;
					$wCount++;
				}
				//Each temperature type is extracted from the code, and the appropriate start and end times are added. 
				foreach($params['temperature'] as $temp){
					//The temperature type and time layout is determined.
					$type = $temp['@attributes']['type'].'_temp';
					$tl = $temp['@attributes']['time-layout'];
					$tCount = 0;
					foreach($temp['value'] as $t){
						//Value is determined, and each tTime value is gone through to ensure the necessary time values are added.
						$input['value'] = $t.'&deg;F';
						foreach($tTimes as $tt){
							if(array_key_exists($tt, $timeLayout[$tl])){
								$input[$tt] = $timeLayout[$tl][$tt][$tCount];
							}
						}
						$output['results']['data'][$type][] = $input;
						$tCount++;
					}
				}
				//The precipitation attributes are extracted from the code, and the appropriate start and end times are added. 
				$ptl = $params['precipitation']['@attributes']['time-layout'];
				$pCount = 0;
				foreach($params['precipitation']['value'] as $value){
					//Value is determined, and each tTime value is gone through to ensure the necessary time values are added.
					$input['value'] = $value.' inches';
					foreach($tTimes as $tt){
						if(array_key_exists($tt, $timeLayout[$ptl])){
							$input[$tt] = $timeLayout[$ptl][$tt][$pCount];
						}
					}
					$output['results']['data']['precipitation'][] = $input;
					$pCount++;
				}
				//The cloud cover attributes are extracted from the code, and the appropriate start and end times are added. 
				$ctl = $params['cloud-amount']['@attributes']['time-layout'];
				$cCount = 0;
				foreach($params['cloud-amount']['value'] as $value){
					//Value is determined, and each tTime value is gone through to ensure the necessary time values are added.
					$input['value'] = $value.'%';
					foreach($tTimes as $tt){
						if(array_key_exists($tt, $timeLayout[$ctl])){
							$input[$tt] = $timeLayout[$ctl][$tt][$cCount];
						}
					}
					$output['results']['data']['cloud_cover'][] = $input;
					$cCount++;
				}
				//The 12 hour precipitation probability attributes are extracted from the code, and the appropriate start and end times are added. 
				$ptl = $params['probability-of-precipitation']['@attributes']['time-layout'];
				$pCount = 0;
				foreach($params['probability-of-precipitation']['value'] as $value){
					//Value is determined, and each tTime value is gone through to ensure the necessary time values are added.
					$input['value'] = $value.'%';
					foreach($tTimes as $tt){
						if(array_key_exists($tt, $timeLayout[$ptl])){
							$input[$tt] = $timeLayout[$ptl][$tt][$pCount];
						}
					}
					$output['results']['data']['12_hour_precip_prob'][] = $input;
					$pCount++;
				}
				//The humidity attributes are extracted from the code, and the appropriate start and end times are added. 
				$htl = $params['humidity']['@attributes']['time-layout'];
				$hCount = 0;
				foreach($params['humidity']['value'] as $value){
					//Value is determined, and each tTime value is gone through to ensure the necessary time values are added.
					$input['value'] = $value.'%';
					foreach($tTimes as $tt){
						if(array_key_exists($tt, $timeLayout[$htl])){
							$input[$tt] = $timeLayout[$htl][$tt][$hCount];
						}
					}
					$output['results']['data']['humidity'][] = $input;
					$hCount++;
				}
				//The wind direction attributes are extracted from the code, and the appropriate start and end times are added. 
				$wdtl = $params['direction']['@attributes']['time-layout'];
				$wdCount = 0;
				foreach($params['direction']['value'] as $value){
					//Value is determined, and each tTime value is gone through to ensure the necessary time values are added.
					$input['value'] = $this->degreeToDirection($value);
					foreach($tTimes as $tt){
						if(array_key_exists($tt, $timeLayout[$wdtl])){
							$input[$tt] = $timeLayout[$wdtl][$tt][$wdCount];
						}
					}
					$output['results']['data']['wind_dir'][] = $input;
					$wdCount++;
				}
				//The wind speed attributes are extracted from the code, and the appropriate start and end times are added. 
				$wstl = $params['wind-speed']['@attributes']['time-layout'];
				$wsCount = 0;
				foreach($params['wind-speed']['value'] as $value){
					//Value is determined, and each tTime value is gone through to ensure the necessary time values are added.
					$input['value'] = $this->knotsToMiles($value). ' MPH';
					foreach($tTimes as $tt){
						if(array_key_exists($tt, $timeLayout[$wstl])){
							$input[$tt] = $timeLayout[$wstl][$tt][$wsCount];
						}
					}
					$output['results']['data']['wind_speed'][] = $input;
					$wsCount++;
				}
				//The field names are extracted from the array to populate into the database.
				$fields = array();
				foreach($output['results'] as $k=>$v){
					$fields[] = $k;
				}
				//Tmestamp and forecast length are added to the fields
				$fields[] = 'timestamp';
				$fields[] = 'forecast_length';
				//The values variable is formatted as an array for database insertion.g
				$values = $output['results'];
				$values['timestamp'] = REQUEST_TIME;
				//The data gets JSON encoded for database storage.
				$values['data'] = json_encode($values['data']);
				$values['forecast_length'] = $endString;
				//Insertion query is run
				db_insert('geolocation_weather_cache')->fields($fields)->values($values)->execute();
				$output['msg'] = 'Got from NWS directly';
			}
		}
		//The output is converted to JSON unless otherwise specified, and then returned.
		if($isJson){
			$output = json_encode($output);
		}
		return $output;
	}

	/**
	*The ipCheck function provides a default IP in the instance of the site being run in a local dev environment
	*@param string $ip
	*@return The ip will be passed back as entered unless an array match is found, in which case it will enter a Kirkland, WA based IP.
	*/
	protected function ipCheck($ip){
		$convert = array('127.0.0.1', '::1');
		if(in_array($ip, $convert)){
			$ip = '24.19.187.86';
		}
		return $ip;
	}

	/**
	*The ipCorrect function scrubs the inputted data to ensure city, state/region, and postal code are present, and will update the database if called for.
	*@param array $row
	*@param boolean $update default is true.
	*@return The $row array is passed back with corrected values as needed.
	*/
	protected function ipCorrect($row, $update = true){
		//Values checked for are in an array, and checked against the inputted array for a blank value.
		$checkFor = array('city','state_region','postal_code');
		$correctIt = false;
		foreach($checkFor as $c){
			if($row[$c] == '' || is_null($row[$c])){
				$correctIt = true;
				break;
			}
		}
		//If a blank value was detected, the system will call up the getAddressByGeo function to perform a lookup via the Google Geolocator API.
		if($correctIt){
			$geo = $row['lat'].','.$row['lng'];
			//For more info, see the getAddressByGeo function.
			$getNew = $this->getAddressByGeo($geo, false);
			$insert = array();
			//The updated values are inserted into the $row and $insert arrays
			foreach($checkFor as $c){
				$row[$c] = $getNew['results'][$c];
				$insert[$c] = $getNew['results'][$c];
			}
			//If an update is indicated, the system will automatically update the internal database with the new information.
			if($update){
				$q = db_update('cities')->fields($insert)->condition('id', $row['id'], '=')->execute();
			}
		}
		//Data is returned.
		return $row;
	}

	/**
	*The knotsToMiles function converts nautical miles into standard miles.  It is used in conjunction with the getWeather function.
	*@param string $speed
	*@return The conversion from knots to miles rounded to the nearest 100th.
	*/
	protected function knotsToMiles($speed){
		return round(($speed*1.15078), 2);
	}

	/**
	*The objectToArray function converts a data object into an associative array.
	*@param object/array $data
	*@return the $data array in associative array format.
	*/
	protected function objectToArray($data){
	    if (is_array($data) || is_object($data)){
	        $result = array();
	        foreach ($data as $key => $value){
	            $result[$key] = $this->objectToArray($value);
	        }
	        return $result;
	    }
	    return $data;
	}

	/**
	*The addressForm function returns the Drupal-based form settings for the reverse geolocation by address lookup.
	*@param array $form
	*@param array $form_state
	*@return array $form
	*/
	public function address_form($form, &$form_state){
		$form['address'] = array(
			'#type'=>'textfield',
			'#title'=>'Address',
			'#size'=>20,
			'#required'=>true,
		);
		$form['city'] = array(
			'#type'=>'textfield',
			'#title'=>'City',
			'#required'=>true,
			'#size'=>20,
		);
		$form['state'] = array(
			'#type'=>'textfield',
			'#title'=>'State',
			'#required'=>true,
			'#size'=>5,
		);
		$form['postal_code'] = array(
			'#type'=>'textfield',
			'#title'=>'Postal Code',
			'#size'=>20,
		);
		$form['submit_button'] = array(
			'#type'=>'submit',
			'#value'=>'Lookup',
		);
		return t($form);
	}

	/**
	*The addressFormValidate function ensures the necessary values are filled out.
	*@param array $form
	*@param array $form_state
	*@return Calls up the form_set_error function if any of the required values are blank
	*/
	public function address_form_validate($form, &$form_state){
		$checkFor = array('address', 'city', 'state');
		$msg = '';
		foreach($checkFor as $c){
			if (($form_state['values'][$c] == '')){
				if($msg != ''){
					$msg .= '<br />';
				}
				$msg .= 'The '.$c.' value is required.';
			}
		}
		if($msg != ''){
			form_set_error($c, t('The value for '.$c.' is required.'));
		}
	}

	/**
	*The addressFormSubmit function processes the reverse geolocation lookup by address and returns the results to the user.
	*@param array $form
	*@param array $form_state
	*@return The results in HTML format sent out to the drupal_set_message function
	*/
	public function address_form_submit($form, &$form_state){
		//The address is arranged in a format that is easy to be read by Google.
		$address = str_replace(' ', '+', $form_state['values']['address']).',+'.$form_state['values']['city'].',+'.$form_state['values']['state'];
		//Postal code is added if filled out.
		if($form_state['values']['postal_code'] != ''){
			$address .= ',+'.$form_state['values']['postal_code'];
		}
		//The getGeoByAddress function is called up, with a false value set to return the results as a PHP array.
		$results = $this->getGeoByAddress($address, false);
		//Output HTML is Determined
		$output = 'Latitude: '.$results['results']['lat'].'<br />';
		$output .= 'Longitude: '.$results['results']['lng'].'<br />';
		$output .= '<a href="'.$GLOBALS['base_url'].'/geolocation/export?type=geo_by_address&value='.$address.'">Export URL</a>';
		//drupal_set_message is called
		drupal_set_message(t($output));
	}

	/**
	*The export1 function processes the REST API callups to the application via the geolocation/export page, and returns the requested data in JSON format.
	*@param array $_REQUEST
	*@return the requested information is printed out in JSON format.
	*/
	public function export1(){
		//The type value in the array must be set.
		if(isset($_REQUEST['type'])){
			//The access control headers are modified to allow cross-domain requests.
			header("Access-Control-Allow-Origin: *");
			header('Access-Control-Allow-Methods', 'GET POST');

			//Switch statement performed for type
			switch($_REQUEST['type']){
				//DMA infromation is returned
				case 'dma':
					if(!isset($_REQUEST['value'])){
						echo $this->error();
					}
					else{
						echo $this->getDma($_REQUEST['value']);
					}
					break;
				//Location by IP address data is returned.
				case 'ip_address':
					if(!isset($_REQUEST['value'])){
						echo $this->getIp($_SERVER['REMOTE_ADDR']);
					}
					else{
						echo $this->getIp($_REQUEST['value']);
					}
					break;
				//Location by postal code data is returned.
				case 'postal_code':
					if(!isset($_REQUEST['value'])){
						echo $this->error();
					}
					else{
						echo $this->getPostalCode($_REQUEST['value']);
					}
					break;
				//Geolocation by address data is returned.
				case 'geo_by_address':
					if(!isset($_REQUEST['value'])){
						echo $this->error();
					}
					else{
						echo $this->getGeoByAddress($_REQUEST['value']);
					}
					break;
				//Address by geolocation data is returned.
				case 'address_by_geo':
					if(!isset($_REQUEST['value'])){
						echo $this->error();
					}
					else{
						echo $this->getAddressByGeo($_REQUEST['value']);
					}
					break;
				//City-based information by geolocation is returned.
				case 'city_postal_by_geo':
					if(!isset($_REQUEST['value'])){
						echo $this->error();
					}
					else{
						echo $this->getCityPostalByGeo($_REQUEST['value']);
					}
					break;
				//Weather information data is returned
				case 'weather':
					if($_REQUEST['subtype'] != 'ip_address' && $_REQUEST['value'] == ''){
						echo $this->error();
						break;
					}
					if($_REQUEST['subtype'] == 'ip_address' && (!isset($_REQUEST['value']) || $_REQUEST['value'] == '')){
						$_REQUEST['value'] = $this->ipCheck($_SERVER['REMOTE_ADDR']);
					}
					echo $this->getWeather($_REQUEST);
					break;
			}
			
		}
		//The type wasn't specified, so the error function is called up to print out a generic error message.
		else{
			echo $this->error();
		}
	}

	/**
	*The ip_form function returns the Drupal-based form settings for the IP Address lookup form
	*@param array $form
	*@param array $form_state
	*@return array $form
	*/
	public function ip_form($form, &$form_state){
		$form['description'] = array(
			'#type'=>'markup',
			'#title'=>'Instructions',
			'#markup'=>'The field may be left blank to resolve your current IP Address of '.$this->ipCheck($_SERVER['REMOTE_ADDR']),
		);
		$form['ip_address'] = array(
			'#type'=>'textfield',
			'#title'=>'IP Address',
			'#size'=>20,
		);
		$form['submit_button'] = array(
			'#type'=>'submit',
			'#value'=>'Lookup',
		);
		return t($form);
	}

	/**
	*The ip_form_submit function processes the IP Address lookup and returns the results to the user.
	*@param array $form
	*@param array $form_state
	*@return The results in HTML format sent out to the drupal_set_message function
	*/
	public function ip_form_submit($form, &$form_state){
		//The IP address will be the user's address by default, unless the ip_address form value is filled in.
		$ipAddress = $this->ipCheck($_SERVER['REMOTE_ADDR']);
		if($form_state['values']['ip_address'] != ''){
			$ipAddress = $form_state['values']['ip_address'];
		}
		//The getIp function is called up, with a false value set to return the results as a PHP array.
		$results = $this->getIp($ipAddress, false);
		//Output HTML is determined.
		$output = $results['results']['city'].', '.$results['results']['state_region']. ' '.$results['results']['postal_code'].'<br />';
		$output .= 'Latitude: '.$results['results']['lat'].'<br />';
		$output .= 'Longitude: '.$results['results']['lng'].'<br />';
		$output .= '<a href="'.$GLOBALS['base_url'].'/geolocation/export?type=ip_address&value='.$ipAddress.'" target="_blank">Export URL</a>';
		//$output .= '<br />ip is ~'.$form_state['values']['ip_address'].'~.';
		//drupal_set_message is called up.
		drupal_set_message(t($output));
	}

	/**
	*The latLon1_form function returns the Drupal-based form settings for the City lookup by lat/lon.
	*@param array $form
	*@param array $form_state
	*@return array $form
	*/
	public function latlon1_form($form, &$form_state){
		$form['lat'] = array(
			'#type'=>'textfield',
			'#title'=>'Latitude',
			'#required'=>true,
			'#size'=>20,
			'#attributes' => array(
    			'class' => array('lat'),
  			),
		);
		$form['lon'] = array(
			'#type'=>'textfield',
			'#title'=>'Longitude',
			'#required'=>true,
			'#size'=>20,
			'#attributes' => array(
    			'class' => array('lng'),
  			),
		);
		$form['lat_lon_lookup'] = array(
			'#type'=>'checkbox',
			'#title'=>'Use Current Lat/Lon',
			'#attributes' => array(
    			'class' => array('lat_lng_lookup'),
  			),
		);
		$form['submit_button'] = array(
			'#type'=>'submit',
			'#value'=>'Lookup',
		);
		return t($form);
	}

	/**
	*The latLon1_form_validate function ensures the latitude and longitude code value are filled out.
	*@param array $form
	*@param array $form_state
	*@return Calls up the form_set_error function if either of the two values are blank
	*/
	public function latLon1_form_validate($form, &$form_state){
		if (($form_state['values']['lat'] == '' || $form_state['values']['lat'] == '')){
			form_set_error('postal_code', t('Latitude/Longitude values are required.'));
		}
	}

	/**
	*The latLon1_form_submit function processes the city lookup by lat/lon and returns the results to the user.
	*@param array $form
	*@param array $form_state
	*@return The results in HTML format sent out to the drupal_set_message function
	*/
	public function latLon1_form_submit($form, &$form_state){
		//Combines the lat and lon values for the standard comma-separated value.
		$latLon = $form_state['values']['lat'].','.$form_state['values']['lon'];
		//The getCityPostalByGeo function is called up, with a false value set to return the results as a PHP array.
		$results = $this->getCityPostalByGeo($latLon, false);
		//Output HTML is determined.
		$output = $results['results']['city'].', '.$results['results']['state_region']. ' '.$results['results']['postal_code'].'<br />';
		$output .= '<a href="'.$GLOBALS['base_url'].'/geolocation/export?type=city_postal_by_geo&value='.$latLon.'" target="_blank">Export URL</a>';
		//drupal_set_message is called up.
		drupal_set_message(t($output));
	}

	/**
	*The latLon1_form function returns the Drupal-based form settings for the full address lookup by lat/lon.
	*@param array $form
	*@param array $form_state
	*@return array $form
	*/
	public function latLon2_form($form, &$form_state){
		$form['lat'] = array(
			'#type'=>'textfield',
			'#title'=>'Latitude',
			'#required'=>true,
			'#size'=>20,
			'#attributes' => array(
    			'class' => array('lat'),
  			),
		);
		$form['lon'] = array(
			'#type'=>'textfield',
			'#title'=>'Longitude',
			'#required'=>true,
			'#size'=>20,
			'#attributes' => array(
    			'class' => array('lng'),
  			),
		);
		$form['lat_lon_lookup'] = array(
			'#type'=>'checkbox',
			'#title'=>'Use Current Lat/Lon',
			'#attributes' => array(
    			'class' => array('lat_lng_lookup'),
  			),
		);
		$form['submit_button'] = array(
			'#type'=>'submit',
			'#value'=>'Lookup',
		);
		return t($form);
	}

	/**
	*The latLon2_form_validate function ensures the latitude and longitude code value are filled out.
	*@param array $form
	*@param array $form_state
	*@return Calls up the form_set_error function if either of the two values are blank
	*/
	public function latLon2_form_validate($form, &$form_state){
		if (($form_state['values']['lat'] == '' || $form_state['values']['lat'] == '')){
			form_set_error('postal_code', t('Latitude/Longitude values are required.'));
		}
	}

	/**
	*The latLon2_form_submit function processes the address lookup by lat/lon and returns the results to the user.
	*@param array $form
	*@param array $form_state
	*@return The results in HTML format sent out to the drupal_set_message function
	*/
	public function latLon2_form_submit($form, &$form_state){
		//Combines the lat and lon values for the standard comma-separated value.
		$latLon = $form_state['values']['lat'].','.$form_state['values']['lon'];
		//The getAddressByGeo function is called up, with a false value set to return the results as a PHP array.
		$results = $this->getAddressByGeo($latLon, false);
		//Output HTML is determined.
		$output = $results['results']['address'].'<br />'.$results['results']['city'].', '.$results['results']['state_region']. ' '.$results['results']['postal_code'].'<br />';
		$output .= '<a href="'.$GLOBALS['base_url'].'/geolocation/export?type=address_by_geo&value='.$latLon.'" target="_blank">Export URL</a>';
		//drupal_set_message is called up.
		drupal_set_message(t($output));
	}

	/**
	*The menu function containes all of the UI menu items for the user to interact with.
	*@return The $items array is returned for Drupal to properly process each menu item and what to do for each page callup.
	*/
	public function menu(){
		//The IP Address lookup menu item
		$items['geolocation/ip_address'] = array(
			'title'=>'IP Address Lookup',
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_basic_ip_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
			'type'=>MENU_CALLBACK,
		);
		//The Postal Code lookup menu item
		$items['geolocation/postal_code'] = array(
			'title'=>'Postal Code Lookup',
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_basic_pc_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
			'type'=>MENU_CALLBACK,
		);
		//The City, State, Zip lookup by lat/lon
		$items['geolocation/lat_lon_city'] = array(
			'title'=>'City Lookup',
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_basic_latlon1_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
			'type'=>MENU_CALLBACK,
		);
		//The comprehensive address lookup by lat/lon
		$items['geolocation/lat_lon_address'] = array(
			'title'=>'Reverse Street Address Lookup',
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_basic_latlon2_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
			'type'=>MENU_CALLBACK,
		);
		//The lat/lon lookup by address
		$items['geolocation/address'] = array(
			'title'=>'Street Address Lookup',
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_basic_address_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
			'type'=>MENU_CALLBACK,
		);
		//The export page for API callups.
		$items['geolocation/export'] = array(
			'title'=>'Geolocation Export',
			'type'=>MENU_CALLBACK,
			'page callback'=>'geolocation_basic_export1',
			'access arguments'=>array('access content'),
		);
		//The weather page
		$items['geolocation/weather'] = array(
			'title'=>'Geolocation Weather',
			'type'=>MENU_CALLBACK,
			'page callback'=>'drupal_get_form',
			'page arguments'=>array('geolocation_basic_weather_form'),
			'access arguments'=>array('access geolocation content'),
			'access callback'=>'user_access',
		);
		return t($items);
	}

	/**
	*The permission function returns to Drupal the permission settings.
	*@return The permission settings array.
	*/
	public function permission(){
		return array(
	    	'access geolocation content' => array(
	     		'title' => t('Access Geolocation Content'),
	    	)
	  	);
	}

	/**
	*The pcForm function returns the Drupal-based form settings for the Postal Code lookup form
	*@param array $form
	*@param array $form_state
	*@return array $form
	*/
	public function pc_form($form, &$form_state){
		$form['postal_code'] = array(
			'#type'=>'textfield',
			'#title'=>'Postal Code',
			'#required'=>TRUE,
			'#size'=>20,
		);
		$form['submit_button'] = array(
			'#type'=>'submit',
			'#value'=>'Lookup',
		);
		return t($form);
	}

	/**
	*The pc_form_validate function ensures the postal code value is filled out.
	*@param array $form
	*@param array $form_state
	*@return Calls up the form_set_error function if the postal code value is blank
	*/
	public function pc_form_validate($form, &$form_state){
		if (($form_state['values']['postal_code'] == '')){
			return form_set_error('postal_code', t('A Postal Code is required.'));
		}
	}

	/**
	*The pc_form_submit function processes the Postal Code lookup and returns the results to the user.
	*@param array $form
	*@param array $form_state
	*@return The results in HTML format sent out to the drupal_set_message function
	*/
	public function pc_form_submit($form, &$form_state){
		//The getPostalCode function is called up, with a false value set to return the results as a PHP array.
		$results = $this->getPostalCode($form_state['values']['postal_code'], false);
		//Output HTML is determined.
		$output = $results['results']['city'].', '.$results['results']['state_region']. ' '.$results['results']['postal_code'].'<br />';
		$output .= 'Latitude: '.$results['results']['lat'].'<br />';
		$output .= 'Longitude: '.$results['results']['lng'].'<br />';
		$output .= '<a href="'.$GLOBALS['base_url'].'/geolocation/export?type=postal_code&value='.$form_state['values']['postal_code'].'" target="_blank">Export URL</a>';
		//drupal_set_message is called up.
		drupal_set_message(t($output));
	}

	/**
	*The weatherForm function returns the Drupal-based form settings for the weather lookup, along with lookup instructions.
	*@param array $form
	*@param array $form_state
	*@return array $form
	*/
	public function weather_form($form, &$form_state){
		$form['description'] = array(
			'#type'=>'markup',
			'#title'=>'Instructions',
			'#markup'=>'Enter either Latitude/Longitude coordinates, a postal code, or an IP address for real time weather information.  Latitude/Longitude will take precidence over postal code, and postal code will take precidence over an IP address.  If no fields are entered, the system will pull information based on your current IP address.  Default weather results are for 1 full day, unless a value is otherwise indicated in the Date Range field.',
		);
		$form['lat'] = array(
			'#type'=>'textfield',
			'#title'=>'Latitude',
			'#size'=>20,
			'#attributes' => array(
    			'class' => array('lat'),
  			),
		);
		$form['lng'] = array(
			'#type'=>'textfield',
			'#title'=>'Longitude',
			'#size'=>20,
			'#attributes' => array(
    			'class' => array('lng'),
  			),
		);
		$form['lat_lon_lookup'] = array(
			'#type'=>'checkbox',
			'#title'=>'Use Current Lat/Lon',
			'#attributes' => array(
    			'class' => array('lat_lng_lookup'),
  			),
		);
		$form['postal_code'] = array(
			'#type'=>'textfield',
			'#title'=>'Postal Code',
			'#size'=>20,
		);
		$form['ip_address'] = array(
			'#type'=>'textfield',
			'#title'=>'IP Address',
			'#size'=>20,
		);
		//The date range (end) field can utilize the PHP technique for determining the end time, i.e. +2 days
		$form['end'] = array(
			'#type'=>'textfield',
			'#title'=>'Date Range',
			'#size'=>20,
		);
		$form['submit_button'] = array(
			'#type'=>'submit',
			'#value'=>'Lookup',
		);
		return t($form);
	}

	/**
	*The weatherFormSubmit function processes the weather lookup and returns the results to the user.
	*/
	public function weather_form_submit($form, &$form_state){
		$request = array();
		$output = '';
		//Inserts the end value into the $request array if it is set.
		if($form_state['values']['end'] != ''){
			$request['end'] = $form_state['values']['end'];
		}
		//If both latitude and longitude values are filled out, they take precidence and the weather information is determined by these values.
		if($form_state['values']['lat'] != '' && $form_state['values']['lng'] != ''){
			$request['subtype'] = 'latlng';
			$request['value'] = $form_state['values']['lat'].','.$form_state['values']['lng'];
		}
		//If lat/lon were not filled out, but the postal code is, the weather is looked up by postal code as the location.
		elseif($form_state['values']['postal_code'] != ''){
			$request['subtype'] = 'postal_code';
			$request['value'] = $form_state['values']['postal_code'];
		}
		//The weather will be looked up by using the user's IP address as the location
		else{
			$request['subtype'] = 'ip_address';
			//The system will take the IP address filled out by the user, or utilize the user's current IP if it isn't filled out.
			if($form_state['values']['ip_address'] != ''){
				$request['value'] = $form_state['values']['ip_address'];
			}
			else{
				$request['value'] = $this->ipCheck($_SERVER['REMOTE_ADDR']);
			}
		}
		//Export URL is determined
		$exportUrl = $GLOBALS['base_url'].'/geolocation/export?type=weather&subtype='.$request['subtype'].'&value='.$request['value'];
		if($form_state['values']['end'] != ''){
			$exportUrl .= '&end=+'.urlencode($form_state['values']['end']);
		}
		//The getWeather function is called up, with a false value set to return the results as a PHP array.
		$results = $this->getWeather($request, false);
		//If the results do include weather information...
		if($results['status'] = 'success'){
			//The human-readable location, NWS source XML, and comprehensive NWS weather page are added to the $output variable.
			$output .= '<h2>Weather Information For '.$results['results']['city'].', '.$results['results']['state_region'].'</h2>';
			$output .= '<p><a href="'.$exportUrl.'" target="_blank">Export URL</a><br />';
			$output.='<a href="'.$results['results']['nws_xml'].'" target="_blank">NWS XML Source</a><br />';
			$output.= '<a href="'.$results['results']['nws_page'].'" target="_blank">NWS Page</a></p>';
			//Weather for current day, which will show the overall weather icon, current temp, high temp, preciptation, cloud cover, 12 hour precipitation probability, humidity, and wind data.
			$output .= '<div><h3>Today</h3>';
			$output .= '<img src="'.$results['results']['data']['icon'][0]['value'].'" /><br />';
			$output .= 'Temperature: '.$results['results']['data']['hourly_temp'][0]['value'].' - '.$results['results']['data']['maximum_temp'][0]['value'].'<br />';
			$output .= 'Precipitation: '.$results['results']['data']['precipitation'][0]['value'].'<br />';
			$output .= 'Cloud Cover: '.$results['results']['data']['cloud_cover'][0]['value'].'<br />';
			$output .= '12 Hour Probability of Precipitation: '.$results['results']['data']['12_hour_precip_prob'][0]['value'].'<br />';
			$output .= 'Humidity: '.$results['results']['data']['humidity'][0]['value'].'<br />';
			$output .= 'Winds out of the '.$results['results']['data']['wind_dir'][0]['value'].' at '.$results['results']['data']['wind_speed'][0]['value'].'</div>';
			$days = count($results['results']['data']['maximum_temp']);
			//If the results have multiple days, the system will loop through and add in the name of the day, weather icon, minimum temp, and maximum temp.
			if($days > 1){
				$output .= '<div id="weather_days">';
				for($i=1; $i<$days; $i++){
					//Ensures the time value can be properly read.
					$wDate = explode('T', $results['results']['data']['maximum_temp'][$i]['start_valid_time']);
					//A regular expression for the date is formulated because of the wide variations in the time coverage.
					$wdc = '/'.$wDate[0].'T11:00:00/i';
					//Only the icon is needed for the time being, but the other values are there just in case.
					$wVals = array(
						'precipitation'=>'',
						'cloud_cover'=>'',
						'icon'=>'',
					);
					//Each of the values are looped through to extract the proper information.
					foreach($wVals as $w=>$ww){
						for($t=0; $t<count($results['results']['data'][$w]); $t++){
							if(preg_match($wdc, $results['results']['data'][$w][$t]['start_valid_time'])){
								$wVals[$w] = $results['results']['data'][$w][$t]['value'];
								break;
							}
						}
					}
					//The date-specific weather information is added to the output.
					$output .= '<div><h3>'.date('l', strtotime($wDate[0])).'</h3>';
					$output .= '<img src="'.$wVals['icon'].'" /><br />';
					$output .= $results['results']['data']['minimum_temp'][$i-1]['value'].' - '.$results['results']['data']['maximum_temp'][$i]['value'].'</div>';
				}
				$output .= '</div>';
			}
		}
		//If no results were returned, the system grabs the results message for output.
		else{
			$output .= $results['msg'];
		}
		//drupal_set_message is run
		drupal_set_message(t($output));
	}
}